#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <stdbool.h>
#include <stdint.h>

#define DATA_LENGTH 2

typedef struct {
  uint32_t type;
  uint32_t data[DATA_LENGTH];
} event_t;

void events_init(void);
bool events_enqueue(event_t event);
bool events_dequeue(event_t *event);

#endif
