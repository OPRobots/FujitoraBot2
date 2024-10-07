#ifndef __ENCODERS_H
#define __ENCODERS_H

#include "config.h"
#include "constants.h"
#include "setup.h"


#include <libopencm3/stm32/timer.h>
#include <stdint.h>

int32_t get_encoder_left_ticks(void);
int32_t get_encoder_right_ticks(void);

int32_t get_encoder_left_micrometers(void);
int32_t get_encoder_right_micrometers(void);
int32_t get_encoder_avg_micrometers(void);

int32_t get_encoder_left_millimeters(void);
int32_t get_encoder_right_millimeters(void);
int32_t get_encoder_avg_millimeters(void);

float get_encoder_left_speed(void);
float get_encoder_right_speed(void);
float get_encoder_avg_speed(void);
float get_encoder_angular_speed(void);

float get_encoder_curernt_angle(void);
void reset_encoder_current_angle(void);

int32_t get_encoder_x_position(void);
int32_t get_encoder_y_position(void);

int32_t max_likelihood_counter_diff(uint16_t now, uint16_t before);
void update_encoder_readings(void);

#endif
