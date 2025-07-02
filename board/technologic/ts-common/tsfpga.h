/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __TS_FPGA_H__
#define __TS_FPGA_H__

#define FPGA_BASE		((uint8_t *)0x28000000)
#define FPGA_MODEL		(FPGA_BASE + 0x0)
#define FPGA_TAG_VERSION	(FPGA_BASE + 0x4)
#define FPGA_HASH		(FPGA_BASE + 0x8)
#define FPGA_SCRATCH0		(FPGA_BASE + 0x10)
#define FPGA_SCRATCH1		(FPGA_BASE + 0x14)
#define FPGA_GPIO0		(FPGA_BASE + 0x40)
#define FPGA_GPIO1		(FPGA_BASE + 0x80)
#define FPGA_GPIO2		(FPGA_BASE + 0xC0)

#define FPGA_GPIO_FROM_ID(_bank, _bit) ((uint16_t)(((_bank) << 8) | (_bit)))

#define EN_RED_LED_N    FPGA_GPIO_FROM_ID(4, 1)
#define EN_GREEN_LED_N  FPGA_GPIO_FROM_ID(4, 0)
#define DIO_52          FPGA_GPIO_FROM_ID(6, 19)
#define DIO_84          FPGA_GPIO_FROM_ID(7, 19)
#define N_FPGA_GPIO_BANKS 4

#define FPGA_BANK_FROM_GPIO(_gpio) (((_gpio) >> 8) - 4)
#define FPGA_LINE_FROM_GPIO(_gpio) ((_gpio) & 0x00FF)

#define FPGA_GPIO_BANK_OE_SET_ADDR(bank_number) \
	((volatile void *)(FPGA_BASE + 0x40 + ((bank_number) * 0x40) + 0x00))
#define FPGA_GPIO_BANK_OE_CLEAR_ADDR(bank_number) \
	((volatile void *)(FPGA_BASE + 0x40 + ((bank_number) * 0x40) + 0x04))
#define FPGA_GPIO_BANK_DATA_SET_ADDR(bank_number) \
	((volatile void*)(FPGA_BASE + 0x40 + ((bank_number) * 0x40) + 0x08))
#define FPGA_GPIO_BANK_DATA_CLEAR_ADDR(bank_number) \
	((volatile void *)(FPGA_BASE + 0x40 + ((bank_number) * 0x40) + 0x0C))
#define FPGA_GPIO_BANK_DATA_IN_ADDR(bank_number) \
	((volatile void *)(FPGA_BASE + 0x40 + ((bank_number) * 0x40) + 0x0C))

bool fpga_is_bootloader(void);
void print_fpga_version(void);
void fpga_gpio_set_as_input(uint16_t gpio);
void fpga_gpio_set_as_output(uint16_t gpio);
void fpga_gpio_output(uint16_t gpio, bool value);
bool fpga_gpio_input(uint16_t gpio);

#endif
