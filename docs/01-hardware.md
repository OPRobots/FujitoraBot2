# Hardware

## Microcontrolador

| Característica | Detalle |
|---------------|---------|
| **Modelo** | STM32F405RGT6 |
| **Arquitectura** | ARM Cortex-M4F |
| **Frecuencia** | 168 MHz (SYSCLK) |
| **Flash** | 1024 KB |
| **RAM** | 192 KB |
| **Encapsulado** | LQFP-64 |

El reloj principal se genera a partir del HSE de 8 MHz con PLL hasta 168 MHz
([`setup.c:15`](../source_code/src/setup.c#L15)).

## Placa base

Diseño propio en KiCad con dos variantes:

- **Main** — placa principal con MCU, drivers de motor, MPU, y conectores
- **Sensors 24** — placa frontal con 24 sensores IR y multiplexores

Ambos diseños están disponibles en [`pcb_files/kicad_project/`](../pcb_files/kicad_project/).

## Pinout

### GPIOA

| Pin | Función | AF | Descripción |
|-----|---------|----|-------------|
| PA0–PA7 | GPIO Output | — | LEDs de info (menú) |
| PA8–PA11 | AF1 (TIM1) | 1 | PWM LEDs RGB |
| PA15 | GPIO Output | — | CS del MPU (NSS software) |

### GPIOB

| Pin | Función | AF | Descripción |
|-----|---------|----|-------------|
| PB4–PB7 | AF2 (TIM3/4) | 2 | Encoders en cuadratura |
| PB10–PB11 | AF7 (USART3) | 7 | UART debug |
| PB12–PB15 | GPIO Input | — | Botones de menú + IR start |

### GPIOC

| Pin | Función | AF | Descripción |
|-----|---------|----|-------------|
| PC0–PC3 | ADC1 (IN10–IN12) | — | Sensores de línea (3 canales ADC) |
| PC4–PC5 | GPIO Output | — | LEDs de info adicionales |
| PC6–PC9 | AF3 (TIM8) | 3 | PWM motores + ventilador |
| PC10–PC12 | AF6 (SPI3) | 6 | SPI para MPU-6500 |
| PC13–PC15 | GPIO Output | — | Multiplexor de sensores (A/B/C) |

## Sensores

| Característica | Detalle |
|---------------|---------|
| **Sensores de línea** | 24 pares IR (emisor + fototransistor) |
| **Multiplexado** | 3× multiplexores 8:1 → 3 canales ADC (PC0/PC1/PC2) |
| **Giroscopio** | MPU-6500 vía SPI3, ±2000 dps |

(ver [Sensores](03-sensors.md))

## Actuadores

| Componente | Control | Detalle |
|-----------|---------|---------|
| **Motor izquierdo** | TIM8 CH1 (PC6) | PWM OneShot125, ESC BLHeli |
| **Motor derecho** | TIM8 CH3 (PC8) | PWM OneShot125, ESC BLHeli |
| **Ventilador** | TIM8 CH2+CH4 (PC7+PC9) | PWM, velocidad configurable por perfil |

(ver [Movimiento](04-movement.md))

## Alimentación

| Característica | Detalle |
|---------------|---------|
| **Batería** | LiPo 2S (7.4 V) o 3S (11.1 V) |
| **Detección** | Automática por rango de tensión |
| **Medición** | ADC2 (PC3/IN13) con divisor ×3.8673 |
| **Límites** | 2S: 7.4–8.2 V / 3S: 11.1–12.6 V |

(ver [Batería y LEDs](11-battery-leds.md))

## Chasis

| Componente | Material | Diseño |
|-----------|----------|--------|
| **Chasis principal** | Impresión 3D & PCB | [`3d_model/stl_files/`](../3d_model/stl_files/) |
| **Soporte sensores** | PCB dedicada (sensors_24) | [`pcb_files/kicad_project/sensors_24/`](../pcb_files/kicad_project/sensors_24/) |
| **Ruedas** | — | Control diferencial, separación 0.12 m |

---

*Documento generado el 2026-06-25. Ver también [Arquitectura Software](02-software-architecture.md), [Sensores](03-sensors.md).*
