#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define logp(...) {printf("%s:%s", __FILE__, __func__); printf(__VA_ARGS__);}

#define BUFLEN 1024
#define MAXADDRLEN 256
#define PORT 8888

int server()
{
    int sock = -1;
    struct sockaddr_in bindaddr;
    char buf[BUFLEN] = {0};
    char addr[MAXADDRLEN] = {0};
    socklen_t addrlen = 0;
    int nrecv = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
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

    while (1) {
        nrecv = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *)&addr, &addrlen);
        if (nrecv > 0) {
            printf("%d bytes recved, data: %s\n", nrecv, buf);
        }
    }

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

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        logp("open socket failed!\n");
        return -1;
    }

    bzero((void*)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    strncpy(buf, "hello world!", BUFLEN);

    while (1) {
        nsend = sendto(sock, buf, BUFLEN, 0, (struct sockaddr *)&addr, sizeof(addr));
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