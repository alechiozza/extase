#ifndef __EDITOR_TEXTBUFFER_H
#define __EDITOR_TEXTBUFFER_H

#include <stdbool.h>

typedef struct Row Row;
typedef struct Syntax Syntax;

typedef struct TextBuffer
{
    char *filename;
    int numrows;
    Row *rows;
    Syntax *syntax;
    bool dirty;
} TextBuffer;

TextBuffer *createBuffer(const char *filename);
void deleteBuffer(TextBuffer *buf);

#endif /* __EDITOR_TEXTBUFFER_H */
