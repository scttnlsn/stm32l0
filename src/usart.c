#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include "usart.h"

// TX = PA2, RX = PA3
#define TX_PIN GPIO2
#define RX_PIN GPIO3

void usart_init(void) {
  rcc_periph_clock_enable(RCC_GPIOA);

  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, TX_PIN | RX_PIN);
  gpio_set_af(GPIOA, GPIO_AF4, TX_PIN | RX_PIN);

  rcc_periph_clock_enable(RCC_USART2);

  usart_set_baudrate(USART2, 115200);
  usart_set_databits(USART2, 8);
  usart_set_parity(USART2, USART_PARITY_NONE);
  usart_set_stopbits(USART2, USART_STOPBITS_1);
  usart_set_mode(USART2, USART_MODE_TX_RX);
  usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
  usart_enable(USART2);
}

void usart_write(uint16_t value) {
  usart_send_blocking(USART2, value);
}
