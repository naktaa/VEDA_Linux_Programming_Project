#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <softTone.h>
#include <pthread.h>
#include "globals.h"

void* seven_thread(void* p) {
    while (1) {
        pthread_mutex_lock(&seven_lock);
        while (seven_num < 0) {
            pthread_cond_wait(&seven_cond, &seven_lock);
        }
        int num = seven_num;
        pthread_mutex_unlock(&seven_lock);

        for (int n = num; n >= 0; n--) {
            for (int i = 0; i < 4; i++) {
                digitalWrite(sevenGPIO[i], sevenNumber[n][i] ? HIGH : LOW); /* FND에 값을 출력 */
            }
            delay(1000);
        }

        int buzzer_save = 0;
        pthread_mutex_lock(&buzzer_lock);
        buzzer_save = buzzer_run;
        buzzer_run = 0;
        pthread_mutex_unlock(&buzzer_lock);

        // seg 동작 끝나면 부저 울리기
        for (int i = 0; i < 3; i++) {
            softToneWrite(buzzer, 784);
            delay(400);
            softToneWrite(buzzer, 0);
            delay(100);
        }
        delay(400);

        pthread_mutex_lock(&buzzer_lock);
        buzzer_run = buzzer_save;
        pthread_mutex_unlock(&buzzer_lock);

        pthread_mutex_lock(&seven_lock);
        seven_num = -1;
        pthread_mutex_unlock(&seven_lock);
    }
}