/*
 * drivers/video/s3c/s3cfb_lte480wv.c
 *
 * $Id: s3cfb_ltv350qv.c,v 1.1 2008/11/17 11:12:08 jsgood Exp $
 *
 * Copyright (C) 2008 Jinsung Yang <jsgood.yang@samsung.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 *	S3C Frame Buffer Driver
 *	based on skeletonfb.c, sa1100fb.h, s3c2410fb.c
 */

#include <linux/wait.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#include <plat/regs-lcd.h>

#include "s3cfb.h"

#if defined(CONFIG_PLAT_S5PC1XX)
#define S3C_FB_SPI_CH		0	/* spi channel for module init */
#else
#define S3C_FB_SPI_CH		1	/* spi channel for module init */
#endif

#define S3C_FB_HFP		3	/* front porch */
#define S3C_FB_HSW		10	/* hsync width */
#define S3C_FB_HBP		5	/* back porch */

#define S3C_FB_VFP		3	/* front porch */
#define S3C_FB_VSW		4	/* vsync width */
#define S3C_FB_VBP		5	/* back porch */

#define S3C_FB_HRES		320	/* horizon pixel  x resolition */
#define S3C_FB_VRES		240	/* line cnt       y resolution */

#define S3C_FB_HRES_VIRTUAL	S3C_FB_HRES     /* horizon pixel  x resolition */
#define S3C_FB_VRES_VIRTUAL	S3C_FB_VRES * 2 /* line cnt       y resolution */

#define S3C_FB_HRES_OSD		320	/* horizon pixel  x resolition */
#define S3C_FB_VRES_OSD		240	/* line cnt       y resolution */

#define S3C_FB_HRES_OSD_VIRTUAL S3C_FB_HRES_OSD	    /* horizon pixel  x resolition */
#define S3C_FB_VRES_OSD_VIRTUAL	S3C_FB_VRES_OSD * 2 /* line cnt       y resolution */

#if defined(CONFIG_PLAT_S3C24XX)
#define S3C_FB_VFRAME_FREQ     	75	/* frame rate freq */
#else
#define S3C_FB_VFRAME_FREQ     	60	/* frame rate freq */
#endif

#define S3C_FB_PIXEL_CLOCK	(S3C_FB_VFRAME_FREQ * (S3C_FB_HFP + S3C_FB_HSW + S3C_FB_HBP + S3C_FB_HRES) * (S3C_FB_VFP + S3C_FB_VSW + S3C_FB_VBP + S3C_FB_VRES))

