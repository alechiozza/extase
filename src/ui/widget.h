#ifndef __EDITOR_WIDGET_H
#define __EDITOR_WIDGET_H

#define WIDGET_CONTINUE 0
#define WIDGET_CLOSE    1

typedef struct Framebuffer Framebuffer;

typedef struct Widget
{
    int x, y, width, height;
    int z_index;

    void (*draw)(Framebuffer *fb, struct Widget *self);
    int (*handle_input)(struct Widget *self, int key);
    void (*destroy)(struct Widget *self);
    
    // struct Widget *parent;

    void *data; 
} Widget;

void createWidget(Widget *w);
void deleteWidget(Widget *w);

#endif /* __EDITOR_WIDGET_H */
