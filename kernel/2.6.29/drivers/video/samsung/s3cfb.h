/*
 * drivers/video/samsung/s3cfb.h
 *
 * $Id: s3cfb.h,v 1.1 2008/11/17 11:12:08 jsgood Exp $
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

#ifndef _S3CFB_H_
#define _S3CFB_H_

#include <linux/interrupt.h>
#include <linux/earlysuspend.h>

#if  defined(CONFIG_S3C6410_PWM)
extern int s3c6410_timer_setup (int channel, int usec, unsigned long g_tcnt, unsigned long g_tcmp);
#endif

/*
 *  Debug macros
 */
#define DEBUG 0

#if DEBUG
#define DPRINTK(fmt, args...)	printk("%s: " fmt, __FUNCTION__ , ## args)
#else
#define DPRINTK(fmt, args...)
#endif

/*
 *  Definitions
 */
#ifndef MHZ
#define MHZ (1000 * 1000)
#endif

#define ON 	1
#define OFF	0

#define S3C_FB_PIXEL_BPP_8	8
#define S3C_FB_PIXEL_BPP_16	16	/*  RGB 5-6-5 format for SMDK EVAL BOARD */
#define S3C_FB_PIXEL_BPP_24	24	/*  RGB 8-8-8 format for SMDK EVAL BOARD */

#define S3C_FB_OUTPUT_RGB	0
#define S3C_FB_OUTPUT_TV	1
#define S3C_FB_OUTPUT_I80_LDI0	2
#define S3C_FB_OUTPUT_I80_LDI1	3

#if defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
#define S3C_FB_MAX_NUM	2

#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410) || defined(CONFIG_CPU_S5P6440)
#define S3C_FB_MAX_NUM	5

#else
#define S3C_FB_MAX_NUM	1

#endif

#define S3C_FB_PALETTE_BUFF_CLEAR	(0x80000000)	/* entry is clear/invalid */
#define S3C_FB_COLOR_KEY_DIR_BG 	0
#define S3C_FB_COLOR_KEY_DIR_FG 	1
#define S3C_FB_DEFAULT_BACKLIGHT_LEVEL	2
#define S3C_FB_MAX_DISPLAY_OFFSET	200
#define S3C_FB_DEFAULT_DISPLAY_OFFSET	100
#define S3C_FB_MAX_ALPHA_LEVEL		0xf
#define S3C_FB_MAX_BRIGHTNESS		90
#define S3C_FB_DEFAULT_BRIGHTNESS	4
#define S3C_FB_VS_SET 			12
#define S3C_FB_VS_MOVE_LEFT		15
#define S3C_FB_VS_MOVE_RIGHT		16
#define S3C_FB_VS_MOVE_UP		17
#define S3C_FB_VS_MOVE_DOWN		18
#define S3CFB_ALPHA_MODE_PLANE		0
#define S3CFB_ALPHA_MODE_PIXEL		1

/*
 *  macros
 */
#define PRINT_MHZ(m) 			((m) / MHZ), ((m / 1000) % 1000)
#define FB_MAX_NUM(x, y)		((x) > (y) ? (y) : (x))
#define S3C_FB_NUM			FB_MAX_NUM(S3C_FB_MAX_NUM, CONFIG_FB_S3C_NUM)

/*
 *  ioctls
 */
#define S3C_FB_GET_BRIGHTNESS		_IOR ('F', 1,  unsigned int)
#define S3C_FB_SET_BRIGHTNESS		_IOW ('F', 2,  unsigned int)
#define S3C_FB_WIN_ON			_IOW ('F', 10, unsigned int)
#define S3C_FB_WIN_OFF			_IOW ('F', 11, unsigned int)
#define FBIO_WAITFORVSYNC		_IOW ('F', 32, unsigned int)

