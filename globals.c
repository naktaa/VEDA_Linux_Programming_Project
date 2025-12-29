#include "globals.h"

int sevenGPIO[4] = {4, 1, 16, 15}; /* A, B, C, D : 23 18 15 14 */
// 7-segment 핀 배치
int sevenNumber[10][4] = {{0, 0, 0, 0},  /* 0 */
                          {0, 0, 0, 1},  /* 1 */
                          {0, 0, 1, 0},  /* 2 */
                          {0, 0, 1, 1},  /* 3 */
                          {0, 1, 0, 0},  /* 4 */
                          {0, 1, 0, 1},  /* 5 */
                          {0, 1, 1, 0},  /* 6 */
                          {0, 1, 1, 1},  /* 7 */
                          {1, 0, 0, 0},  /* 8 */
                          {1, 0, 0, 1}}; /* 9 */

int led_light = 50;
int led_run = 0;
int buzzer_run = 1;
int music_run = 0;
int music_current = 2;
int seven_num = -1;
int fd_CDS = -1;
int illuminance = 0;

MUSIC musics[MUSIC_COUNT];

OP_FUNC fbuzzer = NULL;
OP_FUNC fcds = NULL;
OP_FUNC fseven = NULL;

volatile stop_flag;
pthread_t ptBuzzer, ptCds, ptSeven;

pthread_mutex_t buzzer_lock;
pthread_mutex_t cds_lock;
pthread_mutex_t seven_lock;
pthread_cond_t seven_cond;