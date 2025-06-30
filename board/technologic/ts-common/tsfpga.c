// SPDX-License-Identifier: GPL-2.0+

#include <stdint.h>
#include <stdbool.h>

#include <asm/io.h>
#include "tsfpga.h"

bool fpga_is_bootloader(void)
{
	u32 model = readl((void *)FPGA_MODEL);

	return (model == 0xc0de);
}

void fpga_gpio_set_as_input(uint16_t gpio)
{
	int bank = FPGA_BANK_FROM_GPIO(gpio);
	uint32_t bit_mask = (1 << FPGA_LINE_FROM_GPIO(gpio));
	assert(bank <= 3);

	writel(bit_mask, FPGA_GPIO_BANK_OE_CLEAR_ADDR(bank));
}

void fpga_gpio_set_as_output(uint16_t gpio)
{
	int bank = FPGA_BANK_FROM_GPIO(gpio);
	uint32_t bit_mask = (1 << FPGA_LINE_FROM_GPIO(gpio));
	assert(bank <= 3);

	writel(bit_mask, FPGA_GPIO_BANK_OE_SET_ADDR(bank));
}

void fpga_gpio_output(uint16_t gpio, bool value)
{
	int bank = FPGA_BANK_FROM_GPIO(gpio);
	uint32_t bit_mask = (1 << FPGA_LINE_FROM_GPIO(gpio));
	assert(bank < N_FPGA_GPIO_BANKS);
	if (value)
		writel(bit_mask, FPGA_GPIO_BANK_DATA_SET_ADDR(bank));
	else
		writel(bit_mask, FPGA_GPIO_BANK_DATA_CLEAR_ADDR(bank));
}

bool fpga_gpio_input(uint16_t gpio)
{
	int bank = FPGA_BANK_FROM_GPIO(gpio);
	uint32_t bit_mask = (1 << FPGA_LINE_FROM_GPIO(gpio));
	assert(bank < N_FPGA_GPIO_BANKS);
	if (readl(FPGA_GPIO_BANK_DATA_IN_ADDR(bank)) & bit_mask)
		return true;
	return false;
}

void print_fpga_version(void)
{
	u32 model = readl((void *)FPGA_MODEL);
	u32 tag_version = readl((void *)FPGA_TAG_VERSION);
	u8 git_dirty = (tag_version >> 31) & 0x1;
	u8 major = (tag_version >> 24) & 0x7F;
	u8 minor = (tag_version >> 16) & 0xFF;
	u8 patch = (tag_version >> 8) & 0xFF;
	u8 extra = tag_version & 0xFF;

	if (fpga_is_bootloader())
		printf("FPGA Bootloader: v%d.%d.%d",
		       major,
		       minor,
		       patch);
	else
		printf("FPGA TS-%04X: v%d.%d.%d",
		       model,
		       major,
		       minor,
		       patch);

	if (extra > 0)
		printf("-%d", extra);
	if (git_dirty)
		printf("-dirty");
	printf("\n");
}
