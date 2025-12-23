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

#define music_length 40            /* 학교종의 전체 계이름의 수 */
#define I2C_DEV "/dev/i2c-1";      /* I2C를 위한 장치 파일 */
#define I2C_ADDR 0x48              // I2C 장치 주소
#define cdsChannel 0               // 조도 센서
int sevenGPIO[4] = {4, 1, 16, 15}; /* A, B, C, D : 23 18 15 14 */
int seven_num = -1, seven_finish = 0;
int fd;
int illuminance;

/* 스레드에서 사용하는 뮤텍스 */
pthread_mutex_t cds_lock, seven_lock;

// int notes[] = {/* 슈퍼 마리오 브라더스 오버월드 테마 */
//                659, 659, 0, 659, 0, 523, 659, 0,
//                784, 0, 392, 0,
//                523, 0, 392, 0, 330, 0,
//                440, 0, 494, 0, 466, 440,
//                392, 659, 784, 880,
//                698, 784, 0, 659,
//                523, 587, 494, 0};

int notes[] = {
    660, 660, 0, 660, 0, 523, 660, 0,
    784, 784, 0, 0, 392, 392, 0, 0,
    523, 523, 0, 392, 392, 0, 329, 329,
    0, 440, 0, 494, 0, 466, 440, 440,
    392, 660, 784, 880, 0, 0, 784, 880,
    0, 0, 660, 523, 587, 494, 0, 0,
    523, 523, 0, 392, 392, 0, 330, 330,
    0, 440, 0, 494, 0, 466, 440, 440};

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

void* buzzerControl(void* p);
void* cdsControl(void* p);
void* sevenControl(void* num);

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
    if ((fd = wiringPiI2CSetup(I2C_ADDR)) < 0) {
        printf("wiringPiI2CSetup failed:\n");
    }
    if ((fd = wiringPiI2CSetupInterface("/dev/i2c-1", I2C_ADDR)) < 0) {
        printf("wiringPiI2CSetupInterface failed:\n");
    }

    // 기본세팅
    wiringPiSetup();          /* wiringPi 초기화 */
    pinMode(LED_LED, OUTPUT); /* Pin 모드를 출력으로 설정 */
    pinMode(LED_CDS, OUTPUT); /* Pin 모드를 출력으로 설정 */
    for (int i = 0; i < 4; i++) {
        pinMode(sevenGPIO[i], OUTPUT); /* 모든 Pin의 출력 설정 */
    }

    pthread_mutex_init(&seven_lock, NULL); /* 뮤텍스 초기화 */

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

                    // LED_LED
                    if (!strcmp(cmd1, "LED_LED")) {
                        ret = strtok(NULL, " \r\n");
                        strcpy(cmd2, (ret != NULL) ? ret : "");

                        if (!strcmp(cmd2, "ON")) {
                            softPwmStop(LED_LED);
                            digitalWrite(LED_LED, HIGH);
                        }
                        else if (!strcmp(cmd2, "OFF")) {
                            softPwmStop(LED_LED);
                            digitalWrite(LED_LED, LOW);
                        }
                        else if (!strcmp(cmd2, "MAX")) {
                            softPwmCreate(LED_LED, 0, 255); /* PWM의 범위 설정 */
                            softPwmWrite(LED_LED, 210 & 255);
                        }
                        else if (!strcmp(cmd2, "MID")) {
                            softPwmCreate(LED_LED, 0, 255); /* PWM의 범위 설정 */
                            softPwmWrite(LED_LED, 130 & 255);
                        }
                        else if (!strcmp(cmd2, "MIN")) {
                            softPwmCreate(LED_LED, 0, 255); /* PWM의 범위 설정 */
                            softPwmWrite(LED_LED, 50 & 255);
                        }
                        else {
                            // client에 "잘못된 명령" send
                        }

                        printf("\n%s %s\n", cmd1, cmd2);
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
                        if (illuminance > 180) {
                            sprintf(buf_send, "조도 센서 값 = %d --> Bright!", illuminance);
                        }
                        else {
                            sprintf(buf_send, "조도 센서 값 = %d --> Dark!!", illuminance);
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
                                    printf("\n%s %s\n", cmd1, cmd2);
                                }
                                pthread_mutex_unlock(&seven_lock);
                            }
                            else {
                                // 0~9가 아닌 문자열
                                // client에 "잘못된 인자입니다. 0~9만 허용" send
                            }
                        }
                    }

                    write(client_list[i].data.fd, buf_send, 256);
                }
            } // else-end
        } // for-end
    } // while -end
    return 0;
}

void* buzzerControl(void* p) {
    softToneCreate(buzzer); /* 톤 출력을 위한 GPIO 설정 */

    while (1) {
        for (int i = 0; i < music_length; ++i) {
            if (seven_finish) {
                softToneWrite(buzzer, 500);
                delay(500);
                seven_finish = 0;
            }
            softToneWrite(buzzer, notes[i]);
            delay(160); /* 음의 전체 길이만큼 출력되도록 대기 */
        }
    }
}

void* cdsControl(void* p) {
    while (1) {
        wiringPiI2CWrite(fd, 0x00 | cdsChannel);
        illuminance = wiringPiI2CRead(fd);
        delay(5);
        illuminance = wiringPiI2CRead(fd);

        // 180 넘으면 어두우니까 켜기
        if (illuminance > 160) {
            digitalWrite(LED_CDS, HIGH);
        }
        else {
            digitalWrite(LED_CDS, LOW);
        }

        printf("현재 조도 값 = %d\n", illuminance);
        delay(1000);
    }
}

void* sevenControl(void* num) {
    while (1) {
        pthread_mutex_lock(&seven_lock);
        if (seven_num < 0) {
            pthread_mutex_unlock(&seven_lock);
            delay(20);
            continue;
        }

        int seven = seven_num;
        pthread_mutex_unlock(&seven_lock);
        for (int n = seven; n >= 0; n--) {
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