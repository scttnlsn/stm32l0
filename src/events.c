#include "events.h"
#include "ringbuf.h"

#define MAX_EVENTS 10

static ringbuf_t ringbuf;
static event_t events[MAX_EVENTS];

void events_init(void) {
  ringbuf_init(&ringbuf, events, sizeof(events) / sizeof(event_t), sizeof(event_t));
}

bool events_enqueue(event_t event) {
  return ringbuf_push(&ringbuf, &event);
}

bool events_dequeue(event_t *event) {
  return ringbuf_pop(&ringbuf, event);
}
