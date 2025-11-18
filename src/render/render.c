#include "render.h"

#include "syntax.h"
#include "core.h"
#include "event.h"
#include "term.h"
#include "textbuffer.h"
#include "editor.h"

#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

static void updateRenderedRow(Row *row)
{
    unsigned int tabs = 0;

    for (int j = 0; j < row->size; j++)
        if (row->chars[j] == TAB) tabs++;
    
    unsigned int allocsize = row->size + tabs*TAB_SIZE + 1;
    if (allocsize > UINT32_MAX)
    {
        editorFatalError("Lines are too long (you crazy boy)\n");
        exit(EXIT_FAILURE);
    }

    row->render.c = realloc(row->render.c, allocsize);
    if (row->render.c == NULL)
    {
        // TODO: handle memory check
        return;
    }

    row->render.hl = realloc(row->render.hl, allocsize);
    if (row->render.hl == NULL)
    {
        return; // TODO: Handle memory failure
    }

    RenderRow *render = &row->render;

    /* We memset the highlight here because we should be able to
    force highlight on some characters (special and invalid characters) */
    memset(render->hl, HL_NORMAL, allocsize);

    int idx = 0;
    for (int j = 0; j < row->size; j++)
    {
        if (row->chars[j] == TAB)
        {
            render->c[idx] = '>';
            render->hl[idx] = HL_TAB;
            idx++;

            while((idx) % TAB_SIZE != 0)
            {
                render->c[idx] = ' ';
                render->hl[idx] = HL_TAB;
                idx++;
            }
        }
        else if (!isprint(row->chars[j]))
        {
            render->c[idx] = '?';
            render->hl[idx] = HL_NONPRINT;
            idx++;
        }
        // TODO: render indentation differently
        // else if (row->chars[j] == ' ')
        // {
        //     render[idx++] = '*';
        // } 
        else
        {
            render->c[idx] = row->chars[j];
            render->hl[idx] = HL_NORMAL;
            idx++;
        }
    }
    render->size = idx;
    render->c[idx] = '\0';
}

void editorUpdateRow(TextBuffer *buf, int row_idx)
{
    updateRenderedRow(&buf->rows[row_idx]);
    editorUpdateSyntax(buf, row_idx);
}

void freeRender(RenderRow *r)
{
    free(r->c);
    free(r->hl);
}
