// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Technologic Systems, Inc. (dba embeddedTS)
 *
 * For legacy reasons, the Wizard treats addresses and data as
 * little-endian, despite normal I2C conventions. The read and write
 * functions handle the byte swapping, so the caller does not need to
 * be aware.
 */

#include <common.h>
#include <dm/uclass.h>
#include <i2c.h>

#include "wizard.h"

/*
 * super_get_i2c_chip - Locate the Wizard, wherever it may be.
 *
 * This has to work using the provisional/default device tree; before
 * the correct device tree for this board model has been selected.
 *
 * Will break if something is at the Wizard's address on a bus it
 * checks before the bus the Wizard resides on.
 */
static struct udevice *super_get_i2c_chip(void)
{
	static struct udevice *chip;
	static bool found = 0;
	struct udevice *bus;
	uint16_t le_value;
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
                ret = dm_i2c_read(chip, wizard_byte_order(0),
                                  (uint8_t *)&le_value, sizeof(le_value));
                if (!ret) {
			break;
                }
	}
	if (ret)
		return NULL;
	found = 1;
	return chip;
}

uint16_t wizard_byte_order(uint16_t addr_value)
{
	uint16_t le_value;
	le_value = addr_value;
	return le_value;
}

int super_write(uint16_t addr, uint16_t value)
{
	struct udevice *chip;
	uint16_t swap_value;

	chip = super_get_i2c_chip();
	if (!chip)
		return -ENODEV;

        swap_value = wizard_byte_order(value);
	return dm_i2c_write(chip, wizard_byte_order(addr),
                            (uint8_t *)&swap_value, 2);
}

int super_read(uint16_t addr, uint16_t *value)
{
	struct udevice *chip;
	int ret;
	uint16_t le_value;

	chip = super_get_i2c_chip();
	if (!chip) {
		return -ENODEV;
	}

	ret = dm_i2c_read(chip, wizard_byte_order(addr),
			  (uint8_t *)&le_value, sizeof(le_value));
	if (ret) {
		return ret;
	}
	*value = wizard_byte_order(le_value);
	return 0;
}

int wizard_read_mac(uint8_t *mac_buffer)
{
	int ret;
	uint16_t reg_addr;
	uint16_t word;
	int n_words = 3;
	reg_addr = 0x22;

	while (n_words--) {
		ret = super_read(reg_addr, &word);
		if (ret) {
			printf("i2c read failed at addr %04x, rc=%d (-ve)\n",
			       wizard_byte_order(reg_addr), ret);
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

static struct udevice *super_get_i2c_chip_early(void)
{
	struct udevice *chip;
	struct udevice *bus;
	uint16_t le_value;
        int busses[2] = {3, 0};
	int i;
	int ret = 1;

        for (i = 0; i < (sizeof(busses)/sizeof(int)); i++) {
		ret = uclass_get_device_by_seq(UCLASS_I2C, busses[i], &bus);
		if (ret) {
			continue;
		}
		ret = i2c_get_chip(bus, SUPER_I2C_ADDR, 2, &chip);
		if (ret) {
			continue;
		}
                ret = dm_i2c_read(chip, wizard_byte_order(0),
                                  (uint8_t *)&le_value, sizeof(le_value));
                if (!ret) {
			break;
                }
	}
	if (ret) {
		return NULL;
	}
	return chip;
}

int super_read_early(uint16_t addr, uint16_t *value)
{
	struct udevice *chip;
	int ret;
	uint16_t le_value;

	chip = super_get_i2c_chip_early();
	if (!chip) {
		return -ENODEV;
	}

	ret = dm_i2c_read(chip, wizard_byte_order(addr),
			  (uint8_t *)&le_value, sizeof(le_value));
	if (ret) {
		return ret;
	}
	*value = wizard_byte_order(le_value);
	return 0;
}

uint16_t get_board_model_register_early(void)
{
	uint16_t board_model_register = 0;
	super_read_early(0, &board_model_register);
	return board_model_register;
}
