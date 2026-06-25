# Debug

FujitoraBot2 dispone de un sistema de debug por UART/USART3 con múltiples modos
de visualización y un buffer de telemetría (MacroArray) para análisis
post-carrera.

## Comunicación UART

| Característica | Detalle |
|---------------|---------|
| **Periférico** | USART3 |
| **Pines** | PB10 (TX), PB11 (RX) |
| **Baudrate** | 115200 |
| **Formato** | 8N1 |
| **Redirección** | `printf` → `_write()` → USART3 |

La redirección de `printf` se implementa en [`usart.c`](../source_code/src/usart.c)
sobrescribiendo `_write()`.

## Modos de debug

Los modos se seleccionan desde el menú Config → Debug. Cada modo muestra datos
en tiempo real por UART:

| ID | Modo | Descripción |
|----|------|-------------|
| `0` | `DEBUG_NONE` | Sin salida |
| `1` | `DEBUG_MACROARRAY` | Vuelca el buffer de telemetría |
| `2` | `DEBUG_TYPE_SENSORS_RAW` | Lecturas crudas de los 24 sensores |
| `3` | `DEBUG_TYPE_SENSORS_CALIBRATED` | Lecturas calibradas de los 24 sensores |
| `4` | `DEBUG_TYPE_LINE_POSITION` | Posición de línea calculada |
| `5` | `DEBUG_TYPE_MOTORS` | PWM y velocidades de motores |
| `6` | `DEBUG_TYPE_ENCODERS` | Lecturas de encoders y odometría |
| `7` | `DEBUG_TYPE_DIGITAL_IO` | Estado de entradas/salidas digitales |
| `8` | `DEBUG_TYPE_CORRECCION_POSICION` | Corrección de posición (error, voltajes) |
| `9` | `DEBUG_TYPE_LEDS_PARTY` | Demo de LEDs |
| `10` | `DEBUG_TYPE_FANS_DEMO` | Demo de ventilador |

## MacroArray

Buffer circular de telemetría que almacena variables durante la carrera para
su análisis posterior:

| Capacidad | 30 000 registros |
|-----------|-------------------|
| **Duración máxima** | ~30 s a 1 kHz |
| **Formato** | Entradas de tamaño variable con timestamp |
| **Salida** | Plot (separado por `\n`) o Tabs (separado por `\t`) |

### Datos registrados en carrera

Cada iteración del lazo de control almacena 9 valores:

```
[timestamp, target_speed, ideal_speed, measured_speed,
 line_position, linear_voltage, angular_voltage,
 pwm_left, pwm_right, battery_voltage]
```

El buffer se vuelca por UART con el modo `DEBUG_MACROARRAY` tras la carrera
para análisis con herramientas externas (Serial Plotter, hojas de cálculo).

### Canales alternativos

Existen canales adicionales comentados en el código para activar según
necesidad de diagnóstico:

- **Canal 1**: error de sensores de línea + voltaje angular
- **Canal 4**: velocidad lineal, angular + posición cartesiana (X, Y)

## LEDs de estado

| LED | Color | Significado |
|-----|-------|-------------|
| **Status** | Azul | Estado general (heartbeat, warning) |
| **RGB** | RGB | Información contextual del menú y carrera |
| **Info 0–9** | Azul | Indicadores de modo y configuración |

El LED RGB se apaga completamente (`set_RGB_color(0,0,0)`) al iniciar la
carrera para no interferir con los sensores IR.

---

*Documento generado el 2026-06-25. Ver también [Menú](06-menu-system.md), [Control PID](05-control-system.md).*
