#ifndef __EDITOR_TERM_H
#define __EDITOR_TERM_H

enum KEY_ACTION
{
    KEY_INVALID = -1,
    KEY_NULL = 0,    /* NULL */
    CTRL_A = 1,
    CTRL_B = 2,
    CTRL_C = 3,      /* ETX, ignore */
    CTRL_D = 4,      /* EOT, ignore */
    CTRL_E = 5,
    CTRL_F = 6,      /* Find */
    CTRL_G = 7,
    CTRL_H = 8,      /* Backspace */
    TAB = 9,         /* Tab */
    CTRL_J = 10,     /* Newline */
    CTRL_K = 11,
    CTRL_L = 12,     /* Toggle line numbers */
    ENTER = 13,      /* Enter */
    CTRL_N = 14,
    CTRL_O = 15,     /* Open, TODO: implement */
    CTRL_P = 16,
    CTRL_Q = 17,
    CTRL_R = 18,
    CTRL_S = 19,     /* Save */
    CTRL_T = 20,
    CTRL_U = 21,
    CTRL_V = 22,
    CTRL_W = 23,    /* Switch to next window */
    CTRL_X = 24,
    CTRL_Y = 25,    /* Redo */
    CTRL_Z = 26,    /* Undo */
    ESC = 27,
    CTRL_BSLASH = 28,
    CTRL_RBRACKET = 29,
    CTRL_HAT = 30,
    CTRL_UNDERSCORE = 31,
    BACKSPACE = 127, /* Backspace */
    /* Soft codes */
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};

enum CursorMode
{
    CURSOR_DEFAULT  = 0,
    CURSOR_STEADY_BLOCK = 2,
    CURSOR_BLINK_UNDERLINE = 3,
    CURSOR_STEADY_UNDERLINE = 4,
    CURSOR_BLINK_BAR = 5,
    CURSOR_STEADY_BAR = 6
};

#define ESC_RESET_MODES         "0"
#define ESC_BOLD                "1"
#define ESC_DIM                 "2"
#define ESC_ITALIC              "3"
#define ESC_UNDERLINE           "4"
#define ESC_BLINKING            "5"
#define ESC_INVERSE             "7"

#define ESC_BOLD_DISABLE         "\x1b[22m"
#define ESC_DIM_DISABLE          "\x1b[22m"
#define ESC_ITALIC_DISABLE       "\x1b[23m"
#define ESC_UNDERLINE_DISABLE    "\x1b[24m"
#define ESC_BLINKING_DISABLE     "\x1b[25m"
#define ESC_INVERSE_MODE_DISABLE "\x1b[27m"

#define ESC_ERASE_LINE_END      "\x1b[0K"
#define ESC_ERASE_LINE_START    "\x1b[1K"
#define ESC_ERASE_LINE          "\x1b[2K"
#define ESC_ERASE_SCREEN        "\x1b[2J"

#define ESC_HIDE_CURSOR         "\x1b[?25l"
#define ESC_SHOW_CURSOR         "\x1b[?25h"
#define ESC_GET_CURSOR_POS      "\x1b[6n"

#define ESC_CURSOR_HOME         "\x1b[H"

#define ESC_ENABLE_ALT_SCREEN   "\x1b[?1049h"
#define ESC_DISABLE_ALT_SCREEN  "\x1b[?1049l"

int enableRawMode(int fd);
int getWindowSize(int ifd, int ofd, int *rows, int *cols);
int editorReadKey(int fd);

int setCursorMode(enum CursorMode mode);
void setCursorPosition(int fd, int row, int col);

#endif /* __EDITOR_TERM_H */
