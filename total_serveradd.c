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

#define LED_LED 0 /* GPIO17 */
#define buzzer 2  /* GPIO25 */
#define LED_CDS 7 /* GPIO4 */

#define music_length (sizeof(notes) / sizeof(notes[0])) /* 음악 전체 계이름의 수 */
#define I2C_DEV "/dev/i2c-1"                            /* I2C를 위한 장치 파일 */
#define I2C_ADDR 0x48                                   /* I2C 장치 주소 */
#define cdsChannel 0                                    /* 조도 센서*/
#define cdsBoundary 160                                 /* 조도 센서 밝기 경계값 */

/* LED 설정값 */
#define ON 255
#define OFF 0
#define MAX 210
#define MID 130
#define MIN 50

#define BIT 100

int sevenGPIO[4] = {4, 1, 16, 15}; /* A, B, C, D : 23 18 15 14 */
int buzzer_run = 1, seven_num = -1, seven_finish = 0;
int fd;
int illuminance;

/* 스레드에서 사용하는 뮤텍스 */
pthread_mutex_t buzzer_lock, seven_lock;

// int notes[] = {/* 슈퍼 마리오 브라더스 오버월드 테마 */
//                659, 659, 0, 659, 0, 523, 659, 0,
//                784, 0, 392, 0,
//                523, 0, 392, 0, 330, 0,
//                440, 0, 494, 0, 466, 440,
//                392, 659, 784, 880,
//                698, 784, 0, 659,
//                523, 587, 494, 0};

// int notes[] = { // 슈퍼마리오
//     660, 660, 0, 660, 0, 523, 660, 0,
//     784, 784, 0, 0, 392, 392, 0, 0,
//     523, 523, 0, 392, 392, 0, 329, 329,
//     0, 440, 0, 494, 0, 466, 440, 440,
//     392, 660, 784, 880, 0, 0, 784, 880,
//     0, 0, 660, 523, 587, 494, 0, 0,
//     523, 523, 0, 392, 392, 0, 330, 330,
//     0, 440, 0, 494, 0, 466, 440, 440};

/*
262 294 330 349 392 440 494
523 587 659 698 784 880 988
*/

int notes[] = { // 징글벨
    294, 494, 440, 392, 294, 294, 294, 494, 440, 392, 330, 330, 523, 494, 440, 370,
    587, 587, 523, 440, 494, 392, 294, 494, 440, 392, 294, 294, 294, 494, 440, 392, 330, 330, 330, 523, 494, 440,
    587, 587, 587, 587, 659, 587, 494, 440, 392, 494, 494, 494, 494, 494, 494, 494, 587, 392, 440, 494,
    523, 523, 523, 523, 523, 494, 494, 494, 494, 494, 440, 440, 494, 440, 587, 494, 494, 494, 494, 494, 494,
    494, 587, 392, 440, 494, 523, 523, 523, 523, 523, 494, 494, 494, 587, 587, 523, 440, 392};
int delays[] = {
    BIT, BIT, BIT, BIT, BIT * 3, BIT, BIT, BIT, BIT, BIT, BIT * 4, BIT, BIT, BIT, BIT, BIT * 4,
    BIT, BIT, BIT, BIT, BIT * 2, BIT * 2, BIT, BIT, BIT, BIT, BIT * 3, BIT, BIT, BIT, BIT, BIT, BIT * 3, BIT, BIT, BIT, BIT, BIT,
    BIT * 3 / 2, BIT / 2, BIT, BIT, BIT, BIT, BIT, BIT, BIT * 4, BIT, BIT, BIT * 2, BIT, BIT, BIT * 2, BIT, BIT, BIT * 3 / 2, BIT / 2, BIT * 4,
    BIT, BIT, BIT * 3 / 2, BIT / 2, BIT, BIT, BIT, BIT / 2, BIT / 2, BIT, BIT, BIT, BIT, BIT * 2, BIT * 2, BIT, BIT, BIT * 2, BIT, BIT, BIT * 2,
    BIT, BIT, BIT * 3 / 2, BIT / 2, BIT * 4, BIT, BIT, BIT * 3 / 2, BIT / 2, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT * 4};

