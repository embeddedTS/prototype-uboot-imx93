// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Technologic Systems, Inc. (dba embeddedTS)
 *
 * This provides an abstraction to the wizard microcontroller
 * on embeddedTS platforms. This device uses 16-bit address/data
 */

#include <dm/uclass.h>
#include <i2c.h>

#include "wizard.h"

/*
 * wizard_get_i2c_chip - Locate the Wizard
 *
 * On early prototypes the wizard is on bus 0, on new designs
 * it is on bus 3. When the early prototypes are dropped this
 * can be simplified to just use bus 3, or locate the compatible node
 * on the device tree.
 *
 * This has to work using the provisional/default device tree; before
 * the correct device tree for this board model has been selected.
 */
static struct udevice *wizard_get_i2c_chip(void)
{
	static struct udevice *chip;
	static bool found;
	struct udevice *bus;
	u16 value;
	int busses[2] = {3, 0};
	int i;
	int ret = 1;

	if (found)
		return chip;

	for (i = 0; i < (sizeof(busses) / sizeof(int)); i++) {
		ret = uclass_get_device_by_seq(UCLASS_I2C, busses[i], &bus);
		if (ret)
			continue;

		ret = i2c_get_chip(bus, WIZARD_I2C_ADDR, 2, &chip);
		if (ret)
			continue;

		ret = i2c_set_chip_offset_len(chip, 2);
		if (!ret)
			break;

		ret = dm_i2c_read(chip, 0, (uint8_t *)&value, sizeof(value));
		if (!ret)
			break;
	}
	if (ret)
		return NULL;
	found = 1;
	return chip;
}

int wizard_write(u16 addr, u16 value)
{
	struct udevice *chip;

	chip = wizard_get_i2c_chip();
	if (!chip)
		return -ENODEV;

	return dm_i2c_write(chip, cpu_to_be16(addr), (uint8_t *)&value, 2);
}

int wizard_read(u16 addr, u16 *value)
{
	struct udevice *chip;
	int ret;

	chip = wizard_get_i2c_chip();
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
	u16 reg_addr = WIZARD_SERIAL;
	int n_words = 3;
	u16 word;
	int ret;

	while (n_words--) {
		ret = wizard_read(reg_addr, &word);
		if (ret) {
			printf("i2c read failed at addr %04x, rc=%d (-ve)\n",
			       reg_addr, ret);
			break;
		}
		mac_buffer[2 * (2 - n_words)] = word & 0xff;
		mac_buffer[2 * (2 - n_words) + 1] = (word >> 8) & 0xff;
		reg_addr += 1;
	}

	return ret;
}

u16 get_board_model_register(void)
{
	static u16 board_model_register;
	static bool found;
	int ret;

	if (!found) {
		ret = wizard_read(0, &board_model_register);
		if (!ret)
			found = 1;
	}

	return board_model_register;
}

const char *get_board_model(void)
{
	static char str_buffer[5];
	static bool loaded;
	u16 condensed_register_form;

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
	static bool loaded;

	if (!loaded) {
		snprintf(name_str, sizeof(name_str), "TS-%s", get_board_model());
		loaded = 1;
	}
	return name_str;
}
