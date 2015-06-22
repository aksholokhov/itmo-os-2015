#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>

#define BUF_SIZE 6

int main(int argc, char** argv) {
    int pipefd[2];
    char buf[BUF_SIZE];
    int read_counter;
    int write_counter;
        
    int shift = BUF_SIZE/2;
    int sp = 0;
    int checker = 0;


    if (argc < 3) {
        exit(1);
    }

    if (pipe(pipefd) == -1) {
        exit(1);
    }

    int pid = fork();
    if (pid == -1) {
        exit(1);
    }
    if (pid == 0) {
        close(pipefd[1]);   
        char* substr = argv[3];
        int substr_len = strlen(substr);
        int sum = 0;
        int pos = 0;
        do {
            read_counter = read(pipefd[0], buf + pos, sizeof(buf)/2 - pos);
            if (read_counter == -1) {
                close(pipefd[0]);
                exit(1);
            }
            read_counter += pos;
            printf("\n rec: %s, %d \n", buf, read_counter);
            sum = 0;
            pos = 0;
            for (int i = 0; i < read_counter; i++) {
                if (buf[i] == '\n') {
                   // printf("%d %d \n", substr_len, checker);
                    if (checker >= substr_len) {
                        memcpy(buf + shift + sum, buf + pos, i - pos + 1);
                        checker = 0;
                        sum += i - pos + 1;
                    }
                    pos = i + 1;
                } else {
                    if (buf[i] == substr[checker]) {
                        checker++;
                    } else {
                        if (checker < substr_len) checker = 0;
                    }   
                }
            }
           // printf("%d \n", sum);
            write_counter = write(STDOUT_FILENO, buf + shift, sum);
            memcpy(buf, buf +  pos, read_counter - pos);
            pos = read_counter - pos;
            //printf("%d, %d, %d \n", pos, sum, checker);
        } while (write_counter == sum && read_counter != 0);
        close (pipefd[0]);

    } else {
        close(pipefd[0]);
        char* pat = argv[1];
        char* subst = argv[2];
        int pat_len = strlen(pat);
        int subst_len = strlen(subst);

        do {
            read_counter = read(STDIN_FILENO, buf, sizeof(buf)/2);
            if (read_counter == -1) {
                close(pipefd[1]);
                kill(pid, SIGTERM);
                exit(1);
            }
           
            sp = 0;
            for (int i = 0; i < read_counter; i++) {
                if (checker == pat_len) {
                    for (int j = 0; j < subst_len; j++) {
                        buf[shift + sp] = subst[j];
                        sp++;
                    }
                    checker = 0;
                }
                
                if (buf[i] == pat[checker]) {
                    checker++;
                } else {
                    for (int j = 0; j < checker; j++) {
                        buf[shift + sp] = pat[j];
                        sp++;
                    }
                    checker = 0;
                    buf[shift + sp] = buf[i];
                    sp++;
                }
            }
            
            write_counter = write(pipefd[1], buf + shift, sp);
        } while (write_counter == sp && read_counter != 0);
        close(pipefd[1]);
    }
} 
