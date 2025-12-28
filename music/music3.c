#include "globals.h"

#define BIT 200 /* 음악 재생 박자 */

const int notes3[] = {
    294, 494, 440, 392, 294, 294, 294, 494, 440, 392, 330, 330, 523, 494, 440, 370,
    587, 587, 523, 440, 494, 392, 294, 494, 440, 392, 294, 294, 294, 494, 440, 392, 330, 330, 330, 523, 494, 440,
    587, 587, 587, 587, 659, 587, 494, 440, 392, 494, 494, 494, 494, 494, 494, 494, 587, 392, 440, 494,
    523, 523, 523, 523, 523, 494, 494, 494, 494, 494, 440, 440, 494, 440, 587, 494, 494, 494, 494, 494, 494,
    494, 587, 392, 440, 494, 523, 523, 523, 523, 523, 494, 494, 494, 587, 587, 523, 440, 392};
const int delays3[] = {
    BIT, BIT, BIT, BIT, BIT * 3, BIT, BIT, BIT, BIT, BIT, BIT * 4, BIT, BIT, BIT, BIT, BIT * 4,
    BIT, BIT, BIT, BIT, BIT * 2, BIT * 2, BIT, BIT, BIT, BIT, BIT * 3, BIT, BIT, BIT, BIT, BIT, BIT * 3, BIT, BIT, BIT, BIT, BIT,
    BIT * 3 / 2, BIT / 2, BIT, BIT, BIT, BIT, BIT, BIT, BIT * 4, BIT, BIT, BIT * 2, BIT, BIT, BIT * 2, BIT, BIT, BIT * 3 / 2, BIT / 2, BIT * 4,
    BIT, BIT, BIT * 3 / 2, BIT / 2, BIT, BIT, BIT, BIT / 2, BIT / 2, BIT, BIT, BIT, BIT, BIT * 2, BIT * 2, BIT, BIT, BIT * 2, BIT, BIT, BIT * 2,
    BIT, BIT, BIT * 3 / 2, BIT / 2, BIT * 4, BIT, BIT, BIT * 3 / 2, BIT / 2, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT * 4};

void init_music2(void) {
    musics[2].notes = notes3;
    musics[2].beats = delays3;
    musics[2].length = sizeof(notes3) / sizeof(notes3[0]);
}