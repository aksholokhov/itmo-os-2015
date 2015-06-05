#define _POSIX_SOURCE
#define _GNU_SOURCE

#include <bufio.h>
#include <stdio.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/poll.h>



#define MAX_CLIENTS  127
#define BUF_SIZE  4096

const char usage[30] = "usage: port1 port2 \n";
int ccounter;

struct pollfd polls[2 * MAX_CLIENTS + 2];
struct buf_t* buffers[2 * MAX_CLIENTS];
int server1, server2;

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

int get_and_poll(int sock) {
    int pcount = 2 * ccounter + 2;
    char buf[BUF_SIZE];
    
    for (int i = 0; i < 2; i++) {
        if (polls[i].fd == sock && ccounter < MAX_CLIENTS) {
            polls[i].events = POLLIN;
        } else {
            polls[i].events = 0;
        }
    }

    while(1) {
        for (int i = 0; i < ccounter; i++) {
            int b1s = buffers[2 * i] -> size;
            int b2s = buffers[2 * i + 1] -> size;

            polls[2 * i + 2].events = b1s == BUF_SIZE ? 0 : POLLIN;
            polls[2 * i + 2].events |= b2s == 0 ? 0 : POLLOUT;
            polls[2 * i + 3].events = b1s == 0 ? 0 : POLLOUT;
            polls[2 * i + 3].events |= b2s == BUF_SIZE ? 0 : POLLIN;
        }

        if (poll(polls, pcount, -1) < 0) {
            return -1;
        }

        for (int i = 0; i < pcount; i++) {
            int t = i ^ 1;
            int ans = polls[i].revents;
            if (ans & POLLIN) {
                if (polls[i].fd == sock) {
                    return accept(sock, NULL, NULL);
                }
                else {
                    int free_space = BUF_SIZE - buffers[i - 2]->size;
                    int read_counter = recv(polls[i].fd, &buf, free_space, 0);
                    if (read_counter > 0) {
                        memcpy(buffers[i - 2]->data + buffers[i-2]->size, buf, read_counter);
                        buffers[i - 2]->size +=read_counter;
                    } else if (read_counter == 0) {
                        return -(i & ~1);
                    }
                }
            }

            if (ans & POLLOUT) {
                int free_space = buffers[t - 2]->size;
                int write_counter = send(polls[i].fd, buffers[t - 2]->data, free_space, 0);
                if (write_counter > 0) {
                    buffers[t - 2]->size = 0;
                }
            }

            if (ans & POLLHUP) {
                return -(i & ~1);
            }
        }

    }

}

void empty_sigaction(int signo) {
    close(server1);
    close(server2);
    printf(" exiting...\n");
    exit(0);
}

void close_pair(int t) {
	close(polls[t].fd);
    close(polls[t+1].fd);

    buffers[t - 2] = buffers[2 * ccounter - 2];
    buffers[t - 1] = buffers[2 * ccounter - 1];

    polls[t].fd = polls[2 * ccounter].fd;
    
    polls[t + 1].fd = polls[2 * ccounter + 1].fd;
    
    ccounter--;
}

void new_pair(int fd1, int fd2) {
	polls[2 * ccounter + 2].fd = fd1;
    polls[2 * ccounter + 3].fd = fd2;

    buffers[2 * ccounter] = buf_new(BUF_SIZE);
    buffers[2 * ccounter + 1] = buf_new(BUF_SIZE);

    ccounter++;
}

int main (int argc, char** argv) {
    if (argc != 3) {
        printf(usage);
        return 1;
    }


    struct sigaction block  = {
        .sa_handler = empty_sigaction,
        .sa_flags = 0,
    };
    sigemptyset(&block.sa_mask);
    sigaction(SIGINT, &block, NULL);

    server1 = create_server(argv[1]);
    server2 = create_server(argv[2]);

    polls[0].fd = server1;
    polls[1].fd = server2;

    ccounter = 0;

    while(1) {
    	int client1_fd = get_and_poll(server1);
    	if (client1_fd == -1) {
    		if (errno == EINTR) {
    			break;
    		} else {
    			printf("get&poll error\n");
    			return 1;
    		}
    	}

        int client2_fd = 0;
        if (client1_fd > 0) {
            client2_fd = get_and_poll(server2);
            if (client2_fd == -1) {
                if (errno == EINTR) {
                    break;
                } else {
    				printf("get&poll error\n");
                    return 1;
                }
            }
        }
        
        if (client1_fd < 0 || client2_fd < 0) {
            int t;
            if (client1_fd < 0)
                t = -client1_fd;
            else 
                t = -client2_fd;
            close_pair(t);
            
        }
        else {
            new_pair(client1_fd, client2_fd);
        }
    }
    printf(" exiting...\n");
}
