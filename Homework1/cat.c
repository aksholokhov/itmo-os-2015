#include <stdio.h>
#include <stdlib.h>
#include "helpers.h"

int main () {
    char buf[1024];

    int read_counter;
    int write_conter;

    do {
        read_counter = read_(STDIN_FILENO, buf, sizeof(buf));

        if (read_counter == -1) {
            printf("read error");
            exit(1);
        }

        write_conter = write_(STDOUT_FILENO, buf, read_counter);

        if (write_conter != read_counter) {
            printf("write error");
            exit(1);
        }

    } while (read_counter == sizeof(buf));

    return 0;
}

