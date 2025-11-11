#include "editor.h"
#include "event.h"
#include "term.h"
#include "modes.h"
#include "core.h"
#include "syntax.h"
#include "commands.h"
#include "window.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

struct editorConfig E;

TextBuffer *createBuffer(const char *filename)
{
    if (E.num_buf == EDITOR_MAX_BUF) return NULL;

    TextBuffer *buf = malloc(sizeof(TextBuffer));
    if (buf == NULL)
    {
        editorFatalError("Fatal! Memory error during text buffer allocation\n");
        exit(EXIT_FAILURE);
    }
    buf->numrows = 0;
    buf->rows = NULL;
    buf->dirty = false;
    buf->syntax = NULL;
    buf->filename = strdup(filename);

    E.buf[E.num_buf] = buf;

    E.num_buf++;

    return buf;
}

void deleteBuffer(TextBuffer *buf)
{
    int found_idx = -1;
    for (size_t i = 0; i < E.num_buf; i++)
    {
        if (E.buf[i] == buf)
        {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1)
    {
        exit(EXIT_FAILURE);
    }

    free(buf->filename);

    for (int i = 0; i < buf->numrows; i++)
    {
        free(buf->rows[i].chars);
        free(buf->rows[i].render);
        free(buf->rows[i].hl);
    }

    free(buf->rows);

    free(buf);

    int remaining_elements = E.num_buf - 1 - found_idx;

    if (remaining_elements > 0)
    {
        memmove(&E.buf[found_idx], &E.buf[found_idx + 1], sizeof(TextBuffer *) * remaining_elements);
    }

    E.num_buf--;

    E.buf[E.num_buf] = NULL;
}

static void initWindow(const char *filename)
{
    editorCreateWindow();
    editorOpen(E.active_win, filename);
}

static void initLayout(void)
{
    if (E.active_win == NULL) return;

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

    E.layout_root = rootNode;
    E.active_win->node = E.layout_root;

    computeWindowLayout();
}

void initEditor(char *filename)
{
    E.active_win = NULL;
    E.num_win = 0;
    E.num_buf = 0;
    E.too_small = false;
    E.rawmode = false;
    E.last_search = NULL;
    E.layout_root = NULL;
    E.color_mode = getColorMode();

    E.linenums = true;
    E.relativenums = true;
    E.auto_paren = true;
    E.auto_indent = true;

    updateWindowSize();
    signal(SIGWINCH, handleSigWinCh);

    if (enableRawMode(STDIN_FILENO) == -1)
    {
        editorFatalError("No tty\n");
        exit(EXIT_FAILURE);
    }

    setCursorMode(CURSOR_BLINK_BAR);

    editorSetInsertMode();

    initWindow(filename);
    initLayout();
}

void editorInsertChar(Window *W, int c)
{
    int filerow = W->viewport.rowoff + W->cy;
    int filecol = W->viewport.coloff + W->cx;
    TextBuffer *buf = W->buf;
    Row *row = (filerow >= buf->numrows) ? NULL : &buf->rows[filerow];

    // TODO: meh
    if (!row)
    {
        while (buf->numrows <= filerow)
            editorInsertRow(buf, buf->numrows, "", 0);
    }

    editorRowInsertChar(buf, filerow, filecol, c);

    if (W->cx == W->viewport.cols - 1)
    {
        W->viewport.coloff++;
    }
    else
    {
        W->cx++;
    }   
    W->expected_cx = W->cx;
    buf->dirty = true;
}

char editorGetCharAtCursor(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;
    int filecol = W->viewport.coloff + W->cx;

    Row *row = (filerow >= W->buf->numrows) ? NULL : &W->buf->rows[filerow];
    if (!row) return '\0';

    return editorRowGetChar(row, filecol);
}

char editorGetCharBeforeCursor(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;
    int filecol = W->viewport.coloff + W->cx;

    if (filecol == 0) return '\0';

    Row *row = (filerow >= W->buf->numrows) ? NULL : &W->buf->rows[filerow];
    if (!row) return '\0';

    return editorRowGetChar(row, filecol-1);
}

void editorIndentLine(Window *W)
{
    int column = W->viewport.coloff + W->cx;
    do
    {
        editorInsertChar(W, ' ');
        column++;
    } while (column % TAB_SIZE != 0);
}

static void editorIndentNewline(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;
    Row *previous_row = (filerow > 0) ? &W->buf->rows[filerow-1] : NULL;

    if (previous_row == NULL) return; /* Function misused */

    for (char *current = previous_row->chars; *current == ' '; current++)
    {
        editorInsertChar(W, ' ');
    }
}

/* Inserting a newline is slightly complex as we have to handle inserting a
 * newline in the middle of a line, splitting the line as needed. */
void editorInsertNewline(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;
    int filecol = W->viewport.coloff + W->cx;
    TextBuffer *buf = W->buf;
    Row *row = (filerow >= buf->numrows) ? NULL : &buf->rows[filerow];

    if (filecol == 0)
    {
        editorInsertRow(buf, filerow, "", 0);
    }
    else
    {
        /* We are in the middle of a line. Split it between two rows. */
        editorInsertRow(buf, filerow + 1, row->chars + filecol, row->size - filecol);
        row = &buf->rows[filerow];
        row->chars[filecol] = '\0';
        row->size = filecol;
        editorUpdateRow(buf, filerow);
    }

    /* Move the cursor down */
    if (W->cy > W->viewport.rows - (W->viewport.scroll_margin + W->viewport.top))
    {
        W->viewport.rowoff++;
    }
    else
    {
        W->cy++;
    }
    W->cx = 0;
    W->expected_cx = 0;
    W->viewport.coloff = 0;

    editorIndentNewline(W);
}

/* Delete the char at the current prompt position. */
void editorDelChar(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;
    int filecol = W->viewport.coloff + W->cx;
    TextBuffer *buf = W->buf;
    Row *row = (filerow >= buf->numrows) ? NULL : &buf->rows[filerow];

    if (!row || (filecol == 0 && filerow == 0))
        return;
    
    if (filecol == 0)
    {
        /* Handle the case of column 0, we need to move the current line
         * on the right of the previous one. */
        filecol = buf->rows[filerow - 1].size;
        editorRowAppendString(buf, filerow - 1, row->chars, row->size);
        editorDelRow(buf, filerow);
        if (W->cy == 0)
            W->viewport.rowoff--;
        else
            W->cy--;
        W->cx = filecol;
        if (W->cx >= W->viewport.cols)
        {
            int shift = (W->viewport.cols - W->cx) + 1;
            W->cx -= shift;
            W->viewport.coloff += shift;
        }
    }
    else
    {
        /* Delete multiple spaces in one move */
        if (editorRowGetChar(row, filecol-1) == ' ')
        {
            do
            {
                editorRowDelChar(buf, filerow, filecol - 1);

                if (W->cx == 0)
                    W->viewport.coloff--;
                else
                    W->cx--;
                
                filecol--;
            } while (filecol > 0 && filecol % TAB_SIZE != 0 && editorRowGetChar(row, filecol-1) == ' ');
        }
        else
        {
            editorRowDelChar(buf, filerow, filecol - 1);

            if (W->cx == 0)
                W->viewport.coloff--;
            else
                W->cx--;
        }

        editorUpdateRow(buf, filerow);
    }

    W->expected_cx = W->cx;
    
    buf->dirty = true;
}

void editorDelNextChar(Window *W)
{
    int filerow = W->viewport.rowoff + W->cy;
    int filecol = W->viewport.coloff + W->cx;
    TextBuffer *buf = W->buf;
    Row *row = (filerow >= buf->numrows) ? NULL : &buf->rows[filerow];

    if (!row) return;

    if (filecol == row->size)
    {
        if (filerow + 1 >= buf->numrows)
            return;
            
        /* Handle the case of last column, we need to move the next line
         * on the right of the current one. */
        Row *nextRow = &buf->rows[filerow+1];
        editorRowAppendString(buf, filerow, nextRow->chars, nextRow->size);
        editorDelRow(buf, filerow+1);
    }
    else
    {
        editorRowDelChar(buf, filerow, filecol);
    }
    
    buf->dirty = true;
}
