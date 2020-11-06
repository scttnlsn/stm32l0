#ifndef __RFM69_H__
#define __RFM69_H__

#include <libopencm3/stm32/gpio.h>
#include <stdint.h>
#include <stdbool.h>

#include "rfm69_defs.h"

#define RFM69_RCC RCC_GPIOA
#define RFM69_PORT GPIOA
#define RFM69_NSS GPIO4

void rfm69_init(uint8_t network_id, uint8_t node_addr);
void rfm69_set_key(const char *key);
void rfm69_sleep(void);
void rfm69_wakeup(void);
void rfm69_send(uint8_t to_addr, const void *buffer, uint8_t len);

#endif
