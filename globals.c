#include "globals.h"

int sevenGPIO[4] = {4, 1, 16, 15}; /* A, B, C, D : 23 18 15 14 */

int led_light = 50;
int led_run = 0;
int buzzer_run = 1;
int seven_num = -1;
int seven_finish = 0;
int fd_CDS = -1;
int illuminance = 0;

OP_FUNC fbuzzer = NULL;
OP_FUNC fcds = NULL;
OP_FUNC fseven = NULL;

pthread_mutex_t buzzer_lock;
pthread_mutex_t seven_lock;