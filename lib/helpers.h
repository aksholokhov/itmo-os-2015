#ifndef HELPERS_H
#define HELPERS_H

#include <unistd.h>

ssize_t read_(int fd, void * buf, size_t count);
ssize_t write_(int fd, const void * buf, size_t count);
ssize_t read_until(int fd, void * buf, size_t count, char separator);
int spawn(const char * file, char * const argv []);

typedef struct execargs_t execargs_t;
int exec(execargs_t* args);
execargs_t* new_execargs_t (char* file, char** args);
execargs_t* makeExecFromStr(char* buf, int start, int end);
int runpiped(execargs_t** programs, size_t n);

#endif // HELPERS_H

