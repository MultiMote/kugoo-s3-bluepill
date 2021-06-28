#ifndef HARDWARE_H
#define HARDWARE_H

#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/cm3/nvic.h>

void hardware_setup(void);
uint32_t millis(void);
void msleep(uint32_t ms);

/**
 * @brief Outputs frequency to the buzzer
 *
 * Do not set frequency lower than 150 Hz
 *
 * @param freq Frequency in hertz
 * @param volume 0-100
 */
void buzzer_freq_vol(uint16_t freq, uint8_t volume);
void beep_blocking(uint16_t freq, uint16_t ms, uint8_t volume);
void beep(uint16_t freq, uint16_t ms, uint8_t volume);

uint16_t throttle_adc_value(void);
uint16_t brake_adc_value(void);

/// 0-1000, based on min and max value
uint16_t throttle_value_normalized(void);
/// Get throttle trigger value in range from 0 to speed limit
float throttle_value_to_kmh(void);

/// 0-1000, based on min and max value
uint16_t brake_value_normalized(void);

// No floating point, 39.2v is 392
uint16_t battery_value(void);

#define power_enable() gpio_set(GPIOB, GPIO8)
#define power_disable() do { gpio_clear(GPIOB, GPIO8); while(1){} } while(0)

#define light_on() gpio_set(GPIOA, GPIO8)
#define light_off() gpio_clear(GPIOA, GPIO8)
#define light_toggle() gpio_toggle(GPIOA, GPIO8)


#define set_button_pressed() (!gpio_get(GPIOB, GPIO0))
#define power_button_pressed() (!gpio_get(GPIOA, GPIO7))
#define light_button_pressed() (!gpio_get(GPIOA, GPIO6))

#ifdef SWAP_BEEP_AND_SPEED
    #define beep_button_pressed() (!gpio_get(GPIOA, GPIO5))
    #define speed_button_pressed() (!gpio_get(GPIOB, GPIO1))
#else
    #define beep_button_pressed() (!gpio_get(GPIOB, GPIO1))
    #define speed_button_pressed() (!gpio_get(GPIOA, GPIO5))
#endif

#endif /* HARDWARE_H */
