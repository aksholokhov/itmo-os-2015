#include <helpers.h>
#include <stdlib.h>
size_t read_to_char(int fd, const void * buf, size_t count, char separator) {
    if (count == 0) {
        return read(fd, buf, 0);
    }

    int curr_count = 0;
    int pointer = 0;
    int stop = 0;

    do {
        curr_count = read(fd, buf + pointer, count);

        if (curr_count == -1) return -1;

        int i = 0;
        for (i = 0; i < curr_count; i++) {
            if (((char *)buf)[pointer+i] == separator) {
                stop = 1;
                break;
            }
        }

        pointer += curr_count;
        count -= curr_count;
    } while (count > 0 && curr_count > 0 && !stop);

    return pointer;
}

size_t read_(int fd, void * buf, size_t count) {
    return read_to_char(fd, buf, count, -1);
}

size_t write_(int fd, const void * buf, size_t count) {
    int pointer = 0;
    int curr_count = 0;

    if (count == 0) {
        return write(fd, buf, 0);
    }

    do {
        curr_count = write(fd, buf + pointer, count);

        if (curr_count == -1) {
            return -1;
        }

        pointer += curr_count;
        count -= curr_count;
    } while (count > 0 && curr_count > 0);

    return pointer;
}


