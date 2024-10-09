/*
 * Copyright (C) 2019-2024 Technologic Systems dba embeddedTS
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

/*
 * Knowledge of TS-9370 strapping is encapsulated in this file.
 */

#include <vsprintf.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch-imx9/imx93_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm-generic/gpio.h>
#include <asm/mach-imx/gpio.h>
#include <linux/delay.h>

#include "parse_straps.h"
#include "ocram_debug.h"

#define FPGA_PAD_CTRL	(PAD_CTL_DSE(6) | PAD_CTL_ODE | PAD_CTL_PUE)

#define	UART6_TXD	IMX_GPIO_NR(1, 4)	/* R131 / Bit 7 */
#define	UART7_HS	IMX_GPIO_NR(1, 10)	/* R133 / Bit 6 */
#define	UART3_TXD	IMX_GPIO_NR(1, 14)	/* R133 / Bit 5 */
#define	UART5_TXD	IMX_GPIO_NR(1, 0)	/* R134 / Bit 4  */
#define	UART8_TXD	IMX_GPIO_NR(1, 12)	/* R127 / Bit 3 */
#define	UART7_TXD	IMX_GPIO_NR(1, 8)	/* R128 / Bit 2 */
#define	SPI_CSn		IMX_GPIO_NR(1, 18)	/* R129 / Bit 1 */
#define	SPI_MOSILK	IMX_GPIO_NR(1, 20)	/* R130 / Bit 0 */

#define GPIO_01	IMX_GPIO_NR(1, 1)
#define GPIO_02	IMX_GPIO_NR(1, 2)
#define GPIO_03	IMX_GPIO_NR(1, 3)
#define GPIO_05	IMX_GPIO_NR(1, 5)
#define GPIO_06	IMX_GPIO_NR(1, 6)
#define GPIO_07	IMX_GPIO_NR(1, 7)
#define GPIO_09	IMX_GPIO_NR(1, 9)
#define GPIO_11	IMX_GPIO_NR(1, 11)
#define GPIO_13	IMX_GPIO_NR(1, 13)
#define GPIO_15	IMX_GPIO_NR(1, 15)
#define GPIO_16	IMX_GPIO_NR(1, 16)
#define GPIO_17	IMX_GPIO_NR(1, 17)
#define GPIO_19	IMX_GPIO_NR(1, 19)
#define GPIO_21	IMX_GPIO_NR(1, 21)

static void preserve_straps(uint16_t straps);
#ifndef CONFIG_SPL_BUILD
static uint16_t read_straps_preserved_by_spl(void);
#endif

#define STRAP_PAD_PD_CTRL ( PAD_CTL_PDE )

static iomux_v3_cfg_t const strap_pads[] = {
	MX93_PAD_GPIO_IO00__GPIO2_IO00 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO04__GPIO2_IO04 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO08__GPIO2_IO08 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO10__GPIO2_IO10 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO12__GPIO2_IO12 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO14__GPIO2_IO14 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO18__GPIO2_IO18 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO20__GPIO2_IO20 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO01__GPIO2_IO01 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO02__GPIO2_IO02 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO03__GPIO2_IO03 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO05__GPIO2_IO05 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO06__GPIO2_IO06 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO07__GPIO2_IO07 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO09__GPIO2_IO09 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO11__GPIO2_IO11 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO13__GPIO2_IO13 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO15__GPIO2_IO15 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO16__GPIO2_IO16 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO17__GPIO2_IO17 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO19__GPIO2_IO19 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL),
	MX93_PAD_GPIO_IO21__GPIO2_IO21 | MUX_PAD_CTRL(STRAP_PAD_PD_CTRL)
};

const char *get_board_model(void)
{
	return "9370";
}

const char *get_board_name(void)
{
	static char name_str[12] = {0};

	snprintf(name_str, sizeof(name_str), "TS-9370");
	return name_str;
}

const char get_board_version_char(void)
{
	uint16_t raw_cpu_straps = 0;

	raw_cpu_straps =  read_raw_cpu_straps();
	return '1';
}

uint32_t get_straps(void)
{
	uint16_t raw_cpu_straps = 0;

	raw_cpu_straps =  read_raw_cpu_straps();
	return raw_cpu_straps;
}

const char *get_straps_str(void)
{
	uint16_t raw_cpu_straps = 0;
	static char straps_str[10] = {0};

	raw_cpu_straps =  read_raw_cpu_straps();
	snprintf(straps_str, sizeof(straps_str), "%04x", raw_cpu_straps);
	return straps_str;
}

