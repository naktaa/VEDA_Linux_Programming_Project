#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>
#include <softTone.h>
#include <dlfcn.h>
#include "globals.h"
#include "music.h"

static void* handle[3];

void init_modules(void) {
    wiringPiSetup();

    // I2C
    if ((fd_CDS = wiringPiI2CSetupInterface(I2C_DEV, I2C_ADDR)) < 0) {
        perror("wiringPiI2CSetupInterface");
        exit(1);
    }

    // LED, buzzer, cds, 7-segment 설정
    pinMode(LED_LED, OUTPUT);
    softPwmCreate(LED_LED, 0, DUTY);
    softToneCreate(buzzer);
    pinMode(LED_CDS, OUTPUT);
    for (int i = 0; i < 4; i++) {
        pinMode(sevenGPIO[i], OUTPUT);
    }

    // music
    init_music1();
    init_music2();
    init_music3();
    init_music4();

    // 동적 라이브러리 설정
    handle[0] = dlopen("./lib/libbuzzer.so", RTLD_LAZY);
    if (!handle[0]) {
        perror("dlopen");
        exit(1);
    }
    handle[1] = dlopen("./lib/libcds.so", RTLD_LAZY);
    if (!handle[1]) {
        perror("dlopen");
        exit(1);
    }
    handle[2] = dlopen("./lib/libseven.so", RTLD_LAZY);
    if (!handle[2]) {
        perror("dlopen");
        exit(1);
    }

    fbuzzer = (OP_FUNC)dlsym(handle[0], "buzzer_thread");
    fcds = (OP_FUNC)dlsym(handle[1], "cds_thread");
    fseven = (OP_FUNC)dlsym(handle[2], "seven_thread");

    pthread_mutex_init(&buzzer_lock, NULL);
    pthread_mutex_init(&cds_lock, NULL);
    pthread_mutex_init(&seven_lock, NULL);
    pthread_cond_init(&seven_cond, NULL);

    pthread_create(&ptBuzzer, NULL, fbuzzer, NULL);
    pthread_create(&ptCds, NULL, fcds, NULL);
    pthread_create(&ptSeven, NULL, fseven, NULL);
}

void stop_modules(void) {
    stop_flag = 1;

    // thread 정리
    pthread_cond_broadcast(&seven_cond); /* seven_thread 부분 cond 강제로 깨우기*/
    pthread_join(ptBuzzer, NULL);
    pthread_join(ptCds, NULL);
    pthread_join(ptSeven, NULL);

    // mutex 정리
    pthread_mutex_destroy(&buzzer_lock);
    pthread_mutex_destroy(&cds_lock);
    pthread_mutex_destroy(&seven_lock);
    pthread_cond_destroy(&seven_cond);

    // 동적 라이브러리 정리
    for (int i = 0; i < 3; i++) {
        dlclose(handle[i]);
    }

    // 모듈 OFF 처리
    softToneWrite(buzzer, 0);
    softPwmWrite(LED_LED, 0);
    if (fd_CDS >= 0) close(fd_CDS);
}