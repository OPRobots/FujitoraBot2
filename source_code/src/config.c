#include "config.h"


/**
 * @brief Calcula el tiempo máximo fuera de pista en función de la configuración de carrera/debug
 *
 * @return uint16_t Tiempo máximo fuera de pista
 */
uint16_t get_offtrack_time(void) {
  if (menu_run_get_run_type() == RUN_RACE) {
    return (uint16_t)(TIEMPO_SIN_PISTA * 1.33);
  } else {
    return TIEMPO_SIN_PISTA;
  }
}

/**
 * @brief Calcula el tiempo máximo fuera de pista en función de la configuración de carrera/debug
 *
 * @return uint16_t Tiempo máximo fuera de pista
 */
uint16_t get_start_millis(void) {
  if (menu_run_get_run_type() == RUN_RACE) {
    return MILLIS_INICIO_RUN;
  } else {
    return MILLIS_INICIO_DEBUG;
  }
}