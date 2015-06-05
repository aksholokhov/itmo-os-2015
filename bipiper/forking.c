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

int create_server(int port) {
	int sock = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    
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
        0, 0, 0, 0, 0           //other parameters are not used in gettadrinfo's hints
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
        read_counter = buf_fill(in, buf, 1);
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

    int port, port2;
    sscanf(argv[1], "%d", &port);
    sscanf(argv[2], "%d", &port2);
    char* file = argv[2];

    struct sigaction block  = {
        .sa_handler = empty_sigaction,
        .sa_flags = 0,
    };
    sigemptyset(&block.sa_mask);
    sigaction(SIGINT, &block, NULL);

    server1 = create_server(port);
    server2 = create_server(port2);

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
