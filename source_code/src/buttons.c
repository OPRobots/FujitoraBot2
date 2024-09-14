#include "buttons.h"

uint32_t ir_start_ms = 0;
uint32_t btn_menu_up_ms = 0;
uint32_t btn_menu_down_ms = 0;
uint32_t btn_menu_mode_ms = 0;

void check_buttons(void) {
  if ((bool)gpio_get(GPIOB, GPIO15)) {
    if (ir_start_ms == 0) {
      ir_start_ms = get_clock_ticks();
    }
  } else {
    ir_start_ms = 0;
  }
  if ((bool)gpio_get(GPIOB, GPIO14)) {
    if (btn_menu_up_ms == 0) {
      btn_menu_up_ms = get_clock_ticks();
    }
  } else {
    btn_menu_up_ms = 0;
  }
  if ((bool)gpio_get(GPIOB, GPIO13)) {
    if (btn_menu_down_ms == 0) {
      btn_menu_down_ms = get_clock_ticks();
    }
  } else {
    btn_menu_down_ms = 0;
  }
  if ((bool)gpio_get(GPIOB, GPIO12)) {
    if (btn_menu_mode_ms == 0) {
      btn_menu_mode_ms = get_clock_ticks();
    }
  } else {
    btn_menu_mode_ms = 0;
  }
}

/**
 * @brief Obtiene el estado del botón de inicio
 *
 * @return bool
 */
bool get_ir_start(void) {
  return ir_start_ms > 0 && get_clock_ticks() - ir_start_ms > 25;
}

/**
 * @brief Obtiene el estado del botón de Menú Arriba
 *
 * @return bool
 */
bool get_menu_up_btn(void) {
  return btn_menu_up_ms > 0 && get_clock_ticks() - btn_menu_up_ms > 50;
}

/**
 * @brief Obtiene el estado del botón de Menú Abajo
 *
 * @return bool
 */
bool get_menu_down_btn(void) {
  return btn_menu_down_ms > 0 && get_clock_ticks() - btn_menu_down_ms > 50;
}

/**
 * @brief Obtiene el estado del botón de Menú Modo
 *
 * @return bool
 */
bool get_menu_mode_btn(void) {
  return btn_menu_mode_ms > 0 && get_clock_ticks() - btn_menu_mode_ms > 50;
}
