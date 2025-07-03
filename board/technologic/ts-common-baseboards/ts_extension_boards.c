
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <common.h>
#include <extension_board.h>
#include <env.h>

int extension_board_scan(struct list_head *extension_list)
{
	struct extension *extension = NULL;
	uint8_t baseboard_id;
	char *board;
	char *soc_type;
	uint8_t baseboard_rev;
	bool valid = false;

	baseboard_id = env_get_hex("baseboard_id", 0);
	if (!baseboard_id)
		return 0;
	baseboard_rev = env_get_hex("baseboard_rev", 0);
	board = from_env("board");
	if (!board)
		return 0;
	soc_type = from_env("soc_type");
	if (!soc_type)
		return 0;

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
	snprintf(extension->overlay, sizeof(extension->overlay),
		 "%s-%s-%02x.dtbo", soc_type, board, baseboard_id);
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
