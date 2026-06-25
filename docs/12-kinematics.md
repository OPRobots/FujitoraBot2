# Cinemática

FujitoraBot2 define 6 perfiles de velocidad preconfigurados que controlan
la velocidad lineal máxima, las rampas de aceleración/frenado y la velocidad
del ventilador.

## Perfiles de velocidad

| Perfil | Velocidad (mm/s) | Acel. (mm/s²) | Frenada (mm/s²) | PWM (%) | Ventilador |
|--------|-----------------|---------------|-----------------|---------|------------|
| **TEST** | 2000 | 10000 | 5000 | 27% | 20% |
| **BASE** | 2500 | 10000 | 10000 | 33% | 30% |
| **NORMAL** | 3000 | 10000 | 10000 | 40% | 40% |
| **MEDIUM** | 3500 | 20000 | 20000 | 50% | 50% |
| **FAST** | 4000 | 20000 | 20000 | 60% | 60% |
| **HAKI** | 4500 | 20000 | 20000 | 70% | 75% |

Definidos en [`kinematics.c:3-57`](../source_code/src/kinematics.c#L3-L57).

## Estructura `kinematics`

Cada perfil contiene:

| Campo | Tipo | Descripción |
|-------|------|-------------|
| `linear_speed` | `int16_t` | Velocidad lineal objetivo (mm/s) |
| `linear_accel` | `int16_t` | Aceleración lineal (mm/s²) |
| `linear_break` | `int16_t` | Deceleración lineal (mm/s²) |
| `linear_speed_percent` | `int16_t` | % PWM en modo CONTROL_PWM |
| `linear_accel_percent` | `int16_t` | Aceleración en modo PWM (%/s) |
| `linear_break_percent` | `int16_t` | Deceleración en modo PWM (%/s) |
| `fan_speed` | `int16_t` | Velocidad del ventilador (0–100) |

## Rampas de velocidad

La velocidad ideal sigue una rampa lineal gobernada por los parámetros de
aceleración y frenada:

```
Modo ENCODERS:
  si ideal < target → ideal += linear_accel / 1000  (cada ms)
  si ideal > target → ideal -= linear_break / 1000

Modo PWM:
  si ideal_pct < target_pct → ideal_pct += linear_accel_percent / 1000
  si ideal_pct > target_pct → ideal_pct -= linear_break_percent / 1000
```

La frecuencia de actualización es 1 kHz, por lo que la aceleración expresada
en mm/s² se divide entre 1000.

### Ejemplo: perfil FAST

Arranque en modo encoder de 0 a 4000 mm/s:
- Incremento por tick: 20000 / 1000 = 20 mm/s por ms
- Tiempo hasta velocidad objetivo: 4000 / 20 = 200 ms

## Ventilador

La velocidad del ventilador sigue su propia rampa independiente:

```
aceleración_fan = (target − actual) × 1000 / ms_transición
```

Al iniciar la carrera, el ventilador arranca al 75% de la cuenta atrás (si
`fan_speed > 0` en el perfil) a la mitad de la velocidad objetivo configurada.

Tras la carrera, el ventilador se detiene 500 ms después del `emergency_stop`
o la finalización normal.

## Selección de perfil

El perfil se selecciona desde el menú Run → Speed. El valor se carga en
`configure_kinematics()` al iniciar la carrera:

```c
configure_kinematics(menu_run_get_speed());
set_target_linear_speed(get_kinematics().linear_speed);
set_target_linear_speed_percent(get_kinematics().linear_speed_percent);
set_target_fan_speed(get_kinematics().fan_speed, 500);
```

## Modo de control

El perfil define valores tanto para el control por encoders como por PWM.
El modo activo se selecciona en el menú Run → Encoders:

- **Encoders ON** (`CONTROL_ENCODERS`): usa `linear_speed`, `linear_accel`, `linear_break`
- **Encoders OFF** (`CONTROL_PWM`): usa `linear_speed_percent`, `linear_accel_percent`, `linear_break_percent`

(ver [Control PID](05-control-system.md))

---

*Documento generado el 2026-06-25. Ver también [Control PID](05-control-system.md), [Movimiento](04-movement.md), [Menú](06-menu-system.md).*
