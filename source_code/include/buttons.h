#ifndef __BUTTONS_H
#define __BUTTONS_H

#include "config.h"
#include "delay.h"
#include <math.h>

#include <libopencm3/stm32/gpio.h>
#include <stdio.h>

void check_buttons(void);
bool get_ir_start(void);
bool get_menu_up_btn(void);
bool get_menu_down_btn(void);
bool get_menu_mode_btn(void);

#endif