#ifndef __CONTROL_H
#define __CONTROL_H

#include "battery.h"
#include "config.h"
#include "delay.h"
#include "encoders.h"
#include "kinematics.h"
#include "leds.h"
#include "macroarray.h"
#include "sensors.h"
#include "stdio.h"
#include "usart.h"
#include "utils.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <math.h>

bool is_race_started(void);
void set_race_started(bool state);

void control_loop(void);

bool check_start_run(void);
uint32_t get_race_started_ms(void);
void set_mpu_correction(bool enabled);
void set_line_sensors_correction(bool enabled);
void reset_control_errors(void);
void reset_control_speed(void);
void reset_control_all(void);

void set_target_linear_speed(int32_t linear_speed);
int32_t get_ideal_linear_speed(void);
void set_target_linear_speed_percent(int32_t linear_speed_percent);
void set_target_fan_speed(int32_t fan_speed, int32_t ms);

uint32_t get_emergency_stop_ms(void);
void reset_emergency_stop_ms(void);
void emergency_stop(void);

#endif