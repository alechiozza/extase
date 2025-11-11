#ifndef __EDITOR_H
#define __EDITOR_H

#define EDITOR_VERSION "0.0.1"

#define TAB_SIZE 4
#define TOPBAR_SIZE 1
#define INFOBAR_SIZE 1
#define SCROLLBAR_SIZE 1

#define MIN_LNUM_WIDTH 3

#define EDITOR_QUERY_LEN 128
#define EDITOR_STATUSMSG_LENGTH 256

#define EDITOR_MIN_WIDTH 40
#define EDITOR_MIN_HEIGHT 6

#include <stdbool.h>
#include "color.h"

typedef struct Window Window;
typedef struct Row Row;
typedef struct Syntax Syntax;
typedef struct LayoutNode LayoutNode;

typedef struct TextBuffer
{
    char *filename;
    int numrows;
    Row *rows;
    Syntax *syntax;
    bool dirty;
} TextBuffer;

#define EDITOR_MAX_WIN 10
#define EDITOR_MAX_BUF 10

struct editorConfig
{
    int screenrows;
    int screencols;
    ColorMode color_mode;

    LayoutNode *layout_root;
    Window *win[EDITOR_MAX_WIN];
    Window *active_win;
    size_t num_win;

    TextBuffer *buf[EDITOR_MAX_BUF];
    size_t num_buf;

    bool linenums;
    bool relativenums; /* Enable line numbers relative numeration */
    bool rawmode;    /* Is terminal raw mode enabled? */
    bool too_small;  /* The screen is too small to be rendered */
    bool auto_paren; /* Enable smart parentesis */
    bool auto_indent;
    char statusmsg[EDITOR_STATUSMSG_LENGTH];
    int mode;

    char *last_search;
};

extern struct editorConfig E;

TextBuffer *createBuffer(const char *filename);
void deleteBuffer(TextBuffer *buf);

void initEditor(char *filename);

void editorInsertNewline(Window *W);
void editorDelChar(Window *W);
void editorDelNextChar(Window *W);
void editorInsertChar(Window *W, int c);
void editorIndentLine(Window *W);

char editorGetCharAtCursor(Window *W);
char editorGetCharBeforeCursor(Window *W);

#endif /* __EDITOR_H */
