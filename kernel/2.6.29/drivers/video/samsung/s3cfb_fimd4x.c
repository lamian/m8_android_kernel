/*
 * drivers/video/samsung/s3cfb_fimd4x.c
 *
 * $Id: s3cfb_fimd4x.c,v 1.2 2008/11/17 23:44:28 jsgood Exp $
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/string.h>
#include <linux/ioctl.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/earlysuspend.h>

#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <plat/gpio-cfg.h>
#include <plat/regs-clock.h>
#include <plat/regs-lcd.h>
#include <plat/regs-gpio.h>

#include <mach/map.h>

#ifdef CONFIG_S3C64XX_DOMAIN_GATING
//#define USE_LCD_DOMAIN_GATING
#endif /* CONFIG_S3C64XX_DOMAIN_GATING */

#if defined(CONFIG_PM)
#include <plat/pm.h>
#include <plat/power-clock-domain.h>
#endif

#include "s3cfb.h"

s3c_fimd_info_t s3c_fimd = {
	.vidcon0 = S3C_VIDCON0_INTERLACE_F_PROGRESSIVE | S3C_VIDCON0_VIDOUT_RGB_IF | S3C_VIDCON0_L1_DATA16_SUB_16_MODE | \
	           S3C_VIDCON0_L0_DATA16_MAIN_16_MODE | S3C_VIDCON0_PNRMODE_RGB_P | \
	           S3C_VIDCON0_CLKVALUP_ALWAYS | S3C_VIDCON0_CLKDIR_DIVIDED | S3C_VIDCON0_CLKSEL(1) | \
	           S3C_VIDCON0_ENVID_DISABLE | S3C_VIDCON0_ENVID_F_DISABLE,

	.dithmode = (S3C_DITHMODE_RDITHPOS_5BIT | S3C_DITHMODE_GDITHPOS_6BIT | S3C_DITHMODE_BDITHPOS_5BIT ) & S3C_DITHMODE_DITHERING_DISABLE,

#if defined (CONFIG_FB_S3C_BPP_8)
	.wincon0 =  S3C_WINCONx_BYTSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD | S3C_WINCONx_BPPMODE_F_8BPP_PAL,
	.wincon1 =  S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD | S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1,
	.bpp = S3C_FB_PIXEL_BPP_8,
	.bytes_per_pixel = 1,
	.wpalcon = S3C_WPALCON_W0PAL_16BIT,

#elif defined (CONFIG_FB_S3C_BPP_16)
	.wincon0 = S3C_WINCONx_ENLOCAL_DMA | S3C_WINCONx_BUFSEL_0 | S3C_WINCONx_BUFAUTOEN_DISABLE | \
	           S3C_WINCONx_BITSWP_DISABLE | S3C_WINCONx_BYTSWP_DISABLE | S3C_WINCONx_HAWSWP_ENABLE | \
	           S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_ENWIN_F_DISABLE,

	.wincon1 = S3C_WINCONx_ENLOCAL_DMA | S3C_WINCONx_BUFSEL_0 | S3C_WINCONx_BUFAUTOEN_DISABLE | \
	           S3C_WINCONx_BITSWP_DISABLE | S3C_WINCONx_BYTSWP_DISABLE | S3C_WINCONx_HAWSWP_ENABLE | \
	           S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_BPPMODE_F_16BPP_565 | \
	           S3C_WINCONx_ALPHA_SEL_1 | S3C_WINCONx_ENWIN_F_DISABLE,

	.wincon2 = S3C_WINCONx_ENLOCAL_DMA | S3C_WINCONx_BITSWP_DISABLE | S3C_WINCONx_BYTSWP_DISABLE | \
	           S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_4WORD | S3C_WINCONx_BURSTLEN_16WORD | \
	           S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_ALPHA_SEL_1 | S3C_WINCONx_ENWIN_F_DISABLE,

	.wincon3 = S3C_WINCONx_BITSWP_DISABLE | S3C_WINCONx_BYTSWP_DISABLE | S3C_WINCONx_HAWSWP_ENABLE | \
	           S3C_WINCONx_BURSTLEN_4WORD | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BLD_PIX_PLANE | \
	           S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_ALPHA_SEL_1 | S3C_WINCONx_ENWIN_F_DISABLE,

	.wincon4 = S3C_WINCONx_BITSWP_DISABLE | S3C_WINCONx_BYTSWP_DISABLE | S3C_WINCONx_HAWSWP_ENABLE | \
	           S3C_WINCONx_BURSTLEN_4WORD | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BLD_PIX_PLANE |
	           S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_ALPHA_SEL_1 | S3C_WINCONx_ENWIN_F_DISABLE,

	.bpp = S3C_FB_PIXEL_BPP_16,
	.bytes_per_pixel = 2,
	.wpalcon = S3C_WPALCON_W0PAL_16BIT,

#elif defined (CONFIG_FB_S3C_BPP_24)
	.wincon0 = S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_24BPP_888,
	.wincon1 = S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1,
	.wincon2 = S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1,
	.wincon3 = S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1,
	.wincon4 = S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1,
	.bpp = S3C_FB_PIXEL_BPP_24,
	.bytes_per_pixel = 4,
	.wpalcon = S3C_WPALCON_W0PAL_24BIT,
#endif

	.vidosd1c = S3C_VIDOSDxC_ALPHA1_B(S3C_FB_MAX_ALPHA_LEVEL) | S3C_VIDOSDxC_ALPHA1_G(S3C_FB_MAX_ALPHA_LEVEL) | S3C_VIDOSDxC_ALPHA1_R(S3C_FB_MAX_ALPHA_LEVEL),
	.vidosd2c = S3C_VIDOSDxC_ALPHA1_B(S3C_FB_MAX_ALPHA_LEVEL) | S3C_VIDOSDxC_ALPHA1_G(S3C_FB_MAX_ALPHA_LEVEL) | S3C_VIDOSDxC_ALPHA1_R(S3C_FB_MAX_ALPHA_LEVEL),
	.vidosd3c = S3C_VIDOSDxC_ALPHA1_B(S3C_FB_MAX_ALPHA_LEVEL) | S3C_VIDOSDxC_ALPHA1_G(S3C_FB_MAX_ALPHA_LEVEL) | S3C_VIDOSDxC_ALPHA1_R(S3C_FB_MAX_ALPHA_LEVEL),
	.vidosd4c = S3C_VIDOSDxC_ALPHA1_B(S3C_FB_MAX_ALPHA_LEVEL) | S3C_VIDOSDxC_ALPHA1_G(S3C_FB_MAX_ALPHA_LEVEL) | S3C_VIDOSDxC_ALPHA1_R(S3C_FB_MAX_ALPHA_LEVEL),

	.vidintcon0 = S3C_VIDINTCON0_FRAMESEL0_VSYNC | S3C_VIDINTCON0_FRAMESEL1_NONE | S3C_VIDINTCON0_INTFRMEN_DISABLE | \
	              S3C_VIDINTCON0_FIFOSEL_WIN0 | S3C_VIDINTCON0_FIFOLEVEL_25 | S3C_VIDINTCON0_INTFIFOEN_DISABLE | S3C_VIDINTCON0_INTEN_ENABLE,
	.vidintcon1 = 0,

	.xoffset = 0,
	.yoffset = 0,

	.w1keycon0 = S3C_WxKEYCON0_KEYBLEN_DISABLE | S3C_WxKEYCON0_KEYEN_F_DISABLE | S3C_WxKEYCON0_DIRCON_MATCH_FG_IMAGE | S3C_WxKEYCON0_COMPKEY(0x0),
	.w1keycon1 = S3C_WxKEYCON1_COLVAL(0xffffff),
	.w2keycon0 = S3C_WxKEYCON0_KEYBLEN_DISABLE | S3C_WxKEYCON0_KEYEN_F_DISABLE | S3C_WxKEYCON0_DIRCON_MATCH_FG_IMAGE | S3C_WxKEYCON0_COMPKEY(0x0),
	.w2keycon1 = S3C_WxKEYCON1_COLVAL(0xffffff),
	.w3keycon0 = S3C_WxKEYCON0_KEYBLEN_DISABLE | S3C_WxKEYCON0_KEYEN_F_DISABLE | S3C_WxKEYCON0_DIRCON_MATCH_FG_IMAGE | S3C_WxKEYCON0_COMPKEY(0x0),
	.w3keycon1 = S3C_WxKEYCON1_COLVAL(0xffffff),
	.w4keycon0 = S3C_WxKEYCON0_KEYBLEN_DISABLE | S3C_WxKEYCON0_KEYEN_F_DISABLE | S3C_WxKEYCON0_DIRCON_MATCH_FG_IMAGE | S3C_WxKEYCON0_COMPKEY(0x0),
	.w4keycon1 = S3C_WxKEYCON1_COLVAL(0xffffff),

	.sync = 0,
	.cmap_static = 1,

	.vs_offset = S3C_FB_DEFAULT_DISPLAY_OFFSET,
	.brightness = S3C_FB_DEFAULT_BRIGHTNESS,
	.backlight_level = S3C_FB_DEFAULT_BACKLIGHT_LEVEL,
	.backlight_power = 1,
	.lcd_power = 1,
};

#if  defined(CONFIG_S3C6410_PWM)
void s3cfb_set_brightness(int val)
{
	int channel = 1;	/* must use channel-1 */
	int usec = 0;		/* don't care value */
	unsigned long tcnt = 1000;
	unsigned long tcmp = 0;

	if (val < 0)
		val = 0;

	if (val > S3C_FB_MAX_BRIGHTNESS)
		val = S3C_FB_MAX_BRIGHTNESS;

	s3c_fimd.brightness = val;
	tcmp = val * 5;

	s3c6410_timer_setup (channel, usec, tcnt, tcmp);
}
#endif

#if defined(CONFIG_FB_S3C_DOUBLE_BUFFERING)

static void s3cfb_change_buff(int req_win, int req_fb)
{
	switch (req_win) {
	case 0:
		if (req_fb == 0)
			s3c_fimd.wincon0 &= ~S3C_WINCONx_BUFSEL_MASK;
		else
			s3c_fimd.wincon0 |= S3C_WINCONx_BUFSEL_1;

		writel(s3c_fimd.wincon0 | S3C_WINCONx_ENWIN_F_ENABLE, S3C_WINCON0);
		break;

	case 1:
		if (req_fb == 0)
			s3c_fimd.wincon1 &= ~S3C_WINCONx_BUFSEL_MASK;
		else
			s3c_fimd.wincon1 |= S3C_WINCONx_BUFSEL_1;

		writel(s3c_fimd.wincon1 | S3C_WINCONx_ENWIN_F_ENABLE, S3C_WINCON1);
		break;

	default:
		break;
	}
}

#endif

#if defined(CONFIG_FB_S3C_VIRTUAL_SCREEN)

