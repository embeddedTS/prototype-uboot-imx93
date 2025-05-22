#ifndef __PARSE_STRAPS_H__
#define __PARSE_STRAPS_H__

const char *get_board_model(void);
const char *get_board_name(void);
const char get_board_version_char(void);
const char *get_board_version_str(void);
uint32_t get_straps(void);

u32 get_board_rev(void);

uint8_t read_board_opts(void);

uint16_t read_raw_cpu_straps(void);
#endif
