#include <helpers.h>

#include <sys/types.h>
#include <sys/wait.h>

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

execargs_t* new_execargs_t (char* file, char** args) {
	execargs_t* new_execargs = (execargs_t*)malloc(sizeof(execargs_t));
	new_execargs->file = file;
	new_execargs->args = args;
	return new_execargs;
}


int exec(execargs_t* args) {
	return spawn(args->file, args-> args);
}

int runpiped(execargs_t** programs, size_t n) {
    int last_end = STDIN_FILENO;
    int pipefd[2];
    for (int i = 0; i < n; i++) {
        if (pipe(pipefd) == -1) {
            
        }
        pid_t pid = fork();
        if (pid_t == -1) {

        }
        if (pid == 0) {
            dup2(last_end, STDIN_FILENO);
            dup2(pipefd[1], STDOUT_FILENO);
            last_end = pipefd[0];
            return exec(execargs[i]);
        } 
    }   
    dup2(STDOUT_FILENO, last_end);
}
