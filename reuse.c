#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define logp(...) {printf("[%s:%d]  ", __FILE__, __LINE__); printf(__VA_ARGS__);fflush(stdout);}

#define BUFLEN 1024
#define MAXADDRLEN 256
#define PORT 8888
#define BACKLOG 10

int max(int a, int b)
{
    if(a >= b)
        return a;
    else
        return b;
}

int tcp_new_server()
{
    int sock = -1;
    int clisock = -1;
    struct sockaddr_in bindaddr;
    char buf[BUFLEN] = {0};
    char addr[MAXADDRLEN] = {0};
    socklen_t addrlen = 0;
    int nrecv = 0;
    struct sockaddr *saddr = NULL;
    int yes = 1;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        logp("open socket failed!\n");
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        logp("setsockopt failed!\n");
        goto FAIL;
    }

    bzero((void*)&addr, sizeof(addr));
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_port = htons(PORT);
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) < 0) {
        logp("bind addr failed, %s\n", strerror(errno));
        goto FAIL;
    }

    if (listen(sock, 5) != 0) {
        logp("listen on sock error!\n");
        goto FAIL;
    }

    return sock;

FAIL:
    close(sock);
    return -1;
}

int udp_new_server()
{
    int sock = -1;
    struct sockaddr_in bindaddr;
    char buf[BUFLEN] = {0};
    char addr[MAXADDRLEN] = {0};
    socklen_t addrlen = 0;
    int nrecv = 0;
    int yes = 1;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        logp("open socket failed!\n");
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        logp("setsockopt failed!\n");
        goto FAIL;
    }

    bzero((void*)&addr, sizeof(addr));
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_port = htons(PORT);
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) < 0) {
        logp("bind addr failed, %s\n", strerror(errno));
        goto FAIL;
    }

    return sock;

FAIL:
    close(sock);
    return -1;
}

int addfds(int fd, int *fds)
{
    int i = 0;
    for (i=0; i< BACKLOG; i++) {
        if (fds[i] <= 0) {
            fds[i] = fd;
            return i;
        }
    }

    return -1;
}

int server_select(void)
{
    int tcpfd = 0;
    int udpfd = 0;
    int maxfd = 0;
    int fds[BACKLOG] = {0};
    fd_set fdset;
    int maxsock;
    struct timeval tv;
    int i = 0;
    int ret = 0;
    char buf[BUFLEN] = {0};
    char addr[MAXADDRLEN] = {0};
    int clisock = 0;
    socklen_t addrlen = 0;
    int nrecv = 0;

    tcpfd = tcp_new_server();
    udpfd = udp_new_server();

    if ((tcpfd < 0) || (udpfd < 0)) {
        return -1;
    }

    tv.tv_sec = 30;
    tv.tv_usec = 0;

    addfds(udpfd, fds);
    while (1) {
        FD_ZERO(&fdset);
        FD_SET(tcpfd, &fdset);

        maxfd = tcpfd;

        for (i = 0; i < BACKLOG; i++) {
            if (fds[i] > 0) {
                FD_SET(fds[i], &fdset);
                maxfd = max(fds[i], maxfd);
            }
        }

        ret = select(maxfd + 1, &fdset, NULL, NULL, &tv);
        if (ret < 0) {
            logp("error on select, %s\n", strerror(errno));
            break;
        } else {
            if (ret == 0) {
                logp("select timeout!\n");
                continue;
            }
        }

       logp("selected!\n"); 
       for (i=0; i<BACKLOG; i++) {
        if (FD_ISSET(fds[i], &fdset)) {
            nrecv = recv(fds[i], buf, BUFLEN, 0);
            if ( nrecv > 0) {
                logp("recv buf, %s\n", buf);
            } else {
                if (nrecv == 0) {
                    logp("connection fds[%d]=%d disconnected!\n", i, fds[i]);
                    close(fds[i]);
                    fds[i] = 0;
                }
            }
        }
       }

        if (FD_ISSET(tcpfd, &fdset)) {
            clisock = accept(tcpfd, (struct sockaddr *)addr, &addrlen);
            if (clisock >= 0) {
                logp("accept from %s\n", inet_ntoa(*(struct in_addr*)addr));
                if (addfds(clisock, fds) < 0) {
                    logp("backlog full, can't add connection!\n");
                }
            }
        }
    }

    close(tcpfd);
    for (i=0; i<BACKLOG; i++) {
        if (fds[i] > 0) {
            close(fds[i]);
            fds[i] = -1;
        }
    }

    return 0;
}

int server_poll(void)
{
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        goto FAIL;
    }

    if (strcmp(argv[1], "select") == 0) {
        return server_select();
    } else {
        if (strcmp(argv[1], "poll") == 0) {
            return server_poll();
        } else {
            goto FAIL;
        }
    }

FAIL:
    printf("%s select/poll\n", argv[0]);
    return 0;
}