#include "control.h"

static bool race_started = false;
static uint32_t race_start_ms = 0;
static uint32_t race_finish_ms = 0;
static uint32_t emergency_stop_ms = 0;
static bool ir_before_start = false;

static volatile int32_t target_linear_speed = 0;
static volatile int32_t ideal_linear_speed = 0;

static volatile int32_t target_linear_speed_percent = 0;
static volatile float ideal_linear_speed_percent = 0;

static volatile float ideal_angular_speed = 0.0;

static volatile int32_t target_fan_speed = 0;
static volatile float ideal_fan_speed = 0;
static volatile float fan_speed_accel = 0;

static volatile float linear_error;
static volatile float last_linear_error;

static volatile float angular_error;
static volatile float last_angular_error;

static volatile bool mpu_correction_enabled = false;
static volatile bool line_sensors_correction_enabled = false;

static volatile float line_sensors_error;
static volatile float sum_line_sensors_error;
static volatile float last_line_sensors_error;

static volatile float voltage_left;
static volatile float voltage_right;
static volatile int32_t pwm_left;
static volatile int32_t pwm_right;

/**
 * @brief Convierte un valor de voltaje dado a su correspondiente PWM
 *
 * @param voltage
 * @return int32_t PWM a aplicar al motor
 */
static int32_t voltage_to_motor_pwm(float voltage) {
  if (voltage <= 0) { // TODO: definir un voltage mínimo de funcionamiento
    return 0;
  } else {
    return voltage / /* 12.0  */ get_battery_voltage() * (MOTORS_MAX_PWM / 2);
  }
}

static void update_ideal_linear_speed(void) {
  switch (menu_run_get_control_strategy()) {
    case CONTROL_ENCODERS:
      if (ideal_linear_speed < target_linear_speed) {
        ideal_linear_speed += get_kinematics().linear_accel / CONTROL_FREQUENCY_HZ;
        if (ideal_linear_speed > target_linear_speed) {
          ideal_linear_speed = target_linear_speed;
        }
      } else if (ideal_linear_speed > target_linear_speed) {
        ideal_linear_speed -= get_kinematics().linear_break / CONTROL_FREQUENCY_HZ;
        if (ideal_linear_speed < target_linear_speed) {
          ideal_linear_speed = target_linear_speed;
        }
      }
      break;
    case CONTROL_PWM:
      if (ideal_linear_speed_percent < target_linear_speed_percent) {
        ideal_linear_speed_percent += (float)(get_kinematics().linear_accel_percent) / CONTROL_FREQUENCY_HZ;
        if (ideal_linear_speed_percent > target_linear_speed_percent) {
          ideal_linear_speed_percent = target_linear_speed_percent;
        }
      } else if (ideal_linear_speed_percent > target_linear_speed_percent) {
        ideal_linear_speed_percent -= get_kinematics().linear_break_percent / CONTROL_FREQUENCY_HZ;
        if (ideal_linear_speed_percent < target_linear_speed_percent) {
          ideal_linear_speed_percent = target_linear_speed_percent;
        }
      }

      break;
  }
}

static void update_fan_speed(void) {
  if (ideal_fan_speed < target_fan_speed) {
    ideal_fan_speed += fan_speed_accel / CONTROL_FREQUENCY_HZ;
    if (ideal_fan_speed > target_fan_speed) {
      ideal_fan_speed = target_fan_speed;
    }
  } else if (ideal_fan_speed > target_fan_speed) {
    ideal_fan_speed -= fan_speed_accel / CONTROL_FREQUENCY_HZ;
    if (ideal_fan_speed < target_fan_speed) {
      ideal_fan_speed = target_fan_speed;
    }
  }
}

static float get_measured_linear_speed(void) {
  return (get_encoder_left_speed() + get_encoder_right_speed()) / 2.0f;
}

static float get_measured_angular_speed(void) {
  return -get_gyro_z_radps();
}

/**
 * @brief Comprueba si el robot está en funcionamiento
 *
 * @return bool
 */

bool is_race_started(void) {
  return race_started;
}

/**
 * @brief Establece el estado actual del robot
 *
 * @param state Estado actual del robot
 */

void set_race_started(bool state) {
  race_started = state;
  if (state) {
    race_start_ms = get_clock_ticks();
  } else {
    menu_reset();
    race_finish_ms = get_clock_ticks();
  }
}

uint32_t get_race_started_ms(void) {
  return race_start_ms;
}

bool check_start_run(void) {
  if (menu_run_can_start()) {
    if (get_ir_start() && !ir_before_start) {
      return true;
    } else if (get_menu_mode_btn()) {
      uint32_t ms_pressed = get_clock_ticks();
      while (get_menu_mode_btn()) {
        if (get_clock_ticks() - ms_pressed > 1000) {
          warning_status_led(25);
        }
      }
      if (get_clock_ticks() - ms_pressed > 1200) {
        return true;
      }
    }
  } else {
    ir_before_start = get_ir_start();
  }
  return false;
}

