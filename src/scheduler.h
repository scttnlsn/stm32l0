#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <stdint.h>

void scheduler_init(void);
void scheduler_every(uint32_t period, void (*callback)(void));
void scheduler_once(uint32_t delay, void (*callback)(void));
void scheduler_run(void);

#endif
