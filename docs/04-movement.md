# Movimiento

## Motores DC

FujitoraBot2 usa dos motores DC brushless con controladores ESC compatibles con
el protocolo **OneShot125** (PWM de 125–250 µs de pulso), más un ventilador de
refrigeración controlado por PWM.

| Característica | Detalle |
|---------------|---------|
| **Tipo** | DC brushless con ESC |
| **Protocolo** | OneShot125 (PWM 4 kHz, resolución 1024 pasos) |
| **Timer PWM** | TIM8, canales 1–4 |
| **Frecuencia PWM** | 4 kHz |
| **Pulso neutro** | MOTORS_STOP_PWM = 512 |
| **Pulso máximo** | MOTORS_MAX_PWM = 1024 |

## Control PWM

### Timer y canales

| Canal TIM8 | Pin | Función |
|-----------|-----|---------|
| CH1 | PC6 | Motor izquierdo |
| CH2 | PC7 | Ventilador (canal A) |
| CH3 | PC8 | Motor derecho |
| CH4 | PC9 | Ventilador (canal B) |

El timer se configura en [`setup.c:214-233`](../source_code/src/setup.c#L214-L233).

### Inicialización de ESCs

Al arrancar, los ESCs requieren una secuencia de inicialización de 5 segundos
con pulso neutro (`MOTORS_STOP_PWM = 512`) antes de aceptar comandos. Esto se
gestiona en `init_esc()` ([`motors.c:10-28`](../source_code/src/motors.c#L10-L28)):

1. Se habilita la salida de los 4 canales TIM8
2. Durante 5 s se mantiene el pulso en `MOTORS_STOP_PWM`
3. Transcurrido ese tiempo, `escInited = true` y se aceptan comandos

### Conversión velocidad → PWM

Hay dos modos de control de velocidad lineal (ver [Control PID](05-control-system.md)):

1. **Modo encoder** (`CONTROL_ENCODERS`): el lazo PID calcula un voltaje lineal
   que se convierte a PWM:
   ```
   pwm = MOTORS_STOP_PWM + (voltaje / V_batería) × (MOTORS_MAX_PWM / 2)
   ```

2. **Modo PWM** (`CONTROL_PWM`): se aplica un porcentaje directo de PWM sobre
   la tensión nominal de batería.

En ambos modos, la corrección angular (diferencial) se suma al PWM de una rueda
y se resta de la otra:

```
pwm_left  = MOTORS_STOP_PWM + voltage_to_pwm(linear_voltage + angular_voltage)
pwm_right = MOTORS_STOP_PWM + voltage_to_pwm(linear_voltage - angular_voltage)
```

### Zona muerta y límites

- **PWM mínimo**: `MOTORS_MIN_PWM = 522` (≈2% por encima del neutro) para
  evitar el *cogging* del motor a bajas revoluciones
- **Velocidad mínima inicial**: 165 (valor empírico calculado en
  [`control.c:214`](../source_code/src/control.c#L214))
- **PWM máximo**: 1024 (saturación por hardware)

### Parada de emergencia

`emergency_stop()` detiene inmediatamente los motores poniendo PWM neutro y
desactiva el modo carrera. Se dispara por:

- Pérdida de señal IR (fin de carrera o desconexión)
- Pérdida de línea durante más de `TIEMPO_SIN_PISTA` ms

(ver [Control PID](05-control-system.md#parada-de-emergencia))

## Ventilador

| Característica | Detalle |
|---------------|---------|
| **Control** | TIM8 CH2 + CH4 |
| **Velocidad** | 0–100 (mapeado a PWM) |
| **Rampa** | Aceleración configurable por perfil de velocidad |
| **Inicio** | Arranca al 75% de la cuenta atrás si `fan_speed > 0` |
| **Parada** | Se apaga 500 ms después de terminar la carrera |

La velocidad objetivo y la rampa de aceleración se configuran en el perfil
cinemático seleccionado.

(ver [Cinemática](12-kinematics.md))

---

*Documento generado el 2026-06-25. Ver también [Control PID](05-control-system.md), [Cinemática](12-kinematics.md).*
