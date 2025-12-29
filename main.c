#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "init.h"
#include "server.h"
#include "globals.h"

void sig_handler(int signo) {
    stop_flag = 1;
}

int main(int argc, char** argv) {
    // if (daemon(1, 0) == -1) {
    //     perror("daemon");
    //     exit(1);
    // }

    // struct sigaction act;
    // act.sa_handler = sig_handler;
    // sigemptyset(&act.sa_mask);
    // act.sa_flags = SA_RESTART;
    // sigaction(SIGTERM, &act, NULL); // kill 신호

    init_modules(); // 모듈들 init 설정
    run_server();   // 서버 동작
    stop_modules(); // 모든 자원 정리

    return 0;
}