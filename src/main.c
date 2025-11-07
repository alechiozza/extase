#include "ui.h"
#include "editor.h"
#include "event.h"
#include "term.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: editor <filename>\n");
        exit(EXIT_FAILURE);
    }

    initEditor(argv[1]);

    while (1)
    {
        if (!E.too_small)
        {
            editorRefreshScreen();
            editorProcessKeypress(STDIN_FILENO);
        }
    }

    return 0;
}
