// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Technologic Systems, Inc. dba embeddedTS
 */

int include_stdint;
#include "linux/types.h"

int include_ocram_debug;
#include "ocram_debug.h"

// Borrow space from ROM log at 0x2048782c
struct ocram_debug_t *const global_ocram_debug_area_p =
	(struct ocram_debug_t * const)OCRAM_DEBUG_AREA_ADDR;

void ocram_debug_init(void)
{
	struct ocram_debug_t *const ocram_debug_area_p =
		(struct ocram_debug_t * const)OCRAM_DEBUG_AREA_ADDR;

	if (ocram_debug_area_p->breadcrumb_save_area_begin == 0xfeedbeef)
		return;

	ocram_debug_area_p->breadcrumb_save_area_begin = 0xfeedbeef;
	ocram_debug_area_p->breadcrumb_save_area_end = 0xdeadbeef;
}
