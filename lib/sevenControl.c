#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <softTone.h>
#include <pthread.h>
#include "globals.h"

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

        pthread_mutex_lock(&buzzer_lock);
        pthread_mutex_lock(&seven_lock);
        buzzer_run = 0;
        seven_num = -1;
        pthread_mutex_unlock(&buzzer_lock);
        pthread_mutex_unlock(&seven_lock);

        // seg 동작 끝나면 부저 울리기
        for (int i = 0; i < 3; i++) {
            softToneWrite(buzzer, 784);
            delay(400);
            softToneWrite(buzzer, 0);
            delay(100);
        }

        pthread_mutex_lock(&buzzer_lock);
        buzzer_run = 1;
        pthread_mutex_unlock(&buzzer_lock);
    }
}