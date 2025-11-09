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
        editorInsertNewline(E.win[E.active_win]);
        break;
    case BACKSPACE:
    case CTRL_H:
        editorDelChar(E.win[E.active_win]);
        break;
    case CTRL_K:
        editorScrollDown(E.win[E.active_win]);
        break;
    case DEL_KEY:
        editorDelNextChar(E.win[E.active_win]);
        break;
    case PAGE_UP:
        editorMoveCursorPageUp(E.win[E.active_win]);
        break;
    case PAGE_DOWN:
        editorMoveCursorPageDown(E.win[E.active_win]);
        break;
    case ARROW_LEFT:
        editorMoveCursorLeft(E.win[E.active_win]);
        break;
    case ARROW_RIGHT:
        editorMoveCursorRight(E.win[E.active_win]);
        break;
    case ARROW_UP:
        editorMoveCursorUp(E.win[E.active_win]);
        break;
    case ARROW_DOWN:
        editorMoveCursorDown(E.win[E.active_win]);
        break;
    case TAB:
        editorInsertTab(E.win[E.active_win]);
        break;
    case CTRL_W:
        editorSwitchWindow();
        break;
    case ESC:
        editorSetNormalMode();
        break;
    default:
        if (isprint(c))
            editorInsertChar(E.win[E.active_win], c);
        break;
    }
}
