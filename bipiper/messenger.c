#define _POSIX_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <bufio.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>

#define MAX_CLIENTS 255
#define BUF_SIZE 4096

char usage[30] = "uasge: messenger port \n";
char local_buf[BUF_SIZE];

int serv_fd;
int ccounter;

struct pollfd polls[1 + MAX_CLIENTS];
struct buf_t* buffs[1 + MAX_CLIENTS];

void client_connected(int client_fd) {
    ccounter++;
    polls[ccounter].fd = client_fd;
    buffs[ccounter] = buf_new(BUF_SIZE);
}

void client_disconnected(int c) {
    int client = -c - 1;
    shutdown(polls[client].fd, SHUT_RDWR);
    polls[client].fd = polls[ccounter].fd;
    ccounter--;
}



int get_and_poll() {
    char buf[BUF_SIZE];

    if (ccounter < MAX_CLIENTS) {
        polls[0].events = POLLIN;
    } else {
        polls[0].events = 0;
    }
    while (1) {
        for (int i = 1; i <= ccounter; i++) {
            int bs = buffs[i]->size;
            polls[i].events = POLLIN;
            polls[i].events |= bs == 0 ? 0 : POLLOUT;
        }

        if (poll(polls, 1 + ccounter, -1) == -1) {
            return -1;
        }
        for (int i = 0; i <= ccounter; i++) {
            int happened = polls[i].revents;
            if (happened & POLLIN) {
                if (i == 0) {
                    int res = accept(serv_fd, NULL, NULL);
                    return res;
                } else {
                    int read_counter = recv(polls[i].fd, &local_buf, BUF_SIZE, 0);
 //                   printf("smth recieved \n");
                    for (int j = 1; j <= ccounter; j++) {
                        if (i != j) {
                            int free_space = BUF_SIZE - buffs[j]->size;
                            if (read_counter > free_space) {
                                client_disconnected(j);
                            }
                            memcpy(buffs[j]->data + buffs[j]->size, local_buf, read_counter);
                            buffs[j]->size += read_counter;
                        }
                    }
                }
            }
            if (happened & POLLOUT) {
                //printf("pollout: %d \n", i);
                send(polls[i].fd, buffs[i]->data, buffs[i]->size, 0);
                buffs[i]->size = 0;
            }
            if (happened & POLLHUP) {
                return -(i + 1);
            }
        }

    }
    

}

int make_socket(struct addrinfo *info) {
    while (info) {
        int sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (sock == -1) continue;
        int one = 1;
        int s = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        if (s == -1) {
            if (close(sock) == -1) return -1;
            continue;
        }

        s == bind(sock, info->ai_addr, info->ai_addrlen);
        if (s == 0) {
            return sock;
        }

        if (close(sock) == -1) return -1;
        info = info->ai_next;
    }
    return -1;
}

int make_server(char* port) {
    struct addrinfo hints;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* result;
    
    if (getaddrinfo("localhost", port, &hints, &result) != 0) {
        printf("geraddrinfo error \n");
        return 1;
    }

    int sock = make_socket(result);

    if (listen(sock, 1) == -1) {
        printf("listen error \n");
        return 1;
    }
    
    return sock;
}

void close_all(int signo) {
    for (int i = 0; i<= ccounter; i++) {
        shutdown(polls[i].fd, SHUT_RDWR);
        close(polls[i].fd);
    }
    printf(" closing... \n");
    exit(0);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf(usage);
    }

    struct sigaction close_action = {
        .sa_handler = close_all,
        .sa_flags = 0,
    };
    sigemptyset(&close_action.sa_mask);
    sigaction(SIGINT, &close_action, NULL);

    ccounter = 0;
    serv_fd = make_server(argv[1]);
    polls[0].fd = serv_fd;

    printf("listening started... \n");

    while (1) {
       int client_fd = get_and_poll();
       if (client_fd < 0) {
            client_disconnected(client_fd);
       } else if (client_fd > 0) {
           printf("connected: %d", client_fd);
            client_connected(client_fd);
       }
    }
}
