#include "setup.h"

/**
 * @brief Configura los relojes principales del robot
 *
 * SYSCLK a 168 MHz
 * GPIO: A, B, C
 * USART3
 * Timers: 1-PWM, 2-PID, 8-OneShot125
 * ADC
 * DWT
 *
 */
static void setup_clock(void) {
  rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]); // TODO: 84MHz?? :(

  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);

  rcc_periph_clock_enable(RCC_SYSCFG);

  rcc_periph_clock_enable(RCC_USART3);

  rcc_periph_clock_enable(RCC_SPI3);

  rcc_periph_clock_enable(RCC_DMA2);

  rcc_periph_clock_enable(RCC_TIM1);
  rcc_periph_clock_enable(RCC_TIM2);
  rcc_periph_clock_enable(RCC_TIM3);
  rcc_periph_clock_enable(RCC_TIM4);
  rcc_periph_clock_enable(RCC_TIM5);
  rcc_periph_clock_enable(RCC_TIM8);

  rcc_periph_clock_enable(RCC_ADC1);
  rcc_periph_clock_enable(RCC_ADC2);

  dwt_enable_cycle_counter();
}

/**
 * @brief Configura el SysTick para 1ms
 *
 */
static void setup_systick(void) {
  systick_set_frequency(SYSTICK_FREQUENCY_HZ, SYSCLK_FREQUENCY_HZ);
  systick_counter_enable();
  systick_interrupt_enable();
}

static void setup_timer_priorities(void) {
  nvic_set_priority(NVIC_SYSTICK_IRQ, 16 * 0);
  nvic_set_priority(NVIC_DMA2_STREAM0_IRQ, 16 * 1);
  nvic_set_priority(NVIC_TIM2_IRQ, 16 * 2);
  nvic_set_priority(NVIC_TIM5_IRQ, 16 * 3);
  nvic_set_priority(NVIC_USART3_IRQ, 16 * 4);

  nvic_enable_irq(NVIC_DMA2_STREAM0_IRQ);
  nvic_enable_irq(NVIC_TIM5_IRQ);
  nvic_enable_irq(NVIC_TIM2_IRQ);
  nvic_enable_irq(NVIC_USART3_IRQ);
}

static void setup_usart(void) {
  usart_set_baudrate(USART3, 115200);
  usart_set_databits(USART3, 8);
  usart_set_stopbits(USART3, USART_STOPBITS_1);
  usart_set_parity(USART3, USART_PARITY_NONE);
  usart_set_flow_control(USART3, USART_FLOWCONTROL_NONE);
  usart_set_mode(USART3, USART_MODE_TX_RX);
  // USART_CR1(USART3) |= USART_CR1_RXNEIE;
  // usart_enable_tx_interrupt(USART3);
  usart_enable(USART3);
}

static void setup_gpio(void) {

  // Entradas analógicas sensores de línea y batería
  gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0 | GPIO1 | GPIO2 | GPIO3);

  // Salidas digitales multiplexadores
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13 | GPIO14 | GPIO15);

  // Entradas digitales Menú e IR-Remote
  gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO12 | GPIO13 | GPIO14 | GPIO15);

  // Entradas Encoders
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4 | GPIO5 | GPIO6 | GPIO7);
  gpio_set_af(GPIOB, GPIO_AF2, GPIO4 | GPIO5 | GPIO6 | GPIO7);

  // Salida digital LED's Menu
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 | GPIO5 | GPIO6 | GPIO7);
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO4 | GPIO5);

  // Salida PWM LEDS
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8 | GPIO9 | GPIO10 | GPIO11);
  gpio_set_af(GPIOA, GPIO_AF1, GPIO8 | GPIO9 | GPIO10 | GPIO11);

  // Salida PWM Motores
  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7 | GPIO8 | GPIO9);
  gpio_set_af(GPIOC, GPIO_AF3, GPIO6 | GPIO7 | GPIO8 | GPIO9);

  // USART3
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11);
  gpio_set_af(GPIOB, GPIO_AF7, GPIO10 | GPIO11);

  // MPU
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO15);
  gpio_set(GPIOA, GPIO15);
  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, GPIO10 | GPIO11 | GPIO12);
  gpio_set_af(GPIOC, GPIO_AF6, GPIO10 | GPIO11 | GPIO12);
}

