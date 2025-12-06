#include "window.h"

#include "editor.h"
#include "event.h"
#include "commands.h"
#include "ui.h"
#include "textbuffer.h"
#include "modes.h"
#include "fb.h"
#include "core.h"
#include "syntax.h"
#include "utf8.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>

static void windowInit(Window *W)
{
    W->cx = 0;
    W->cy = 0;
    W->expected_cx = 0;
    W->buf = NULL;
    W->node = NULL;

    W->viewport.top = 0;
    W->viewport.bottom = 0;
    W->viewport.left = 0;
    W->viewport.right = 0;
    W->viewport.scroll_margin = 5;
    W->viewport.horizontal_margin = 0;
    W->viewport.rowoff = 0;
    W->viewport.coloff = 0;
}

Window *createWindow(void)
{
    if (E.num_win == EDITOR_MAX_WIN)
        return NULL;

    Window *new_win = malloc(sizeof(Window));
    if (new_win == NULL)
    {
        // TODO: handle memory error
        editorFatalError("Fatal! Unable to create a new window\n");
        exit(1);
    }

    E.win[E.num_win] = new_win;
    E.active_win = new_win;
    windowInit(new_win);

    E.num_win++;

    return new_win;
}

void windowProcessKeypress(int key)
{
    switch (E.mode)
    {
        case NORMAL_MODE:
            editorNMProcessKeypress(key);
            break;
        case INSERT_MODE:
            editorIMProcessKeypress(key);
            break;
    }
}

void deleteWindowBuf(Window *W)
{
    bool found_buf = false;
    for (size_t i = 0; i < E.num_win; i++)
    {
        if (E.win[i] != W && W->buf == E.win[i]->buf)
        {
            found_buf = true;
            break;
        }
    }

    if (!found_buf)
    {
        deleteBuffer(W->buf);
    }
}

static void destroyWindow(Window *W)
{
    if (W == NULL)
        return;

    int found_idx = -1;
    for (size_t i = 0; i < E.num_win; i++)
    {
        if (E.win[i] == W)
        {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1)
    {
        /* Should be unreachable */
        editorFatalError("Fatal! Tried to destroy a window that isn't in the global list.\n");
        exit(EXIT_FAILURE);
    }

    deleteWindowBuf(W);

    free(E.win[found_idx]);

    int remaining_elements = E.num_win - 1 - found_idx;

    if (remaining_elements > 0)
    {
        memmove(&E.win[found_idx], &E.win[found_idx + 1], sizeof(Window *) * remaining_elements);
    }

    E.num_win--;

    E.win[E.num_win] = NULL; /* just to be sure */
}

void closeWindow(void)
{
    if (E.num_win <= 1)
        exit(0);

    Window *winToClose = E.active_win;
    LayoutNode *leafToClose = winToClose->node;

    if (!leafToClose || leafToClose->type != LAYOUT_LEAF)
    {
        // editorWarningError("Fatal! a window has invalid layout node.\n");
        return;
    }

    LayoutNode *splitNode = leafToClose->parent;
    if (!splitNode)
    {
        // editorWarningError("Fatal! Attempting to close root leaf when other windows exist.\n");
        return;
    }

    LayoutNode *siblingNode = (splitNode->child1 == leafToClose) ? splitNode->child2 : splitNode->child1;
    LayoutNode *grandParent = splitNode->parent;

    if (grandParent == NULL)
    {
        setLayoutRoot(siblingNode);
        siblingNode->parent = NULL;
    }
    else
    {
        if (grandParent->child1 == splitNode)
        {
            grandParent->child1 = siblingNode;
        }
        else
        {
            grandParent->child2 = siblingNode;
        }
        siblingNode->parent = grandParent;
    }

    LayoutNode *newActiveNode = siblingNode;
    while (newActiveNode && newActiveNode->type != LAYOUT_LEAF)
    {
        newActiveNode = newActiveNode->child1;
    }

    if (newActiveNode && newActiveNode->window)
    {
        E.active_win = newActiveNode->window;
    }
    else
    {
        E.active_win = E.win[0];
    }

    destroyWindow(winToClose);

    free(leafToClose);
    free(splitNode);

    computeWindowLayout();
}

void switchWindow(Direction dir)
{
    /* NOTE: Originally I implemented this walking up and 
    down the tree to find the correct window to switch to... 
    But the code turned out to be much more long and complex 
    Theorically this geometrical approach here is slower (O(N))
    but the tradeoff is minimal and it's way clener. */

    if (E.num_win <= 1 || !E.active_win)
    {
        return;
    }

    Window *a = E.active_win;
    Window *nearest = NULL;

    for (size_t i = 0; i < E.num_win && !nearest; i++)
    {
        Window *c = E.win[i];

        if (c == a) continue;

        switch (dir)
        {
        case DIR_RIGHT:
            if (a->y >= c->y && 
                a->y < c->y + c->height &&
                c->x - (a->x + a->width) == 0)
            {
                nearest = c;
            }
            break;
        case DIR_LEFT:
            if (a->y >= c->y && 
                a->y < c->y + c->height &&
                a->x - (c->x + c->width) == 0)
            {
                nearest = c;
            }
            break;
        case DIR_DOWN:
            if (a->x >= c->x && 
                a->x < c->x + c->width &&
                c->y - (a->y + a->height) == 0)
            {
                nearest = c;
            }
            break;
        case DIR_UP:
            if (a->x >= c->x && 
                a->x < c->x + c->width &&
                a->y - (c->y + c->height) == 0)
            {
                nearest = c;
            }
            break;
        }
    }

    if (nearest != NULL)
    {
        E.active_win = nearest;
    }
}

#define MIN_RATIO 0.2
#define MAX_RATIO 0.8

void resizeWindow(Window *W, float amount, bool direct)
{
    if (W->node->parent == NULL) return;

    LayoutNode *parent = W->node->parent;
    if (parent->type != LAYOUT_SPLIT_HORIZONTAL && 
        parent->type != LAYOUT_SPLIT_VERTICAL)
        return;

    if (!direct && W->node == parent->child2) amount *= -1;
    parent->ratio += amount;

    if (parent->ratio < MIN_RATIO) parent->ratio = MIN_RATIO;
    else if (parent->ratio > MAX_RATIO) parent->ratio = MAX_RATIO;

    computeNodeLayout(parent);
}

// ================== Window Drawing logic =========================== //

static void drawInfoBar(FrameBuffer *fb, Window *W)
{
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), 
            "%.20s - %d lines %s", W->buf->filename, W->buf->numrows, W->buf->dirty ? "(modified)" : "");
    int rlen = snprintf(rstatus, sizeof(rstatus),
            "%d/%d ", W->viewport.rowoff + W->cy + 1, W->buf->numrows);
    if (len > W->width)
        len = W->width;

    Color bg = (E.active_win == W) ? COLOR_UI_BLACK : COLOR_UI_DARK_BLACK;
    
    fbWindowDrawChars(fb, W, 0, W->height-1, status, len, (Style){COLOR_WHITE, bg,0});

    while (len < W->width)
    {
        if (W->width - len == rlen)
        {
            fbWindowDrawChars(fb, W, len, W->height-1, rstatus, rlen, 
                (Style){COLOR_WHITE, bg,0});
            break;
        }
        else
        {
            fbWindowPutChar(fb, W, len, W->height-1, ' ',
                (Style){COLOR_WHITE, bg,0});
        }
        len++;
    }
}

