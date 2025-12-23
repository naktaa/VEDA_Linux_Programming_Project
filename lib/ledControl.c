#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <pthread.h>
#include <softPwm.h>

#define LED 2 /* GPIO18 */
pthread_mutex_t led_mid, pwm_lid;

void ledControl() {
    pinMode(LED, OUTPUT); /* Pin 모드를 출력으로 설정 */

    for (;;) {
        digitalWrite(LED, HIGH);
        delay(250);
        digitalWrite(LED, LOW);
        delay(250);
    }
}

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
    pthread_t ptLed, ptPwm;
    pthread_mutex_init(&led_mid, NULL);
    pthread_mutex_init(&pwm_mid, NULL);

    pthread_create(&ptLed, NULL, ledControl, NULL);
    pthread_create(&ptPwm, NULL, PwmControl, NULL);

    ledControl();
    // PwmControl();

    return 0;
}