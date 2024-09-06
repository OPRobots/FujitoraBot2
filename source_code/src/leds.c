#include "leds.h"

static bool statusLedState = false;

static uint32_t lastTicksRainbow = 0;
static uint32_t rainbowRGB[3] = {LEDS_MAX_PWM, 0, 0};
static int rainbowColorDesc = 0;
static int rainbowColorAsc = 1;

static uint32_t lastTicksHeartbeat = 0;
static uint32_t lastTicksWarning = 0;

static uint32_t lastTicksWarningRGB = 0;
static bool lastWarningRGBState = false;

static uint32_t lastTicksWave = 0;
static uint8_t currentIndexWave = 0;

static uint32_t lastTicksWarningBateria = 0;

/**
 * @brief Establece el estado del led de estado
 *
 * @param state Estado del led
 */
void set_status_led(bool state) {
  statusLedState = state;
  if (state) {
    timer_set_oc_value(TIM1, TIM_OC2, LEDS_MAX_PWM);
  } else {
    timer_set_oc_value(TIM1, TIM_OC2, 0);
  }
}

/**
 * @brief Aleterna el estado del led de estado
 *
 */
void toggle_status_led(void) {
  set_status_led(!statusLedState);
}

/**
 * @brief Activa el modo warning del led de estado
 *
 * @param ms Tiempo entre cambios de estado del modo warning
 */
void warning_status_led(uint16_t ms) {
  if (get_clock_ticks() > lastTicksWarning + ms) {
    toggle_status_led();
    lastTicksWarning = get_clock_ticks();
  }
}

/**
 * @brief Set the RGB color object
 *
 * @param r
 * @param g
 * @param b
 */
void set_RGB_color(uint32_t r, uint32_t g, uint32_t b) {
  timer_set_oc_value(TIM1, TIM_OC4, r);
  timer_set_oc_value(TIM1, TIM_OC3, g);
  timer_set_oc_value(TIM1, TIM_OC2, b);
}

void warning_RGB_color(uint32_t r, uint32_t g, uint32_t b, uint16_t ms) {
  if (get_clock_ticks() > lastTicksWarningRGB + ms) {
    if (lastWarningRGBState) {
      set_RGB_color(0, 0, 0);
    } else {
      set_RGB_color(r, g, b);
    }
    lastTicksWarningRGB = get_clock_ticks();
    lastWarningRGBState = !lastWarningRGBState;
  }
}

void set_RGB_rainbow(void) {
  if (get_clock_ticks() > lastTicksRainbow + 10) {
    lastTicksRainbow = get_clock_ticks();
    rainbowRGB[rainbowColorDesc] -= 20;
    rainbowRGB[rainbowColorAsc] += 20;
    set_RGB_color(rainbowRGB[0], rainbowRGB[1], rainbowRGB[2]);
    if (rainbowRGB[rainbowColorDesc] <= 0 || rainbowRGB[rainbowColorAsc] >= LEDS_MAX_PWM) {
      rainbowRGB[rainbowColorDesc] = 0;
      rainbowRGB[rainbowColorAsc] = LEDS_MAX_PWM;
      set_RGB_color(rainbowRGB[0], rainbowRGB[1], rainbowRGB[2]);
      rainbowColorDesc++;
      if (rainbowColorDesc > 2) {
        rainbowColorDesc = 0;
      }
      rainbowColorAsc = rainbowColorDesc == 2 ? 0 : rainbowColorDesc + 1;
    }
  }
}

void set_neon_fade(uint32_t n) {
  timer_set_oc_value(TIM1, TIM_OC1, n);
}

int32_t fade = 0;
int32_t increment = 20;
void set_neon_heartbeat(void) {
  if (get_clock_ticks() > lastTicksHeartbeat + 10) {
    lastTicksHeartbeat = get_clock_ticks();
    fade += increment;
    if (fade > LEDS_MAX_PWM) {
      fade = LEDS_MAX_PWM;
      increment = -increment;
    }
    if (fade <= 0) {
      fade = 0;
      increment = -increment;
    }
    set_neon_fade(fade);
  }
}

