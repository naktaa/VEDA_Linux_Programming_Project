#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include <signal.h>

#define LED_LED 0  /* GPIO 17 */
#define buzzer 2   /* GPIO 27 */
#define LED_CDS 25 /* GPIO 24 */
extern int sevenGPIO[4];
extern int sevenNumber[10][4];

#define I2C_DEV "/dev/i2c-1" /* I2C를 위한 장치 파일 */
#define I2C_ADDR 0x48        /* I2C 장치 주소 */
#define cdsChannel 0         /* 조도 센서 채널 */
#define cdsBoundary 210      /* 조도 센서 밝기 경계값 */

#define DUTY 100 /* LED PWM duty 값 */
#define MAX 80
#define MID 50
#define MIN 25

#define MUSIC_COUNT 3 /* music 개수 */
#define off_delay 30  /* music 재생할 때 */

typedef struct { /* music 관리용 구조체 */
    int* notes;  /* 음계 */
    int* delays; /* 박자 */
    int length;  /* 재생 길이 */
    int beat;    /* 메트로놈 */
} MUSIC;
extern MUSIC musics[MUSIC_COUNT];

extern int notes1[], delays1[];
extern int notes2[], delays2[];
extern int notes3[], delays3[];

typedef void* (*OP_FUNC)(void*);
extern OP_FUNC fbuzzer, fcds, fseven;

volatile sig_atomic_t stop_flag; /* 서버 종료됐는지 flag */
extern pthread_t ptBuzzer, ptCds, ptSeven;

extern pthread_mutex_t buzzer_lock;
extern pthread_mutex_t cds_lock;
extern pthread_mutex_t seven_lock;
extern pthread_cond_t seven_cond;

extern int led_light;     /* LED 밝기 저장 */
extern int led_run;       /* LED ON/OFF 상태*/
extern int music_run;     /* buzzer ON/OFF 상태 */
extern int buzzer_run;    /* 7-segment에서 일시적으로 변경할 state */
extern int music_current; /* 현재 재생되는 음악 번호 */
extern int seven_num;     /* 현재 설정된 7-segment 숫자 값 */
extern int fd_CDS;        /* 조도 센서 파일 디스크럽터 */
extern int illuminance;   /* 현재 조도센서 밝기 값 */

#endif