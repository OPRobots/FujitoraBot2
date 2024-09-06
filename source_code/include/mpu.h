/*
    Reference: https://github.com/Bulebots/bulebule
*/

#ifndef __MPU_H
#define __MPU_H

#include <stdint.h>

#include "buttons.h"
#include "constants.h"
#include "delay.h"
#include "eeprom.h"
#include "leds.h"
#include "math.h"
#include "motors.h"
#include "setup.h"

void mpu_set_updating(bool updating);

uint8_t mpu_who_am_i(void);

void setup_mpu(void);
void gyro_z_calibration(void);
void update_gyro_readings(void);
float get_gyro_z_degrees(void);
int16_t get_gyro_z_raw(void);
float get_gyro_z_radps(void);
float get_gyro_z_dps(void);
void set_z_angle(float angle);
void keep_z_angle(void);
void set_gyro_z_degrees(float deg);

void mpu_load_eeprom(void);

#endif /* __MPU_H */
