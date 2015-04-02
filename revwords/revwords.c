#include <stdio.h>
#include <stdlib.h>
#include <helpers.h>


char word[1024];
char separator = ' ';
int pos = 0;

int main()
{
    char buffer[1024];
    int counter = 1;
    while (counter > 0) {
        counter = read_until(STDIN_FILENO, buffer, sizeof(buffer), separator);

        if (counter < 0) {
            printf("read error");
            exit(1);
        }
        int i;
        for (i = 0; i < counter; i++) {
            if (buffer[i] == separator) {
                if (pos != 0) write_reversed();
                pos = 0;
                write_(STDOUT_FILENO, &separator, 1);
            } else {
                word[pos] = buffer[i];
                pos++;
            }

        }
    }

    if (pos != 0) {
        write_reversed();
    }

    return 0;
}

int write_reversed() {
    int i;
    for (i = 0; i < pos / 2; i++) {
        char temp = word[i];
        word[i] = word[pos-i-1];
        word[pos-i-1] = temp;
    }
    write_(STDOUT_FILENO, word, pos);
}

