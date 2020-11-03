#ifndef __RTC_H__
#define __RTC_H__

#include <stdint.h>

void rtc_init(void);
void rtc_set_wakeup(uint16_t seconds);

#endif
