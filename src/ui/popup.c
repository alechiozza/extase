#include "popup.h"

#include "widget.h"
#include "editor.h"
#include "fb.h"
#include "utf8.h"

#include <string.h>
#include <stdlib.h>

static void popupDraw(Framebuffer *fb, Widget *self)
{
    for (int x = 1; x < self->width-1; x++)
    {
        fbPutCodepoint(fb, x+self->x, self->y, BOX_HORIZONTAL, STYLE_NORMAL);
        fbPutCodepoint(fb, x+self->x, self->y+self->height-1, BOX_HORIZONTAL, STYLE_NORMAL);
    }

    for (int y = 1; y < self->height-1; y++)
    {
        fbPutCodepoint(fb, self->x, y+self->y, BOX_VERTICAL, STYLE_NORMAL);
        for (int x = 1; x < self->width-1; x++)
        {
            fbPutChar(fb, x+self->x, y+self->y, ' ', STYLE_NORMAL);
        }
        fbPutCodepoint(fb, self->x+self->width-1, y+self->y, BOX_VERTICAL, STYLE_NORMAL);
    }

    /* Draw corners */
    fbPutCodepoint(fb, self->x, self->y, BOX_TOPLEFT, STYLE_NORMAL);
    fbPutCodepoint(fb, self->x+self->width-1, self->y, BOX_TOPRIGHT, STYLE_NORMAL);
    fbPutCodepoint(fb, self->x, self->y+self->height-1, BOX_BOTTOMLEFT, STYLE_NORMAL);
    fbPutCodepoint(fb, self->x+self->width-1, self->y+self->height-1, BOX_BOTTOMRIGHT, STYLE_NORMAL);
    
    PopupWindow * popup = (PopupWindow*)self->data;
    int msg_y = self->y + self->height/2;
    int msg_len = strlen(popup->message);
    if (msg_len > self->width - 2)
    {
        msg_len = self->width - 2;
    }

    fbDrawChars(fb, self->x+1, msg_y, popup->message, msg_len, STYLE_NORMAL);
}

static void popupHandleInput(Widget *self, int key)
{
    
}

static void popupDestroy(Widget *self)
{
    PopupWindow *popup = (PopupWindow*)self->data;
    free(popup->message);
    free(popup);

    free(self);
}

Widget *popupNew(const char *message)
{
    PopupWindow *popup = malloc(sizeof(PopupWindow));
    popup->message = strdup(message);

    Widget *widget = malloc(sizeof(Widget));
    widget->width = 20;
    widget->height = 7;
    widget->x = (E.screencols - widget->width) / 2;
    widget->y = (E.screenrows - widget->height) / 2;

    widget->draw = &popupDraw;
    widget->handle_input = &popupHandleInput;
    widget->destroy = &popupDestroy;

    widget->data = popup;

    return widget;
}
