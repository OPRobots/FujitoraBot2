#ifndef __LEDS_H
#define __LEDS_H

#include "constants.h"
#include "config.h"
#include "delay.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

void set_status_led(bool state);
void toggle_status_led(void);
void warning_status_led(uint16_t ms);
void set_status_fade(uint32_t f);
void set_status_heartbeat(void);
void set_RGB_color(uint32_t r, uint32_t g, uint32_t b);
void warning_RGB_color(uint32_t r, uint32_t g, uint32_t b, uint16_t ms);
void set_RGB_rainbow(void);
void set_leds_battery_level(float battery_level);
void set_info_led(uint8_t index, bool state); 
void set_info_leds_wave(int ms);
void set_info_leds_blink(int ms);
void set_info_leds(void);
void clear_info_leds(void);
void all_leds_clear(void);

#endif