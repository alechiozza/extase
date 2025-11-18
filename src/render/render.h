#ifndef __EDITOR_RENDER_H
#define __EDITOR_RENDER_H

#include <stdint.h>
#include <unistd.h>

typedef struct TextBuffer TextBuffer;

typedef struct RenderRow
{
    char *c;
    unsigned char *hl;
    size_t size;
} RenderRow;

#define RENDER_NULL (RenderRow){NULL, NULL, 0}

void editorUpdateRow(TextBuffer *buf, int row_idx);
void freeRender(RenderRow *r);

#endif /* __EDITOR_RENDER_H */
