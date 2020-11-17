#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "delay.h"
#include "onewire.h"

void onewire_init(void) {
  rcc_periph_clock_enable(ONEWIRE_RCC);
  gpio_mode_setup(ONEWIRE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ONEWIRE_PIN);

  // start with the bus high
  gpio_set(ONEWIRE_PORT, ONEWIRE_PIN);
}

// returns 1 if the device asserts a presence pulse
uint8_t onewire_reset(void) {
  // pull pin low for 480us
  gpio_mode_setup(ONEWIRE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ONEWIRE_PIN);
  gpio_clear(ONEWIRE_PORT, ONEWIRE_PIN);
  delay_us(480);

  // if present, the device will hold the pin low for at least 60us
  gpio_mode_setup(ONEWIRE_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ONEWIRE_PIN);
  delay_us(70);
  uint8_t res = gpio_get(ONEWIRE_PORT, ONEWIRE_PIN);

  // TODO: wait for line to go high instead of relying on timing

  delay_us(410);

  return !res;
}

void onewire_write_bit(uint8_t bit) {
  if (bit) {
    // hold low for <15us => write a 1
    gpio_mode_setup(ONEWIRE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ONEWIRE_PIN);
    gpio_clear(ONEWIRE_PORT, ONEWIRE_PIN);
    delay_us(6);
    gpio_set(ONEWIRE_PORT, ONEWIRE_PIN);
    delay_us(64);
  } else {
    // hold low for >60us => write a 0
    gpio_mode_setup(ONEWIRE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ONEWIRE_PIN);
    gpio_clear(ONEWIRE_PORT, ONEWIRE_PIN);
    delay_us(60);
    gpio_set(ONEWIRE_PORT, ONEWIRE_PIN);
    delay_us(10);
  }
}

void onewire_write(uint8_t byte) {
  for (uint8_t i = 0; i < 8; i++) {
    uint8_t bit = (byte >> i) & 0x01;
    onewire_write_bit(bit);
  }
}

uint8_t onewire_read_bit(void) {
  // hold low for <15us
  gpio_mode_setup(ONEWIRE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ONEWIRE_PIN);
  gpio_clear(ONEWIRE_PORT, ONEWIRE_PIN);
  delay_us(6);

  // device will hold low for 60us (0), otherwise it will do nothing (1)
  gpio_mode_setup(ONEWIRE_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ONEWIRE_PIN);
  delay_us(1);
  uint8_t res = gpio_get(ONEWIRE_PORT, ONEWIRE_PIN);

  delay_us(55);

  return !!res;
}

uint8_t onewire_read(void) {
  uint8_t byte = 0x00;

  for (uint8_t i = 0; i < 8; i++) {
    if (onewire_read_bit()) {
      byte |= (0x01 << i);
    }
  }

  return byte;
}
