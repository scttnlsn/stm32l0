/* Linker script for STM32L0xx4, 16K flash, 2K RAM. */

/* Define memory regions. */
MEMORY
{
	rom (rx) : ORIGIN = 0x08000000, LENGTH = 16K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 2K
	eep (r) : ORIGIN = 0x08080000, LENGTH = 512
}

/* Include the common ld script. */
INCLUDE ./libopencm3/lib/stm32/l0/libopencm3_stm32l0.ld
