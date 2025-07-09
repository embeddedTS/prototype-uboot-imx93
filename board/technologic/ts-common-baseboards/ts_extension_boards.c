
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <common.h>
#include <extension_board.h>
#include <env.h>

/*
 * construct_baseboard_overlay_str
 *
 * This function returns `true` if the environment has the variables
 * needed to construct the name of the dtbo, `false` if it was not.
 *
 * Call do_bbdetect() before calling this function. Doing so ensures
 * the needed environment variables are populated.
 */
bool construct_baseboard_overlay_str(char *string_buffer, int buffer_len, char *prefix)
{
	uint8_t baseboard_id;
	char *board;
	char *soc_type;

	baseboard_id = env_get_hex("baseboard_id", 0);
	if (!baseboard_id)
		return false;
	board = from_env("board");
	if (!board)
		return false;
	soc_type = from_env("soc_type");
	if (!soc_type)
		return false;

	if (!prefix)
		prefix = "";
	snprintf(string_buffer, buffer_len,
		 "%s%s-%s-%02x.dtbo", prefix, soc_type, board, baseboard_id);
	return true;
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
	struct extension *extension = NULL;
	uint8_t baseboard_rev;
	uint8_t baseboard_id;
	bool valid = false;

	baseboard_id = env_get_hex("baseboard_id", 0);
	if (!baseboard_id)
		return 0;
	baseboard_rev = env_get_hex("baseboard_rev", 0);

	switch (baseboard_id) {
	case 0x16:
		valid = true;
		break;
	default:
		if (baseboard_id <= 63)
			valid = true;
		break;
	}

	if (!valid) {
		printf("Unknown extension board ID: %02x\n", baseboard_id);
		return 0;
	}

	extension = calloc(1, sizeof(struct extension));
	if (!extension)
		return 0;

	snprintf(extension->owner, sizeof(extension->owner),
		 "embeddedTS");
	construct_baseboard_overlay_str(extension->overlay,
					sizeof(extension->overlay), (char *)0);
	snprintf(extension->version, sizeof(extension->version),
		 "%d", baseboard_rev);
	switch (baseboard_id) {
	case 0x16:
		snprintf(extension->name, sizeof(extension->name),
			 "TS-8551");
		break;
	default:
		snprintf(extension->name, sizeof(extension->name),
			 "Custom Baseboard ID=0x%02x", baseboard_id);
		break;
	}

	list_add_tail(&extension->list, extension_list);
	return 1;
}
