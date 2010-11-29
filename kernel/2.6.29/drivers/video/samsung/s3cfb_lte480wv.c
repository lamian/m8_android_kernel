/*
 * drivers/video/samsung/s3cfb_lte480wv.c
 *
 * $Id: s3cfb_lte480wv.c,v 1.12 2008/06/05 02:13:24 jsgood Exp $
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
#include <linux/leds.h>
#include <linux/gpio.h>

#include <plat/regs-gpio.h>
#include <plat/regs-lcd.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-clock.h>

#include "s3cfb.h"

#define BACKLIGHT_STATUS_ALC	0x100
#define BACKLIGHT_LEVEL_VALUE	0x0FF	/* 0 ~ 255 */

#define BACKLIGHT_LEVEL_MIN		1
#define BACKLIGHT_LEVEL_DEFAULT	(BACKLIGHT_STATUS_ALC | 0xFF)	/* Default Setting */
#define BACKLIGHT_LEVEL_MAX		(BACKLIGHT_STATUS_ALC | BACKLIGHT_LEVEL_VALUE)

int lcd_power = OFF;
EXPORT_SYMBOL(lcd_power);

int lcd_power_ctrl(s32 value);
EXPORT_SYMBOL(lcd_power_ctrl);

int backlight_power = OFF;
EXPORT_SYMBOL(backlight_power);

void backlight_power_ctrl(s32 value);
EXPORT_SYMBOL(backlight_power_ctrl);

int backlight_level = BACKLIGHT_LEVEL_DEFAULT;
EXPORT_SYMBOL(backlight_level);

void backlight_level_ctrl(s32 value);
EXPORT_SYMBOL(backlight_level_ctrl);

#define S3C_FB_HFP		8	/* front porch */
#define S3C_FB_HSW		3	/* hsync width */
#define S3C_FB_HBP		13	/* back porch */

#define S3C_FB_VFP		5	/* front porch */
#define S3C_FB_VSW		1	/* vsync width */
#define S3C_FB_VBP		7	/* back porch */

#define S3C_FB_HRES		800	/* horizon pixel  x resolition */
#define S3C_FB_VRES		480	/* line cnt       y resolution */

#define S3C_FB_HRES_VIRTUAL	S3C_FB_HRES     /* horizon pixel  x resolition */
#define S3C_FB_VRES_VIRTUAL	S3C_FB_VRES * 2 /* line cnt       y resolution */

#define S3C_FB_HRES_OSD		800	/* horizon pixel  x resolition */
#define S3C_FB_VRES_OSD		480	/* line cnt       y resolution */

#define S3C_FB_HRES_OSD_VIRTUAL	S3C_FB_HRES_OSD     /* horizon pixel  x resolition */
#define S3C_FB_VRES_OSD_VIRTUAL S3C_FB_VRES_OSD * 2 /* line cnt       y resolution */

#define S3C_FB_VFRAME_FREQ     	75	/* frame rate freq */

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

	s3c_fimd.set_lcd_power = lcd_power_ctrl;
	s3c_fimd.set_backlight_power = backlight_power_ctrl;
	s3c_fimd.set_brightness = backlight_level_ctrl;

	s3c_fimd.backlight_min = BACKLIGHT_LEVEL_MIN;
	s3c_fimd.backlight_max = BACKLIGHT_LEVEL_MAX;
}
#if  defined(CONFIG_S3C6410_PWM)
extern void s3cfb_set_brightness(int val);
#else
void s3cfb_set_brightness(int val){}
#endif
int lcd_power_ctrl(s32 value)
{
	int err;

	if (value) {
		printk(KERN_INFO "LCD power on sequence start\n");
		if (gpio_is_valid(S3C64XX_GPN(5))) {
			err = gpio_request(S3C64XX_GPN(5), "GPN");

			if (err) {
				printk(KERN_ERR "failed to request GPN for "
					"lcd reset control\n");
				return -1;
			}
			gpio_direction_output(S3C64XX_GPN(5), 1);
		}
		printk(KERN_INFO "LCD power on sequence end\n");
	}
	else {
		printk(KERN_INFO "LCD power off sequence start\n");
		if (gpio_is_valid(S3C64XX_GPN(5))) {
			err = gpio_request(S3C64XX_GPN(5), "GPN");

			if (err) {
				printk(KERN_ERR "failed to request GPN for "
					"lcd reset control\n");
				return -1;
			}
			gpio_direction_output(S3C64XX_GPN(5), 0);
		}
		printk(KERN_INFO "LCD power off sequence end\n");
	}
	gpio_free(S3C64XX_GPN(5));
	lcd_power = value;
}

