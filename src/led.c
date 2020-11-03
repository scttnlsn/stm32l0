#include "led.h"

static uint8_t led_state = 0;

void led_init(void) {
  rcc_periph_clock_enable(LED_RCC);
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
}

void led_on(void) {
  gpio_set(LED_PORT, LED_PIN);
  led_state = 1;
}

void led_off(void) {
  gpio_clear(LED_PORT, LED_PIN);
  led_state = 0;
}

void led_toggle(void) {
  if (led_state) {
    led_off();
  } else {
    led_on();
  }
}
