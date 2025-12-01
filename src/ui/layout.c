#include "layout.h"

#include "window.h"
#include "editor.h"
#include "event.h"
#include "ui.h"

#include <stdlib.h>

static LayoutNode *layout_root;

void initLayout(void)
{
    if (E.active_win == NULL || layout_root != NULL) return;

    LayoutNode *rootNode = malloc(sizeof(LayoutNode));
    if (!rootNode)
    {
        editorFatalError("Fatal! Memory error\n");
        exit(1);
    }

    rootNode->type = LAYOUT_LEAF;
    rootNode->parent = NULL;
    rootNode->child1 = NULL;
    rootNode->child2 = NULL;
    rootNode->window = E.active_win;

    layout_root = rootNode;
    E.active_win->node = layout_root;

    computeWindowLayout();
}

void setLayoutRoot(LayoutNode *node)
{
    layout_root = node;
}

void computeNodeLayout(LayoutNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case LAYOUT_LEAF:
    {
        Window *W = node->window;
        if (!W)
            return;

        W->x = node->x;
        W->y = node->y;
        W->width = node->width;
        W->height = node->height;

        int left_padding = (E.linenums ? getLineNumberWidth(W) : 0);

        W->viewport.top = TAB_VIEW_SIZE;
        W->viewport.bottom = INFOBAR_SIZE;
        W->viewport.left = left_padding;
        W->viewport.right = SCROLLBAR_SIZE;

        W->viewport.rows = W->height - W->viewport.top - W->viewport.bottom;
        W->viewport.cols = W->width - W->viewport.left - W->viewport.right;

        if (W->viewport.rows < 0)
            W->viewport.rows = 0;
        if (W->viewport.cols < 0)
            W->viewport.cols = 0;

        if (W->cy >= W->viewport.rows)
        {
            W->cy = (W->viewport.rows > 0) ? W->viewport.rows - 1 : 0;
        }
        if (W->cx >= W->viewport.cols)
        {
            W->cx = (W->viewport.cols > 0) ? W->viewport.cols - 1 : 0;
        }
        break;
    }

    case LAYOUT_SPLIT_VERTICAL:
    {
        if (!node->child1 || !node->child2)
            return;

        int child1_width = (int)(node->width * node->ratio);
        int child2_width = node->width - child1_width;

        node->child1->x = node->x;
        node->child1->y = node->y;
        node->child1->width = child1_width;
        node->child1->height = node->height;

        node->child2->x = node->x + child1_width;
        node->child2->y = node->y;
        node->child2->width = child2_width;
        node->child2->height = node->height;

        computeNodeLayout(node->child1);
        computeNodeLayout(node->child2);
        break;
    }

    case LAYOUT_SPLIT_HORIZONTAL:
    {
        if (!node->child1 || !node->child2)
            return;

        int child1_height = (int)(node->height * node->ratio);
        int child2_height = node->height - child1_height;

        node->child1->x = node->x;
        node->child1->y = node->y;
        node->child1->width = node->width;
        node->child1->height = child1_height;

        node->child2->x = node->x;
        node->child2->y = node->y + child1_height;
        node->child2->width = node->width;
        node->child2->height = child2_height;

        computeNodeLayout(node->child1);
        computeNodeLayout(node->child2);
        break;
    }
    }
}

void computeWindowLayout(void)
{
    if (!layout_root)
        return;

    layout_root->x = 0;
    layout_root->y = (E.no_topbar ? 0 : TOPBAR_SIZE);
    layout_root->width = E.screencols;
    layout_root->height = E.screenrows - layout_root->y - INFOBAR_SIZE;

    computeNodeLayout(layout_root);
}

void splitWindowLayout(bool split) /* TODO: check if there's enough space for the split */
{
    Window *original_win = E.active_win;
    if (!original_win)
        return;

    LayoutNode *original_leaf = original_win->node;
    if (!original_leaf || original_leaf->type != LAYOUT_LEAF)
    {
        return;
    }

    LayoutNode *new_leaf = malloc(sizeof(LayoutNode));
    if (!new_leaf)
    {
        // TODO: handle memory error
        return;
    }
    new_leaf->type = LAYOUT_LEAF;

    Window *new_win = createWindow();
    if (!new_win)
    {
        E.active_win = original_win;
        return;
    }

    new_win->buf = original_win->buf;
    new_win->viewport = original_win->viewport;
    new_win->cx = original_win->cx;
    new_win->cy = original_win->cy;
    new_win->expected_cx = original_win->expected_cx;
    new_win->node = new_leaf;
    new_leaf->window = new_win;

    LayoutNode *newSplitNode = malloc(sizeof(LayoutNode));
    if (!newSplitNode)
    {
        free(new_leaf);
        free(new_win);
        E.num_win--;
        E.active_win = original_win;
        return;
    }
    newSplitNode->type = (split == SPLIT_VERTICAL) ? LAYOUT_SPLIT_VERTICAL : LAYOUT_SPLIT_HORIZONTAL;
    newSplitNode->ratio = 0.5;
    newSplitNode->parent = original_leaf->parent;

    if (newSplitNode->parent == NULL)
    {
        layout_root = newSplitNode;
    }
    else if (newSplitNode->parent->child1 == original_leaf)
    {
        newSplitNode->parent->child1 = newSplitNode;
    }
    else
    {
        newSplitNode->parent->child2 = newSplitNode;
    }

    newSplitNode->child1 = original_leaf;
    newSplitNode->child2 = new_leaf;

    original_leaf->parent = newSplitNode;
    new_leaf->parent = newSplitNode;

    computeWindowLayout();
}
