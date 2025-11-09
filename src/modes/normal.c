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
    editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find | ':' = command shell");
}

static void processWindowMode(int fd)
{
    int c = editorReadKey(fd);
    switch (c)
    {
        case '+':
            /* TODO: change window height */
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
            editorQuit(E.active_win->buf, fd);
            break;
        case '|':
            editorSplitWindow(SPLIT_VERTICAL);
            break;
        case '-':
            editorSplitWindow(SPLIT_HORIZONTAL);
            break;
    }
}

void editorNMProcessKeypress(int fd)
{
    int c = editorReadKey(fd);
    switch (c)
    {
    case CTRL_C:
    case CTRL_D:
        /* We ignore, it can't be so simple to lose the changes
         * to the edited file. */
        break;
    case CTRL_K:
        editorScrollDown(E.active_win);    
        break;
    case CTRL_Q:
        editorQuit(E.active_win->buf, fd);
        break;
    case CTRL_S:
        editorSave(E.active_win->buf);
        break;
    case CTRL_F:
        editorFind(E.active_win, fd);
        break;
    case CTRL_L:
        editorToggleLinenum();
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
        processWindowMode(fd);
        break;
    case 'i':
    case 'I':
        editorSetInsertMode();
        break;
    case 'r':
        E.relativenums = !E.relativenums;
        break;
    case ':':
        editorShell(STDIN_FILENO);
        break;
    default:
        break;
    }
}
