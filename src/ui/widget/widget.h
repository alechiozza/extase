#ifndef __EDITOR_WIDGET_H
#define __EDITOR_WIDGET_H

#define WIDGET_CONTINUE 0
#define WIDGET_CLOSE    1

typedef struct FrameBuffer FrameBuffer;
typedef struct Window Window;

typedef struct Widget
{
    int x, y, width, height;
    int cx, cy;
    int z_index;

    void (*draw)(struct Widget *self, FrameBuffer *fb);
    int (*handle_input)(struct Widget *self, int key);
    void (*destroy)(struct Widget *self);
    
    // struct Widget *parent;

    void *data; 
} Widget;

void createWidget(Widget *w);
void deleteWidget(Widget *w);

Widget *popupCreate(const char *title, const char *message);
Widget *filePickerCreate(Window *W);

#endif /* __EDITOR_WIDGET_H */
