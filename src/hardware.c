#include "hardware.h"
#include "persistence.h"
#include "utils.h"
#include "globals.h"
#include <errno.h>
#include <string.h>

static volatile uint32_t system_millis = 0;
static volatile uint16_t beep_time_left = 0;
#define BUZZER_PRESCALER 10U

struct adc_buf_t {
  uint16_t vbat;
  uint16_t throttle;
  uint16_t brake;
};
static volatile struct adc_buf_t adc_buf;

void nops(uint32_t n) {
  volatile uint32_t i;
  for (i = 0; i < n; i++) {
    __asm__("nop");
  }
}

void sys_tick_handler(void) {
  system_millis++;

  if (beep_time_left > 0) {
    if (beep_time_left == 1) {
      buzzer_freq_vol(0, 0);
    }
    --beep_time_left;
  }
}

inline uint32_t millis(void) { return system_millis; }

void msleep(uint32_t ms) {
  uint32_t wake = system_millis + ms;
  while (wake > system_millis) {
    iwdg_reset();
  }
}

void buzzer_freq_vol(uint16_t freq, uint8_t volume) {
  if (freq == 0) {
    timer_disable_oc_output(TIM1, TIM_OC4);
    gpio_clear(GPIOA, GPIO_TIM1_CH4);
  } else {
    uint16_t period = (rcc_ahb_frequency / BUZZER_PRESCALER) / freq;
    timer_enable_oc_output(TIM1, TIM_OC4);
    timer_set_period(TIM1, period);
    timer_set_oc_value(TIM1, TIM_OC4, convert_range_u16(volume, 100U, period / 2));
  }
}

void beep_blocking(uint16_t freq, uint16_t ms, uint8_t volume) {
  if(!storage.mute) {
    beep_time_left = 0;
    buzzer_freq_vol(freq, volume);
  }
  msleep(ms);
  buzzer_freq_vol(0, volume);
}

void beep(uint16_t freq, uint16_t ms, uint8_t volume) {
  if(storage.mute) {
    buzzer_freq_vol(0, 0);
    return;
  }
  buzzer_freq_vol(freq, volume);
  beep_time_left = ms;
}


uint16_t throttle_adc_value(void) { return adc_buf.throttle; }

uint16_t brake_adc_value(void) { return adc_buf.brake; }

uint16_t throttle_value_normalized(void) {
  uint16_t clamped = clamp_u16(throttle_adc_value(),
                               storage.throttle_trigger_min_value,
                               storage.throttle_trigger_max_value);

  return ((clamped - storage.throttle_trigger_min_value) * TRIGGER_MAX) /
         (storage.throttle_trigger_max_value - storage.throttle_trigger_min_value);
}

float throttle_value_to_kmh(void){
  return convert_range_float(throttle_value_normalized(), TRIGGER_MAX, storage.speed_limit_last);
}

uint16_t brake_value_normalized(void) {
  uint16_t clamped = clamp_u16(brake_adc_value(),
                               storage.brake_trigger_min_value,
                               storage.brake_trigger_max_value);

  return ((clamped - storage.brake_trigger_min_value) * TRIGGER_MAX) /
         (storage.brake_trigger_max_value - storage.brake_trigger_min_value);
}

uint16_t battery_value(void) {
  // volts per unit = 3.3v / 4096 = 0.0008056640625
  // res divider = r2 / (r1 + r2) = 3.3k / (47k + 3.3k) = 0.06560636182902585
  // coeff = (res divider / volts per unit)*100 = ~814
  // voltage_int = (adc_val * 1000) / coeff
  // volts = voltage_int / 10
  // .1 volts = voltage_int % 10
  return ((uint32_t)adc_buf.vbat * 100) / 814;
}

static void enable_peripherals(void) {
  rcc_periph_clock_enable(RCC_AFIO);
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);
  rcc_periph_clock_enable(RCC_ADC1);
  rcc_periph_clock_enable(RCC_USART1);
  rcc_periph_clock_enable(RCC_USART3);
  rcc_periph_clock_enable(RCC_SPI2);
  rcc_periph_clock_enable(RCC_I2C1);
  rcc_periph_clock_enable(RCC_TIM1);
  rcc_periph_clock_enable(RCC_DMA1);
}

static void systick_setup(void) {
  rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
  systick_set_frequency(1000, rcc_ahb_frequency); // 1 ms
  systick_counter_enable();
  systick_interrupt_enable();
}

void gpio_setup(void) {
  // builtin led
  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
                GPIO13);
  gpio_set(GPIOC, GPIO13);

  // power en
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
                GPIO8);

  // power, light, speed buttons
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN,
                GPIO5 | GPIO6 | GPIO7);
  gpio_set(GPIOA, GPIO5 | GPIO6 | GPIO7);

  // set, beep buttons
  gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN,
                GPIO0 | GPIO1);
  gpio_set(GPIOB, GPIO0 | GPIO1);

  // adc
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG,
                GPIO0 | GPIO1 | GPIO2);

  // usart1
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                GPIO_USART1_TX);
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN,
                GPIO_USART1_RX);
  gpio_set(GPIOA, GPIO_USART1_RX);

   // usart3
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                GPIO_USART3_TX);
  gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN,
                GPIO_USART3_RX);
  gpio_set(GPIOB, GPIO_USART3_RX);

  // i2c1
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN,
                GPIO_I2C1_SCL | GPIO_I2C1_SDA);

  // buzzer pwm
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                GPIO_TIM1_CH4);

  // light
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
                GPIO8);
}

