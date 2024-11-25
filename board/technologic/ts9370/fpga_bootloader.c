// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Technologic Systems, Inc. (dba embeddedTS)
 */
#include <common.h>
#include <console.h>
#include <command.h>
#include <asm/io.h>
#include <rand.h>
#include <linux/delay.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <image.h>
#include <mmc.h>

#include "tsfpga.h"
#include "fpga_bootloader.h"

#define FPGA_UPDATE_BLK (0x240000 / 512)
#define FPGA_UPDATE_BLK_CNT (0x47000 / 512)

static int erase_fit_from_flash(void)
{
	struct mmc *mmc;
	int res;

	mmc = find_mmc_device(0);
	if (!mmc) {
		printf("Could not find eMMC device\n");
		return 1;
	}

	/* Erase the specified region */
	res = blk_derase(mmc_get_blk_desc(mmc), FPGA_UPDATE_BLK, FPGA_UPDATE_BLK_CNT);
	if (res != FPGA_UPDATE_BLK_CNT) {
		printf("Failed to erase eMMC region\n");
		return 1;
	}
	return 0;
}

static int read_fit_from_flash(void **fit_image)
{
	u32 *buffer = (void *)(uintptr_t)CONFIG_SYS_LOAD_ADDR;
	struct mmc *mmc;
	int res;

	res = mmc_init_device(0);
	if (res) {
		printf("Failed to init eMMC\n");
		return 1;
	}

	mmc = find_mmc_device(0);
	if (!mmc) {
		printf("Could not find eMMC device\n");
		return 1;
	}

	res = mmc_init(mmc);
	if (res) {
		printf("Failed to initialize eMMC device\n");
		return 1;
	}

	res = blk_select_hwpart_devnum(UCLASS_MMC, 0, 1);
	if (res) {
		printf("Error: Couldn't select boot partition\n");
		return 1;
	}

	if (mmc_getwp(mmc) == 1) {
		printf("Error: Card is write-protected, no further updates\n");
		return 1;
	}

	/* Read the FIT image into memory */
	res = blk_dread(mmc_get_blk_desc(mmc), FPGA_UPDATE_BLK, FPGA_UPDATE_BLK_CNT, buffer);
	if (res != FPGA_UPDATE_BLK_CNT) {
		printf("Failed to read eMMC\n");
		return 1;
	}

	/* Validate FIT format */
	res = fit_check_format(buffer, IMAGE_SIZE_INVAL);
	if (res) {
		/* No pending update */
		return 1;
	}

	*fit_image = buffer;
	return 0;
}

int fpga_process_fit_updates(const void *fit)
{
	int res, confs_noffset, images_noffset;
	const char *config;
	size_t size;
	const void *image_data;

	confs_noffset = fdt_path_offset(fit, "/configurations");
	if (confs_noffset < 0) {
		printf("No configurations node found\n");
		return 1;
	}

	fdt_for_each_subnode(images_noffset, fit, confs_noffset) {
		config = fit_get_name(fit, images_noffset, NULL);
		if (!config) {
			printf("Unnamed configuration found, skipping\n");
			continue;
		}
		images_noffset = fit_conf_get_prop_node(fit, images_noffset, "firmware", 0);
		if (images_noffset < 0) {
			printf("Failed to find image node for configuration: %s\n", config);
			continue;
		}
		res = fit_image_get_data(fit, images_noffset, &image_data, &size);
		if (res) {
			printf("Failed to get image data for configuration: %s\n", config);
			continue;
		}
		printf("%s update, %zu bytes\n", config, size);

		if (strcmp(config, "fpga_application") == 0) {
			res = flash_update_app((uintptr_t)image_data, (u32)size);
		} else if (strcmp(config, "fpga_bootloader") == 0) {
			res = flash_update_bootloader((uintptr_t)image_data, (u32)size);
		} else {
			printf("Unknown update type: %s\n", config);
			continue;
		}

		if (res) {
			printf("%s update failed\n", config);
			return 1;
		}

		printf("%s updated successfully\n", config);
	}

	return 0;
}

int fpga_update_from_flash(void)
{
	void *fit;
	int res;

	res = read_fit_from_flash(&fit);
	if (res) {
		/* No pending update */
		return 1;
	}


	res = fpga_process_fit_updates(fit);
	if (res) {
		printf("Failed to process FIT image\n");
		return 1;
	}

	res = erase_fit_from_flash();
	if (res) {
		printf("Failed to erase FIT image region\n");
		return 1;
	}	

	return 0;
}

uint32_t swap_bitstream_order(uint32_t x)
{
	/* Reverse all bits to match order on raw flash */
	x = (x >> 16) | (x << 16);
	x = ((x & 0xFF00FF00) >> 8) | ((x & 0x00FF00FF) << 8);
	x = ((x & 0xF0F0F0F0) >> 4) | ((x & 0x0F0F0F0F) << 4);
	x = ((x & 0xCCCCCCCC) >> 2) | ((x & 0x33333333) << 2);
	x = ((x & 0xAAAAAAAA) >> 1) | ((x & 0x55555555) << 1);
	return x;
}

