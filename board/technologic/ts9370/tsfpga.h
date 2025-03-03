#pragma once

#define FPGA_BASE		0x28000000
#define FPGA_MODEL		(FPGA_BASE + 0x0)
#define FPGA_TAG_VERSION	(FPGA_BASE + 0x4)
#define FPGA_HASH		(FPGA_BASE + 0x8)
#define FPGA_SCRATCH0		(FPGA_BASE + 0x10)
#define FPGA_SCRATCH1		(FPGA_BASE + 0x14)
#define FPGA_GPIO0		(FPGA_BASE + 0x40)
#define FPGA_GPIO1		(FPGA_BASE + 0x80)
#define FPGA_GPIO2		(FPGA_BASE + 0xC0)

#define FPGA_GPIO_BANK_OE_SET_ADDR(bank_number) 	(FPGA_BASE + 0x40 + ((bank_number)*0x40) + 0x00)
#define FPGA_GPIO_BANK_OE_CLEAR_ADDR(bank_number)	(FPGA_BASE + 0x40 + ((bank_number)*0x40) + 0x04)
#define FPGA_GPIO_BANK_DATA_OUT_ADDR(bank_number)	(FPGA_BASE + 0x40 + ((bank_number)*0x40) + 0x08)
#define FPGA_GPIO_BANK_DATA_IN_ADDR(bank_number)	(FPGA_BASE + 0x40 + ((bank_number)*0x40) + 0x0C)

bool fpga_is_bootloader(void);
void print_fpga_version(void);
