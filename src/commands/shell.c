#include "commands.h"

#include "editor.h"
#include "event.h"
#include "term.h"
#include "ui.h"
#include "window.h"
#include "utils.h"
#include "cursor.h"

#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// TODO: shell should also able to interpret commands as config file command,
// the exec command will call to the interpreter

typedef void (*CommandHandler)(int fd, int argc, char **argv);

typedef struct ShellCommand
{
    const char *name;
    CommandHandler handler;
    int min_args;
    int max_args; // -1 for unlimited
} ShellCommand;

typedef struct ShellAlias
{
    const char *name;
    const char *command;
} ShellAlias;

static const ShellCommand BUILTIN_COMMANDS[] = {
    {"quit",    command_handler_quit,       0, 0},
    {"find",    command_handler_find,       0, 1},
    {"line",    command_handler_line,       0, 0},
    {"save",    command_handler_save,       0, 1},
    {"open",    command_handler_open,       0, 0},
    {"tabs",    command_handler_tabs,       0, 0}
};

// TODO: alias customizable from the config file
static const ShellAlias BUILTIN_ALIASES[] = {
    {"q",       "quit"},
    {"f",       "find"},
    {"s",       "save"},
    {"o",       "open"}
};

#define CMD_NUM (sizeof(BUILTIN_COMMANDS)/sizeof(ShellCommand))
#define ALIAS_NUM (sizeof(BUILTIN_ALIASES)/sizeof(ShellAlias))

#define MAX_CMD_ARGS 8

static int commandExec(int fd, const char *command, int len)
{
    if (len == 0)
        return 0;

    char *cmd_copy = malloc(len + 1);
    if (!cmd_copy)
        return -1;
    memcpy(cmd_copy, command, len);
    cmd_copy[len] = '\0';

    char *argv[MAX_CMD_ARGS];
    int argc = 0;

    char *token = strtok(cmd_copy, " \t");
    while (token != NULL && argc < MAX_CMD_ARGS)
    {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }

    if (argc == 0)
    {
        free(cmd_copy);
        return 0;
    }

    const char *cmd_name = argv[0];
    bool found = false;
    
    if (is_num(cmd_name))
    {
        int lnum = atoi(cmd_name);

        if (cmd_name[0] == '+' || cmd_name[0] == '-')
        {
            editorMoveCursorTo(E.active_win, 0, E.active_win->cy+E.active_win->viewport.rowoff+lnum);
        }
        else
        {
            editorMoveCursorTo(E.active_win, 0, lnum-1);
        }

        free(cmd_copy);
        return 0;
    }

    /* Resolve aliases first */
    for (size_t i = 0; i < ALIAS_NUM; i++)
    {
        if (strcmp(cmd_name, BUILTIN_ALIASES[i].name) == 0)
        {
            cmd_name = BUILTIN_ALIASES[i].command;
            break;
        }
    }

    for (size_t i = 0; i < CMD_NUM; i++)
    {
        if (strcmp(cmd_name, BUILTIN_COMMANDS[i].name) == 0)
        {
            found = true;

            const ShellCommand *cmd = &BUILTIN_COMMANDS[i];

            int handler_argc = argc - 1;

            if (handler_argc < cmd->min_args)
            {
                editorSetStatusMessage("Error: not enough arguments for '%s'.", cmd->name);
                break;
            }

            if (cmd->max_args != -1 && handler_argc > cmd->max_args)
            {
                editorSetStatusMessage("Error: too many arguments for '%s'.", cmd->name);
                break;
            }

            cmd->handler(fd, handler_argc, argv + 1); /* exec the actual command */
            break;
        }
    }

    if (!found)
    {
        editorSetStatusMessage("Error: Command not found: '%s'.", cmd_name);
    }

    free(cmd_copy);

    return (found ? 0 : -1);
}

void editorShell(int fd)
{
    char query[EDITOR_QUERY_LEN + 1] = {0};
    int qlen = 0;

    while (1)
    {
        editorSetStatusMessage(" :%s", query);
        editorRefreshScreen();

        int c = editorReadKey(fd);

        if (c == DEL_KEY || c == CTRL_H || c == BACKSPACE)
        {
            if (qlen != 0)
                query[--qlen] = '\0';
        }
        else if (c == ESC)
        {
            editorSetStatusMessage("");
            return;
        }
        else if (c == ENTER)
        {
            editorSetStatusMessage("");
            commandExec(fd, query, qlen);
            return;
        }
        else if (isprint(c))
        {
            if (qlen < EDITOR_QUERY_LEN)
            {
                query[qlen++] = c;
                query[qlen] = '\0';
            }
        }
    }
}