#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define logp(...) {printf("[%s:%d]  ", __FILE__, __LINE__); printf(__VA_ARGS__);}

#define BUFLEN 1024
#define MAXADDRLEN 256
#define PORT 8888

int server()
{
    int sock = -1;
    int clisock = -1;
    struct sockaddr_in bindaddr;
    char buf[BUFLEN] = {0};
    char addr[MAXADDRLEN] = {0};
    socklen_t addrlen = 0;
    int nrecv = 0;
    struct sockaddr *saddr = NULL;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        logp("open socket failed!\n");
        return -1;
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

    saddr = (struct sockaddr *)&addr;
    clisock = accept(sock, saddr, &addrlen);
    if (clisock < 0) {
        logp("accept on sock failed, %s!\n", strerror(errno));
        goto FAIL;
    }

    logp("accept from %s\n", inet_ntoa(*(struct in_addr*)saddr));

    while (1) {
        nrecv = recv(clisock, buf, BUFLEN, 0);
        if (nrecv > 0) {
            printf("%d bytes recved, data: %s\n", nrecv, buf);
        }
    }

    close(clisock);

FAIL:
    close(sock);
    return 0;
}

int client() 
{
    int sock = -1;
    struct sockaddr_in addr;
    char buf[BUFLEN] = {0};
    int nsend = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        logp("open socket failed!\n");
        return -1;
    }

    bzero((void*)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);


    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        logp("connect to sock error, %s!\n", strerror(errno));
        goto FAIL;
    }

    strncpy(buf, "hello world!", BUFLEN);

    while (1) {
        nsend = send(sock, buf, BUFLEN, 0);
        if (nsend > 0) {
            printf("%d bytes send\n", nsend);
        }
        sleep(1);
    }

FAIL:
    close(sock);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        goto FAIL;
    }

    if (strcmp(argv[1], "server") == 0) {
        return server();
    } else {
        if (strcmp(argv[1], "client") == 0) {
            return client();
        } else {
            goto FAIL;
        }
    }

FAIL:
    printf("%s server/client\n", argv[0]);
    return 0;
}