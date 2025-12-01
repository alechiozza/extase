#include "ui.h"

#include "editor.h"
#include "syntax.h"
#include "term.h"
#include "fb.h"
#include "window.h"
#include "core.h"
#include "event.h"
#include "color.h"
#include "utils.h"
#include "widget.h"
#include "textbuffer.h"
#include "utf8.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static void drawTopBar(FrameBuffer *fb)
{
    char topbar[80];
    int topbarlen = snprintf(topbar, sizeof(topbar),
        "   Extase %s", EDITOR_VERSION);
    fbDrawChars(fb, 0, 0, topbar, topbarlen, (Style){COLOR_WHITE, COLOR_UI_BLACK,0});

    fbEraseLineFrom(fb, 0, topbarlen, COLOR_UI_BLACK);
}

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

static void drawStatusBar(FrameBuffer *fb)
{
    int msglen = strlen(E.statusmsg);
    if (msglen)
    {
        fbDrawChars(fb, 0, E.screenrows-1, 
            E.statusmsg, msglen <= E.screencols ? msglen : E.screencols,
            STYLE_NORMAL);
    }

    fbEraseLineFrom(fb, E.screenrows-1, msglen, COLOR_BLACK);
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

static int computeCX()
{
    /* Handle TABs */
    int cx = 1;

    if (E.active_widget != NULL)
    {
        cx += E.active_widget->x + E.active_widget->cx;
        return cx;
    }

    Window *W = E.active_win;

    int filerow = W->viewport.rowoff+W->cy;
    Row *row = (filerow >= W->buf->numrows) ? NULL : &W->buf->rows[filerow];
    if (row)
    {
        for (int j = W->viewport.coloff; j < (W->cx+W->viewport.coloff); j++)
        {
            if (j < row->size && row->chars[j] == TAB) 
                cx += W->buf->indent_size - ((cx)%W->buf->indent_size);
            
            cx++;
        }
    }

    cx += W->x + W->viewport.left;

    return cx;
}

static int computeCY()
{
    int cy = 1;
    
    if (E.active_widget != NULL)
    {
        cy += E.active_widget->y + E.active_widget->cy;
        return cy;
    }

    Window *W = E.active_win;
    cy += W->y + W->viewport.top + W->cy;
    return cy;
}

void editorRefreshScreen(void)
{
    if (!E.no_topbar)
        drawTopBar(E.fb);

    // TODO: for every window

    for (size_t i = 0; i < E.num_win; i++)
    {
        drawTabsView(E.fb, E.win[i]);

        if (E.win[i]->buf->numrows == 0)
            drawWelcomeScreen(E.fb, E.win[i]);
        else
            drawTextBuffer(E.fb, E.win[i]);

        drawInfoBar(E.fb, E.win[i]);
        drawScrollBar(E.fb, E.win[i]);
    }

    for (size_t i = 0; i < E.num_widget; i++)
    {
        E.widgets[i]->draw(E.widgets[i], E.fb);
    }

    drawStatusBar(E.fb);

    AppendBuffer ab = ABUF_INIT;

    abAppendString(&ab, ESC_HIDE_CURSOR);
    {
        fbRender(E.fb, &ab);

        /* Set cursor position*/
        char buf[32];
        int cx = computeCX();
        int cy = computeCY();
        snprintf(buf, sizeof(buf), "\x1b[%d;%dH", cy, cx);
        abAppendString(&ab, buf);
    }
    abAppendString(&ab, ESC_SHOW_CURSOR);

    if (writen(STDOUT_FILENO, ab.b, ab.len) == -1)
    {
        editorFatalError("Fatal write error during framebuffer render\n");
        exit(EXIT_FAILURE);
    }
    
    abFree(&ab);
}

void editorTooSmallScreen(void)
{
    AppendBuffer ab = ABUF_INIT;

    abAppendString(&ab, ESC_HIDE_CURSOR);
    abAppendString(&ab, ESC_CURSOR_HOME);
    abAppendString(&ab, ESC_ERASE_SCREEN);

    char msg[] = "< Terminal too small. Resize to continue. >";
    int msglen = strlen(msg);
    if (msglen > E.screencols) msglen = E.screencols;

    abAppend(&ab, msg, msglen);
    abAppendString(&ab, ESC_SHOW_CURSOR);

    writen(STDOUT_FILENO, ab.b, ab.len);
    
    abFree(&ab);
}
