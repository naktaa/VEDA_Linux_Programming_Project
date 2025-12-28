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

void handle_help(int fd, char* arg);
void handle_led(int fd, char* arg);
void handle_buzzer(int fd, char* arg);
void handle_cds(int fd, char* arg);
void handle_seven(int fd, char* arg);
void handle_music(int fd, char* arg);
void command_check(int fd, char* buf_in);

CMD_TYPE client_cmds[] = {
    {"help", handle_help},
    {"LED", handle_led},
    {"buzzer", handle_buzzer},
    {"cds", handle_cds},
    {"seg", handle_seven},
    {"music", handle_music},
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

    char* msg = "잘못된 명령어를 입력했습니다. \"help\"를 입력해 명령어 목록을 확인하세요\n";
    write(fd, msg, strlen(msg));
}

void handle_help(int fd, char* arg) {
    char msg[1024] =
        "================= Command Menu =================\n"
        "*************** (대소문자 구분 없음) *************/\n"
        "\n"
        "  help                         - 도움말 출력\n"
        "  LED     ON/OFF/MAX/MID/MIN   - LED 온오프 및 밝기 제어\n"
        "  buzzer  ON/OFF               - 부저 음악 재생/정지\n"
        "  cds                          - 현재 조도 값 출력\n"
        "  seg     0~9                  - 7-Segment 카운트다운 시작\n"
        "  music   1~3                  - music 종류 선택\n"
        "\n"
        "================================================\n";
    write(fd, msg, strlen(msg));
}

void handle_led(int fd, char* arg) {
    char msg[128];

    if (!arg) {
        sprintf(msg, "[LED] 사용법: LED ON/OFF/MAX/MID/MIN\n");
        write(fd, msg, strlen(msg));
        return;
    }

    if (!strcasecmp(arg, "ON")) {
        led_run = 1;
        softPwmWrite(LED_LED, led_light);
    }
    else if (!strcasecmp(arg, "OFF")) {
        led_run = 0;
        softPwmWrite(LED_LED, 0);
    }
    else if (!strcasecmp(arg, "MAX")) {
        led_light = MAX;
        if (led_run) softPwmWrite(LED_LED, led_light);
    }
    else if (!strcasecmp(arg, "MID")) {
        led_light = MID;
        if (led_run) softPwmWrite(LED_LED, led_light);
    }
    else if (!strcasecmp(arg, "MIN")) {
        led_light = MIN;
        if (led_run) softPwmWrite(LED_LED, led_light);
    }
    else {
        sprintf(msg, "[LED] 잘못된 설정값 입력. ON/OFF/MAX/MID/MIN 중 하나 입력\n");
        write(fd, msg, strlen(msg));
        return;
    }

    sprintf(msg, "[LED] %s 설정 완료 | 현재 밝기 = %d\n", arg, led_light);
    write(fd, msg, strlen(msg));
}

void handle_buzzer(int fd, char* arg) {
    char msg[128];

    if (!arg) {
        sprintf(msg, "[buzzer] 사용법: buzzer ON/OFF\n");
        write(fd, msg, strlen(msg));
        return;
    }

    pthread_mutex_lock(&buzzer_lock);
    if (!strcasecmp(arg, "ON")) {
        music_run = 1;
        sprintf(msg, "[buzzer] ON\n");
    }
    else if (!strcasecmp(arg, "OFF")) {
        music_run = 0;
        sprintf(msg, "[buzzer] OFF\n");
    }
    else {
        sprintf(msg, "[buzzer] 잘못된 설정값 입력. ON/OFF 중 하나 입력\n");
    }
    pthread_mutex_unlock(&buzzer_lock);

    write(fd, msg, strlen(msg));
}

void handle_cds(int fd, char* arg) {
    char msg[128];

    if (arg != NULL) {
        sprintf(msg, "[cds] 사용법: cds (추가 인자 없음)\n");
        write(fd, msg, strlen(msg));
        return;
    }

    pthread_mutex_lock(&cds_lock);
    int val = illuminance;
    pthread_mutex_unlock(&cds_lock);

    if (val > cdsBoundary) {
        sprintf(msg, "[cds] 현재 조도 값 = %d --> 어두움!!\n", val);
    }
    else {
        sprintf(msg, "[cds] 현재 조도 값 = %d --> 밝음!!\n", val);
    }
    write(fd, msg, strlen(msg));
}

void handle_seven(int fd, char* arg) {
    char msg[128];

    if (!arg) {
        sprintf(msg, "[7-Segment] \"seg 7\" 과 같이 0~9의 값을 쓰세요\n");
        write(fd, msg, strlen(msg));
        return;
    }

    if (!(arg[0] >= '0' && arg[0] <= '9' && arg[1] == '\0')) {
        sprintf(msg, "[7-Segment] 0~9초만 가능합니다. 다시 입력하세요.\n");
        write(fd, msg, strlen(msg));
        return;
    }

    int num_input = arg[0] - '0';

    pthread_mutex_lock(&seven_lock);
    if (seven_num >= 0) {
        sprintf(msg, "[7-Segment] 현재 카운트다운 동작 중입니다. 잠시 후 다시 시도\n");
    }
    else {
        seven_num = num_input;
        pthread_cond_signal(&seven_cond);
        sprintf(msg, "[7-Segment] %d 초부터 카운트다운 시작\n", num_input);
    }
    pthread_mutex_unlock(&seven_lock);

    write(fd, msg, strlen(msg));
}

void handle_music(int fd, char* arg) {
    char msg[128];

    if (!arg) {
        sprintf(msg, "[music] \"music 2\" 과 같이 1~3의 값을 쓰세요\n");
        write(fd, msg, strlen(msg));
        return;
    }

    if (!(arg[0] >= '1' && arg[0] <= '3' && arg[1] == '\0')) {
        sprintf(msg, "[music] 1~3 만 가능합니다. 다시 입력하세요.\n");
        write(fd, msg, strlen(msg));
        return;
    }

    int num_input = arg[0] - '0';

    pthread_mutex_lock(&buzzer_lock);
    if ((num_input - 1) == music_current) {
        sprintf(msg, "[music] 이미 %d번 음악 재생 중. 다른 번호를 입력하세요\n", num_input);
    }
    else {
        music_current = num_input - 1;
        sprintf(msg, "[music] 현재 재생 음악 --> %d번 음악\n", num_input);
    }
    pthread_mutex_unlock(&buzzer_lock);

    write(fd, msg, strlen(msg));
}