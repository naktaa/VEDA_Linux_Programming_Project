#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <pthread.h>

#define I2C_DEV "/dev/i2c-1" /* I2C를 위한 장치 파일 */
#define I2C_ADDR 0x48        /* I2C 장치 주소 */
#define cdsChannel 0         /* 조도 센서*/
#define cdsBoundary 160      /* 조도 센서 밝기 경계값 */
#define LED_CDS 25           /* GPIO 24 */

extern int fd_CDS;
extern int illuminance;

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