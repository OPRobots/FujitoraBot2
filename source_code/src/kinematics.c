#include <kinematics.h>

static struct kinematics kinematics_settings[] = {
    [SPEED_TEST] = {
        .linear_speed = 2000,
        .linear_accel = 10000,
        .linear_break = 5000,
        .linear_speed_percent = 27,
        .linear_accel_percent = 100,
        .linear_break_percent = 100,
        .fan_speed = 20,
    },
    [SPEED_BASE] = {
        .linear_speed = 2500,
        .linear_accel = 10000,
        .linear_break = 10000,
        .linear_speed_percent = 33,
        .linear_accel_percent = 100,
        .linear_break_percent = 100,
        .fan_speed = 30,
    },
    [SPEED_NORMAL] = {
        .linear_speed = 3000,
        .linear_accel = 10000,
        .linear_break = 10000,
        .linear_speed_percent = 39,
        .linear_accel_percent = 100,
        .linear_break_percent = 100,
        .fan_speed = 40,
    },
    [SPEED_MEDIUM] = {
        .linear_speed = 3500,
        .linear_accel = 20000,
        .linear_break = 20000,
        .linear_speed_percent = 45,
        .linear_accel_percent = 100,
        .linear_break_percent = 100,
        .fan_speed = 50,
    },
    [SPEED_FAST] = {
        .linear_speed = 4000,
        .linear_accel = 10000,
        .linear_break = 10000,
        .linear_speed_percent = 51,
        .linear_accel_percent = 100,
        .linear_break_percent = 100,
        .fan_speed = 65,
    },
    [SPEED_HAKI] = {
        .linear_speed = 4500,
        .linear_accel = 10000,
        .linear_break = 10000,
        .linear_speed_percent = 57,
        .linear_accel_percent = 100,
        .linear_break_percent = 100,
        .fan_speed = 80,
    }};

static struct kinematics kinematics;

void configure_kinematics(enum speed_strategy speed) {
  kinematics = kinematics_settings[speed];
}

struct kinematics get_kinematics(void) {
  return kinematics;
}