#ifndef __EDITOR_WIDGET_H
#define __EDITOR_WIDGET_H

typedef struct Framebuffer Framebuffer;

typedef struct Widget
{
    int x, y, width, height;
    int z_index;

    void (*draw)(Framebuffer *fb, struct Widget *self);
    void (*handle_input)(struct Widget *self, int key);
    void (*destroy)(struct Widget *self);
    
    // struct Widget *parent;

    void *data; 
} Widget;

#endif /* __EDITOR_WIDGET_H */
