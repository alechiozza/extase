#ifndef __EDITOR_UTILS_H
#define __EDITOR_UTILS_H

#include <sys/types.h>
#include <stdbool.h>

ssize_t writen(int fd, const void *buf, size_t n);
bool is_separator(int c);
bool is_num(const char *str);
const char *get_filename_from_path(const char *path);

#endif /* __EDITOR_UTILS_H */
