#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <softTone.h>
#include <pthread.h>

#define buzzer 2                                        /* GPIO 27 */
#define TOTAL 32                                        /* 학교종의 전체 계이름의 수 */
#define BIT 200                                         /* 박자 설정 */
#define music_length (sizeof(notes) / sizeof(notes[0])) /* 음악 전체 계이름의 수 */

extern int buzzer_run;
extern pthread_mutex_t buzzer_lock;

int notes[] = { // 징글벨
    294, 494, 440, 392, 294, 294, 294, 494, 440, 392, 330, 330, 523, 494, 440, 370,
    587, 587, 523, 440, 494, 392, 294, 494, 440, 392, 294, 294, 294, 494, 440, 392, 330, 330, 330, 523, 494, 440,
    587, 587, 587, 587, 659, 587, 494, 440, 392, 494, 494, 494, 494, 494, 494, 494, 587, 392, 440, 494,
    523, 523, 523, 523, 523, 494, 494, 494, 494, 494, 440, 440, 494, 440, 587, 494, 494, 494, 494, 494, 494,
    494, 587, 392, 440, 494, 523, 523, 523, 523, 523, 494, 494, 494, 587, 587, 523, 440, 392};
int delays[] = {
    BIT, BIT, BIT, BIT, BIT * 3, BIT, BIT, BIT, BIT, BIT, BIT * 4, BIT, BIT, BIT, BIT, BIT * 4,
    BIT, BIT, BIT, BIT, BIT * 2, BIT * 2, BIT, BIT, BIT, BIT, BIT * 3, BIT, BIT, BIT, BIT, BIT, BIT * 3, BIT, BIT, BIT, BIT, BIT,
    BIT * 3 / 2, BIT / 2, BIT, BIT, BIT, BIT, BIT, BIT, BIT * 4, BIT, BIT, BIT * 2, BIT, BIT, BIT * 2, BIT, BIT, BIT * 3 / 2, BIT / 2, BIT * 4,
    BIT, BIT, BIT * 3 / 2, BIT / 2, BIT, BIT, BIT, BIT / 2, BIT / 2, BIT, BIT, BIT, BIT, BIT * 2, BIT * 2, BIT, BIT, BIT * 2, BIT, BIT, BIT * 2,
    BIT, BIT, BIT * 3 / 2, BIT / 2, BIT * 4, BIT, BIT, BIT * 3 / 2, BIT / 2, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT * 4};

void* buzzerControl(void* p) {
    softToneCreate(buzzer); /* 톤 출력을 위한 GPIO 설정 */

    while (1) {
        pthread_mutex_lock(&buzzer_lock);
        int run = buzzer_run;
        pthread_mutex_unlock(&buzzer_lock);

        if (!run) {
            delay(20);
            continue;
        }

        for (int i = 0; i < music_length; ++i) {
            // if (seven_finish) {
            //     softToneWrite(buzzer, 500);
            //     delay(500);
            //     seven_finish = 0;
            // }
            pthread_mutex_lock(&buzzer_lock);
            int still_run = buzzer_run;
            pthread_mutex_unlock(&buzzer_lock);

            if (!still_run) {
                break;
            }

            softToneWrite(buzzer, notes[i]);
            // delay(delays[i]);
            delay(delays[i] * 0.9);
            softToneWrite(buzzer, 0);
            delay(delays[i] * 0.1);
        }
    }
}