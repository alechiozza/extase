#include "commands.h"

#include "editor.h"
#include "term.h"
#include "syntax.h"
#include "ui.h"
#include "event.h"
#include "window.h"
#include "core.h"
#include "cursor.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

typedef enum FindDirection
{
    FIND_NEXT = 1,
    FIND_PREVIOUS = -1,
    STAY_STILL = 0
} FindDirection;

typedef struct Match
{
    int x, y;
} Match;

#define NO_MATCH (Match){-1,-1}

static void restoreHL(Row *row, char **saved_hl)
{
    if (*saved_hl)
    {
        memcpy(row->hl, *saved_hl, row->rsize);
        free(*saved_hl);
        *saved_hl = NULL;
    }     
}

static Match findForward(Window *W, const char *query, Match start_pos)
{
    TextBuffer *buf = W->buf;

    if (start_pos.y == -1)
        return NO_MATCH;

    // Check for a match on the *current line* after the cursor
    Row *row = &buf->rows[start_pos.y];
    char *found = NULL;
    if (start_pos.x + 1 < row->rsize)
    {
        found = strstr(row->render + start_pos.x + 1, query);
    }

    if (found)
    {
        return (Match){(int)(found - row->render), start_pos.y};
    }

    // If not found, search all subsequent lines
    for (int i = 1; i < buf->numrows; i++)
    {
        int next_y = (start_pos.y + i) % buf->numrows;
        row = &buf->rows[next_y];
        found = strstr(row->render, query);
        if (found)
        {
            return (Match){(int)(found - row->render), next_y};
        }
    }

    return NO_MATCH;
}

static Match findBackward(Window *W, const char *query, Match start_pos)
{
    TextBuffer *buf = W->buf;
    if (start_pos.y == -1)
        return NO_MATCH;

    Row *row = &buf->rows[start_pos.y];
    char *found = NULL;
    char *last_match_on_line = NULL;
    char *current_pos = row->render;

    // Check for matches on the *current line* before the cursor
    while ((found = strstr(current_pos, query)) != NULL)
    {
        if (found - row->render < start_pos.x)
        {
            last_match_on_line = found;
            current_pos = found + 1;
        }
        else
        {
            break;
        }
    }

    if (last_match_on_line)
    {
        return (Match){(int)(last_match_on_line - row->render), start_pos.y};
    }

    // If not, search all previous lines
    for (int i = 1; i < buf->numrows; i++)
    {
        int prev_y = (start_pos.y - i + buf->numrows) % buf->numrows;
        row = &buf->rows[prev_y];

        // Find the *last* match on this line
        last_match_on_line = NULL;
        current_pos = row->render;
        while ((found = strstr(current_pos, query)) != NULL)
        {
            last_match_on_line = found;
            current_pos = found + 1;
        }

        if (last_match_on_line)
        {
            return (Match){(int)(last_match_on_line - row->render), prev_y};
        }
    }

    return NO_MATCH;
}

