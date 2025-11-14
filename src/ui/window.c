#include "window.h"

#include "editor.h"
#include "event.h"
#include "commands.h"
#include "ui.h"
#include "textbuffer.h"
#include "modes.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

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

static void computeNodeLayout(LayoutNode *node)
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

        W->viewport.top = 0;
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
    layout_root->y = TOPBAR_SIZE;
    layout_root->width = E.screencols;
    layout_root->height = E.screenrows - TOPBAR_SIZE - INFOBAR_SIZE;

    computeNodeLayout(layout_root);
}

static void windowInit(Window *W)
{
    W->cx = 0;
    W->cy = 0;
    W->expected_cx = 0;
    W->buf = NULL;
    W->node = NULL;

    W->viewport.top = 0;
    W->viewport.bottom = 0;
    W->viewport.left = 0;
    W->viewport.right = 0;
    W->viewport.scroll_margin = 5;
    W->viewport.horizontal_margin = 0;
    W->viewport.rowoff = 0;
    W->viewport.coloff = 0;
}

Window *editorCreateWindow(void)
{
    if (E.num_win == EDITOR_MAX_WIN)
        return NULL;

    Window *new_win = malloc(sizeof(Window));
    if (new_win == NULL)
    {
        // TODO: handle memory error
        editorFatalError("Fatal! Unable to create a new window\n");
        exit(1);
    }

    E.win[E.num_win] = new_win;
    E.active_win = new_win;
    windowInit(new_win);

    E.num_win++;

    return new_win;
}

void windowProcessKeypress(int key)
{
    switch (E.mode)
    {
        case NORMAL_MODE:
            editorNMProcessKeypress(key);
            break;
        case INSERT_MODE:
            editorIMProcessKeypress(key);
            break;
    }
}

void deleteWindowBuf(Window *W)
{
    bool found_buf = false;
    for (size_t i = 0; i < E.num_win; i++)
    {
        if (E.win[i] != W && W->buf == E.win[i]->buf)
        {
            found_buf = true;
            break;
        }
    }

    if (!found_buf)
    {
        deleteBuffer(W->buf);
    }
}

static void destroyWindow(Window *W)
{
    if (W == NULL)
        return;

    int found_idx = -1;
    for (size_t i = 0; i < E.num_win; i++)
    {
        if (E.win[i] == W)
        {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1)
    {
        /* Should be unreachable */
        editorFatalError("Fatal! Tried to destroy a window that isn't in the global list.\n");
        exit(EXIT_FAILURE);
    }

    deleteWindowBuf(W);

    free(E.win[found_idx]);

    int remaining_elements = E.num_win - 1 - found_idx;

    if (remaining_elements > 0)
    {
        memmove(&E.win[found_idx], &E.win[found_idx + 1], sizeof(Window *) * remaining_elements);
    }

    E.num_win--;

    E.win[E.num_win] = NULL; /* just to be sure */
}

void editorCloseWindow(void)
{
    if (E.num_win <= 1)
        exit(0);

    Window *winToClose = E.active_win;
    LayoutNode *leafToClose = winToClose->node;

    if (!leafToClose || leafToClose->type != LAYOUT_LEAF)
    {
        // editorWarningError("Fatal! a window has invalid layout node.\n");
        return;
    }

    LayoutNode *splitNode = leafToClose->parent;
    if (!splitNode)
    {
        // editorWarningError("Fatal! Attempting to close root leaf when other windows exist.\n");
        return;
    }

    LayoutNode *siblingNode = (splitNode->child1 == leafToClose) ? splitNode->child2 : splitNode->child1;
    LayoutNode *grandParent = splitNode->parent;

    if (grandParent == NULL)
    {
        layout_root = siblingNode;
        siblingNode->parent = NULL;
    }
    else
    {
        if (grandParent->child1 == splitNode)
        {
            grandParent->child1 = siblingNode;
        }
        else
        {
            grandParent->child2 = siblingNode;
        }
        siblingNode->parent = grandParent;
    }

    LayoutNode *newActiveNode = siblingNode;
    while (newActiveNode && newActiveNode->type != LAYOUT_LEAF)
    {
        newActiveNode = newActiveNode->child1;
    }

    if (newActiveNode && newActiveNode->window)
    {
        E.active_win = newActiveNode->window;
    }
    else
    {
        E.active_win = E.win[0];
    }

    destroyWindow(winToClose);

    free(leafToClose);
    free(splitNode);

    computeWindowLayout();
}

void editorSwitchWindow(Direction dir)
{
    /* NOTE: Originally I implemented this walking up and 
    down the tree to find the correct window to switch to... 
    But the code turned out to be much more long and complex 
    Theorically this geometrical approach here is slower (O(N))
    but the tradeoff is minimal and it's way clener. */

    if (E.num_win <= 1 || !E.active_win)
    {
        return;
    }

    Window *a = E.active_win;
    Window *nearest = NULL;

    for (size_t i = 0; i < E.num_win && !nearest; i++)
    {
        Window *c = E.win[i];

        if (c == a) continue;

        switch (dir)
        {
        case DIR_RIGHT:
            if (a->y >= c->y && 
                a->y < c->y + c->height &&
                c->x - (a->x + a->width) == 0)
            {
                nearest = c;
            }
            break;
        case DIR_LEFT:
            if (a->y >= c->y && 
                a->y < c->y + c->height &&
                a->x - (c->x + c->width) == 0)
            {
                nearest = c;
            }
            break;
        case DIR_DOWN:
            if (a->x >= c->x && 
                a->x < c->x + c->width &&
                c->y - (a->y + a->height) == 0)
            {
                nearest = c;
            }
            break;
        case DIR_UP:
            if (a->x >= c->x && 
                a->x < c->x + c->width &&
                a->y - (c->y + c->height) == 0)
            {
                nearest = c;
            }
            break;
        }
    }

    if (nearest != NULL)
    {
        E.active_win = nearest;
    }
}

void editorSplitWindow(bool split) /* TODO: check if there's enough space for the split */
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

    Window *new_win = editorCreateWindow();
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

#define MIN_RATIO 0.2
#define MAX_RATIO 0.8

void editorResizeWindow(Window *W, float amount, bool direct)
{
    if (W->node->parent == NULL) return;

    LayoutNode *parent = W->node->parent;
    if (parent->type != LAYOUT_SPLIT_HORIZONTAL && 
        parent->type != LAYOUT_SPLIT_VERTICAL)
        return;

    if (!direct && W->node == parent->child2) amount *= -1;
    parent->ratio += amount;

    if (parent->ratio < MIN_RATIO) parent->ratio = MIN_RATIO;
    else if (parent->ratio > MAX_RATIO) parent->ratio = MAX_RATIO;

    computeNodeLayout(parent);
}