#if defined(CONFIG_FB_S3C_VIRTUAL_SCREEN)
#define S3C_FB_VS_START			_IO  ('F', 103)
#define S3C_FB_VS_STOP			_IO  ('F', 104)
#define S3C_FB_VS_SET_INFO		_IOW ('F', 105, s3c_vs_info_t)
#define S3C_FB_VS_MOVE			_IOW ('F', 106, unsigned int)
#endif

#define S3C_FB_OSD_START		_IO  ('F', 201)
#define S3C_FB_OSD_STOP			_IO  ('F', 202)
#define S3C_FB_OSD_ALPHA_UP		_IO  ('F', 203)
#define S3C_FB_OSD_ALPHA_DOWN		_IO  ('F', 204)
#define S3C_FB_OSD_MOVE_LEFT		_IO  ('F', 205)
#define S3C_FB_OSD_MOVE_RIGHT		_IO  ('F', 206)
#define S3C_FB_OSD_MOVE_UP		_IO  ('F', 207)
#define S3C_FB_OSD_MOVE_DOWN		_IO  ('F', 208)
#define S3C_FB_OSD_SET_INFO		_IOW ('F', 209, s3c_win_info_t)
#define S3C_FB_OSD_ALPHA_SET		_IOW ('F', 210, unsigned int)
#define S3C_FB_OSD_ALPHA0_SET		_IOW ('F', 211, unsigned int)
#define S3C_FB_OSD_ALPHA_MODE		_IOW ('F', 212, unsigned int)

#define S3C_FB_COLOR_KEY_START		_IO  ('F', 300)
#define S3C_FB_COLOR_KEY_STOP		_IO  ('F', 301)
#define S3C_FB_COLOR_KEY_ALPHA_START	_IO  ('F', 302)
#define S3C_FB_COLOR_KEY_ALPHA_STOP	_IO  ('F', 303)
#define S3C_FB_COLOR_KEY_SET_INFO	_IOW ('F', 304, s3c_color_key_info_t)
#define S3C_FB_COLOR_KEY_VALUE		_IOW ('F', 305, s3c_color_val_info_t)

#if defined(CONFIG_FB_S3C_DOUBLE_BUFFERING)
#define S3C_FB_GET_NUM			_IOWR('F', 306, unsigned int)
#endif

#define S3C_FB_GET_INFO			_IOR ('F', 307, s3c_fb_dma_info_t)
#define S3C_FB_CHANGE_REQ		_IOW ('F', 308, int)
#define S3C_FB_SET_VSYNC_INT		_IOW ('F', 309, int)
#define S3C_FB_SET_NEXT_FB_INFO		_IOW ('F', 320, s3c_fb_next_info_t)
#define S3C_FB_GET_CURR_FB_INFO		_IOR ('F', 321, s3c_fb_next_info_t)

/*
 *  structures
 */
typedef struct {
	int bpp;
	int left_x;
	int top_y;
	int width;
	int height;
} s3c_win_info_t;

typedef struct {
	int width;
	int height;
	int bpp;
	int offset;
	int v_width;
	int v_height;
} s3c_vs_info_t;

typedef struct {
	int direction;
	unsigned int compkey_red;
	unsigned int compkey_green;
	unsigned int compkey_blue;
} s3c_color_key_info_t;

typedef struct {
	unsigned int colval_red;
	unsigned int colval_green;
	unsigned int colval_blue;
} s3c_color_val_info_t;

typedef struct {
	wait_queue_head_t wait_queue;
	int count;
} s3c_vsync_info_t;

typedef struct
{
	dma_addr_t map_dma_f1;
	dma_addr_t map_dma_f2;
} s3c_fb_dma_info_t;

typedef struct {
	__u32 phy_start_addr;
	__u32 xres;		/* visible resolution*/
	__u32 yres;
	__u32 xres_virtual;	/* virtual resolution*/
	__u32 yres_virtual;
	__u32 xoffset;		/* offset from virtual to visible */
	__u32 yoffset;		/* resolution	*/
	__u32 lcd_offset_x;
	__u32 lcd_offset_y;
} s3c_fb_next_info_t;

