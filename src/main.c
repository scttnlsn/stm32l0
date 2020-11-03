#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <stdio.h>

#include "led.h"
#include "rtc.h"
#include "tick.h"
#include "usart.h"
#include "power.h"

void init(void);
void sleep(void);

void init(void) {
  // high-speed internal (16mHz)
  rcc_osc_on(RCC_HSI16);
  rcc_wait_for_osc_ready(RCC_HSI16);
  rcc_set_sysclk_source(RCC_HSI16);

  // these are defined globally by libopencm3
  rcc_ahb_frequency = 16e6;
  rcc_apb1_frequency = 16e6;
  rcc_apb2_frequency = 16e6;

  tick_init();
  led_init();
  power_init();
  rtc_init();
}

void sleep(void) {
  tick_disable();

  rtc_set_wakeup(3);
  power_enter_stop_mode();

  tick_enable();
}

int main(void) {
  init();

  // FOR DEBUGGING
  // usart_init();
  // printf("");

  while (1) {
    led_on();
    tick_delay(100);
    led_off();

    sleep();
  }

  return 0;
}
