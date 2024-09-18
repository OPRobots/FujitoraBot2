#include "sensors.h"

// ADC de derecha a izquierda
static uint8_t adc_channels[NUM_ADC_CHANNELS] = {ADC_CHANNEL10, ADC_CHANNEL11, ADC_CHANNEL12};

static volatile uint16_t adc_raw[NUM_ADC_CHANNELS];
static volatile uint16_t sensores_raw[NUM_SENSORS];
static volatile uint16_t mux_index = 0;

static int16_t sensores_max[NUM_SENSORS];
static int16_t sensores_min[NUM_SENSORS];
static int16_t sensores_umb[NUM_SENSORS];

static volatile int32_t line_position = 0;
static uint32_t ultimaLinea = 0;

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
  return NUM_SENSORS;
}

uint16_t get_sensor_raw(uint8_t pos) {
  if (pos < NUM_SENSORS) {
    return sensores_raw[pos];
  } else {
    return 0;
  }
}

uint16_t get_sensor_calibrated(uint8_t pos) {
  if (pos < NUM_SENSORS) {
    uint16_t sensor_calibrado = get_sensor_raw(pos);

    if (sensor_calibrado >= sensores_umb[pos]) {
      sensor_calibrado = LECTURA_MAXIMO_SENSORES_LINEA;
    } else {
      sensor_calibrado = LECTURA_MINIMO_SENSORES_LINEA;
    }

#ifdef CONFIG_LINE_BLACK
    return sensor_calibrado;
#elif CONFIG_LINE_WHITE
    return LECTURA_MAXIMO_SENSORES_LINEA - sensor_calibrado;
#else
    return sensor_calibrado;
#endif

  } else {
    return 0;
  }
}

int32_t get_sensor_line_position(void) {
  return line_position;
}

void sensors_calibration(void) {
  bool use_eeprom_calibration = true;
  while (!get_menu_mode_btn()) {
    set_status_heartbeat();
    if (get_menu_down_btn()) {
      use_eeprom_calibration = !use_eeprom_calibration;
      set_status_led(use_eeprom_calibration);
      while (get_menu_down_btn()) {
      }
    }
    set_status_led(use_eeprom_calibration);
  }
  while (get_menu_mode_btn()) {
    set_status_heartbeat();
    warning_status_led(75);
  }

  if (use_eeprom_calibration) {
    set_status_led(true);
    bool sensorsMinChecked[get_sensors_num()];
    bool sensorsMaxChecked[get_sensors_num()];
    for (uint8_t sensor = 0; sensor < get_sensors_num(); sensor++) {
      sensorsMinChecked[sensor] = false;
      sensorsMaxChecked[sensor] = false;
    }

    uint8_t countSensorsChecked = 0;
    uint32_t millisSensorsChecked = 0;
    while (!get_menu_mode_btn() && (countSensorsChecked < get_sensors_num() || get_clock_ticks() - millisSensorsChecked < 500)) {
      for (uint8_t sensor = 0; sensor < get_sensors_num(); sensor++) {
        if (abs(get_sensor_raw(sensor) - sensores_min[sensor]) < 200) {
          sensorsMinChecked[sensor] = true;
        }
        if (abs(get_sensor_raw(sensor) - sensores_max[sensor]) < 200 && sensorsMinChecked[sensor]) {
          if (!sensorsMaxChecked[sensor]) {
            printf("Sensor %2d OK\n", (sensor + 1));
          }
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
    set_status_led(false);
    delay(1000);

    // Resetear los valores máximos, mínimos y umbrales
    for (uint8_t sensor = 0; sensor < NUM_SENSORS; sensor++) {
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
    for (int sensor = 0; sensor < get_sensors_num(); sensor++) {
      if (abs(sensores_max[sensor] - sensores_min[sensor]) < 1000) {
        calibrationOK = false;
      }
      sensores_umb[sensor] = sensores_min[sensor] + ((sensores_max[sensor] - sensores_min[sensor]) * 2 / 3);
    }

    eeprom_set_data(DATA_INDEX_SENSORS_MAX, sensores_max, NUM_SENSORS);
    eeprom_set_data(DATA_INDEX_SENSORS_MIN, sensores_min, NUM_SENSORS);
    eeprom_set_data(DATA_INDEX_SENSORS_UMB, sensores_umb, NUM_SENSORS);

    if (calibrationOK) {
      set_RGB_color(0, 100, 0);
      delay(500);
    } else {
      while (!get_menu_mode_btn()) {
        if (calibrationOK) {
          set_RGB_color(0, 100, 0);
        } else if (!calibrationOK) {
          set_RGB_color(100, 0, 0);
        }
        while (get_menu_mode_btn()) {
        };
      }
    }
  }
  set_RGB_color(0, 0, 0);
  sensors_print_calibration();
  delay(250);
}

void sensors_load_eeprom(void) {
  int16_t *eeprom_data = eeprom_get_data();
  for (uint16_t i = DATA_INDEX_SENSORS_MAX; i < DATA_INDEX_SENSORS_MAX + NUM_SENSORS; i++) {
    sensores_max[i - DATA_INDEX_SENSORS_MAX] = eeprom_data[i];
  }
  for (uint16_t i = DATA_INDEX_SENSORS_MIN; i < DATA_INDEX_SENSORS_MIN + NUM_SENSORS; i++) {
    sensores_min[i - DATA_INDEX_SENSORS_MIN] = eeprom_data[i];
  }
  for (uint16_t i = DATA_INDEX_SENSORS_UMB; i < DATA_INDEX_SENSORS_UMB + NUM_SENSORS; i++) {
    sensores_umb[i - DATA_INDEX_SENSORS_UMB] = eeprom_data[i];
  }
  sensors_print_calibration();
}

void sensors_print_calibration(void) {
  for (int sensor = 0; sensor < get_sensors_num(); sensor++) {
    printf("Sensor %2d: %4d <> %4d <> %4d\n", sensor + 1, sensores_min[sensor], sensores_umb[sensor], sensores_max[sensor]);
  }
}

void sensors_update_mux_readings(void) {
  // ! Caution: Parece que al activar el ADC2 (lectura de batería) se desordenan los canales del ADC1 wtf?
  sensores_raw[(MUX_CHANNELS) + mux_index] = adc_raw[1];
  sensores_raw[mux_index] = adc_raw[2];
  sensores_raw[2 * (MUX_CHANNELS) + mux_index] = adc_raw[0];

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

void sensors_update_line_position(void) {
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

  if (sensores_detectando > 0 && sensores_detectando_sin_filtro < get_sensors_num() / 2) {
    ultimaLinea = get_clock_ticks();
  } else if (is_race_started()) {
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

void sensors_reset_line_position(void) {
  line_position = 0;
}