
#include "parse_straps.h"

extern void ocram_debug_init(void);

typedef struct {
  uint32_t breadcrumb_save_area_begin;
  uint32_t spl_saved_straps;
  uint32_t saved_straps;
#ifdef DEBUG_STRAPS
  uint32_t spl_pdir; // 0x50 0x003fffff  (bits 0-21 set, 22-31 clear)
  uint32_t spl_pddr; // 0x54
  uint32_t spl_pidr; // 0x58
  uint32_t pdir; // 0x50 0x003fffff  (bits 0-21 set, 22-31 clear)
  uint32_t pddr; // 0x54
  uint32_t pidr; // 0x58
#endif
  uint32_t breadcrumb_save_area_end;
} ocram_debug_t;

extern ocram_debug_t *ocram_debug_area_p;

#define OCRAM_DEBUG_AVAILABLE
