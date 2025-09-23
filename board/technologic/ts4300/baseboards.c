#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <common.h>
#include <extension_board.h>
#include <dm/uclass.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <i2c.h>
#include <env.h>

#include "../ts-common/tsfpga.h"

/*
 * construct_baseboard_overlay_str
 *
 * This function allocates and initializes a struct extension.
 * It returns a pointer to the allocated struct if successful,
 * or NULL if the environment does not have the required variables.
 *
 * The caller is responsible for freeing the returned struct.
 */
struct extension *create_extension(char *suffix)
{
	uint8_t baseboard_id;
	struct extension *ext;

	baseboard_id = env_get_hex("baseboard_id", 0);
	if (!baseboard_id)
		return NULL;

	ext = calloc(1, sizeof(struct extension));
	if (!ext)
		return NULL;

	snprintf(ext->owner, sizeof(ext->owner), "embeddedTS");

	if (!suffix)
		snprintf(ext->overlay, sizeof(ext->overlay),
			 "imx93-ts4300-%02x.dtbo", baseboard_id);
	else
		snprintf(ext->overlay, sizeof(ext->overlay),
			 "imx93-ts4300-%02x-%s.dtbo", baseboard_id, suffix);

	return ext;
}

		/* Set gpio 6 17=0 */
		/* Wait 10ms */
		/* i2c dev 0 */
		/* i2c probe */
		/* Detect mipi2dpi card at 0x0f */

int ts8551_mipi2dp_present(void)
{
	static struct udevice *chip;
	struct udevice *bus;
	uint8_t value;
	int ret;

	/* Set CN1_096 high, DP_RESET */
	writel(1 << 17, FPGA_GPIO_BANK_DATA_SET_ADDR(2));
	writel(1 << 17, FPGA_GPIO_BANK_OE_SET_ADDR(2));
	udelay(2); /* MIPI DC is 2us tRSTON */
	writel(1 << 17, FPGA_GPIO_BANK_DATA_SET_ADDR(2));
	mdelay(1); /* MIPI DC is 1ms tCORERDY */

	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &bus);
	if (ret) {
		printf("%s: Failed to get i2c device\n", __FUNCTION__);
		return 0;
	}

	ret = i2c_get_chip(bus, 0x0f, 2, &chip);
	if (ret) {
		printf("%s: Failed to get i2c chip\n", __FUNCTION__);
		return 0;
	}

	ret = dm_i2c_read(chip, 0, &value, sizeof(value));
	if (!ret){
		printf("%s: Failed to get read from dp chip\n", __FUNCTION__);
		return 0;
	}

	return 1;
}

/*
 * extension_board_scan
 *
 * This function returns "the number of extension boards found", i.e.,
 * 1 if a recognized/valid extension board was added to the list of
 * extensions, 0 if not.
 *
 * Call do_bbdetect() before calling this function. Doing so ensures
 * the needed environment variables are populated.
 */
int extension_board_scan(struct list_head *extension_list)
{
	struct extension *baseboard;
	uint8_t baseboard_rev;
	uint8_t baseboard_id;
	int extensions = 1; /* will always be at least 1 baseboard extension */

	baseboard_id = env_get_hex("baseboard_id", 0);
	if (!baseboard_id)
		return 0;
	baseboard_rev = env_get_hex("baseboard_rev", 0);

	/* Add baseboard extension */
	baseboard = create_extension(0);
	if (!baseboard)
		return 0;
	list_add_tail(&baseboard->list, extension_list);

	snprintf(baseboard->version, sizeof(baseboard->version),
		 "%d", baseboard_rev);

	switch (baseboard_id) {
	case 0x16:
		snprintf(baseboard->name, sizeof(baseboard->name), "TS-8551");

		/* Check for TS-RD-MIPI2DP card by toggling CN1_096 to take it out of
		 * reset, and looking for 0x0f to ack.
		 */
		if (ts8551_mipi2dp_present()) {
			struct extension *dc = create_extension("mipi2dpi");

			snprintf(dc->name, sizeof(dc->name), "TS-RD-MIPI2DP");
			list_add_tail(&dc->list, extension_list);
			extensions++;
		}

		break;
	default:
		snprintf(baseboard->name, sizeof(baseboard->name),
			 "Custom Baseboard ID=0x%02x", baseboard_id);
		break;
	}

	return extensions;
}