static int s3cfb_vs_start(s3c_fb_info_t *fbi)
{
	switch(fbi->win_id)
	{
		case 0 :
			s3c_fimd.wincon0 &= ~(S3C_WINCONx_ENWIN_F_ENABLE);
			writel(s3c_fimd.wincon0 | S3C_WINCONx_ENWIN_F_ENABLE, S3C_WINCON0);
			break;
		case 1 :
			s3c_fimd.wincon1 &= ~(S3C_WINCONx_ENWIN_F_ENABLE);
			writel(s3c_fimd.wincon1 | S3C_WINCONx_ENWIN_F_ENABLE, S3C_WINCON1);
			break;
		case 2 :
			s3c_fimd.wincon2 &= ~(S3C_WINCONx_ENWIN_F_ENABLE);
			writel(s3c_fimd.wincon2 | S3C_WINCONx_ENWIN_F_ENABLE, S3C_WINCON2);
			break;
		case 3 :
			s3c_fimd.wincon3 &= ~(S3C_WINCONx_ENWIN_F_ENABLE);
			writel(s3c_fimd.wincon3 | S3C_WINCONx_ENWIN_F_ENABLE, S3C_WINCON3);
			break;
		case 4 :
			s3c_fimd.wincon4 &= ~(S3C_WINCONx_ENWIN_F_ENABLE);
			writel(s3c_fimd.wincon4 | S3C_WINCONx_ENWIN_F_ENABLE, S3C_WINCON4);
			break;
		default : 
			printk("%s::invalid win_num(%d)\n", __func__, fbi->win_id);
			return -EINVAL;
	}

	fbi->fb.var.xoffset = s3c_fimd.xoffset;
	fbi->fb.var.yoffset = s3c_fimd.yoffset;
	
	return 0;
}

static int s3cfb_vs_stop(s3c_fb_info_t *fbi)
{
	fbi->fb.var.xoffset = 0;
	fbi->fb.var.yoffset = 0;

	switch(fbi->win_id)
	{
		case 0 :
			s3c_fimd.vidw00add0b0 = fbi->screen_dma_f1;
			s3c_fimd.vidw00add0b1 = fbi->screen_dma_f2;
			writel(s3c_fimd.vidw00add0b0, S3C_VIDW00ADD0B0);
			writel(s3c_fimd.vidw00add0b1, S3C_VIDW00ADD0B1);
			break;
		case 1 :
			s3c_fimd.vidw01add0b0 = fbi->screen_dma_f1;
			s3c_fimd.vidw01add0b1 = fbi->screen_dma_f2;
			writel(s3c_fimd.vidw01add0b0, S3C_VIDW01ADD0B0);
			writel(s3c_fimd.vidw01add0b1, S3C_VIDW01ADD0B1);
			break;
		case 2 :
			s3c_fimd.vidw02add0 = fbi->screen_dma_f1;
			s3c_fimd.vidw02add1 = fbi->screen_dma_f2;
			writel(s3c_fimd.vidw02add0, S3C_VIDW02ADD0);
			writel(s3c_fimd.vidw02add1, S3C_VIDW02ADD0);
			break;
		case 3 :
			s3c_fimd.vidw03add0 = fbi->screen_dma_f1;
			s3c_fimd.vidw03add1 = fbi->screen_dma_f2;
			writel(s3c_fimd.vidw03add0, S3C_VIDW03ADD0);
			writel(s3c_fimd.vidw03add1, S3C_VIDW03ADD1);
			break;
		case 4 :
			s3c_fimd.vidw04add0 = fbi->screen_dma_f1;
			s3c_fimd.vidw04add1 = fbi->screen_dma_f2;
			writel(s3c_fimd.vidw04add0, S3C_VIDW04ADD0);
			writel(s3c_fimd.vidw04add1, S3C_VIDW04ADD1);
			break;
		default : 
			printk("%s::invalid fbi->win_id(%d)\n", __func__, fbi->win_id);
			return -EINVAL;
	}
	
	return 0;
}

static int s3cfb_vs_set(s3c_fb_info_t *fbi)
{
	int page_width;
	int offset;
	unsigned int frame_size;
	unsigned int start;

	page_width =  s3c_fimd.xres * s3c_fimd.bytes_per_pixel;
	offset     = (s3c_fimd.xres_virtual - s3c_fimd.xres) * s3c_fimd.bytes_per_pixel;
	frame_size = (page_width + offset) * (s3c_fimd.yres);
	start      = fbi->fb.fix.line_length * fbi->fb.var.yoffset;

	switch(fbi->win_id)
	{
		case 0 :
			// size
			s3c_fimd.vidw00add2 = S3C_VIDWxxADD2_OFFSIZE_F(offset) | S3C_VIDWxxADD2_PAGEWIDTH_F(page_width);
			writel(s3c_fimd.vidw00add2, S3C_VIDW00ADD2);
			
			// start address
			s3c_fimd.vidw00add0b0 = fbi->screen_dma_f1 + start;
			s3c_fimd.vidw00add0b1 = fbi->screen_dma_f2 + start;
			writel(s3c_fimd.vidw00add0b0, S3C_VIDW00ADD0B0);
			writel(s3c_fimd.vidw00add0b1, S3C_VIDW00ADD0B1);

			// end address
			s3c_fimd.vidw00add1b0 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw00add0b0 + frame_size);
			s3c_fimd.vidw00add1b1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw00add0b1 + frame_size);
			writel(s3c_fimd.vidw00add1b0, S3C_VIDW00ADD1B0);
			writel(s3c_fimd.vidw00add1b1, S3C_VIDW00ADD1B1);
			break;

		case 1 :
			// size
			s3c_fimd.vidw01add2 = S3C_VIDWxxADD2_OFFSIZE_F(offset) | S3C_VIDWxxADD2_PAGEWIDTH_F(page_width);
			writel(s3c_fimd.vidw01add2, S3C_VIDW01ADD2);
			
			// start address
			s3c_fimd.vidw01add0b0 = fbi->screen_dma_f1 + start;
			s3c_fimd.vidw01add0b1 = fbi->screen_dma_f2 + start;
			writel(s3c_fimd.vidw01add0b0, S3C_VIDW01ADD0B0);
			writel(s3c_fimd.vidw01add0b1, S3C_VIDW01ADD0B1);

			// end address
			s3c_fimd.vidw01add1b0 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw01add0b0 + frame_size);
			s3c_fimd.vidw01add1b1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw01add0b1 + frame_size);
			writel(s3c_fimd.vidw01add1b0, S3C_VIDW01ADD1B0);
			writel(s3c_fimd.vidw01add1b1, S3C_VIDW01ADD1B1);
			break;
			
		case 2 :
			// size
			s3c_fimd.vidw02add2 = S3C_VIDWxxADD2_OFFSIZE_F(offset) | S3C_VIDWxxADD2_PAGEWIDTH_F(page_width);
			writel(s3c_fimd.vidw02add2, S3C_VIDW02ADD2);
			
			// start address
			s3c_fimd.vidw02add0 = fbi->screen_dma_f1 + start;
			writel(s3c_fimd.vidw02add0, S3C_VIDW02ADD0);
			
			// end address
			s3c_fimd.vidw02add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw02add0 + frame_size);
			writel(s3c_fimd.vidw02add1, S3C_VIDW02ADD1);
			break;

		case 3 :
			// size
			s3c_fimd.vidw03add2 = S3C_VIDWxxADD2_OFFSIZE_F(offset) | S3C_VIDWxxADD2_PAGEWIDTH_F(page_width);
			writel(s3c_fimd.vidw03add2, S3C_VIDW02ADD2);
			
			// start address
			s3c_fimd.vidw03add0 = fbi->screen_dma_f1 + start;
			writel(s3c_fimd.vidw03add0, S3C_VIDW03ADD0);
			
			// end address
			s3c_fimd.vidw03add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw03add0 + frame_size);
			writel(s3c_fimd.vidw03add1, S3C_VIDW03ADD1);
			break;
		case 4 :
			// size
			s3c_fimd.vidw04add2 = S3C_VIDWxxADD2_OFFSIZE_F(offset) | S3C_VIDWxxADD2_PAGEWIDTH_F(page_width);
			writel(s3c_fimd.vidw04add2, S3C_VIDW04ADD2);
			
			// start address
			s3c_fimd.vidw04add0 = fbi->screen_dma_f1 + start;
			writel(s3c_fimd.vidw04add0, S3C_VIDW04ADD0);
			
			// end address
			s3c_fimd.vidw02add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw04add0 + frame_size);
			writel(s3c_fimd.vidw04add1, S3C_VIDW04ADD1);
			break;

		default :
			printk("%s::invalid fbi->win_id(%d)\n", __func__, fbi->win_id);
			return -EINVAL;
			break;
	}

	return 0;
}

static int s3cfb_vs_move_left(s3c_fb_info_t *fbi)
{
	int page_width;
	int offset;
	int shift_value;
	unsigned int shift_pixel;
	unsigned int frame_size;

	page_width  =  s3c_fimd.xres * s3c_fimd.bytes_per_pixel;
	offset      = (s3c_fimd.xres_virtual - s3c_fimd.xres) * s3c_fimd.bytes_per_pixel;
	
	// get shift 
	if (s3c_fimd.xoffset < s3c_fimd.vs_offset)
		shift_value = s3c_fimd.xoffset;
	else
		shift_value = s3c_fimd.vs_offset;

	s3c_fimd.xoffset -= shift_value;	
	
	shift_pixel = s3c_fimd.bytes_per_pixel * shift_value;
	frame_size  = (page_width + offset) * (s3c_fimd.yres);
	
	switch(fbi->win_id)
	{
		case 0 :
			// start address
			s3c_fimd.vidw00add0b0 = s3c_fimd.vidw00add0b0 - shift_pixel;
			s3c_fimd.vidw00add0b1 = s3c_fimd.vidw00add0b1 - shift_pixel;			
			writel(s3c_fimd.vidw00add0b0, S3C_VIDW00ADD0B0);
			writel(s3c_fimd.vidw00add0b1, S3C_VIDW00ADD0B1);

			// end address
			s3c_fimd.vidw00add1b0 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw00add0b0 + frame_size);
			s3c_fimd.vidw00add1b1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw00add0b1 + frame_size);
			writel(s3c_fimd.vidw00add1b0, S3C_VIDW00ADD1B0);
			writel(s3c_fimd.vidw00add1b1, S3C_VIDW00ADD1B1);
			break;

		case 1 :
			// start address
			s3c_fimd.vidw01add0b0 = s3c_fimd.vidw01add0b0 - shift_pixel;
			s3c_fimd.vidw01add0b1 = s3c_fimd.vidw01add0b1 - shift_pixel;			
			writel(s3c_fimd.vidw01add0b0, S3C_VIDW01ADD0B0);
			writel(s3c_fimd.vidw01add0b1, S3C_VIDW01ADD0B1);

			// end address
			s3c_fimd.vidw01add1b0 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw01add0b0 + frame_size);
			s3c_fimd.vidw01add1b1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw01add0b1 + frame_size);
			writel(s3c_fimd.vidw01add1b0, S3C_VIDW01ADD1B0);
			writel(s3c_fimd.vidw01add1b1, S3C_VIDW01ADD1B1);
			break;
			
		case 2 :		
			// start address
			s3c_fimd.vidw02add0 = s3c_fimd.vidw02add0 - shift_pixel;
			writel(s3c_fimd.vidw02add0, S3C_VIDW02ADD0);
			
			// end address
			s3c_fimd.vidw02add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw02add0 + frame_size);
			writel(s3c_fimd.vidw02add1, S3C_VIDW02ADD1);
			break;

		case 3 :
			// start address
			s3c_fimd.vidw03add0 = s3c_fimd.vidw03add0 - shift_pixel;
			writel(s3c_fimd.vidw03add0, S3C_VIDW03ADD0);
			
			// end address
			s3c_fimd.vidw03add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw03add0 + frame_size);
			writel(s3c_fimd.vidw03add1, S3C_VIDW03ADD1);
			break;

		case 4 :
			// start address
			s3c_fimd.vidw04add0 = s3c_fimd.vidw04add0 - shift_pixel;
			writel(s3c_fimd.vidw04add0, S3C_VIDW04ADD0);

			// end address
			s3c_fimd.vidw04add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw04add0 + frame_size);
			writel(s3c_fimd.vidw04add1, S3C_VIDW04ADD1);
			break;

		default :
			printk("%s::invalid fbi->win_id(%d)\n", __func__, fbi->win_id);
			return -EINVAL;
			break;
	}
	return 0;
}

