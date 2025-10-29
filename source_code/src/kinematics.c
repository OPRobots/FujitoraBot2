#include <kinematics.h>

static struct kinematics kinematics_settings[] = {
    [SPEED_TEST] = {
        .linear_speed = 2000,
        .linear_accel = 10000,
        .linear_break = 5000,
        .linear_speed_percent = 11,
        .linear_accel_percent = 50,
        .linear_break_percent = 50,
        .fan_speed = 0,
    },
    [SPEED_BASE] = {
        .linear_speed = 2500,
        .linear_accel = 10000,
        .linear_break = 10000,
        .linear_speed_percent = 14,
        .linear_accel_percent = 50,
        .linear_break_percent = 50,
        .fan_speed = 20,
    },
    [SPEED_NORMAL] = {
        .linear_speed = 3000,
        .linear_accel = 10000,
        .linear_break = 10000,
        .linear_speed_percent = 21,
        .linear_accel_percent = 50,
        .linear_break_percent = 50,
        .fan_speed = 30,
    },
    [SPEED_MEDIUM] = {
        .linear_speed = 3500,
        .linear_accel = 20000,
        .linear_break = 20000,
        .linear_speed_percent = 28,
        .linear_accel_percent = 50,
        .linear_break_percent = 50,
        .fan_speed = 40,
    },
    [SPEED_FAST] = {
        .linear_speed = 4000,
        .linear_accel = 20000,
        .linear_break = 20000,
        .linear_speed_percent = 35,
        .linear_accel_percent = 50,
        .linear_break_percent = 50,
        .fan_speed = 60,
    },
    [SPEED_HAKI] = {
        .linear_speed = 4500,
        .linear_accel = 20000,
        .linear_break = 20000,
        .linear_speed_percent = 42,
        .linear_accel_percent = 50,
        .linear_break_percent = 50,
        .fan_speed = 65,
    }};

static struct kinematics kinematics;

void configure_kinematics(enum speed_strategy speed) {
  kinematics = kinematics_settings[speed];
}

struct kinematics get_kinematics(void) {
  return kinematics;
}