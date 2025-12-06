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

static void drawTopBar(FrameBuffer *fb)
{
    char topbar[80];
    int topbarlen = snprintf(topbar, sizeof(topbar),
        "   Extase %s", EDITOR_VERSION);
    fbDrawChars(fb, 0, 0, topbar, topbarlen, (Style){COLOR_WHITE, COLOR_UI_BLACK,0});

    fbEraseLineFrom(fb, 0, topbarlen, COLOR_UI_BLACK);
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
        drawWindow(E.fb, E.win[i]);
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
