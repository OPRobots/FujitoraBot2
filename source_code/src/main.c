
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

static bool last_ir_start = false;

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
  init_esc();
  check_buttons();
  sensors_update_line_position();
  update_gyro_readings();
  update_battery_voltage();
}

int main(void) {
  setup();
  show_battery_level();
  eeprom_load();

  sensors_calibration();
  
  while (1) {
    if (!is_race_started()) {
      menu_handler();
      if (check_start_run()) {
        menu_run_reset();
        last_ir_start = get_ir_start();
        configure_kinematics(menu_run_get_speed());
        set_race_started(true);
        set_RGB_color(0, 0, 0);
        set_status_led(false);
        clear_info_leds();
        uint32_t millisInicio = get_clock_ticks();
        uint16_t millisPasados = 5;
        while (get_clock_ticks() < (millisInicio + get_start_millis())) {
          millisPasados = get_clock_ticks() - millisInicio;
          uint8_t r = 0, g = 0;
          r = map(millisPasados, 0, get_start_millis(), 255, 0);
          g = map(millisPasados, 0, 1000, 0, 255);
          set_RGB_color(r, g, 0);
          if ((millisPasados > get_start_millis() * 0.75 || get_start_millis() == 0) && get_kinematics().fan_speed > 0) {
            set_target_fan_speed(get_kinematics().fan_speed / 2, 500);
          }
        }
        set_RGB_color(0, 0, 0);
        set_target_linear_speed(get_kinematics().linear_speed);
        set_target_linear_speed_percent(get_kinematics().linear_speed_percent);
        set_target_fan_speed(get_kinematics().fan_speed, 500);
        set_line_sensors_correction(true);
      }
    } else {
      if ((menu_run_get_run_type() == RUN_DEBUG && get_clock_ticks() - get_race_started_ms() > (uint32_t)(3000 + get_start_millis())) || (!get_ir_start() && last_ir_start)) {
        emergency_stop();
        last_ir_start = get_ir_start();
      }
      if (get_ir_start()) {
        last_ir_start = true;
      }
    }
  };
}