int flash_wait_until_idle(uint32_t timeout_ms, uint32_t *reg)
{
	int timeout = 1;

	for (int i = 0; i < timeout_ms; i++) {
		*reg = readl(UPDATER_STATUS);
		if ((*reg & UPDATER_STATUS_BUSY_MASK) == 0) {
			timeout = 0;
			break;
		}
		udelay(1000);
	}

	if (timeout) {
		printf("Flash Operation timed out (%dms)\n", timeout_ms);
		return 1;
	}

	return 0;
}

int flash_write(uint32_t flash_addr, uint32_t data_addr, uint32_t len)
{
	uint32_t data;
	uint32_t reg;
	int ret;

	if (!fpga_is_bootloader()) {
		printf("FPGA is already booted\n");
		return 1;
	}

	printf("Writing Flash Addr 0x%08X from memory 0x%08X, len %d bytes\n", flash_addr, data_addr, len);

	for (uint32_t i = 0; i < len; i += 4) {
		data = *(volatile uint32_t *)(uintptr_t)(data_addr + i);
		data = swap_bitstream_order(data);
		writel(WORD_ADDRESS(flash_addr + i), UPDATER_ADDR);
		writel(data, UPDATER_FLASHDATA);
		/* UFM Programming max time is 305 us */
		ret = flash_wait_until_idle(1000, &reg);
		if (ret || ((reg & UPDATER_STATUS_WRITE_SUCCESS) == 0)) {
			printf("Flash Write failed at offset %d\n", i);
			return 1;
		}

		if (ctrlc()) {
			printf("Killed\n");
			return 1;
		}
	}

	return 0;
}

int flash_read(uint32_t flash_addr, uint32_t data_addr, uint32_t len)
{
	uint32_t data;
	uint32_t reg;
	int ret;

	if (!fpga_is_bootloader()) {
		printf("FPGA is already booted\n");
		return 1;
	}

	printf("Reading Flash Addr 0x%08X to memory 0x%08X, len %d bytes\n", flash_addr, data_addr, len);

	for (uint32_t i = 0; i < len; i += 4) {
		writel(WORD_ADDRESS(flash_addr + i), UPDATER_ADDR);
		writel(UPDATER_CTRL_START_READ, UPDATER_CTRL);
		/* MAX 10 Documentation does not list max read time. */
		ret = flash_wait_until_idle(10000, &reg);
		if (ret || ((reg & UPDATER_STATUS_READ_SUCCESS) == 0)) {
			printf("Flash Read failed at offset %d\n", i);
			return 1;
		}

		/* Read the data from UPDATER_FLASHDATA and store it in system RAM */
		data = readl(UPDATER_FLASHDATA);
		data = swap_bitstream_order(data);
		*(volatile uint32_t *)(uintptr_t)(data_addr + i) = data;

		if (ctrlc()) {
			printf("Killed\n");
			return 1;
		}
	}

	return 0;
}

int flash_sector_erase(uint8_t sector)
{
	uint32_t reg;
	int ret;

	if (sector > 5)
		return 1;

	writel(sector << 20, UPDATER_CTRL);

	/* Max sector erase time is 350ms */
	ret = flash_wait_until_idle(10000, &reg);
	if (ret || ((reg & UPDATER_STATUS_ERASE_SUCCESS) == 0)) {
		printf("Flash Erase failed at sector %d\n", sector);
		return 1;
	}
	
	return  0;
}

int flash_update_app(uint32_t addr, uint32_t len)
{
	int ret;

	if (!fpga_is_bootloader()) {
		printf("FPGA is already booted\n");
		return 1;
	}

	/* Erase CFM1 */
	ret = flash_sector_erase(3);
	if (ret)
		goto flash_fail;
	ret = flash_sector_erase(4);
	if (ret)
		goto flash_fail;
	return flash_write(CFM1_BASE, addr, len);

flash_fail:
	printf("Flash Erase failed\n");
	return ret;
}

int flash_read_app(uint32_t addr, uint32_t len)
{
	if (!fpga_is_bootloader()) {
		printf("FPGA is already booted\n");
		return 1;
	}

	return flash_read(CFM1_BASE, addr, len);
}

int flash_update_bootloader(uint32_t addr, uint32_t len)
{
	int ret;

	if (!fpga_is_bootloader()) {
		printf("FPGA is already booted\n");
		return 1;
	}

	/* Erase CFM0 */
	ret = flash_sector_erase(5);
	if (ret)
		return ret;
	return flash_write(CFM0_BASE, addr, len);
}

int flash_read_bootloader(uint32_t addr, uint32_t len)
{
	return flash_read(CFM0_BASE, addr, len);
}

