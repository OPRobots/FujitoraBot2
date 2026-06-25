# FujitoraBot2

Versión 2 de FujitoraBot renovando el hardware después de 5 años: un pelín más
rápido, con 24 sensores de línea, giroscopio MPU-6500 y diseño más modular.

## Stack tecnológico
- **Microcontrolador**: STM32F405RGT6 (ARM Cortex-M4F)
- **Framework**: LibOpenCM3
- **Entorno**: PlatformIO
- **Lenguaje**: C11
- **Sensores**: 24× IR de línea (multiplexados), MPU-6500 (giroscopio 3 ejes vía SPI)
- **Actuadores**: 2× motores DC brushless con ESC BLHeli (OneShot125), ventilador

## Estructura del repositorio
- `source_code/` — firmware (PlatformIO + LibOpenCM3)
- `pcb_files/` — diseño electrónico (KiCad, 2 variantes: main + sensors_24)
- `3d_model/` — piezas mecánicas (SketchUp + STL)
- `docs/` — documentación técnica (MkDocs Material)
- `images/` — fotos del robot

## Convenciones de código
- Lenguaje: C11 con LibOpenCM3
- Nombres de función: `snake_case` con prefijo del módulo (`sensors_`, `encoder_`, `mpu_`)
- Formateo: `.clang-format` en `source_code/`
- ISRs en `setup.c` (configuración + handlers en el mismo archivo)
- Módulos con par `.h`/`.c` en `include/` y `src/`

## Documentación
- Usa `/doc-init` para regenerar la documentación desde cero
- Usa `/doc-review` para revisar cambios incrementales y actualizar known issues
- La documentación sigue el estándar OPRobots (MkDocs Material + estilo ZoroBot3)
- Despliegue en GitHub Pages (docs.oprobots.org) vía monorepo OPRobots/docs

## Notas
- Sistema de arranque dual: IR remote + botón físico (pulsación larga >1.2 s)
- EEPROM emulada en sector 11 de la flash (128 KB) para calibraciones y settings
- Control en cascada: PID velocidad lineal + PID angular (giro + sensores)
- MacroArray con capacidad para ~30 s de telemetría a 1 kHz para análisis post-carrera
- La calibración de sensores soporta verificación contra EEPROM para arranque rápido
