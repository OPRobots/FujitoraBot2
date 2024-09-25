#ifndef __CONFIG_H
#define __CONFIG_H

#include "buttons.h"

#include <stdio.h>

#define MS_CALIBRACION_LINEA 5000
#define LECTURA_MAXIMO_SENSORES_LINEA 4096
#define LECTURA_MINIMO_SENSORES_LINEA 0

// #define KP 0.045
// #define KI 0
// #define KD 2.85

// #define KP_MS 10.0
// #define KI_MS 1.2
// #define KD_MS 20.0

#define KP_LINEAR 0.00025
#define KD_LINEAR 0.00//3

#define KP_ANGULAR 0.04
#define KD_ANGULAR 0.3

#define KP_LINE_SENSORS 0.004
#define KI_LINE_SENSORS 0.00
#define KD_LINE_SENSORS 0.02

#define MILLIS_INICIO_DEBUG 2000
#define MILLIS_INICIO_RUN 5000
#define MILLIS_STOP_FAN 1500
#define TIEMPO_SIN_PISTA 150

#define MICROMETERS_PER_TICK 3.6339
#define WHEELS_SEPARATION 0.12

#define MIN_SPEED_PERCENT 5
#define MAX_ACCEL_PERCENT 45.0
#define MIN_ACCEL_MS2 3000
#define MAX_ACCEL_MS2 8000
#define MAX_BREAK_MS2 10000

/** Divisor de Voltage */
#define VOLT_DIV_FACTOR 3.8673
#define BATTERY_2S_HIGH_LIMIT_VOLTAGE 7.4
#define BATTERY_2S_LOW_LIMIT_VOLTAGE 8.2
#define BATTERY_3S_HIGH_LIMIT_VOLTAGE 12.6
#define BATTERY_3S_LOW_LIMIT_VOLTAGE 11.1

#define CONFIG_RUN_RACE 1
#define CONFIG_RUN_DEBUG 0

#define CONFIG_SPEED_MS 1
#define CONFIG_SPEED_PWM 0

#define CONFIG_LINE_BLACK 0
// #define CONFIG_LINE_WHITE 1

uint16_t get_offtrack_time(void);
uint16_t get_start_millis(void);

#endif