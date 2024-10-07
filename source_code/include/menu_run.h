#ifndef MENU_RUN_H
#define MENU_RUN_H

#include <buttons.h>
#include <delay.h>
#include <leds.h>

#define MENU_RUN_NUM_MODES 6

enum speed_strategy {
  SPEED_TEST = 0,
  SPEED_BASE = 1,
  SPEED_NORMAL = 2,
  SPEED_MEDIUM = 3,
  SPEED_FAST = 4,
  SPEED_HAKI = 5,
};

enum adaptative_speed {
  ADAPTATIVE_NONE = 0,
  ADAPTATIVE_ACTIVE = 1,
};

enum filter_mode {
  FILTER_NONE = 0,
  FILTER_BIGGER = 1,
};

enum run_type {
  RUN_RACE = 0,
  RUN_DEBUG = 1,
};

enum control_strategy {
  CONTROL_PWM = 0,
  CONTROL_ENCODERS = 1,
};

bool menu_run_handler(void);
void menu_run_reset(void);

void menu_run_load_values(void);

bool menu_run_can_start(void);
int16_t *get_menu_run_values(void);
enum speed_strategy menu_run_get_speed(void);
enum adaptative_speed menu_run_get_adaptative_speed(void);
enum filter_mode menu_run_get_filter_mode(void);
enum run_type menu_run_get_run_type(void);
enum control_strategy menu_run_get_control_strategy(void);

#endif