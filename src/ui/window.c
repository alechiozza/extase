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

Window *createWindow(void)
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

void closeWindow(void)
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
        setLayoutRoot(siblingNode);
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

void switchWindow(Direction dir)
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

#define MIN_RATIO 0.2
#define MAX_RATIO 0.8

void resizeWindow(Window *W, float amount, bool direct)
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

void drawWindow()
{
    // TODO: move all the window draw logic in this file
}
