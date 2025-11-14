#include "textbuffer.h"

#include "editor.h"
#include "core.h"
#include "event.h"

#include <string.h>
#include <stdlib.h>

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