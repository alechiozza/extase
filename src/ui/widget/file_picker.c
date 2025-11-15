#include "widget.h"

#include "utils.h"
#include "fb.h"
#include "utf8.h"
#include "term.h"
#include "editor.h"
#include "window.h"
#include "textbuffer.h"
#include "commands.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>

typedef enum
{
    FT_FILE,
    FT_DIR
} FileType;

typedef struct
{
    char *name;
    FileType type;
} FileEntry;

typedef struct FilePicker
{
    Window *window;
    char query[EDITOR_QUERY_LEN + 1];
    int qlen;
    char current_path[PATH_MAX + 1];

    FileEntry *files;
    int num_files;
    int selected_index;
} FilePicker;

static int compare_files(const void *a, const void *b)
{
    const FileEntry *fa = (const FileEntry *)a;
    const FileEntry *fb = (const FileEntry *)b;

    if (fa->type != fb->type)
    {
        return (fa->type == FT_DIR) ? -1 : 1;
    }

    return strcmp(fa->name, fb->name);
}

static void filePickerScanDirectory(FilePicker *picker)
{
    if (picker->files)
    {
        for (int i = 0; i < picker->num_files; i++)
        {
            free(picker->files[i].name);
        }
        free(picker->files);
        picker->files = NULL;
    }

    picker->num_files = 0;
    int capacity = 8;
    picker->files = malloc(capacity * sizeof(FileEntry));
    if (!picker->files)
    {
        // TODO: handle memory error
        return;
    }

    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(picker->current_path)) == NULL)
    {
        free(picker->files);
        picker->files = NULL;
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        if (picker->num_files >= capacity)
        {
            capacity *= 2;
            void *new_ptr = realloc(picker->files, capacity * sizeof(FileEntry));
            if (!new_ptr)
            {
                // TODO: handle memory error
                closedir(dir);
                return;
            }
            picker->files = new_ptr;
        }

        picker->files[picker->num_files].name = strdup(entry->d_name);
        if (entry->d_type == DT_DIR)
        {
            picker->files[picker->num_files].type = FT_DIR;
        }
        else
        {
            picker->files[picker->num_files].type = FT_FILE;
        }

        if (picker->files[picker->num_files].name)
        {
            picker->num_files++;
        }
    }
    closedir(dir);

    qsort(picker->files, picker->num_files, sizeof(FileEntry), compare_files);
}

static void filePickerUpdateSelection(FilePicker *picker)
{
    picker->selected_index = -1;
    if (picker->num_files == 0)
        return;

    if (picker->qlen == 0)
    {
        picker->selected_index = 0;
        return;
    }

    for (int i = 0; i < picker->num_files; i++)
    {
        if (strncmp(picker->files[i].name, picker->query, picker->qlen) == 0)
        {
            picker->selected_index = i;
            return;
        }
    }
}

static void filePickerSelectNext(FilePicker *picker)
{
    if (picker->num_files == 0)
        return;
    if (picker->selected_index == -1)
    {
        filePickerUpdateSelection(picker);
        return;
    }

    for (int i = picker->selected_index + 1; i < picker->num_files; i++)
    {
        if (strncmp(picker->files[i].name, picker->query, picker->qlen) == 0)
        {
            picker->selected_index = i;
            return;
        }
    }
}

static void filePickerSelectPrev(FilePicker *picker)
{
    if (picker->num_files == 0 || picker->selected_index <= 0)
        return;

    for (int i = picker->selected_index - 1; i >= 0; i--)
    {
        if (strncmp(picker->files[i].name, picker->query, picker->qlen) == 0)
        {
            picker->selected_index = i;
            return;
        }
    }
}

static void filePickerDraw(Widget *self, FrameBuffer *fb)
{
    FilePicker *picker = (FilePicker *)self->data;

    int path_len = strlen(picker->current_path);
    int max_path_len = self->width - 4;
    char *path_to_draw = picker->current_path;

    if (path_len > max_path_len)
    {
        path_to_draw += (path_len - max_path_len - 3);
        path_to_draw[0] = '.';
        path_to_draw[1] = '.';
        path_to_draw[2] = '.';
    }

    for (int x = 1; x < self->width - 1; x++)
    {
        fbPutCodepoint(fb, x + self->x, self->y, DOUBLEBOX_HORIZONTAL, STYLE_NORMAL);
        fbPutCodepoint(fb, x + self->x, self->y + 2, DOUBLEBOX_HORIZONTAL, STYLE_NORMAL);
        fbPutCodepoint(fb, x + self->x, self->y + self->height - 1, DOUBLEBOX_HORIZONTAL, STYLE_NORMAL);
    }
    fbDrawChars(fb, 2 + self->x, self->y, path_to_draw, strlen(path_to_draw), STYLE_NORMAL);

    for (int y = 1; y < self->height - 1; y++)
    {
        fbPutCodepoint(fb, self->x, y + self->y, DOUBLEBOX_VERTICAL, STYLE_NORMAL);
        for (int x = 1; x < self->width - 1; x++)
        {
            if (y != 2) 
                fbPutChar(fb, x + self->x, y + self->y, ' ', STYLE_NORMAL);
        }
        fbPutCodepoint(fb, self->x + self->width - 1, y + self->y, DOUBLEBOX_VERTICAL, STYLE_NORMAL);
    }

    fbPutCodepoint(fb, self->x, self->y, DOUBLEBOX_TOPLEFT, STYLE_NORMAL);
    fbPutCodepoint(fb, self->x + self->width - 1, self->y, DOUBLEBOX_TOPRIGHT, STYLE_NORMAL);
    fbPutCodepoint(fb, self->x, self->y + self->height - 1, DOUBLEBOX_BOTTOMLEFT, STYLE_NORMAL);
    fbPutCodepoint(fb, self->x + self->width - 1, self->y + self->height - 1, DOUBLEBOX_BOTTOMRIGHT, STYLE_NORMAL);

    fbDrawChars(fb, 2 + self->x, 1 + self->y, picker->query, picker->qlen, STYLE_NORMAL);

    int list_y = 3 + self->y;
    int list_x = 2 + self->x;
    int max_list_height = self->height - 4;
    int items_drawn = 0;
    char display_name[PATH_MAX + 2];

    for (int i = 0; i < picker->num_files; i++)
    {
        if (strncmp(picker->files[i].name, picker->query, picker->qlen) == 0)
        {
            if (items_drawn >= max_list_height)
                break;

            Style style = (i == picker->selected_index) ? STYLE_INVERSE : STYLE_NORMAL;

            char *filename = picker->files[i].name;
            int len = snprintf(display_name, PATH_MAX + 2, "%s", filename);

            if (picker->files[i].type == FT_DIR)
            {
                display_name[len++] = '/';
                display_name[len] = '\0';
            }

            int max_len = self->width - 4;
            if (len > max_len)
                len = max_len;

            fbDrawChars(fb, list_x, list_y + items_drawn, display_name, len, style);

            items_drawn++;
        }
    }
}

