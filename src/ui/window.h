#ifndef __EDITOR_WINDOW_H
#define __EDITOR_WINDOW_H

#include <stdbool.h>

typedef struct Viewport
{
    int top, bottom;
    int left, right;
    int cols, rows;
    int scroll_margin;
    int horizontal_margin;
    int rowoff;     /* Offset of row displayed. */
    int coloff;     /* Offset of column displayed. */
} Viewport;

typedef struct TextBuffer TextBuffer;

typedef struct Window
{
    Viewport viewport;
    TextBuffer *buf;
    int x, y;
    int width, height;
    int cx, cy; /* cursor x, y*/
    int expected_cx;
    bool linenums;
} Window;

void editorOpenWindow(const char *filename);
void editorCloseWindow(void);
void computeWindowLayout(void);
void editorSwitchWindow(void);

#endif /* __EDITOR_WINDOW_H */
