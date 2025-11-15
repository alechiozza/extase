#define _XOPEN_SOURCE

#include "fb.h"

#include "term.h"
#include "editor.h"
#include "window.h"
#include "event.h"
#include "utf8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <wchar.h>
#include <locale.h>
#include <stdint.h>

FrameBuffer *fbCreate(int rows, int cols)
{
    FrameBuffer *fb = malloc(sizeof(FrameBuffer));
    if (fb == NULL)
    {
        editorFatalError("Not enough memory to allocate the framebuffer!\n");
        exit(EXIT_FAILURE);
    }

    fb->rows = rows;
    fb->cols = cols;
    fb->grid = calloc(rows * cols, sizeof(Cell));
    if (fb->grid == NULL)
    {
        editorFatalError("Not enough memory to allocate the framebuffer!\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < rows * cols; i++) 
    {
        fb->grid[i].c = ' ';
        fb->grid[i].width = 1;
        fb->grid[i].style.fg = COLOR_MAGENTA; /* magenta just for debugging purposes */
        fb->grid[i].style.bg = COLOR_MAGENTA;
    }

    return fb;
}

void fbResize(FrameBuffer *fb, int rows, int cols)
{
    if (fb == NULL) return;

    fb->rows = rows;
    fb->cols = cols;
    fb->grid = realloc(fb->grid, rows * cols * sizeof(Cell));
    if (fb->grid == NULL)
    {
        editorFatalError("Not enough memory to realloc the framebuffer!\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < rows * cols; i++) 
    {
        fb->grid[i].c = ' ';
        fb->grid[i].width = 1;
        fb->grid[i].style.fg = COLOR_MAGENTA; /* magenta just for debugging purposes */
        fb->grid[i].style.bg = COLOR_MAGENTA;
    }
}

void fbFree(FrameBuffer *fb)
{
    free(fb->grid);
    free(fb);
}

void fbPutCodepoint(FrameBuffer *fb, int x, int y, uint32_t c, Style style)
{
    if (x < 0 || y < 0 || y >= fb->rows || x >= fb->cols) return;

    int width = wcwidth(c);
    
    if (width <= 0)
    {
        c = '?';
        width = 1;
    }

    if (width > 1 && x + width > fb->cols)
    {
        return; /* we clip it, we can't draw it */
    }

    // Place the main character
    fb->grid[y*fb->cols + x].c = c;
    fb->grid[y*fb->cols + x].style = style;
    fb->grid[y*fb->cols + x].width = width;

    // If it was a wide character, place a "dummy" cell in the next slot
    if (width == 2)
    {
        fb->grid[y*fb->cols + x + 1].c = 0;
        fb->grid[y*fb->cols + x + 1].style = style;
        fb->grid[y*fb->cols + x + 1].width = 0;
    }
}

void fbPutChar(FrameBuffer *fb, int x, int y, char c, Style style)
{
    fbPutCodepoint(fb, x, y, (uint32_t)c, style);
}

void fbDrawChars(FrameBuffer *fb, int x, int y, const char *s, int len, Style style)
{
    for (int i = 0; i < len && x + i < fb->cols; i++)
        fbPutChar(fb, x + i, y, s[i], style);
}

void fbDrawString(FrameBuffer *fb, int x, int y, const char *s, Style style)
{
    for (int i = 0; s[i] && x + i < fb->cols; i++)
        fbPutChar(fb, x + i, y, s[i], style);
}

void fbEraseLine(FrameBuffer *fb, int y, Color color)
{
    for (int i = 0; i < fb->cols; i++)
        fbPutChar(fb, i, y, ' ', (Style){COLOR_DEFAULT_FG, color, 0});
}

void fbEraseLineFrom(FrameBuffer *fb, int y, int x, Color color)
{
    for (int i = x; i < fb->cols; i++)
        fbPutChar(fb, i, y, ' ', (Style){COLOR_DEFAULT_FG, color, 0});
}

void fbWindowPutChar(FrameBuffer *fb, Window *W, int x, int y, char c, Style style)
{
    if (x < 0 || y < 0 || y >= W->height || x >= W->width) return;

    x = W->x + x;
    y = W->y + y;
    
    fb->grid[y*fb->cols + x].c = c;
    fb->grid[y*fb->cols + x].style = style;
}

void fbWindowDrawChars(FrameBuffer *fb, Window *W, int x, int y, const char *s, int len, Style style)
{
    for (int i = 0; i < len && x + i < fb->cols; i++)
        fbWindowPutChar(fb, W, x + i, y, s[i], style);
}

void fbWindowDrawString(FrameBuffer *fb, Window *W, int x, int y, const char *s, Style style)
{
    for (int i = 0; s[i] && x + i < fb->cols; i++)
        fbWindowPutChar(fb, W, x + i, y, s[i], style);
}

void fbWindowEraseLine(FrameBuffer *fb, Window *W, int y, Color color)
{
    for (int i = 0; i < W->width; i++)
        fbWindowPutChar(fb, W, i, y, ' ', (Style){COLOR_DEFAULT_FG, color, 0});
}

void fbWindowEraseLineFrom(FrameBuffer *fb, Window *W, int y, int x, Color color)
{
    for (int i = x; i < W->width; i++)
        fbWindowPutChar(fb, W, i, y, ' ', (Style){COLOR_DEFAULT_FG, color, 0});
}

void fbViewportPutChar(FrameBuffer *fb, Window *W, int x, int y, char c, Style style)
{
    if (x < 0 || y < 0 || y >= W->viewport.rows || x >= W->viewport.cols) return;

    x = W->x + W->viewport.left + x;
    y = W->y + W->viewport.top + y;
    
    fb->grid[y*fb->cols + x].c = c;
    fb->grid[y*fb->cols + x].style = style;
}

void fbViewportDrawChars(FrameBuffer *fb, Window *W, int x, int y, const char *s, int len, Style style)
{
    for (int i = 0; i < len && x + i < fb->cols; i++)
        fbViewportPutChar(fb, W, x + i, y, s[i], style);
}

void fbViewportDrawString(FrameBuffer *fb, Window *W, int x, int y, const char *s, Style style)
{
    for (int i = 0; s[i] && x + i < fb->cols; i++)
        fbViewportPutChar(fb, W, x + i, y, s[i], style);
}

void fbViewportEraseLine(FrameBuffer *fb, Window *W, int y, Color color)
{
    for (int i = 0; i < W->viewport.cols; i++)
        fbViewportPutChar(fb, W, i, y, ' ', (Style){COLOR_DEFAULT_FG, color, 0});
}

void fbViewportEraseLineFrom(FrameBuffer *fb, Window *W, int y, int x, Color color)
{
    for (int i = x; i < W->viewport.cols; i++)
        fbViewportPutChar(fb, W, i, y, ' ', (Style){COLOR_DEFAULT_FG, color, 0});
}

void abAppend(AppendBuffer *ab, const char *s, int len)
{
    if (ab == NULL || s == NULL || len == 0)
        return;

    // Check if more space is needed
    if (ab->len + len >= ab->cap)
    {
        size_t new_cap = ab->cap ? ab->cap * 2 : 64;
        while (new_cap < ab->len + len)
            new_cap *= 2;

        char *new_buf = realloc(ab->b, new_cap);
        if (new_buf == NULL)
            return;

        ab->b = new_buf;
        ab->cap = new_cap;
    }

    memcpy(ab->b + ab->len, s, len);
    ab->len += len;
}

void abAppendString(AppendBuffer *ab, const char *s)
{
    abAppend(ab, s, strlen(s));
}

void abFree(AppendBuffer *ab)
{
    free(ab->b);
}

void fbRender(FrameBuffer *fb, AppendBuffer *ab)
{
    abAppendString(ab, ESC_CURSOR_HOME);

    Style last_style;
    bool first_cell = true;

    int i = 0;
    while (i < fb->rows * fb->cols)
    {
        Cell *cell = &fb->grid[i];

        /* Skip dummy cells */
        if (cell->width == 0)
        {
            i++;
            continue;
        }

        Style *s = &cell->style;

        if (first_cell || s->bg != last_style.bg || s->fg != last_style.fg || 
            s->attr != last_style.attr)
        {
            char seq[64];
            int len = 0;

            len += snprintf(seq + len, sizeof(seq) - len, "\x1b["ESC_RESET_MODES);

            if (s->attr & ATTR_BOLD)        len += snprintf(seq + len, sizeof(seq) - len, ";"ESC_BOLD);
            if (s->attr & ATTR_DIM)         len += snprintf(seq + len, sizeof(seq) - len, ";"ESC_DIM);
            if (s->attr & ATTR_ITALIC)      len += snprintf(seq + len, sizeof(seq) - len, ";"ESC_ITALIC);
            if (s->attr & ATTR_UNDERLINE)   len += snprintf(seq + len, sizeof(seq) - len, ";"ESC_UNDERLINE);
            if (s->attr & ATTR_BLINK)       len += snprintf(seq + len, sizeof(seq) - len, ";"ESC_BLINKING);
            if (s->attr & ATTR_INVERSE)     len += snprintf(seq + len, sizeof(seq) - len, ";"ESC_INVERSE);

            len += printFgColor(seq+len, sizeof(seq)-len, E.color_mode, s->fg);
            len += printBgColor(seq+len, sizeof(seq)-len, E.color_mode, s->bg);
            len += snprintf(seq + len, sizeof(seq) - len, "m");

            abAppendString(ab, seq);

            first_cell = false;
            last_style = *s;
        }

        char utf8_buffer[5]; /* (Max 4 bytes + null) */
        int bytes_written = codepoint_to_utf8(cell->c, utf8_buffer);
        abAppend(ab, utf8_buffer, bytes_written);

        i += cell->width;
    }
}