void set_leds_battery_level(float battery_level) {
  // C4 - A3 - A2 - A1 - A0 - A4 - A5 - A6 - A7 - C5

  float percent_battery_level = map(battery_level, BATTERY_LOW_LIMIT_VOLTAGE, BATTERY_HIGH_LIMIT_VOLTAGE, 0.0f, 100.0f);
  if (percent_battery_level <= 10) {
    gpio_clear(GPIOA, GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 | GPIO5 | GPIO6 | GPIO7);
    gpio_clear(GPIOC, GPIO4 | GPIO5);

    if (get_clock_ticks() > lastTicksWarningBateria + 50) {

      gpio_toggle(GPIOA, GPIO0 | GPIO4);

      lastTicksWarningBateria = get_clock_ticks();
    }
  } else if (percent_battery_level <= 20) {
    gpio_clear(GPIOA, GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 | GPIO5 | GPIO6 | GPIO7);
    gpio_clear(GPIOC, GPIO4 | GPIO5);

    gpio_set(GPIOA, GPIO0 | GPIO4);
  } else if (percent_battery_level <= 40) {
    gpio_clear(GPIOA, GPIO2 | GPIO3 | GPIO6 | GPIO7);
    gpio_clear(GPIOC, GPIO4 | GPIO5);

    gpio_set(GPIOA, GPIO0 | GPIO1 | GPIO4 | GPIO5);
  } else if (percent_battery_level <= 60) {
    gpio_clear(GPIOA, GPIO3 | GPIO7);
    gpio_clear(GPIOC, GPIO4 | GPIO5);

    gpio_set(GPIOA, GPIO0 | GPIO1 | GPIO2 | GPIO4 | GPIO5 | GPIO6);
  } else if (percent_battery_level <= 80) {
    gpio_clear(GPIOC, GPIO4 | GPIO5);

    gpio_set(GPIOA, GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 | GPIO5 | GPIO6 | GPIO7);
  } else if (percent_battery_level <= 90) {
    gpio_set(GPIOA, GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 | GPIO5 | GPIO6 | GPIO7);
    gpio_set(GPIOC, GPIO4 | GPIO5);
  } else {
    if (get_clock_ticks() > lastTicksWarningBateria + 50) {

      gpio_toggle(GPIOA, GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 | GPIO5 | GPIO6 | GPIO7);
      gpio_toggle(GPIOC, GPIO4 | GPIO5);

      lastTicksWarningBateria = get_clock_ticks();
    }
  }
}

void set_info_led(uint8_t index, bool state) {

  switch (index) {
    case 0:
      if (state) {
        gpio_set(GPIOC, GPIO4);
      } else {
        gpio_clear(GPIOC, GPIO4);
      }
      break;
    case 1:
      if (state) {
        gpio_set(GPIOA, GPIO3);
      } else {
        gpio_clear(GPIOA, GPIO3);
      }
      break;
    case 2:
      if (state) {
        gpio_set(GPIOA, GPIO2);
      } else {
        gpio_clear(GPIOA, GPIO2);
      }
      break;
    case 3:
      if (state) {
        gpio_set(GPIOA, GPIO1);
      } else {
        gpio_clear(GPIOA, GPIO1);
      }
      break;
    case 4:
      if (state) {
        gpio_set(GPIOA, GPIO0);
      } else {
        gpio_clear(GPIOA, GPIO0);
      }
      break;
    case 5:
      if (state) {
        gpio_set(GPIOA, GPIO4);
      } else {
        gpio_clear(GPIOA, GPIO4);
      }
      break;
    case 6:
      if (state) {
        gpio_set(GPIOA, GPIO5);
      } else {
        gpio_clear(GPIOA, GPIO5);
      }
      break;
    case 7:
      if (state) {
        gpio_set(GPIOA, GPIO6);
      } else {
        gpio_clear(GPIOA, GPIO6);
      }
      break;
    case 8:
      if (state) {
        gpio_set(GPIOA, GPIO7);
      } else {
        gpio_clear(GPIOA, GPIO7);
      }
      break;
    case 9:
      if (state) {
        gpio_set(GPIOC, GPIO5);
      } else {
        gpio_clear(GPIOC, GPIO5);
      }
      break;
  }
}

void set_info_leds_wave(int ms) {
  if (get_clock_ticks() > lastTicksWave + ms) {
    gpio_clear(GPIOA, GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 | GPIO5 | GPIO6 | GPIO7);
    gpio_clear(GPIOC, GPIO4 | GPIO5);

    switch (currentIndexWave) {
      case 0:
        gpio_set(GPIOA, GPIO0 | GPIO4);
        break;
      case 1:
        gpio_set(GPIOA, GPIO1 | GPIO5);
        break;
      case 2:
        gpio_set(GPIOA, GPIO2 | GPIO6);
        break;
      case 3:
        gpio_set(GPIOA, GPIO3 | GPIO7);
        break;
      case 4:
        gpio_set(GPIOC, GPIO4 | GPIO5);
        break;
    }
    currentIndexWave = (currentIndexWave + 1) % 5;
    lastTicksWave = get_clock_ticks();
  }
}

void set_info_leds(void) {

  gpio_set(GPIOA, GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 | GPIO5 | GPIO6 | GPIO7);
  gpio_set(GPIOC, GPIO4 | GPIO5);
}

void clear_info_leds(void) {

  gpio_clear(GPIOA, GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 | GPIO5 | GPIO6 | GPIO7);
  gpio_clear(GPIOC, GPIO4 | GPIO5);
}

void all_leds_clear(void) {
  set_neon_fade(0);
  set_RGB_color(0, 0, 0);
  set_status_led(false);
  clear_info_leds();
}