#include "sensors.h"

// ADC de derecha a izquierda
static uint8_t adc_channels[NUM_ADC_CHANNELS] = {ADC_CHANNEL10, ADC_CHANNEL11, ADC_CHANNEL12};

static volatile uint16_t adc_raw[NUM_ADC_CHANNELS];
static volatile uint16_t sensores_raw[NUM_SENSORES];
static volatile uint16_t mux_index = 0;

static uint16_t sensores_max[NUM_SENSORES];
static uint16_t sensores_min[NUM_SENSORES];
static uint16_t sensores_umb[NUM_SENSORES];

static volatile int32_t line_position = 0;
static uint32_t ultimaLinea = 0;
static int32_t ticks_ultima_interseccion = 0;

static uint32_t millis_emergency_stop = 0;

static volatile uint32_t us_readings_start = 0;
static volatile uint32_t us_readings_elapsed = 0;

/**
 * @brief Inicializa los sensores cuando se enciende el robot sin calibrarlos a partir de una calibración previa
 *
 */
static void assign_sensors_calibrations(void) {
  uint16_t *eeprom_data = eeprom_get_data();
  for (uint16_t i = DATA_INDEX_SENSORS_MAX; i < NUM_SENSORES; i++) {
    sensores_max[i] = eeprom_data[i];
  }
  for (uint16_t i = DATA_INDEX_SENSORS_MIN; i < DATA_INDEX_SENSORS_MIN + NUM_SENSORES; i++) {
    sensores_min[i - DATA_INDEX_SENSORS_MIN] = eeprom_data[i];
  }
  for (uint16_t i = DATA_INDEX_SENSORS_UMB; i < DATA_INDEX_SENSORS_UMB + NUM_SENSORES; i++) {
    sensores_umb[i - DATA_INDEX_SENSORS_UMB] = eeprom_data[i];
  }
}

void print_sensors_calibrations(void) {
  for (int sensor = 0; sensor < get_sensors_num(); sensor++) {
    printf("sensores_max[%d] = %d;\n", sensor, sensores_max[sensor]);
    printf("sensores_min[%d] = %d;\n", sensor, sensores_min[sensor]);
    printf("sensores_umb[%d] = %d;\n", sensor, sensores_umb[sensor]);
  }
}

uint8_t *get_adc_channels(void) {
  return adc_channels;
}

uint8_t get_adc_channels_num(void) {
  return NUM_ADC_CHANNELS;
}

volatile uint16_t *get_adc_raw(void) {
  return adc_raw;
}

uint8_t get_sensors_num(void) {
  return NUM_SENSORES;
}

uint16_t get_sensor_raw(uint8_t pos) {
  if (pos < NUM_SENSORES) {
    return sensores_raw[pos];
  } else {
    return 0;
  }
}

uint32_t get_us_readings_elapsed(void) {
  return us_readings_elapsed;
}

void update_sensors_readings(void) {
  // ! Caution: Parece que al activar el ADC2 (lectura de batería) se desordenan los canales del ADC1 wtf?
  sensores_raw[(MUX_CHANNELS) + mux_index] = adc_raw[0];
  sensores_raw[mux_index] = adc_raw[1];
  sensores_raw[2 * (MUX_CHANNELS) + mux_index] = adc_raw[2];
  // if(mux_index == 0){
  //   us_readings_start = read_cycle_counter() * (float)MICROSECONDS_PER_SECOND / (float)SYSCLK_FREQUENCY_HZ;
  // }else if(mux_index == 7){
  //   us_readings_elapsed = (read_cycle_counter() * (float)MICROSECONDS_PER_SECOND / (float)SYSCLK_FREQUENCY_HZ) - us_readings_start;
  // }

  mux_index = (mux_index + 1) % MUX_CHANNELS;
  int c = GPIO13;
  int b = GPIO14;
  int a = GPIO15;
  switch (mux_index) {
    case 0: // 000
      gpio_clear(GPIOC, a | b | c);
      break;
    case 1: // 001
      gpio_set(GPIOC, a);
      gpio_clear(GPIOC, c | b);
      break;
    case 2: // 010
      gpio_set(GPIOC, b);
      gpio_clear(GPIOC, c | a);
      break;
    case 3: // 011
      gpio_set(GPIOC, b | a);
      gpio_clear(GPIOC, c);
      break;
    case 4: // 100
      gpio_set(GPIOC, c);
      gpio_clear(GPIOC, b | a);
      break;
    case 5: // 101
      gpio_set(GPIOC, c | a);
      gpio_clear(GPIOC, b);
      break;
    case 6: // 110
      gpio_set(GPIOC, c | b);
      gpio_clear(GPIOC, a);
      break;
    case 7: // 111
      gpio_set(GPIOC, a | b | c);
      break;
  }
  delay_us(3);
}

