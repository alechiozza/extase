#ifndef __EDITOR_SYNTAX_H
#define __EDITOR_SYNTAX_H

#include "color.h"

typedef struct TextBuffer TextBuffer;
typedef struct Style Style;
typedef struct SyntaxGroup SyntaxGroup;

enum HL
{
    HL_NORMAL = 0,
    HL_NONPRINT,
    HL_COMMENT,
    HL_MLCOMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_KEYWORD3,
    HL_KEYWORD4,
    HL_KEYWORD5,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH,
    HL_TAB
};

typedef struct Syntax
{
    char **filematch;
    SyntaxGroup *groups;
    size_t group_num;
    char singleline_comment_start[2];
    char multiline_comment_start[3];
    char multiline_comment_end[3];
    int flags;
} Syntax;

void editorSelectSyntaxHighlight(TextBuffer *buf, const char *filename);
Style editorSyntaxToColor(unsigned char hl);
void editorUpdateSyntax(TextBuffer *buf, int row_idx);

#endif /* __EDITOR_SYNTAX_H */
