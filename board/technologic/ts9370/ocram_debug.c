// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Technologic Systems, Inc. dba embeddedTS
 */

#ifndef CONFIG_SPL_BUILD
#include <stdio.h>
#endif
int include_stdint;
#include "linux/types.h"
int include_ocram_debug;
#include "ocram_debug.h"

// Borrow space from ROM log at 0x2048782c
ocram_debug_t *ocram_debug_area_p = (ocram_debug_t *) 0x20487900;

void ocram_debug_init(void)
{
	if (ocram_debug_area_p->breadcrumb_save_area_begin == 0xfeedbeef)
		return;
	ocram_debug_area_p->breadcrumb_save_area_begin = 0xfeedbeef;
	ocram_debug_area_p->breadcrumb_save_area_end = 0xdeadbeef;
}
