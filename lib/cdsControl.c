#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <pthread.h>
#include "globals.h"

void* cdsControl(void* p) {
    while (1) {
        wiringPiI2CWrite(fd_CDS, 0x00 | cdsChannel);
        illuminance = wiringPiI2CRead(fd_CDS);
        delay(5);
        illuminance = wiringPiI2CRead(fd_CDS);

        if (illuminance > cdsBoundary) {
            digitalWrite(LED_CDS, HIGH);
        }
        else {
            digitalWrite(LED_CDS, LOW);
        }

        printf("[ cds ] 현재 조도 값 = %d\n", illuminance);
        delay(1000);
    }
}