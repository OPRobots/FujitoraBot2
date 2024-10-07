#include "motors.h"

static bool escInited = false;
static uint32_t escInitMillis = 0;

bool is_esc_inited(void) {
  return escInited;
}

void init_esc(void) {
  if (!escInited) {
    if (escInitMillis == 0) {
      escInitMillis = get_clock_ticks();

      timer_enable_oc_output(TIM8, TIM_OC1);
      timer_enable_oc_output(TIM8, TIM_OC2);
      timer_enable_oc_output(TIM8, TIM_OC3);
      timer_enable_oc_output(TIM8, TIM_OC4);
    }
    if (get_clock_ticks() - escInitMillis < 5000) {
      timer_set_oc_value(TIM8, TIM_OC1, MOTORS_STOP_PWM);
      timer_set_oc_value(TIM8, TIM_OC2, MOTORS_STOP_PWM);
      timer_set_oc_value(TIM8, TIM_OC3, MOTORS_STOP_PWM);
      timer_set_oc_value(TIM8, TIM_OC4, MOTORS_STOP_PWM);
    } else {
      escInited = true;
    }
  }
}

void set_motors_speed(float velI, float velD) {
  if (!escInited) {
    return;
  }
  float ocI = MOTORS_STOP_PWM;
  float ocD = MOTORS_STOP_PWM;

  if (velI != 0) {
    ocI = map(velI, 0, 100, MOTORS_STOP_PWM, MOTORS_MAX_PWM);
  }

  if (velD != 0) {
    ocD = map(velD, 0, 100, MOTORS_STOP_PWM, MOTORS_MAX_PWM);
  }
  timer_set_oc_value(TIM8, TIM_OC1, (uint32_t)ocI);
  timer_set_oc_value(TIM8, TIM_OC3, (uint32_t)ocD);
}

void set_motors_pwm(uint32_t pwm_left, uint32_t pwm_right) {
  if (!escInited) {
    return;
  }
  timer_set_oc_value(TIM8, TIM_OC1, pwm_left);
  timer_set_oc_value(TIM8, TIM_OC3, pwm_right);
}

void set_fan_speed(uint8_t vel) {
  if (!escInited) {
    return;
  }

  uint32_t ocF = MOTORS_STOP_PWM;
  if (vel != 0) {
    ocF = map(abs(vel), 0, 100, MOTORS_STOP_PWM, MOTORS_MAX_PWM);
  }
  timer_set_oc_value(TIM8, TIM_OC2, ocF);
  timer_set_oc_value(TIM8, TIM_OC4, ocF);
}