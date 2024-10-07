#include "battery.h"

static float battery_full_voltage = BATTERY_3S_LOW_LIMIT_VOLTAGE;
static volatile float battery_voltage = 0;

void update_battery_voltage(void) {
  // ! Caution: Parece que al activar el ADC2 (lectura de batería) se desordenan los canales del ADC1 wtf?
  adc_start_conversion_regular(ADC2);
  while (!adc_eoc(ADC2)) {
  }
  float voltage = adc_read_regular(ADC2) * ADC_LSB * VOLT_DIV_FACTOR;
  if (battery_voltage == 0) {
    battery_voltage = voltage;
  } else {
    battery_voltage = BATTERY_VOLTAGE_LOW_PASS_FILTER_ALPHA * voltage + (1 - BATTERY_VOLTAGE_LOW_PASS_FILTER_ALPHA) * battery_voltage;
  }
}

float get_battery_full_voltage(void) {
  return battery_full_voltage;
}

float get_battery_voltage(void) {
  return battery_voltage;
}

void show_battery_level(void) {
  uint32_t ticksInicio = get_clock_ticks();
  float voltage = get_battery_voltage();
  float battery_level = 0;
  if (voltage >= BATTERY_3S_LOW_LIMIT_VOLTAGE && voltage <= BATTERY_3S_HIGH_LIMIT_VOLTAGE) {
    battery_full_voltage = BATTERY_3S_HIGH_LIMIT_VOLTAGE;
    battery_level = map(voltage, BATTERY_3S_LOW_LIMIT_VOLTAGE, BATTERY_3S_HIGH_LIMIT_VOLTAGE, 0.0f, 100.0f);
  } else if (voltage >= BATTERY_2S_LOW_LIMIT_VOLTAGE && voltage <= BATTERY_2S_HIGH_LIMIT_VOLTAGE) {
    battery_full_voltage = BATTERY_2S_HIGH_LIMIT_VOLTAGE;
    battery_level = map(voltage, BATTERY_2S_LOW_LIMIT_VOLTAGE, BATTERY_2S_HIGH_LIMIT_VOLTAGE, 0.0f, 100.0f);
  }
  while (get_clock_ticks() < ticksInicio + 750) {
    set_leds_battery_level(battery_level);
  }
  all_leds_clear();
}
