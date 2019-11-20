#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <stdio.h>

#include "events.h"
#include "scheduler.h"
#include "tick.h"
#include "usart.h"

#define LED_RCC RCC_GPIOB
#define LED_PORT GPIOB
#define LED_PIN GPIO3
#define LED_BLINK_DELAY 250

void init(void);
void clock_init(void);
void led_init(void);
void blink_task(void);
void counter_task(void);

void init(void) {
  clock_init();
  tick_init();
  usart_init();
  events_init();
  scheduler_init();

  led_init();
}

void clock_init(void) {
  // Internal 16mHz oscillator
  rcc_osc_on(RCC_HSI16);
  rcc_wait_for_osc_ready(RCC_HSI16);
  rcc_set_sysclk_source(RCC_HSI16);

  // these are defined globally by libopencm3
  rcc_ahb_frequency = 16e6;
  rcc_apb1_frequency = 16e6;
  rcc_apb2_frequency = 16e6;
}

void led_init(void) {
  rcc_periph_clock_enable(LED_RCC);
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
}

static uint8_t led_on = 0;

void blink_task(void) {
  if (led_on) {
    gpio_clear(LED_PORT, LED_PIN);
  } else {
    gpio_set(LED_PORT, LED_PIN);
  }

  led_on = !led_on;
}

typedef enum {
  EVENT_COUNTER
} event_type_t;

static uint32_t counter = 0;

void counter_task(void) {
  counter++;

  event_t event;
  event.type = EVENT_COUNTER;
  event.data[0] = counter;
  events_enqueue(event);
}

int main(void) {
  init();
  printf("ready\r\n");

  scheduler_every(250, &blink_task);
  scheduler_every(2000, &counter_task);

  while (1) {
    scheduler_run();

    event_t event;
    if (events_dequeue(&event)) {
      if (event.type == EVENT_COUNTER) {
        printf("counter: %ld\r\n", event.data[0]);
      }
    }
  }

  return 0;
}
