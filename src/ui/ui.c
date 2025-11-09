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

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void drawTopBar(Framebuffer *fb)
{
    char topbar[80];
    int topbarlen = snprintf(topbar, sizeof(topbar),
        "   Extase %s", EDITOR_VERSION);
    fbDrawChars(fb, 0, 0, topbar, topbarlen, (Style){COLOR_BLACK, COLOR_BRIGHT_BLACK,0});

    fbEraseLineFrom(fb, 0, topbarlen, COLOR_BRIGHT_BLACK);
}

static void drawInfoBar(Framebuffer *fb, Window *W)
{
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), 
            "%.20s - %d lines %s", W->buf->filename, W->buf->numrows, W->buf->dirty ? "(modified)" : "");
    int rlen = snprintf(rstatus, sizeof(rstatus),
            "%d/%d ", W->viewport.rowoff + W->cy + 1, W->buf->numrows);
    if (len > W->width)
        len = W->width;
    
    fbWindowDrawChars(fb, W, 0, W->height-1, status, len, (Style){COLOR_BLACK, COLOR_BRIGHT_BLACK,0});

    while (len < W->width)
    {
        if (W->width - len == rlen)
        {
            fbWindowDrawChars(fb, W, len, W->height-1, rstatus, rlen, 
                (Style){COLOR_BLACK, COLOR_BRIGHT_BLACK,0});
            break;
        }
        else
        {
            fbWindowPutChar(fb, W, len, W->height-1, ' ',
                (Style){COLOR_BLACK, COLOR_BRIGHT_BLACK,0});
        }
        len++;
    }
}

static void drawStatusBar(Framebuffer *fb)
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

static void drawLineNumber(Framebuffer *fb, Window *W, int filerow, int n)
{
    char buf[24];
    int rnum = filerow+1 % 1000;
    int current_row = W->viewport.rowoff + W->cy;

    int blen;
    if (E.relativenums)
    {
        if (filerow == current_row)
            blen = snprintf(buf, sizeof(buf), "%3d  ", rnum);
        else
            blen = snprintf(buf, sizeof(buf), " %3d ", abs(current_row-filerow));
    }
    else
    {
        blen = snprintf(buf, sizeof(buf), " %3d ", rnum);
    }

    fbWindowDrawChars(fb, W, 0, n, buf, blen, (Style){COLOR_BRIGHT_BLACK, COLOR_DEFAULT_BG,0});
}

static void drawWelcomeScreen(Framebuffer *fb, Window *W)
{
    for (int y = 0; y < W->viewport.rows; y++)
    {
        int filerow = W->viewport.rowoff + y;

        if (E.linenums)
            drawLineNumber(fb, W, filerow, y);

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

            // TODO: maybe write a fbEraleLineTo(...)
            
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

static void drawTextBuffer(Framebuffer *fb, Window *W)
{
    for (int y = 0; y < W->viewport.rows; y++)
    {
        int filerow = W->viewport.rowoff + y;

        if (E.linenums)
            drawLineNumber(fb, W, filerow, y);

        if (filerow >= W->buf->numrows)
        {
            fbViewportPutChar(fb, W, 0, y, '~', STYLE_NORMAL);
            fbViewportEraseLineFrom(fb, W, y, 1, COLOR_DEFAULT_BG);
            continue;
        }

        Row *r = &W->buf->rows[filerow];

        int len = r->rsize - W->viewport.coloff;

        if (len > W->viewport.cols)
            len = W->viewport.cols;
        
        char *c = r->render + W->viewport.coloff;
        unsigned char *hl = r->hl + W->viewport.coloff;
        for (int x = 0; x < len; x++)
        {
            if (hl[x] == HL_NONPRINT)
            {
                fbViewportPutChar(fb, W, x, y, *(c+x), STYLE_INVERSE);
            }
            else
            {
                Style style = editorSyntaxToColor(hl[x]);
                
                if (E.active_win == W && y == W->cy && 
                    memcmp(&style.bg, &COLOR_DEFAULT_BG, sizeof(Color)) == 0)
                {
                    /* The cursor line is highlighted  only if the background is the default one */
                    fbViewportPutChar(fb, W, x, y, *(c+x),
                        (Style){style.fg, COLOR_LNE_HIGHLIGHT, style.attr});
                }
                else
                {
                    fbViewportPutChar(fb, W, x, y, *(c+x), style);
                }
            }
        }

        if (len < 0) len = 0; /* fuck me, it took an hour to figure this out */

        if (E.active_win == W && y == W->cy)
        {
            fbViewportEraseLineFrom(fb, W, y, len, COLOR_LNE_HIGHLIGHT);
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

static int computeCX(Window *W)
{
    /* Handle TABs */
    int cx = 1;
    int filerow = W->viewport.rowoff+W->cy;
    Row *row = (filerow >= W->buf->numrows) ? NULL : &W->buf->rows[filerow];
    if (row)
    {
        for (int j = W->viewport.coloff; j < (W->cx+W->viewport.coloff); j++)
        {
            if (j < row->size && row->chars[j] == TAB) 
                cx += TAB_SIZE - ((cx)%TAB_SIZE);
            
            cx++;
        }
    }

    cx += W->x + W->viewport.left;

    return cx;
}

static int computeCY(Window *W)
{
    int cy = 1 + W->y + W->viewport.top + W->cy;
    return cy;
}

void editorRefreshScreen(void)
{
    Framebuffer *fb = fbCreate(E.screenrows, E.screencols);
    if (fb == NULL)
    {
        editorFatalError("Not enough memory to allocate the framebuffer!\n");
        exit(EXIT_FAILURE);
    }

    drawTopBar(fb);

    // TODO: for every window

    for (size_t i = 0; i < E.num_win; i++)
    {
        if (E.win[i]->buf->numrows == 0)
            drawWelcomeScreen(fb, E.win[i]);
        else
            drawTextBuffer(fb, E.win[i]);

        drawInfoBar(fb, E.win[i]);
    }

    drawStatusBar(fb);

    AppendBuffer ab = ABUF_INIT;

    abAppendString(&ab, ESC_HIDE_CURSOR);
    {
        fbRender(fb, &ab);

        /* Set cursor position*/
        char buf[32];
        int cx = computeCX(E.active_win);
        int cy = computeCY(E.active_win);
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
    fbFree(fb);
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
