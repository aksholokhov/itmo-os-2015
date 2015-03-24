#include <helpers.h>
#include <stdlib.h>
#include <string.h>

char line[4097];
size_t pos = 0;
char delimiter = '\n';
char ** spawn_argv;

int main(int argc, char ** argv) {
    char buf[4096];
    size_t nread;

    if (argc < 2) {
        return 1;
    } else {
        spawn_argv = malloc(sizeof(char *) * (argc + 1));
        spawn_argv[argc - 1] = line;
        spawn_argv[argc] = NULL;
        for (int i = 0; i < argc - 1; i++) {
            spawn_argv[i] = argv[i + 1];
        }
    }

    do {
        nread = read_until(STDIN_FILENO, buf, sizeof(buf), '\n');

        for (int i = 0; i < nread; i++) {
            if (buf[i] == '\n') {
                if (pos != 0) {
                    line[pos] = 0;
                    int spawn_result = spawn(spawn_argv[0], spawn_argv);
                    if (spawn_result == 0) {
                        write(STDOUT_FILENO, line, strlen(line));
                        write(STDOUT_FILENO, &delimiter, 1);
                    }
                }
                pos = 0;
            } else {
                line[pos++] = buf[i];
            }
        }

    } while (nread > 0);

    if (pos != 0) {
        line[pos] = 0;
        int spawn_result = spawn(spawn_argv[0], spawn_argv);
        if (spawn_result == 0) {
            write(STDOUT_FILENO, line, strlen(line));
            write(STDOUT_FILENO, &delimiter, 1);
        }
    }

    free(spawn_argv);

    return 0;
}