static void setup_adc1(void) {
  adc_power_off(ADC1);
  adc_disable_external_trigger_regular(ADC1);
  adc_set_resolution(ADC1, ADC_CR1_RES_12BIT);
  adc_set_right_aligned(ADC1);
  // adc_set_clk_prescale(ADC_CCR_ADCPRE_BY2);
  adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_15CYC);
  adc_enable_scan_mode(ADC1);

  adc_set_regular_sequence(ADC1, get_adc_channels_num(), get_adc_channels());
  adc_set_continuous_conversion_mode(ADC1);
  adc_enable_eoc_interrupt(ADC1);

  adc_power_on(ADC1);
  int i;
  for (i = 0; i < 800000; i++) {
    /* Wait a bit. */
    __asm__("nop");
  }

  adc_start_conversion_regular(ADC1);
}

/**
 * @brief Configura el ADC2 para lectura del sensor de batería
 *
 */
static void setup_adc2(void) {
  uint8_t lista_canales[16];

  lista_canales[0] = ADC_CHANNEL13;
  adc_power_off(ADC2);
  adc_disable_scan_mode(ADC2);
  adc_set_single_conversion_mode(ADC2);
  adc_set_right_aligned(ADC2);
  adc_set_sample_time_on_all_channels(ADC2, ADC_SMPR_SMP_15CYC);
  adc_set_regular_sequence(ADC2, 1, lista_canales);
  adc_power_on(ADC2);
}

static void setup_dma_adc1(void) {
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_ADC1EN);
  rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_DMA2EN);
  dma_stream_reset(DMA2, DMA_STREAM0);

  dma_set_peripheral_address(DMA2, DMA_STREAM0, (uint32_t)&ADC_DR(ADC1));
  dma_set_memory_address(DMA2, DMA_STREAM0, (uint32_t)get_adc_raw());
  dma_enable_memory_increment_mode(DMA2, DMA_STREAM0);
  dma_set_peripheral_size(DMA2, DMA_STREAM0, DMA_SxCR_PSIZE_16BIT);
  dma_set_memory_size(DMA2, DMA_STREAM0, DMA_SxCR_MSIZE_16BIT);
  dma_set_priority(DMA2, DMA_STREAM0, DMA_SxCR_PL_LOW);

  dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM0);
  // dma_enable_half_transfer_interrupt(DMA2, DMA_STREAM0);
  dma_set_number_of_data(DMA2, DMA_STREAM0, get_adc_channels_num());
  dma_enable_circular_mode(DMA2, DMA_STREAM0);
  dma_set_transfer_mode(DMA2, DMA_STREAM0, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
  dma_channel_select(DMA2, DMA_STREAM0, DMA_SxCR_CHSEL_0);

  dma_enable_stream(DMA2, DMA_STREAM0);
  adc_enable_dma(ADC1);
  adc_set_dma_continue(ADC1);
}

void dma2_stream0_isr(void) {
  if (dma_get_interrupt_flag(DMA2, DMA_STREAM0, DMA_TCIF)) {
    sensors_update_mux_readings();
    dma_clear_interrupt_flags(DMA2, DMA_STREAM0, DMA_TCIF);
  }
}

static void setup_leds_pwm(void) {
  timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  timer_set_prescaler(TIM1, rcc_apb2_frequency * 2 / 4000000 - 2);
  // 400000 es la frecuencia a la que irá el PWM 4 kHz, los dos últimos ceros no se porqué, pero son necesarios ya que rcc_apb2_frequency también añade dos ceros a mayores
  timer_set_repetition_counter(TIM1, 0);
  timer_enable_preload(TIM1);
  timer_continuous_mode(TIM1);
  timer_set_period(TIM1, LEDS_MAX_PWM);

  timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM1, TIM_OC2, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM1, TIM_OC3, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM1, TIM_OC4, TIM_OCM_PWM1);
  timer_set_oc_value(TIM1, TIM_OC1, 0);
  timer_set_oc_value(TIM1, TIM_OC2, 0);
  timer_set_oc_value(TIM1, TIM_OC3, 0);
  timer_set_oc_value(TIM1, TIM_OC4, 0);
  timer_enable_oc_output(TIM1, TIM_OC1);
  timer_enable_oc_output(TIM1, TIM_OC2);
  timer_enable_oc_output(TIM1, TIM_OC3);
  timer_enable_oc_output(TIM1, TIM_OC4);

  timer_enable_break_main_output(TIM1);

  timer_enable_counter(TIM1);
}

