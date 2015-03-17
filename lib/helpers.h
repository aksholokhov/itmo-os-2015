#ifndef HELPERS_H
#define HELPERS_H

#include <unistd.h>

size_t read_(int fd, void * buf, size_t count);
size_t write_(int fd, const void * buf, size_t count);
size_t read_to_char(int fd, const void * buf, size_t count, char separator);


#endif // HELPERS_H

