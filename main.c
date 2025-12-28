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

    init_modules(); // 모듈들 init 설정
    run_server();   // 서버 동작

    return 0;
}