static void s3cfb_set_fimd_info(void)
{
	s3c_fimd.vidcon1 = S3C_VIDCON1_IHSYNC_INVERT | S3C_VIDCON1_IVSYNC_INVERT | S3C_VIDCON1_IVDEN_NORMAL;
	s3c_fimd.vidtcon0 = S3C_VIDTCON0_VBPD(S3C_FB_VBP - 1) | S3C_VIDTCON0_VFPD(S3C_FB_VFP - 1) | S3C_VIDTCON0_VSPW(S3C_FB_VSW - 1);
	s3c_fimd.vidtcon1 = S3C_VIDTCON1_HBPD(S3C_FB_HBP - 1) | S3C_VIDTCON1_HFPD(S3C_FB_HFP - 1) | S3C_VIDTCON1_HSPW(S3C_FB_HSW - 1);
	s3c_fimd.vidtcon2 = S3C_VIDTCON2_LINEVAL(S3C_FB_VRES - 1) | S3C_VIDTCON2_HOZVAL(S3C_FB_HRES - 1);

	s3c_fimd.vidosd0a = S3C_VIDOSDxA_OSD_LTX_F(0) | S3C_VIDOSDxA_OSD_LTY_F(0);
	s3c_fimd.vidosd0b = S3C_VIDOSDxB_OSD_RBX_F(S3C_FB_HRES - 1) | S3C_VIDOSDxB_OSD_RBY_F(S3C_FB_VRES - 1);

	s3c_fimd.vidosd1a = S3C_VIDOSDxA_OSD_LTX_F(0) | S3C_VIDOSDxA_OSD_LTY_F(0);
	s3c_fimd.vidosd1b = S3C_VIDOSDxB_OSD_RBX_F(S3C_FB_HRES_OSD - 1) | S3C_VIDOSDxB_OSD_RBY_F(S3C_FB_VRES_OSD - 1);

	s3c_fimd.width = S3C_FB_HRES;
	s3c_fimd.height = S3C_FB_VRES;
	s3c_fimd.xres = S3C_FB_HRES;
	s3c_fimd.yres = S3C_FB_VRES;

#if defined(CONFIG_FB_S3C_VIRTUAL_SCREEN)
	s3c_fimd.xres_virtual = S3C_FB_HRES_VIRTUAL;
	s3c_fimd.yres_virtual = S3C_FB_VRES_VIRTUAL;
#else
	s3c_fimd.xres_virtual = S3C_FB_HRES;
	s3c_fimd.yres_virtual = S3C_FB_VRES;
#endif

	s3c_fimd.osd_width = S3C_FB_HRES_OSD;
	s3c_fimd.osd_height = S3C_FB_VRES_OSD;
	s3c_fimd.osd_xres = S3C_FB_HRES_OSD;
	s3c_fimd.osd_yres = S3C_FB_VRES_OSD;

#if defined(CONFIG_FB_S3C_VIRTUAL_SCREEN)
	s3c_fimd.osd_xres_virtual = S3C_FB_HRES_OSD_VIRTUAL;
	s3c_fimd.osd_yres_virtual = S3C_FB_VRES_OSD_VIRTUAL;
#else
	s3c_fimd.osd_xres_virtual = S3C_FB_HRES_OSD;
	s3c_fimd.osd_yres_virtual = S3C_FB_VRES_OSD;
#endif
     	s3c_fimd.pixclock = S3C_FB_PIXEL_CLOCK;

	s3c_fimd.hsync_len = S3C_FB_HSW;
	s3c_fimd.vsync_len = S3C_FB_VSW;
	s3c_fimd.left_margin = S3C_FB_HFP;
	s3c_fimd.upper_margin = S3C_FB_VFP;
	s3c_fimd.right_margin = S3C_FB_HBP;
	s3c_fimd.lower_margin = S3C_FB_VBP;
}

void s3cfb_spi_write(int address, int data)
{
	unsigned int delay = 50;
	unsigned char dev_id = 0x1d;
	int i;

	s3cfb_spi_lcd_den(S3C_FB_SPI_CH, 1);
	s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 1);
	s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 1);
	udelay(delay);

	s3cfb_spi_lcd_den(S3C_FB_SPI_CH, 0);
	udelay(delay);

	for (i = 5; i >= 0; i--) {
		s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 0);

		if ((dev_id >> i) & 0x1)
			s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 1);
		else
			s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 0);

		udelay(delay);

		s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 1);
		udelay(delay);
	}

	s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 0);
	s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 0);
	udelay(delay);

	s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 1);
	udelay(delay);

	s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 0);
	s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 0);
	udelay(delay);

	s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 1);
	udelay(delay);

	for (i = 15; i >= 0; i--) {
		s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 0);

		if ((address >> i) & 0x1)
			s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 1);
		else
			s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 0);

		udelay(delay);

		s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 1);
		udelay(delay);
	}

	s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 1);
	udelay(delay);

	s3cfb_spi_lcd_den(S3C_FB_SPI_CH, 1);
	udelay(delay * 10);

	s3cfb_spi_lcd_den(S3C_FB_SPI_CH, 0);
	udelay(delay);

	for (i = 5; i >= 0; i--) {
		s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 0);

		if ((dev_id >> i) & 0x1)
			s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 1);
		else
			s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 0);

		udelay(delay);

		s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 1);
		udelay(delay);

	}

	s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 0);
	s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 1);
	udelay(delay);

	s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 1);
	udelay(delay);

	s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 0);
	s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 0);
	udelay(delay);

	s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 1);
	udelay(delay);

	for (i = 15; i >= 0; i--) {
		s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 0);

		if ((data >> i) & 0x1)
			s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 1);
		else
			s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 0);

		udelay(delay);

		s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 1);
		udelay(delay);

	}

	s3cfb_spi_lcd_den(S3C_FB_SPI_CH, 1);
	udelay(delay);
}

