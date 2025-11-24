#ifndef __EDITOR_UTILS_H
#define __EDITOR_UTILS_H

#include <sys/types.h>
#include <stdbool.h>

ssize_t writen(int fd, const void *buf, size_t n);
bool is_separator(int c);
bool is_num(const char *str);

#endif /* __EDITOR_UTILS_H */
