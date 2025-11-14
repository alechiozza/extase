#include <utf8.h>

#include <stdint.h>

/**
 * Decodes the first UTF-8 character from 's' and stores its codepoint in 'codepoint'.
 * Returns the number of bytes read (1-4) on success, or 0 on invalid UTF-8.
 */
int utf8_decode(const char *s, uint32_t *codepoint)
{
    unsigned char c = (unsigned char)s[0];

    if (c < 0x80) /* 1 byte (ASCII) */
    {
        *codepoint = c;
        return 1;
    }
    else if (c < 0xE0) /* 2 bytes */
    {
        if (!s[1] || (s[1] & 0xC0) != 0x80)
            return 0;
        *codepoint = ((uint32_t)(c & 0x1F) << 6) | (uint32_t)(s[1] & 0x3F);
        return 2;
    }
    else if (c < 0xF0) /* 3 bytes */
    {
        if (!s[1] || (s[1] & 0xC0) != 0x80 || !s[2] || (s[2] & 0xC0) != 0x80)
            return 0;
        *codepoint = ((uint32_t)(c & 0x0F) << 12) |
                     ((uint32_t)(s[1] & 0x3F) << 6) |
                     (uint32_t)(s[2] & 0x3F);
        return 3;
    }
    else if (c < 0xF8) /* 4 bytes */
    {
        if (!s[1] || (s[1] & 0xC0) != 0x80 || !s[2] || (s[2] & 0xC0) != 0x80 || !s[3] || (s[3] & 0xC0) != 0x80)
            return 0;
        *codepoint = ((uint32_t)(c & 0x07) << 18) |
                     ((uint32_t)(s[1] & 0x3F) << 12) |
                     ((uint32_t)(s[2] & 0x3F) << 6) |
                     (uint32_t)(s[3] & 0x3F);
        return 4;
    }

    return 0; /* Invalid */
}

/**
 * Encodes a Unicode codepoint into a UTF-8 string in 'out_buffer'.
 * Returns the number of bytes written (1-4).
 */
int codepoint_to_utf8(uint32_t codepoint, char *out_buffer)
{
    if (codepoint <= 0x7F)
    {
        out_buffer[0] = (char)codepoint;
        return 1;
    }
    else if (codepoint <= 0x7FF)
    {
        out_buffer[0] = (char)(0xC0 | (codepoint >> 6));
        out_buffer[1] = (char)(0x80 | (codepoint & 0x3F));
        return 2;
    }
    else if (codepoint <= 0xFFFF)
    {
        out_buffer[0] = (char)(0xE0 | (codepoint >> 12));
        out_buffer[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        out_buffer[2] = (char)(0x80 | (codepoint & 0x3F));
        return 3;
    }
    else if (codepoint <= 0x10FFFF)
    {
        out_buffer[0] = (char)(0xF0 | (codepoint >> 18));
        out_buffer[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        out_buffer[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        out_buffer[3] = (char)(0x80 | (codepoint & 0x3F));
        return 4;
    }
    
    /* Invalid codepoint */
    out_buffer[0] = '?';
    return 1;
}
