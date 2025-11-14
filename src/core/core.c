#include "core.h"

#include "term.h"
#include "editor.h"
#include "event.h"
#include "syntax.h"
#include "textbuffer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void updateRenderedRow(Row *row)
{
    unsigned int tabs = 0;

    free(row->render);

    for (int j = 0; j < row->size; j++)
        if (row->chars[j] == TAB) tabs++;
    
    unsigned int allocsize = row->size + tabs*TAB_SIZE + 1;
    if (allocsize > UINT32_MAX)
    {
        editorFatalError("Lines are too long (you crazy boy)\n");
        exit(EXIT_FAILURE);
    }

    row->render = malloc(row->size + tabs*TAB_SIZE + 1);
    if (row->render == NULL)
    {
        // TODO: handle memory check
        return;
    }

    int idx = 0;
    for (int j = 0; j < row->size; j++)
    {
        if (row->chars[j] == TAB)
        {
            row->render[idx++] = ' ';
            while((idx) % TAB_SIZE != 0)
                row->render[idx++] = ' ';
        }
        // TODO: render indentation differently
        // else if (row->chars[j] == ' ')
        // {
        //     row->render[idx++] = '*';
        // } 
        else
        {
            row->render[idx++] = row->chars[j];
        }
    }
    row->rsize = idx;
    row->render[idx] = '\0';
}

void editorUpdateRow(TextBuffer *buf, int row_idx)
{
    updateRenderedRow(&buf->rows[row_idx]);
    editorUpdateSyntax(buf, row_idx);
}

void editorInsertRow(TextBuffer *buf, int at, char *s, size_t len)
{
    if (!buf || at > buf->numrows)
        return;

    // TODO: meh
    Row *new_rows = realloc(buf->rows, sizeof(Row) * (buf->numrows + 1));
    if (!new_rows)
    {
        // TODO: handle memory error
        return;
    }
    buf->rows = new_rows;

    if (at < buf->numrows)
    {
        /* Shift everything down by one */
        memmove(&buf->rows[at + 1], &buf->rows[at], sizeof(Row) * (buf->numrows - at));
        
        for (int j = at + 1; j <= buf->numrows; j++)
            buf->rows[j].idx++;
    }

    Row *row = &buf->rows[at];

    row->idx = at;
    row->size = len;
    row->chars = malloc(len + 1);
    if (!row->chars)
    {
        // TODO: handle memory error
        return;
    }
    memcpy(row->chars, s, len + 1);
    row->hl = NULL;
    row->hl_oc = 0;
    row->render = NULL;
    row->rsize = 0;

    editorUpdateRow(buf, at);

    buf->numrows++;
    buf->dirty = true;
}

static void editorFreeRow(Row *row)
{
    free(row->render);
    free(row->chars);
    free(row->hl);
}

void editorDelRow(TextBuffer *buf, int at)
{
    if (!buf || at >= buf->numrows)
        return;
    
    Row *row = &buf->rows[at];
    editorFreeRow(row);

    memmove(&buf->rows[at], &buf->rows[at + 1], sizeof(Row) * (buf->numrows - at - 1));

    buf->numrows--;

    for (int j = at; j < buf->numrows; j++)
        buf->rows[j].idx++;
    
    buf->dirty = true;
}

void editorRowInsertChar(TextBuffer *buf, int row_idx, int at, int c)
{
    if (!buf || row_idx < 0 || row_idx >= buf->numrows)
        return;

    Row *row = &buf->rows[row_idx];

    if (at > row->size)
    {
        int padlen = at - row->size;
        char *new_chars = realloc(row->chars, row->size + padlen + 2); /* +2 : new char and null term. */
        if (!new_chars)
        {
            // TODO: handle memory error
            return;
        }
        row->chars = new_chars;

        memset(row->chars + row->size, ' ', padlen);
        row->chars[row->size + padlen + 1] = '\0';
        row->size += padlen + 1;
    }
    else
    {
        char *new_chars = realloc(row->chars, row->size + 2);
        if (!new_chars)
        {
            // TODO: handle memory error
            return;
        }
        row->chars = new_chars;

        memmove(row->chars + at + 1, row->chars + at, row->size - at + 1);
        row->size++;
    }

    row->chars[at] = c;

    editorUpdateRow(buf, row_idx);

    buf->dirty = true;
}

char editorRowGetChar(Row *row, int at)
{
    if (row->size <= at)
        return '\0';
    
    return *(row->chars + at);
}

void editorRowAppendString(TextBuffer *buf, int row_idx, char *s, size_t len)
{
    if (!buf || row_idx < 0 || row_idx >= buf->numrows)
        return;

    Row *row = &buf->rows[row_idx];

    char *new_chars = realloc(row->chars, row->size + len + 1);
    if (!new_chars)
    {
        // TODO: handle memory error
        return;
    }
    row->chars = new_chars;

    memcpy(row->chars + row->size, s, len);
    row->size += len;
    row->chars[row->size] = '\0';

    editorUpdateRow(buf, row_idx);
    
    buf->dirty = true;
}

void editorRowDelChar(TextBuffer *buf, int row_idx, int at)
{
    if (!buf || row_idx < 0 || row_idx >= buf->numrows)
        return;

    Row *row = &buf->rows[row_idx];
    
    if (at >= row->size)
        return;
    
    memmove(row->chars + at, row->chars + at + 1, row->size - at);
    row->size--;
    
    editorUpdateRow(buf, row_idx);
    
    buf->dirty = true;
}
