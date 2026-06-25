# Calibración

FujitoraBot2 dispone de procedimientos de calibración para el giroscopio y los
sensores de línea. Los valores calibrados se persisten en la EEPROM emulada en
flash.

## Tipos de calibración

| ID | Tipo | Descripción |
|----|------|-------------|
| `0` | `CALIBRATE_NONE` | Sin calibración activa |
| `1` | `CALIBRATE_GYRO_Z` | Calibración del offset Z del giroscopio |
| `2` | `CALIBRATE_SENSORS` | Calibración de los 24 sensores de línea |
| `3` | `CALIBRATE_STORE_EEPROM` | Guardado manual a EEPROM |

Se accede desde el menú Config → Calibration.

## Calibración del giroscopio (eje Z)

El procedimiento `gyro_z_calibration()` en [`mpu.c:144-161`](../source_code/src/mpu.c#L144-L161):

1. **Requisito**: el robot debe estar completamente inmóvil
2. Se toman 300 muestras del eje Z con un filtro de media móvil (factor 2)
3. Se calcula el offset como el complemento a 2 del promedio
4. El offset se escribe directamente en los registros `MPU_Z_OFFS_USR_H/L`
   del MPU-6500 (compensación por hardware)
5. El valor se guarda en EEPROM para la próxima inicialización

| Parámetro | Valor |
|-----------|-------|
| `MPU_CAL_SAMPLE_NUM` | 300 |
| `MPU_CAL_SAMPLE_US` | 1000 µs |
| `MPU_AVERAGE_FACTOR` | 2 |

## Calibración de sensores de línea

El procedimiento `sensors_calibration()` en
[`sensors.c:80-193`](../source_code/src/sensors.c#L80-L193) soporta dos modos:

### Modo verificación (EEPROM)

Si los valores de calibración ya están en EEPROM:

1. El LED de estado muestra `true` (encendido)
2. Se pasa el robot sobre la línea para verificar que cada sensor detecta
   tanto el mínimo como el máximo dentro de ±200 del valor almacenado
3. LED RGB: rojo = ningún sensor OK, amarillo = algunos OK, verde = todos OK
4. Espera 500 ms adicionales con todos los sensores OK antes de confirmar

### Modo calibración completa

Si se desactiva el uso de EEPROM (botón Down):

1. Se resetean mínimos a 4096 y máximos a 0
2. Durante 5 segundos (`MS_CALIBRACION_LINEA = 5000`) se mueve el robot sobre
   la línea para que cada sensor vea tanto blanco como negro
3. LED RGB muestra arcoíris durante el barrido
4. Se calculan los umbrales como `min + (max − min) × 2/3`
5. Si la diferencia max−min es < 1000 en algún sensor, se muestra error (rojo)
6. Al confirmar, se guardan los 3×24 valores en EEPROM

### Formato de salida

Cada sensor se imprime por UART como:
```
Sensor XX: MIN <> UMB <> MAX
```

(ver [Sensores](03-sensors.md))

## Guardado en EEPROM

La calibración del giroscopio y los sensores se guardan independientemente:

- **Automático**: `gyro_z_calibration()` guarda el offset tras calibrar
- **Automático**: `sensors_calibration()` guarda los 72 valores tras calibrar
- **Manual**: opción `CALIBRATE_STORE_EEPROM` fuerza un guardado completo

(ver [EEPROM](09-storage.md))

---

*Documento generado el 2026-06-25. Ver también [Sensores](03-sensors.md), [Encoders y Giroscopio](10-encoders-gyro.md), [EEPROM](09-storage.md).*
