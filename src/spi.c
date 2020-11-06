#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include "spi.h"

#define SPI_MOSI GPIO7
#define SPI_MISO GPIO6
#define SPI_SCK GPIO5

void spi_init() {
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_SPI1);

  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI_MOSI | SPI_MISO | SPI_SCK);
  gpio_set_af(GPIOA, GPIO_AF0, SPI_MOSI | SPI_MISO | SPI_SCK);

  spi_init_master(SPI1,
                  SPI_CR1_BAUDRATE_FPCLK_DIV_256,
                  SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                  SPI_CR1_CPHA_CLK_TRANSITION_1,
                  SPI_CR1_DFF_8BIT,
                  SPI_CR1_MSBFIRST);

  spi_enable_ss_output(SPI1);
  spi_enable(SPI1);
}

void spi_enable_() {
  spi_enable(SPI1);
}

void spi_disable_() {
  spi_wait();
  spi_disable(SPI1);
}

uint8_t spi_transfer(uint8_t value) {
    return spi_xfer(SPI1, value);
}

void spi_wait(void) {
  while (SPI_SR(SPI1) & SPI_SR_BSY);
}
