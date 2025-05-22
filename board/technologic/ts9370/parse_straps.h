/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __PARSE_STRAPS_H__
#define __PARSE_STRAPS_H__

const char *get_board_model(void);
const char *get_board_name(void);
const char get_board_version_char(void);
const char *get_board_version_str(void);
u32 get_straps(void);
u32 get_board_rev(void);
u8 read_board_opts(void);
u16 read_raw_cpu_straps(void);

#endif