static int s3cfb_vs_move_right(s3c_fb_info_t *fbi)
{
	int page_width;
	int offset;
	int shift_value;
	unsigned int shift_pixel;
	unsigned int frame_size;

	page_width  =  s3c_fimd.xres * s3c_fimd.bytes_per_pixel;
	offset      = (s3c_fimd.xres_virtual - s3c_fimd.xres) * s3c_fimd.bytes_per_pixel;

	if ((s3c_fimd.vs_info.v_width - (s3c_fimd.xoffset + s3c_fimd.vs_info.width)) < (s3c_fimd.vs_offset))
		shift_value = s3c_fimd.vs_info.v_width - (s3c_fimd.xoffset + s3c_fimd.vs_info.width);
	else
		shift_value = s3c_fimd.vs_offset;

	s3c_fimd.xoffset += shift_value;

	shift_pixel = s3c_fimd.bytes_per_pixel * shift_value;
	frame_size  = (page_width + offset) * (s3c_fimd.yres);
	
	switch(fbi->win_id)
	{
		case 0 :
			// start address
			s3c_fimd.vidw00add0b0 = s3c_fimd.vidw00add0b0 + shift_pixel;
			s3c_fimd.vidw00add0b1 = s3c_fimd.vidw00add0b1 + shift_pixel;			
			writel(s3c_fimd.vidw00add0b0, S3C_VIDW00ADD0B0);
			writel(s3c_fimd.vidw00add0b1, S3C_VIDW00ADD0B1);

			// end address
			s3c_fimd.vidw00add1b0 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw00add0b0 + frame_size);
			s3c_fimd.vidw00add1b1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw00add0b1 + frame_size);
			writel(s3c_fimd.vidw00add1b0, S3C_VIDW00ADD1B0);
			writel(s3c_fimd.vidw00add1b1, S3C_VIDW00ADD1B1);
			break;

		case 1 :
			// start address
			s3c_fimd.vidw01add0b0 = s3c_fimd.vidw01add0b0 + shift_pixel;
			s3c_fimd.vidw01add0b1 = s3c_fimd.vidw01add0b1 + shift_pixel;			
			writel(s3c_fimd.vidw01add0b0, S3C_VIDW01ADD0B0);
			writel(s3c_fimd.vidw01add0b1, S3C_VIDW01ADD0B1);

			// end address
			s3c_fimd.vidw01add1b0 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw01add0b0 + frame_size);
			s3c_fimd.vidw01add1b1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw01add0b1 + frame_size);
			writel(s3c_fimd.vidw01add1b0, S3C_VIDW01ADD1B0);
			writel(s3c_fimd.vidw01add1b1, S3C_VIDW01ADD1B1);
			break;
			
		case 2 :		
			// start address
			s3c_fimd.vidw02add0 = s3c_fimd.vidw02add0 + shift_pixel;
			writel(s3c_fimd.vidw02add0, S3C_VIDW02ADD0);

			// end address
			s3c_fimd.vidw02add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw02add0 + frame_size);
			writel(s3c_fimd.vidw02add1, S3C_VIDW02ADD1);
			break;

		case 3 :
			// start address
			s3c_fimd.vidw03add0 = s3c_fimd.vidw03add0 + shift_pixel;
			writel(s3c_fimd.vidw03add0, S3C_VIDW03ADD0);

			// end address
			s3c_fimd.vidw03add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw03add0 + frame_size);
			writel(s3c_fimd.vidw03add1, S3C_VIDW03ADD1);
			break;

		case 4 :
			// start address
			s3c_fimd.vidw04add0 = s3c_fimd.vidw04add0 + shift_pixel;
			writel(s3c_fimd.vidw04add0, S3C_VIDW04ADD0);
			
			// end address
			s3c_fimd.vidw04add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw04add0 + frame_size);
			writel(s3c_fimd.vidw04add1, S3C_VIDW04ADD1);
			break;

		default :
			printk("%s::invalid fbi->win_id(%d)\n", __func__, fbi->win_id);
			return -EINVAL;
			break;
	}
	return 0;
}

static int s3cfb_vs_move_up(s3c_fb_info_t *fbi)
{
	int page_width;
	int offset;
	int shift_value;
	unsigned int shift_pixel;
	unsigned int frame_size;

	page_width  =  s3c_fimd.xres * s3c_fimd.bytes_per_pixel;
	offset      = (s3c_fimd.xres_virtual - s3c_fimd.xres) * s3c_fimd.bytes_per_pixel;

	// get shift 
	if (s3c_fimd.yoffset < s3c_fimd.vs_offset)
		shift_value = s3c_fimd.yoffset;
	else
		shift_value = s3c_fimd.vs_offset;

	s3c_fimd.yoffset -= shift_value;
	
	shift_pixel = (s3c_fimd.xres_virtual * s3c_fimd.bytes_per_pixel * shift_value);
	frame_size  = (page_width + offset) * (s3c_fimd.yres);
	
	switch(fbi->win_id)
	{
		case 0 :
			// start address
			s3c_fimd.vidw00add0b0 = s3c_fimd.vidw00add0b0 - shift_pixel;
			s3c_fimd.vidw00add0b1 = s3c_fimd.vidw00add0b1 - shift_pixel;			
			writel(s3c_fimd.vidw00add0b0, S3C_VIDW00ADD0B0);
			writel(s3c_fimd.vidw00add0b1, S3C_VIDW00ADD0B1);

			// end address
			s3c_fimd.vidw00add1b0 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw00add0b0 + frame_size);
			s3c_fimd.vidw00add1b1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw00add0b1 + frame_size);
			writel(s3c_fimd.vidw00add1b0, S3C_VIDW00ADD1B0);
			writel(s3c_fimd.vidw00add1b1, S3C_VIDW00ADD1B1);
			break;

		case 1 :
			// start address
			s3c_fimd.vidw01add0b0 = s3c_fimd.vidw01add0b0 - shift_pixel;
			s3c_fimd.vidw01add0b1 = s3c_fimd.vidw01add0b1 - shift_pixel;			
			writel(s3c_fimd.vidw01add0b0, S3C_VIDW01ADD0B0);
			writel(s3c_fimd.vidw01add0b1, S3C_VIDW01ADD0B1);

			// end address
			s3c_fimd.vidw01add1b0 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw01add0b0 + frame_size);
			s3c_fimd.vidw01add1b1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw01add0b1 + frame_size);
			writel(s3c_fimd.vidw01add1b0, S3C_VIDW01ADD1B0);
			writel(s3c_fimd.vidw01add1b1, S3C_VIDW01ADD1B1);
			break;
			
		case 2 :		
			// start address
			s3c_fimd.vidw02add0 = s3c_fimd.vidw02add0 - shift_pixel;
			writel(s3c_fimd.vidw02add0, S3C_VIDW02ADD0);
			
			// end address
			s3c_fimd.vidw02add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw02add0 + frame_size);
			writel(s3c_fimd.vidw02add1, S3C_VIDW02ADD1);
			break;

		case 3 :
			// start address
			s3c_fimd.vidw03add0 = s3c_fimd.vidw03add0 - shift_pixel;
			writel(s3c_fimd.vidw03add0, S3C_VIDW03ADD0);

			// end address
			s3c_fimd.vidw03add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw03add0 + frame_size);
			writel(s3c_fimd.vidw03add1, S3C_VIDW03ADD1);
			break;

		case 4 :
			// start address
			s3c_fimd.vidw04add0 = s3c_fimd.vidw04add0 - shift_pixel;
			writel(s3c_fimd.vidw04add0, S3C_VIDW04ADD0);
			
			// end address
			s3c_fimd.vidw04add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw04add0 + frame_size);
			writel(s3c_fimd.vidw04add1, S3C_VIDW04ADD1);
			break;

		default :
			printk("%s::invalid fbi->win_id(%d)\n", __func__, fbi->win_id);
			return -EINVAL;
			break;
	}
	return 0;
}


static int s3cfb_vs_move_down(s3c_fb_info_t *fbi)
{
	int page_width;
	int offset;
	int shift_value;
	unsigned int shift_pixel;
	unsigned int frame_size;

	page_width  =  s3c_fimd.xres * s3c_fimd.bytes_per_pixel;
	offset      = (s3c_fimd.xres_virtual - s3c_fimd.xres) * s3c_fimd.bytes_per_pixel;

	if (s3c_fimd.yoffset < s3c_fimd.vs_offset)
		shift_value = s3c_fimd.yoffset;
	else
		shift_value = s3c_fimd.vs_offset;

	s3c_fimd.yoffset -= shift_value;

	shift_pixel = (s3c_fimd.xres_virtual * s3c_fimd.bytes_per_pixel * shift_value);
	frame_size  = (page_width + offset) * (s3c_fimd.yres);
	
	switch(fbi->win_id)
	{
		case 0 :
			// start address
			s3c_fimd.vidw00add0b0 = s3c_fimd.vidw00add0b0 + shift_pixel;
			s3c_fimd.vidw00add0b1 = s3c_fimd.vidw00add0b1 + shift_pixel;			
			writel(s3c_fimd.vidw00add0b0, S3C_VIDW00ADD0B0);
			writel(s3c_fimd.vidw00add0b1, S3C_VIDW00ADD0B1);

			// end address
			s3c_fimd.vidw00add1b0 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw00add0b0 + frame_size);
			s3c_fimd.vidw00add1b1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw00add0b1 + frame_size);
			writel(s3c_fimd.vidw00add1b0, S3C_VIDW00ADD1B0);
			writel(s3c_fimd.vidw00add1b1, S3C_VIDW00ADD1B1);
			break;

		case 1 :
			// start address
			s3c_fimd.vidw01add0b0 = s3c_fimd.vidw01add0b0 + shift_pixel;
			s3c_fimd.vidw01add0b1 = s3c_fimd.vidw01add0b1 + shift_pixel;			
			writel(s3c_fimd.vidw01add0b0, S3C_VIDW01ADD0B0);
			writel(s3c_fimd.vidw01add0b1, S3C_VIDW01ADD0B1);

			// end address
			s3c_fimd.vidw01add1b0 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw01add0b0 + frame_size);
			s3c_fimd.vidw01add1b1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw01add0b1 + frame_size);
			writel(s3c_fimd.vidw01add1b0, S3C_VIDW01ADD1B0);
			writel(s3c_fimd.vidw01add1b1, S3C_VIDW01ADD1B1);
			break;
			
		case 2 :		
			// start address
			s3c_fimd.vidw02add0 = s3c_fimd.vidw02add0 + shift_pixel;
			writel(s3c_fimd.vidw02add0, S3C_VIDW02ADD0);
			
			// end address
			s3c_fimd.vidw02add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw02add0 + frame_size);
			writel(s3c_fimd.vidw02add1, S3C_VIDW02ADD1);
			break;

		case 3 :
			// start address
			s3c_fimd.vidw03add0 = s3c_fimd.vidw03add0 + shift_pixel;
			writel(s3c_fimd.vidw03add0, S3C_VIDW03ADD0);
			
			// end address
			s3c_fimd.vidw03add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw03add0 + frame_size);
			writel(s3c_fimd.vidw03add1, S3C_VIDW03ADD1);
			break;

		case 4 :
			// start address
			s3c_fimd.vidw04add0 = s3c_fimd.vidw04add0 + shift_pixel;
			writel(s3c_fimd.vidw04add0, S3C_VIDW04ADD0);

			// end address
			s3c_fimd.vidw04add1 = S3C_VIDWxxADD1_VBASEL_F(s3c_fimd.vidw04add0 + frame_size);
			writel(s3c_fimd.vidw04add1, S3C_VIDW04ADD1);
			break;

		default :
			printk("%s::invalid fbi->win_id(%d)\n", __func__, fbi->win_id);
			return -EINVAL;
			break;
	}
	return 0;
}