void fpga_reconfig(void)
{
	if (!fpga_is_bootloader()) {
		printf("FPGA is already booted\n");
		return;
	}

	writel(UPDATER_RECONFIG_IMAGE_SELECT | UPDATER_RECONFIG_RECONFIG_START,
	       UPDATER_RECONFIG);
	mdelay(18); // Supports 10M08/10M16 compressed RBF config time

	if (fpga_is_bootloader()) {
		printf("FPGA stuck in bootloader\n");
		writel(UPDATER_RECONFIG_EN_ERROR_LEDS, UPDATER_RECONFIG);
		return;
	}
}

/* Arbitrary number to take ~1 second */
#define NUM_TEST_ITERATIONS 500000
static int fpga_scratch_test(void)
{
    int ret = 0;
    volatile uint32_t *scratch0_addr = (volatile uint32_t *)FPGA_SCRATCH0;
    volatile uint32_t *scratch1_addr = (volatile uint32_t *)FPGA_SCRATCH1;
    uint32_t read_value;
    uint32_t random_value;
    int total_tests = 0;
    int failed_tests = 0;

    // Test predefined values on both scratch registers
    uint32_t test_values[] = {0x00000000, 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA};
    for (int i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++) {
        // Test FPGA_SCRATCH0
        writel(test_values[i], scratch0_addr);
        read_value = readl(scratch0_addr);
        total_tests++;
        if (read_value != test_values[i]) {
			if (failed_tests < 10) {
				printf("Error: FPGA_SCRATCH0 wrote 0x%08X but read back 0x%08X\n", test_values[i], read_value);
			}
            failed_tests++;
            ret = -1;
        }

        // Test FPGA_SCRATCH1
        writel(test_values[i], scratch1_addr);
        read_value = readl(scratch1_addr);
        total_tests++;
        if (read_value != test_values[i]) {
			if (failed_tests < 10) {
				printf("Error: FPGA_SCRATCH1 wrote 0x%08X but read back 0x%08X\n", test_values[i], read_value);
			}
            failed_tests++;
            ret = -1;
        }
    }

    // Randomized stress test on both scratch registers
    for (int i = 0; i < NUM_TEST_ITERATIONS; i++) {
        random_value = rand();

        // Write and verify FPGA_SCRATCH0
        writel(random_value, scratch0_addr);
        read_value = readl(scratch0_addr);
        total_tests++;
        if (read_value != random_value) {
			if (failed_tests < 10) {
            	printf("Error: FPGA_SCRATCH0 wrote 0x%08X but read back 0x%08X\n", random_value, read_value);
			}
            failed_tests++;
            ret = -1;
        }

        // Write and verify FPGA_SCRATCH1
        writel(random_value, scratch1_addr);
        read_value = readl(scratch1_addr);
        total_tests++;
        if (read_value != random_value) {
			if (failed_tests < 10) {
            	printf("Error: FPGA_SCRATCH1 wrote 0x%08X but read back 0x%08X\n", random_value, read_value);
			}
            failed_tests++;
            ret = -1;
        }
    }

    int passed_tests = total_tests - failed_tests;
    int pass_percentage = (passed_tests * 100) / total_tests;
    int fail_percentage = (failed_tests * 100) / total_tests;

    printf("Scratch register test completed.\n");
    printf("Total tests: %d, Passed: %d (%d%%), Failed: %d (%d%%)\n",
           total_tests, passed_tests, pass_percentage, failed_tests, fail_percentage);

    return ret;
}

static int do_fpgaboot(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	uint32_t addr;
	uint32_t len;
	int ret = 0;

	if (argc == 2) {
		if (strcmp(argv[1], "info") == 0) {
			print_fpga_version();
		} else if (strcmp(argv[1], "start") == 0) {
			fpga_reconfig();
		}  else if (strcmp(argv[1], "test") == 0) {
			fpga_scratch_test();
		}  else if (strcmp(argv[1], "update") == 0) {
			fpga_update_from_flash();
		} else {
			ret = CMD_RET_USAGE;
		}
	} else if (argc == 4) {
		addr = simple_strtoul(argv[2], NULL, 16);
		len = simple_strtoul(argv[3], NULL, 16);

		if (strcmp(argv[1], "write") == 0) {
			ret = flash_update_app(addr, len);
		} else if (strcmp(argv[1], "read") == 0) {
			ret = flash_read_app(addr, len);
		} else if (strcmp(argv[1], "unsafe_update_bootloader") == 0) {
			ret = flash_update_bootloader(addr, len);
		} else if (strcmp(argv[1], "read_bootloader") == 0) {
			ret = flash_read_bootloader(addr, len);
		} else {
			ret = CMD_RET_USAGE;
		}
	} else {
		ret = CMD_RET_USAGE;
	}

	return ret;
}

U_BOOT_CMD(fpgaboot, 5, 1, do_fpgaboot, "fpga bootloader command",
	   "info\n"
	   "write addr len\n"
	   "read addr len\n"
	   "unsafe_update_bootloader addr len\n"
	   "read_bootloader addr len\n"
	   "test\n"
	   "update\n"
	   "start\n");
