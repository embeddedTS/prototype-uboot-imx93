
#include <common.h>
#include <asm/io.h>
#include "tsfpga.h"

bool fpga_is_bootloader(void)
{
	uint32_t model = readl((void *)FPGA_MODEL);

	return (model == 0xc0de);
}

void print_fpga_version(void)
{
	uint32_t model = readl((void *)FPGA_MODEL);
	uint32_t tag_version = readl((void *)FPGA_TAG_VERSION);
	uint32_t git_hash = readl((void *)FPGA_HASH);
	uint8_t git_dirty = (git_hash >> 31) & 0x1;
	uint8_t major = (tag_version >> 24) & 0xFF;
	uint8_t minor = (tag_version >> 16) & 0xFF;
	uint8_t patch = (tag_version >> 8) & 0xFF;
	uint8_t extra = tag_version & 0xFF;

	if (fpga_is_bootloader())
		printf("FPGA Bootloader: v%d.%d.%d",
			   major,
			   minor,
			   patch);
	else
		printf("FPGA TS-%04X: v%d.%d.%d",
			   model,
			   major,
			   minor,
			   patch);

	if (extra > 0)
		printf("-%d", extra);
	if (git_dirty)
		printf("-dirty");
	printf("\n");
}
