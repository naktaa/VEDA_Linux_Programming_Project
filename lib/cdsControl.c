#include <stdio.h>
#include <wiringPi.h>
#include <pthread.h>

#define CDS 0 /* GPIO17 */

extern int light;
extern pthread_mutex_t mid;

void* cdsControl(void* p) {
    pinMode(CDS, INPUT); /* Pin 모드를 입력으로 설정 */

    for (;;) {
        pthread_mutex_lock(&mid);
        if (digitalRead(CDS) == HIGH) {
            light = 1;
        }
        else {
            light = 0;
        }
        pthread_mutex_unlock(&mid);
        delay(50);
    }
}