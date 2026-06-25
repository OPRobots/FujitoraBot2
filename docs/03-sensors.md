# Sensores

FujitoraBot2 usa 24 sensores IR de línea multiplexados en 3 canales ADC del
STM32F405, más un giroscopio MPU-6500 para corrección angular.

## Sensores de línea

### Arquitectura

24 pares emisor-receptor IR están distribuidos linealmente en la PCB frontal
`sensors_24`. Se leen mediante 3 multiplexores analógicos 8:1 controlados por
los pines PC13 (A), PC14 (B) y PC15 (C):

| Canal ADC | Pin STM32 | Multiplexor | Sensores |
|-----------|-----------|-------------|----------|
| ADC1 IN10 | PC0 | MUX 1 | 8–15 |
| ADC1 IN11 | PC1 | MUX 2 | 0–7 |
| ADC1 IN12 | PC2 | MUX 3 | 16–23 |

> **💡 Consejo**: Los canales ADC están mapeados de forma no secuencial.
> El canal 10 corresponde a los sensores 8–15, no a los primeros. Ver
> [`sensors.c:4`](../source_code/src/sensors.c#L4) y la nota en
> [`sensors.c:217-218`](../source_code/src/sensors.c#L217-L218) sobre
> desorden de canales al activar ADC2.

### Lectura con DMA

El ADC1 opera en modo scan continuo con DMA circular (DMA2 Stream0). Cada
transferencia completa dispara `sensors_update_mux_readings()` que:

1. Asigna las 3 lecturas ADC al slot correspondiente del array `sensors_raw[24]`
2. Avanza el índice del multiplexor (0→7)
3. Actualiza los pines A/B/C del mux para la siguiente ronda
4. Espera 3 µs de settling time

La tasa de muestreo efectiva es de aproximadamente 1 kHz por sensor (8 slots × 3
canales = 24 lecturas por ciclo completo DMA).

### Calibración

Cada sensor tiene tres valores almacenados en EEPROM:

| Parámetro | Significado |
|-----------|-------------|
| `sensors_min` | Lectura ADC sobre superficie blanca (mínima reflectividad) |
| `sensors_max` | Lectura ADC sobre línea negra (máxima reflectividad) |
| `sensors_umb` | Umbral de detección = min + (max − min) × 2/3 |

(ver [Calibración](08-calibration.md))

### Modos de funcionamiento

| Modo | Enum | Comportamiento |
|------|------|---------------|
| **Analógico** | `SENSORS_ANALOG` | Valores crudos dentro del 20%–80% del rango de calibración; el resto se satura |
| **Digital** | `SENSORS_DIGITAL` | Binario: supera o no el umbral `sensors_umb` |

Configurado desde el menú de carrera ([`menu_run.c`](../source_code/src/menu_run.c)).

### Filtro BiggerFilter

Cuando `FILTER_BIGGER` está activo, solo los 4 sensores alrededor de la
posición estimada de la línea contribuyen al cálculo de posición. El resto
se fuerzan a 0.

Esto reduce el ruido de sensores lejanos que aún detectan la línea en curvas
cerradas o cuando el robot está descentrado, mejorando la precisión de la
estimación de posición a alta velocidad.

### Cálculo de posición de línea

El algoritmo en [`sensors.c:262-331`](../source_code/src/sensors.c#L262-L331):

1. **Ventana de búsqueda**: se reduce a los 4 sensores alrededor de la última
   posición conocida
2. **Media ponderada**: cada sensor contribuye proporcionalmente a su lectura:
   ```
   posición = Σ(sensor_i × valor_i × 1000) / Σ(valor_i) − offset_central
   ```
3. **Normalización**: resultado mapeado al rango [−1000, +1000]
4. **Detección de pérdida**: si ningún sensor supera el umbral durante
   `TIEMPO_SIN_PISTA` ms (150 ms por defecto), se activa la parada de emergencia

### Detección de fuera de pista

Si la mitad o más de los sensores detectan línea simultáneamente, se considera
ruido (robot completamente fuera del circuito) y se ignora el evento. En caso
contrario, se actualiza un timestamp (`ultimaLinea`). Si pasa demasiado tiempo
sin detectar línea, se ejecuta `emergency_stop()`.

## Giroscopio MPU-6500

| Característica | Detalle |
|---------------|---------|
| **Modelo** | MPU-6500 (InvenSense/TDK) |
| **Interfaz** | SPI3 (PC10–PC12, NSS en PA15) |
| **Rango** | ±2000 dps |
| **Sensibilidad** | 16.4 LSB/dps |
| **Filtro** | DLPF 250 Hz |
| **Sample rate** | 8 kHz interno (SMPLRT_DIV = 0) |
| **Calibración** | Offset en registro MPU + soft integración |

(ver [Encoders y Giroscopio](10-encoders-gyro.md))

---

*Documento generado el 2026-06-25. Ver también [Hardware](01-hardware.md), [Calibración](08-calibration.md), [Encoders y Giroscopio](10-encoders-gyro.md).*
