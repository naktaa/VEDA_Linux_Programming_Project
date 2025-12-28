#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <pthread.h>
#include "globals.h"

void* cdsControl(void* p) {
    while (1) {
        wiringPiI2CWrite(fd_CDS, 0x00 | cdsChannel);
        int val = wiringPiI2CRead(fd_CDS);
        delay(5);
        val = wiringPiI2CRead(fd_CDS);

        pthread_mutex_lock(&cds_lock);
        illuminance = val;
        pthread_mutex_unlock(&cds_lock);

        if (val > cdsBoundary) {
            digitalWrite(LED_CDS, HIGH);
        }
        else {
            digitalWrite(LED_CDS, LOW);
        }

        delay(1000);
    }
}