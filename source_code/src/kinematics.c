#include <kinematics.h>

static struct kinematics kinematics_settings[] = {
    [SPEED_BASE] = {
        .linear_speed = 1250,
        .linear_accel = 5000,
        .linear_break = 5000,
        .linear_speed_percent = 20,
        .linear_accel_percent = 100,
        .linear_break_percent = 100,
        .fan_speed = 35,
    },
    [SPEED_NORMAL] = {
        .linear_speed = 1250,
        .linear_accel = 5000,
        .linear_break = 5000,
        .linear_speed_percent = 20,
        .linear_accel_percent = 15,
        .linear_break_percent = 15,
        .fan_speed = 35,
    },
    [SPEED_MEDIUM] = {
        .linear_speed = 1250,
        .linear_accel = 5000,
        .linear_break = 5000,
        .linear_speed_percent = 20,
        .linear_accel_percent = 15,
        .linear_break_percent = 15,
        .fan_speed = 35,
    },
    [SPEED_FAST] = {
        .linear_speed = 1250,
        .linear_accel = 5000,
        .linear_break = 5000,
        .linear_speed_percent = 20,
        .linear_accel_percent = 15,
        .linear_break_percent = 15,
        .fan_speed = 35,
    },
    [SPEED_HAKI] = {
        .linear_speed = 1250,
        .linear_accel = 5000,
        .linear_break = 5000,
        .linear_speed_percent = 20,
        .linear_accel_percent = 15,
        .linear_break_percent = 15,
        .fan_speed = 35,
    }};

static struct kinematics kinematics;

void configure_kinematics(enum speed_strategy speed) {
  kinematics = kinematics_settings[speed];
}

struct kinematics get_kinematics(void) {
  return kinematics;
}