typedef struct {
	struct fb_bitfield red;
	struct fb_bitfield green;
	struct fb_bitfield blue;
	struct fb_bitfield transp;
} s3c_fb_rgb_t;

const static s3c_fb_rgb_t s3c_fb_rgb_8 = {
	.red    = {.offset = 0,  .length = 8,},
	.green  = {.offset = 0,  .length = 8,},
	.blue   = {.offset = 0,  .length = 8,},
	.transp = {.offset = 0,  .length = 0,},
};

const static s3c_fb_rgb_t s3c_fb_rgb_16 = {
	.red    = {.offset = 11, .length = 5,},
	.green  = {.offset = 5,  .length = 6,},
	.blue   = {.offset = 0,  .length = 5,},
	.transp = {.offset = 0,  .length = 0,},
};

const static s3c_fb_rgb_t s3c_fb_rgb_24 = {
	.red    = {.offset = 16, .length = 8,},
	.green  = {.offset = 8,  .length = 8,},
	.blue   = {.offset = 0,  .length = 8,},
	.transp = {.offset = 0,  .length = 0,},
};

const static s3c_fb_rgb_t s3c_fb_rgb_32 = {
	.red    = {.offset = 16, .length = 8,},
	.green  = {.offset = 8,  .length = 8,},
	.blue   = {.offset = 0,  .length = 8,},
	.transp = {.offset = 24, .length = 8,},
};

typedef struct {
	struct fb_info		fb;
	struct device		*dev;

	struct clk		*clk;

	struct resource		*mem;
	void __iomem		*io;

	unsigned int		win_id;

	unsigned int		max_bpp;
	unsigned int		max_xres;
	unsigned int		max_yres;

	/* raw memory addresses */
	dma_addr_t		map_dma_f1;	/* physical */
	u_char *		map_cpu_f1;	/* virtual */
	unsigned int		map_size_f1;

	/* addresses of pieces placed in raw buffer */
	u_char *		screen_cpu_f1;	/* virtual address of frame buffer */
	dma_addr_t		screen_dma_f1;	/* physical address of frame buffer */

	/* raw memory addresses */
	dma_addr_t		map_dma_f2;	/* physical */
	u_char *		map_cpu_f2;	/* virtual */
	unsigned int		map_size_f2;

	/* addresses of pieces placed in raw buffer */
	u_char *		screen_cpu_f2;	/* virtual address of frame buffer */
	dma_addr_t		screen_dma_f2;	/* physical address of frame buffer */

	unsigned int		palette_ready;
	unsigned int		fb_change_ready;

	struct early_suspend	early_suspend;
	
	/* keep these registers in case we need to re-write palette */
	unsigned int		palette_buffer[256];
	unsigned int		pseudo_pal[16];

	unsigned int		lcd_offset_x;
	unsigned int		lcd_offset_y;
	unsigned int		next_fb_info_change_req;
	s3c_fb_next_info_t	next_fb_info;
} s3c_fb_info_t;

