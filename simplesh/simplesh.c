#define _POSIX_C_SOURCE 199309L
#include <unistd.h>
#include <stdio.h>
#include <helpers.h>
#include <bufio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

//This invokes when SIGINT appears. It does nothing.
void signal_handler(int signo) {
    //do nothing
}

int main() {
    //Buffer for bufferized input
    struct buf_t* buffer = buf_new(4096); 
    //Local buffer for one line processing
    char buf[2048];
    
    char dollar[5] = "\n$";
    int count = 0;
    
    //Custon sigaction for SIGINT (CTRL+C) interrupting 
    struct sigaction action;
                        
    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);     
    
	while(1) {
        write(STDOUT_FILENO, &dollar, 2);
        execargs_t** programs = (execargs_t**)malloc(sizeof(execargs_t*)*10);
        //read a line from buffer
        count = buf_readline(buf, STDIN_FILENO, buffer, sizeof(buf));
        if (count == -1) {
            if (errno != EINTR) {
                break;
            } else {
                errno = 0;
                continue;
            }
        }
        //if it's just an '\n' - try again
        if (count == 1) {
            continue;
        } 

        //if it's a 0 symbol (CTRL-D) - exit
        if (count == 0) {
            char exit_text[10] = " exit...\n";
            int write_count = write(STDOUT_FILENO, exit_text, sizeof(exit_text)); 
            if (write_count == -1) {
                exit(1);
            }
            return 0;
        }

        //Split line by '|' and parse programs
        int pos = -1;
        int pNum = 0;
        for (int i = 0; i < count; i++) {
            if (buf[i] == '|' || i + 1 == count) {
                programs[pNum++] = makeExecFromStr(buf, pos+1, i);
                pos = i;
            }
        }

        //run the sequence of programs
        runpiped(programs, pNum);
    }
}
