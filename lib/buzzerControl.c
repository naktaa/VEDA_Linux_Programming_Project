#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <softTone.h>
#include <pthread.h>
#include "globals.h"

void* buzzer_thread(void* p) {
    while (1) {
        pthread_mutex_lock(&buzzer_lock);
        int buzzer_on = buzzer_run;
        int music_on = music_run;
        int music_num = music_current;
        pthread_mutex_unlock(&buzzer_lock);

        if (!buzzer_on || !music_on) {
            delay(20);
            continue;
        }

        MUSIC m = musics[music_num];

        for (int i = 0; i < m.length; i++) {
            pthread_mutex_lock(&buzzer_lock);
            buzzer_on = buzzer_run;
            music_on = music_run;
            int music_num_check = music_current;
            pthread_mutex_unlock(&buzzer_lock);

            if (!buzzer_on || !music_on || music_num != music_num_check) {
                break;
            }

            softToneWrite(buzzer, m.notes[i]);
            delay(m.beats[i] * 0.9);
            softToneWrite(buzzer, 0);
            delay(m.beats[i] * 0.1);
        }

        // 재생마다 0.5초 딜레이 주기
        softToneWrite(buzzer, 0);
        delay(500);
    }
}