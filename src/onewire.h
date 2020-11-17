#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define ONEWIRE_RCC RCC_GPIOA
#define ONEWIRE_PORT GPIOA
#define ONEWIRE_PIN GPIO3

void onewire_init(void);
uint8_t onewire_reset(void);
void onewire_write(uint8_t byte);
uint8_t onewire_read(void);
void onewire_write_bit(uint8_t bit);
uint8_t onewire_read_bit(void);

#endif