static int s3cfb_set_vs_registers(s3c_fb_info_t *fbi, int vs_cmd)
{
	int ret = 0;
	
	switch (vs_cmd)
	{
		case S3C_FB_VS_SET:
			ret = s3cfb_vs_set(fbi);
			break;

		case S3C_FB_VS_MOVE_LEFT:
			ret = s3cfb_vs_move_left(fbi);
			break;

		case S3C_FB_VS_MOVE_RIGHT:
			ret = s3cfb_vs_move_right(fbi);
			break;

		case S3C_FB_VS_MOVE_UP:
			ret = s3cfb_vs_move_up(fbi);
			break;

		case S3C_FB_VS_MOVE_DOWN:
			ret = s3cfb_vs_move_down(fbi);			
			break;

		default:
			return -EINVAL;
	}

	return ret;
}

#endif // CONFIG_FB_S3C_VIRTUAL_SCREEN

void s3cfb_write_palette(s3c_fb_info_t *fbi)
{
	unsigned int i;
	unsigned long ent;
	unsigned int win_num = fbi->win_id;

	fbi->palette_ready = 0;

	writel((s3c_fimd.wpalcon | S3C_WPALCON_PALUPDATEEN), S3C_WPALCON);

	for (i = 0; i < 256; i++) {
		if ((ent = fbi->palette_buffer[i]) == S3C_FB_PALETTE_BUFF_CLEAR)
			continue;

		writel(ent, S3C_TFTPAL0(i) + 0x400 * win_num);

		/* it seems the only way to know exactly
		 * if the palette wrote ok, is to check
		 * to see if the value verifies ok
		 */
		if (readl(S3C_TFTPAL0(i) + 0x400 * win_num) == ent) {
			fbi->palette_buffer[i] = S3C_FB_PALETTE_BUFF_CLEAR;
		} else {
			fbi->palette_ready = 1;   /* retry */
			printk("Retry writing into the palette\n");
		}
	}

	writel(s3c_fimd.wpalcon, S3C_WPALCON);
}

irqreturn_t s3cfb_irq(int irqno, void *param)
{
	unsigned long buffer_size = 0;
	unsigned int i;
	unsigned int buffer_page_offset, buffer_page_width;
	unsigned int fb_start_address, fb_end_address;

	if (s3c_fb_info[s3c_fimd.palette_win].palette_ready)
		s3cfb_write_palette(&s3c_fb_info[s3c_fimd.palette_win]);

	for (i = 0; i < CONFIG_FB_S3C_NUM; i++)
	{
		if (s3c_fb_info[i].next_fb_info_change_req)
		{
			/* fb variable setting */
			s3c_fb_info[i].fb.fix.smem_start   = s3c_fb_info[i].next_fb_info.phy_start_addr;

			s3c_fb_info[i].fb.fix.line_length  =   s3c_fb_info[i].next_fb_info.xres_virtual
			                                     * s3c_fimd.bytes_per_pixel;

			s3c_fb_info[i].fb.fix.smem_len     =   s3c_fb_info[i].next_fb_info.xres_virtual
			                                     * s3c_fb_info[i].next_fb_info.yres_virtual
			                                     * s3c_fimd.bytes_per_pixel;

			s3c_fb_info[i].fb.var.xres         = s3c_fb_info[i].next_fb_info.xres;
			s3c_fb_info[i].fb.var.yres         = s3c_fb_info[i].next_fb_info.yres;
			s3c_fb_info[i].fb.var.xres_virtual = s3c_fb_info[i].next_fb_info.xres_virtual;
			s3c_fb_info[i].fb.var.yres_virtual = s3c_fb_info[i].next_fb_info.yres_virtual;
			s3c_fb_info[i].fb.var.xoffset      = s3c_fb_info[i].next_fb_info.xoffset;
			s3c_fb_info[i].fb.var.yoffset      = s3c_fb_info[i].next_fb_info.yoffset;
			s3c_fb_info[i].lcd_offset_x        = s3c_fb_info[i].next_fb_info.lcd_offset_x;
			s3c_fb_info[i].lcd_offset_y        = s3c_fb_info[i].next_fb_info.lcd_offset_y;


			/* fb start / end address setting */
			fb_start_address =   (s3c_fb_info[i].next_fb_info.phy_start_addr)
			                   + (s3c_fb_info[i].fb.fix.line_length * s3c_fb_info[i].next_fb_info.yoffset)
			                   + (s3c_fb_info[i].next_fb_info.xoffset * s3c_fimd.bytes_per_pixel);

			fb_end_address   =   fb_start_address
			                   + (s3c_fb_info[i].fb.fix.line_length * s3c_fb_info[i].next_fb_info.yres);

			writel(fb_start_address, S3C_VIDW00ADD0B0 + 0x8 * i);
			writel(S3C_VIDWxxADD1_VBASEL_F(fb_end_address), S3C_VIDW00ADD1B0 + 0x8 * i);

			/* fb virtual / visible size setting */
			buffer_page_width  = s3c_fb_info[i].next_fb_info.xres * s3c_fimd.bytes_per_pixel;
			buffer_page_offset =  (s3c_fb_info[i].next_fb_info.xres_virtual - s3c_fb_info[i].next_fb_info.xres)
			                     * s3c_fimd.bytes_per_pixel;

			buffer_size =    S3C_VIDWxxADD2_OFFSIZE_F(buffer_page_offset)
			              | (S3C_VIDWxxADD2_PAGEWIDTH_F(buffer_page_width));

			writel(buffer_size, S3C_VIDW00ADD2 + 0x04 * i);

			/* LCD position setting */
			writel(  S3C_VIDOSDxA_OSD_LTX_F(s3c_fb_info[i].next_fb_info.lcd_offset_x)
			       | S3C_VIDOSDxA_OSD_LTY_F(s3c_fb_info[i].next_fb_info.lcd_offset_y),
				   S3C_VIDOSD0A+(0x10 * i));

			writel(  S3C_VIDOSDxB_OSD_RBX_F(s3c_fb_info[i].next_fb_info.lcd_offset_x - 1 + s3c_fb_info[i].next_fb_info.xres)
			       | S3C_VIDOSDxB_OSD_RBY_F(s3c_fb_info[i].next_fb_info.lcd_offset_y - 1 + s3c_fb_info[i].next_fb_info.yres),
				   S3C_VIDOSD0B + (0x10 * i));


			/* fb size setting */
			if (i == 0)
				writel(S3C_VIDOSD0C_OSDSIZE(s3c_fb_info[i].next_fb_info.xres * s3c_fb_info[i].next_fb_info.yres), S3C_VIDOSD0C);
			else if (i == 1)
				writel(S3C_VIDOSD0C_OSDSIZE(s3c_fb_info[i].next_fb_info.xres * s3c_fb_info[i].next_fb_info.yres), S3C_VIDOSD1D);
			else if (i == 2)
				writel(S3C_VIDOSD0C_OSDSIZE(s3c_fb_info[i].next_fb_info.xres * s3c_fb_info[i].next_fb_info.yres), S3C_VIDOSD2D);

			s3c_fb_info[i].next_fb_info_change_req = 0;
		}
	}

	/* for clearing the interrupt source */
	writel(readl(S3C_VIDINTCON1), S3C_VIDINTCON1);

	s3c_fimd.vsync_info.count++;
	wake_up_interruptible(&s3c_fimd.vsync_info.wait_queue);

	return IRQ_HANDLED;
}

void s3cfb_enable_local_post(int in_yuv)
{
	unsigned int value;

	s3c_fimd.wincon0 &= ~(S3C_WINCONx_ENLOCAL_MASK | S3C_WINCONx_INRGB_MASK);
	value = S3C_WINCONx_ENLOCAL_POST | S3C_WINCONx_ENWIN_F_ENABLE;

	if (in_yuv)
		value |= S3C_WINCONx_INRGB_YUV;

	writel(s3c_fimd.wincon0 | value, S3C_WINCON0);
}

EXPORT_SYMBOL(s3cfb_enable_local_post);

// andyEdit - 2.6.28 Camera driver
static void s3cfb_check_line_count(void)
{
	int timeout = 30 * 5300;
	unsigned int cfg;
	int i;

	i = 0;
	do {
		if (!(readl(S3C_VIDCON1) & 0x7ff0000))
			break;
		i++;
	} while (i < timeout);

	if (i == timeout) {
		printk(KERN_WARNING "line count mismatch\n");

		cfg = readl(S3C_VIDCON0);
		cfg |= (S3C_VIDCON0_ENVID_F_ENABLE | S3C_VIDCON0_ENVID_ENABLE);
		writel(cfg, S3C_VIDCON0);
	}	
}

static void s3cfb_enable_local0(int in_yuv)
{
	unsigned int value;

	s3c_fimd.wincon0 = readl(S3C_WINCON0);
	s3c_fimd.wincon0 &= ~S3C_WINCONx_ENWIN_F_ENABLE;
	writel(s3c_fimd.wincon0, S3C_WINCON0);	

	s3c_fimd.wincon0 &= ~(S3C_WINCONx_ENLOCAL_MASK | S3C_WINCONx_INRGB_MASK);
	value = S3C_WINCONx_ENLOCAL | S3C_WINCONx_ENWIN_F_ENABLE;

	if (in_yuv)
		value |= S3C_WINCONx_INRGB_YUV;

	writel(s3c_fimd.wincon0 | value, S3C_WINCON0);
}

static void s3cfb_enable_local1(int in_yuv, int sel)
{
	unsigned int value;

	s3c_fimd.wincon1 = readl(S3C_WINCON1);
	s3c_fimd.wincon1 &= ~S3C_WINCONx_ENWIN_F_ENABLE;
	writel(s3c_fimd.wincon1, S3C_WINCON1);

	s3c_fimd.wincon1 &= ~(S3C_WINCONx_ENLOCAL_MASK | S3C_WINCONx_INRGB_MASK);
	s3c_fimd.wincon1 &= ~(S3C_WINCON1_LOCALSEL_MASK);
	value = sel | S3C_WINCONx_ENLOCAL | S3C_WINCONx_ENWIN_F_ENABLE;

	if (in_yuv)
		value |= S3C_WINCONx_INRGB_YUV;

	writel(s3c_fimd.wincon1 | value, S3C_WINCON1);
}

static void s3cfb_enable_local2(int in_yuv, int sel)
{
	unsigned int value;

	s3c_fimd.wincon2 = readl(S3C_WINCON2);
	s3c_fimd.wincon2 &= ~S3C_WINCONx_ENWIN_F_ENABLE;
	s3c_fimd.wincon2 &= ~S3C_WINCON2_LOCALSEL_MASK;
	writel(s3c_fimd.wincon2, S3C_WINCON2);

	s3c_fimd.wincon2 &= ~(S3C_WINCONx_ENLOCAL_MASK | S3C_WINCONx_INRGB_MASK);
	value = sel | S3C_WINCONx_ENLOCAL | S3C_WINCONx_ENWIN_F_ENABLE;

	if (in_yuv)
		value |= S3C_WINCONx_INRGB_YUV;

	writel(s3c_fimd.wincon2 | value, S3C_WINCON2);
}

