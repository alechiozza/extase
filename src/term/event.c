#include "event.h"
#include "editor.h"

#include "modes.h"
#include "term.h"
#include "ui.h"
#include "window.h"
#include "fb.h"

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>

void updateWindowSize(void)
{
    if (getWindowSize(STDIN_FILENO, STDOUT_FILENO,
                      &E.screenrows, &E.screencols) == -1)
    {
        editorFatalError("Unable to query the screen for size (columns / rows)");
        exit(EXIT_FAILURE);
    }

    if (E.screencols < EDITOR_MIN_WIDTH || E.screenrows < EDITOR_MIN_HEIGHT)
    {
        E.too_small = true;
        return;
    }
    E.too_small = false;

    computeWindowLayout();

    fbResize(E.fb, E.screenrows, E.screencols);
}

void handleSigWinCh(int unused)
{
    (void)unused;

    updateWindowSize();
    
    if (E.too_small)
        editorTooSmallScreen();
    else
        editorRefreshScreen();
}

void editorSetStatusMessage(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
}

// TODO: fix this shitty error reporting system
char errorbuffer[1024] = {0};

void editorFatalError(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(errorbuffer, sizeof(errorbuffer), fmt, ap);
    va_end(ap);
}

void editorPrintFatalError(void)
{
    fprintf(stderr, "%s", errorbuffer);
}

