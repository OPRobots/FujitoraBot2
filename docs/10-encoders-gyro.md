# Encoders y Giroscopio

FujitoraBot2 combina dos sistemas de navegación inercial: encoders magnéticos en
cuadratura para odometría y un giroscopio MPU-6500 para medición de velocidad
angular. Ambos se actualizan en el SysTick a 1 kHz.

## Encoders en cuadratura

### Hardware

| Característica | Detalle |
|---------------|---------|
| **Tipo** | Encoders magnéticos en cuadratura |
| **Timer izquierdo** | TIM4 (PB6, PB7) — modo encoder 3 (TI1+TI2) |
| **Timer derecho** | TIM3 (PB4, PB5) — modo encoder 3 (TI1+TI2) |
| **Resolución** | 16 bits (0–65535) |
| **Micrómetros por tick** | 14.5356 µm/tick |

### Configuración

Los timers se configuran en modo *slave encoder 3* (`TIM_SMCR_SMS_EM3`) que
cuenta automáticamente los flancos en ambas señales de cuadratura
([`setup.c:254-266`](../source_code/src/setup.c#L254-L266)).

### Algoritmo de lectura

La función `max_likelihood_counter_diff()` en
[`encoders.c:117-126`](../source_code/src/encoders.c#L117-L126) maneja el
overflow del contador de 16 bits:

- Calcula la diferencia hacia adelante y hacia atrás
- Asume la diferencia más pequeña como la dirección real
- Esto funciona siempre que las lecturas sean más rápidas que medio overflow
  (32768 ticks entre lecturas → suficiente a 1 kHz)

### Variables calculadas

Cada 1 ms, `update_encoder_readings()` actualiza:

| Variable | Unidad | Fórmula |
|----------|--------|---------|
| `left/right_diff_ticks` | ticks | diff desde última lectura |
| `left/right_total_ticks` | ticks | acumulado absoluto |
| `left/right_micrometers` | µm | `total_ticks × 14.5356` |
| `left/right_millimeters` | mm | `micrometers / 1000` |
| `left/right_speed` | mm/s | `diff_ticks × 14.5356/1000 × 1000` |
| `angular_speed` | rad/s | `(speed_L − speed_R) / 1000 / 0.12` |
| `current_angle` | rad | integración de `angular_speed / 1000` |
| `position_x` | mm | integración de velocidad × cos(angle) |
| `position_y` | mm | integración de velocidad × sin(angle) |

> **💡 Consejo**: La constante `MICROMETERS_PER_TICK = 14.5356` es específica
> del diámetro de rueda y la resolución del encoder magnético. Debe
> recalibrarse si se cambian las ruedas o los encoders.

### Separación entre ruedas

| Parámetro | Valor |
|-----------|-------|
| `WHEELS_SEPARATION` | 0.12 m (120 mm) |

## Giroscopio MPU-6500

### Hardware

| Característica | Detalle |
|---------------|---------|
| **Modelo** | MPU-6500 (TDK/InvenSense) |
| **Interfaz** | SPI3 (PC10=SCK, PC11=MISO, PC12=MOSI, PA15=NSS) |
| **Rango** | ±2000 dps |
| **Sensibilidad** | 16.4 LSB/dps |
| **Filtro DLPF** | 250 Hz bandwidth (DLPF_CFG = 0) |
| **Sample rate interno** | 8 kHz (SMPLRT_DIV = 0) |

### Comunicación SPI

El MPU-6500 requiere dos velocidades SPI diferentes:

| Modo | Velocidad | Uso |
|------|-----------|-----|
| **Low speed** | ~656 kHz (PCLK1/128) | Escritura de registros |
| **High speed** | ~10.5 MHz (PCLK1/8) | Lectura de datos |

La conmutación se realiza con `setup_spi_low_speed()` y `setup_spi_high_speed()`
en [`setup.c:303-318`](../source_code/src/setup.c#L303-L318).

### Registros relevantes

| Registro | Dirección | Descripción |
|----------|-----------|-------------|
| `MPU_PWR_MGMT_1` | 107 | Control de sleep/reset |
| `MPU_SIGNAL_PATH_RESET` | 104 | Reset del path de señal |
| `MPU_USER_CTRL` | 106 | Control I2C/SPI |
| `MPU_SMPLRT_DIV` | 25 | Divisor de sample rate |
| `MPU_CONFIG` | 26 | Configuración DLPF |
| `MPU_GYRO_CONFIG` | 27 | Rango del giroscopio |
| `MPU_GYRO_ZOUT_H/L` | 71/72 | Lectura eje Z |
| `MPU_Z_OFFS_USR_H/L` | 23/24 | Offset de calibración Z |
| `MPU_WHOAMI` | 117 | Registro de identidad (0x70) |

### Actualización de lecturas

`update_gyro_readings()` en [`mpu.c:173-179`](../source_code/src/mpu.c#L173-L179):

1. Lee el valor crudo del eje Z vía SPI
2. Aplica un filtro paso bajo: `raw = 0.5 × new + 0.5 × old`
3. Integra el ángulo: `angle −= dps / 1000`

### Conversión de unidades

```
rad/s = raw / 16.4 × (π / 180)
dps   = raw / 16.4
```

### Calibración

El offset se aplica por hardware escribiendo en `MPU_Z_OFFS_USR_H/L`. Esto
simplifica la lectura ya que el valor crudo ya está compensado. Ver
[Calibración](08-calibration.md#calibracion-del-giroscopio-eje-z).

### Fusión sensorial

La corrección angular del lazo de control puede usar el giroscopio, los
sensores de línea, o ambos simultáneamente. La velocidad angular medida por
el giro es la fuente primaria; los sensores de línea corrigen la deriva
lateral. Ver [Control PID](05-control-system.md).

---

*Documento generado el 2026-06-25. Ver también [Sensores](03-sensors.md), [Control PID](05-control-system.md), [Calibración](08-calibration.md).*
