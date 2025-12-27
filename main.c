#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "init.h"
#include "server.h"
#include "globals.h"

int main(int argc, char** argv) {
    if (daemon(1, 0) == -1) {
        perror("daemon");
        exit(1);
    }

    init_modules(); // dlopen, dlsym, thread 생성
    run_server();   // 소켓/epoll 생성 + 무한 루프

    return 0;
}