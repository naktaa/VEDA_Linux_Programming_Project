#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <softTone.h>
#include <pthread.h>
#include "globals.h"

void* buzzer_thread(void* p) {
    int note_idx = 0;      // 연주할 음 index
    int last_music = -1;   // 직전에 재생하던 곡
    int prev_music_on = 0; // 직전의 music_run 상태

    while (!stop_flag) {
        pthread_mutex_lock(&buzzer_lock);
        int buzzer_on = buzzer_run;
        int music_on = music_run;
        int music_num = music_current;
        pthread_mutex_unlock(&buzzer_lock);

        // buzzer off 시키면 index 초기화
        if (!music_on && prev_music_on == 1) {
            note_idx = 0;
        }

        if (!buzzer_on || !music_on) {
            prev_music_on = music_on;
            delay(20);
            continue;
        }

        // 음악 바꾸면 index 초기화하고 last_music 갱신
        if (music_num != last_music) {
            note_idx = 0;
            last_music = music_num;
        }

        // 새로 재생할 때마다 0.5초 딜레이 주기
        if (note_idx == 0) {
            softToneWrite(buzzer, 0);
            delay(500);
        }

        MUSIC m = musics[music_num];

        // 음악 길이 넘어가면 index 초기화
        if (note_idx >= m.length) {
            note_idx = 0; // 반복재생
        }

        softToneWrite(buzzer, m.notes[note_idx]);
        delay(m.delays[note_idx] * m.beat - off_delay);
        softToneWrite(buzzer, 0);
        delay(off_delay);

        note_idx++;
        prev_music_on = music_on;
    }
}