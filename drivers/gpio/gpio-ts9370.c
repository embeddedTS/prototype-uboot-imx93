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
#define TS9370_OE_IN		0x00
#define TS9370_OUT_DATA		0x08
#define TS9370_IN		0x0C

/* Write Decodes */
#define TS9370_OE_SET		0x00
#define TS9370_OE_CLR		0x04
#define TS9370_DAT_SET		0x08
#define TS9370_DAT_CLR		0x0C

struct ts9370_gpio {
	void __iomem *base;
};

static int ts9370_gpio_set(struct udevice *dev, unsigned offset, int value)
{
	struct ts9370_gpio *ts9370_gpio = dev_get_priv(dev);

	if (value)
		writel(BIT(offset), ts9370_gpio->base + TS9370_DAT_SET);
	else
		writel(BIT(offset), ts9370_gpio->base + TS9370_DAT_CLR);

	return 0;
}

static int ts9370_gpio_get(struct udevice *dev, unsigned int pin)
{
	struct ts9370_gpio *ts9370_gpio = dev_get_priv(dev);

	return !!(readl(ts9370_gpio->base + TS9370_IN) & BIT(pin));
}

static int ts9370_gpio_direction_input(struct udevice *dev,
					unsigned int pin)
{
	struct ts9370_gpio *ts9370_gpio = dev_get_priv(dev);

	writel(BIT(pin), ts9370_gpio->base + TS9370_OE_CLR);
	return 0;
}

static int ts9370_gpio_direction_output(struct udevice *dev,
					unsigned int pin, int val)
{
	struct ts9370_gpio *ts9370_gpio = dev_get_priv(dev);

	ts9370_gpio_set(dev, pin, val);
	writel(BIT(pin), ts9370_gpio->base + TS9370_OE_SET);
	return 0;
}

static int ts9370_gpio_probe(struct udevice *dev)
{
	struct ts9370_gpio *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	return 0;
}

static const struct udevice_id ts9370_gpio_of_match[] = {
	{ .compatible = "technologic,ts9370-gpio", },
	{},
};

static const struct dm_gpio_ops ts9370_gpio_ops = {
	.direction_input	= ts9370_gpio_direction_input,
	.direction_output	= ts9370_gpio_direction_output,
	.get_value		= ts9370_gpio_get,
	.set_value		= ts9370_gpio_set,
};

U_BOOT_DRIVER(ts9370_gpio) = {
	.name		= "ts9370-gpio",
	.id		= UCLASS_GPIO,
	.of_match 	= ts9370_gpio_of_match,
	.ops		= &ts9370_gpio_ops,
	.priv_auto	= sizeof(struct ts9370_gpio),
	.probe		= ts9370_gpio_probe,
};
