#ifndef __EDITOR_H
#define __EDITOR_H

#define EDITOR_VERSION "0.0.1"

#define TOPBAR_SIZE 1
#define TAB_VIEW_SIZE 1
#define INFOBAR_SIZE 1
#define SCROLLBAR_SIZE 1

#define MIN_LNUM_WIDTH 3

#define EDITOR_QUERY_LEN 128
#define EDITOR_STATUSMSG_LENGTH 256

#define EDITOR_MIN_WIDTH 40
#define EDITOR_MIN_HEIGHT 6

#include <stdbool.h>
#include "color.h"

typedef struct TextBuffer TextBuffer;
typedef struct Window Window;
typedef struct FrameBuffer FrameBuffer;
typedef struct Widget Widget;

#define EDITOR_MAX_WIN 10
#define EDITOR_MAX_WIDG 10

struct editorConfig
{
    int screenrows;
    int screencols;
    ColorMode color_mode;

    FrameBuffer *fb;

    Window *win[EDITOR_MAX_WIN];
    Window *active_win;
    size_t num_win;

    Widget *widgets[EDITOR_MAX_WIDG];
    Widget *active_widget;
    size_t num_widget;

    bool linenums;
    bool relativenums; /* Enable line numbers relative numeration */
    bool rawmode;    /* Is terminal raw mode enabled? */
    bool too_small;  /* The screen is too small to be rendered */
    bool auto_paren;
    bool auto_indent;
    bool no_topbar;
    char statusmsg[EDITOR_STATUSMSG_LENGTH];
    int mode;

    char *last_search;
};

extern struct editorConfig E;

void initEditor(char *file_path);

void editorInsertNewline(Window *W);
void editorDelLine(Window *W);
void editorDelLineTo(Window *W);
void editorDelLineFrom(Window *W);
void editorDelChar(Window *W);
void editorDelNextChar(Window *W);
void editorInsertChar(Window *W, int c);
void editorIndentLine(Window *W);

char editorGetCharAtCursor(Window *W);
char editorGetCharBeforeCursor(Window *W);

#endif /* __EDITOR_H */