const char *get_board_version_str(void)
{
	static char model_str[24] = {0};

	snprintf(model_str, sizeof(model_str), "P3");
	return model_str;
}

uint8_t read_board_opts(void)
{
	uint8_t cpu_opts;

	cpu_opts = read_raw_cpu_straps();
	return cpu_opts;
}

static uint16_t gpio_bits_to_straps(void)
{
	uint16_t cpu_straps = 0;

	cpu_straps |= (gpio_get_value(UART6_TXD) << 7);  // GPIO_4
	cpu_straps |= (gpio_get_value(UART7_HS) << 6);   // GPIO_10
	cpu_straps |= (gpio_get_value(UART3_TXD) << 5);  // GPIO_14
	cpu_straps |= (gpio_get_value(UART5_TXD) << 4);  // GPIO_0
	cpu_straps |= (gpio_get_value(UART8_TXD) << 3);  // GPIO_12
	cpu_straps |= (gpio_get_value(UART7_TXD) << 2);  // GPIO_8
	cpu_straps |= (gpio_get_value(SPI_CSn) << 1);     // GPIO_18
	cpu_straps |= (gpio_get_value(SPI_MOSILK) << 0);  // GPIO_20
	cpu_straps &= 0x00ff;
	return cpu_straps;
}

uint16_t read_raw_cpu_straps(void)
{
	static uint16_t cpu_straps = 0;
	static bool read = 0;

	if (!read) {
		imx_iomux_v3_setup_multiple_pads(strap_pads, ARRAY_SIZE(strap_pads));

		gpio_request(UART6_TXD, "UART6_TXD");	/* R131 / (1, 4) */
		gpio_request(UART7_HS, "UART7_HS");	/* R132 / (1, 10) */
		gpio_request(UART3_TXD, "UART3_TXD");	/* R133 / (1, 14) */
		gpio_request(UART5_TXD, "UART5_TXD");	/* R134 / (1, 0) */
		gpio_request(UART8_TXD, "UART8_TXD");	/* R127 / (1, 12) */
		gpio_request(UART7_TXD, "UART7_TXD");	/* R128 / (1, 8) */
		gpio_request(SPI_CSn, "SPI_CSn");	/* R129 / (1, 18) */
		gpio_request(SPI_MOSILK, "SPI_MOSILK");	/* R130 / (1, 20) */

		mdelay(1);

		gpio_direction_input(UART6_TXD);
		gpio_direction_input(UART7_HS);
		gpio_direction_input(UART3_TXD);
		gpio_direction_input(UART5_TXD);
		gpio_direction_input(UART8_TXD);
		gpio_direction_input(UART7_TXD);
		gpio_direction_input(SPI_CSn);
		gpio_direction_input(SPI_MOSILK);

		mdelay(1);

		cpu_straps = gpio_bits_to_straps();
		/*
		 * Populated means pulled up to 1.8 V.
		 * Resistor populated (pull-up) means the pad should read a 1
		 * and a 0 when the resistor is not populated.
		 */
		preserve_straps(cpu_straps);

#ifndef CONFIG_SPL_BUILD
		/*
		 * There is no guarantee that U-Boot can read the
		 * straps accurately at this point. Thus SPL saves
		 * what it finds in OCRAM, and U-Boot should use
		 * that. Before switching to the SPL's value, we save
		 * what U-Boot read (in `saved_straps` rather than
		 * `spl_saved_straps`) as an engineering data
		 * point. When the FPGA is available in U-Boot, we
		 * will pass the strapping in register memory there.
		 */
		cpu_straps = read_straps_preserved_by_spl();
#endif
		read = 1;
	}
	return cpu_straps;
}

#ifndef CONFIG_SPL_BUILD
static uint16_t read_straps_preserved_by_spl(void)
{
	static uint32_t tmp_save_intermediate = 0;
	tmp_save_intermediate = ocram_debug_area_p->spl_saved_straps;
	return tmp_save_intermediate;
}
#endif

static void preserve_straps(uint16_t straps)
{
	static uint32_t tmp_save_intermediate = 0;
	tmp_save_intermediate |= straps;
#ifdef CONFIG_SPL_BUILD
	ocram_debug_area_p->spl_saved_straps = tmp_save_intermediate;
#else
	ocram_debug_area_p->saved_straps = tmp_save_intermediate;
#endif
}
