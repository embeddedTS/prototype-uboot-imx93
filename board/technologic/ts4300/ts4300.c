// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 * Copyright 2023-2024 Technologic Systems, Inc. (dba embeddedTS)
 */

#include <env.h>
#include <efi_loader.h>
#include <init.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <asm/arch-imx9/ccm_regs.h>
#include <asm/arch-imx9/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-imx9/imx93_pins.h>
#include <asm/arch/clock.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <asm/gpio.h>
#include <linux/delay.h>

#include "parse_straps.h"

#include <dm/root.h>
#include "../ts-common/fpga_bootloader.h"
#include "../ts-common/ts-macs.h"
#include "../ts-common/tsfpga.h"
#include "../ts-common/wizard.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_DSE(6) | PAD_CTL_FSEL2)
#define FPGA_PAD_CTRL	(PAD_CTL_DSE(6) | PAD_CTL_ODE | PAD_CTL_PUE)

static const iomux_v3_cfg_t uart_pads[] = {
	MX93_PAD_UART1_RXD__LPUART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX93_PAD_UART1_TXD__LPUART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static int setup_eqos(void);
static const iomux_v3_cfg_t fpga_pads[] = {
	MX93_PAD_GPIO_IO02__GPIO2_IO02 | MUX_PAD_CTRL(FPGA_PAD_CTRL), // DEV_CLRN / GPIO_02
	MX93_PAD_GPIO_IO11__GPIO2_IO11 | MUX_PAD_CTRL(FPGA_PAD_CTRL), // NSTATUS / GPIO_11
	MX93_PAD_GPIO_IO10__GPIO2_IO10 | MUX_PAD_CTRL(FPGA_PAD_CTRL), // CONF_DONE / GPIO_10
};

static int setup_fec(void);
static const iomux_v3_cfg_t fec_enet_pads[] = {
	MX93_PAD_SD3_DATA3__GPIO3_IO25 | MUX_PAD_CTRL(FPGA_PAD_CTRL), // ETH2_RESET#
	MX93_PAD_SD3_DATA2__GPIO3_IO24 | MUX_PAD_CTRL(FPGA_PAD_CTRL), // ETH1_RESET#
};

#if CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT)
#define IMX_BOOT_IMAGE_GUID \
	EFI_GUID(0xbc550d86, 0xda26, 0x4b70, 0xac, 0x05, \
		 0x2a, 0x44, 0x8e, 0xda, 0x6f, 0x21)

struct efi_fw_image fw_images[] = {
	{
		.image_type_id = IMX_BOOT_IMAGE_GUID,
		.fw_name = u"IMX93-TS-4300",
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 0=flash-bin raw 0 0x2000 mmcpart 1",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */

// Used in SPL:
int board_early_init_f(void)
{
	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
	init_uart_clk(LPUART1_CLK_ROOT);

	// Emit a clock for the onboard USB hub
	ccm_clk_root_cfg(CCM_CKO1_CLK_ROOT, OSC_24M_CLK, 1);

	return 0;
}

static int setup_fec(void)
{
	imx_iomux_v3_setup_multiple_pads(fec_enet_pads, ARRAY_SIZE(fec_enet_pads));

	return set_clk_enet(ENET_125MHZ);
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

static int setup_eqos(void)
{
	struct blk_ctrl_wakeupmix_regs *bctrl =
		(struct blk_ctrl_wakeupmix_regs *)BLK_CTRL_WAKEUPMIX_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(fpga_pads, ARRAY_SIZE(fpga_pads));
	/* set INTF as RGMII, enable RGMII TXC clock */
	clrsetbits_le32(&bctrl->eqos_gpr,
			BCTRL_GPR_ENET_QOS_INTF_MODE_MASK,
			BCTRL_GPR_ENET_QOS_INTF_SEL_RGMII | BCTRL_GPR_ENET_QOS_CLK_GEN_EN);

	return 0;
}

int board_init(void)
{
	if (CONFIG_IS_ENABLED(DTB_RESELECT)) {
		int rescan;
		int ret;
		const char *model;

		ret = fdtdec_resetup(&rescan);
		if (!ret && rescan) {
			dm_uninit();
			dm_init_and_scan(false);
		}
		model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
		if (model)
			printf("Model: %s\n", model);
	}

	if (CONFIG_IS_ENABLED(FEC_MXC))
		setup_fec();

	if (CONFIG_IS_ENABLED(DWC_ETH_QOS))
		setup_eqos();

	return 0;
}

int board_late_init(void)
{
	char rev_as_str[2] = {0};
	u32 cpu_straps;
	u32 board_straps;

	/* TS-4300 has 2 onboard ethernet, and reserves 1 mac for some carrier
	 * boards that have a USB ethernet */
	setup_mac_addresses(3);

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	env_set("sec_boot", "no");
#ifdef CONFIG_AHAB_BOOT
	env_set("sec_boot", "yes");
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("model", get_board_model());

	cpu_straps = read_raw_cpu_straps();
	env_set_hex("raw_cpu_straps", cpu_straps);

	env_set("board_name", get_board_name());
	rev_as_str[0] = get_board_version_char();
	env_set("board_rev", rev_as_str);
	env_set("board_rev_name", get_board_version_str());
	env_set_hex("board_rev_straps", get_straps());
	board_straps = get_straps();
	env_set_hex("board_early_straps", board_straps);
#endif

	fpga_update_from_flash();

	if (!env_get("skip_fpga_reconfig")) {
		print_fpga_version();
		fpga_reconfig();
	} else {
		printf("Skipping FPGA reconfig\n");
	}
	print_fpga_version();

	/* Drive EN_USB_HOST_5V high */
	writel(1 << 7, FPGA_GPIO_BANK_DATA_SET_ADDR(1));

	/* Pulse OFF_BD_RESET# for 1ms */
	writel(1 << 6, FPGA_GPIO_BANK_DATA_CLEAR_ADDR(0));
	mdelay(1);
	writel(1 << 6, FPGA_GPIO_BANK_DATA_SET_ADDR(0));

	/* Leave on RED LED by default. TODO: Migrate to dts/driver/config*/
	writel(1 << 1, FPGA_GPIO_BANK_DATA_CLEAR_ADDR(0));

	return 0;
}

#ifdef CONFIG_DTB_RESELECT
int embedded_dtb_select(void)
{
	// If CONFIG_DISPLAY_BOARDINFO!=n, you'll see "Model: TS-4300" until board_init() runs
	fdtdec_setup();
	return 0;
}
#endif

#ifdef CONFIG_FSL_FASTBOOT
#ifdef CONFIG_ANDROID_RECOVERY
int is_recovery_key_pressing(void)
{
	return 0;
}
#endif /*CONFIG_ANDROID_RECOVERY*/
#endif /*CONFIG_FSL_FASTBOOT*/
