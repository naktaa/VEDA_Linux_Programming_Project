#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <pthread.h>
#include <softPwm.h>

#define LED 1 /* GPIO18 */

// void ledControl() {
//     pinMode(LED, OUTPUT); /* Pin 모드를 출력으로 설정 */

//     for (;;) {
//         pthread_mutex_lock(&mid);
//         if (light == 0) {
//             digitalWrite(LED, HIGH);
//         }
//         else {
//             digitalWrite(LED, LOW);
//         }
//         pthread_mutex_unlock(&mid);
//         delay(50);
//     }
// }

void PwmControl() {
    pinMode(LED, OUTPUT);       /* Pin의 출력 설정 */
    softPwmCreate(LED, 0, 255); /* PWM의 범위 설정 */

    for (int i = 0; i < 10000; i++) {
        softPwmWrite(LED, i & 255); /* PWM 값을 출력: LED 켜기 */
        delay(5);
    }

    softPwmWrite(LED, 0); /* LED 끄기 */
}

int main() {
    wiringPiSetup(); /* wiringPi 초기화 */

    PwmControl(LED);

    return 0;
}