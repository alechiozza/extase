#include "commands.h"

#include "core.h"
#include "editor.h"
#include "term.h"
#include "window.h"
#include "ui.h"
#include "event.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

void editorQuit(TextBuffer *buf, int fd)
{
    if (buf->dirty == false)
    {
        editorCloseWindow();
        return;
    }

    char query[EDITOR_QUERY_LEN + 1] = {0};
    int qlen = 0;

    while (1)
    {
        editorSetStatusMessage(
            "There are unsaved changes, do you really want to quit? (y/N): %s",
            query);
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
            if (!strcmp(query, "y") || !strcmp(query, "Y") || !strcmp(query, "yes") ||
                !strcmp(query, "Yes") || !strcmp(query, "YES"))
            {
                editorCloseWindow();
            }

            editorSetStatusMessage("");
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

void command_handler_quit(int fd, int argc, char **argv)
{
    (void)argc;
    (void)argv;
    editorQuit(E.active_win->buf, fd);
}

void editorToggleLinenum(void)
{
    E.linenums = !E.linenums;

    updateWindowSize();
}

void command_handler_line(int fd, int argc, char **argv)
{
    (void)fd;
    (void)argc;
    (void)argv;
    editorToggleLinenum();
}

static bool confirmOpening(int fd)
{
    char query[EDITOR_QUERY_LEN + 1] = {0};
    int qlen = 0;

    while (1)
    {
        editorSetStatusMessage(
            "Unsaved changes found, do you really want to open another file? (y/N): %s",
            query);
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
            return false;
        }
        else if (c == ENTER)
        {
            if (!strcmp(query, "y") || !strcmp(query, "Y") || !strcmp(query, "yes") ||
                !strcmp(query, "Yes") || !strcmp(query, "YES"))
            {
                editorSetStatusMessage("");
                return true;
            }

            editorSetStatusMessage("");
            return false;
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

void editorOpenFromWin(Window *W, int fd)
{
    char query[EDITOR_QUERY_LEN + 1] = {0};
    int qlen = 0;

    while (1)
    {
        editorSetStatusMessage("Type the name of the file: %s", query);
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
            if (query[0] != '\0')
            {
                if (strcmp(query, W->buf->filename) == 0) return;

                if (W->buf->dirty)
                {
                    if (confirmOpening(fd) == false) return;
                }
                
                editorOpen(W, query);
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

