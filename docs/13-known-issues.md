# Problemas Conocidos

> **Última revisión**: 2026-06-25 (commit `8a8beb2` — doc-init inicial)
> **Total**: 29 issues — 6 críticos, 9 moderados, 14 leves

---

## 🔴 Críticos — Riesgo de fallo catastrófico

### BL-01 — Límites de batería 2S invertidos: detección nunca se activa

- **Archivo**: [`config.h:40-41`](../source_code/include/config.h#L40-L41), [`battery.c:34`](../source_code/src/battery.c#L34)
- **Descripción**: `BATTERY_2S_HIGH_LIMIT_VOLTAGE = 7.4` y `BATTERY_2S_LOW_LIMIT_VOLTAGE = 8.2` están invertidos (HIGH < LOW). La condición `voltage >= 8.2 && voltage <= 7.4` es **siempre falsa**. Las baterías 2S nunca se detectan; `battery_full_voltage` se queda en el valor por defecto (11.1V de 3S) y el nivel de batería siempre es 0%.
- **Impacto**: Alto — con batería 2S, la compensación de tensión en el cálculo de PWM usa un voltaje de referencia incorrecto (11.1V en vez de ~8.4V), causando que los motores reciban menos potencia de la esperada. El indicador de nivel de batería no funciona.
- **Solución propuesta**: Intercambiar los valores:
  ```c
  #define BATTERY_2S_LOW_LIMIT_VOLTAGE 7.4
  #define BATTERY_2S_HIGH_LIMIT_VOLTAGE 8.4
  ```

### CT-01 — PID de velocidad lineal es control I puro sin término proporcional ni derivativo

- **Archivo**: [`control.c:251-257`](../source_code/src/control.c#L251-L257), [`config.h:20-21`](../source_code/include/config.h#L20-L21)
- **Descripción**: La variable `linear_error` acumula la integral del error de velocidad (`linear_error += ...`). `KP_LINEAR = 0.00025` multiplica esta integral (actúa como ganancia integral, no proporcional). `KD_LINEAR = 0.00//3` está anulado por el comentario `//3`. El control resultante es **integral puro** sin respuesta proporcional ni amortiguamiento derivativo, lo que causa overshoot y oscilación en la respuesta de velocidad.
- **Impacto**: Alto — sin término proporcional, el robot no responde inmediatamente a cambios de velocidad; sin derivativo, no hay amortiguamiento. Esto degrada el seguimiento de velocidad en aceleraciones y frenadas.
- **Solución propuesta**: Reestructurar como PID estándar:
  ```c
  float error = ideal_linear_speed - get_measured_linear_speed();
  integral_linear_error += error;
  // anti-windup simétrico
  linear_voltage = KP * error + KI * integral_linear_error + KD * (error - last_error);
  ```
  Y definir constantes separadas `KP`, `KI`, `KD` con valores apropiados.

### SW-01 — Variables compartidas ISR↔main sin cualificador `volatile`

- **Archivo**: [`control.c:3`](../source_code/src/control.c#L3), [`menu_run.c:18`](../source_code/src/menu_run.c#L18), [`motors.c:3`](../source_code/src/motors.c#L3), [`buttons.c:3-6`](../source_code/src/buttons.c#L3-L6), [`kinematics.c:59`](../source_code/src/kinematics.c#L59)
- **Descripción**: Las siguientes variables se leen y escriben desde contexto de ISR y desde el main loop sin cualificador `volatile`:
  - `race_started` (control.c) — ISR TIM5 / main loop
  - `valueRun[]` (menu_run.c) — ISR TIM5 / main loop
  - `escInited` (motors.c) — ISR SysTick / ISR TIM5
  - `ir_start_ms`, `btn_menu_up_ms`, `btn_menu_down_ms`, `btn_menu_mode_ms` (buttons.c) — ISR SysTick / main loop
  - `kinematics` (kinematics.c) — main loop / ISR TIM5
- **Impacto**: Alto — el compilador puede cachear estas variables en registros, causando que la ISR lea valores stale o que el main loop nunca vea cambios hechos por la ISR. Podría impedir el arranque de carrera o causar lecturas incorrectas de botones.
- **Mitigación actual**: Ninguna. En la práctica funciona porque las variables se leen en cada iteración y el compilador con `-Os` tiende a re-leer de memoria, pero no está garantizado.
- **Solución propuesta**: Añadir `volatile` a todas las declaraciones listadas.

### CT-02 — División por cero en `voltage_to_motor_pwm()` al inicio

- **Archivo**: [`control.c:49`](../source_code/src/control.c#L49), [`battery.c:4`](../source_code/src/battery.c#L4)
- **Descripción**: `battery_voltage` se inicializa a `0.0f`. `voltage_to_motor_pwm()` divide por `get_battery_voltage()` sin comprobar que sea > 0. Si el lazo de control se ejecuta antes de que el filtro paso-bajo de batería produzca un valor válido, la división por cero en FPU genera NaN/inf que se propaga al PWM de los motores.
- **Impacto**: Alto — PWM indefinido en los primeros milisegundos tras el arranque. Podría causar un movimiento brusco e inesperado de los motores.
- **Solución propuesta**: Añadir guarda:
  ```c
  float batt = get_battery_voltage();
  if (batt <= 0.001f) return 0;
  return voltage / batt * (MOTORS_MAX_PWM / 2);
  ```

### EE-01 — Borrado/programación de flash con interrupciones habilitadas

- **Archivo**: [`eeprom.c:22-27`](../source_code/src/eeprom.c#L22-L27)
- **Descripción**: `eeprom_save()` llama a `flash_erase_sector(11)` y `flash_program_word()` con las interrupciones habilitadas. El sector 11 está en el banco 1 de flash (0x080E0000). Si cualquier ISR (SysTick @ 1kHz, TIM5 @ 1kHz, DMA) se dispara durante el borrado, la CPU intenta leer el vector de interrupción del banco 1 bloqueado → hard fault.
- **Impacto**: Alto — crashea el sistema al guardar configuración (ej. al activar Race Mode desde el menú, que llama a `eeprom_save()`).
- **Mitigación actual**: El guardado solo ocurre cuando el usuario interactúa con el menú, no durante la carrera. Pero el riesgo es real cada vez que se guarda.
- **Solución propuesta**: Deshabilitar interrupciones durante la operación:
  ```c
  __disable_irq();
  flash_unlock();
  flash_erase_sector(EEPROM_SECTOR, FLASH_CR_PROGRAM_X16);
  // ... program words ...
  flash_lock();
  __enable_irq();
  ```

### CT-03 — Términos de corrección stale aplicados cuando la corrección está desactivada

- **Archivo**: [`control.c:269-279`](../source_code/src/control.c#L269-L279)
- **Descripción**: Cuando `line_sensors_correction_enabled = false`, los términos `line_sensors_error`, `sum_line_sensors_error` y `last_line_sensors_error` no se actualizan (bloque `if` en línea 269), pero sus **valores anteriores** siguen contribuyendo a `angular_voltage` en la línea 279. Si la corrección por sensores se desactiva en mitad de una curva, el error stale fuerza una corrección angular incorrecta.
- **Impacto**: Alto — corrección angular fantasma tras desactivar sensores de línea. Puede desestabilizar el robot en recta si la última lectura fue en curva.
- **Solución propuesta**: Gatear la contribución a `angular_voltage`:
  ```c
  float angular_voltage = 0;
  if (mpu_correction_enabled) { angular_voltage += ...; }
  if (line_sensors_correction_enabled) { angular_voltage += ...; }
  ```

---

## 🟡 Moderados — Riesgo de comportamiento incorrecto

### CT-04 — `set_target_fan_speed()` divide por `ms` sin comprobar cero

- **Archivo**: [`control.c:228`](../source_code/src/control.c#L228)
- **Descripción**: `fan_speed_accel = (fan_speed - ideal_fan_speed) * CONTROL_FREQUENCY_HZ / ms`. Si `ms = 0`, división por cero. Todos los callers actuales pasan 500, pero no hay defensa.
- **Impacto**: Medio — solo si se añade un caller con `ms = 0`.
- **Solución propuesta**: Añadir `if (ms <= 0) return;` al inicio de la función.

### CT-05 — Anti-windup de `linear_error` solo en dirección positiva

- **Archivo**: [`control.c:254-256`](../source_code/src/control.c#L254-L256)
- **Descripción**: El clamp `if (linear_error > 8000) linear_error = 8000` solo actúa en dirección positiva. Si el robot frena bruscamente o la velocidad medida supera la ideal, la integral negativa crece sin límite, causando una recuperación lenta al volver a acelerar.
- **Impacto**: Medio — afecta la respuesta de frenada y re-aceleración.
- **Solución propuesta**: Añadir clamp simétrico: `else if (linear_error < -8000) linear_error = -8000`.

### CT-06 — `angular_error` y `sum_line_sensors_error` sin anti-windup

- **Archivo**: [`control.c:266`](../source_code/src/control.c#L266), [`control.c:272`](../source_code/src/control.c#L272)
- **Descripción**: Ambas integrales crecen sin límite. `angular_error` acumula `ideal_angular_speed - measured` (ideal=0 siempre, así que acumula `-gyro_bias`). Con `KP_ANGULAR = 0.04`, un bias de 1º/s produce ~0.06 rad/s de corrección espuria tras 10 segundos. `sum_line_sensors_error` tiene `KI_LINE_SENSORS = 0` actualmente, así que es latente.
- **Impacto**: Medio — `angular_error` causa deriva en la corrección angular durante carreras largas. `sum_line_sensors_error` es latente hasta que se active `KI_LINE_SENSORS`.
- **Solución propuesta**: Añadir clamping condicional a ambas integrales.

### EG-01 — Calibración del giroscopio usa media móvil en vez de promedio real

- **Archivo**: [`mpu.c:152`](../source_code/src/mpu.c#L152)
- **Descripción**: `zout_av = (raw + zout_av) / 2` aplica un filtro de media móvil exponencial con α = 0.5. Tras 300 muestras, el resultado está dominado por las últimas ~2 lecturas, no por el promedio de las 300. La calibración es sensible al ruido de las últimas muestras.
- **Impacto**: Medio — el offset del giro puede tener un error de varios LSB, causando deriva angular.
- **Solución propuesta**: Usar promedio acumulativo real: acumular en `int32_t`, dividir entre 300 al final.

### SS-01 — ADC2 corrompe el orden de canales de ADC1

- **Archivo**: [`sensors.c:217-218`](../source_code/src/sensors.c#L217-L218), [`battery.c:7`](../source_code/src/battery.c#L7)
- **Descripción**: El desarrollador dejó comentarios de advertencia indicando que activar ADC2 (batería) desordena los canales de ADC1. Las lecturas de los sensores de línea pueden asignarse a posiciones incorrectas del array `sensors_raw[]`.
- **Impacto**: Medio — si los canales se intercambian, la posición de línea calculada es errónea, causando zigzag o salida de pista.
- **Mitigación actual**: El mapeo actual en `sensors_update_mux_readings()` (línea 219-221) parece ser el resultado de ajustes empíricos para compensar el desorden.
- **Solución propuesta**: Investigar la configuración de `ADC_CCR` (registro común de ADC) en STM32F4. Posiblemente falte configuración del modo dual-ADC o la sincronización de relojes entre ADC1 y ADC2.

### HW-01 — `CONTROL_FREQUENCY_HZ` no coincide con la frecuencia real de TIM5

- **Archivo**: [`constants.h:14`](../source_code/include/constants.h#L14), [`setup.c:241`](../source_code/src/setup.c#L241)
- **Descripción**: `CONTROL_FREQUENCY_HZ = 1000`, pero TIM5 está configurado con prescaler 82 y período 1024 sobre un reloj de 84 MHz: frecuencia real = 84 MHz / 83 / 1024 ≈ 988 Hz. Las rampas de aceleración (`linear_accel / CONTROL_FREQUENCY_HZ`) son ~1.2% más lentas de lo esperado.
- **Impacto**: Medio — error acumulativo en distancias de aceleración/frenada.
- **Solución propuesta**: Ajustar `timer_set_period(TIM5, 1012)` y `timer_set_prescaler(TIM5, 82)` para obtener exactamente 1000 Hz, o definir `CONTROL_FREQUENCY_HZ 988`.

### MN-01 — `millisPasados` truncado a 16 bits con riesgo de overflow

- **Archivo**: [`main.c:48-50`](../source_code/src/main.c#L48-L50)
- **Descripción**: `uint16_t millisPasados = get_clock_ticks() - millisInicio` trunca un `uint32_t` a 16 bits. Si `get_start_millis() > 65535` (65.5 s), el contador wrapa y el fade del LED RGB durante la cuenta atrás es incorrecto.
- **Impacto**: Medio — solo afecta al indicador visual de cuenta atrás, no a la carrera. Pero con cuentas atrás largas (>65s) el LED muestra valores incorrectos.
- **Solución propuesta**: Cambiar `uint16_t millisPasados` a `uint32_t millisPasados`.

### SW-02 — `delay()` sin protección contra wrap-around del timer

- **Archivo**: [`delay.c:26-30`](../source_code/src/delay.c#L26-L30)
- **Descripción**: `delay()` calcula `awake = clock_ticks + ms` y espera `awake > clock_ticks`. Cuando `clock_ticks` está cerca de `UINT32_MAX` (~49.7 días de uptime), la suma wrapa y la función retorna inmediatamente. El mismo patrón incorrecto aparece en `main.c:49`, `sensors.c:151` y `sensors.c:312`.
- **Impacto**: Medio — no es relevante para carreras (< 1 minuto), pero sí para sesiones de desarrollo largas o si el robot se deja encendido días.
- **Solución propuesta**: Usar el patrón seguro:
  ```c
  uint32_t start = clock_ticks;
  while ((clock_ticks - start) < ms);
  ```

### BL-02 — Rango de tensión 8.2V–11.1V sin manejar

- **Archivo**: [`battery.c:31-36`](../source_code/src/battery.c#L31-L36)
- **Descripción**: Una batería 3S descargada a < 11.1V o una 2S completamente cargada a > 8.2V cae en un hueco donde ninguna de las dos ramas de detección aplica. `battery_level` se queda en 0 y `battery_full_voltage` conserva el valor por defecto.
- **Impacto**: Medio — nivel de batería incorrecto en el margen entre 2S cargada y 3S descargada.
- **Solución propuesta**: Fusionar en un solo rango continuo o añadir una tercera rama para voltajes en el hueco, asignando al tipo de batería más probable según el valor.

---

## 🟢 Leves — Riesgo de mantenibilidad o comportamiento menor

### SW-03 — `#elif CONFIG_RUN_DEBUG` sin `defined()`

- **Archivo**: [`config.c:11,26`](../source_code/src/config.c#L11), [`sensors.c:65-66`](../source_code/src/sensors.c#L65-L66)
- **Descripción**: `#elif CONFIG_RUN_DEBUG` evalúa un macro no definido como 0 (false), que es el comportamiento deseado. Pero si alguien define `CONFIG_RUN_DEBUG 0`, `#elif 0` también es false — la intención no se cumple. Lo mismo para `#elif CONFIG_LINE_WHITE`.
- **Impacto**: Bajo — funciona por accidente, pero es frágil.
- **Solución propuesta**: Usar `#elif defined(CONFIG_RUN_DEBUG)` y `#elif defined(CONFIG_LINE_WHITE)`.

### SW-04 — Variables globales sin `static`

- **Archivo**: [`calibrations.c:3`](../source_code/src/calibrations.c#L3), [`debug.c:3-4`](../source_code/src/debug.c#L3-L4), [`buttons.c:3-6`](../source_code/src/buttons.c#L3-L6), [`menu_run.c:9,18,20`](../source_code/src/menu_run.c#L9)
- **Descripción**: `calibration_enabled`, `debug_enabled`, `last_print_debug`, `ir_start_ms`, `btn_menu_up_ms`, `btn_menu_down_ms`, `btn_menu_mode_ms`, `modeRun`, `valueRun[]`, `lastBlinkMs`, `blinkState` son globales sin `static`. Pueden causar colisiones de nombres con otros módulos.
- **Impacto**: Bajo — no hay colisiones reales hoy, pero es frágil ante nuevos módulos.
- **Solución propuesta**: Añadir `static` a todas estas declaraciones.

### KN-01 — `configure_kinematics()` sin bounds check

- **Archivo**: [`kinematics.c:62`](../source_code/src/kinematics.c#L62)
- **Descripción**: `kinematics = kinematics_settings[speed]` indexa el array con un `enum speed_strategy` sin verificar que `speed <= SPEED_HAKI`. Un valor corrupto (ej. por memory corruption o un bug en el menú) leería basura de la memoria adyacente.
- **Impacto**: Bajo — requiere corrupción de memoria o bug en menú para manifestarse.
- **Solución propuesta**: Añadir `if (speed < 0 || speed > SPEED_HAKI) speed = SPEED_TEST;`.

### EG-02 — Typo en nombre de función: `get_encoder_curernt_angle`

- **Archivo**: [`encoders.h:28`](../source_code/include/encoders.h#L28), [`encoders.c:93`](../source_code/src/encoders.c#L93)
- **Descripción**: `curernt` en vez de `current`. La función compila y funciona, pero el nombre es confuso.
- **Impacto**: Bajo — solo afecta a la legibilidad.
- **Solución propuesta**: Renombrar a `get_encoder_current_angle` en header y fuente.

### EE-02 — `eeprom_set_data()` sin bounds check

- **Archivo**: [`eeprom.c:56-63`](../source_code/src/eeprom.c#L56-L63)
- **Descripción**: Si `index + length > DATA_LENGTH`, se escribe fuera del array `eeprom_data[]`. Todos los callers actuales respetan los límites, pero no hay defensa.
- **Impacto**: Bajo — requiere un bug en un caller para manifestarse.
- **Solución propuesta**: Añadir `if (index + length > DATA_LENGTH || !data) return;`.

### BL-03 — Rainbow LED: comparación `<= 0` sobre `uint32_t`

- **Archivo**: [`leds.c:108-111`](../source_code/src/leds.c#L108-L111)
- **Descripción**: `rainbowRGB[rainbowColorDesc] <= 0` sobre `uint32_t` es equivalente a `== 0`. Al decrementar de 20 en 20, el valor salta de 4 a 0xFFFFFFF0 (wrap), nunca pasa por 0, así que la condición nunca se cumple. El canal experimenta un flash blanco breve antes de que el otro límite (`>= LEDS_MAX_PWM/4`) fuerce la transición.
- **Impacto**: Bajo — flicker apenas perceptible en el LED RGB durante la calibración.
- **Solución propuesta**: Usar `<= 20` o clamp explícito antes del decremento.

### SS-02 — `delay_us(0)` como NOP frágil para timing de SPI del MPU

- **Archivo**: [`mpu.c:60`](../source_code/src/mpu.c#L60)
- **Descripción**: El comentario del desarrollador indica que `delay_us(0)` es necesario para que el MPU "no se prenda fuego". Pero `delay_us(0)` no garantiza ningún ciclo de espera — solo el overhead de la llamada a función. Un cambio de compilador u optimización podría eliminarlo.
- **Impacto**: Bajo — el MPU funciona actualmente, pero es frágil.
- **Solución propuesta**: Reemplazar con `delay_us(1)` o `__asm__("nop"); __asm__("nop"); __asm__("nop");`.

### SS-03 — VLAs en stack durante calibración de sensores

- **Archivo**: [`sensors.c:99-100`](../source_code/src/sensors.c#L99-L100)
- **Descripción**: `bool sensorsMinChecked[get_sensors_num()]` y `bool sensorsMaxChecked[get_sensors_num()]` son arrays de longitud variable (VLA) en el stack. Con 24 sensores son 48 bytes — seguro hoy, pero frágil si `NUM_SENSORS` aumenta.
- **Impacto**: Bajo — seguro con la configuración actual.
- **Solución propuesta**: Usar arrays de tamaño fijo `bool sensorsMinChecked[NUM_SENSORS]`.

### SS-04 — `round()` usa aritmética `double` en Cortex-M4F

- **Archivo**: [`sensors.c:270`](../source_code/src/sensors.c#L270)
- **Descripción**: `round(map(...))` llama a `round(double)` que requiere emulación software en Cortex-M4F (single-precision FPU). Es más lento y aumenta el tamaño del binario.
- **Impacto**: Bajo — llamado una vez por ms en el SysTick, el overhead es mínimo.
- **Solución propuesta**: Usar `roundf()` (single-precision).

### DB-01 — `debug_motors()` aplica PWM sin confirmación

- **Archivo**: [`debug.c`](../source_code/src/debug.c) — función `debug_motors()`
- **Descripción**: El modo debug de motores aplica `set_motors_speed(15, 15)` sin ninguna confirmación ni guarda. Si se activa por error con el robot sobre la mesa, los motores giran inesperadamente.
- **Impacto**: Bajo — solo en modo debug, requiere interacción deliberada con el menú.
- **Solución propuesta**: Añadir una secuencia de confirmación (ej. pulsar dos botones simultáneamente) antes de activar los motores en modo debug.

### SW-05 — Include guard `MOVE_H` no coincide con `kinematics.h`

- **Archivo**: [`kinematics.h:1`](../source_code/include/kinematics.h#L1)
- **Descripción**: El header usa `#ifndef MOVE_H` pero el archivo se llama `kinematics.h`. Si algún día existe un `move.h` con guard `MOVE_H`, habrá colisión silenciosa.
- **Impacto**: Bajo — no hay `move.h` actualmente.
- **Solución propuesta**: Cambiar a `#ifndef KINEMATICS_H`.

### SW-06 — Prescaler de timers con fórmula no comprendida por el autor

- **Archivo**: [`setup.c:189,218,238`](../source_code/src/setup.c#L189)
- **Descripción**: `timer_set_prescaler(TIM1, rcc_apb2_frequency * 2 / 4000000 - 2)` tiene un comentario que admite no entender por qué funciona. La fórmula `* 2 / 4000000 - 2` es frágil ante cambios de reloj.
- **Impacto**: Bajo — funciona hoy, pero es una trampa de mantenimiento.
- **Solución propuesta**: Documentar la derivación: `(APB2_freq * 2 / target_freq) - 1`, donde `*2` es porque el timer recibe 2× APB2 cuando APB prescaler > 1.

### SW-07 — `PI` con solo 5 cifras significativas

- **Archivo**: [`constants.h:5`](../source_code/include/constants.h#L5)
- **Descripción**: `#define PI 3.1415` tiene ~0.003% de error. Afecta conversiones giroscópicas (rad/s, integración angular).
- **Impacto**: Bajo — error acumulativo despreciable en carreras de < 1 minuto.
- **Solución propuesta**: `#define PI 3.14159265f`.

### DB-02 — LED de info 7 sin uso

- **Archivo**: [`menu_run.c`](../source_code/src/menu_run.c), [`leds.c:221`](../source_code/src/leds.c#L221)
- **Descripción**: El LED índice 7 (PA6) nunca se referencia en `menu_run_handler()`. Es hardware instalado pero sin función asignada, o se perdió un modo de menú que debería usarlo.
- **Impacto**: Bajo — no afecta funcionalidad.
- **Solución propuesta**: Asignar LED 7 a un nuevo indicador o documentar que es reserva.

---

## Correcciones recientes

*(Ninguna — documento creado en la auditoría inicial del 2026-06-25.)*

---

*Documento generado el 2026-06-25. Actualizar con `/doc-review` tras cambios de código.*
