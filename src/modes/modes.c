#include "modes.h"
#include "editor.h"

void editorProcessKeypress(int fd)
{
    switch (E.mode)
    {
        case NORMAL_MODE:
            editorNMProcessKeypress(fd);
            break;
        case INSERT_MODE:
            editorIMProcessKeypress(fd);
            break;
    }
}