// 7-segment 핀 배치
int number[10][4] = {{0, 0, 0, 0},  /* 0 */
                     {0, 0, 0, 1},  /* 1 */
                     {0, 0, 1, 0},  /* 2 */
                     {0, 0, 1, 1},  /* 3 */
                     {0, 1, 0, 0},  /* 4 */
                     {0, 1, 0, 1},  /* 5 */
                     {0, 1, 1, 0},  /* 6 */
                     {0, 1, 1, 1},  /* 7 */
                     {1, 0, 0, 0},  /* 8 */
                     {1, 0, 0, 1}}; /* 9 */

// void ledControl(char* pwm);
void* buzzerControl(void* p);
void* cdsControl(void* p);
void* sevenControl(void* p);

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

    // I2C 장치 연결
    if ((fd = wiringPiI2CSetupInterface(I2C_DEV, I2C_ADDR)) < 0) {
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

    /* 뮤텍스 초기화 */
    pthread_mutex_init(&buzzer_lock, NULL);
    pthread_mutex_init(&seven_lock, NULL);

    // 쓰레드 생성
    // pthread_create(&ptBuzzer, NULL, buzzerControl, NULL);
    pthread_create(&ptCds, NULL, cdsControl, NULL);
    pthread_create(&ptSeven, NULL, sevenControl, NULL);

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
                            // client에 "buzzer ON" send
                        }
                        else if (!strcmp(cmd2, "OFF")) {
                            buzzer_run = 0;
                            // client에 "buzzer OFF" send
                        }
                        else {
                            // client에 "잘못된 명령" send
                        }
                        pthread_mutex_unlock(&buzzer_lock);
                    }

                    // 조도센서
                    if (!strcmp(cmd1, "cds")) {
                        if (illuminance > cdsBoundary) {
                            sprintf(buf_send, "조도 센서 값 = %d --> Bright!", illuminance);
                            write(client_list[i].data.fd, buf_send, strlen(buf_send));
                        }
                        else {
                            sprintf(buf_send, "조도 센서 값 = %d --> Dark!!", illuminance);
                            write(client_list[i].data.fd, buf_send, strlen(buf_send));
                        }
                    }

                    // 7세그먼트
                    if (!strcmp(cmd1, "seven")) {
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
                                    printf("[ seg ] %d부터 카운트다운 시작\n", num_input);
                                    sprintf(buf_send, "%d부터 카운트다운 시작\n", num_input);
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

void* buzzerControl(void* p) {
    softToneCreate(buzzer); /* 톤 출력을 위한 GPIO 설정 */

    while (1) {
        pthread_mutex_lock(&buzzer_lock);
        int run = buzzer_run;
        pthread_mutex_unlock(&buzzer_lock);

        if (!run) {
            delay(20);
            continue;
        }

        for (int i = 0; i < music_length; ++i) {
            // if (seven_finish) {
            //     softToneWrite(buzzer, 500);
            //     delay(500);
            //     seven_finish = 0;
            // }
            pthread_mutex_lock(&buzzer_lock);
            int still_run = buzzer_run;
            pthread_mutex_unlock(&buzzer_lock);

            if (!still_run) {
                break;
            }

            softToneWrite(buzzer, notes[i]);
            pthread_mutex_unlock(&buzzer_lock);
            delay(delays[i]);
            // delay(delays[i] * 0.9);
            // softToneWrite(buzzer, 0);
            // delay(delays[i] * 0.1);
        }
    }
}

void* cdsControl(void* p) {
    while (1) {
        wiringPiI2CWrite(fd, 0x00 | cdsChannel);
        illuminance = wiringPiI2CRead(fd);
        delay(5);
        illuminance = wiringPiI2CRead(fd);

        if (illuminance > cdsBoundary) {
            digitalWrite(LED_CDS, HIGH);
        }
        else {
            digitalWrite(LED_CDS, LOW);
        }

        printf("[ cds ] 현재 조도 값 = %d\n", illuminance);
        delay(1000);
    }
}

void* sevenControl(void* p) {
    while (1) {
        pthread_mutex_lock(&seven_lock);
        int num = seven_num;
        pthread_mutex_unlock(&seven_lock);

        if (num < 0) {
            delay(20);
            continue;
        }

        for (int n = num; n >= 0; n--) {
            for (int i = 0; i < 4; i++) {
                digitalWrite(sevenGPIO[i], number[n][i] ? HIGH : LOW); /* FND에 값을 출력 */
            }
            delay(1000);
        }

        pthread_mutex_lock(&seven_lock);
        seven_num = -1;
        seven_finish = 1;
        pthread_mutex_unlock(&seven_lock);
        // 부저 실행
    }
}