static void drawScrollBar(FrameBuffer *fb, Window *W)
{
    for (int i = 0; i < W->height-1; i++)
    {
        fbWindowPutChar(fb, W, W->width-1, i, ' ', (Style){COLOR_DEFAULT_FG, COLOR_UI_DARK_BLACK, 0});
    }

    float hratio = (float)W->viewport.rows/W->buf->numrows;
    if (hratio > 1) return;
    int h = (W->height-1)*hratio;

    float yratio = (float)W->viewport.rowoff/W->buf->numrows;
    if (yratio > 1) yratio = 1;

    int y = (W->height-1)*yratio;

    if (h+y > W->height) y = W->height-1 - h;

    for (int i = y; i < y+h; i++)
    {
        fbWindowPutChar(fb, W, W->width-1, i, ' ', (Style){COLOR_DEFAULT_FG, COLOR_BRIGHT_BLACK, 0});
    }
}

static void drawTabsView(FrameBuffer *fb, Window *W)
{
    fbWindowEraseLine(fb, W, 0, COLOR_DARK_BLACK);

    int len = 0;
    Style style;
    Style bar_style;

    for (int i = 0; i < 1; i++)
    {
        style = (i == 0) ? STYLE_NORMAL : (Style){COLOR_BRIGHT_BLACK, COLOR_DARK_BLACK, 0};
        bar_style = (i == 0) ?  (Style){COLOR_BRIGHT_BLACK, COLOR_DEFAULT_BG, 0} : 
                                (Style){(Color){RGB_COLOR(5,5,5)}, COLOR_DARK_BLACK, 0};

        fbWindowPutCodepoint(fb, W, len, 0, UNICODE_LEFT_BAR, bar_style);
        len++;
        fbWindowDrawString(fb, W, len, 0, "   ", style);
        len+=3;
        fbWindowDrawString(fb, W, len, 0, W->buf->filename, style);
        len += strlen(W->buf->filename);
        fbWindowDrawString(fb, W, len, 0, "    ", style);
        len+=4;

        if (len >= W->width)
            break;
    }
}

int getLineNumberWidth(Window *W)
{
    if (W->buf->numrows == 0) return MIN_LNUM_WIDTH;
    
    int width = (int)log10(W->buf->numrows) + 1 + 1; // +1 for final space

    return (width > MIN_LNUM_WIDTH) ? width : MIN_LNUM_WIDTH;
}

