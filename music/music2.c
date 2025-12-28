#include "globals.h"

int notes2[] = {
    659, 659, 0, 659,
    0, 523, 659, 0, 784, 0, 0, 0,
    523, 0, 0, 784, 0, 0, 659, 0, 0, 880, 0, 987, 0, 932, 880, 0,
    784, 659, 784, 880, 0, 698, 784, 0, 659, 0, 523, 587, 494};

int delays2[] = {
    2, 2, 1, 2,
    1, 2, 2, 1, 4, 2, 2, 4,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 1, 3, 1, 1, 4, 2,
    2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 4};

void init_music2(void) {
    musics[1].notes = notes2;
    musics[1].delays = delays2;
    musics[1].length = sizeof(notes2) / sizeof(notes2[0]);
    musics[1].beat = 120;
}