void s3cfb_enable_local(int win, int in_yuv, int sel)
{
	s3cfb_check_line_count();

	switch (win) {
	case 0:
		s3cfb_enable_local0(in_yuv);
		break;

	case 1:
		s3cfb_enable_local1(in_yuv, sel);
		break;

	case 2:
		s3cfb_enable_local2(in_yuv, sel);
		break;

	default:
		break;
	}
}

void s3cfb_enable_dma(void)
{
	u32 value;

	s3c_fimd.wincon0 &= ~(S3C_WINCONx_ENLOCAL_MASK | S3C_WINCONx_INRGB_MASK);
	value = S3C_WINCONx_ENLOCAL_DMA | S3C_WINCONx_ENWIN_F_ENABLE;

	__raw_writel(s3c_fimd.wincon0 | value, S3C_WINCON0);
}

EXPORT_SYMBOL(s3cfb_enable_dma);

int s3cfb_init_registers(s3c_fb_info_t *fbi)
{
	struct clk *lcd_clock;
	struct fb_var_screeninfo *var = &fbi->fb.var;
	unsigned long flags = 0, page_width = 0, offset = 0;
	unsigned long video_phy_temp_f1 = fbi->screen_dma_f1;
	unsigned long video_phy_temp_f2 = fbi->screen_dma_f2;
	int win_num =  fbi->win_id;

	/* Initialise LCD with values from hare */
	local_irq_save(flags);

	page_width = var->xres * s3c_fimd.bytes_per_pixel;
	offset = (var->xres_virtual - var->xres) * s3c_fimd.bytes_per_pixel;

	if (win_num == 0)
	{
#if defined(S3C_FB_DISPLAY_LOGO)
		lcd_clock = clk_get(NULL, "lcd");	/* Early Clock Setting to Sync Bootloader */
		s3c_fimd.vidcon0 |= S3C_VIDCON0_CLKVAL_F((int)(((((clk_get_rate(lcd_clock) * 10) / s3c_fimd.pixclock) % 10) > 4) ?
												(clk_get_rate(lcd_clock) / s3c_fimd.pixclock) :
												((clk_get_rate(lcd_clock) / s3c_fimd.pixclock) - 1)));
		s3c_fimd.vidcon0 = s3c_fimd.vidcon0 | (S3C_VIDCON0_ENVID_ENABLE | S3C_VIDCON0_ENVID_F_ENABLE);
		writel(s3c_fimd.vidcon0, S3C_VIDCON0);
#else
		s3c_fimd.vidcon0 = s3c_fimd.vidcon0 & ~(S3C_VIDCON0_ENVID_ENABLE | S3C_VIDCON0_ENVID_F_ENABLE);
		writel(s3c_fimd.vidcon0, S3C_VIDCON0);
		lcd_clock = clk_get(NULL, "lcd");
		s3c_fimd.vidcon0 |= S3C_VIDCON0_CLKVAL_F((int)(((((clk_get_rate(lcd_clock) * 10) / s3c_fimd.pixclock) % 10) > 4) ?
												(clk_get_rate(lcd_clock) / s3c_fimd.pixclock) :
												((clk_get_rate(lcd_clock) / s3c_fimd.pixclock) - 1)));

#endif
	}

#if defined(CONFIG_FB_S3C_VIRTUAL_SCREEN)
	{
		offset = 0;

		switch(win_num)
		{
			case 0 :
				s3c_fimd.vidw00add0b0 = video_phy_temp_f1;
				s3c_fimd.vidw00add0b1 = video_phy_temp_f2;
				s3c_fimd.vidw00add1b0 = S3C_VIDWxxADD1_VBASEL_F((unsigned long) video_phy_temp_f1 + (page_width + offset) * (var->yres));
				s3c_fimd.vidw00add1b1 = S3C_VIDWxxADD1_VBASEL_F((unsigned long) video_phy_temp_f2 + (page_width + offset) * (var->yres));
				break;
			case 1 :
				s3c_fimd.vidw01add0b0 = video_phy_temp_f1;
				s3c_fimd.vidw01add0b1 = video_phy_temp_f2;
				s3c_fimd.vidw01add1b0 = S3C_VIDWxxADD1_VBASEL_F((unsigned long) video_phy_temp_f1 + (page_width + offset) * (var->yres));
				s3c_fimd.vidw01add1b1 = S3C_VIDWxxADD1_VBASEL_F((unsigned long) video_phy_temp_f2 + (page_width + offset) * (var->yres));
				break;
			case 2 :
				s3c_fimd.vidw02add0 = video_phy_temp_f1;
				s3c_fimd.vidw02add1 = S3C_VIDWxxADD1_VBASEL_F((unsigned long) video_phy_temp_f1 + (page_width + offset) * (var->yres));
				break;
			case 3 :
				s3c_fimd.vidw03add0 = video_phy_temp_f1;
				s3c_fimd.vidw03add1 = S3C_VIDWxxADD1_VBASEL_F((unsigned long) video_phy_temp_f1 + (page_width + offset) * (var->yres));
				break;
			case 4 :
				s3c_fimd.vidw04add0 = video_phy_temp_f1;
				s3c_fimd.vidw04add1 = S3C_VIDWxxADD1_VBASEL_F((unsigned long) video_phy_temp_f1 + (page_width + offset) * (var->yres));
				break;
			default:
				break;
		}
	}	
	#endif

#if defined(S3C_FB_DISPLAY_LOGO)
		s3cfb_display_logo(win_num);
#endif

	writel(video_phy_temp_f1, S3C_VIDW00ADD0B0 + (0x08 * win_num));
	writel(S3C_VIDWxxADD1_VBASEL_F((unsigned long) video_phy_temp_f1 + (page_width + offset) * (var->yres)), S3C_VIDW00ADD1B0 + (0x08 * win_num));
	writel(S3C_VIDWxxADD2_OFFSIZE_F(offset) | (S3C_VIDWxxADD2_PAGEWIDTH_F(page_width)), S3C_VIDW00ADD2 + (0x04 * win_num));

	if (win_num < 2)
	{
		writel(video_phy_temp_f2, S3C_VIDW00ADD0B1 + (0x08 * win_num));
		writel(S3C_VIDWxxADD1_VBASEL_F((unsigned long) video_phy_temp_f2 + (page_width + offset) * (var->yres)), S3C_VIDW00ADD1B1 + (0x08 * win_num));
	}
	
	switch (win_num)
	{
		case 0:
			writel(s3c_fimd.wincon0,    S3C_WINCON0);
			writel(s3c_fimd.vidcon0,    S3C_VIDCON0);
			writel(s3c_fimd.vidcon1,    S3C_VIDCON1);
			writel(s3c_fimd.vidtcon0,   S3C_VIDTCON0);
			writel(s3c_fimd.vidtcon1,   S3C_VIDTCON1);
			writel(s3c_fimd.vidtcon2,   S3C_VIDTCON2);
			writel(s3c_fimd.dithmode,   S3C_DITHMODE);
			writel(s3c_fimd.vidintcon0, S3C_VIDINTCON0);
			writel(s3c_fimd.vidintcon1, S3C_VIDINTCON1);
			writel(s3c_fimd.vidosd0a,   S3C_VIDOSD0A);
			writel(s3c_fimd.vidosd0b,   S3C_VIDOSD0B);
			writel(s3c_fimd.vidosd0c,   S3C_VIDOSD0C);
			writel(s3c_fimd.wpalcon,    S3C_WPALCON);

			s3cfb_onoff_win(fbi, ON);
			break;

		case 1:
			writel(s3c_fimd.wincon1,  S3C_WINCON1);
			writel(s3c_fimd.vidosd1a, S3C_VIDOSD1A);
			writel(s3c_fimd.vidosd1b, S3C_VIDOSD1B);
			writel(s3c_fimd.vidosd1c, S3C_VIDOSD1C);
			writel(s3c_fimd.vidosd1d, S3C_VIDOSD1D);
			writel(s3c_fimd.wpalcon,  S3C_WPALCON);

			s3cfb_onoff_win(fbi, OFF);
			break;

		case 2:
			writel(s3c_fimd.wincon2,  S3C_WINCON2);
			writel(s3c_fimd.vidosd2a, S3C_VIDOSD2A);
			writel(s3c_fimd.vidosd2b, S3C_VIDOSD2B);
			writel(s3c_fimd.vidosd2c, S3C_VIDOSD2C);
			writel(s3c_fimd.vidosd2d, S3C_VIDOSD2D);
			writel(s3c_fimd.wpalcon,  S3C_WPALCON);

			s3cfb_onoff_win(fbi, OFF);
#if defined(S3C_FB_DISPLAY_LOGO)
			s3cfb_start_progress();
#endif
			break;

		case 3:
			writel(s3c_fimd.wincon3,  S3C_WINCON3);
			writel(s3c_fimd.vidosd3a, S3C_VIDOSD3A);
			writel(s3c_fimd.vidosd3b, S3C_VIDOSD3B);
			writel(s3c_fimd.vidosd3c, S3C_VIDOSD3C);
			writel(s3c_fimd.wpalcon,  S3C_WPALCON);

			s3cfb_onoff_win(fbi, OFF);
			break;

		case 4:
			writel(s3c_fimd.wincon4,  S3C_WINCON4);
			writel(s3c_fimd.vidosd4a, S3C_VIDOSD4A);
			writel(s3c_fimd.vidosd4b, S3C_VIDOSD4B);
			writel(s3c_fimd.vidosd4c, S3C_VIDOSD4C);
			writel(s3c_fimd.wpalcon,  S3C_WPALCON);

			s3cfb_onoff_win(fbi, OFF);
			break;
	}

	local_irq_restore(flags);

	return 0;
 }

void s3cfb_activate_var(s3c_fb_info_t *fbi, struct fb_var_screeninfo *var)
{
	DPRINTK("%s: var->bpp = %d\n", __FUNCTION__, var->bits_per_pixel);

	switch (var->bits_per_pixel) {
	case 8:
		s3c_fimd.wincon0 = S3C_WINCONx_BYTSWP_ENABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_8BPP_PAL;
		s3c_fimd.wincon1 = S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;
		s3c_fimd.wincon2 = S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;
		s3c_fimd.wincon3 = S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;
		s3c_fimd.wincon4 = S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;
		s3c_fimd.bpp     = S3C_FB_PIXEL_BPP_8;
		s3c_fimd.bytes_per_pixel = 1;
		s3c_fimd.wpalcon = S3C_WPALCON_W0PAL_16BIT;
		break;

	case 16:
		s3c_fimd.wincon0 = S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_16BPP_565;
		s3c_fimd.wincon1 = S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;
		s3c_fimd.wincon2 = S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;
		s3c_fimd.wincon3 = S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;
		s3c_fimd.wincon4 = S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_16BPP_565 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;
		s3c_fimd.bpp     = S3C_FB_PIXEL_BPP_16;
		s3c_fimd.bytes_per_pixel = 2;
		break;

	case 24:
		s3c_fimd.wincon0 = S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_24BPP_888;
		s3c_fimd.wincon1 = S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;
		s3c_fimd.wincon2 = S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;
		s3c_fimd.wincon3 = S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;
		s3c_fimd.wincon4 = S3C_WINCONx_HAWSWP_DISABLE | S3C_WINCONx_BURSTLEN_16WORD | S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1;
        s3c_fimd.bpp     = S3C_FB_PIXEL_BPP_24;
		s3c_fimd.bytes_per_pixel = 4;
		break;

	case 32:
		s3c_fimd.bytes_per_pixel = 4;
		break;
	}

	/* write new registers */
	writel(s3c_fimd.wincon0, S3C_WINCON0);
	writel(s3c_fimd.wincon1, S3C_WINCON1);
	writel(s3c_fimd.wincon2, S3C_WINCON2);
	writel(s3c_fimd.wincon3, S3C_WINCON3);
	writel(s3c_fimd.wincon4, S3C_WINCON4);
	writel(s3c_fimd.wpalcon, S3C_WPALCON);
	writel(s3c_fimd.wincon0 | S3C_WINCONx_ENWIN_F_ENABLE, S3C_WINCON0);
	writel(s3c_fimd.vidcon0 | S3C_VIDCON0_ENVID_ENABLE | S3C_VIDCON0_ENVID_F_ENABLE, S3C_VIDCON0);
}

