#ifndef __PARSE_STRAPS_H__
#define __PARSE_STRAPS_H__

#define DEBUG_STRAPS

#undef DRIVE_STRAPS_LOW_BEFORE_READING
#undef DEBUG_STRAPS
#define DEBUG_PRINT_STRAPS
#undef SPL_DEBUG_STRAP_SPINLOOP
#undef P1_BUILD
#ifndef CONFIG_SPL_BUILD
#  undef DRIVE_STRAPS_LOW_BEFORE_READING
#endif

const char *get_board_model(void);
const char *get_board_name(void);
const char get_board_version_char(void);
const char *get_board_version_str(void);
uint32_t get_straps(void);
const char *get_straps_str(void);

u32 get_board_rev(void);

uint8_t read_board_opts(void);

uint16_t read_raw_cpu_straps(void);
uint16_t read_raw_fpga_straps(void);
#endif
