#ifndef __EDITOR_SYNTAX_H
#define __EDITOR_SYNTAX_H

typedef struct TextBuffer TextBuffer;
typedef struct Style Style;

enum HL
{
    HL_NORMAL = 0,
    HL_NONPRINT = 1,
    HL_COMMENT = 2,
    HL_MLCOMMENT = 3,
    HL_KEYWORD1 = 4,
    HL_KEYWORD2 = 5,
    HL_STRING = 6,
    HL_NUMBER = 7,
    HL_MATCH = 8
};

enum HL_Flags{
    HL_HIGHLIGHT_STRINGS = (1 << 0),
    HL_HIGHLIGHT_NUMBERS = (1 << 1)
};

typedef struct Syntax
{
    char **filematch;
    char **keywords;
    char singleline_comment_start[2];
    char multiline_comment_start[3];
    char multiline_comment_end[3];
    int flags;
} Syntax;

void editorSelectSyntaxHighlight(TextBuffer *buf, const char *filename);
Style editorSyntaxToColor(int hl);
void editorUpdateSyntax(TextBuffer *buf, int row_idx);

#endif /* __EDITOR_SYNTAX_H */
