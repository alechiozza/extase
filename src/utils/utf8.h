#ifndef __EDITOR_UTF8_H
#define __EDITOR_UTF8_H

#include <stdint.h>

#define UNICODE_UNKNOWN 0xFFFD
#define UNICODE_RIGHT_ARROW 0x2192
#define UNICODE_LEFT_BAR 0x23B8
#define UNICODE_CROSS 0x2715

#define BOX_HORIZONTAL  0x2500
#define BOX_VERTICAL    0x2502
#define BOX_TOPLEFT     0x250c
#define BOX_TOPRIGHT    0x2510
#define BOX_BOTTOMLEFT  0x2514
#define BOX_BOTTOMRIGHT 0x2518

#define DOUBLEBOX_HORIZONTAL  0x2550
#define DOUBLEBOX_VERTICAL    0x2551
#define DOUBLEBOX_TOPLEFT     0x2554
#define DOUBLEBOX_TOPRIGHT    0x2557
#define DOUBLEBOX_BOTTOMLEFT  0x255A
#define DOUBLEBOX_BOTTOMRIGHT 0x255D

int utf8_decode(const char *s, uint32_t *codepoint);
int codepoint_to_utf8(uint32_t codepoint, char *out_buffer);

#endif /* __EDITOR_UTF8_H */
