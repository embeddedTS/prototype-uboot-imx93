// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 * Copyright 2023-2024 Technologic Systems, Inc. (dba embeddedTS)
 */

#include <common.h>
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

#include "parse_straps.h"
#include "tsfpga.h"
#include "fpga_bootloader.h"

#include <dm/root.h>
#include "../ts-common/wizard.h"
#include "../ts-common/ts-macs.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_DSE(6) | PAD_CTL_FSEL2)
#define FPGA_PAD_CTRL	(PAD_CTL_DSE(6) | PAD_CTL_ODE | PAD_CTL_PUE)

static iomux_v3_cfg_t const uart_pads[] = {
	MX93_PAD_UART1_RXD__LPUART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX93_PAD_UART1_TXD__LPUART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#if CONFIG_IS_ENABLED(DWC_ETH_QOS)
static int setup_eqos(void);
static iomux_v3_cfg_t const fpga_pads[] = {
	MX93_PAD_GPIO_IO02__GPIO2_IO02 | MUX_PAD_CTRL(FPGA_PAD_CTRL), // DEV_CLRN / GPIO_02
	MX93_PAD_GPIO_IO11__GPIO2_IO11 | MUX_PAD_CTRL(FPGA_PAD_CTRL), // NSTATUS / GPIO_11
	MX93_PAD_GPIO_IO10__GPIO2_IO10 | MUX_PAD_CTRL(FPGA_PAD_CTRL), // CONF_DONE / GPIO_10
};
#endif

#if CONFIG_IS_ENABLED(FEC_MXC)
static int setup_fec(void);
static iomux_v3_cfg_t const fec_enet_pads[] = {
	MX93_PAD_SD3_DATA3__GPIO3_IO25 | MUX_PAD_CTRL(FPGA_PAD_CTRL), // ETH2_RESET#
	MX93_PAD_SD3_DATA2__GPIO3_IO24 | MUX_PAD_CTRL(FPGA_PAD_CTRL), // ETH1_RESET#
};
#endif

#if CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT)
#define IMX_BOOT_IMAGE_GUID \
	EFI_GUID(0xbc550d86, 0xda26, 0x4b70, 0xac, 0x05, \
		 0x2a, 0x44, 0x8e, 0xda, 0x6f, 0x21)

struct efi_fw_image fw_images[] = {
	{
		.image_type_id = IMX_BOOT_IMAGE_GUID,
		.fw_name = u"IMX93-11X11-EVK-RAW",
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

	/* Pass through USB clock */
	ccm_clk_root_cfg(CCM_CKO1_CLK_ROOT, OSC_24M_CLK, 1);

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	uint16_t board_model_register = 0;

	board_model_register = get_board_model_register_early();

	switch (board_model_register) {
	case 0x9370:
		if (!strcmp(name, "imx93-ts9370"))
			return 0;
		break;
	case 0x9390:
		if (!strcmp(name, "imx93-ts9390"))
			return 0;
		break;
	case 0x4300:
		if (!strcmp(name, "imx93-ts4300"))
			return 0;
		break;
	case 0x0000:
		// Default if board_model_register can't be read at this time:
		if (!strcmp(name, CONFIG_DEFAULT_DEVICE_TREE))
			return 0;
		break;
	default:
		// -EINVAL if the board model is unrecognized
		break;
	}
	return -EINVAL;
}

#if CONFIG_IS_ENABLED(FEC_MXC)
static int setup_fec(void)
{
	imx_iomux_v3_setup_multiple_pads(fec_enet_pads, ARRAY_SIZE(fec_enet_pads));

	return set_clk_enet(ENET_125MHZ);
}
#endif

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#if CONFIG_IS_ENABLED(DWC_ETH_QOS)
static int setup_eqos(void)
{
	struct blk_ctrl_wakeupmix_regs *bctrl =
		(struct blk_ctrl_wakeupmix_regs *)BLK_CTRL_WAKEUPMIX_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(fpga_pads, ARRAY_SIZE(fpga_pads));

	if (!IS_ENABLED(CONFIG_TARGET_IMX93_14X14_EVK)) {
		/* set INTF as RGMII, enable RGMII TXC clock */
		clrsetbits_le32(&bctrl->eqos_gpr,
				BCTRL_GPR_ENET_QOS_INTF_MODE_MASK,
				BCTRL_GPR_ENET_QOS_INTF_SEL_RGMII | BCTRL_GPR_ENET_QOS_CLK_GEN_EN);

		return set_clk_eqos(ENET_125MHZ);
	}

	return 0;
}
#endif

int board_init(void)
{
#if CONFIG_IS_ENABLED(DTB_RESELECT)
	int rescan;
	int ret;
#  if !IS_ENABLED(CONFIG_DISPLAY_BOARDINFO_LATE) && !IS_ENABLED(CONFIG_DISPLAY_BOARDINFO)
	const char *model;
#  endif

	ret = fdtdec_resetup(&rescan);
        if (!ret && rescan) {
		dm_uninit();
		dm_init_and_scan(false);
	}
#  if !IS_ENABLED(CONFIG_DISPLAY_BOARDINFO_LATE) && !IS_ENABLED(CONFIG_DISPLAY_BOARDINFO)
	model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
	if (model) {
		printf("Model: %s\n", model);
	}
#  endif
#endif

#if CONFIG_IS_ENABLED(FEC_MXC)
	setup_fec();
#endif

#if CONFIG_IS_ENABLED(DWC_ETH_QOS)
	setup_eqos();
#endif

	return 0;
}

int board_late_init(void)
{
	char rev_as_str[2] = {0};
	uint32_t cpu_straps;
	uint32_t board_straps;

	setup_mac_addresses();
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
	env_set("board_rev_straps", get_straps_str());
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
