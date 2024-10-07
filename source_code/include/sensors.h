#ifndef __SENSORS_H
#define __SENSORS_H

#include "buttons.h"
#include "config.h"
#include "delay.h"
#include "eeprom.h"
#include "encoders.h"
#include "leds.h"
#include "motors.h"

#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <stdint.h>

#define NUM_ADC_CHANNELS 3
#define NUM_SENSORS 24
#define MUX_CHANNELS (NUM_SENSORS / NUM_ADC_CHANNELS)

uint8_t *get_adc_channels(void);
uint8_t get_adc_channels_num(void);
volatile uint16_t *get_adc_raw(void);
uint8_t get_sensors_num(void);
uint16_t get_sensor_raw(uint8_t pos);
uint16_t get_sensor_calibrated(uint8_t pos);
int32_t get_sensor_line_position(void);

void sensors_calibration(void);

void sensors_load_eeprom(void);
void sensors_print_calibration(void);

void sensors_update_mux_readings(void);
void sensors_update_line_position(void);
void sensors_reset_line_position(void);

#endif