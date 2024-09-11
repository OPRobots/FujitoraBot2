#ifndef __CONFIG_H
#define __CONFIG_H

#include "buttons.h"

#include <stdio.h>

#define MS_CALIBRACION_LINEA 5000
#define LECTURA_MAXIMO_SENSORES_LINEA 4096
#define LECTURA_MINIMO_SENSORES_LINEA 0

#define KP 0.045
#define KI 0
#define KD 2.85

#define KP_MS 10.0
#define KI_MS 1.2
#define KD_MS 20.0

#define MILLIS_INICIO_DEBUG 2000
#define MILLIS_INICIO_RUN 5000
#define MILLIS_STOP_FAN 1500
#define TIEMPO_SIN_PISTA 150

#define MICROMETERS_PER_TICK 19.5925
#define WHEELS_SEPARATION 0.1169

#define MIN_SPEED_PERCENT 5
#define MAX_ACCEL_PERCENT 45.0
#define MIN_ACCEL_MS2 3.0
#define MAX_ACCEL_MS2 8.0
#define MAX_BREAK_MS2 10.0

/** Divisor de Voltage */
#define VOLT_DIV_FACTOR 3.8673
#define BATTERY_HIGH_LIMIT_VOLTAGE 12.6
#define BATTERY_LOW_LIMIT_VOLTAGE 11.1

#define CONFIG_RUN_RACE 1
#define CONFIG_RUN_DEBUG 0

#define CONFIG_SPEED_MS 1
#define CONFIG_SPEED_PWM 0

#define CONFIG_TRACK_TYPE_ENHANCED 1
#define CONFIG_TRACK_TYPE_NORMAL 0

#define CONFIG_LINE_WHITE 1
#define CONFIG_LINE_BLACK 0

void set_all_configs(void);
uint16_t get_config_run(void);
uint16_t get_config_speed(void);
uint16_t get_config_line(void);

uint16_t get_offtrack_time(void);
uint16_t get_start_millis(void);

#endif