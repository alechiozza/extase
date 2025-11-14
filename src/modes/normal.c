#include "modes.h"

#include "editor.h"
#include "term.h"
#include "cursor.h"
#include "commands.h"
#include "event.h"
#include "ui.h"
#include "window.h"

#include <unistd.h>

void editorSetNormalMode(void)
{
    E.mode = NORMAL_MODE;
    editorSetStatusMessage(" -- NORMAL --");
}

static void processWindowMode(int fd)
{
    int c = editorReadKey(fd);
    switch (c)
    {
        case '+':
        case '=': /* us keyboard sucks */
            editorResizeWindow(E.active_win, 0.1f, false);
            break;
        case '-':
            editorResizeWindow(E.active_win, -0.1f, false);
            break;
        case '>':
            editorResizeWindow(E.active_win, 0.1f, true);
            break;
        case '<':
            editorResizeWindow(E.active_win, -0.1f, true);
            break;
        case ARROW_LEFT:
        case 'h':
            editorSwitchWindow(DIR_LEFT);
            break;
        case ARROW_DOWN:
        case 'j':
            editorSwitchWindow(DIR_DOWN);
            break;
        case ARROW_UP:
        case 'k':
            editorSwitchWindow(DIR_UP);
            break;
        case ARROW_RIGHT:
        case 'l':
            editorSwitchWindow(DIR_RIGHT);
            break;
        case 'q':
            editorQuit(E.active_win->buf, STDIN_FILENO);
            break;
        case 'v':
            editorSplitWindow(SPLIT_VERTICAL);
            break;
        case 's':
            editorSplitWindow(SPLIT_HORIZONTAL);
            break;
    }
}

void editorNMProcessKeypress(int key)
{
    switch (key)
    {
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
    case CTRL_Q:
        editorQuit(E.active_win->buf, STDIN_FILENO);
        break;
    case CTRL_S:
        editorSave(E.active_win->buf);
        break;
    case CTRL_F:
        editorFind(E.active_win, STDIN_FILENO);
        break;
    case CTRL_L:
        editorToggleLinenum();
        break;
    case CTRL_O:
        editorOpenFromWin(E.active_win, STDIN_FILENO);
        break;
    case PAGE_UP:
        editorMoveCursorPageUp(E.active_win);
        break;
    case PAGE_DOWN:
        editorMoveCursorPageDown(E.active_win);
        break;
    case ARROW_LEFT:
    case 'h':
        editorMoveCursorLeft(E.active_win);
        break;
    case ARROW_RIGHT:
    case 'l':
        editorMoveCursorRight(E.active_win);
        break;
    case ARROW_UP:
    case 'k':
        editorMoveCursorUp(E.active_win);
        break;
    case ARROW_DOWN:
    case 'j':
        editorMoveCursorDown(E.active_win);
        break;
    case TAB:
        break;
    case CTRL_W:
        processWindowMode(STDIN_FILENO);
        break;
    case 'i':
    case 'I':
        editorSetInsertMode();
        break;
    case 'r':
        E.relativenums = !E.relativenums;
        break;
    case 'o':
        editorSetInsertMode();
        editorInsertNewline(E.active_win);
        break;
    case '0':
        editorMoveCursorLineStart(E.active_win);
        break;
    case '$':
        editorMoveCursorLineEnd(E.active_win);
        break;
    case ':':
        editorShell(STDIN_FILENO);
        break;
    default:
        break;
    }
}
