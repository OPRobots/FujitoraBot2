#ifndef MOVE_H
#define MOVE_H

#include "control.h"
#include "menu.h"
#include "menu_run.h"

struct kinematics {
  int16_t linear_speed;
  int16_t linear_accel;
  int16_t linear_break;
  int16_t linear_speed_percent;
  int16_t linear_accel_percent;
  int16_t linear_break_percent;
  int16_t fan_speed;
};

enum speed_strategy;
void configure_kinematics(enum speed_strategy speed);
struct kinematics get_kinematics(void);

#endif