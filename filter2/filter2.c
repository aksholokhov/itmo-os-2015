#include <unistd.h>
#include <stdio.h>
#include <helpers.h>
#include <bufio.h>
#include <stdlib.h>
#include <errno.h>


int main() {
    struct buf_t* buffer = buf_new(4096); 
    char buf[2048];
    
    int count = 0;
    
	while(1) {
        execargs_t** programs = (execargs_t**)malloc(sizeof(execargs_t*));
        count = buf_readline(buf, STDIN_FILENO, buffer, sizeof(buf));
        if (count == -1) {
            if (errno != EINTR) {
                break;
            } else {
                errno = 0;
                continue;
            }
        }
        if (count == 1) {
            continue;
        } 

        if (count == 0) {
            return 0;
        }

        int pos = -1;
        for (int i = 0; i < count; i++) {
            if (buf[i] == '\n' || i + 1 == count) {
                programs[0] = makeExecFromStr(buf, pos+1, i);
                if (exec(programs[0]) == 0) {
                    int len = i - pos;
                    if (write(STDOUT_FILENO, buf + pos + 1, len) < len) {
                        return 1;   
                    }
                }
                pos = i;
            }
        }
    }
}
