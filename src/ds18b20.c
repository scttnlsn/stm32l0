#include "ds18b20.h"
#include "onewire.h"
#include "tick.h"

// assumes there's only a single device on the onewire bus

float ds18b20_read_temp(void) {
  uint8_t ok = onewire_reset();
  if (!ok) {
    return 0xFFFF;
  }

  // skip ROM
  onewire_write(0xCC);

  // read temp and put it on the scratchpad
  onewire_write(0x44);

  // wait for conversion to complete
  /* uint8_t bit; */
  /* do { */
  /*   bit = onewire_read_bit(); */
  /* } while (!bit); */
  tick_delay(800);

  onewire_reset();

  // skip ROM
  onewire_write(0xCC);

  // read scratchpad
  onewire_write(0xBE);

  uint16_t temp_lsb = onewire_read();
  uint16_t temp_msb = onewire_read();

  // stop reading the rest of the scratchpad
  // TODO: read entire scratchpad and compare CRC
  onewire_reset();

  float integral = ((temp_msb << 8) | temp_lsb) >> 4;
  float decimal = temp_lsb & 0xF;

  uint8_t sign = temp_msb & 0b10000000;
  if (sign) {
    integral *= -1;
  }

  float temp_c = integral + (decimal * 0.0625);
  float temp_f = temp_c * 9 / 5 + 32;

  return temp_f;
}
