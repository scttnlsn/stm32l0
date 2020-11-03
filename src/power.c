#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/scb.h>

#include "power.h"

void power_init(void) {
  rcc_periph_clock_enable(RCC_PWR);
}

void power_enter_stop_mode() {
  // RM section 6.3.9 (stop mode)

  pwr_clear_wakeup_flag();
  pwr_set_stop_mode();

  // ultra-low power mode
  // fast wake-up
  // regulator back into main mode after exiting low-power mode
  PWR_CR |= PWR_CR_ULP | PWR_CR_FWU | PWR_CR_LPSDSR;

  // regulator in Low-power deepsleep mode
  pwr_voltage_regulator_low_power_in_stop();

  // wake-up from stop clock selection
  RCC_CFGR |= RCC_CFGR_STOPWUCK_HSI16;

  // use "deep sleep" as low-power mode
  SCB_SCR |= SCB_SCR_SLEEPDEEP;

  // wait for interrupt
  __asm volatile ("wfi");
}
