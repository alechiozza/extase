#include "ui.h"
#include "editor.h"
#include "event.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <locale.h>

int main(int argc, char **argv)
{
    setlocale(LC_CTYPE, "");
    
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