void set_mpu_correction(bool enabled) {
  mpu_correction_enabled = enabled;
}

void set_line_sensors_correction(bool enabled) {
  line_sensors_correction_enabled = enabled;
}

void reset_control_errors(void) {
  line_sensors_error = 0;
  sum_line_sensors_error = 0;
  last_line_sensors_error = 0;
}

void reset_control_speed(void) {
  target_linear_speed = 0;
  ideal_linear_speed = 0;
  target_linear_speed_percent = 0;
  ideal_linear_speed_percent = 0;
  ideal_angular_speed = 0.0;
  voltage_left = 0;
  voltage_right = 0;
  pwm_left = 0;
  pwm_right = 0;
}

void reset_control_all(void) {
  reset_control_errors();
  reset_control_speed();
}

void set_target_linear_speed(int32_t linear_speed) {
  target_linear_speed = linear_speed;
}

int32_t get_ideal_linear_speed(void) {
  return ideal_linear_speed;
}

void set_target_linear_speed_percent(int32_t linear_speed_percent) {
  target_linear_speed_percent = linear_speed_percent;
}

void set_target_fan_speed(int32_t fan_speed, int32_t ms) {
  target_fan_speed = fan_speed;
  fan_speed_accel = (fan_speed - ideal_fan_speed) * CONTROL_FREQUENCY_HZ / ms;
}

/**
 * @brief Función ISR del Timer5 encargada del control de posición y velocidad
 *
 */
void control_loop(void) {
  if (!is_race_started()) {
    set_motors_speed(0, 0);
    if (race_finish_ms > 0 && get_clock_ticks() - race_finish_ms >= 3000) {
      set_fan_speed(0);
    }
    return;
  }

  update_ideal_linear_speed();
  update_fan_speed();

  sensors_update_line_position();

  float linear_voltage = 0;

  switch (menu_run_get_control_strategy()) {
    case CONTROL_ENCODERS:
      last_linear_error = linear_error;
      linear_error += ideal_linear_speed - get_measured_linear_speed();
      linear_voltage = KP_LINEAR * linear_error + KD_LINEAR * (linear_error - last_linear_error);
      break;
    case CONTROL_PWM:
      linear_voltage = get_battery_full_voltage() * ideal_linear_speed_percent / 100.0f;
      break;
  }

  last_angular_error = angular_error;
  if (mpu_correction_enabled) {
    angular_error += ideal_angular_speed - get_measured_angular_speed();
  }

  if (line_sensors_correction_enabled) {
    last_line_sensors_error = line_sensors_error;
    line_sensors_error = get_sensor_line_position();
    sum_line_sensors_error += line_sensors_error;
    // printf("%.4f | ", line_sensors_error);
  }

  float angular_voltage =
      KP_ANGULAR * angular_error + KD_ANGULAR * (angular_error - last_angular_error) +
      KP_LINE_SENSORS * line_sensors_error + KI_LINE_SENSORS * sum_line_sensors_error + KD_LINE_SENSORS * (line_sensors_error - last_line_sensors_error);

  voltage_left = linear_voltage + angular_voltage;
  voltage_right = linear_voltage - angular_voltage;
  pwm_left = MOTORS_STOP_PWM + voltage_to_motor_pwm(voltage_left);
  pwm_right = MOTORS_STOP_PWM + voltage_to_motor_pwm(voltage_right);
  if (voltage_left != 0 && pwm_left < MOTORS_MIN_PWM) {
    pwm_left = MOTORS_MIN_PWM;
  }
  if (voltage_right != 0 && pwm_right < MOTORS_MIN_PWM) {
    pwm_right = MOTORS_MIN_PWM;
  }
  // printf("%ld - %ld\n", pwm_left, pwm_right);
  set_motors_pwm(pwm_left, pwm_right);

  macroarray_store(
      0,
      0b11100,
      5,
      (int16_t)(get_measured_linear_speed() * 100.0),
      (int16_t)(get_measured_angular_speed() * 100.0),
      (int16_t)(get_gyro_z_degrees() * 100.0),
      (int16_t)get_encoder_x_position(),
      (int16_t)get_encoder_y_position());

  // set_fan_speed(ideal_fan_speed);
}

uint32_t get_emergency_stop_ms(void) {
  return emergency_stop_ms;
}

void reset_emergency_stop_ms(void) {
  emergency_stop_ms = 0;
}

void emergency_stop(void) {
  set_race_started(false);
  emergency_stop_ms = get_clock_ticks();
  reset_control_all();
}