void editorFind(Window *W, int fd)
{
    TextBuffer *buf = W->buf;

    int saved_cx = W->cx, saved_cy = W->cy;
    int saved_coloff = W->viewport.coloff, saved_rowoff = W->viewport.rowoff;

    char query[EDITOR_QUERY_LEN + 1] = {0};
    int qlen = 0;

    if (E.last_search)
    {
        strncpy(query, E.last_search, EDITOR_QUERY_LEN);
        qlen = strlen(query);
    }

    Match match = NO_MATCH;
    Match search_start_pos = (Match){saved_cx, saved_cy};
    FindDirection d = STAY_STILL;
    
    int saved_hl_line = -1;
    char *saved_hl = NULL;
    
    const char *prompt_prefix = "Search: ";

    if (buf->numrows == 0)
    {
        editorSetStatusMessage("Empty file, nothing to find!");
        return;
    }

    while (1)
    {
        editorSetStatusMessage("%s%s (ESC=Cancel, ENTER=Confirm, Arrows=Navigate)", 
                               prompt_prefix, query);
        
        editorRefreshScreen();

        // 4. Manually move physical cursor to the prompt in the status bar
        // We assume status bar is at 1-based screen row E.screenrows
        int prompt_screen_y = E.screenrows;
        int prompt_screen_x = (int)strlen(prompt_prefix) + 1; 
        setCursorPosition(STDOUT_FILENO, prompt_screen_y, prompt_screen_x + qlen);
        
        int c = editorReadKey(fd);

        switch (c)
        {
        case DEL_KEY:
        case CTRL_H:
        case BACKSPACE:
            if (qlen != 0)
                query[--qlen] = '\0';
            
            // Reset search
            match = NO_MATCH;
            search_start_pos = (Match){saved_cx, saved_cy}; // Start from original pos
            d = FIND_NEXT; // Trigger a new search

            restoreHL(saved_hl_line >= 0 ? &buf->rows[saved_hl_line] : NULL, &saved_hl);
            saved_hl_line = -1;
            break;
            
        case ESC:
            W->cx = saved_cx;
            W->cy = saved_cy;
            W->viewport.coloff = saved_coloff;
            W->viewport.rowoff = saved_rowoff;
            
            restoreHL(saved_hl_line >= 0 ? &buf->rows[saved_hl_line] : NULL, &saved_hl);
            editorSetStatusMessage("");
            return;
            
        case ENTER:
            if (match.y == -1)
            {
                W->cx = saved_cx;
                W->cy = saved_cy;
                W->viewport.coloff = saved_coloff;
                W->viewport.rowoff = saved_rowoff;
                
                restoreHL(saved_hl_line >= 0 ? &buf->rows[saved_hl_line] : NULL, &saved_hl);
                editorSetStatusMessage("");

                return;
            }

            if (qlen > 0) 
            {
                free(E.last_search);
                E.last_search = strdup(query);
            }
            restoreHL(saved_hl_line >= 0 ? &buf->rows[saved_hl_line] : NULL, &saved_hl);
            
            editorSetStatusMessage("");

            return;
            
        case ARROW_RIGHT:
        case ARROW_DOWN:
            d = FIND_NEXT;
            // Start next search from the last match
            if (match.y != -1) search_start_pos = match;
            break;
            
        case ARROW_LEFT:
        case ARROW_UP:
            d = FIND_PREVIOUS;
            // Start prev search from the last match
            if (match.y != -1) search_start_pos = match;
            break;
            
        default:
            if (isprint(c))
            {
                if (qlen < EDITOR_QUERY_LEN)
                {
                    query[qlen++] = c;
                    query[qlen] = '\0';

                    match = NO_MATCH;
                    search_start_pos = (Match){saved_cx, saved_cy};
                    d = FIND_NEXT;
                    restoreHL(saved_hl_line >= 0 ? &buf->rows[saved_hl_line] : NULL, &saved_hl);
                    saved_hl_line = -1;
                }
            }
            else
            {
                d = STAY_STILL;
            }
            break;
        }

        if (qlen == 0 || d == STAY_STILL)
        {
            continue;
        }
        
        // Restore highlight from *previous* match before finding next
        restoreHL(saved_hl_line >= 0 ? &buf->rows[saved_hl_line] : NULL, &saved_hl);
        saved_hl_line = -1;

        if (d == FIND_NEXT)
        {
            match = findForward(W, query, search_start_pos);
        }
        else if (d == FIND_PREVIOUS)
        {
            match = findBackward(W, query, search_start_pos);
        }

        if (match.y != -1)
        {
            Row *row = &buf->rows[match.y];
    
            saved_hl_line = match.y;
            saved_hl = malloc(row->rsize);
            memcpy(saved_hl, row->hl, row->rsize);
            memset(row->hl + match.x, HL_MATCH, strlen(query));

            W->cy = match.y;
            W->viewport.rowoff = 0;
            W->cx = match.x;
            W->viewport.coloff = 0;

            editorCenterCursor(W);
        }

        // Reset direction so we don't loop forever
        d = STAY_STILL;
    }
}

void command_handler_find(int fd, int argc, char **argv)
{
    if (argc != 0)
    {
        free(E.last_search);
        E.last_search = strdup(argv[0]);
    }

    editorFind(E.win[E.active_win], fd);
}

// static void editorReplaceWith(const char *query)
// {

// }

// void editorReplace(void)
// {
//     char query[EDITOR_QUERY_LEN + 1] = {0};
//     int qlen = 0;

//     while (1)
//     {
//         editorSetStatusMessage(
//             "Type the characters to replace: %s",
//             query);
//         editorRefreshScreen();

//         int c = editorReadKey(STDIN_FILENO);

//         if (c == DEL_KEY || c == CTRL_H || c == BACKSPACE)
//         {
//             if (qlen != 0)
//                 query[--qlen] = '\0';
//         }
//         else if (c == ESC)
//         {
//             editorSetStatusMessage("");
//             return;
//         }
//         else if (c == ENTER)
//         {
//             if (qlen != 0)
//                 editorReplaceWith(query);
//             else
//                 editorSetStatusMessage("");
//             return;
//         }
//         else if (isprint(c))
//         {
//             if (qlen < EDITOR_QUERY_LEN)
//             {
//                 query[qlen++] = c;
//                 query[qlen] = '\0';
//             }
//         }
//     }
// }