typedef struct {

	/* Screen size */
	int width;
	int height;

	/* Screen info */
	int xres;
	int yres;

	/* Virtual Screen info */
	int xres_virtual;
	int yres_virtual;
	int xoffset;
	int yoffset;

	/* OSD Screen size */
	int osd_width;
	int osd_height;

	/* OSD Screen info */
	int osd_xres;
	int osd_yres;

	/* OSD Screen info */
	int osd_xres_virtual;
	int osd_yres_virtual;

	int bpp;
	int bytes_per_pixel;
	unsigned long pixclock;

	int hsync_len;
	int left_margin;
	int right_margin;
	int vsync_len;
	int upper_margin;
	int lower_margin;
	int sync;

	int cmap_grayscale:1;
	int cmap_inverse:1;
	int cmap_static:1;
	int unused:29;

	/* backlight info */
	int backlight_min;
	int backlight_max;
	int backlight_default;

	int vs_offset;
	int brightness;
	int palette_win;
	int backlight_level;
	int backlight_power;
	int lcd_power;

	s3c_vsync_info_t vsync_info;
	s3c_vs_info_t vs_info;

	/* lcd configuration registers */
	unsigned long lcdcon1;
	unsigned long lcdcon2;

        unsigned long lcdcon3;
	unsigned long lcdcon4;
	unsigned long lcdcon5;

	/* GPIOs */
	unsigned long gpcup;
	unsigned long gpcup_mask;
	unsigned long gpccon;
	unsigned long gpccon_mask;
	unsigned long gpdup;
	unsigned long gpdup_mask;
	unsigned long gpdcon;
	unsigned long gpdcon_mask;

	/* lpc3600 control register */
	unsigned long lpcsel;
	unsigned long lcdtcon1;
	unsigned long lcdtcon2;
	unsigned long lcdtcon3;
	unsigned long lcdosd1;
	unsigned long lcdosd2;
	unsigned long lcdosd3;
	unsigned long lcdsaddrb1;
	unsigned long lcdsaddrb2;
	unsigned long lcdsaddrf1;
	unsigned long lcdsaddrf2;
	unsigned long lcdeaddrb1;
	unsigned long lcdeaddrb2;
	unsigned long lcdeaddrf1;
	unsigned long lcdeaddrf2;
	unsigned long lcdvscrb1;
	unsigned long lcdvscrb2;
	unsigned long lcdvscrf1;
	unsigned long lcdvscrf2;
	unsigned long lcdintcon;
	unsigned long lcdkeycon;
	unsigned long lcdkeyval;
	unsigned long lcdbgcon;
	unsigned long lcdfgcon;
	unsigned long lcddithcon;

	unsigned long vidcon0;
	unsigned long vidcon1;
	unsigned long vidtcon0;
	unsigned long vidtcon1;
	unsigned long vidtcon2;
	unsigned long vidtcon3;
	unsigned long wincon0;
	unsigned long wincon2;
	unsigned long wincon1;
	unsigned long wincon3;
	unsigned long wincon4;

	unsigned long vidosd0a;
	unsigned long vidosd0b;
	unsigned long vidosd0c;
	unsigned long vidosd1a;
	unsigned long vidosd1b;
	unsigned long vidosd1c;
	unsigned long vidosd1d;
	unsigned long vidosd2a;
	unsigned long vidosd2b;
	unsigned long vidosd2c;
	unsigned long vidosd2d;
	unsigned long vidosd3a;
	unsigned long vidosd3b;
	unsigned long vidosd3c;
	unsigned long vidosd4a;
	unsigned long vidosd4b;
	unsigned long vidosd4c;

	unsigned long vidw00add0b0;
	unsigned long vidw00add0b1;
	unsigned long vidw01add0;
	unsigned long vidw01add0b0;
	unsigned long vidw01add0b1;

	unsigned long vidw00add1b0;
	unsigned long vidw00add1b1;
	unsigned long vidw01add1;
	unsigned long vidw01add1b0;
	unsigned long vidw01add1b1;

	unsigned long vidw00add2b0;
	unsigned long vidw00add2b1;

	unsigned long vidw02add0;
	unsigned long vidw03add0;
	unsigned long vidw04add0;

	unsigned long vidw02add1;
	unsigned long vidw03add1;
	unsigned long vidw04add1;
	unsigned long vidw00add2;
	unsigned long vidw01add2;
	unsigned long vidw02add2;
	unsigned long vidw03add2;
	unsigned long vidw04add2;

	unsigned long vidintcon;
	unsigned long vidintcon0;
	unsigned long vidintcon1;
	unsigned long w1keycon0;
	unsigned long w1keycon1;
	unsigned long w2keycon0;
	unsigned long w2keycon1;
	unsigned long w3keycon0;
	unsigned long w3keycon1;
	unsigned long w4keycon0;
	unsigned long w4keycon1;

	unsigned long win0map;
	unsigned long win1map;
	unsigned long win2map;
	unsigned long win3map;
	unsigned long win4map;

	unsigned long wpalcon;
	unsigned long dithmode;
	unsigned long intclr0;
	unsigned long intclr1;
	unsigned long intclr2;

	unsigned long win0pal;
	unsigned long win1pal;

	/* utility functions */
	void (*set_backlight_power)(int);
	void (*set_lcd_power)(int);
	void (*set_brightness)(int);
	int (*map_video_memory)(s3c_fb_info_t *);
	int (*unmap_video_memory)(s3c_fb_info_t *);
}s3c_fimd_info_t;

