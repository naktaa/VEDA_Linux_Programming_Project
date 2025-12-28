#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <wiringPi.h>
#include <softPwm.h>
#include "globals.h"
#include "event.h"

typedef struct cmd_type {
    char* cmd;
    void (*cmd_func)(int, char*);
} CMD_TYPE;

void handle_led(int fd, char* arg);
void handle_buzzer(int fd, char* arg);
void handle_cds(int fd, char* arg);
void handle_seven(int fd, char* arg);
void handle_help(int fd, char* arg);
void command_check(int fd, char* buf_in);

CMD_TYPE client_cmds[] = {
    {"LED", handle_led},
    {"buzzer", handle_buzzer},
    {"cds", handle_cds},
    {"seg", handle_seven},
    {"help", handle_help},
    {NULL, NULL},
};

int client_event(int efd, struct epoll_event* client_list) {
    int fd = client_list->data.fd;
    char buf_in[256];
    int readn = read(fd, buf_in, 255);

    // 에러 or 종료 일때 epoll에서 제거하고 소켓 close
    if (readn <= 0) {
        epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        return -1;
    }

    buf_in[readn] = '\0';
    command_check(fd, buf_in);
    return 0;
}

void command_check(int fd, char* buf_in) {
    char* ret = strtok(buf_in, " \r\n");
    char* cmd1;
    char* cmd2 = NULL;

    if (!ret) return; /* 빈 줄 무시 */
    cmd1 = ret;

    ret = strtok(NULL, " \r\n");
    cmd2 = ret;

    // 등록된 cmd인지 찾기
    for (int i = 0; client_cmds[i].cmd; i++) {
        if (!strcasecmp(cmd1, client_cmds[i].cmd)) {
            client_cmds[i].cmd_func(fd, cmd2);
            return;
        }
    }

    char* msg = "잘못된 명령어 입력\n";
    write(fd, msg, strlen(msg));
}

void handle_led(int fd, char* arg) {
    char msg[128];

    if (!arg) {
        sprintf(msg, "LED 명령: ON/OFF/MAX/MID/MIN 중 하나를 입력\n");
        write(fd, msg, strlen(msg));
        return;
    }

    if (!strcasecmp(arg, "ON")) {
        softPwmWrite(LED_LED, led_light & DUTY);
    }
    else if (!strcasecmp(arg, "OFF")) {
        softPwmWrite(LED_LED, 0);
    }
    else if (!strcasecmp(arg, "MAX")) {
        led_light = MAX;
    }
    else if (!strcasecmp(arg, "MID")) {
        led_light = MID;
    }
    else if (!strcasecmp(arg, "MIN")) {
        led_light = MIN;
    }
    else {
        sprintf(msg, "LED 잘못된 설정값 입력\n");
        write(fd, msg, strlen(msg));
        return;
    }

    sprintf(msg, "LED %s 설정 완료\n", arg);
    write(fd, msg, strlen(msg));
}

void handle_buzzer(int fd, char* arg) {
    char msg[128];

    if (!arg) {
        sprintf(msg, "buzzer 명령: ON/OFF 중 하나를 입력\n");
        write(fd, msg, strlen(msg));
        return;
    }

    pthread_mutex_lock(&buzzer_lock);
    if (!strcasecmp(arg, "ON")) {
        buzzer_run = 1;
        sprintf(msg, "buzzer ON\n");
    }
    else if (!strcasecmp(arg, "OFF")) {
        buzzer_run = 0;
        sprintf(msg, "buzzer OFF\n");
    }
    else {
        sprintf(msg, "buzzer 잘못된 설정값 입력\n");
    }
    pthread_mutex_unlock(&buzzer_lock);

    write(fd, msg, strlen(msg));
}

void handle_cds(int fd, char* arg) {
    char msg[128];

    if (illuminance > cdsBoundary) {
        sprintf(msg, "현재 조도 값 = %d --> 어두움!!\n", illuminance);
    }
    else {
        sprintf(msg, "현재 조도 값 = %d --> 밝음!!\n", illuminance);
    }
    write(fd, msg, strlen(msg));
}

void handle_seven(int fd, char* arg) {
    char msg[128];

    if (!arg) {
        sprintf(msg, "\"seg 7\" 과 같이 0~9의 값을 쓰세요\n");
        write(fd, msg, strlen(msg));
        return;
    }

    if (!(arg[0] >= '0' && arg[0] <= '9' && arg[1] == '\0')) {
        sprintf(msg, "seg 인자는 0~9만 허용\n");
        write(fd, msg, strlen(msg));
        return;
    }

    int num_input = arg[0] - '0';

    // 여기서 seg_running 검사하고 thread 생성
    pthread_mutex_lock(&seven_lock);
    if (seven_num >= 0) {
        sprintf(msg, "현재 7 Segment 동작 중입니다. 잠시 후 다시 시도\n");
    }
    else {
        seven_num = num_input;
        sprintf(msg, "7 segment [ %d ] 부터 카운트다운 시작\n", num_input);
    }
    pthread_mutex_unlock(&seven_lock);

    write(fd, msg, strlen(msg));
}

void handle_help(int fd, char* arg) {
    char msg[] =
        "==== Command Menu ====\n"
        "/*   대소문자 구분 X  */\n"
        "LED    ON/OFF/MAX/MID/MIN\n"
        "buzzer ON/OFF\n"
        "cds    현재 조도 값 출력\n"
        "seg    0~9 값으로 카운트다운 시작\n"
        "help   도움말 보기\n"
        "=====================\n";
    write(fd, msg, strlen(msg));
}