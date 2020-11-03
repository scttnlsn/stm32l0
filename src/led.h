#ifndef __LED_H__
#define __LED_H__

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define LED_RCC RCC_GPIOB
#define LED_PORT GPIOB
#define LED_PIN GPIO3

void led_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);

#endif
