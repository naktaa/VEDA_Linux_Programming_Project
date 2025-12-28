#include "globals.h"

int sevenGPIO[4] = {4, 1, 16, 15}; /* A, B, C, D : 23 18 15 14 */

int led_light = 50;
int led_run = 0;
int buzzer_run = 1;
int music_run = 1;
int music_current = 0;
int seven_num = -1;
int fd_CDS = -1;
int illuminance = 0;

MUSIC musics[MUSIC_COUNT] = {
    {notes1, delays1, sizeof(notes1) / sizeof(notes1[0])},
    {notes2, delays2, sizeof(notes2) / sizeof(notes2[0])},
    {notes3, delays3, sizeof(notes3) / sizeof(notes3[0])},
};

OP_FUNC fbuzzer = NULL;
OP_FUNC fcds = NULL;
OP_FUNC fseven = NULL;

pthread_mutex_t buzzer_lock;
pthread_mutex_t music_lock;
pthread_mutex_t cds_lock;
pthread_mutex_t seven_lock;
pthread_cond_t seven_cond;