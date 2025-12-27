#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <wiringPi.h>
#include <pthread.h>
#include <wiringPiI2C.h>
#include <softPwm.h>

#define LED 0                 /* GPIO17 */
#define I2C_DEV "/dev/i2c-1"; /* I2C를 위한 장치 파일 */
#define I2C_ADDR 0x48         // I2C 장치 주소
#define Channel 0;            // 조도 센서

int main(int argc, char** argv) {
    struct sockaddr_in serveraddr, clientaddr;
    int sockfd;
    int client_sd;
    int socklen;
    int n, i;
    int readn;
    struct epoll_event ev, *client_list;
    int efd;
    char buf_in[256];
    int optval = 1;

    wiringPiSetup();      /* wiringPi 초기화 */
    pinMode(LED, OUTPUT); /* Pin 모드를 출력으로 설정 */

    // event 담을 공간 50개로 확보
    client_list = (struct epoll_event*)malloc(sizeof(*client_list) * 50);

    // epoll 인스턴스 생성
    if ((efd = epoll_create(100)) < 0) {
        perror("epoll_create");
        exit(1);
    }

    // TCP 소켓 생성 및 세팅
    socklen = sizeof(clientaddr);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket error :");
        close(sockfd);
        return 1;
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(60000);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
        close(sockfd);
        return 1;
    }
    listen(sockfd, 1); // 1대1 통신

    // listen 소켓을 epoll에 등록
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &ev);

    while (1) {
        char* ret;
        char cmd1[256];
        char cmd2[256];
        char cmd3[256];

        // wait로 걸어둠
        if ((n = epoll_wait(efd, client_list, 50, -1)) == -1) {
            perror("epoll_wait");
        }
        // wait로 받은 n개 이벤트 처리
        for (i = 0; i < n; i++) {
            // listen 소켓일 경우 accept 해서 new 소켓 생성
            if (client_list[i].data.fd == sockfd) {
                printf("Accept\n");
                client_sd = accept(sockfd, (struct sockaddr*)&clientaddr, &socklen);

                ev.events = EPOLLIN;
                ev.data.fd = client_sd;
                epoll_ctl(efd, EPOLL_CTL_ADD, client_sd, &ev);
            }
            else {
                readn = read(client_list[i].data.fd, buf_in, 255);
                // 에러 or 종료 일때 epoll에서 제거하고 소켓 close
                if (readn <= 0) {
                    epoll_ctl(efd, EPOLL_CTL_DEL, client_list[i].data.fd, client_list);
                    close(client_list[i].data.fd);
                }
                else {
                    buf_in[readn] = '\0';
                    ret = strtok(buf_in, " \r\n");
                    strcpy(cmd1, (ret != NULL) ? ret : "");

                    // LED
                    if (!strcmp(cmd1, "LED")) {
                        ret = strtok(NULL, " \r\n");
                        strcpy(cmd2, (ret != NULL) ? ret : "");

                        if (!strcmp(cmd2, "ON")) {
                            softPwmStop(LED);
                            digitalWrite(LED, HIGH);
                            printf("\n%s %s\n", cmd1, cmd2);
                        }
                        else if (!strcmp(cmd2, "OFF")) {
                            softPwmStop(LED);
                            digitalWrite(LED, LOW);
                            printf("\n%s %s\n", cmd1, cmd2);
                        }
                        else if (!strcmp(cmd2, "MAX")) {
                            softPwmCreate(LED, 0, 255); /* PWM의 범위 설정 */
                            softPwmWrite(LED, 210 & 255);
                            printf("\n%s %s\n", cmd1, cmd2);
                        }
                        else if (!strcmp(cmd2, "MID")) {
                            softPwmCreate(LED, 0, 255); /* PWM의 범위 설정 */
                            softPwmWrite(LED, 130 & 255);
                            printf("\n%s %s\n", cmd1, cmd2);
                        }
                        else if (!strcmp(cmd2, "MIN")) {
                            softPwmCreate(LED, 0, 255); /* PWM의 범위 설정 */
                            softPwmWrite(LED, 50 & 255);
                            printf("\n%s %s\n", cmd1, cmd2);
                        }
                        else {
                            // client에 "잘못된 명령" send
                        }
                    }

                    // 부저
                    if (!strcmp(cmd1, "buzzer")) {
                        if (!strcmp(cmd2, "ON")) {
                            // client에 "buzzer ON" send
                            // 음악 켜기
                        }
                        else if (!strcmp(cmd2, "OFF")) {
                            // client에 "buzzer OFF" send
                            // 음악 끄기
                        }
                        else {
                            // client에 "잘못된 명령" send
                        }
                    }
                    // 조도센서
                    if (!strcmp(cmd1, "cds")) {
                        // 조도 값 체크
                    }
                    // 7세그먼트
                    if (!strcmp(cmd1, "seven")) {
                        //
                    }
                }
            } // else-end
        } // for-end
    } // while -end
    return 0;
}
