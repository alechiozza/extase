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
typedef struct LayoutNode LayoutNode;

typedef struct Window
{
    LayoutNode *node;
    Viewport viewport;
    TextBuffer *buf;
    int x, y;
    int width, height;
    int cx, cy; /* cursor x, y*/
    int expected_cx;
} Window;

typedef enum
{
    LAYOUT_LEAF,
    LAYOUT_SPLIT_VERTICAL,
    LAYOUT_SPLIT_HORIZONTAL
} LayoutNodeType;

#define SPLIT_VERTICAL 0
#define SPLIT_HORIZONTAL 1

typedef struct LayoutNode
{
    LayoutNodeType type;
    struct LayoutNode *parent;

    Window *window;

    struct LayoutNode *child1;
    struct LayoutNode *child2;
    float ratio; 

    int x, y, width, height;

} LayoutNode;

typedef enum Direction
{
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

Window *editorCreateWindow(void);
void editorCloseWindow(void);
void computeWindowLayout(void);
void editorSwitchWindow(Direction dir);
void editorSplitWindow(bool split);
void editorResizeWindow(Window *W, float amount, bool directed);

void deleteWindowBuf(Window *W);

#endif /* __EDITOR_WINDOW_H */