/* JJNAHM comment.
 * We had some problems related to frame buffer address.
 * We used 2 frame buffers (FB0 and FB1) and GTK used FB1.
 * When GTK launched, GTK set FB0's address to FB1's address.
 * (GTK calls s3c_fb_pan_display() and then it calls this s3c_fb_set_lcdaddr())
 * Even though fbi->win_id is not 0, above original codes set ONLY FB0's address.
 * So, I modified the codes like below.
 * It works by fbi->win_id value.
 * Below codes are not verified yet
 * and there are nothing about Double buffering features
 */
void s3cfb_set_fb_addr(s3c_fb_info_t *fbi)
{
	unsigned long video_phy_temp_f1 = fbi->screen_dma_f1;
	unsigned long start_address, end_address;
	unsigned int  start;

	start = fbi->fb.fix.line_length * fbi->fb.var.yoffset;

	/* for buffer start address and end address */
	start_address = video_phy_temp_f1 + start;
	end_address   = start_address + (fbi->fb.fix.line_length * fbi->fb.var.yres);

	switch (fbi->win_id)
	{
		case 0:
			s3c_fimd.vidw00add0b0 = start_address;
			s3c_fimd.vidw00add1b0 = end_address;
			__raw_writel(0x800, S3C_PRTCON); /* enable protection */
			__raw_writel(s3c_fimd.vidw00add0b0, S3C_VIDW00ADD0B0);
			__raw_writel(s3c_fimd.vidw00add1b0, S3C_VIDW00ADD1B0);
			__raw_writel(0x0, S3C_PRTCON); /* disable protection */
        	break;

		case 1:
			s3c_fimd.vidw01add0b0 = start_address;
			s3c_fimd.vidw01add1b0 = end_address;
			__raw_writel(0x800, S3C_PRTCON); /* enable protection */
			__raw_writel(s3c_fimd.vidw01add0b0, S3C_VIDW01ADD0B0);
			__raw_writel(s3c_fimd.vidw01add1b0, S3C_VIDW01ADD1B0);
			__raw_writel(0x0, S3C_PRTCON);	/* disable protection */
			break;

		case 2:
			s3c_fimd.vidw02add0 = start_address;
			s3c_fimd.vidw02add1 = end_address;
			__raw_writel(0x800, S3C_PRTCON); /* enable protection */
			__raw_writel(s3c_fimd.vidw02add0, S3C_VIDW02ADD0);
			__raw_writel(s3c_fimd.vidw02add1, S3C_VIDW02ADD1);
			__raw_writel(0x0, S3C_PRTCON); /* disable protection */
			break;

		case 3:
			s3c_fimd.vidw03add0 = start_address;
			s3c_fimd.vidw03add1 = end_address;
			__raw_writel(0x800, S3C_PRTCON); /* enable protection */
			__raw_writel(s3c_fimd.vidw03add0, S3C_VIDW03ADD0);
			__raw_writel(s3c_fimd.vidw03add1, S3C_VIDW03ADD1);
			__raw_writel(0x0, S3C_PRTCON); /* disable protection */
			break;

		case 4:
			s3c_fimd.vidw04add0 = start_address;
			s3c_fimd.vidw04add1 = end_address;
			__raw_writel(0x800, S3C_PRTCON); /* enable protection */
			__raw_writel(s3c_fimd.vidw04add0, S3C_VIDW04ADD0);
			__raw_writel(s3c_fimd.vidw04add1, S3C_VIDW04ADD1);
			__raw_writel(0x0, S3C_PRTCON); /* disable protection */
			break;
		default :
			printk("%s::invalid win_num(%d)\n", __func__, fbi->win_id);
			break;
	}
}

static int s3cfb_set_alpha_level(s3c_fb_info_t *fbi, unsigned int level, unsigned int alpha_index)
{
	unsigned long alpha_val;
	int win_num = fbi->win_id;

	if (win_num == 0) {
		printk("WIN0 do not support alpha blending.\n");
		return -1;
	}

	alpha_val = readl(S3C_VIDOSD0C+(0x10 * win_num));

	if (alpha_index == 0) {
		alpha_val &= ~(S3C_VIDOSDxC_ALPHA0_B(0xf) | S3C_VIDOSDxC_ALPHA0_G(0xf) | S3C_VIDOSDxC_ALPHA0_R(0xf));
		alpha_val |= S3C_VIDOSDxC_ALPHA0_B(level) | S3C_VIDOSDxC_ALPHA0_G(level) | S3C_VIDOSDxC_ALPHA0_R(level);
	} else {
		alpha_val &= ~(S3C_VIDOSDxC_ALPHA1_B(0xf) | S3C_VIDOSDxC_ALPHA1_G(0xf) | S3C_VIDOSDxC_ALPHA1_R(0xf));
		alpha_val = S3C_VIDOSDxC_ALPHA1_B(level) | S3C_VIDOSDxC_ALPHA1_G(level) | S3C_VIDOSDxC_ALPHA1_R(level);
	}

	writel(alpha_val, S3C_VIDOSD0C + (0x10 * win_num));

	return 0;
}

int s3cfb_set_alpha_mode(s3c_fb_info_t *fbi, int mode)
{
	unsigned long alpha_mode;
	int win_num = fbi->win_id;

	if (win_num == 0) {
		printk("WIN0 do not support alpha blending.\n");
		return -1;
	}

	alpha_mode = readl(S3C_WINCON0 + (0x04 * win_num));
	alpha_mode &= ~(S3C_WINCONx_BLD_PIX_PIXEL | S3C_WINCONx_ALPHA_SEL_1);

	switch (mode) {
	case S3CFB_ALPHA_MODE_PLANE: /* Plane Blending */
		writel(alpha_mode | S3C_WINCONx_BLD_PIX_PLANE | S3C_WINCONx_ALPHA_SEL_1, S3C_WINCON0 + (0x04 * win_num));
		break;

	case S3CFB_ALPHA_MODE_PIXEL: /* Pixel Blending & chroma(color) key */
		writel(alpha_mode | S3C_WINCONx_BLD_PIX_PIXEL | S3C_WINCONx_ALPHA_SEL_0, S3C_WINCON0 + (0x04 * win_num));
		break;
	}

	return 0;
}

int s3cfb_set_win_position(s3c_fb_info_t *fbi, int left_x, int top_y, int width, int height)
{
	struct fb_var_screeninfo *var= &fbi->fb.var;
	int win_num = fbi->win_id;

	writel(S3C_VIDOSDxA_OSD_LTX_F(left_x) | S3C_VIDOSDxA_OSD_LTY_F(top_y), S3C_VIDOSD0A + (0x10 * win_num));
	writel(S3C_VIDOSDxB_OSD_RBX_F(width - 1 + left_x) | S3C_VIDOSDxB_OSD_RBY_F(height - 1 + top_y), S3C_VIDOSD0B + (0x10 * win_num));

	var->xoffset = left_x;
	var->yoffset = top_y;

	return 0;
}

int s3cfb_set_win_size(s3c_fb_info_t *fbi, int width, int height)
{
	struct fb_var_screeninfo *var= &fbi->fb.var;
	int win_num = fbi->win_id;

	if (win_num == 1)
		writel(S3C_VIDOSD0C_OSDSIZE(width * height), S3C_VIDOSD1D);
	else if (win_num == 2)
		writel(S3C_VIDOSD0C_OSDSIZE(width * height), S3C_VIDOSD2D);

	var->xres = width;
	var->yres = height;

	// is this bug?
	//var->xres_virtual = width;
	//var->yres_virtual = height;

	return 0;
}

int s3cfb_set_fb_size(s3c_fb_info_t *fbi)
{
	struct fb_var_screeninfo *var= &fbi->fb.var;
	int win_num = fbi->win_id;
	unsigned long offset = 0;
	unsigned long page_width = 0;
	unsigned long fb_size = 0;

	page_width = var->xres * s3c_fimd.bytes_per_pixel;
	offset = (var->xres_virtual - var->xres) * s3c_fimd.bytes_per_pixel;

#if defined(CONFIG_FB_S3C_VIRTUAL_SCREEN)
	if (win_num == 0)
		offset=0;
#endif

	writel(S3C_VIDWxxADD1_VBASEL_F((unsigned long) readl(S3C_VIDW00ADD0B0 + (0x08 * win_num)) + (page_width + offset) * (var->yres)), S3C_VIDW00ADD1B0 + (0x08 * win_num));

	if (win_num == 1)
		writel(S3C_VIDWxxADD1_VBASEL_F((unsigned long) readl(S3C_VIDW00ADD0B1 + (0x08 * win_num)) + (page_width + offset) * (var->yres)), S3C_VIDW00ADD1B1 + (0x08 * win_num));

	/* size of frame buffer */
	fb_size = S3C_VIDWxxADD2_OFFSIZE_F(offset) | (S3C_VIDWxxADD2_PAGEWIDTH_F(page_width));

	writel(fb_size, S3C_VIDW00ADD2 + (0x04 * win_num));

	return 0;
}

void s3cfb_set_output_path(int out)
{
	unsigned int tmp;

	tmp = readl(S3C_VIDCON0);

	/* if output mode is LCD mode, Scan mode always should be progressive mode */
	if (out == S3C_FB_OUTPUT_TV)
		tmp &= ~S3C_VIDCON0_INTERLACE_F_MASK;

	tmp &= ~S3C_VIDCON0_VIDOUT_MASK;
	tmp |= S3C_VIDCON0_VIDOUT(out);

	writel(tmp, S3C_VIDCON0);
}

EXPORT_SYMBOL(s3cfb_set_output_path);

void s3cfb_enable_rgbport(int on)
{
	if (on)
		writel(S3C_VIDCON2_ORGYUV_CBCRY | S3C_VIDCON2_YUVORD_CRCB, S3C_VIDCON2);
	else
		writel(0x0, S3C_VIDCON2);
}

EXPORT_SYMBOL(s3cfb_enable_rgbport);

int s3cfb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	s3c_fb_info_t *fbi = container_of(info, s3c_fb_info_t, fb);

	s3c_win_info_t       win_info;
	s3c_color_key_info_t colkey_info;
	s3c_color_val_info_t colval_info;
	s3c_fb_dma_info_t    dma_info;
	s3c_fb_next_info_t   next_fb_info;
	struct fb_var_screeninfo *var= &fbi->fb.var;
	unsigned int crt, alpha_level, alpha_mode;

#if defined(CONFIG_S3C6410_PWM)
	int brightness;
#endif

#if defined(CONFIG_FB_S3C_DOUBLE_BUFFERING)
	unsigned int f_num_val;
#endif

