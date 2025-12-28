#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>
#include <softTone.h>
#include <dlfcn.h>
#include "globals.h"

void init_modules(void) {
    void* handle[3];
    pthread_t ptBuzzer, ptCds, ptSeven;

    wiringPiSetup();

    // I2C
    if ((fd_CDS = wiringPiI2CSetupInterface(I2C_DEV, I2C_ADDR)) < 0) {
        perror("wiringPiI2CSetupInterface");
        exit(1);
    }

    pinMode(LED_LED, OUTPUT);
    softPwmCreate(LED_LED, 0, 100);
    softToneCreate(buzzer);
    pinMode(LED_CDS, OUTPUT);
    for (int i = 0; i < 4; i++) {
        pinMode(sevenGPIO[i], OUTPUT);
    }

    init_music1();
    init_music2();
    init_music3();

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

    fbuzzer = (OP_FUNC)dlsym(handle[0], "buzzerControl");
    fcds = (OP_FUNC)dlsym(handle[1], "cdsControl");
    fseven = (OP_FUNC)dlsym(handle[2], "sevenControl");

    pthread_mutex_init(&buzzer_lock, NULL);
    pthread_mutex_init(&cds_lock, NULL);
    pthread_mutex_init(&seven_lock, NULL);
    pthread_cond_init(&seven_cond, NULL);

    pthread_create(&ptBuzzer, NULL, fbuzzer, NULL);
    pthread_create(&ptCds, NULL, fcds, NULL);
    pthread_create(&ptSeven, NULL, fseven, NULL);
}