#ifndef __RINGBUF_H__
#define __RINGBUF_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint32_t capacity;
  uint32_t count;
  uint32_t size;
  void *buffer;
  void *head;
  void *tail;
} ringbuf_t;

void ringbuf_init(ringbuf_t *ringbuf, void *buffer, uint32_t capacity, uint32_t size);
bool ringbuf_push(ringbuf_t *ringbuf, const void *data);
bool ringbuf_pop(ringbuf_t *ringbuf, void *data);

#endif
