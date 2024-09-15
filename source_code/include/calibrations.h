#ifndef CALIBRATIONS_H
#define CALIBRATIONS_H

#include <stdint.h>

#include <delay.h>
#include <eeprom.h>
#include <mpu.h>
#include <sensors.h>

#define CALIBRATE_NONE 0
#define CALIBRATE_GYRO_Z 1
#define CALIBRATE_SENSORS 2
#define CALIBRATE_STORE_EEPROM 3

void calibrate_from_config(uint8_t type);

#endif