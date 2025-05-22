/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __OCRAM_DEBUG_H__
#define __OCRAM_DEBUG_H__

#define OCRAM_DEBUG_AREA_ADDR 0x20487900

void ocram_debug_init(void);

struct ocram_debug_t {
	u32 breadcrumb_save_area_begin;
	u32 spl_saved_straps;
	u32 saved_straps;
	u32 breadcrumb_save_area_end;
};

extern struct ocram_debug_t *const global_ocram_debug_area_p;

#endif
