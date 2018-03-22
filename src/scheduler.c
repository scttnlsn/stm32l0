#include "scheduler.h"
#include "tick.h"

#define MAX_TASKS 10

typedef enum {
  TASK_NONE,
  TASK_PERIODIC,
  TASK_ONCE
} task_type_t;

typedef struct {
  task_type_t type;
  uint32_t time;
  uint32_t last;
  void (*callback)(void);
} task_t;

static task_t tasks[MAX_TASKS];

void noop() {
}

void scheduler_init(void) {
  for (int i = 0; i < MAX_TASKS; i++) {
    tasks[i].type = TASK_NONE;
    tasks[i].time = 0;
    tasks[i].callback = &noop;
    tasks[i].last = 0;
  }
}

void scheduler_every(uint32_t period, void (*callback)(void)) {
  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].type == TASK_NONE) {
      tasks[i].type = TASK_PERIODIC;
      tasks[i].time = period;
      tasks[i].callback = callback;
      tasks[i].last = 0;
      break;
    }
  }
}

void scheduler_once(uint32_t delay, void (*callback)(void)) {
  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].type == TASK_NONE) {
      tasks[i].type = TASK_ONCE;
      tasks[i].time = delay;
      tasks[i].callback = callback;
      tasks[i].last = tick_get();
      break;
    }
  }
}

void scheduler_run(void) {
  for (int i = 0; i < MAX_TASKS; i++) {
    switch (tasks[i].type) {
    case TASK_PERIODIC:
      if (tick_since(tasks[i].last) > tasks[i].time) {
        tasks[i].callback();
        tasks[i].last = tick_get();
      }
      break;
    case TASK_ONCE:
      if (tick_since(tasks[i].last) > tasks[i].time) {
        tasks[i].callback();

        // clear task
        tasks[i].type = TASK_NONE;
        tasks[i].time = 0;
        tasks[i].callback = &noop;
        tasks[i].last = 0;
      }
      break;
    case TASK_NONE:
      break;
    }
  }
}
