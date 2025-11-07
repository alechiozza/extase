#include "window.h"
#include "editor.h"

#include <stdlib.h>

void computeWindowLayout(void)
{
    // TODO: implement
    // for (int i = 0; i < E.num_win; i++)
    // {
    //     Window *W = E.win[i];
    //     W->viewport.top = 0;
    //     W->viewport.bottom = 2;
    //     W->viewport.left = E.screencols/E.num_win * i;
    //     W->viewport.right = E.screencols/E.num_win * (i+1);
    // }

    // TODO: Hardcoded for now

    E.win[0]->x = 0;
    E.win[0]->y = 1;
    E.win[0]->viewport.top = 0;
    E.win[0]->viewport.bottom = 1;
    E.win[0]->viewport.left = (E.win[0]->linenums ? LINENUMBER_SIZE : 0);
    E.win[0]->viewport.right = 0;
    E.win[0]->width = E.screencols;
    E.win[0]->height = E.screenrows-TOPBAR_SIZE-1;

    E.win[0]->viewport.rows = E.win[0]->height - E.win[0]->viewport.top - E.win[0]->viewport.bottom;
    E.win[0]->viewport.cols = E.win[0]->width - E.win[0]->viewport.left - E.win[0]->viewport.right;

    if (E.win[0]->cy > E.win[0]->viewport.rows)
        E.win[0]->cy = E.win[0]->viewport.rows - 1;
    if (E.win[0]->cx > E.win[0]->viewport.cols)
        E.win[0]->cx = E.win[0]->viewport.cols - 1;
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

    // TODO: HARDCODED
    W->buf = &E.buf;

    W->buf->numrows = 0;
    W->buf->rows = NULL;
    W->buf->dirty = false;
    W->buf->filename = NULL;
    W->buf->syntax = NULL;

    W->linenums = true;
}

void editorOpenWindow(void)
{
    if (E.num_win == EDITOR_MAX_WIN || E.num_win < 0) return;

    Window *new_win = malloc(sizeof(Window));
    if (new_win == NULL)
    {
        // TODO: handle memory error
        return;
    }

    E.win[E.num_win] = new_win;
    E.active_win = new_win;

    windowInit(new_win);

    E.num_win++;
}

void editorCloseWindow(Window *W)
{
    free(W);
    E.num_win--;
}
