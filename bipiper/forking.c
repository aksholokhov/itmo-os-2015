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

const char usage[30] = "usage: filesender port file \n";
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

int get_client(int sock, struct sockaddr_in* client) {
	int client_fd = accept(sock, (struct sockaddr *)client, &(socklen_t){sizeof(struct sockaddr_in)});
    if (client_fd == -1) {
        if (errno != EINTR) {
            printf("accept error \n");
        }
        return -1;
    }
    return client_fd;
}

void proceed_transmition(int in, int out) {
	ssize_t read_counter, write_counter;
    struct buf_t * buf = buf_new(4096);

    do {
        int size_before = buf->size;
        read_counter = buf_fill(in, buf, 1);
      //  if (((char*)buf->data)[buf->size-1] == 0) {
        if (size_before == read_counter) {
           // printf("aaaaa \n");
            write_counter = buf_flush(out, buf, buf->size-1);
            shutdown(in, SHUT_RDWR);
            shutdown(out, SHUT_RDWR);
            break;
        }
        if (read_counter == -1) {
            printf("child: unable to read from file \n");
            break;
        }

        write_counter = buf_flush(out, buf, buf->size);
        if (write_counter == -1) {
            printf("child: unable to write to the client \n");
            break;
        }
    } while (read_counter > 0);

    exit(0);
}

void empty_sigaction(int signo) {
    close(server1);
    close(server2);
    printf(" exiting... \n");
    exit(0);
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

    struct sockaddr_in client1, client2;
    while(1) {
    	int client1_fd = get_client(server1, &client1);
    	if (client1_fd == -1) {
    		if (errno == EINTR) {
    			break;
    		} else {
    			return 1;
    		}
    	}

    	int client2_fd = get_client(server2, &client2);
    	if (client2_fd == -1) {
    		if (errno == EINTR) {
    			break;
    		} else {
    			return 1;
    		}
    	}
        
        if (fork() == 0) {
        	proceed_transmition(client1_fd, client2_fd);
        }
        if (fork() == 0) {
        	proceed_transmition(client2_fd, client1_fd);
        }

        close(client1_fd);
        close(client2_fd);
    }
    printf(" exiting...\n");
}
