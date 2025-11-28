#ifndef __EDITOR_TEXTBUFFER_H
#define __EDITOR_TEXTBUFFER_H

#include <stdbool.h>

#define INDENT_WITH_TABS 0
#define INDENT_WITH_SPACES 1

#define DEFAULT_INDENT_SIZE 4

typedef struct Row Row;
typedef struct Syntax Syntax;

typedef struct TextBuffer
{
    char *file_path;
    const char *filename;
    int numrows;
    Row *rows;
    Syntax *syntax;
    bool dirty;
    bool indent_mode;
    unsigned char indent_size;
} TextBuffer;

TextBuffer *createBuffer(const char *file_path);
void deleteBuffer(TextBuffer *buf);

#endif /* __EDITOR_TEXTBUFFER_H */