static void i2c_setup(void) {
  i2c_reset(I2C1);
  i2c_peripheral_disable(I2C1);
  i2c_enable_ack(I2C1);
  i2c_set_fast_mode(I2C1);
  i2c_set_speed(I2C1, i2c_speed_fm_400k, rcc_apb1_frequency / 1000000);
  i2c_peripheral_enable(I2C1);
}

static void pwm_setup(void) {
  rcc_periph_reset_pulse(RST_TIM1);

  // timer mode: no divider (72MHz), edge aligned, upcounting
  timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  timer_set_prescaler(TIM1, BUZZER_PRESCALER-1);

  // PWM mode 1 (output high if CNT > CCR1)
  timer_set_oc_mode(TIM1, TIM_OC4, TIM_OCM_PWM1);

  timer_enable_oc_preload(TIM1, TIM_OC4);
  timer_enable_preload(TIM1);

  timer_enable_break_main_output(TIM1);

  timer_disable_oc_output(TIM1, TIM_OC4);

  // timer_enable_oc_output(TIM1, TIM_OC4);

  timer_enable_counter(TIM1);
}

void dma1_channel1_isr(void) {
  dma_clear_interrupt_flags(DMA1, DMA_CHANNEL1, DMA_IFCR_CGIF1);
}

static void adc_setup_dma(void) {
  uint8_t channel_seq[16];

  dma_disable_channel(DMA1, DMA_CHANNEL1);

  dma_enable_circular_mode(DMA1, DMA_CHANNEL1);
  dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL1);

  dma_set_peripheral_size(DMA1, DMA_CHANNEL1, DMA_CCR_PSIZE_16BIT);
  dma_set_memory_size(DMA1, DMA_CHANNEL1, DMA_CCR_MSIZE_16BIT);

  dma_set_read_from_peripheral(DMA1, DMA_CHANNEL1);
  dma_set_peripheral_address(DMA1, DMA_CHANNEL1, (uint32_t)&ADC_DR(ADC1));

  dma_set_memory_address(DMA1, DMA_CHANNEL1, (uint32_t)&adc_buf);
  dma_set_number_of_data(DMA1, DMA_CHANNEL1, 3);

  dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL1);
  dma_enable_channel(DMA1, DMA_CHANNEL1);

  adc_power_off(ADC1);

  adc_enable_scan_mode(ADC1);
  adc_set_continuous_conversion_mode(ADC1);
  adc_disable_discontinuous_mode_regular(ADC1);

  adc_enable_external_trigger_regular(ADC1, ADC_CR2_EXTSEL_SWSTART);
  adc_set_right_aligned(ADC1);
  adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_7DOT5CYC);

  adc_power_on(ADC1);

  nops(800000);

  adc_reset_calibration(ADC1);
  adc_calibrate(ADC1);

  channel_seq[0] = ADC_CHANNEL0;
  channel_seq[1] = ADC_CHANNEL1;
  channel_seq[2] = ADC_CHANNEL2;

  adc_set_regular_sequence(ADC1, 3, channel_seq);

  adc_enable_dma(ADC1);
  nops(100);
  adc_start_conversion_regular(ADC1);
}

static void usart_setup(void) {
  nvic_enable_irq(NVIC_USART1_IRQ);
  usart_set_baudrate(USART1, 9600);
  usart_set_databits(USART1, 8);
  usart_set_stopbits(USART1, USART_STOPBITS_1);
  usart_set_mode(USART1, USART_MODE_TX_RX);
  usart_set_parity(USART1, USART_PARITY_NONE);
  usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
  usart_enable_rx_interrupt(USART1);
  usart_enable(USART1);

  usart_set_baudrate(USART3, 9600);
  usart_set_databits(USART3, 8);
  usart_set_stopbits(USART3, USART_STOPBITS_1);
  usart_set_mode(USART3, USART_MODE_TX_RX);
  usart_set_parity(USART3, USART_PARITY_NONE);
  usart_set_flow_control(USART3, USART_FLOWCONTROL_NONE);
  usart_enable(USART3);
}

/* printf redirect */
int _write(int file, char *ptr, int len) {
  int i;

  if (file == 1) {
    for (i = 0; i < len; i++) {
      usart_send_blocking(USART3, ptr[i]);
    }
    return i;
  }

  errno = EIO;
  return -1;
}

// extern void initialise_monitor_handles(void);

void hardware_setup(void) {
  enable_peripherals();
  systick_setup();
  gpio_setup();

  power_enable();

  adc_setup_dma();
  usart_setup();
  i2c_setup();
  pwm_setup();
  iwdg_set_period_ms(2000); // 2s watchdog
  iwdg_start();
}
