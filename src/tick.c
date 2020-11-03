#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include "tick.h"

#define TICK_MAX 0xFFFFFFFF // 32-bit

static volatile uint32_t counter;

void sys_tick_handler(void) {
  counter++;
}

void tick_init(void) {
  systick_counter_disable();
  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);

  // clock speed / 1000 - 1 = 1ms ticks
  systick_set_reload(15999);
  systick_clear();

  tick_enable();
}

uint32_t tick_get(void) {
  return counter;
}

uint32_t tick_since(uint32_t since) {
  uint32_t now = tick_get();

  if (now >= since) {
    return now - since;
  } else {
    // counter overflow (roughly every 50 days)
    return now + (TICK_MAX - since);
  }
}

void tick_delay(uint32_t ticks) {
  uint32_t target = counter + ticks;
  while (counter < target);
}

void tick_enable(void) {
  systick_interrupt_enable();
  systick_counter_enable();
}

void tick_disable(void) {
  systick_interrupt_disable();
  systick_counter_disable();
}
