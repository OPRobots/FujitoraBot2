#ifndef __DEBUG_H
#define __DEBUG_H

#include "buttons.h"
#include "config.h"
#include "control.h"
#include "delay.h"
#include "encoders.h"
#include "macroarray.h"
#include "motors.h"
#include "sensors.h"
#include "usart.h"

#include <stdint.h>

#define DEBUG_NONE 0
#define DEBUG_MACROARRAY 1
#define DEBUG_TYPE_SENSORS_RAW 2
#define DEBUG_TYPE_SENSORS_CALIBRATED 3
#define DEBUG_TYPE_LINE_POSITION 4
#define DEBUG_TYPE_MOTORS 5
#define DEBUG_TYPE_ENCODERS 6
#define DEBUG_TYPE_DIGITAL_IO 7
#define DEBUG_TYPE_CORRECCION_POSICION 8
#define DEBUG_TYPE_LEDS_PARTY 9
#define DEBUG_TYPE_FANS_DEMO 10

bool is_debug_enabled(void);
void debug_from_config(uint8_t type);

void debug_sensors_calibration(void);

#endif