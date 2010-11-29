/* linux/arch/arm/plat-s5pc1xx/setup-fimc0.c
 *
 * Copyright 2009 Samsung Electronics
 *	Jinsung Yang <jsgood.yang@samsung.com>
 *	http://samsungsemi.com/
 *
 * Base S5PC1XX FIMC controller 0 gpio configuration
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/gpio-bank-b.h>
#include <plat/gpio-bank-f.h>

struct platform_device; /* don't need the contents */

void s3c_fimc0_cfg_gpio(struct platform_device *dev)
{
	int i;

	s3c_gpio_cfgpin(S3C64XX_GPF(0), S3C64XX_GPF0_CAMIF_CLK);
	s3c_gpio_cfgpin(S3C64XX_GPF(1), S3C64XX_GPF1_CAMIF_HREF);
	s3c_gpio_cfgpin(S3C64XX_GPF(2), S3C64XX_GPF2_CAMIF_PCLK);
	s3c_gpio_cfgpin(S3C64XX_GPF(3), S3C64XX_GPF3_CAMIF_nRST);
	s3c_gpio_cfgpin(S3C64XX_GPF(4), S3C64XX_GPF4_CAMIF_VSYNC);
	s3c_gpio_cfgpin(S3C64XX_GPF(5), S3C64XX_GPF5_CAMIF_YDATA0);
	s3c_gpio_cfgpin(S3C64XX_GPF(6), S3C64XX_GPF6_CAMIF_YDATA1);
	s3c_gpio_cfgpin(S3C64XX_GPF(7), S3C64XX_GPF7_CAMIF_YDATA2);
	s3c_gpio_cfgpin(S3C64XX_GPF(8), S3C64XX_GPF8_CAMIF_YDATA3);
	s3c_gpio_cfgpin(S3C64XX_GPF(9), S3C64XX_GPF9_CAMIF_YDATA4);
	s3c_gpio_cfgpin(S3C64XX_GPF(10), S3C64XX_GPF10_CAMIF_YDATA5);
	s3c_gpio_cfgpin(S3C64XX_GPF(11), S3C64XX_GPF11_CAMIF_YDATA6);
	s3c_gpio_cfgpin(S3C64XX_GPF(12), S3C64XX_GPF12_CAMIF_YDATA7);
	s3c_gpio_cfgpin(S3C64XX_GPB(4), S3C64XX_GPB4_CAM_FIELD);

	for (i = 0; i < 12; i++)
		s3c_gpio_setpull(S3C64XX_GPF(i), S3C_GPIO_PULL_UP);

	s3c_gpio_setpull(S3C64XX_GPB(4), S3C_GPIO_PULL_UP);
}
