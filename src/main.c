#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <stdio.h>

#include "delay.h"
#include "ds18b20.h"
#include "led.h"
#include "onewire.h"
#include "power.h"
#include "rfm69.h"
#include "rtc.h"
#include "spi.h"
#include "tick.h"
#include "usart.h"

#define NETWORK_ID 100
#define GATEWAY_ADDR 1
#define NODE_ADDR 10
#define ENCRYPTION_KEY "ABCDEFGHIJKLMNOP" // must be 16 bytes

static void init(void) {
  // high-speed internal (16mHz)
  rcc_osc_on(RCC_HSI16);
  rcc_wait_for_osc_ready(RCC_HSI16);
  rcc_set_sysclk_source(RCC_HSI16);

  // these are defined globally by libopencm3
  rcc_ahb_frequency = 16e6;
  rcc_apb1_frequency = 16e6;
  rcc_apb2_frequency = 16e6;

  tick_init();
  delay_init();
  power_init();
  rtc_init();
  spi_init();
  led_init();
  rfm69_init(NETWORK_ID, NODE_ADDR);
  rfm69_set_key(ENCRYPTION_KEY);
  onewire_init();
}

static void sleep(void) {
  rtc_set_wakeup(3);

  rfm69_sleep();
  spi_disable_();
  tick_disable();

  power_enter_stop_mode();

  // wakeup
  tick_enable();
  spi_enable_();
  rfm69_wakeup();
}

int main(void) {
  init();

  // FOR DEBUGGING
  // usart_init();
  // printf("");

  while (1) {
    led_on();

    float temp = ds18b20_read_temp();
    rfm69_send(GATEWAY_ADDR, &temp, sizeof(float));

    tick_delay(100);
    led_off();

    sleep();
  }

  return 0;
}
