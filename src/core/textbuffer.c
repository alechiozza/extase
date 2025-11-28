#include "textbuffer.h"

#include "editor.h"
#include "core.h"
#include "event.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>

TextBuffer *createBuffer(const char *file_path)
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
    buf->syntax = NULL;
    buf->file_path = strdup(file_path);
    buf->filename = get_filename_from_path(buf->file_path);
    buf->dirty = false;
    buf->indent_mode = INDENT_WITH_SPACES;
    buf->indent_size = DEFAULT_INDENT_SIZE;

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

    free(buf->file_path);

    for (int i = 0; i < buf->numrows; i++)
    {
        free(buf->rows[i].chars);
        freeRender(&buf->rows[i].render);
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