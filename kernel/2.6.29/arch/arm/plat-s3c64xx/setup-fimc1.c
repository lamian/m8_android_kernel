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

void s3c_fimc1_cfg_gpio(struct platform_device *dev)
{
	/* nothing to do */
}
