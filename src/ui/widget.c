#include "widget.h"

#include "editor.h"

#include <stdlib.h>
#include <string.h>

void createWidget(Widget *w)
{
    if (E.num_widget == EDITOR_MAX_WIDG) return;

    E.widgets[E.num_widget] = w;
    E.num_widget++;

    E.active_widget = w;
}

void deleteWidget(Widget *w)
{
    if (w->destroy != NULL)
    {
        w->destroy(w);
    }

    int found_idx = -1;
    for (size_t i = 0; i < E.num_widget; i++)
    {
        if (E.widgets[i] == w)
        {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1)
    {
        exit(EXIT_FAILURE);
    }

    free(w);

    int remaining_elements = E.num_widget - 1 - found_idx;

    if (remaining_elements > 0)
    {
        memmove(&E.widgets[found_idx], &E.widgets[found_idx + 1], sizeof(Widget *) * remaining_elements);
    }

    E.num_widget--;
    E.widgets[E.num_widget] = NULL;

    E.active_widget = NULL;
}
