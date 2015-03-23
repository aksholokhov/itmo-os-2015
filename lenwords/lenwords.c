
#include <stdio.h>
#include <stdlib.h>
#include <helpers.h>
#include <string.h>

char separator = ' ';
int pos = 0;
char out_buffer[30];

int main()
{
    char buffer[1024];
    int counter = 1;
    while (counter > 0) {
        counter = read_to_char(STDIN_FILENO, buffer, sizeof(buffer), separator);

        if (counter < 0) {
            printf("read error");
            exit(1);
        }
        
        int i;
        
        for (i = 0; i < counter; i++) {
            if (buffer[i] == separator) {
                 write_result(pos);
                pos = 0;
            } else {
                pos++;
            }

        }
    }

    if (pos != 0) {
        write_result(pos);
    }

    return 0;
}

int write_result (int count) {
    sprintf(out_buffer, "%d ", count);
    write_(STDOUT_FILENO, out_buffer, strlen(out_buffer));
}

