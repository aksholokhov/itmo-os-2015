#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define BUF_SIZE 4

int main () {
    char buf[BUF_SIZE];
    int shift = BUF_SIZE/2;
    int sp = 0;
    char b = 'b';

    int read_counter;
    int write_counter;
    int maybe_found = 0;

    do {
        read_counter = read(STDIN_FILENO, buf, sizeof(buf)/2);
        if (read_counter == -1) {
            exit(1);
        }
        
        sp = 0;

        for (int i = 0; i < read_counter; i++) {
            if (buf[i] == 'b') {
                if (maybe_found == 0) {
                    maybe_found = 1;
                } else {
                    buf[shift + sp] = 'a';
                    sp++;
                    maybe_found = 0;
                }
            } else {
                if (maybe_found == 1) {
                    buf[shift + sp] = 'b';
                    maybe_found = 0;
                    sp++;
                }
                buf[shift + sp] = buf[i];
                sp++;
            }
        }
                
        write_counter = write(STDOUT_FILENO, buf + shift, sp);
    } while (write_counter == sp && read_counter != 0);   

    if (maybe_found == 1) {
        write(STDOUT_FILENO, &b, 1);
    }

    return 0;
}
