/* linux/arch/arm/plat-s3c64xx/setup-fb.c
 *
 * Copyright 2009 Samsung Electronics
 *	Jinsung Yang <jsgood.yang@samsung.com>
 *	http://samsungsemi.com/
 *
 * Base S3C64XX FIMD gpio configuration
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/io.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-gpio.h>
#include <mach/map.h>

struct platform_device; /* don't need the contents */

void s3cfb_cfg_gpio(struct platform_device *pdev)
{
	void __iomem *hostb_base;
	unsigned int cfg;
	int i;

	/* must be '0' for normal-path instead of bypass */
	hostb_base = ioremap(S3C64XX_PA_HOSTIFB, SZ_1M);
	cfg = readl(hostb_base + S3C64XX_MIFPCON);
	cfg &= ~S3C64XX_MIFPCON_LCD_MUX_MASK;
	cfg |= S3C64XX_MIFPCON_LCD_MUX_NORMAL;
	writel(cfg, hostb_base + S3C64XX_MIFPCON);
	iounmap(hostb_base);

	/* select TFT LCD type (RGB I/F) */
	cfg = readl(S3C64XX_SPC_BASE);
	cfg &= ~S3C64XX_SPCON_LCD_SEL_MASK;
	cfg |= S3C64XX_SPCON_LCD_SEL_RGB;
	writel(cfg, S3C64XX_SPC_BASE);

	for (i = 0; i < 16; i++)
		s3c_gpio_cfgpin(S3C64XX_GPI(i), S3C_GPIO_SFN(2));

	for (i = 0; i < 12; i++)
		s3c_gpio_cfgpin(S3C64XX_GPJ(i), S3C_GPIO_SFN(2));
}

#ifndef CONFIG_BACKLIGHT_PWM
int s3cfb_backlight_on(struct platform_device *pdev)
{
	int err;

	err = gpio_request(S3C64XX_GPF(15), "GPF");
	if (err) {
		printk(KERN_ERR "failed to request GPF for "
			"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(S3C64XX_GPF(15), 1);
	gpio_free(S3C64XX_GPF(15));

	return 0;
}
#else
int s3cfb_backlight_on(struct platform_device *pdev)
{
	return 0;
}
#endif

int s3cfb_reset_lcd(struct platform_device *pdev)
{
	int err;

	err = gpio_request(S3C64XX_GPN(5), "GPN");
	if (err) {
		printk(KERN_ERR "failed to request GPN for "
			"lcd reset control\n");
		return err;
	}

	gpio_direction_output(S3C64XX_GPN(5), 1);

	mdelay(100);

	gpio_set_value(S3C64XX_GPN(5), 0);
	mdelay(10);

	gpio_set_value(S3C64XX_GPN(5), 1);
	mdelay(10);

	gpio_free(S3C64XX_GPN(5));

	return 0;
}

