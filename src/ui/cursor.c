#include "cursor.h"

#include "editor.h"
#include "window.h"
#include "core.h"

#define CURSOR_HORIZONTAL_GAP 5 /* TODO: implement*/

void editorMoveCursorLeft(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;

    if (filerow == 0 && W->cx == 0) return;

    if (W->cx == 0)
    {
        if (W->viewport.coloff)
        {
            W->viewport.coloff--;
        }
        else
        {
            editorMoveCursorUp(W);
            W->cx = W->buf->rows[filerow - 1].size;

            if (W->cx > W->viewport.cols - 1)
            {
                W->viewport.coloff = W->cx - W->viewport.cols + 1;
                W->cx = W->viewport.cols - 1;
            }
        }
    }
    else
    {
        W->cx -= 1;
    }

    W->expected_cx = W->cx;
}

void editorMoveCursorRight(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;
    int filecol = W->viewport.coloff + W->cx;

    TextBuffer *buf = W->buf;
    Row *row = (filerow >= buf->numrows) ? NULL : &buf->rows[filerow];

    if (row == NULL) return;    

    if (filecol < row->size)
    {
        if (W->cx == W->viewport.cols - 1)
        {
            W->viewport.coloff++;
        }
        else
        {
            W->cx += 1;
        }
    }
    else if (filecol == row->size)
    {
        W->cx = 0;
        W->viewport.coloff = 0;
        
        editorMoveCursorDown(W);
    }

    W->expected_cx = W->cx;
}

/* Fix cx if the current line has not enough chars...
   Useful for vertical motion */
static void fixCursorX(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;
    int filecol = W->viewport.coloff + W->expected_cx;
    Row *row = (filerow >= W->buf->numrows) ? NULL : &W->buf->rows[filerow];
    int rowlen = row ? row->size : 0;

    if (filecol > rowlen)
    {
        W->cx = rowlen -W->viewport.coloff;
    }
    else
    {
        W->cx = W->expected_cx - W->viewport.coloff;
    }
    
    if (W->cx < 0)
    {
        W->viewport.coloff += W->cx;
        W->cx = 0;
    }
}

void editorMoveCursorUp(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;

    if (filerow == 0) return;

    if (W->cy < W->viewport.scroll_margin)
    {
        if (W->viewport.rowoff)
            W->viewport.rowoff--;
        else if (W->cy > 0)
            W->cy--;
    }
    else
    {
        W->cy--;
    }

    fixCursorX(W);
}

void editorMoveCursorDown(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;

    if (filerow >= W->buf->numrows) return;

    if (W->cy >= W->viewport.rows - W->viewport.scroll_margin)
    {
        W->viewport.rowoff++;
    }
    else
    {
        W->cy++;
    }

    fixCursorX(W);
}

void editorMoveCursorPageUp(Window *W)
{
    int times = W->viewport.rows;
    while (times--)
    {
        editorMoveCursorUp(W);
    }
}

void editorMoveCursorPageDown(Window *W)
{
    int times = W->viewport.rows;
    while (times--)
    {
        editorMoveCursorDown(W);
    }
}

void editorCenterCursor(Window *W)
{
    if (W->cy > W->viewport.rows/2)
    {
        int diff = W->cy - W->viewport.rows/2;
        W->cy -= diff;
        W->viewport.rowoff += diff;
    }

    if (W->cx > W->viewport.cols)
    {
        int diff = W->cx - W->viewport.cols;
        W->cx -= diff;
        W->viewport.coloff += diff;
    }
}

void editorScrollUp(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;

    if (filerow == 0) return;

    W->viewport.rowoff--;

    fixCursorX(W);
}

void editorScrollDown(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;

    if (filerow >= W->buf->numrows) return;

    W->viewport.rowoff++;

    fixCursorX(W);
}

void editorMoveCursorLineEnd(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;

    TextBuffer *buf = W->buf;
    Row *row = (filerow >= buf->numrows) ? NULL : &buf->rows[filerow];

    if (row == NULL) return;    

    W->viewport.coloff = row->size - (W->viewport.cols-1);
    if (W->viewport.coloff < 0) W->viewport.coloff = 0;
    W->cx = row->size - W->viewport.coloff;

    W->expected_cx = W->cx;
}

void editorCursorReset(Window *W)
{
    W->cx = 0;
    W->cy = 0;
    W->expected_cx = 0;
    W->viewport.rowoff = 0;
    W->viewport.coloff = 0;
}
