#ifndef __EDITOR_LAYOUT_H
#define __EDITOR_LAYOUT_H

#include <stdbool.h>

typedef struct Window Window;

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

void initLayout(void);
void setLayoutRoot(LayoutNode *node);

void computeNodeLayout(LayoutNode *node);
void computeWindowLayout(void);

void splitWindowLayout(bool split);

#endif /* __EDITOR_LAYOUT_H */
