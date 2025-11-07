#include "commands.h"
#include "editor.h"
#include "event.h"
#include "term.h"
#include "ui.h"
#include "window.h"

#include <unistd.h>
#include <string.h>
#include <ctype.h>

// TODO: shell should also able to interpret commands as config file command,
// the exec command will call to the interpreter

static int commandExec(const char *command, int len)
{
    if (len == 0) return 0;
    if (len == 1)
    {
        switch (command[0])
        {
        case 'q':
            editorQuit(E.active_win->buf, STDIN_FILENO);
            return 0;
            break;
        case 'f':
            editorFind(E.active_win, STDIN_FILENO);
            return 0;
        }
        return -1;
    }

    if (strcmp(command, "quit") == 0)
    {
        editorQuit(E.active_win->buf, STDIN_FILENO);
        return 0;
    }
    if (strcmp(command, "line") == 0)
    {
        editorToggleLinenum();
        return 0;
    }
    if (strcmp(command, "find") == 0)
    {
        editorFind(E.active_win, STDIN_FILENO);
        return 0;
    }

    return -1;
}

void editorShell()
{
    char query[EDITOR_QUERY_LEN + 1] = {0};
    int qlen = 0;

    while (1)
    {
        editorSetStatusMessage(" :%s", query);
        editorRefreshScreen();

        int c = editorReadKey(STDIN_FILENO);

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
            if (commandExec(query, qlen) == -1)
            {
                editorSetStatusMessage("Unknown command \"%s\"", query);
            }
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