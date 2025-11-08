#include "fb.h"

#include "term.h"
#include "editor.h"
#include "window.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

Framebuffer *fbCreate(int rows, int cols)
{
    Framebuffer *fb = malloc(sizeof(Framebuffer));
    fb->rows = rows;
    fb->cols = cols;
    fb->grid = calloc(rows * cols, sizeof(Cell));
    if (fb->grid == NULL)
    {
        // TODO: handle memory error
        return NULL;
    }

    for (int i = 0; i < rows * cols; i++) 
    {
        fb->grid[i].c = ' ';
        fb->grid[i].style.fg = COLOR_MAGENTA; /* magenta just for debugging purposes */
        fb->grid[i].style.bg = COLOR_MAGENTA;
    }

    return fb;
}

void fbFree(Framebuffer *fb)
{
    free(fb->grid);
    free(fb);
}

void fbPutChar(Framebuffer *fb, int x, int y, char c, Style style)
{
    if (x < 0 || y < 0 || y >= fb->rows || x >= fb->cols) return;

    fb->grid[y*fb->cols + x].c = c;
    fb->grid[y*fb->cols + x].style = style;
}

void fbDrawChars(Framebuffer *fb, int x, int y, const char *s, int len, Style style)
{
    for (int i = 0; i < len && x + i < fb->cols; i++)
        fbPutChar(fb, x + i, y, s[i], style);
}

void fbDrawString(Framebuffer *fb, int x, int y, const char *s, Style style)
{
    for (int i = 0; s[i] && x + i < fb->cols; i++)
        fbPutChar(fb, x + i, y, s[i], style);
}

void fbEraseLine(Framebuffer *fb, int y, Color color)
{
    for (int i = 0; i < fb->cols; i++)
        fbPutChar(fb, i, y, ' ', (Style){color, color, 0});
}

void fbEraseLineFrom(Framebuffer *fb, int y, int x, Color color)
{
    for (int i = x; i < fb->cols; i++)
        fbPutChar(fb, i, y, ' ', (Style){color, color, 0});
}

void fbWindowPutChar(Framebuffer *fb, Window *W, int x, int y, char c, Style style)
{
    if (x < 0 || y < 0 || y >= W->height || x >= W->width) return;

    x = W->x + x;
    y = W->y + y;
    
    fb->grid[y*fb->cols + x].c = c;
    fb->grid[y*fb->cols + x].style = style;
}

void fbWindowDrawChars(Framebuffer *fb, Window *W, int x, int y, const char *s, int len, Style style)
{
    for (int i = 0; i < len && x + i < fb->cols; i++)
        fbWindowPutChar(fb, W, x + i, y, s[i], style);
}

void fbWindowDrawString(Framebuffer *fb, Window *W, int x, int y, const char *s, Style style)
{
    for (int i = 0; s[i] && x + i < fb->cols; i++)
        fbWindowPutChar(fb, W, x + i, y, s[i], style);
}

void fbWindowEraseLine(Framebuffer *fb, Window *W, int y, Color color)
{
    for (int i = 0; i < W->width; i++)
        fbWindowPutChar(fb, W, i, y, ' ', (Style){color, color, 0});
}

void fbWindowEraseLineFrom(Framebuffer *fb, Window *W, int y, int x, Color color)
{
    for (int i = x; i < W->width; i++)
        fbWindowPutChar(fb, W, i, y, ' ', (Style){color, color, 0});
}

void fbViewportPutChar(Framebuffer *fb, Window *W, int x, int y, char c, Style style)
{
    if (x < 0 || y < 0 || y >= W->viewport.rows || x >= W->viewport.cols) return;

    x = W->x + W->viewport.left + x;
    y = W->y + W->viewport.top + y;
    
    fb->grid[y*fb->cols + x].c = c;
    fb->grid[y*fb->cols + x].style = style;
}

void fbViewportDrawChars(Framebuffer *fb, Window *W, int x, int y, const char *s, int len, Style style)
{
    for (int i = 0; i < len && x + i < fb->cols; i++)
        fbViewportPutChar(fb, W, x + i, y, s[i], style);
}

void fbViewportDrawString(Framebuffer *fb, Window *W, int x, int y, const char *s, Style style)
{
    for (int i = 0; s[i] && x + i < fb->cols; i++)
        fbViewportPutChar(fb, W, x + i, y, s[i], style);
}

void fbViewportEraseLine(Framebuffer *fb, Window *W, int y, Color color)
{
    for (int i = 0; i < W->viewport.cols; i++)
        fbViewportPutChar(fb, W, i, y, ' ', (Style){color, color, 0});
}

void fbViewportEraseLineFrom(Framebuffer *fb, Window *W, int y, int x, Color color)
{
    for (int i = x; i < W->viewport.cols; i++)
        fbViewportPutChar(fb, W, i, y, ' ', (Style){color, color, 0});
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

// TODO: optimize... a lot
void fbRender(Framebuffer *fb, AppendBuffer *ab)
{
    abAppendString(ab, ESC_CURSOR_HOME);

    Style last_style;
    bool first_cell = true;

    for (int i = 0; i < fb->rows * fb->cols; i++)
    {
        Style *s = &fb->grid[i].style;

        if (first_cell || memcmp(&s->fg, &last_style, sizeof(Style)))
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

        abAppend(ab, &(fb->grid[i].c), 1); /* print the actual character */
    }
}
