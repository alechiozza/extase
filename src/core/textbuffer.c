#include "textbuffer.h"

#include "editor.h"
#include "core.h"
#include "event.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>

typedef struct TextBufferVector
{
    TextBuffer **data;
    size_t size;
    size_t capacity;
} TextBufferVector;

#define TEXT_BUF_VEC_INIT (TextBufferVector){NULL, 0, 0}

static TextBufferVector B = TEXT_BUF_VEC_INIT;

TextBuffer *createBuffer(const char *file_path)
{
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

    if (B.size == B.capacity)
    {
        size_t new_capacity = next_capacity(B.capacity, B.size+1);
        TextBuffer **new_data = realloc(B.data, new_capacity * sizeof(TextBuffer *));
        if (!new_data)
        {
            new_capacity = B.size+1; /* One last hope*/
            new_data = realloc(B.data, new_capacity * sizeof(TextBuffer *));
            if (!new_data) return NULL;
        }
        
        B.data = new_data;
        B.capacity = new_capacity;
    }

    B.data[B.size] = buf;
    B.size++;

    return buf;
}

void deleteBuffer(TextBuffer *buf)
{
    int found_idx = -1;
    for (size_t i = 0; i < B.size; i++)
    {
        if (B.data[i] == buf)
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

    int remaining_elements = B.size - 1 - found_idx;

    if (remaining_elements > 0)
    {
        memmove(&B.data[found_idx], &B.data[found_idx + 1], sizeof(TextBuffer *) * remaining_elements);
    }

    B.size--;

    B.data[B.size] = NULL;
}

TextBuffer *findOpenBuffer(const char *file_path)
{
    for (size_t i = 0; i < B.size; i++)
    {
        if (strcmp(B.data[i]->file_path, file_path) == 0)
        {
            return B.data[i];
        }
    }
    return NULL;
}