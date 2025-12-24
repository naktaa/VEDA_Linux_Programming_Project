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
#include <softTone.h>
#include <dlfcn.h>

#define LED_LED 0                  /* GPIO 17 */
#define buzzer 2                   /* GPIO 27 */
#define LED_CDS 25                 /* GPIO 24 */
int sevenGPIO[4] = {4, 1, 16, 15}; /* A, B, C, D : 23 18 15 14 */

#define I2C_DEV "/dev/i2c-1" /* I2C를 위한 장치 파일 */
#define I2C_ADDR 0x48        /* I2C 장치 주소 */
#define cdsChannel 0         /* 조도 센서*/
#define cdsBoundary 160      /* 조도 센서 밝기 경계값 */

#define ON 255
#define OFF 0
#define MAX 210
#define MID 130
#define MIN 50

#define BIT 200
int buzzer_run = 1, seven_num = -1, seven_finish = 0;
int fd_CDS;
int illuminance;

typedef void* (*OP_FUNC)(void*);
OP_FUNC fbuzzer, fcds, fseven;

/* 스레드에서 사용하는 뮤텍스 */
pthread_mutex_t buzzer_lock, seven_lock;

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
    char buf_send[256];
    int optval = 1;
    pthread_t ptBuzzer, ptCds, ptSeven;
    void* handle[3]; // 동적 라이브러리 핸들

    // I2C 장치 연결
    if ((fd_CDS = wiringPiI2CSetupInterface(I2C_DEV, I2C_ADDR)) < 0) {
        printf("wiringPiI2CSetupInterface failed:\n");
    }

    // 기본세팅
    wiringPiSetup();                /* wiringPi 초기화 */
    pinMode(LED_LED, OUTPUT);       /* Pin 모드를 출력으로 설정 */
    softPwmCreate(LED_LED, 0, 255); /* PWM의 범위 설정 */
    pinMode(LED_CDS, OUTPUT);       /* Pin 모드를 출력으로 설정 */
    for (int i = 0; i < 4; i++) {
        pinMode(sevenGPIO[i], OUTPUT); /* 모든 Pin의 출력 설정 */
    }

    // 동적 라이브러리 연결
    handle[0] = dlopen("/home/pi/vedaproject/lib/libbuzzer.so", RTLD_LAZY);
    if (handle[0] == NULL) {
        printf("%s\n", dlerror());
        exit(1);
    }
    handle[1] = dlopen("/home/pi/vedaproject/lib/libcds.so", RTLD_LAZY);
    if (handle[1] == NULL) {
        printf("%s\n", dlerror());
        exit(1);
    }
    handle[2] = dlopen("/home/pi/vedaproject/lib/libseven.so", RTLD_LAZY);
    if (handle[2] == NULL) {
        printf("%s\n", dlerror());
        exit(1);
    }

    fbuzzer = (OP_FUNC)dlsym(handle[0], "buzzerControl");
    fcds = (OP_FUNC)dlsym(handle[1], "cdsControl");
    fseven = (OP_FUNC)dlsym(handle[2], "sevenControl");

    /* 뮤텍스 초기화 */
    pthread_mutex_init(&buzzer_lock, NULL);
    pthread_mutex_init(&seven_lock, NULL);

    // 쓰레드 생성
    pthread_create(&ptBuzzer, NULL, fbuzzer, NULL);
    pthread_create(&ptCds, NULL, fcds, NULL);
    pthread_create(&ptSeven, NULL, fseven, NULL);

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

                        // if (strcmp(cmd2, "ON") || strcmp(cmd2, "OFF") || strcmp(cmd2, "MAX") || strcmp(cmd2, "MID") || strcmp(cmd2, "MIN")) {
                        //     printf("LED 잘못된 설정값 입력\n");
                        // }
                        // else {
                        //     ledControl()
                        //     printf("\n%s %s\n", cmd1, cmd2);
                        // }

                        if (!strcmp(cmd2, "ON")) {
                            softPwmWrite(LED_LED, ON & 255);
                        }
                        else if (!strcmp(cmd2, "OFF")) {
                            softPwmWrite(LED_LED, OFF & 255);
                        }
                        else if (!strcmp(cmd2, "MAX")) {
                            softPwmWrite(LED_LED, MAX & 255);
                        }
                        else if (!strcmp(cmd2, "MID")) {
                            softPwmWrite(LED_LED, MID & 255);
                        }
                        else if (!strcmp(cmd2, "MIN")) {
                            softPwmWrite(LED_LED, MIN & 255);
                        }
                        else {
                            printf("LED 잘못된 설정값 입력\n");
                        }
                    }

                    // 부저
                    if (!strcmp(cmd1, "buzzer")) {
                        ret = strtok(NULL, " \r\n");
                        strcpy(cmd2, (ret != NULL) ? ret : "");

                        pthread_mutex_lock(&buzzer_lock);
                        if (!strcmp(cmd2, "ON")) {
                            buzzer_run = 1;
                            printf("buzzer ON\n");
                        }
                        else if (!strcmp(cmd2, "OFF")) {
                            buzzer_run = 0;
                            printf("buzzer OFF\n");
                        }
                        else {
                            printf("buzzer 잘못된 명렁\n");
                        }
                        pthread_mutex_unlock(&buzzer_lock);
                    }

                    // 조도센서
                    if (!strcmp(cmd1, "cds")) {
                        if (illuminance > cdsBoundary) {
                            sprintf(buf_send, "조도 센서 값 = %d --> Dark!", illuminance);
                            write(client_list[i].data.fd, buf_send, strlen(buf_send));
                        }
                        else {
                            sprintf(buf_send, "조도 센서 값 = %d --> Bright!!", illuminance);
                            write(client_list[i].data.fd, buf_send, strlen(buf_send));
                        }
                    }

                    // 7세그먼트
                    if (!strcmp(cmd1, "seg")) {
                        ret = strtok(NULL, " \r\n");

                        if (ret == NULL) {
                            sprintf(buf_send, "seg 7 과 같이 0~9의 값을 쓰세요");
                        }
                        else {
                            if (ret[0] >= '0' && ret[0] <= '9' && ret[1] == '\0') {
                                int num_input = ret[0] - '0';

                                // 여기서 seg_running 검사하고 thread 생성
                                pthread_mutex_lock(&seven_lock);
                                if (seven_num >= 0) {
                                    printf("이미 7 Segment 동작중\n\n");
                                    sprintf(buf_send, "현재 7 Segment 동작 중입니다. 잠시 후 다시 시도");
                                }
                                else {
                                    seven_num = num_input;
                                    printf("[ seg ] %d 부터 카운트다운 시작\n", num_input);
                                    sprintf(buf_send, "%d 부터 카운트다운 시작\n", num_input);
                                    write(client_list[i].data.fd, buf_send, strlen(buf_send));
                                }
                                pthread_mutex_unlock(&seven_lock);
                            }
                            else {
                                // 0~9가 아닌 문자열
                                // client에 "잘못된 인자입니다. 0~9만 허용" send
                            }
                        }
                    }
                }
            } // else-end
        } // for-end
    } // while -end
    return 0;
}