#include "debug.h"

bool debug_enabled = false;
uint32_t last_print_debug = 0;

static void debug_macroarray(void) {
  macroarray_print();
  debug_enabled = false;
  menu_config_reset_values();
}

/**
 * @brief Imprime los valores de los sensores sin aplicar ninguna corrección
 *
 */
static void debug_sensors_raw(void) {
  if (get_clock_ticks() > last_print_debug + 125) {

    // printf("us: %ld - ", get_us_readings_elapsed());
    for (int8_t sensor = 0; sensor < get_sensors_num(); sensor++) {
      printf("%4d", get_sensor_raw(sensor));
      if (sensor < get_sensors_num() - 1) {
        printf("|");
      }
    }
    printf("\n");
    last_print_debug = get_clock_ticks();
  }
}

/**
 * @brief Imprime los valores de los sensores calibrándolos y escalandolos
 *
 */
static void debug_sensors_calibrated(void) {
  if (get_clock_ticks() > last_print_debug + 50) {
    for (int8_t sensor = get_sensors_num() - 1; sensor >= 0; sensor--) {
      printf("%d\t", get_sensor_calibrated(sensor));
    }
    printf("\n");
    last_print_debug = get_clock_ticks();
  }
}

static void debug_all_leds(void) {
  set_RGB_rainbow();
  set_status_heartbeat();
  warning_status_led(125);
}

static void debug_digital_io(void) {
  if (get_clock_ticks() > last_print_debug + 250) {
    printf("BTN: %d CFM: %d CFU: %d CFD: %d\n", get_ir_start(), get_menu_mode_btn(), get_menu_up_btn(), get_menu_down_btn());
    last_print_debug = get_clock_ticks();
  }
}

static void debug_posicion_correccion(void) {
  if (get_clock_ticks() > last_print_debug + 50) {
    sensors_update_line_position();
    printf("%ld\n", get_sensor_line_position());
    last_print_debug = get_clock_ticks();
  }
}

static void debug_line_position(void) {
  if (get_clock_ticks() > last_print_debug + 50) {
    sensors_update_line_position();
    printf("%d\t%ld\t%d\n", -(get_sensors_num() + 2) * 1000 / 2, get_sensor_line_position(), (get_sensors_num() + 2) * 1000 / 2);
    last_print_debug = get_clock_ticks();
  }
}

static void debug_encoders(void) {
  if (get_clock_ticks() > last_print_debug + 50) {
    printf("%ld (%ld)\t%ld (%ld)\n", get_encoder_left_ticks(), get_encoder_left_micrometers(), get_encoder_right_ticks(), get_encoder_right_micrometers());
    last_print_debug = get_clock_ticks();
  }
}

static void debug_motors(void) {
  if (is_esc_inited()) {
    set_motors_speed(15, 15);
  }
}

static void debug_fans(void) {
  if (is_esc_inited()) {
    set_fan_speed(30);
  }
}

static void check_debug_active(void) {
  if (get_menu_mode_btn()) {
    uint32_t pressed_ms = get_clock_ticks();
    while (get_menu_mode_btn()) {
    }
    if (get_clock_ticks() - pressed_ms <= 500) {
      debug_enabled = !debug_enabled;
    } else {
      debug_enabled = false;
    }
    delay(50);
  }
  if (debug_enabled) {
    set_RGB_rainbow();
  } else {
    set_RGB_color(0, 0, 0);
  }
}

bool is_debug_enabled(void) {
  return debug_enabled;
}

void debug_from_config(uint8_t type) {
  if (type != DEBUG_NONE) {
    check_debug_active();
  } else {
    debug_enabled = false;
  }
  if (is_debug_enabled()) {
    switch (type) {
      case DEBUG_MACROARRAY:
        debug_macroarray();
        break;
      case DEBUG_TYPE_SENSORS_RAW:
        debug_sensors_raw();
        break;
      case DEBUG_TYPE_SENSORS_CALIBRATED:
        debug_sensors_calibrated();
        break;
      case DEBUG_TYPE_LINE_POSITION:
        debug_line_position();
        break;
      case DEBUG_TYPE_MOTORS:
        debug_motors();
        break;
      case DEBUG_TYPE_ENCODERS:
        debug_encoders();
        break;
      case DEBUG_TYPE_DIGITAL_IO:
        debug_digital_io();
        break;
      case DEBUG_TYPE_CORRECCION_POSICION:
        debug_posicion_correccion();
        break;
      case DEBUG_TYPE_LEDS_PARTY:
        debug_all_leds();
        break;
      case DEBUG_TYPE_FANS_DEMO:
        debug_fans();
        break;
    }
  } else {
    set_RGB_color(0, 0, 0);
    switch (type) {
      case DEBUG_TYPE_MOTORS:
        set_motors_speed(0, 0);
        break;
      case DEBUG_TYPE_LEDS_PARTY:
        all_leds_clear();
        break;
      case DEBUG_TYPE_FANS_DEMO:
        set_fan_speed(0);
        break;
    }
  }
}

void debug_sensors_calibration(void) {
  if (get_clock_ticks() > last_print_debug + 1000) {
    sensors_print_calibration();
    last_print_debug = get_clock_ticks();
  }
}