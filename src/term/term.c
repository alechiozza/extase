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
    /* Don't even check the return value as it's too late. */
    if (E.rawmode)
    {
        tcsetattr(fd, TCSAFLUSH, &orig_termios);
        E.rawmode = false;
    }
}

int setCursorMode(enum CursorMode mode)
{
    if (mode > 6) return -1;

    char seq[16];
    int len = snprintf(seq, sizeof(seq), "\x1b[%u q", mode);

    if (writen(STDOUT_FILENO, seq, len) != len)
        return -1;

    fflush(stdout);

    return 0;
}

void terminalPrint(int fd, const char *buf)
{
    writen(fd, buf, strlen(buf));
}

static void enableAltScreen(void)
{
    terminalPrint(STDOUT_FILENO, ESC_ENABLE_ALT_SCREEN);
}

static void disableAltScreen(void)
{
    terminalPrint(STDOUT_FILENO, ESC_DISABLE_ALT_SCREEN);
}

static void editorAtExit(void)
{
    disableRawMode(STDIN_FILENO);
    disableAltScreen();

    setCursorMode(CURSOR_DEFAULT);

    /* TODO: implement only for terminals without alt screen */
    if (0)
    {
        writen(STDOUT_FILENO, ESC_CURSOR_HOME, 4);
        writen(STDOUT_FILENO, ESC_ERASE_SCREEN, 4);
    }

    editorPrintFatalError();
}

/* Raw mode: 1960 magic shit. */
int enableRawMode(int fd)
{
    struct termios raw;

    if (E.rawmode)
        return 0; /* Already enabled. */
    if (!isatty(STDIN_FILENO))
        goto fatal;
    atexit(editorAtExit);
    if (tcgetattr(fd, &orig_termios) == -1)
        goto fatal;

    raw = orig_termios; /* modify the original mode */
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    raw.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);
    /* local modes - choing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer. */
    raw.c_cc[VMIN] = 0;  /* Return each byte, or zero for timeout. */
    raw.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second). */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(fd, TCSAFLUSH, &raw) < 0)
        goto fatal;

    enableAltScreen();

    E.rawmode = true;
    return 0;

fatal:
    errno = ENOTTY;
    return -1;
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

/* On success the position of the cursor is stored at *rows and 
   *cols and 0 is returned. -1 on error */
static int getCursorPosition(int ifd, int ofd, int *rows, int *cols)
{
    char buf[32];
    unsigned int i = 0;

    if (writen(ofd, ESC_GET_CURSOR_POS, 4) != 4)
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
            goto failed;

        /* Go to right/bottom margin */
        if (writen(ofd, "\x1b[999C\x1b[999B", 12) != 12)
            goto failed;
        if (getCursorPosition(ifd, ofd, rows, cols) == -1)
            goto failed;

        /* Restore position. */
        char seq[32];
        snprintf(seq, 32, "\x1b[%d;%dH", orig_row, orig_col);
        if (writen(ofd, seq, strlen(seq)) == -1)
        {
            /* Can't recover... */
        }
        return 0;
    }
    else
    {
        if (cols) *cols = ws.ws_col;
        if (rows) *rows = ws.ws_row;
        
        return 0;
    }

failed:
    return -1;
}
