#ifndef __EDITOR_CORE_H
#define __EDITOR_CORE_H

#include <stddef.h>

typedef struct TextBuffer TextBuffer;

// TODO: reimplement all of this using a better data structure
typedef struct Row
{
    int idx;
    int size;
    char *chars;       /* null terminated */
    int rsize;         /* rendered row size */
    char *render;      /* Row rendered for screen (for TABs) */
    unsigned char *hl; /* syntax highlight */
    int hl_oc;         /* Row had open comment at end in last syntax highlight check, shitty */
} Row;

void editorUpdateRow(TextBuffer *buf, int row_idx);
void editorInsertRow(TextBuffer *buf, int at, char *s, size_t len);
void editorDelRow(TextBuffer *buf, int at);
void editorRowInsertChar(TextBuffer *buf, int row_idx, int at, int c);
char editorRowGetChar(Row *row, int at);
void editorRowAppendString(TextBuffer *buf, int row_idx, char *s, size_t len);
void editorRowDelChar(TextBuffer *buf, int row_idx, int at);


#endif /* __EDITOR_CORE_H */
