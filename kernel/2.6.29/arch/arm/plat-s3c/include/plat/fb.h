/* linux/arch/arm/plat-s3c/include/plat/fb.h
 *
 * Platform header file for Samsung Display Controller (FIMD) driver
 *
 * Jinsung Yang, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _FB_H
#define _FB_H

#define FB_SWAP_WORD	(1 << 24)
#define FB_SWAP_HWORD	(1 << 16)
#define FB_SWAP_BYTE	(1 << 8)
#define FB_SWAP_BIT	(1 << 0)

struct platform_device;

struct s3c_platform_fb {
	int		hw_ver;
	const char	clk_name[16];
	int		nr_wins;
	int		nr_buffers[5];
	int		default_win;
	int		swap;

	void		(*cfg_gpio)(struct platform_device *dev);
	int		(*backlight_on)(struct platform_device *dev);
	int		(*reset_lcd)(struct platform_device *dev);
};

extern void s3cfb_set_platdata(struct s3c_platform_fb *fimd);

/* defined by architecture to configure gpio */
extern void s3cfb_cfg_gpio(struct platform_device *pdev);
extern int s3cfb_backlight_on(struct platform_device *pdev);
extern int s3cfb_reset_lcd(struct platform_device *pdev);

#endif /* _FB_H */