uint16_t get_sensor_calibrated(uint8_t pos) {
  if (pos < NUM_SENSORES) {
    uint16_t sensor_calibrado = get_sensor_raw(pos);

    if (sensor_calibrado >= sensores_umb[pos]) {
      sensor_calibrado = LECTURA_MAXIMO_SENSORES_LINEA;
    } else {
      sensor_calibrado = LECTURA_MINIMO_SENSORES_LINEA;
    }

    if (get_config_line() == CONFIG_LINE_BLACK) {
      return sensor_calibrado;
    } else {
      return LECTURA_MAXIMO_SENSORES_LINEA - sensor_calibrado;
    }
  } else {
    return 0;
  }
}

void calibrate_sensors(void) {
  assign_sensors_calibrations();
  bool use_eeprom_calibration = true;
  while (!get_start_btn()) {
  set_neon_heartbeat();
  if (get_menu_mode_btn()) {
  use_eeprom_calibration = !use_eeprom_calibration;
  set_status_led(use_eeprom_calibration);
  while (get_menu_mode_btn()) {
  }
    }
    set_status_led(use_eeprom_calibration);
  }
  while (get_start_btn()) {
  set_neon_heartbeat();
  warning_status_led(75);
  }

  if (use_eeprom_calibration) {
    bool sensorsMinChecked[get_sensors_num()];
    bool sensorsMaxChecked[get_sensors_num()];
    for (uint8_t sensor = 0; sensor < get_sensors_num(); sensor++) {
      sensorsMinChecked[sensor] = false;
      sensorsMaxChecked[sensor] = false;
    }

    uint8_t countSensorsChecked = 0;
    uint32_t millisSensorsChecked = 0;
    while (!get_start_btn() && (countSensorsChecked < get_sensors_num() || get_clock_ticks() - millisSensorsChecked < 500)) {
      for (uint8_t sensor = 0; sensor < get_sensors_num(); sensor++) {
        if (abs(get_sensor_raw(sensor) - sensores_min[sensor]) < 200) {
          sensorsMinChecked[sensor] = true;
        }
        if (abs(get_sensor_raw(sensor) - sensores_max[sensor]) < 200 && sensorsMinChecked[sensor]) {
          sensorsMaxChecked[sensor] = true;
        }
      }

      countSensorsChecked = 0;
      for (uint8_t sensor = 0; sensor < get_sensors_num(); sensor++) {
        if (sensorsMinChecked[sensor] && sensorsMaxChecked[sensor]) {
          countSensorsChecked++;
        }
      }

      if (countSensorsChecked == 0) {
        set_RGB_color(125, 0, 0);
      } else if (countSensorsChecked < get_sensors_num()) {
        set_RGB_color(125, 125, 0);
      } else {
        set_RGB_color(0, 125, 0);
        if (millisSensorsChecked == 0) {
          millisSensorsChecked = get_clock_ticks();
        }
      }
    }
  } else {
    delay(1000);

    // Resetear los valores máximos, mínimos y umbrales
    for (uint8_t sensor = 0; sensor < NUM_SENSORES; sensor++) {
      sensores_max[sensor] = LECTURA_MINIMO_SENSORES_LINEA;
      sensores_min[sensor] = LECTURA_MAXIMO_SENSORES_LINEA;
      sensores_umb[sensor] = LECTURA_MINIMO_SENSORES_LINEA;
    }

    uint32_t ms_inicio = get_clock_ticks();
    while (ms_inicio + MS_CALIBRACION_LINEA >= get_clock_ticks()) {
      for (int sensor = 0; sensor < get_sensors_num(); sensor++) {
        if (sensores_raw[sensor] < sensores_min[sensor]) {
          sensores_min[sensor] = sensores_raw[sensor];
        }
        if (sensores_raw[sensor] > sensores_max[sensor]) {
          sensores_max[sensor] = sensores_raw[sensor];
        }
      }

      set_RGB_rainbow();
    }

    bool calibrationOK = true;
    bool marksOK = true;
    for (int sensor = 0; sensor < get_sensors_num(); sensor++) {
      if (abs(sensores_max[sensor] - sensores_min[sensor]) < 1000) {
        calibrationOK = false;
      }
      sensores_umb[sensor] = sensores_min[sensor] + ((sensores_max[sensor] - sensores_min[sensor]) * 2 / 3);
    }

    eeprom_set_data(DATA_INDEX_SENSORS_MAX, sensores_max, NUM_SENSORES);
    eeprom_set_data(DATA_INDEX_SENSORS_MIN, sensores_min, NUM_SENSORES);
    eeprom_set_data(DATA_INDEX_SENSORS_UMB, sensores_umb, NUM_SENSORES);

    if (calibrationOK && marksOK) {
      set_RGB_color(0, 100, 0);
      delay(500);
    } else {
      while (!get_start_btn()) {
        if (calibrationOK && marksOK) {
          set_RGB_color(0, 100, 0);
        } else if (!calibrationOK) {
          set_RGB_color(100, 0, 0);
        } else if (!marksOK) {
          set_RGB_color(100, 20, 0);
        }
      }
      while (get_start_btn()) {
        if (calibrationOK) {
          set_RGB_color(0, 100, 0);
        } else {
          set_RGB_color(100, 0, 0);
        }
      }
    }
  }
  set_RGB_color(0, 0, 0);
  delay(250);
  }

