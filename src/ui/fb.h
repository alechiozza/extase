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

typedef struct FrameBuffer
{
    int rows;
    int cols;
    Cell *grid;
} FrameBuffer;

typedef struct AppendBuffer
{
    char *b;
    size_t len;
    size_t cap;
} AppendBuffer;

#define ABUF_INIT {NULL, 0, 0}

FrameBuffer *fbCreate(int rows, int cols);
void fbResize(FrameBuffer *fb, int rows, int cols);
void fbFree(FrameBuffer *fb);

void fbPutChar(FrameBuffer *fb, int x, int y, char c, Style style);
void fbPutCodepoint(FrameBuffer *fb, int x, int y, uint32_t c, Style style);

void fbDrawChars(FrameBuffer *fb, int x, int y, const char *s, int len, Style style);
void fbDrawString(FrameBuffer *fb, int x, int y, const char *s, Style style);
void fbEraseLine(FrameBuffer *fb, int y, Color color);
void fbEraseLineFrom(FrameBuffer *fb, int y, int x, Color color);

void fbWindowPutChar(FrameBuffer *fb, Window *W, int x, int y, char c, Style style);
void fbWindowPutCodepoint(FrameBuffer *fb, Window *W, int x, int y, uint32_t c, Style style);
void fbWindowDrawChars(FrameBuffer *fb, Window *W, int x, int y, const char *s, int len, Style style);
void fbWindowDrawString(FrameBuffer *fb, Window *W, int x, int y, const char *s, Style style);
void fbWindowEraseLine(FrameBuffer *fb, Window *W, int y, Color color);
void fbWindowEraseLineFrom(FrameBuffer *fb, Window *W, int y, int x, Color color);

void fbViewportPutChar(FrameBuffer *fb, Window *W, int x, int y, char c, Style style);
void fbViewportPutCodepoint(FrameBuffer *fb, Window *W, int x, int y, uint32_t c, Style style);
void fbViewportDrawChars(FrameBuffer *fb, Window *W, int x, int y, const char *s, int len, Style style);
void fbViewportDrawString(FrameBuffer *fb, Window *W, int x, int y, const char *s, Style style);
void fbViewportEraseLine(FrameBuffer *fb, Window *W, int y, Color color);
void fbViewportEraseLineFrom(FrameBuffer *fb, Window *W, int y, int x, Color color);

void fbRender(FrameBuffer *fb, AppendBuffer *ab);

void abAppend(AppendBuffer *ab, const char *s, int len);
void abAppendString(AppendBuffer *ab, const char *s);
void abFree(AppendBuffer *ab);

#endif /* __EDITOR_FB_H */
