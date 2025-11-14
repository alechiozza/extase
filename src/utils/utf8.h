#ifndef __EDITOR_UTF8_H
#define __EDITOR_UTF8_H

#include <stdint.h>

#define UNICODE_UNKNOWN 0xFFFD
#define BOX_HORIZONTAL  0x2500
#define BOX_VERTICAL    0x2502
#define BOX_TOPLEFT     0x250c
#define BOX_TOPRIGHT    0x2510
#define BOX_BOTTOMLEFT  0x2514
#define BOX_BOTTOMRIGHT 0x2518

int utf8_decode(const char *s, uint32_t *codepoint);
int codepoint_to_utf8(uint32_t codepoint, char *out_buffer);

#endif /* __EDITOR_UTF8_H */
