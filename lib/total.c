#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <pthread.h>
#include <softPwm.h>
#include <softTone.h>

#define LED 0                     /* GPIO18 */
#define buzzer 2                  /* GPIO25 */
#define music_length 36           /* 학교종의 전체 계이름의 수 */
#define I2C_DEV "/dev/i2c-1";     /* I2C를 위한 장치 파일 */
#define I2C_ADDR 0x48             // I2C 장치 주소
#define Channel 0;                // 조도 센서
int gpiopins[4] = {4, 1, 16, 15}; /* A, B, C, D : 23 18 15 14 */
int fd;

void* PwmControl(void* p);
void* musicPlay(void* p);
void* fndControl(void* p);

// int notes[] = {/* 학교종을 연주하기 위한 계이름 */
//                391, 391, 440, 440, 391, 391, 329.63, 329.63,
//                391, 391, 329.63, 329.63, 293.66, 293.66, 293.66, 0,
//                391, 391, 440, 440, 391, 391, 329.63, 329.63,
//                391, 329.63, 293.66, 329.63, 261.63, 261.63, 261.63, 0};

int notes[] = {/* 슈퍼 마리오 브라더스 오버월드 테마 */
               659, 659, 0, 659, 0, 523, 659, 0,
               784, 0, 392, 0,
               523, 0, 392, 0, 330, 0,
               440, 0, 494, 0, 466, 440,
               392, 659, 784, 880,
               698, 784, 0, 659,
               523, 587, 494, 0};

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

int main() {
    pthread_t ptPwm, ptBuzzer, ptSeven;

    if ((fd = wiringPiI2CSetup(I2C_ADDR)) < 0) {
        printf("wiringPiI2CSetup failed:\n");
    }

    if ((fd = wiringPiI2CSetupInterface("/dev/i2c-1", I2C_ADDR)) < 0) {
        printf("wiringPiI2CSetupInterface failed:\n");
    }

    wiringPiSetup(); /* wiringPi 초기화 */
    pthread_create(&ptPwm, NULL, PwmControl, NULL);
    pthread_create(&ptBuzzer, NULL, musicPlay, NULL);
    pthread_create(&ptSeven, NULL, fndControl, NULL);

    for (;;)
        ;

    return 0;
}

void* PwmControl(void* p) {
    pinMode(LED, OUTPUT);       /* Pin의 출력 설정 */
    softPwmCreate(LED, 0, 255); /* PWM의 범위 설정 */

    while (1) {
        for (int i = 0; i < 10000; i++) {
            softPwmWrite(LED, i & 255); /* PWM 값을 출력: LED 켜기 */
            delay(5);
        }
    }
    // softPwmWrite(LED, 0); /* LED 끄기 */
}

void* musicPlay(void* p) {
    softToneCreate(buzzer); /* 톤 출력을 위한 GPIO 설정 */

    while (1) {
        for (int i = 0; i < 32; ++i) {
            softToneWrite(buzzer, notes[i]); /* 톤 출력 : 학교종 연주 */
            delay(280);                      /* 음의 전체 길이만큼 출력되도록 대기 */
        }
    }
}

void* fndControl(void*) {
    for (int i = 0; i < 4; i++) {
        pinMode(gpiopins[i], OUTPUT); /* 모든 Pin의 출력 설정 */
    }

    while (1) {
        for (int num = 0; num < 10; num++) {
            for (int i = 0; i < 4; i++) {
                digitalWrite(gpiopins[i], number[num][i] ? HIGH : LOW); /* FND에 값을 출력 */
            }
            delay(250);
        }
    }
}