#include "popup.h"

#include "widget.h"
#include "editor.h"
#include "fb.h"
#include "utf8.h"
#include "term.h"

#include <string.h>
#include <stdlib.h>

static void popupDraw(Framebuffer *fb, Widget *self)
{
    PopupWindow * popup = (PopupWindow*)self->data;
    int title_len = strlen(popup->title);

    for (int x = 1; x < self->width-1; x++)
    {
        if (x >= 2 && x-2 < title_len)
            fbPutChar(fb, x+self->x, self->y, popup->title[x-2], STYLE_NORMAL);
        else
            fbPutCodepoint(fb, x+self->x, self->y, DOUBLEBOX_HORIZONTAL, STYLE_NORMAL);
        
        fbPutCodepoint(fb, x+self->x, self->y+self->height-1, DOUBLEBOX_HORIZONTAL, STYLE_NORMAL);
    }

    for (int y = 1; y < self->height-1; y++)
    {
        fbPutCodepoint(fb, self->x, y+self->y, DOUBLEBOX_VERTICAL, STYLE_NORMAL);
        for (int x = 1; x < self->width-1; x++)
        {
            fbPutChar(fb, x+self->x, y+self->y, ' ', STYLE_NORMAL);
        }
        fbPutCodepoint(fb, self->x+self->width-1, y+self->y, DOUBLEBOX_VERTICAL, STYLE_NORMAL);
    }

    /* Draw corners */
    fbPutCodepoint(fb, self->x, self->y, DOUBLEBOX_TOPLEFT, STYLE_NORMAL);
    fbPutCodepoint(fb, self->x+self->width-1, self->y, DOUBLEBOX_TOPRIGHT, STYLE_NORMAL);
    fbPutCodepoint(fb, self->x, self->y+self->height-1, DOUBLEBOX_BOTTOMLEFT, STYLE_NORMAL);
    fbPutCodepoint(fb, self->x+self->width-1, self->y+self->height-1, DOUBLEBOX_BOTTOMRIGHT, STYLE_NORMAL);
    
    int msg_y = self->y + self->height/2;
    int msg_len = strlen(popup->message);
    if (msg_len > self->width - 2)
    {
        msg_len = self->width - 2;
    }

    fbDrawChars(fb, self->x+1, msg_y, popup->message, msg_len, STYLE_NORMAL);
}

static int popupHandleInput(Widget *self, int key)
{
    switch (key)
    {
    case ESC:
    case 'q':
        return WIDGET_CLOSE;
    }

    return WIDGET_CONTINUE;
}

static void popupDestroy(Widget *self)
{
    PopupWindow *popup = (PopupWindow*)self->data;
    free(popup->message);
    free(popup);
}

Widget *popupNew(const char *title, const char *message)
{
    PopupWindow *popup = malloc(sizeof(PopupWindow));
    popup->title = strdup(title);
    popup->message = strdup(message);

    Widget *widget = malloc(sizeof(Widget));
    widget->width = strlen(message) + 4;
    widget->height = 5;
    widget->x = (E.screencols - widget->width) / 2;
    widget->y = (E.screenrows - widget->height) / 2;

    widget->draw = &popupDraw;
    widget->handle_input = &popupHandleInput;
    widget->destroy = &popupDestroy;

    widget->data = popup;

    return widget;
}
