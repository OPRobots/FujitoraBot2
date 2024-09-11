
#include "battery.h"
#include "buttons.h"
#include "config.h"
#include "control.h"
#include "debug.h"
#include "delay.h"
#include "eeprom.h"
#include "encoders.h"
#include "leds.h"
#include "menu.h"
#include "motors.h"
#include "sensors.h"
#include "setup.h"
#include "usart.h"

static uint32_t millis_iniciado = 0;

void sys_tick_handler(void) {
  clock_tick();
  check_buttons();
  if (!is_esc_inited()) {
    init_esc();
  }
  update_battery_voltage();
  update_encoder_readings();
  update_gyro_readings();
  if (is_competicion_iniciada()) {
    update_log();
  }
}

int main(void) {
  setup();
  show_battery_level();
  // gyro_z_calibration();
  set_all_configs();
  eeprom_load();

  do {
    // set_RGB_rainbow();
    // set_info_leds_wave(200);
    // debug_from_config(DEBUG_TYPE_SENSORS_RAW);
    // printf("%x\n", mpu_who_am_i());
    // printf("%.4f\n", get_gyro_z_degrees());
    // set_neon_fade(1024);
    // set_fan_speed(25);
    set_neon_fade((bool)gpio_get(GPIOB, GPIO15) ? 1024 : 0);
  } while (true);

  do {
    check_menu_button();
  } while (in_debug_mode() || !get_start_btn());
  do {
    reset_menu_mode();
    set_RGB_color(0, 0, 0);
  } while (get_start_btn());

  calibrate_sensors();

  eeprom_save();

  millis_iniciado = 0;
  while (1) {
    if (!is_competicion_iniciada()) {
      if (get_millis_emergency_stop() != 0 && get_clock_ticks() - get_millis_emergency_stop() > MILLIS_STOP_FAN) {
        reset_millis_emergency_stop();
        set_fan_speed(0);
      }
      check_menu_button();
      if (!in_debug_mode()) {
        if (get_start_btn()) {
          reset_menu_mode();
          set_status_led(false);
          uint32_t millisInicio = get_clock_ticks();
          uint16_t millisPasados = 5;
          while (get_clock_ticks() < (millisInicio + get_start_millis())) {
            millisPasados = get_clock_ticks() - millisInicio;
            uint8_t r = 0, g = 0;
            r = map(millisPasados, 0, get_start_millis(), 255, 0);
            g = map(millisPasados, 0, 1000, 0, 255);
            set_RGB_color(r, g, 0);
            if ((millisPasados > get_start_millis() * 0.75 || get_start_millis() == 0) && get_base_fan_speed() > 0) {
              set_fan_speed(get_base_fan_speed() * 0.75);
            }
          }
          set_competicion_iniciada(true);
          millis_iniciado = get_clock_ticks();
          set_RGB_color(0, 0, 0);
          set_ideal_motors_speed(get_base_speed());
          set_ideal_motors_ms_speed(get_base_ms_speed());
          set_acceleration_mss(MAX_ACCEL_MS2);
          set_deceleration_mss(MAX_BREAK_MS2);
          set_ideal_fan_speed(get_base_fan_speed());
          set_fan_speed(get_base_fan_speed());
          resume_pid_speed_timer();
        }
      }
    } else {

      if ((get_config_run() == CONFIG_RUN_DEBUG && get_clock_ticks() - millis_iniciado > 5000) || !get_start_btn()) {
        emergency_stop();
      }
    }
  }
}