static void setup_motors_pwm(void) {
  timer_set_mode(TIM8, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  // 84000000
  timer_set_prescaler(TIM8, rcc_apb2_frequency * 2 / 4000000 - 2);
  // 4000000 es la frecuencia a la que irá el PWM 4 kHz, los dos últimos ceros no se porqué, pero son necesarios ya que rcc_apb2_frequency también añade dos ceros a mayores
  timer_set_repetition_counter(TIM8, 0);
  timer_enable_preload(TIM8);
  timer_continuous_mode(TIM8);
  timer_set_period(TIM8, MOTORS_MAX_PWM);

  timer_set_oc_mode(TIM8, TIM_OC1, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM8, TIM_OC2, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM8, TIM_OC3, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM8, TIM_OC4, TIM_OCM_PWM1);

  timer_enable_break_main_output(TIM8);

  timer_enable_counter(TIM8);
}

static void setup_main_loop_timer(void) {
  rcc_periph_reset_pulse(RST_TIM5);
  timer_set_mode(TIM5, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_prescaler(TIM5, ((rcc_apb1_frequency * 2) / 1000000 - 2));
  timer_disable_preload(TIM5);
  timer_continuous_mode(TIM5);
  timer_set_period(TIM5, 1024);

  timer_enable_counter(TIM5);
  timer_enable_irq(TIM5, TIM_DIER_CC1IE);
}

void tim5_isr(void) {
  if (timer_get_flag(TIM5, TIM_SR_CC1IF)) {
    timer_clear_flag(TIM5, TIM_SR_CC1IF);
    control_loop();
  }
}

static void setup_quadrature_encoders(void) {
  timer_set_period(TIM4, 0xFFFF);
  timer_slave_set_mode(TIM4, TIM_SMCR_SMS_EM3);
  timer_ic_set_input(TIM4, TIM_IC2, TIM_IC_IN_TI1);
  timer_ic_set_input(TIM4, TIM_IC1, TIM_IC_IN_TI2);
  timer_enable_counter(TIM4);

  timer_set_period(TIM3, 0xFFFF);
  timer_slave_set_mode(TIM3, TIM_SMCR_SMS_EM3);
  timer_ic_set_input(TIM3, TIM_IC2, TIM_IC_IN_TI1);
  timer_ic_set_input(TIM3, TIM_IC1, TIM_IC_IN_TI2);
  timer_enable_counter(TIM3);
}

/**
 * @brief Setup SPI.
 *
 * SPI is configured as follows:
 *
 * - Master mode.
 * - Clock baud rate: PCLK1 / speed_div; PCLK1 = 36MHz.
 * - Clock polarity: 0 (idle low; leading edge is a rising edge).
 * - Clock phase: 0 (out changes on the trailing edge and input data
 *   captured on rising edge).
 * - Data frame format: 8-bits.
 * - Frame format: MSB first.
 *
 * NSS is configured to be managed by software.
 *
 * Reference: https://github.com/Bulebots/meiga
 */
static void setup_spi(uint8_t speed_div) {
  spi_reset(SPI3);

  spi_init_master(SPI3, speed_div, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);

  spi_enable_software_slave_management(SPI3);
  spi_set_nss_high(SPI3);

  spi_enable(SPI3);
}

/**
 * @brief Setup SPI for gyroscope read, less than 20 MHz.
 *
 * The clock baudrate is 84 MHz / 8 = 10.5 MHz.
 *
 * Reference: https://github.com/Bulebots/meiga
 */
void setup_spi_high_speed(void) {
  setup_spi(SPI_CR1_BAUDRATE_FPCLK_DIV_8);
  delay(100);
}

/**
 * @brief Setup SPI for gyroscope Write, less than 1 MHz.
 *
 * The clock baudrate is 84 MHz / 128 = 0.65625 MHz.
 *
 * Reference: https://github.com/Bulebots/meiga
 */
void setup_spi_low_speed(void) {
  setup_spi(SPI_CR1_BAUDRATE_FPCLK_DIV_128);
  delay(100);
}

void setup(void) {
  setup_clock();
  setup_systick();
  setup_timer_priorities();
  setup_usart();
  setup_gpio();
  setup_adc2();
  setup_adc1();
  setup_dma_adc1();
  setup_leds_pwm();
  setup_motors_pwm();
  setup_main_loop_timer();
  setup_quadrature_encoders();
  setup_mpu();
}