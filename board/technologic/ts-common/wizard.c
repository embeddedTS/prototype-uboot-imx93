// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Technologic Systems, Inc. (dba embeddedTS)
 *
 * This provides an abstraction to the wizard microcontroller
 * on embeddedTS platforms. This device uses 16-bit address/data
 */

#include <common.h>
#include <dm/uclass.h>
#include <i2c.h>

#include "wizard.h"

/*
 * super_get_i2c_chip - Locate the Wizard
 *
 * On early prototypes the wizard is on bus 0, on new designs
 * it is on bus 3. When the early prototypes are dropped this
 * can be simplified to just use bus 3, or locate the compatible node
 * on the device tree.
 *
 * This has to work using the provisional/default device tree; before
 * the correct device tree for this board model has been selected.
 */
static struct udevice *super_get_i2c_chip(void)
{
	static struct udevice *chip;
	static bool found = 0;
	struct udevice *bus;
	uint16_t value;
	int busses[2] = {3, 0};
	int i;
	int ret = 1;

	if (found) {
		return chip;
	}

	for (i = 0; i < (sizeof(busses)/sizeof(int)); i++) {
		ret = uclass_get_device_by_seq(UCLASS_I2C, busses[i], &bus);
		if (ret)
			continue;
		ret = i2c_get_chip(bus, SUPER_I2C_ADDR, 2, &chip);
		if (ret)
			continue;

		ret = i2c_set_chip_offset_len(chip, 2);
		if (!ret) {
			break;
		}
		ret = dm_i2c_read(chip, 0, (uint8_t *)&value, sizeof(value));
		if (!ret) {
			break;
		}
	}
	if (ret)
		return NULL;
	found = 1;
	return chip;
}

int super_write(uint16_t addr, uint16_t value)
{
	struct udevice *chip;

	chip = super_get_i2c_chip();
	if (!chip)
		return -ENODEV;

	return dm_i2c_write(chip, cpu_to_be16(addr), (uint8_t *)&value, 2);
}

int super_read(uint16_t addr, uint16_t *value)
{
	struct udevice *chip;
	int ret;

	chip = super_get_i2c_chip();
	if (!chip) {
		printf("Error: No I2C chip found\n");
		return -ENODEV;
	}

	ret = dm_i2c_read(chip, cpu_to_be16(addr), (uint8_t *)value, 2);
	if (ret) {
		printf("Error: dm_i2c_read failed, ret=%d\n", ret);
		return ret;
	}

	return 0;
}

int wizard_read_mac(uint8_t *mac_buffer)
{
	int ret;
	uint16_t reg_addr;
	uint16_t word;
	int n_words = 3;
	reg_addr = SUPER_SERIAL;

	while (n_words--) {
		ret = super_read(reg_addr, &word);
		if (ret) {
			printf("i2c read failed at addr %04x, rc=%d (-ve)\n",
			       reg_addr, ret);
			break;
		}
		mac_buffer[2*(2-n_words)] = (word >> 8) & 0xff;
		mac_buffer[2*(2-n_words)+1] = word & 0xff;
		reg_addr += 1;
	}

	return ret;
}

uint16_t get_board_model_register(void)
{
	static uint16_t board_model_register = 0;
	static bool found = 0;
	int ret;

	if (!found) {
		ret = super_read(0, &board_model_register);
		if (!ret) {
			found = 1;
		}
	}

	return board_model_register;
}

const char *get_board_model(void)
{
	static char str_buffer[5] = {0};
	static bool loaded = 0;
	uint16_t condensed_register_form;

	if (!loaded) {
		condensed_register_form = get_board_model_register();
		snprintf(str_buffer, sizeof(str_buffer), "%04X", condensed_register_form);
		loaded = 1;
	}
	return str_buffer;
}

const char *get_board_name(void)
{
	static char name_str[12] = {0};
	static bool loaded = 0;

	if (!loaded) {
		snprintf(name_str, sizeof(name_str), "TS-%s", get_board_model());
		loaded = 1;
	}
	return name_str;
}
