#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/exti.h>

#include "rtc.h"
#include "led.h"

void rtc_init(void) {
  // disable RTC register write protection
  pwr_disable_backup_domain_write_protect();

  // reset all RTC registers
  RCC_CSR |= RCC_CSR_RTCRST;
  RCC_CSR &= ~RCC_CSR_RTCRST;

  // enable LSI
  rcc_osc_on(RCC_LSI);
  rcc_wait_for_osc_ready(RCC_LSI);

  // use LSI for RTC clock (37kHz)
  RCC_CSR &= ~(RCC_CSR_RTCSEL_MASK << RCC_CSR_RTCSEL_SHIFT);
  RCC_CSR |= (RCC_CSR_RTCSEL_LSI << RCC_CSR_RTCSEL_SHIFT);

  // RM section 22.4.3
  // (288 + 1) * (127 + 1) = 36992
  // as close as we can get to 37000 (a 1 second timeout period)
  uint32_t prediv_a = 127;
  uint32_t prediv_s = 288;
  rtc_set_prescaler(prediv_s, prediv_a);

  // enable RTC
  RCC_CSR |= RCC_CSR_RTCEN;
}

void rtc_set_wakeup(uint16_t seconds) {
  // RM section 22.4.6 (periodic auto wakeup)

  rtc_unlock();

  // our SPRE is 1 Hz => wakeup value in seconds
  rtc_set_wakeup_time(seconds, RTC_CR_WUCLKSEL_SPRE);

  // enable periodic wakeup
  RTC_CR |= RTC_CR_WUTIE;

  // RM section 6.3.1
  // we'll be waking up from stop mode => EXTI 20 rising edges
  exti_set_trigger(EXTI20, EXTI_TRIGGER_RISING);
  exti_enable_request(EXTI20);

  rtc_lock();

  // enable RTC interrupts
  nvic_enable_irq(NVIC_RTC_IRQ);
}

void rtc_isr(void) {
  rtc_clear_wakeup_flag();
  exti_reset_request(EXTI20);
}