#if defined(CONFIG_FB_S3C_VIRTUAL_SCREEN)
	s3c_vs_info_t vs_info;
#endif

	switch(cmd){
	case S3C_FB_GET_INFO:
		dma_info.map_dma_f1 = fbi->map_dma_f1;
		dma_info.map_dma_f2 = fbi->map_dma_f2;

		if(copy_to_user((void *) arg, (const void *) &dma_info, sizeof(s3c_fb_dma_info_t)))
			return -EFAULT;
		break;

	case S3C_FB_OSD_SET_INFO:
		if (copy_from_user(&win_info, (s3c_win_info_t *) arg, sizeof(s3c_win_info_t)))
			return -EFAULT;

		s3cfb_init_win(fbi, win_info.bpp, win_info.left_x, win_info.top_y, win_info.width, win_info.height, OFF);
		break;

	case S3C_FB_OSD_START:
		s3cfb_onoff_win(fbi, ON);
		break;

	case S3C_FB_OSD_STOP:
		if (fbi->win_id > 0)
			s3cfb_set_alpha_level(fbi, S3C_FB_MAX_ALPHA_LEVEL, 1);

		s3cfb_onoff_win(fbi, OFF);
		break;

	case S3C_FB_OSD_ALPHA_UP:
		alpha_level = readl(S3C_VIDOSD0C + (0x10 * fbi->win_id)) & 0xf;

		if (alpha_level < S3C_FB_MAX_ALPHA_LEVEL)
			alpha_level++;

		s3cfb_set_alpha_level(fbi, alpha_level, 1);
		break;

	case S3C_FB_OSD_ALPHA_DOWN:
		alpha_level = readl(S3C_VIDOSD0C + (0x10 * fbi->win_id)) & 0xf;

		if (alpha_level > 0)
			alpha_level--;

		s3cfb_set_alpha_level(fbi, alpha_level, 1);
		break;

	case S3C_FB_OSD_ALPHA0_SET:
		alpha_level = (unsigned int) arg;

		if (alpha_level > S3C_FB_MAX_ALPHA_LEVEL)
			alpha_level = S3C_FB_MAX_ALPHA_LEVEL;
		else if(alpha_level < 0)
			alpha_level = 0;

		s3cfb_set_alpha_level(fbi, alpha_level, 0);
		break;

	case S3C_FB_OSD_ALPHA_SET:
		alpha_level = (unsigned int) arg;

		if (alpha_level > S3C_FB_MAX_ALPHA_LEVEL)
			alpha_level = S3C_FB_MAX_ALPHA_LEVEL;
		else if (alpha_level < 0)
			alpha_level = 0;

		s3cfb_set_alpha_level(fbi, alpha_level, 1);
		break;

	case S3C_FB_OSD_ALPHA_MODE:
		alpha_mode = (unsigned int) arg;
		s3cfb_set_alpha_mode(fbi, alpha_mode);
		break;

	case S3C_FB_OSD_MOVE_LEFT:
		if (var->xoffset > 0)
			var->xoffset--;

		s3cfb_set_win_position(fbi, var->xoffset, var->yoffset, var->xres, var->yres);
		break;

	case S3C_FB_OSD_MOVE_RIGHT:
		if (var->xoffset < (s3c_fimd.width - var->xres))
			var->xoffset++;

		s3cfb_set_win_position(fbi, var->xoffset, var->yoffset, var->xres, var->yres);
		break;

	case S3C_FB_OSD_MOVE_UP:
		if (var->yoffset > 0)
			var->yoffset--;

		s3cfb_set_win_position(fbi, var->xoffset, var->yoffset, var->xres, var->yres);
		break;

	case S3C_FB_OSD_MOVE_DOWN:
		if (var->yoffset < (s3c_fimd.height - var->yres))
			var->yoffset++;

		s3cfb_set_win_position(fbi, var->xoffset, var->yoffset, var->xres, var->yres);
		break;

	case FBIO_WAITFORVSYNC:
		if (get_user(crt, (unsigned int __user *)arg))
			return -EFAULT;

		return s3cfb_wait_for_vsync();

	case S3C_FB_COLOR_KEY_START:
		s3cfb_onoff_color_key(fbi, ON);
		break;

	case S3C_FB_COLOR_KEY_STOP:
		s3cfb_onoff_color_key(fbi, OFF);
		break;

	case S3C_FB_COLOR_KEY_ALPHA_START:
		s3cfb_onoff_color_key_alpha(fbi, ON);
		break;

	case S3C_FB_COLOR_KEY_ALPHA_STOP:
		s3cfb_onoff_color_key_alpha(fbi, OFF);
		break;

	case S3C_FB_COLOR_KEY_SET_INFO:
		if (copy_from_user(&colkey_info, (s3c_color_key_info_t *) arg, sizeof(s3c_color_key_info_t)))
			return -EFAULT;

		s3cfb_set_color_key_registers(fbi, colkey_info);
		break;

	case S3C_FB_COLOR_KEY_VALUE:
		if (copy_from_user(&colval_info, (s3c_color_val_info_t *) arg, sizeof(s3c_color_val_info_t)))
			return -EFAULT;

		s3cfb_set_color_value(fbi, colval_info);
		break;

	case S3C_FB_SET_VSYNC_INT:
		s3c_fimd.vidintcon0 &= ~S3C_VIDINTCON0_FRAMESEL0_MASK;
		s3c_fimd.vidintcon0 |= S3C_VIDINTCON0_FRAMESEL0_VSYNC;

		if (arg)
			s3c_fimd.vidintcon0 |= S3C_VIDINTCON0_INTFRMEN_ENABLE;
		else
			s3c_fimd.vidintcon0 &= ~S3C_VIDINTCON0_INTFRMEN_ENABLE;

		writel(s3c_fimd.vidintcon0, S3C_VIDINTCON0);
		break;

	case S3C_FB_SET_NEXT_FB_INFO:
		if (copy_from_user(&next_fb_info, (s3c_fb_next_info_t *) arg, sizeof(s3c_fb_next_info_t)))
			return -EFAULT;

		/* check arguments */
		if ((next_fb_info.xres + next_fb_info.xoffset) > next_fb_info.xres_virtual ||
			(next_fb_info.yres + next_fb_info.yoffset) > next_fb_info.yres_virtual ||
			(next_fb_info.xres + next_fb_info.lcd_offset_x ) > s3c_fimd.width ||
			(next_fb_info.yres + next_fb_info.lcd_offset_y ) > s3c_fimd.height)
			return -EINVAL;

		fbi->next_fb_info = next_fb_info;
		fbi->next_fb_info_change_req = 1;
		break;

	case S3C_FB_GET_CURR_FB_INFO:
		next_fb_info.phy_start_addr = fbi->fb.fix.smem_start;
		next_fb_info.xres = fbi->fb.var.xres;
		next_fb_info.yres = fbi->fb.var.yres;
		next_fb_info.xres_virtual = fbi->fb.var.xres_virtual;
		next_fb_info.yres_virtual = fbi->fb.var.yres_virtual;
		next_fb_info.xoffset = fbi->fb.var.xoffset;
		next_fb_info.yoffset = fbi->fb.var.yoffset;
		next_fb_info.lcd_offset_x = fbi->lcd_offset_x;
		next_fb_info.lcd_offset_y = fbi->lcd_offset_y;

		if (copy_to_user((void *)arg, (s3c_fb_next_info_t *) &next_fb_info, sizeof(s3c_fb_next_info_t)))
			return -EFAULT;
		break;

	case S3C_FB_GET_BRIGHTNESS:
		if (copy_to_user((void *)arg, (const void *) &s3c_fimd.brightness, sizeof(int)))
			return -EFAULT;
		break;

#if defined(CONFIG_S3C6410_PWM)
	case S3C_FB_SET_BRIGHTNESS:
		if (copy_from_user(&brightness, (int *) arg, sizeof(int)))
			return -EFAULT;

		s3cfb_set_brightness(brightness);
		break;
#endif

#if defined(CONFIG_FB_S3C_VIRTUAL_SCREEN)

	case S3C_FB_VS_START:
		if (s3cfb_vs_start(fbi)) {
			printk("Error S3C_FB_VS_START\n");
			return -EINVAL;
		}
		break;

	case S3C_FB_VS_STOP:
		if (s3cfb_vs_stop(fbi)) {
			printk("Error S3C_FB_VS_STOP\n");
			return -EINVAL;
		}
		break;

	case S3C_FB_VS_SET_INFO:
		if (copy_from_user(&vs_info, (s3c_vs_info_t *) arg, sizeof(s3c_vs_info_t)))
			return -EFAULT;

		if (s3cfb_set_vs_info(vs_info)) {
			printk("Error S3C_FB_VS_SET_INFO\n");
			return -EINVAL;
		}

		s3cfb_set_vs_registers(fbi, S3C_FB_VS_SET);

		fbi->fb.var.xoffset = s3c_fimd.xoffset;
		fbi->fb.var.yoffset = s3c_fimd.yoffset;
		break;

	case S3C_FB_VS_MOVE:
		s3cfb_set_vs_registers(fbi, arg);

		fbi->fb.var.xoffset = s3c_fimd.xoffset;
		fbi->fb.var.yoffset = s3c_fimd.yoffset;
		break;
#endif

#if defined(CONFIG_FB_S3C_DOUBLE_BUFFERING)
	case S3C_FB_GET_NUM:
		if (copy_from_user((void *)&f_num_val, (const void *)arg, sizeof(u_int)))
			return -EFAULT;

		if (copy_to_user((void *)arg, (const void *) &f_num_val, sizeof(u_int)))
			return -EFAULT;

		break;

	case S3C_FB_CHANGE_REQ:
		s3cfb_change_buff(0, (int) arg);
		break;
#endif

	default:
		return -EINVAL;
	}

	return 0;
}

void s3cfb_pre_init(void)
{
	/* initialize the fimd specific */
	s3c_fimd.vidintcon0 &= ~S3C_VIDINTCON0_FRAMESEL0_MASK;
	s3c_fimd.vidintcon0 |= S3C_VIDINTCON0_FRAMESEL0_VSYNC;
	s3c_fimd.vidintcon0 |= S3C_VIDINTCON0_INTFRMEN_ENABLE;

	writel(s3c_fimd.vidintcon0, S3C_VIDINTCON0);
}

#if 0
int s3cfb_set_gpio(void)
{
	unsigned long val;
#if defined (CONFIG_MACH_SMDK6410)
	int i, err;
#endif
	/* Must be '0' for Normal-path instead of By-pass */
	writel(0x0, S3C_HOSTIFB_MIFPCON);

	/* enable clock to LCD */
	val = readl(S3C_SCLK_GATE);
	val |= S3C_CLKCON_SCLK_LCD;
	writel(val, S3C_SCLK_GATE);

	/* select TFT LCD type (RGB I/F) */
	val = readl(S3C64XX_SPC_BASE);
	val &= ~0x3;
	val |= (1 << 0);
	writel(val, S3C64XX_SPC_BASE);
#if defined (CONFIG_MACH_SMDK6410)
	/* VD */
	for (i = 0; i < 16; i++)
		s3c_gpio_cfgpin(S3C64XX_GPI(i), S3C_GPIO_SFN(2));

	for (i = 0; i < 12; i++)
		s3c_gpio_cfgpin(S3C64XX_GPJ(i), S3C_GPIO_SFN(2));

	/* backlight ON */
	if (gpio_is_valid(S3C64XX_GPF(15))) {
		err = gpio_request(S3C64XX_GPF(15), "GPF");

		if (err) {
			printk(KERN_ERR "failed to request GPF for "
				"lcd backlight control\n");
			return err;
		}

		gpio_direction_output(S3C64XX_GPF(15), 1);
	}

	/* module reset */
	if (gpio_is_valid(S3C64XX_GPN(5))) {
		err = gpio_request(S3C64XX_GPN(5), "GPN");

		if (err) {
			printk(KERN_ERR "failed to request GPN for "
				"lcd reset control\n");
			return err;
		}

		gpio_direction_output(S3C64XX_GPN(5), 1);
	}
	
	mdelay(100);

	gpio_set_value(S3C64XX_GPN(5), 0);
	mdelay(10);

	gpio_set_value(S3C64XX_GPN(5), 1);
	mdelay(10);

	gpio_free(S3C64XX_GPF(15));
	gpio_free(S3C64XX_GPN(5));
#endif
	return 0;
}
#endif

