#define _GNU_SOURCE
#include <unistd.h>
#include <helpers.h>
#include <bufio.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>

#define BUF_SIZE 4096

int main() {
    char buf[BUF_SIZE];
    
    int* pipes[2];
    pipes[0] = new int[2];
    pipes[1] = new int[2];

    if (pipe(pipes[0]) == -1) {
        exit(1);
    }

    if (pipe(pipes[1]) == -1) {
        close(pipes[0][1]);
        close(pipes[0][0]);
        exit(1);
    }

    char** args = (char** )malloc(sizeof(char*)*10);
    char* arg;
    int rpipe = 0;
    int first = 1;
    int last = 0;
    std::string s;
    std::getline(std::cin, s);
    while (s.length() > 0) {
        std::stringstream ss;
            ss << s;
            int i = 0;
            while (ss >> s) {
                arg = new char[s.length()+1];
                strcpy(arg, s.c_str());
                args[i] = arg;
                i++;
            }
            args[i] = 0;
            
            std::getline(std::cin, s);
            if (s.length() == 0) last = 1;
            int pid = fork();
            
            if (pid == 0) {
                int wpipe = rpipe == 0 ? 1 : 0;
                close(pipes[rpipe][1]);
                close(pipes[wpipe][0]);
                if (!first) dup2(pipes[rpipe][0], STDIN_FILENO);
                if (!last) dup2(pipes[wpipe][1], STDOUT_FILENO);
                close(pipes[rpipe][0]);
                close(pipes[wpipe][1]);
                execvp(args[0], args);
            } else {
               // printf("%d, %d, %d \n", rpipe, first, last);
                first = 0;
                rpipe = rpipe == 0 ? 1 : 0;
                int status;
                waitpid(pid, &status, 0);
            }
    }

}
