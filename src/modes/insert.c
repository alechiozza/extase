#include "modes.h"

#include "editor.h"
#include "term.h"
#include "cursor.h"
#include "event.h"
#include "window.h"

#include <ctype.h>

void editorSetInsertMode(void)
{
    E.mode = INSERT_MODE;
    editorSetStatusMessage(" -- INSERT --");
}

static char *PARENTESIS[] = {
    "()",
    "[]",
    "{}",
    "\"\"",
    "''"
};

#define NUM_PAREN (sizeof(PARENTESIS)/sizeof(PARENTESIS[0]))

void editorIMProcessKeypress(int fd)
{
    int c = editorReadKey(fd);
    switch (c)
    {
    case ENTER:
    case CTRL_J:
        if (E.auto_indent)
        {
            char current = editorGetCharAtCursor(E.active_win);
            char previous = editorGetCharBeforeCursor(E.active_win);

            editorInsertNewline(E.active_win);

            for (size_t i = 0; i < NUM_PAREN; i++)
            {
                if (previous == PARENTESIS[i][0])
                {
                    editorIndentLine(E.active_win);
                    if (current == PARENTESIS[i][1])
                    {
                        editorInsertNewline(E.active_win);
                        editorDelChar(E.active_win);
                        editorMoveCursorUp(E.active_win);
                        editorMoveCursorLineEnd(E.active_win);
                    }
                }
            }
        }
        else
        {

        }
        break;
    case BACKSPACE:
    case CTRL_H:
        editorDelChar(E.active_win);
        break;
    case CTRL_K:
        editorScrollDown(E.active_win);
        break;
    case DEL_KEY:
        editorDelNextChar(E.active_win);
        break;
    case PAGE_UP:
        editorMoveCursorPageUp(E.active_win);
        break;
    case PAGE_DOWN:
        editorMoveCursorPageDown(E.active_win);
        break;
    case ARROW_LEFT:
        editorMoveCursorLeft(E.active_win);
        break;
    case ARROW_RIGHT:
        editorMoveCursorRight(E.active_win);
        break;
    case ARROW_UP:
        editorMoveCursorUp(E.active_win);
        break;
    case ARROW_DOWN:
        editorMoveCursorDown(E.active_win);
        break;
    case TAB:
        editorIndentLine(E.active_win);
        break;
    case ESC:
        editorSetNormalMode();
        break;
    default:
        if (!isprint(c)) break;

        if (E.auto_paren)
        {
            for (size_t i = 0; i < NUM_PAREN; i++)
            {
                if (c == PARENTESIS[i][0])
                {
                    editorInsertChar(E.active_win, c);
                    editorInsertChar(E.active_win, PARENTESIS[i][1]);
                    editorMoveCursorLeft(E.active_win);
                    return;
                }
            }
        }

        editorInsertChar(E.active_win, c);
        break;
    }
}
