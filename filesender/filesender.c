#define _POSIX_SOURCE

#include <bufio.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

const char usage[30] = "usage: filesender port file \n";
int sock;


void empty_sigaction(int signo) {
    close(sock);
    printf("exiting... \n");
    exit(0);
}

int main (int argc, char** argv) {
    if (argc != 3) {
        printf(usage);
        return 1;
    }

    int port;
    sscanf(argv[1], "%d", &port);
    char* file = argv[2];

    struct sigaction block  = {
        .sa_handler = empty_sigaction,
        .sa_flags = 0,
    };
    sigemptyset(&block.sa_mask);
    sigaction(SIGINT, &block, NULL);

    
    sock = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sock == -1) {
        printf("Unable to open socket \n");
        return 1;
    }

    int one = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
        printf("setsockopt error \n");
        return 1;
    }

    struct addrinfo hints = {
        AI_V4MAPPED | AI_ADDRCONFIG,
        AF_INET,
        SOCK_STREAM,
        0,
        0, 
        0, 
        0, 
        0           
    };

    hints.ai_next = NULL;
        
    struct addrinfo* result;

    if (getaddrinfo("localhost", NULL, &hints, &result) != 0) {
        printf("getaddrinfo error \n");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    memcpy(&addr, result->ai_addr, result->ai_addrlen);
    freeaddrinfo(result);
    addr.sin_port = htons(port);
    
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("bind error \n");
        return 1;
    }

    if (listen(sock, 1) == -1) {
        printf("listen error \n");
        return 1;
    }

    printf("listening started \n");
    struct sockaddr_in client;
    socklen_t sz = sizeof(client);

    while(1) {
        int client_fd = accept(sock, (struct sockaddr *)&client, &sz);
        if (client_fd == -1) {
            if (errno == EINTR) {
                break;
            } else {
                printf("accept error \n");
                return 1;
            }
        }
        if (fork() == 0) {
            printf("client %d connected \n", client_fd);
            int sendfile_fd = open(file, O_RDONLY);
            if (sendfile_fd == -1) {
                printf("child error: where is my file?! T_T \n");
                exit(0);
            }

            ssize_t read_counter, write_counter;
            struct buf_t * buf = buf_new(4096);

            do {
                read_counter = buf_fill(sendfile_fd, buf, buf->capacity);
                if (read_counter == -1) {
                    printf("child: unable to read from file \n");
                    break;
                }

                write_counter = buf_flush(client_fd, buf, buf->size);
                if (write_counter == -1) {
                    printf("child: unable to write to the client \n");
                    break;
                }
            } while (read_counter > 0);

            printf("client %d finished \n", client_fd);
            exit(0);
        } else {
            close(client_fd);
        }
    }
    
    printf(" exiting...\n");
}
