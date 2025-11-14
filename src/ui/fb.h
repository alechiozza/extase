#ifndef __EDITOR_FB_H
#define __EDITOR_FB_H

#include <stdbool.h>
#include <stddef.h>

#include "color.h"

typedef struct Window Window;

typedef struct Cell
{
    uint32_t c;
    uint8_t width;
    Style style;
} Cell;

typedef struct Framebuffer
{
    int rows;
    int cols;
    Cell *grid;
} Framebuffer;

typedef struct AppendBuffer
{
    char *b;
    size_t len;
    size_t cap;
} AppendBuffer;

#define ABUF_INIT {NULL, 0, 0}

Framebuffer *fbCreate(int rows, int cols);
void fbResize(Framebuffer *fb, int rows, int cols);
void fbFree(Framebuffer *fb);

void fbPutChar(Framebuffer *fb, int x, int y, char c, Style style);
void fbPutCodepoint(Framebuffer *fb, int x, int y, uint32_t c, Style style);

void fbDrawChars(Framebuffer *fb, int x, int y, const char *s, int len, Style style);
void fbDrawString(Framebuffer *fb, int x, int y, const char *s, Style style);
void fbEraseLine(Framebuffer *fb, int y, Color color);
void fbEraseLineFrom(Framebuffer *fb, int y, int x, Color color);

void fbWindowPutChar(Framebuffer *fb, Window *W, int x, int y, char c, Style style);
void fbWindowDrawChars(Framebuffer *fb, Window *W, int x, int y, const char *s, int len, Style style);
void fbWindowDrawString(Framebuffer *fb, Window *W, int x, int y, const char *s, Style style);
void fbWindowEraseLine(Framebuffer *fb, Window *W, int y, Color color);
void fbWindowEraseLineFrom(Framebuffer *fb, Window *W, int y, int x, Color color);

void fbViewportPutChar(Framebuffer *fb, Window *W, int x, int y, char c, Style style);
void fbViewportDrawChars(Framebuffer *fb, Window *W, int x, int y, const char *s, int len, Style style);
void fbViewportDrawString(Framebuffer *fb, Window *W, int x, int y, const char *s, Style style);
void fbViewportEraseLine(Framebuffer *fb, Window *W, int y, Color color);
void fbViewportEraseLineFrom(Framebuffer *fb, Window *W, int y, int x, Color color);

void fbRender(Framebuffer *fb, AppendBuffer *ab);

void abAppend(AppendBuffer *ab, const char *s, int len);
void abAppendString(AppendBuffer *ab, const char *s);
void abFree(AppendBuffer *ab);

#endif /* __EDITOR_FB_H */
