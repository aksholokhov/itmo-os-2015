#ifndef HELPERS_H
#define HELPERS_H

#include <unistd.h>

ssize_t read_(int fd, void * buf, size_t count);
ssize_t write_(int fd, const void * buf, size_t count);
ssize_t read_to_char(int fd, const void * buf, size_t count, char separator);
int spawn(const char * file, char * const argv []);

typedef struct execargs_t execargs_t;

int exec(execargs_t* args);
#endif // HELPERS_H

