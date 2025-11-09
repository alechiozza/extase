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

void editorIMProcessKeypress(int fd)
{
    int c = editorReadKey(fd);
    switch (c)
    {
    case ENTER:
    case CTRL_J:
        editorInsertNewline(E.active_win);
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
        editorInsertTab(E.active_win);
        break;
    case ESC:
        editorSetNormalMode();
        break;
    default:
        if (isprint(c))
            editorInsertChar(E.active_win, c);
        break;
    }
}
