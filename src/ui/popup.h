#ifndef __EDITOR_POPUP_H
#define __EDITOR_POPUP_H

typedef struct Widget Widget;

typedef struct PopupWindow
{
    char *message;
} PopupWindow;

Widget *popupNew(const char *message);

#endif /* __EDITOR_POPUP_H */
