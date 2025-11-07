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
