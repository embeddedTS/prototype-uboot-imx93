#include <env.h>
#include <stdio.h>

#include "../ts-common/tsfpga.h"

void do_bbdetect(void)
{
	uint8_t id = 0;
	int i;

	fpga_gpio_set_as_output(EN_RED_LED_N);
	fpga_gpio_set_as_output(EN_GREEN_LED_N);
	fpga_gpio_set_as_output(DIO_52);
	fpga_gpio_set_as_input(DIO_84);

	for (i = 0; i < 8; i++) {
		if (i & 1)
			fpga_gpio_output(EN_RED_LED_N, 1);
		else
			fpga_gpio_output(EN_RED_LED_N, 0);

		if (i & 2)
			fpga_gpio_output(EN_GREEN_LED_N, 1);
		else
			fpga_gpio_output(EN_GREEN_LED_N, 0);

		if (i & 4)
			fpga_gpio_output(DIO_52, 1);
		else
			fpga_gpio_output(DIO_52, 0);

		id >>= 1;
		if (fpga_gpio_input(DIO_84))
			id |= 0x80;
	}
	printf("Baseboard ID: 0x%X\n", id & ~0xc0);
	printf("Baseboard Rev: %d\n", ((id & 0xc0) >> 6));
	env_set_hex("baseboard_id", id & ~0xc0);
	env_set_hex("baseboard_rev", ((id & 0xc0) >> 6));
}
