#ifndef __EDITOR_WINDOW_H
#define __EDITOR_WINDOW_H

#include <stdbool.h>

struct Viewport
{
    int top, bottom;
    int left, right;
    int cols, rows;
    int scroll_margin;
    int horizontal_margin;
    int rowoff;     /* Offset of row displayed. */
    int coloff;     /* Offset of column displayed. */
};

typedef struct TextBuffer TextBuffer;

typedef struct Window
{
    struct Viewport viewport;
    TextBuffer *buf;
    int x, y;
    int width, height;
    int cx, cy; /* cursor x, y*/
    int expected_cx;
    bool linenums;
} Window;

void editorOpenWindow(void);
void computeWindowLayout(void);

#endif /* __EDITOR_WINDOW_H */
