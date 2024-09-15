#ifndef __EEPROM_H
#define __EEPROM_H

#include <libopencm3/stm32/flash.h>

#include <delay.h>
#include <leds.h>
#include <menu.h>
#include <sensors.h>

#define EEPROM_SECTOR 11
#define EEPROM_BASE_ADDRESS (0x080E0000)

#define DATA_LENGTH (1 + (NUM_SENSORS * 3) + MENU_RUN_NUM_MODES)

#define DATA_INDEX_GYRO_Z 0
#define DATA_INDEX_SENSORS_MAX (DATA_INDEX_GYRO_Z + 1)
#define DATA_INDEX_SENSORS_MIN (DATA_INDEX_SENSORS_MAX + NUM_SENSORS)
#define DATA_INDEX_SENSORS_UMB (DATA_INDEX_SENSORS_MIN + NUM_SENSORS)
#define DATA_INDEX_MENU_RUN (DATA_INDEX_SENSORS_UMB + NUM_SENSORS)

void eeprom_save(void);
void eeprom_load(void);
void eeprom_clear(void);
void eeprom_set_data(uint16_t index, int16_t *data, uint16_t length);
int16_t *eeprom_get_data(void);

#endif
