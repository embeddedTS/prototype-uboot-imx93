// SPDX-License-Identifier: GPL-2.0

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/bitops.h>

/* Read Decodes */
#define TS7820_OE_IN		0x00
#define TS7820_OUT_DATA		0x08
#define TS7820_IN		0x0C

/* Write Decodes */
#define TS7820_OE_SET		0x00
#define TS7820_OE_CLR		0x04
#define TS7820_DAT_SET		0x08
#define TS7820_DAT_CLR		0x0C

struct ts7820_gpio {
	void __iomem *base;
};

static int ts7820_gpio_set(struct udevice *dev, unsigned offset, int value)
{
	struct ts7820_gpio *ts7820_gpio = dev_get_priv(dev);

	if (value)
		writel(BIT(offset), ts7820_gpio->base + TS7820_DAT_SET);
	else
		writel(BIT(offset), ts7820_gpio->base + TS7820_DAT_CLR);

	return 0;
}

static int ts7820_gpio_get(struct udevice *dev, unsigned int pin)
{
	struct ts7820_gpio *ts7820_gpio = dev_get_priv(dev);

	return !!(readl(ts7820_gpio->base + TS7820_IN) & BIT(pin));
}

static int ts7820_gpio_direction_input(struct udevice *dev,
					unsigned int pin)
{
	struct ts7820_gpio *ts7820_gpio = dev_get_priv(dev);

	writel(BIT(pin), ts7820_gpio->base + TS7820_OE_CLR);
	return 0;
}

static int ts7820_gpio_direction_output(struct udevice *dev,
					unsigned int pin, int val)
{
	struct ts7820_gpio *ts7820_gpio = dev_get_priv(dev);

	ts7820_gpio_set(dev, pin, val);
	writel(BIT(pin), ts7820_gpio->base + TS7820_OE_SET);
	return 0;
}

static int ts7820_gpio_probe(struct udevice *dev)
{
	struct ts7820_gpio *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	return 0;
}

static const struct udevice_id ts7820_gpio_of_match[] = {
	{ .compatible = "technologic,ts7820-gpio", },
	{},
};

static const struct dm_gpio_ops ts7820_gpio_ops = {
	.direction_input	= ts7820_gpio_direction_input,
	.direction_output	= ts7820_gpio_direction_output,
	.get_value		= ts7820_gpio_get,
	.set_value		= ts7820_gpio_set,
};

U_BOOT_DRIVER(ts7820_gpio) = {
	.name		= "ts7820-gpio",
	.id		= UCLASS_GPIO,
	.of_match 	= ts7820_gpio_of_match,
	.ops		= &ts7820_gpio_ops,
	.priv_auto	= sizeof(struct ts7820_gpio),
	.probe		= ts7820_gpio_probe,
};