static void drawLineNumber(FrameBuffer *fb, Window *W, int y, int width)
{
    char buf[24];
    int blen;
    int filerow = W->viewport.rowoff + y;
    int current_row = W->viewport.rowoff + W->cy;
    y += W->viewport.top;

    if (filerow >= W->buf->numrows)
    {
        for (int i = 0; i < width; i++) 
            fbWindowPutChar(fb, W, i, y, ' ', (Style){COLOR_BRIGHT_BLACK, COLOR_BLACK,0});
        return;
    }

    if (E.relativenums)
    {
        if (filerow == current_row)
            blen = snprintf(buf, sizeof(buf), "%*d ", width-1, filerow+1);
        else
            blen = snprintf(buf, sizeof(buf), "%*d ", width-1, abs(current_row-filerow));
    }
    else
    {
        blen = snprintf(buf, sizeof(buf), "%*d ", width-1, filerow+1);
    }

    if (filerow == current_row)
        fbWindowDrawChars(fb, W, 0, y, buf, blen, (Style){COLOR_WHITE, COLOR_UI_DARK_BLACK,0});
    else
        fbWindowDrawChars(fb, W, 0, y, buf, blen, (Style){COLOR_BRIGHT_BLACK, COLOR_BLACK,0});
}

static void drawWelcomeScreen(FrameBuffer *fb, Window *W)
{
    for (int y = 0; y < W->viewport.rows; y++)
    {
        if (E.linenums)
            drawLineNumber(fb, W, y, MIN_LNUM_WIDTH);

        if (y == W->viewport.rows / 3)
        {
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome),
                "Extase, version %s", EDITOR_VERSION);
            int padding = (W->viewport.cols - welcomelen) / 2;

            if (padding != 0)
            {
                fbViewportPutChar(fb, W, 0, y, '~', STYLE_NORMAL);
            }

            for (int i = 1; i < padding; i++)
                fbViewportPutChar(fb, W, i, y, ' ', STYLE_NORMAL);
            
            fbViewportDrawChars(fb, W, padding, y, welcome, welcomelen, STYLE_NORMAL);

            fbViewportEraseLineFrom(fb, W, y, padding+welcomelen, COLOR_DEFAULT_BG);
        }
        else
        {
            fbViewportPutChar(fb, W, 0, y, '~', STYLE_NORMAL);
            fbViewportEraseLineFrom(fb, W, y, 1, COLOR_DEFAULT_BG);
        }
    }
}

static void drawTextBuffer(FrameBuffer *fb, Window *W)
{
    int lnum_width = getLineNumberWidth(W);
    W->viewport.left = (E.linenums ? lnum_width : 0); // TODO: hardcoded

    for (int y = 0; y < W->viewport.rows; y++)
    {
        int filerow = W->viewport.rowoff + y;

        if (E.linenums)
            drawLineNumber(fb, W, y, lnum_width);

        if (filerow >= W->buf->numrows)
        {
            fbViewportPutChar(fb, W, 0, y, '~', STYLE_NORMAL);
            fbViewportEraseLineFrom(fb, W, y, 1, COLOR_DEFAULT_BG);
            continue;
        }

        Row *r = &W->buf->rows[filerow];

        int len = r->render.size - W->viewport.coloff;

        if (len > W->viewport.cols)
            len = W->viewport.cols;
        
        char *c = r->render.c + W->viewport.coloff;
        unsigned char *hl = r->render.hl + W->viewport.coloff;
        for (int x = 0; x < len; x++)
        {
            Style style = editorSyntaxToColor(hl[x]);
                
            if (E.active_win == W && y == W->cy && style.bg == COLOR_DEFAULT_BG)
            {
                style.bg = COLOR_LINE_HIGHLIGHT;
            }
            
            fbViewportPutChar(fb, W, x, y, *(c+x), style);
        }

        if (len < 0) len = 0; /* fuck me, it took an hour to figure this out */

        if (E.active_win == W && y == W->cy)
        {
            fbViewportEraseLineFrom(fb, W, y, len, COLOR_LINE_HIGHLIGHT);
        }
        else
        {
            fbViewportEraseLineFrom(fb, W, y, len, COLOR_DEFAULT_BG);
        }
    }

    // for (int y = 0; y < w->height; y++)
    // {
    //     int fbY = w->y + y;
    //     if (fbY < 0 || fbY >= fb->rows) continue;

    //     for (int x = 0; x < w->width; x++)
    //     {
    //         int fbX = w->x + x;
    //         if (fbX < 0 || fbX >= fb->cols) continue;

    //         // get char from window buffer (or space)
    //         char c = windowCharAt(w, x, y);
    //         fbPutChar(fb, fbX, fbY, c, w->fg, w->bg);
    //     }
    // }
}

void drawWindow(FrameBuffer *fb, Window *W)
{
    drawTabsView(fb, W);

    if (W->buf->numrows == 0)
        drawWelcomeScreen(fb, W);
    else
        drawTextBuffer(fb, W);

    drawInfoBar(fb, W);
    drawScrollBar(fb, W);
}