/*
 *  Externs
 */
extern s3c_fb_info_t s3c_fb_info[];
extern s3c_fimd_info_t s3c_fimd;

extern int soft_cursor(struct fb_info *info, struct fb_cursor *cursor);
extern int s3cfb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg);
extern void s3cfb_activate_var(s3c_fb_info_t *fbi, struct fb_var_screeninfo *var);
extern void s3cfb_set_fb_addr(s3c_fb_info_t *fbi);
extern void s3cfb_init_hw(void);
extern irqreturn_t s3cfb_irq(int irqno, void *param);
extern int s3cfb_init_registers(s3c_fb_info_t *fbi);
extern int s3cfb_set_win_position(s3c_fb_info_t *fbi, int left_x, int top_y, int width, int height);
extern int s3cfb_set_win_size(s3c_fb_info_t *fbi, int width, int height);
extern int s3cfb_set_fb_size(s3c_fb_info_t *fbi);
extern int s3cfb_set_vs_info(s3c_vs_info_t vs_info);
extern int s3cfb_wait_for_vsync(void);
extern int s3cfb_onoff_color_key(s3c_fb_info_t *fbi, int onoff);
extern int s3cfb_onoff_color_key_alpha(s3c_fb_info_t *fbi, int onoff);
extern int s3cfb_set_color_key_registers(s3c_fb_info_t *fbi, s3c_color_key_info_t colkey_info);
extern int s3cfb_set_color_value(s3c_fb_info_t *fbi, s3c_color_val_info_t colval_info);
extern int s3cfb_init_win(s3c_fb_info_t *fbi, int bpp, int left_x, int top_y, int width, int height, int onoff);
extern int s3cfb_onoff_win(s3c_fb_info_t *fbi, int onoff);
extern int s3cfb_set_gpio(void);
extern void s3cfb_start_lcd(void);
extern void s3cfb_stop_lcd(void);
extern int s3cfb_suspend(struct platform_device *dev, pm_message_t state);
extern int s3cfb_resume(struct platform_device *dev);
extern int s3cfb_shutdown(struct platform_device *dev);	
extern int s3cfb_spi_gpio_request(int ch);
extern void s3cfb_spi_lcd_den(int ch, int value);
extern void s3cfb_spi_lcd_dseri(int ch, int value);
extern void s3cfb_spi_lcd_dclk(int ch, int value);
extern void s3cfb_spi_set_lcd_data(int ch);
extern int s3cfb_spi_gpio_free(int ch);
extern void s3cfb_pre_init(void);
extern void s3cfb_display_logo(void);
//extern void s3cfb_start_progress(void);
//extern void s3cfb_stop_progress(void);
extern void s3cfb_set_lcd_power(int to);
extern void s3cfb_set_backlight_power(int to);
extern void s3cfb_set_backlight_level(int to);

#ifdef CONFIG_HAS_EARLYSUSPEND
extern void s3cfb_early_suspend(struct early_suspend *h);
extern void s3cfb_late_resume(struct early_suspend *h);
#endif	/* CONFIG_HAS_EARLYSUSPEND */

#endif

