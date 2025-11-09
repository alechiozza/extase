#include "window.h"

#include "editor.h"
#include "event.h"
#include "commands.h"

#include <stdlib.h>
#include <string.h>

void computeWindowLayout(void)
{
    // TODO: for now just split horizontally
    for (size_t i = 0; i < E.num_win; i++)
    {
        Window *W = E.win[i];

        W->x = (E.screencols/E.num_win) * i;
        W->y = 1;
        W->viewport.top = 0;
        W->viewport.bottom = INFOBAR_SIZE;
        W->viewport.left = (W->linenums ? LINENUMBER_SIZE : 0);
        W->viewport.right = 0;
        W->width = E.screencols/E.num_win;
        W->height = E.screenrows-TOPBAR_SIZE-INFOBAR_SIZE;

        W->viewport.rows = W->height - W->viewport.top - W->viewport.bottom;
        W->viewport.cols = W->width - W->viewport.left - W->viewport.right;

        if (W->cy > W->viewport.rows)
            W->cy = W->viewport.rows - 1;
        if (W->cx > W->viewport.cols)
            W->cx = W->viewport.cols - 1;

    }
}

static void windowInit(Window *W)
{
    W->cx = 0;
    W->cy = 0;
    W->expected_cx = 0;

    W->viewport.top = 0;
    W->viewport.bottom = 0;
    W->viewport.left = 0;
    W->viewport.right = 0;
    W->viewport.scroll_margin = 5;
    W->viewport.horizontal_margin = 0;
    W->viewport.rowoff = 0;
    W->viewport.coloff = 0;

    W->buf = NULL;

    W->linenums = true;
}

void editorOpenWindow(const char *filename)
{
    if (E.num_win == EDITOR_MAX_WIN) return;

    Window *new_win = malloc(sizeof(Window));
    if (new_win == NULL)
    {
        // TODO: handle memory error
        editorFatalError("Fatal! Unable to create a new window\n");
        exit(1);
    }

    E.win[E.num_win] = new_win;
    E.active_win = E.num_win;
    windowInit(new_win);

    // TODO: hardcoded
    E.win[E.num_win]->buf = &E.buf;

    editorOpen(E.win[E.num_win], filename);

    E.num_win++;

    computeWindowLayout();
}

void editorCloseWindow(void)
{
    size_t closing_idx = E.active_win;
    free(E.win[closing_idx]);

    if (closing_idx < E.num_win - 1)
    {
        memmove(&E.win[closing_idx], &E.win[closing_idx + 1], 
            sizeof(Window *) * (E.num_win - 1 - closing_idx));
    }

    E.num_win--;

    if (E.num_win == 0)
        exit(0);

    if (E.active_win >= E.num_win)
    {
        E.active_win = E.num_win - 1;
    }

    computeWindowLayout();
}

void editorSwitchWindow(void)
{
    E.active_win = (E.active_win + 1) % E.num_win;
}
