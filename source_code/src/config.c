#include "config.h"

uint16_t _run_mode;
uint16_t _speed_mode;
uint16_t _track_mode;
uint16_t _robot_type;
uint16_t _track_type;
uint16_t _line_type;

/**
 * @brief Establece la configuración de Carrera
 *
 * @param run_mode CONFIG_RUN_RACE | CONFIG_RUN_DEBUG
 */
static void set_config_run(uint16_t run_mode) {
  _run_mode = run_mode;
}

/**
 * @brief Establece la configuración de Velocidad
 *
 * @param speed_mode CONFIG_SPEED_MS | CONFIG_SPEED_PWM
 */
static void set_config_speed(uint16_t speed_mode) {
  _speed_mode = speed_mode;
}

/**
 * @brief Establece la configuración de Tipo de Pista
 *
 * @param track_type CONFIG_TRACK_TYPE_ENHANCED | CONFIG_TRACK_TYPE_NORMAL
 */
static void set_config_track_type(uint16_t track_type) {
  _track_type = track_type;
}

/**
 * @brief Establece la configuración de Línea
 *
 * @param line_type CONFIG_LINE_WHITE | CONFIG_LINE_BLACK
 */
static void set_config_line(uint16_t line_type) {
  _line_type = line_type;
}

/**
 * @brief Obtiene la configuración de Carrera
 *
 * @return uint16_t CONFIG_RUN_RACE | CONFIG_RUN_DEBUG
 */
uint16_t get_config_run(void) {
  return _run_mode;
}

/**
 * @brief Obtiene la configuración de Velocidad
 *
 * @return uint16_t CONFIG_SPEED_MS | CONFIG_SPEED_PWM
 */
uint16_t get_config_speed(void) {
  return _speed_mode;
}

/**
 * @brief Obtiene la configuración de Línea
 *
 * @return uint16_t CONFIG_LINE_WHITE | CONFIG_LINE_BLACK
 */
uint16_t get_config_line(void) {
  return _line_type;
}

/**
 * @brief Establece todas las configuraciones (CARRERA, VELOCIDAD, PISTA) en funcion de los Switches
 *
 */
void set_all_configs(void) {
  // set_config_run(CONFIG_RUN_RACE);
  set_config_run(CONFIG_RUN_DEBUG);
  
  set_config_speed(CONFIG_SPEED_MS);
  // set_config_speed(CONFIG_SPEED_PWM);

  // set_config_track_type(CONFIG_TRACK_TYPE_ENHANCED);
  set_config_track_type(CONFIG_TRACK_TYPE_NORMAL);

  set_config_line(CONFIG_LINE_BLACK);
  // set_config_line(CONFIG_LINE_WHITE);
}

/**
 * @brief Calcula el tiempo máximo fuera de pista en función de la configuración de carrera/debug
 *
 * @return uint16_t Tiempo máximo fuera de pista
 */
uint16_t get_offtrack_time(void) {
  if (get_config_run() == CONFIG_RUN_RACE) {
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
  if (get_config_run() == CONFIG_RUN_RACE) {
    return MILLIS_INICIO_RUN;
  } else {
    return MILLIS_INICIO_DEBUG;
  }
}