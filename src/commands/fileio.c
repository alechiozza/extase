#include "commands.h"

#include "editor.h"
#include "term.h"
#include "event.h"
#include "ui.h"
#include "window.h"
#include "core.h"
#include "utils.h"
#include "syntax.h"
#include "cursor.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>

static bool isBinaryFile(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) return false;

    unsigned char buf[512];
    size_t flen = fread(buf, 1, sizeof(buf), fp);
    fclose(fp);

    for (size_t i = 0; i < flen; i++)
    {
        unsigned char c = buf[i];
        if (!(isprint(c) || c == TAB || c == 10 || c == 13)) /* TODO: shitty check*/
            return true;
    }

    return false;
}

static TextBuffer *findOpenBuffer(const char *filename)
{
    for (size_t i = 0; i < E.num_buf; i++)
    {
        if (E.buf[i]->filename == NULL) continue;
        if (strcmp(E.buf[i]->filename, filename) == 0)
        {
            return E.buf[i];
        }
    }
    return NULL;
}

int editorOpen(Window *W, const char *filename)
{
    TextBuffer *existing = findOpenBuffer(filename);
    if (existing != NULL && existing == W->buf)
    {
        return 0;
    }

    // TODO: handle binary file properly
    if (isBinaryFile(filename))
    {
        editorFatalError("Binary file shit\n");
        exit(EXIT_FAILURE);
    }

    if (W->buf != NULL)
        deleteWindowBuf(W);

    editorCursorReset(W);

    if (existing != NULL)
    {
        W->buf = existing;
        return 0;
    }

    TextBuffer *buf = createBuffer(filename);
    if (buf == NULL)
    {
        exit(EXIT_FAILURE); // TODO: meh
    }

    W->buf = buf;

    editorSelectSyntaxHighlight(W->buf, filename);

    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        if (errno == ENOENT)
        {
            /* Open a new file */   
            return 0;
        }
        else
        {
            editorFatalError("Error opening file %s: %s\n", filename, strerror(errno));
            return -1;
        }
    }

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fp)) != -1)
    {
        while (linelen > 0 && ((line[linelen - 1] == '\n' || line[linelen - 1] == '\r')))
        {
            linelen--;
        }
        line[linelen] = '\0';
        editorInsertRow(buf, buf->numrows, line, linelen);
    }

    buf->dirty = false;
    
    free(line);
    fclose(fp);

    return 0;
}

// TODO: implement
void command_handler_open(int fd, int argc, char **argv)
{
    (void)fd;
    (void)argc;
    (void)argv;
    //editorOpen(E.active_win, idk);
}

static char *editorRowsToString(TextBuffer *buf, int *buflen)
{
    char *strbuf = NULL;
    int totlen = 0;
    int j;

    for (j = 0; j < buf->numrows; j++)
    {
        totlen += buf->rows[j].size + 1; /* +1 for '\0' */
    }

    *buflen = totlen;
    totlen++; /* Make space for nulterm */

    strbuf = malloc(totlen);
    if (!strbuf)
    {
        return NULL;
    }

    char *p = strbuf;
    for (j = 0; j < buf->numrows; j++)
    {
        memcpy(p, buf->rows[j].chars, buf->rows[j].size);
        p += buf->rows[j].size;
        *p = '\n'; /* overwrite '\0' with '\n' */
        p++;
    }
    *p = '\0';

    return strbuf;
}

int editorSave(TextBuffer *buf)
{
    int fd = open(buf->filename, O_RDWR | O_CREAT, 0644); // 0644 = (rw-r--r--)
    if (fd == -1)
    {
        editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
        return 1;
    }

    int len;
    char *strbuf = editorRowsToString(buf, &len);
    if (!strbuf)
    {
        editorSetStatusMessage("Not enough memory!");
        return 1;
    }

    /* TODO: improve this for safety reasons... */
    if (ftruncate(fd, len) == -1)
        goto writeerr;
    if (writen(fd, strbuf, len) == -1)
        goto writeerr;

    close(fd);
    free(strbuf);
    buf->dirty = false;
    editorSetStatusMessage("\"%s\" saved, %d bytes written on disk", buf->filename, len);
    return 0;

writeerr:
    free(strbuf);
    if (fd != -1)
        close(fd);
    editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
    return 1;
}

void command_handler_save(int fd, int argc, char **argv)
{
    (void)fd;
    
    if (argc != 0)
    {
        free(E.active_win->buf->filename);
        E.active_win->buf->filename = strdup(argv[0]);
    }

    editorSave(E.active_win->buf);
}

int editorSaveAs(TextBuffer *buf, int fd)
{
    char query[EDITOR_QUERY_LEN + 1] = {0};
    int qlen = 0;

    while (1)
    {
        editorSetStatusMessage("Type the name of the file: %s", query);
        editorRefreshScreen();

        int c = editorReadKey(fd);

        if (c == DEL_KEY || c == CTRL_H || c == BACKSPACE)
        {
            if (qlen != 0)
                query[--qlen] = '\0';
        }
        else if (c == ESC)
        {
            editorSetStatusMessage("");
            return 0;
        }
        else if (c == ENTER)
        {
            if (query[0] != '\0')
            {
                free(buf->filename);
                buf->filename = strdup(query);
                return editorSave(buf);
            }
        }
        else if (isprint(c))
        {
            if (qlen < EDITOR_QUERY_LEN)
            {
                query[qlen++] = c;
                query[qlen] = '\0';
            }
        }
    }
}

// TODO: implement autosave