int32_t get_sensor_line_position(void) {
  return line_position;
}

void calc_sensor_line_position(void) {
  uint32_t suma_sensores_ponderados = 0;
  uint32_t suma_sensores = 0;
  uint8_t sensores_detectando = 0;
  uint8_t sensores_detectando_sin_filtro = 0;
  // Obtener los sensores importantes para el cálculo de posición
  int8_t sensor_ini_linea = -1;
  int8_t sensor_fin_linea = -1;
  int8_t sensor_inicial = round(map(line_position, -1000, 1000, -1, get_sensors_num()));
  if (sensor_inicial <= 0) {
    sensor_ini_linea = 0;
    sensor_fin_linea = 3;
  } else if (sensor_inicial >= get_sensors_num() - 1) {
    sensor_ini_linea = get_sensors_num() - 4;
    sensor_fin_linea = get_sensors_num() - 1;
  } else {
    sensor_ini_linea = sensor_inicial - 2;
    sensor_fin_linea = sensor_inicial + 1;
    if (sensor_ini_linea < 0) {
      sensor_fin_linea += abs(sensor_ini_linea);
      sensor_ini_linea = 0;
    }
    if (sensor_fin_linea >= get_sensors_num()) {
      sensor_ini_linea -= abs(sensor_fin_linea - get_sensors_num());
      sensor_fin_linea = get_sensors_num() - 1;
    }
  }

  // printf("%d - %d\t|\t", sensor_ini_linea, sensor_fin_linea);

  for (uint8_t sensor = 0; sensor < get_sensors_num(); sensor++) {
    uint16_t sensor_value = get_sensor_calibrated(sensor);

    if (sensor_value >= sensores_umb[sensor]) {
      sensores_detectando_sin_filtro++;
    }
    if (sensor < sensor_ini_linea || sensor > sensor_fin_linea) {
      sensor_value = LECTURA_MINIMO_SENSORES_LINEA;
    }

    if (sensor_value >= sensores_umb[sensor]) {
      sensores_detectando++;
    }
    suma_sensores_ponderados += (sensor + 1) * sensor_value * 1000;
    suma_sensores += sensor_value;
  }

  if (sensores_detectando > 3) {
    ticks_ultima_interseccion = (get_encoder_left_total_ticks() + get_encoder_right_total_ticks()) / 2;
  }

  if (sensores_detectando > 0 && sensores_detectando_sin_filtro < get_sensors_num() / 2) {
    ultimaLinea = get_clock_ticks();
  } else if (is_competicion_iniciada()) {
    if (get_clock_ticks() > (ultimaLinea + get_offtrack_time())) {
            emergency_stop();
    }
    if (abs(line_position) < 800) {
      return;
    }
  }

  int32_t posicion_max = ((1000 * (get_sensors_num() + 1)) / 2);

  int32_t posicion;
  if (sensores_detectando > 0) {
    posicion = (suma_sensores_ponderados / suma_sensores) - posicion_max;
  } else {
    posicion = (line_position >= 0) ? posicion_max : -posicion_max;
  }
  posicion = map(posicion, -posicion_max, posicion_max, -1000, 1000);

  line_position = posicion;
}

uint32_t get_millis_emergency_stop(void) {
  return millis_emergency_stop;
}

void reset_millis_emergency_stop(void) {
  millis_emergency_stop = 0;
}

void emergency_stop(void) {
  set_competicion_iniciada(false);
  pause_pid_speed_timer();
  line_position = 0;
  millis_emergency_stop = get_clock_ticks();
}