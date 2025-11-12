#ifndef __EDITOR_SYNTAX_H
#define __EDITOR_SYNTAX_H

#include "color.h"

typedef struct TextBuffer TextBuffer;
typedef struct Style Style;

enum HL
{
    HL_NORMAL = 0,
    HL_NONPRINT,
    HL_COMMENT,
    HL_MLCOMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_KEYWORD3,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH
};

enum HL_Flags{
    HL_HIGHLIGHT_STRINGS = (1 << 0),
    HL_HIGHLIGHT_NUMBERS = (1 << 1)
};

typedef struct SyntaxGroup
{
    char **keywords;
    Color color;
} SyntaxGroup;

typedef struct Syntax
{
    char **filematch;
    SyntaxGroup *groups;
    char singleline_comment_start[2];
    char multiline_comment_start[3];
    char multiline_comment_end[3];
    int flags;
} Syntax;

void editorSelectSyntaxHighlight(TextBuffer *buf, const char *filename);
Style editorSyntaxToColor(int hl);
void editorUpdateSyntax(TextBuffer *buf, int row_idx);

#endif /* __EDITOR_SYNTAX_H */