static void backlight_ctrl(s32 value)
{
	int err, ret;

	if (value) {
		/* backlight ON */
		if (lcd_power == OFF) {
			ret = lcd_power_ctrl(ON);
			if (ret != 0) {
				printk(KERN_ERR "lcd power on control is failed\n");
				return;
			}
		}
		
		if (gpio_is_valid(S3C64XX_GPF(15))) {
			err = gpio_request(S3C64XX_GPF(15), "GPF");

			if (err) {
				printk(KERN_ERR "failed to request GPF for "
					"lcd backlight control\n");
			}

			gpio_direction_output(S3C64XX_GPF(15), 1);
		}
		s3cfb_set_brightness((int)(value/3));
	}
	else {
		ret = lcd_power_ctrl(OFF);
		if (ret != 0) {
			printk(KERN_ERR "lcd power off control is failed\n");
		}
		/* backlight OFF */
		if (gpio_is_valid(S3C64XX_GPF(15))) {
			err = gpio_request(S3C64XX_GPF(15), "GPF");

			if (err) {
				printk(KERN_ERR "failed to request GPF for "
					"lcd backlight control\n");
			}

			gpio_direction_output(S3C64XX_GPF(15), 0);
		 }
	}
	gpio_free(S3C64XX_GPF(15));
}

void backlight_level_ctrl(s32 value)
{
	if ((value < BACKLIGHT_LEVEL_MIN) ||	/* Invalid Value */
		(value > BACKLIGHT_LEVEL_MAX) ||
		(value == backlight_level))	/* Same Value */
		return;

	if (backlight_power)
		s3cfb_set_brightness((int)(value/3));	
	
	backlight_level = value;	
}

void backlight_power_ctrl(s32 value)
{
	if ((value < OFF) ||	/* Invalid Value */
		(value > ON) ||
		(value == backlight_power))	/* Same Value */
		return;

	backlight_ctrl((value ? backlight_level : OFF));	
	
	backlight_power = value;	
}

#define SMDK_DEFAULT_BACKLIGHT_BRIGHTNESS	255
static DEFINE_MUTEX(smdk_backlight_lock);

static void smdk_set_backlight_level(u8 level)
{
	if (backlight_level == level)
		return;

	backlight_ctrl(level);
	
	backlight_level = level;
}

static void smdk_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	mutex_lock(&smdk_backlight_lock);
	smdk_set_backlight_level(value);
	mutex_unlock(&smdk_backlight_lock);
}

static struct led_classdev smdk_backlight_led  = {
	.name		= "lcd-backlight",
	.brightness = SMDK_DEFAULT_BACKLIGHT_BRIGHTNESS,
	.brightness_set = smdk_brightness_set,
};

static int smdk_bl_probe(struct platform_device *pdev)
{
	led_classdev_register(&pdev->dev, &smdk_backlight_led);
	return 0;
}

static int smdk_bl_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&smdk_backlight_led);
	return 0;
}

#ifdef CONFIG_PM
static int smdk_bl_suspend(struct platform_device *pdev, pm_message_t state)
{
	led_classdev_suspend(&smdk_backlight_led);
	return 0;
}

static int smdk_bl_resume(struct platform_device *dev)
{
	led_classdev_resume(&smdk_backlight_led);
	return 0;
}
#else
#define smdk_bl_suspend	NULL
#define smdk_bl_resume	NULL
#endif

static struct platform_driver smdk_bl_driver = {
	.probe		= smdk_bl_probe,
	.remove		= smdk_bl_remove,
	.suspend	= smdk_bl_suspend,
	.resume		= smdk_bl_resume,
	.driver		= {
		.name	= "smdk-backlight",
	},
};

static int __init smdk_bl_init(void)
{
	printk("SMDK board LCD Backlight Device Driver (c) 2008 Samsung Electronics \n");

	platform_driver_register(&smdk_bl_driver);
	return 0;
}

static void __exit smdk_bl_exit(void)
{
 	platform_driver_unregister(&smdk_bl_driver);
}

module_init(smdk_bl_init);
module_exit(smdk_bl_exit);

MODULE_AUTHOR("Jongpill Lee <boyko.lee@samsung.com>");
MODULE_DESCRIPTION("SMDK board Backlight Driver");
MODULE_LICENSE("GPL");

void s3cfb_init_hw(void)
{
	printk(KERN_INFO "LCD TYPE :: LTE480WV will be initialized\n");

	s3cfb_set_fimd_info();
	s3cfb_set_gpio();
}
