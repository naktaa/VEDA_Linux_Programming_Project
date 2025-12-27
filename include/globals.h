#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>

#define LED_LED 0  /* GPIO 17 */
#define buzzer 2   /* GPIO 27 */
#define LED_CDS 25 /* GPIO 24 */
extern int sevenGPIO[4];

#define I2C_DEV "/dev/i2c-1" /* I2C를 위한 장치 파일 */
#define I2C_ADDR 0x48        /* I2C 장치 주소 */
#define cdsChannel 0         /* 조도 센서*/
#define cdsBoundary 160      /* 조도 센서 밝기 경계값 */

#define ON 255
#define OFF 0
#define MAX 210
#define MID 130
#define MIN 50

#define BIT 200 /* 음악 재생 박자 */

typedef void* (*OP_FUNC)(void*);
extern OP_FUNC fbuzzer, fcds, fseven;

extern pthread_mutex_t buzzer_lock;
extern pthread_mutex_t seven_lock;

extern int buzzer_run;
extern int seven_num;
extern int seven_finish;
extern int fd_CDS;
extern int illuminance;

#endif