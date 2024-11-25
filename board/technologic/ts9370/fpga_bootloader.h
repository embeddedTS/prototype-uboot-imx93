#pragma once

/* These updater regs are only availble while in the bootloader before we have booted 
 * to the application load */
#define UPDATER_BASE				(FPGA_BASE + 0x100)

/* Status Register (0x0) */
#define UPDATER_STATUS				(UPDATER_BASE + 0x0)
#define UPDATER_STATUS_ERASE_SUCCESS		BIT(4)
#define UPDATER_STATUS_WRITE_SUCCESS		BIT(3)
#define UPDATER_STATUS_READ_SUCCESS		BIT(2)
#define UPDATER_STATUS_BUSY_MASK		(0x3)
#define UPDATER_STATUS_BUSY_SHIFT		0
#define UPDATER_STATUS_BUSY_IDLE		(0x0 << UPDATER_STATUS_BUSY_SHIFT)
#define UPDATER_STATUS_BUSY_ERASE		(0x1 << UPDATER_STATUS_BUSY_SHIFT)
#define UPDATER_STATUS_BUSY_WRITE		(0x2 << UPDATER_STATUS_BUSY_SHIFT)
#define UPDATER_STATUS_BUSY_READ		(0x3 << UPDATER_STATUS_BUSY_SHIFT)
#define UPDATER_STATUS_SECTOR1_PROTECTED	BIT(5)
#define UPDATER_STATUS_SECTOR2_PROTECTED	BIT(6)
#define UPDATER_STATUS_SECTOR3_PROTECTED	BIT(7)
#define UPDATER_STATUS_SECTOR4_PROTECTED	BIT(8)
#define UPDATER_STATUS_SECTOR5_PROTECTED	BIT(9)

/* Control Register (0x4) */
#define UPDATER_CTRL				(UPDATER_BASE + 0x4)
#define UPDATER_CTRL_START_READ			BIT(31)
#define UPDATER_CTRL_SECTOR1_WP			BIT(23)
#define UPDATER_CTRL_SECTOR2_WP			BIT(24)
#define UPDATER_CTRL_SECTOR3_WP			BIT(25)
#define UPDATER_CTRL_SECTOR4_WP			BIT(26)
#define UPDATER_CTRL_SECTOR5_WP			BIT(27)
#define UPDATER_CTRL_SECTOR_ERASE_SHIFT		20
#define UPDATER_CTRL_SECTOR_ERASE_MASK		(0x7 << UPDATER_CTRL_SECTOR_ERASE_SHIFT)
#define UPDATER_CTRL_SECTOR1_ERASE		(0x1 << UPDATER_CTRL_SECTOR_ERASE_SHIFT)
#define UPDATER_CTRL_SECTOR2_ERASE		(0x2 << UPDATER_CTRL_SECTOR_ERASE_SHIFT)
#define UPDATER_CTRL_SECTOR3_ERASE		(0x3 << UPDATER_CTRL_SECTOR_ERASE_SHIFT)
#define UPDATER_CTRL_SECTOR4_ERASE		(0x4 << UPDATER_CTRL_SECTOR_ERASE_SHIFT)
#define UPDATER_CTRL_SECTOR5_ERASE		(0x5 << UPDATER_CTRL_SECTOR_ERASE_SHIFT)
#define UPDATER_CTRL_PAGE_ERASE_MASK		(0xFFFFF)

/* Flash Address Register (0x8) */
#define UPDATER_ADDR				(UPDATER_BASE + 0x8)

/* Flash Data Register (0xC) */
#define UPDATER_FLASHDATA			(UPDATER_BASE + 0xC)

/* Reconfiguration Register (0x10) */
#define UPDATER_RECONFIG			(UPDATER_BASE + 0x10)
#define UPDATER_RECONFIG_EN_ERROR_LEDS		BIT(2)
#define UPDATER_RECONFIG_IMAGE_SELECT		BIT(1)
#define UPDATER_RECONFIG_RECONFIG_START		BIT(0)

/*
 * Sectors IDs and addresses
 * 1: 0x00000 - 0x03FFF UFM
 * 2: 0x04000 - 0x07FFF UFM
 * 3: 0x08000 - 0x1CFFF CFM (Image 2)
 * 4: 0x1C800 - 0x2AFFF CFM (Image 2)
 * 5: 0x2B000 - 0x4DFFF CFM (Image 1)
 */
#define CFM0_BASE		0x2B000
#define CFM1_BASE		0x08000
#define CFM_SIZE		0x23000
#define WORD_ADDRESS(val)	((val) >> 2)

uint32_t swap_bitstream_order(uint32_t x);
int flash_wait_until_idle(uint32_t timeout_ms, uint32_t *reg);
int flash_write(uint32_t flash_addr, uint32_t data_addr, uint32_t len);
int flash_read(uint32_t flash_addr, uint32_t data_addr, uint32_t len);
int flash_sector_erase(uint8_t sector);
int flash_update_app(uint32_t addr, uint32_t len);
int flash_read_app(uint32_t addr, uint32_t len);
int flash_update_bootloader(uint32_t addr, uint32_t len);
int flash_read_bootloader(uint32_t addr, uint32_t len);
int fpga_update_from_flash(void);
