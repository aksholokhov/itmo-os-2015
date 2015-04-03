#include <stdio.h>
#include <stdlib.h>
#include <bufio.h>
int main () {
    struct buf_t * buf = buf_new(4096);
    int retcode = 0;
    int read_counter;
    int write_conter;
    do {
        read_counter = buf_fill(STDIN_FILENO, buf, buf_capacity(buf));
        write_conter = buf_flush(STDOUT_FILENO, buf, buf_size(buf));
        if (read_counter == -1 || read_counter == -1) {
           retcode = 1;
           break;
        }
    } while (read_counter > 0);
    buf_free(buf);
    return retcode;
}
