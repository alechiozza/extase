#ifndef __EDITOR_WINDOW_H
#define __EDITOR_WINDOW_H

#include <stdbool.h>

#include "layout.h"

typedef struct Viewport
{
    int top, bottom;
    int left, right;
    int cols, rows;
    int rowoff;     /* Offset of row displayed. */
    int coloff;     /* Offset of column displayed. */
} Viewport;

typedef struct TextBuffer TextBuffer;
typedef struct FrameBuffer FrameBuffer;

#define WINDOW_MAX_TAB 10

typedef struct WindowTab
{
    TextBuffer *buf;
    int cx, cy;     /* cursor x, y*/
    int expected_cx;
    int rowoff;     /* Offset of row displayed. */
    int coloff;     /* Offset of column displayed. */
} WindowTab;

typedef struct Window
{
    LayoutNode *node;
    Viewport viewport;
    WindowTab *tabs[WINDOW_MAX_TAB];
    WindowTab *active_tab;
    TextBuffer *buf;
    int x, y;
    int width, height;
    int cx, cy; /* cursor x, y*/
    int expected_cx;
} Window;

Window *createWindow(void);
void closeWindow(void);
void deleteWindowBuf(Window *W);

void windowProcessKeypress(int key);

void switchWindow(Direction dir);
void resizeWindow(Window *W, float amount, bool directed);

int getLineNumberWidth(Window *W);
void drawWindow(FrameBuffer *fb, Window *W);

#endif /* __EDITOR_WINDOW_H */
