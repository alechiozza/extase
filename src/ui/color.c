#include "color.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ColorMode getColorMode(void)
{
    const char *term = getenv("TERM");
    const char *colorterm = getenv("COLORTERM");

    if (colorterm)
    {
        if (strstr(colorterm, "truecolor") ||
            strstr(colorterm, "24bit"))
        {
            return TRUECOLOR;
        }
    }

    /* Obvious ANSI/VT compatible families */
    if (term)
    {
        if (strstr(term, "xterm") ||
            strstr(term, "tmux") ||
            strstr(term, "screen") ||
            strstr(term, "vte") ||
            strstr(term, "alacritty") ||
            strstr(term, "foot") ||
            strstr(term, "wezterm") ||
            strstr(term, "kitty") ||
            strstr(term, "gnome") ||
            strstr(term, "konsole") ||
            strstr(term, "rxvt")) 
        {
            return COLOR_256;
        }
    }

#if defined(_WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(hOut, &mode))
    {
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if (SetConsoleMode(hOut, mode))
        {
            return COLOR_256;
        }
    }
#endif

    return COLOR_8;
}

size_t printFgColor(char *buf, size_t len, ColorMode mode, Color color)
{
    unsigned int r = color.r;
    unsigned int g = color.g;
    unsigned int b = color.b;

    if (mode == TRUECOLOR)
    {
        return snprintf(buf, len, ";38;2;%d;%d;%d", r, g, b);
    }
    else if (mode == COLOR_256)
    {
        // map RGB roughly to 256-color cube
        int color = 16 + (36 * (r / 51)) + (6 * (g / 51)) + (b / 51);
        return snprintf(buf, len, ";38;5;%d", color);
    }
    else if (mode == COLOR_8)
    {
        int color = (r > 127) * 1 + (g > 127) * 2 + (b > 127) * 4;
        return snprintf(buf, len, ";3%d", color);
    }
    else
    {
        exit(EXIT_FAILURE);
    }

    return 0;
}

size_t printBgColor(char *buf, size_t len, ColorMode mode, Color color)
{
    unsigned int r = color.r;
    unsigned int g = color.g;
    unsigned int b = color.b;

    if (mode == TRUECOLOR)
    {
        return snprintf(buf, len, ";48;2;%d;%d;%d", r, g, b);
    }
    else if (mode == COLOR_256)
    {
        // map RGB roughly to 256-color cube
        int color = 16 + (36 * (r / 51)) + (6 * (g / 51)) + (b / 51);
        return snprintf(buf, len, ";48;5;%d", color);
    }
    else if (mode == COLOR_8)
    {
        int color = (r > 127) * 1 + (g > 127) * 2 + (b > 127) * 4;
        return snprintf(buf, len, ";4%d", color);
    }
    else
    {
        exit(EXIT_FAILURE);
    }

    return 0;
}
