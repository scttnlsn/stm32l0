#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>

void spi_init(void);
void spi_enable_(void);
void spi_disable_(void);
uint8_t spi_transfer(uint8_t value);
void spi_wait(void);

#endif