static void filePickerNavigateUp(FilePicker *picker)
{
    char *last_slash = strrchr(picker->current_path, '/');
    if (last_slash == NULL)
    {
        return;
    }

    if (last_slash == picker->current_path)
    {
        if (picker->current_path[1] != '\0')
        {
            picker->current_path[1] = '\0';
        }
    }
    else
    {
        *last_slash = '\0';
    }

    picker->qlen = 0;
    picker->query[0] = '\0';
    filePickerScanDirectory(picker);
    filePickerUpdateSelection(picker);
}

static int filePickerHandleInput(Widget *self, int key)
{
    FilePicker *picker = (FilePicker *)self->data;
    int query_changed = 0;

    switch (key)
    {
    case DEL_KEY:
    case CTRL_H:
    case BACKSPACE:
        if (picker->qlen != 0)
        {
            picker->query[picker->qlen - 1] = '\0';
            picker->qlen--;
            self->cx--;
            query_changed = 1;
        }
        else
        {
            filePickerNavigateUp(picker);
        }
        break;

    case ESC:
        return WIDGET_CLOSE;

    case ENTER:
    {
        FileEntry *selected_entry = NULL;
        if (picker->selected_index != -1)
        {
            selected_entry = &picker->files[picker->selected_index];
        }

        char full_path[PATH_MAX + 1];

        if (selected_entry)
        {
            snprintf(full_path, PATH_MAX, "%s/%s", picker->current_path, selected_entry->name);
            if (strcmp(picker->current_path, "/") == 0)
            {
                snprintf(full_path, PATH_MAX, "/%s", selected_entry->name);
            }

            if (selected_entry->type == FT_DIR)
            {
                strncpy(picker->current_path, full_path, PATH_MAX);
                picker->qlen = 0;
                picker->query[0] = '\0';
                filePickerScanDirectory(picker);
                filePickerUpdateSelection(picker);
                return WIDGET_CONTINUE;
            }

            if (strcmp(full_path, picker->window->buf->filename) == 0)
                return WIDGET_CLOSE;

            if (picker->window->buf->dirty)
            {
                /* open popup here */
            }

            editorOpen(picker->window, full_path);
        }
        else if (picker->qlen > 0)
        {
            snprintf(full_path, PATH_MAX, "%s/%s", picker->current_path, picker->query);
            if (strcmp(picker->current_path, "/") == 0)
            {
                snprintf(full_path, PATH_MAX, "/%s", picker->query);
            }
            editorOpen(picker->window, full_path);
        }
        return WIDGET_CLOSE;
    }

    case ARROW_DOWN:
        filePickerSelectNext(picker);
        break;

    case ARROW_UP:
        filePickerSelectPrev(picker);
        break;

    default:
        if (isprint(key))
        {
            if (picker->qlen < EDITOR_QUERY_LEN)
            {
                picker->query[picker->qlen++] = key;
                picker->query[picker->qlen] = '\0';
                self->cx++;
                query_changed = 1;
            }
        }
        break;
    }

    if (query_changed)
    {
        filePickerUpdateSelection(picker);
    }

    return WIDGET_CONTINUE;
}

static void filePickerDestroy(Widget *self)
{
    FilePicker *picker = (FilePicker *)self->data;

    if (picker->files)
    {
        for (int i = 0; i < picker->num_files; i++)
        {
            free(picker->files[i].name);
        }
        free(picker->files);
    }

    free(picker);
}

Widget *filePickerCreate(Window *W)
{
    FilePicker *picker = malloc(sizeof(FilePicker));
    picker->window = W;
    picker->qlen = 0;
    picker->query[0] = '\0';
    picker->files = NULL;
    picker->num_files = 0;
    picker->selected_index = -1;

    if (getcwd(picker->current_path, PATH_MAX) == NULL)
    {
        strcpy(picker->current_path, ".");
    }

    filePickerScanDirectory(picker);
    filePickerUpdateSelection(picker);

    Widget *widget = malloc(sizeof(Widget));
    widget->width = E.screencols / 2;
    widget->height = E.screenrows / 2;
    widget->x = (E.screencols - widget->width) / 2;
    widget->y = (E.screenrows - widget->height) / 2;
    widget->cx = 2;
    widget->cy = 1;

    widget->draw = &filePickerDraw;
    widget->handle_input = &filePickerHandleInput;
    widget->destroy = &filePickerDestroy;

    widget->data = picker;

    return widget;
}
