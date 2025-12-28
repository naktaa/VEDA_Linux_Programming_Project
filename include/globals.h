#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>

#define LED_LED 0  /* GPIO 17 */
#define buzzer 2   /* GPIO 27 */
#define LED_CDS 25 /* GPIO 24 */
extern int sevenGPIO[4];
extern int sevenNumber[10][4];

#define I2C_DEV "/dev/i2c-1" /* I2C를 위한 장치 파일 */
#define I2C_ADDR 0x48        /* I2C 장치 주소 */
#define cdsChannel 0         /* 조도 센서*/
#define cdsBoundary 200      /* 조도 센서 밝기 경계값 */

#define DUTY 100 /* LED PWM duty 값 */
#define MAX 90
#define MID 50
#define MIN 20

#define MUSIC_COUNT 3 /* music 개수 */

typedef struct { /* music 관리용 구조체 */
    int* notes;
    int* beats;
    int length;
} MUSIC;
extern MUSIC musics[MUSIC_COUNT];

extern int notes1[], delays1[];
extern int notes2[], delays2[];
extern int notes3[], delays3[];

typedef void* (*OP_FUNC)(void*);
extern OP_FUNC fbuzzer, fcds, fseven;

extern pthread_mutex_t buzzer_lock;
extern pthread_mutex_t cds_lock;
extern pthread_mutex_t seven_lock;
extern pthread_cond_t seven_cond;

extern int led_light; /* LED 밝기 저장 */
extern int led_run;
extern int buzzer_run;
extern int music_run;
extern int music_current;
extern int seven_num;
extern int fd_CDS;
extern int illuminance; /* 현재 조도센서 밝기 값 */

#endif