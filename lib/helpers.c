#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE
#include <helpers.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>


ssize_t read_until(int fd, void * buf, size_t count, char delimeter) {
    size_t nall = 0;
    size_t nread;
    int delim_found = 0;

    if (count == 0) {
        return read(fd, buf, 0);
    }

    do {
        nread = read(fd, buf + nall, count);

        if (nread == -1) {
            return -1;
        }

        for (int i = 0; i < nread; i++) {
            if (((char*) buf)[nall + i] == delimeter) {
                delim_found = 1;
                break;
            }
        }

        nall += nread;
        count -= nread;
    } while (count > 0 && nread > 0 && !delim_found);

    return nall;
}

ssize_t read_(int fd, void * buf, size_t count) {
    size_t nall = 0;
    size_t nread;

    if (count == 0) {
        return read(fd, buf, 0);
    }

    do {
        nread = read(fd, buf + nall, count);

        if (nread == -1) {
            return -1;
        }

        nall += nread;
        count -= nread;
    } while (count > 0 && nread > 0);

    return nall;
}

ssize_t write_(int fd, const void * buf, size_t count) {
    size_t nall = 0;
    size_t nwritten;

    if (count == 0) {
        return write(fd, buf, 0);
    }

    do {
        nwritten = write(fd, buf + nall, count);

        if (nwritten == -1) {
            return -1;
        }

        nall += nwritten;
        count -= nwritten;
    } while (count > 0 && nwritten > 0);

    return nall;
}

int spawn(const char * file, char * const argv []) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(file, argv);
        return -1;
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
}

struct execargs_t {
	char* file;
	char** args;	
};


// Constructor for execargs structure
execargs_t* new_execargs_t (char* file, char** args) {
	execargs_t* new_execargs = (execargs_t*)malloc(sizeof(execargs_t));
	new_execargs->file = file;
	new_execargs->args = args;
	return new_execargs;
}


int exec(execargs_t* args) {
	return spawn(args->file, args-> args);
}


// Empty signal handler for SIGINT and SIGCHLD interception
void signal_handler(int signo) {
    //do nothing
}

//Runs the sequence of the programs
int runpiped(execargs_t** programs, size_t n) {
    
    sigset_t mask;
    siginfo_t info;
    // Block all SIGINT and SIGCHLD events to maintain them manually
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    

    struct sigaction action;
    struct sigaction oldaction, oldaction2;

    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;
    
    // Save previous actions and deploys new
    sigaction(SIGINT, NULL, &oldaction);
    sigaction(SIGCHLD, NULL, &oldaction2);

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGCHLD, &action, NULL);

    // Create the pipeline
    int pipefd[2];
    int* children = (int*)malloc(sizeof(int)*n);
    int* pipes = (int*)malloc(sizeof(int)*2*(n-1));

    for (int i = 0; i < n-1; i+=1) {
        if (pipe2(pipefd, O_CLOEXEC) == -1) {
            for (int j = i-1; j>= 0; j--) {
                close(pipes[2*j]);
                close(pipes[2*j + 1]);
            }
            return 1;
        }
        pipes[2*i] = pipefd[0];
        pipes[2*i+1] = pipefd[1];
    }

    // Invoke all programs
    size_t alive = 0;
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            
        }
        if (pid == 0) {
            // Establish connections between threads
            if (i != 0) {
                dup2(pipes[2*i - 2], STDIN_FILENO);
            }
            if (i != n - 1) {
                dup2(pipes[2*i + 1], STDOUT_FILENO);
            }
            // exec the program
            return execvp(programs[i]->file, programs[i]->args);          
        } else {
            children[i] = pid;
            alive++;
        }
    }   
    // close all pipes after program starting
    for (int i = 0; i < 2*(n-1); i++) {
        close(pipes[i]);
    }


    // wait passively children finishing or SIGINT appearance
    while (alive) {
        if (sigwaitinfo(&mask, &info) == -1) {
            continue;
        }
        int pid;
        switch(info.si_signo) {
            // if one of the children changed his state - terminate everything and exit
            case SIGCHLD:
                while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
                    for (int i = 0; i < n; i++) {
                        if (children[i] == pid) {
                            children[i] = 0;    
                            break;
                        }
                    }
                }
            case SIGINT:
                alive = 0;
        }
    }

    int status;
    int retcode;
    for (int i = 0; i < n; i++) {
        if (children[i] > 0) {
            kill(children[i], SIGKILL);
            waitpid(children[i], &status, 0);
            if (i == n-1) {
                retcode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            }
        }
    } 

    sigaction(SIGINT, &oldaction, NULL);
    sigaction(SIGCHLD, &oldaction2, NULL);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);

    return retcode;
}

execargs_t* makeExecFromStr(char* buf, int start, int end) {
    char* path = (char*)malloc(sizeof(char)*4096);
    char** args = (char**)malloc(sizeof(char*)*4096);
    while(buf[start] == ' ') start++;
    while(buf[end] == ' ' || buf[end] == '|' || buf[end] == '\n') end--;
    int pathGot = 0;
    int pos = 0;
    int argNum = 1;
    for (int i = start; i <= end + 1; i++) {
        if (i == end + 1 || buf[i] == ' ') {
            if (pathGot) {
                args[argNum++][pos] = 0;
                args[argNum] = (char*)malloc(sizeof(char)*4096);
                //printf("%s ", args[argNum-1]);
                pos = 0;
            }
            else {
                path[pos] = 0;
                //printf("%s ", path);
                pos = 0;
                pathGot = 1;
                args[0] = path;
                args[1] = (char*)malloc(sizeof(char)*4096);
            }
        } else {
            if (pathGot) {
                args[argNum][pos++] = buf[i];
            } else {
                path[pos++] = buf[i];    
            }
        }
    }
    args[argNum] = 0;
//    printf("\n");
    return new_execargs_t(path, args);
}
