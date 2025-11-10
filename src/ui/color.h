#ifndef __EDITOR_COLOR_H
#define __EDITOR_COLOR_H

#include <stddef.h>

typedef enum ColorMode
{
    COLOR_8,
    COLOR_16,
    COLOR_256,
    TRUECOLOR
} ColorMode;

// TODO: bit packaging for colors
typedef struct Color
{
    unsigned char r, g, b;
} Color;

enum Attributes
{
    ATTR_BOLD           = 1 << 0,
    ATTR_DIM            = 1 << 1,
    ATTR_ITALIC         = 1 << 2,
    ATTR_UNDERLINE      = 1 << 3,
    ATTR_BLINK          = 1 << 4,
    ATTR_INVERSE        = 1 << 5,
    ATTR_HIDDEN         = 1 << 6,
    ATTR_STRIKETHROUGH  = 1 << 7,
};

typedef struct Style
{
    Color fg;
    Color bg;
    int attr;
} Style;

#define COLOR_BLACK   (Color){ 24,  24,  24}
#define COLOR_RED     (Color){172,  66,  66}
#define COLOR_GREEN   (Color){144, 169,  89}
#define COLOR_YELLOW  (Color){244, 191, 117}
#define COLOR_BLUE    (Color){106, 159, 181}
#define COLOR_MAGENTA (Color){170, 117, 159}
#define COLOR_CYAN    (Color){117, 181, 170}
#define COLOR_WHITE   (Color){216, 216, 216}

#define COLOR_BRIGHT_BLACK   (Color){107, 107, 107}
#define COLOR_BRIGHT_RED     (Color){197,  85,  85}
#define COLOR_BRIGHT_GREEN   (Color){170, 196, 116}
#define COLOR_BRIGHT_YELLOW  (Color){254, 202, 136}
#define COLOR_BRIGHT_BLUE    (Color){130, 184, 200}
#define COLOR_BRIGHT_MAGENTA (Color){194, 140, 184}
#define COLOR_BRIGHT_CYAN    (Color){147, 211, 195}
#define COLOR_BRIGHT_WHITE   (Color){248, 248, 248}

#define COLOR_DEFAULT_FG        COLOR_WHITE
#define COLOR_DEFAULT_BG        COLOR_BLACK

#define COLOR_LNE_HIGHLIGHT     (Color){ 40,  40,  40}
#define COLOR_UI_BLACK          (Color){ 50,  50,  50}

#define STYLE_NORMAL        (Style){COLOR_DEFAULT_FG, COLOR_DEFAULT_BG, 0}
#define STYLE_INVERSE       (Style){COLOR_DEFAULT_BG, COLOR_DEFAULT_FG, 0}

ColorMode getColorMode(void);

size_t printFgColor(char *buf, size_t len, ColorMode mode, Color color);
size_t printBgColor(char *buf, size_t len, ColorMode mode, Color color);

#endif /* __EDITOR_COLOR_H */