static void s3cfb_init_ldi(void)
{
	s3cfb_spi_set_lcd_data(S3C_FB_SPI_CH);
	mdelay(5);

	s3cfb_spi_lcd_den(S3C_FB_SPI_CH, 1);
	s3cfb_spi_lcd_dclk(S3C_FB_SPI_CH, 1);
	s3cfb_spi_lcd_dseri(S3C_FB_SPI_CH, 1);

	s3cfb_spi_write(0x01, 0x001d);
	s3cfb_spi_write(0x02, 0x0000);
    	s3cfb_spi_write(0x03, 0x0000);
    	s3cfb_spi_write(0x04, 0x0000);
    	s3cfb_spi_write(0x05, 0x50a3);
    	s3cfb_spi_write(0x06, 0x0000);
    	s3cfb_spi_write(0x07, 0x0000);
    	s3cfb_spi_write(0x08, 0x0000);
   	s3cfb_spi_write(0x09, 0x0000);
   	s3cfb_spi_write(0x0a, 0x0000);
   	s3cfb_spi_write(0x10, 0x0000);
   	s3cfb_spi_write(0x11, 0x0000);
   	s3cfb_spi_write(0x12, 0x0000);
   	s3cfb_spi_write(0x13, 0x0000);
   	s3cfb_spi_write(0x14, 0x0000);
   	s3cfb_spi_write(0x15, 0x0000);
   	s3cfb_spi_write(0x16, 0x0000);
   	s3cfb_spi_write(0x17, 0x0000);
   	s3cfb_spi_write(0x18, 0x0000);
   	s3cfb_spi_write(0x19, 0x0000);

	mdelay(10);

	s3cfb_spi_write(0x09, 0x4055);
	s3cfb_spi_write(0x0a, 0x0000);

	mdelay(10);

	s3cfb_spi_write(0x0a, 0x2000);

	mdelay(50);

	s3cfb_spi_write(0x01, 0x409d);
	s3cfb_spi_write(0x02, 0x0204);
	s3cfb_spi_write(0x03, 0x2100);
	s3cfb_spi_write(0x04, 0x1000);
	s3cfb_spi_write(0x05, 0x5003);
	s3cfb_spi_write(0x06, 0x0009);
	s3cfb_spi_write(0x07, 0x000f);
	s3cfb_spi_write(0x08, 0x0800);
	s3cfb_spi_write(0x10, 0x0000);
	s3cfb_spi_write(0x11, 0x0000);
	s3cfb_spi_write(0x12, 0x000f);
	s3cfb_spi_write(0x13, 0x1f00);
	s3cfb_spi_write(0x14, 0x0000);
	s3cfb_spi_write(0x15, 0x0000);
	s3cfb_spi_write(0x16, 0x0000);
	s3cfb_spi_write(0x17, 0x0000);
	s3cfb_spi_write(0x18, 0x0000);
	s3cfb_spi_write(0x19, 0x0000);

	mdelay(50);

	s3cfb_spi_write(0x09, 0x4a55);
	s3cfb_spi_write(0x0a, 0x2000);
}

void s3cfb_init_hw(void)
{
	printk(KERN_INFO "LCD TYPE :: LTV350QV will be initialized\n");

	s3cfb_set_fimd_info();
	s3cfb_set_gpio();

	if (s3cfb_spi_gpio_request(S3C_FB_SPI_CH))
		printk(KERN_ERR "failed to request GPIO for spi-lcd\n");
	else {
		s3cfb_init_ldi();
		s3cfb_spi_gpio_free(S3C_FB_SPI_CH);
	}
}

