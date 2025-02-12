#include "config.h"

/**
 * @brief Calcula el tiempo máximo fuera de pista en función de la configuración de carrera/debug
 *
 * @return uint16_t Tiempo máximo fuera de pista
 */
uint16_t get_offtrack_time(void) {
#ifdef CONFIG_RUN_RACE
  return (uint16_t)(TIEMPO_SIN_PISTA * 1.33);
#elif CONFIG_RUN_DEBUG
  return TIEMPO_SIN_PISTA;
#else
  return TIEMPO_SIN_PISTA;
#endif
}

/**
 * @brief Calcula el tiempo máximo fuera de pista en función de la configuración de carrera/debug
 *
 * @return uint16_t Tiempo máximo fuera de pista
 */
uint16_t get_start_millis(void) {
#ifdef CONFIG_RUN_RACE
  return MILLIS_INICIO_RUN;
#elif CONFIG_RUN_DEBUG
  return MILLIS_INICIO_DEBUG;
#else
  return MILLIS_INICIO_DEBUG;
#endif
}