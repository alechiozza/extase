#include "utils.h"

#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

ssize_t writen(int fd, const void *buf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    const char *ptr = buf;

    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            // Check for recoverable errors
            if (nwritten < 0 && (errno == EINTR || errno == EAGAIN))
            {
                continue; 
            }
            else
            {
                return -1; 
            }
        }
        nleft -= nwritten;
        ptr += nwritten;
    }

    return (ssize_t)n; 
}

bool is_separator(int c)
{
    return !(isalnum(c) || c == '_' || c == '#');
}

bool is_num(const char *str)
{
    size_t i = 0;
    if (str[i] == '+' || str[i] == '-') i++;

    while (str[i] != '\0')
    {
        if (str[i] < '0' || str[i] > '9')
            return false;

        i++;
    }

    return true;
}

const char *get_filename_from_path(const char *path)
{
    const char *last_slash = strrchr(path, '/');

    if (last_slash != NULL)
    {
        return last_slash + 1;
    }

    return path;
}

size_t next_capacity(size_t current, size_t needed)
{
    size_t new_cap = current ? current : 8;
    while (new_cap < needed)
    {
        size_t doubled = new_cap << 1;
        if (doubled <= new_cap)
            return needed;
        new_cap = doubled;
    }

    return new_cap;
}
