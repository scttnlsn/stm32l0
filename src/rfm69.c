#include <libopencm3/stm32/rcc.h>
#include <string.h>

#include "spi.h"
#include "tick.h"
#include "rfm69.h"

typedef enum {
  STANDBY = 1,
  TX,
  RX,
  SYNTH,
  SLEEP
} rfm69_mode_t;

typedef struct {
  uint8_t network_id;
  uint8_t node_addr;
  rfm69_mode_t mode;
} rfm69_state_t;

static rfm69_state_t current_state;

static void rfm69_set_mode(rfm69_mode_t mode);
static int16_t rfm69_read_rssi(void);
static bool rfm69_can_send(void);
static uint8_t rfm69_read_reg(uint8_t addr);
static void rfm69_write_reg(uint8_t addr, uint8_t val);
static void rfm69_spi_select(void);
static void rfm69_spi_unselect(void);

void rfm69_init(uint8_t network_id, uint8_t node_addr) {
  memset(&current_state, 0, sizeof(rfm69_state_t));
  current_state.network_id = network_id;
  current_state.node_addr = node_addr;

  gpio_mode_setup(RFM69_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, RFM69_NSS);
  gpio_set(RFM69_PORT, RFM69_NSS);

  // verify chip is responding
  do {
    rfm69_write_reg(REG_SYNCVALUE1, 0xAA);
  } while (rfm69_read_reg(REG_SYNCVALUE1) != 0xAA);
  do {
    rfm69_write_reg(REG_SYNCVALUE1, 0x55);
  } while (rfm69_read_reg(REG_SYNCVALUE1) != 0x55);

  // based on config from https://github.com/LowPowerLab/RFM69
  const uint8_t config[][2] =
    {
     { REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY },
     { REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_00 }, // no shaping
     { REG_BITRATEMSB, RF_BITRATEMSB_55555}, // default: 4.8 KBPS
     { REG_BITRATELSB, RF_BITRATELSB_55555},
     { REG_FDEVMSB, RF_FDEVMSB_50000}, // default: 5KHz, (FDEV + BitRate / 2 <= 500KHz)
     { REG_FDEVLSB, RF_FDEVLSB_50000},

     // using 915 mHz radio
     { REG_FRFMSB, RF_FRFMSB_915 },
     { REG_FRFMID, RF_FRFMID_915 },
     { REG_FRFLSB, RF_FRFLSB_915 },

     { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_16 | RF_RXBW_EXP_2 }, // (BitRate < 2 * RxBw)
     { REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01 }, // DIO0 is the only IRQ we're using
     { REG_DIOMAPPING2, RF_DIOMAPPING2_CLKOUT_OFF }, // DIO5 ClkOut disable for power saving
     { REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN }, // writing to this bit ensures that the FIFO & status flags are reset
     { REG_RSSITHRESH, 220 }, // must be set to dBm = (-Sensitivity / 2), default is 0xE4 = 228 so -114dBm
     { REG_SYNCCONFIG, RF_SYNC_ON | RF_SYNC_FIFOFILL_AUTO | RF_SYNC_SIZE_2 | RF_SYNC_TOL_0 },
     { REG_SYNCVALUE1, 0x2D }, // attempt to make this compatible with sync1 byte of RFM12B lib
     { REG_SYNCVALUE2, current_state.network_id },
     { REG_PACKETCONFIG1, RF_PACKET1_FORMAT_VARIABLE | RF_PACKET1_DCFREE_OFF | RF_PACKET1_CRC_ON | RF_PACKET1_CRCAUTOCLEAR_ON | RF_PACKET1_ADRSFILTERING_OFF },
     { REG_PAYLOADLENGTH, 66 }, // in variable length mode: the max frame size, not used in TX
     { REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RF_FIFOTHRESH_VALUE }, // TX on FIFO not empty
     { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_2BITS | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
     { REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0 }, // run DAGC continuously in RX mode for Fading Margin Improvement, recommended default for AfcLowBetaOn=0
     {255, 0}
    };

  for (uint8_t i = 0; config[i][0] != 255; i++) {
    rfm69_write_reg(config[i][0], config[i][1]);
  }

  // set high power (for RFM69HW)
  rfm69_write_reg(REG_OCP, RF_OCP_OFF);
  rfm69_write_reg(REG_PALEVEL, (rfm69_read_reg(REG_PALEVEL) & 0x1F) | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON);

  rfm69_set_mode(STANDBY);

  // wait for ModeReady
  while ((rfm69_read_reg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00);
}

void rfm69_set_key(const char *key) {
  rfm69_set_mode(STANDBY);

  if (key != 0) {
    rfm69_spi_select();
    spi_transfer(REG_AESKEY1 | 0b10000000);
    for (uint8_t i = 0; i < 16; i++) {
      spi_transfer(key[i]);
    }
    rfm69_spi_unselect();
  }

  rfm69_write_reg(REG_PACKETCONFIG2, (rfm69_read_reg(REG_PACKETCONFIG2) & 0xFE) | (key ? 1 : 0));
}

void rfm69_sleep(void) {
  rfm69_set_mode(SLEEP);
}

void rfm69_wakeup(void) {
  rfm69_set_mode(STANDBY);
}

void rfm69_send(uint8_t to_addr, const void *buffer, uint8_t len) {
  uint64_t now = tick_get();
  while (!rfm69_can_send() && tick_get() - now < RF69_CSMA_LIMIT_MS);

  rfm69_set_mode(STANDBY);
  while ((rfm69_read_reg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00);

  if (len > RF69_MAX_DATA_LEN) {
    len = RF69_MAX_DATA_LEN;
  }

  // control byte
  uint8_t ctl_byte = 0x00;

  // write to FIFO
  rfm69_spi_select();
  spi_transfer(REG_FIFO | 0b10000000);
  spi_transfer(len + 3);
  spi_transfer(to_addr);
  spi_transfer(current_state.node_addr);
  spi_transfer(ctl_byte);

  for (uint8_t i = 0; i < len; i++) {
    spi_transfer(((uint8_t *) buffer)[i]);
  }
  rfm69_spi_unselect();

  rfm69_set_mode(TX);
  uint64_t tx_start = tick_get();

  while ((rfm69_read_reg(REG_IRQFLAGS2) & RF_IRQFLAGS2_PACKETSENT) == 0x00);
  rfm69_set_mode(STANDBY);
}

// private

static void rfm69_set_mode(rfm69_mode_t mode) {
  if (current_state.mode == mode) {
    return;
  }

  switch (mode) {
  case TX:
    rfm69_write_reg(REG_OPMODE, (rfm69_read_reg(REG_OPMODE) & 0xE3) | RF_OPMODE_TRANSMITTER);
    break;
  case RX:
    rfm69_write_reg(REG_OPMODE, (rfm69_read_reg(REG_OPMODE) & 0xE3) | RF_OPMODE_RECEIVER);
    break;
  case SYNTH:
    rfm69_write_reg(REG_OPMODE, (rfm69_read_reg(REG_OPMODE) & 0xE3) | RF_OPMODE_SYNTHESIZER);
    break;
  case STANDBY:
    rfm69_write_reg(REG_OPMODE, (rfm69_read_reg(REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);
    break;
  case SLEEP:
    rfm69_write_reg(REG_OPMODE, (rfm69_read_reg(REG_OPMODE) & 0xE3) | RF_OPMODE_SLEEP);
    break;
  }

  // Wait for proper wakeup from sleep
  while (current_state.mode == SLEEP && (rfm69_read_reg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00);

  current_state.mode = mode;
}

static int16_t rfm69_read_rssi(void) {
  int16_t rssi = 0;
  rssi = -rfm69_read_reg(REG_RSSIVALUE);
  rssi >>= 1;
  return rssi;
}

static bool rfm69_can_send(void) {
  if (current_state.mode == RX && rfm69_read_rssi() < CSMA_LIMIT) {
    rfm69_set_mode(STANDBY);
    return true;
  }

  return false;
}

static void rfm69_write_reg(uint8_t addr, uint8_t val) {
  rfm69_spi_select();
  // msb = 1 for writes
  spi_transfer(addr | 0b10000000);
  spi_transfer(val);
  rfm69_spi_unselect();
}

static uint8_t rfm69_read_reg(uint8_t addr) {
  rfm69_spi_select();
  // msb = 0 for reads
  spi_transfer(addr & 0b01111111);
  // sent value is irrelevant
  uint8_t val = spi_transfer(0);
  rfm69_spi_unselect();
  return val;
}

static void rfm69_spi_select() {
  gpio_clear(RFM69_PORT, RFM69_NSS);
}

static void rfm69_spi_unselect() {
  spi_wait();
  gpio_set(RFM69_PORT, RFM69_NSS);
}
