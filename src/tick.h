#ifndef __TICK_H__
#define __TICK_H__

#include <stdint.h>

void tick_init(void);
uint32_t tick_get(void);
uint32_t tick_since(uint32_t since);
void tick_delay(uint32_t ticks);
void tick_enable(void);
void tick_disable(void);

#endif
