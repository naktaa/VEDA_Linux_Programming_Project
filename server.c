#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "event.h"

void run_server(void) {
    int sockfd, client_sd;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t socklen = sizeof(clientaddr);
    int efd, n, i;
    struct epoll_event ev, *client_list;
    int optval = 1;

    client_list = malloc(sizeof(*client_list) * 50);
    if (!client_list) {
        perror("malloc");
        exit(1);
    }

    if ((efd = epoll_create(100)) < 0) {
        perror("epoll_create");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(60000);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    listen(sockfd, 1);

    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &ev);

    while (1) {
        if ((n = epoll_wait(efd, client_list, 50, -1)) == -1) {
            perror("epoll_wait");
        }

        for (i = 0; i < n; i++) {
            if (client_list[i].data.fd == sockfd) {
                client_sd = accept(sockfd, (struct sockaddr*)&clientaddr, &socklen);
                ev.events = EPOLLIN;
                ev.data.fd = client_sd;
                epoll_ctl(efd, EPOLL_CTL_ADD, client_sd, &ev);
            }
            else { /* client 입력 받는 부분 */
                client_event(efd, &client_list[i]);
            }
        }
    }

    free(client_list);
}