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

int make_socket(struct addrinfo* info) {
    while (info) {
        int sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (sock == -1) continue;

        int one = 1;
        int s = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        if (s == -1) {
            if (close(sock) == -1) return -1;
            continue;
        }

        s = bind(sock, info->ai_addr, info->ai_addrlen);
        if (s == 0) {
            return sock;
        }

        if (close(sock) == -1) return -1;
        info = info->ai_next;
    }

    return -1;
}

int create_server(const char* port) {
	
    struct addrinfo hints;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* result;

    if (getaddrinfo(NULL, port, &hints, &result) != 0) {
        printf("getaddrinfo error \n");
        return 1;
    }

    int sock = make_socket(result);

    if (listen(sock, 1) == -1) {
        printf("listen error \n");
        return 1;
    }

    return sock;
}


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

    int sock = create_server(argv[1]); 
    char* file = argv[2];

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
