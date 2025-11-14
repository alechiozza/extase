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
    "{}"
};

#define NUM_PAREN (sizeof(PARENTESIS)/sizeof(PARENTESIS[0]))

static void indentParentests(void)
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

void editorIMProcessKeypress(int key)
{
    switch (key)
    {
    case ENTER:
    case CTRL_J:
        if (E.auto_indent)
            indentParentests();
        else
            editorInsertNewline(E.active_win);
        break;
    case BACKSPACE:
    case CTRL_H:
        editorDelChar(E.active_win);
        break;
    case CTRL_ARROW_UP:
        editorScrollUp(E.active_win);
        break;
    case CTRL_ARROW_DOWN:
        editorScrollDown(E.active_win);
        break;
    case CTRL_ARROW_RIGHT:
        editorMoveCursorNextWord(E.active_win);
        break;
    case CTRL_ARROW_LEFT:
        editorMoveCursorPreviousWord(E.active_win);
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
        if (!isprint(key)) break;

        if (E.auto_paren)
        {
            for (size_t i = 0; i < NUM_PAREN; i++)
            {
                if (key == PARENTESIS[i][0])
                {
                    editorInsertChar(E.active_win, key);
                    editorInsertChar(E.active_win, PARENTESIS[i][1]);
                    editorMoveCursorLeft(E.active_win);
                    return;
                }
            }
        }

        editorInsertChar(E.active_win, key);
        break;
    }
}
