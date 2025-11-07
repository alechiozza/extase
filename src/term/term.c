#include "term.h"

#include "editor.h"
#include "event.h"
#include "utils.h"

#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static struct termios orig_termios;

static void disableRawMode(int fd)
{
    if (E.rawmode)
    {
        tcsetattr(fd, TCSAFLUSH, &orig_termios);
        E.rawmode = false;
    }
}

int setCursorMode(enum CursorMode mode)
{
    if (mode > 6)
        return -1;

    char seq[16];
    int len = snprintf(seq, sizeof(seq), "\x1b[%u q", mode);

    if (writen(STDOUT_FILENO, seq, len) == -1)
        return -1;

    return 0;
}

void setCursorPosition(int fd, int row, int col)
{
    char seq[32];
    int len;

    len = snprintf(seq, sizeof(seq), "\x1b[%d;%dH", row, col);

    if (len < 0 || (size_t)len >= sizeof(seq))
    {
        editorFatalError("Fatal: snprintf buffer overflow during cursor set\n");
        exit(EXIT_FAILURE);
    }

    if (writen(fd, seq, len) == -1)
    {
        editorFatalError("Fatal: Write failed during cursor set\n");
        exit(EXIT_FAILURE);
    }
}

ssize_t termPrint(int fd, const char *buf)
{
    return writen(fd, buf, strlen(buf));
}

static void enableAltScreen(void)
{
    termPrint(STDOUT_FILENO, ESC_ENABLE_ALT_SCREEN);
}

static void disableAltScreen(void)
{
    termPrint(STDOUT_FILENO, ESC_DISABLE_ALT_SCREEN);
}

static void editorAtExit(void)
{
    disableRawMode(STDIN_FILENO);
    disableAltScreen();

    setCursorMode(CURSOR_DEFAULT);

    /* TODO: implement only for terminals without alt screen */
    if (0)
    {
        termPrint(STDOUT_FILENO, ESC_CURSOR_HOME);
        termPrint(STDOUT_FILENO, ESC_ERASE_SCREEN);
    }

    editorPrintFatalError();
}

int enableRawMode(int fd)
{
    struct termios raw;

    if (E.rawmode) return 0;

    if (!isatty(fd))
    {
        errno = ENOTTY;
        return -1;
    }

    atexit(editorAtExit);

    if (tcgetattr(fd, &orig_termios) == -1)
        return -1;

    raw = orig_termios;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSAFLUSH, &raw) < 0)
        return -1;

    enableAltScreen();

    E.rawmode = true;
    
    return 0;
}

static int parseEsc(int fd)
{
    char seq[3];

    if (read(fd, seq, 1) == 0)
        return ESC;
    if (read(fd, seq + 1, 1) == 0)
        return ESC;

    if (seq[0] == '[')
    {
        if (seq[1] >= '0' && seq[1] <= '9')
        {
            /* Extended escape, read additional byte. */
            if (read(fd, seq + 2, 1) == 0)
                return ESC;
            if (seq[2] == '~')
            {
                switch (seq[1])
                {
                case '3':
                    return DEL_KEY;
                case '5':
                    return PAGE_UP;
                case '6':
                    return PAGE_DOWN;
                }
            }
        }
        else
        {
            switch (seq[1])
            {
            case 'A':
                return ARROW_UP;
            case 'B':
                return ARROW_DOWN;
            case 'C':
                return ARROW_RIGHT;
            case 'D':
                return ARROW_LEFT;
            case 'H':
                return HOME_KEY;
            case 'F':
                return END_KEY;
            }
        }
    }
    else if (seq[0] == 'O')
    {
        switch (seq[1])
        {
        case 'H':
            return HOME_KEY;
        case 'F':
            return END_KEY;
        }
    }

    return KEY_INVALID;
}

int editorReadKey(int fd)
{
    int nread;
    char c;
    while ((nread = read(fd, &c, 1)) == 0);
    if (nread == -1)
    {
        editorFatalError("Unable to read key\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        switch (c)
        {
        case ESC:
            return parseEsc(fd);

        default:
            return c;
        }
    }
}

static int getCursorPosition(int ifd, int ofd, int *rows, int *cols)
{
    char buf[32];
    unsigned int i = 0;

    if (termPrint(ofd, ESC_GET_CURSOR_POS) == -1)
        return -1;

    /* Read the response: ESC[#;#R */
    while (i < sizeof(buf) - 1)
    {
        if (read(ifd, buf + i, 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
        i++;
    }
    buf[i] = '\0';

    /* Parse it. */
    if (buf[0] != ESC || buf[1] != '[')
        return -1;
    if (sscanf(buf + 2, "%d;%d", rows, cols) != 2)
        return -1;

    return 0;
}

int getWindowSize(int ifd, int ofd, int *rows, int *cols)
{
    struct winsize ws;

    if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        /* ioctl() failed. Try to query the terminal itself. */
        int orig_row, orig_col;

        if (getCursorPosition(ifd, ofd, &orig_row, &orig_col) == -1)
            return -1;

        /* Go to right/bottom margin */
        if (termPrint(ofd, "\x1b[999C\x1b[999B") == -1)
            return -1;
        if (getCursorPosition(ifd, ofd, rows, cols) == -1)
            return -1;

        setCursorPosition(ofd, orig_row, orig_col);
    }
    else
    {
        if (cols)
            *cols = ws.ws_col;
        if (rows)
            *rows = ws.ws_row;
    }

    return 0;
}
