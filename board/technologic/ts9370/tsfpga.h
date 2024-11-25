#pragma once

#define FPGA_BASE		0x28000000
#define FPGA_MODEL		(FPGA_BASE + 0x0)
#define FPGA_TAG_VERSION	(FPGA_BASE + 0x4)
#define FPGA_HASH		(FPGA_BASE + 0x8)
#define FPGA_SCRATCH0		(FPGA_BASE + 0x10)
#define FPGA_SCRATCH1		(FPGA_BASE + 0x14)

bool fpga_is_bootloader(void);
void print_fpga_version(void);
