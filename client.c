#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/select.h>
#include <signal.h>

void chatting(int sd);

#define MAXDATASIZE 1024

int main(int argc, char* argv[]) {
    sigset_t sigset;
    int sockfd, numbytes;
    socklen_t addr_len;
    char buf[MAXDATASIZE];
    struct hostent* he;
    struct sockaddr_in server_addr;

    if (argc != 2) {
        fprintf(stderr, "usage : client hostname \n");
        exit(1);
    }
    if ((he = gethostbyname(argv[1])) == NULL) {
        perror("gethostbyname");
        exit(1);
    }
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // 시그널 마스크 설정
    sigfillset(&sigset);
    sigdelset(&sigset, SIGINT);
    sigprocmask(SIG_SETMASK, &sigset, NULL);

    // 서버 정보 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(60000);
    server_addr.sin_addr = *((struct in_addr*)he->h_addr);
    printf("[ %s ]\n", (char*)inet_ntoa(server_addr.sin_addr));
    memset(&(server_addr.sin_zero), '\0', 8);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }
    chatting(sockfd);
    close(sockfd);
    return 0;
}

void chatting(int sd) {
    fd_set mask, read_fds;
    int n, i;
    char ch;
    char buf[MAXDATASIZE];
    struct pollfd pollfds[2];

    pollfds[0].fd = 0;
    pollfds[0].events = POLLIN;
    pollfds[1].fd = sd;
    pollfds[1].events = POLLIN;

    while (1) {
        // printf("=============================================================\n");
        // printf("cmd 입력\n");
        // printf("LED ON/OFF/MAX/MID/MIN\n"); // 켜기, 끄기, 최대, 중간, 최저
        // printf("LED \n");
        // printf("=============================================================\n");

        if (poll(pollfds, 2, -1) == -1) {
            perror("poll");
            return;
        }
        if (pollfds[1].revents & POLLIN) {
            n = recv(sd, buf, MAXDATASIZE - 1, 0);
            if (n <= 0)
                break;
            buf[n] = '\0';
            if (!strcmp(buf, "..."))
                break;
            printf("%s\n", buf);
        }
        else if (pollfds[0].revents & POLLIN) {
            fgets(buf, MAXDATASIZE - 1, stdin);
            buf[strlen(buf) - 1] = '\0';
            send(sd, buf, strlen(buf), 0);
            if (strncmp(buf, "...", 3) == 0)
                break;
        }
        else {
            printf("Unexpected event\n");
            break;
        }
    }
}
