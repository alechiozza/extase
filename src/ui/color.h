#ifndef __EDITOR_COLOR_H
#define __EDITOR_COLOR_H

#include <stddef.h>
#include <stdint.h>

typedef enum ColorMode
{
    COLOR_8,
    COLOR_16,
    COLOR_256,
    TRUECOLOR
} ColorMode;

// TODO: bit packaging for colors
typedef uint32_t Color;

#define RGB_COLOR(r, g, b) ((uint32_t)(((r) & 0xFF) << 16) | \
                                    (((g) & 0xFF) << 8)  | \
                                    ((b) & 0xFF))

#define COLOR_R(c) ((uint8_t)(((c) >> 16) & 0xFF))
#define COLOR_G(c) ((uint8_t)(((c) >> 8) & 0xFF))
#define COLOR_B(c) ((uint8_t)((c) & 0xFF))

enum Attributes
{
    ATTR_NONE           = 0,
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

#define COLOR_BLACK   RGB_COLOR( 24,  24,  24)
#define COLOR_RED     RGB_COLOR(172,  66,  66)
#define COLOR_GREEN   RGB_COLOR(144, 169,  89)
#define COLOR_YELLOW  RGB_COLOR(244, 191, 117)
#define COLOR_BLUE    RGB_COLOR(106, 159, 181)
#define COLOR_MAGENTA RGB_COLOR(170, 117, 159)
#define COLOR_CYAN    RGB_COLOR(117, 181, 170)
#define COLOR_WHITE   RGB_COLOR(216, 216, 216)

#define COLOR_BRIGHT_BLACK   RGB_COLOR(107, 107, 107)
#define COLOR_BRIGHT_RED     RGB_COLOR(197,  85,  85)
#define COLOR_BRIGHT_GREEN   RGB_COLOR(170, 196, 116)
#define COLOR_BRIGHT_YELLOW  RGB_COLOR(254, 202, 136)
#define COLOR_BRIGHT_BLUE    RGB_COLOR(130, 184, 200)
#define COLOR_BRIGHT_MAGENTA RGB_COLOR(194, 140, 184)
#define COLOR_BRIGHT_CYAN    RGB_COLOR(147, 211, 195)
#define COLOR_BRIGHT_WHITE   RGB_COLOR(248, 248, 248)

#define COLOR_DEFAULT_FG        COLOR_WHITE
#define COLOR_DEFAULT_BG        COLOR_BLACK

#define COLOR_LNE_HIGHLIGHT     RGB_COLOR( 40,  40,  40)
#define COLOR_UI_BLACK          RGB_COLOR( 50,  50,  50)
#define COLOR_UI_DARK_BLACK     RGB_COLOR( 35,  35,  35)

#define STYLE_NORMAL        (Style){COLOR_DEFAULT_FG, COLOR_DEFAULT_BG, 0}
#define STYLE_INVERSE       (Style){COLOR_DEFAULT_BG, COLOR_DEFAULT_FG, 0}

ColorMode getColorMode(void);

size_t printFgColor(char *buf, size_t len, ColorMode mode, Color color);
size_t printBgColor(char *buf, size_t len, ColorMode mode, Color color);

#endif /* __EDITOR_COLOR_H */