#if defined(CONFIG_PM)

static struct sleep_save s3c_lcd_save[] = {
	SAVE_ITEM(S3C_VIDCON0),
	SAVE_ITEM(S3C_VIDCON1),

	SAVE_ITEM(S3C_VIDTCON0),
	SAVE_ITEM(S3C_VIDTCON1),
	SAVE_ITEM(S3C_VIDTCON2),
	SAVE_ITEM(S3C_VIDTCON3),

	SAVE_ITEM(S3C_WINCON0),
	SAVE_ITEM(S3C_WINCON1),
	SAVE_ITEM(S3C_WINCON2),
	SAVE_ITEM(S3C_WINCON3),
	SAVE_ITEM(S3C_WINCON4),

	SAVE_ITEM(S3C_VIDOSD0A),
	SAVE_ITEM(S3C_VIDOSD0B),
	SAVE_ITEM(S3C_VIDOSD0C),

	SAVE_ITEM(S3C_VIDOSD1A),
	SAVE_ITEM(S3C_VIDOSD1B),
	SAVE_ITEM(S3C_VIDOSD1C),
	SAVE_ITEM(S3C_VIDOSD1D),

	SAVE_ITEM(S3C_VIDOSD2A),
	SAVE_ITEM(S3C_VIDOSD2B),
	SAVE_ITEM(S3C_VIDOSD2C),
	SAVE_ITEM(S3C_VIDOSD2D),

	SAVE_ITEM(S3C_VIDOSD3A),
	SAVE_ITEM(S3C_VIDOSD3B),
	SAVE_ITEM(S3C_VIDOSD3C),

	SAVE_ITEM(S3C_VIDOSD4A),
	SAVE_ITEM(S3C_VIDOSD4B),
	SAVE_ITEM(S3C_VIDOSD4C),

	SAVE_ITEM(S3C_VIDW00ADD0B0),
	SAVE_ITEM(S3C_VIDW00ADD0B1),
	SAVE_ITEM(S3C_VIDW01ADD0B0),
	SAVE_ITEM(S3C_VIDW01ADD0B1),
	SAVE_ITEM(S3C_VIDW02ADD0),
	SAVE_ITEM(S3C_VIDW03ADD0),
	SAVE_ITEM(S3C_VIDW04ADD0),
	SAVE_ITEM(S3C_VIDW00ADD1B0),
	SAVE_ITEM(S3C_VIDW00ADD1B1),
	SAVE_ITEM(S3C_VIDW01ADD1B0),
	SAVE_ITEM(S3C_VIDW01ADD1B1),
	SAVE_ITEM(S3C_VIDW02ADD1),
	SAVE_ITEM(S3C_VIDW03ADD1),
	SAVE_ITEM(S3C_VIDW04ADD1),
	SAVE_ITEM(S3C_VIDW00ADD2),
	SAVE_ITEM(S3C_VIDW01ADD2),
	SAVE_ITEM(S3C_VIDW02ADD2),
	SAVE_ITEM(S3C_VIDW03ADD2),
	SAVE_ITEM(S3C_VIDW04ADD2),

	SAVE_ITEM(S3C_VIDINTCON0),
	SAVE_ITEM(S3C_VIDINTCON1),
	SAVE_ITEM(S3C_W1KEYCON0),
	SAVE_ITEM(S3C_W1KEYCON1),
	SAVE_ITEM(S3C_W2KEYCON0),
	SAVE_ITEM(S3C_W2KEYCON1),

	SAVE_ITEM(S3C_W3KEYCON0),
	SAVE_ITEM(S3C_W3KEYCON1),
	SAVE_ITEM(S3C_W4KEYCON0),
	SAVE_ITEM(S3C_W4KEYCON1),
	SAVE_ITEM(S3C_DITHMODE),

	SAVE_ITEM(S3C_WIN0MAP),
	SAVE_ITEM(S3C_WIN1MAP),
	SAVE_ITEM(S3C_WIN2MAP),
	SAVE_ITEM(S3C_WIN3MAP),
	SAVE_ITEM(S3C_WIN4MAP),
	SAVE_ITEM(S3C_WPALCON),

	SAVE_ITEM(S3C_TRIGCON),
	SAVE_ITEM(S3C_I80IFCONA0),
	SAVE_ITEM(S3C_I80IFCONA1),
	SAVE_ITEM(S3C_I80IFCONB0),
	SAVE_ITEM(S3C_I80IFCONB1),
	SAVE_ITEM(S3C_LDI_CMDCON0),
	SAVE_ITEM(S3C_LDI_CMDCON1),
	SAVE_ITEM(S3C_SIFCCON0),
	SAVE_ITEM(S3C_SIFCCON1),
	SAVE_ITEM(S3C_SIFCCON2),

	SAVE_ITEM(S3C_LDI_CMD0),
	SAVE_ITEM(S3C_LDI_CMD1),
	SAVE_ITEM(S3C_LDI_CMD2),
	SAVE_ITEM(S3C_LDI_CMD3),
	SAVE_ITEM(S3C_LDI_CMD4),
	SAVE_ITEM(S3C_LDI_CMD5),
	SAVE_ITEM(S3C_LDI_CMD6),
	SAVE_ITEM(S3C_LDI_CMD7),
	SAVE_ITEM(S3C_LDI_CMD8),
	SAVE_ITEM(S3C_LDI_CMD9),
	SAVE_ITEM(S3C_LDI_CMD10),
	SAVE_ITEM(S3C_LDI_CMD11),

	SAVE_ITEM(S3C_W2PDATA01),
	SAVE_ITEM(S3C_W2PDATA23),
	SAVE_ITEM(S3C_W2PDATA45),
	SAVE_ITEM(S3C_W2PDATA67),
	SAVE_ITEM(S3C_W2PDATA89),
	SAVE_ITEM(S3C_W2PDATAAB),
	SAVE_ITEM(S3C_W2PDATACD),
	SAVE_ITEM(S3C_W2PDATAEF),
	SAVE_ITEM(S3C_W3PDATA01),
	SAVE_ITEM(S3C_W3PDATA23),
	SAVE_ITEM(S3C_W3PDATA45),
	SAVE_ITEM(S3C_W3PDATA67),
	SAVE_ITEM(S3C_W3PDATA89),
	SAVE_ITEM(S3C_W3PDATAAB),
	SAVE_ITEM(S3C_W3PDATACD),
	SAVE_ITEM(S3C_W3PDATAEF),
	SAVE_ITEM(S3C_W4PDATA01),
	SAVE_ITEM(S3C_W4PDATA23),

	//SAVE_ITEM(S3C_HOSTIFB_MIFPCON),
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static int lcd_pm_status = 0;
int lcd_late_resume = 1;

static int s3cfb_suspend_sub(s3c_fb_info_t *fbi)
{
	s3c6410_pm_do_save(s3c_lcd_save, ARRAY_SIZE(s3c_lcd_save));

	/* sleep before disabling the clock, we need to ensure
	 * the LCD DMA engine is not going to get back on the bus
	 * before the clock goes off again (bjd) */

	msleep(1);

	clk_disable(fbi->clk);

#ifdef USE_LCD_DOMAIN_GATING
	s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_LP_MODE, S3C64XX_LCD);
#endif /* USE_LCD_DOMAIN_GATING */

        return 0;
}

static int s3cfb_resume_sub(s3c_fb_info_t *fbi)
{
#ifdef USE_LCD_DOMAIN_GATING
	s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_ACTIVE_MODE, S3C64XX_LCD);
	if(s3c_wait_blk_pwr_ready(S3C64XX_BLK_F)) {
		printk(KERN_ERR "[%s] Domain F is not ready\n", __func__);
			return -1;
	}
#endif /* USE_LCD_DOMAIN_GATING */

	clk_enable(fbi->clk);

	s3c6410_pm_do_restore(s3c_lcd_save, ARRAY_SIZE(s3c_lcd_save));

#ifndef CONFIG_MACH_SMDK6410
	s3cfb_set_gpio();
#endif /* CONFIG_MACH_SMDK6410 */

	s3cfb_start_lcd();

	return 0;
}

void s3cfb_early_suspend(struct early_suspend *h)
{
	s3c_fb_info_t *info = container_of(h, s3c_fb_info_t, early_suspend);

	printk("#%s\n", __func__);

	lcd_late_resume = 0;

	s3cfb_suspend_sub(info);

	lcd_pm_status = 0;
}

void s3cfb_late_resume(struct early_suspend *h)
{
	s3c_fb_info_t *info = container_of(h, s3c_fb_info_t, early_suspend);

	printk("#%s\n", __func__);

	if (lcd_pm_status == 0) {
		s3cfb_resume_sub(info);
	}

	lcd_late_resume = 1;
}

/*
 *  Suspend
 */
int s3cfb_suspend(struct platform_device *dev, pm_message_t state)
{
	struct fb_info *fbinfo = platform_get_drvdata(dev);
	s3c_fb_info_t *info = fbinfo->par;

	printk("#%s\n", __func__);
	
	if (lcd_pm_status != 0) {
		s3cfb_suspend_sub(info);
	}

	return 0;
}

/*
 *  Resume
 */
int s3cfb_resume(struct platform_device *dev)
{
	struct fb_info *fbinfo = platform_get_drvdata(dev);
	s3c_fb_info_t *info = fbinfo->par;

	printk("#%s\n", __func__);

	s3cfb_resume_sub(info);

	lcd_pm_status = 1;

	return 0;
}

/*
 * shutdown
 */
extern void lcd_power_ctrl(s32 value);
int s3cfb_shutdown(struct platform_device *dev)
{
//	lcd_power_ctrl(0);	//marked by hui
	return 0;
}
#else

/*
 *  Suspend
 */
int s3cfb_suspend(struct platform_device *dev, pm_message_t state)
{
	struct fb_info *fbinfo = platform_get_drvdata(dev);
	s3c_fb_info_t *info = fbinfo->par;

	s3cfb_suspend_sub(info);

	return 0;
}

/*
 *  Resume
 */
int s3cfb_resume(struct platform_device *dev)
{
	struct fb_info *fbinfo = platform_get_drvdata(dev);
	s3c_fb_info_t *info = fbinfo->par;

	s3cfb_resume_sub(info);

	return 0;
}

/*
 * shutdown
 */
int s3cfb_shutdown(struct platform_device *dev)
{
	lcd_power_ctrl(0);
}
#endif	/* CONFIG_HAS_EARLYSUSPEND */

#else
int s3cfb_suspend(struct platform_device *dev, pm_message_t state)
{
	return 0;
}

int s3cfb_resume(struct platform_device *dev)
{
	return 0;
}

int s3cfb_shutdown(struct platform_device *dev)
{
	return 0;
}
#endif	/* CONFIG_PM */
