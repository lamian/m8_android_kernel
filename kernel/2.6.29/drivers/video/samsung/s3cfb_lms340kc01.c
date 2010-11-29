/*
 * drivers/video/s3c/s3cfb_lms340kc01.c
 *
 * $Id: s3cfb_lms340kc01.c,v 1.1 2009/09/29 
 *
 * Copyright (C) 2009 meizu <meizu@meizu.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 *	S3C Frame Buffer Driver
 */

#include <linux/wait.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#include <plat/regs-lcd.h>

#include <linux/leds.h>
#include <linux/gpio.h>

#include <plat/regs-gpio.h>
#include <plat/regs-lcd.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-clock.h>

#include "s3cfb.h"

#define BACKLIGHT_STATUS_ALC	0x000
#define BACKLIGHT_LEVEL_VALUE	0x0FF	/* 0 ~ 255 */

#define BACKLIGHT_LEVEL_MIN		1
#define BACKLIGHT_LEVEL_DEFAULT	(BACKLIGHT_STATUS_ALC | 0xFF)	/* Default Setting */
#define BACKLIGHT_LEVEL_MAX		(BACKLIGHT_STATUS_ALC | BACKLIGHT_LEVEL_VALUE)

int lcd_power = ON;
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

#if  defined(CONFIG_S3C6410_PWM)
extern void s3cfb_set_brightness(int val);
#else
void s3cfb_set_brightness(int val){}
#endif

//////////////////////////////////////////////

#define S3C_FB_HFP		3		/* front porch */
#define S3C_FB_HSW		6		/* hsync width */
#define S3C_FB_HBP		18		/* back porch */

#define S3C_FB_VFP		3		/* front porch */
#define S3C_FB_VSW		3		/* vsync width */
#define S3C_FB_VBP		3		/* back porch */

#define S3C_FB_HRES		720		/* horizon pixel  x resolition */
#define S3C_FB_VRES		480		/* line cnt       y resolution */

#define S3C_FB_HRES_VIRTUAL	720	/* horizon pixel  x resolition */
#define S3C_FB_VRES_VIRTUAL	960	/* line cnt       y resolution */

#define S3C_FB_HRES_OSD		720	/* horizon pixel  x resolition */
#define S3C_FB_VRES_OSD		480	/* line cnt       y resolution */

#define S3C_FB_VFRAME_FREQ     	60	/* frame rate freq */

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

	s3c_fimd.osd_xres_virtual = S3C_FB_HRES_OSD;
	s3c_fimd.osd_yres_virtual = S3C_FB_VRES_OSD;

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

/*
* S3C64XX_GPQ(8)->SCLK
* S3C64XX_GPQ(7)->SI
* S3C64XX_GPQ(6)->CS
* S3C64XX_GPQ(5)->SO
* S3C64XX_GPQ(4)->RESET
*/
#define MEIZU_M8_SPI_CLK	S3C64XX_GPQ(8)
#define MEIZU_M8_SPI_SI		S3C64XX_GPQ(7)
#define MEIZU_M8_SPI_CS	S3C64XX_GPQ(6)
#define MEIZU_M8_SPI_SO	S3C64XX_GPQ(5)
#define MEIZU_M8_SPI_RST 	S3C64XX_GPQ(4)
#define MEIZU_M8_BACKLIGHT S3C64XX_GPF(15)

static int s3cfb_hx5118_spi_write(bool fDataFlag, int data)
{
	const int delay = 1;
	int i;
	
	/*设置SPI 总线的初始化状态*/
	gpio_set_value(MEIZU_M8_SPI_CS,1);
	gpio_set_value(MEIZU_M8_SPI_CLK,0);
	gpio_set_value(MEIZU_M8_SPI_SI,0);
	udelay(delay);
	
	/*开始传输数据*/
	gpio_set_value(MEIZU_M8_SPI_CS,0);	//先把片选拉低,选中器件
	udelay(delay);

	//0:command ; 1:data
	gpio_set_value(MEIZU_M8_SPI_SI,fDataFlag);	//first bit
	udelay(delay);
	gpio_set_value(MEIZU_M8_SPI_CLK,1);	//时钟上升沿锁定数据
	udelay(delay);
	
	for(i=7;i>=0;i--)	//后面的8 个数据
	{
		gpio_set_value(MEIZU_M8_SPI_CLK,0);	//时钟下降沿传送数据
		gpio_set_value(MEIZU_M8_SPI_SI,(data>>i)&0x1?1:0);	//先传送最高位bit
		udelay(delay);
		gpio_set_value(MEIZU_M8_SPI_CLK,1);	//时钟上升沿锁定数据
		udelay(delay);
	}

	gpio_set_value(MEIZU_M8_SPI_CS,1);	//片选拉高

	/*增加两个时钟作为停止延时*/
	gpio_set_value(MEIZU_M8_SPI_CLK,0);
	udelay(delay);
	gpio_set_value(MEIZU_M8_SPI_CLK,1);
	udelay(delay);
	gpio_set_value(MEIZU_M8_SPI_CLK,0);
	udelay(delay);
	gpio_set_value(MEIZU_M8_SPI_CLK,1);
	udelay(delay);

	return 0;
}


static int s3cfb_hx5118_spi_read(int regs)
{
	const int delay = 1;
	int i;
	bool tmp;
	unsigned int val =0;
		
	/*设置SPI 总线的初始化状态*/
	gpio_set_value(MEIZU_M8_SPI_CS,1);
	gpio_set_value(MEIZU_M8_SPI_CLK,0);
	gpio_set_value(MEIZU_M8_SPI_SI,0);
	udelay(delay);

	/*开始传输数据*/
	gpio_set_value(MEIZU_M8_SPI_CS,0);	//先把片选拉低,选中器件
	udelay(delay);
	
	//0:command ; 1:data
	gpio_set_value(MEIZU_M8_SPI_SI,0);	//first bit
	udelay(delay);
	gpio_set_value(MEIZU_M8_SPI_CLK,1);	//时钟上升沿锁定数据
	udelay(delay);

	for(i=7;i>=0;i--)	//后面的8 个数据
	{
		gpio_set_value(MEIZU_M8_SPI_CLK,0);	//时钟下降沿传送数据
		gpio_set_value(MEIZU_M8_SPI_SI,(regs>>i)&0x1?1:0);	//先传送最高位bit
		udelay(delay);
		gpio_set_value(MEIZU_M8_SPI_CLK,1);	//时钟上升沿锁定数据
		udelay(delay);
	}
	
	switch(regs){
	case 0x04:	//24bit
	case 0x09:	//32bit
		/*Dummy clock*/
		gpio_set_value(MEIZU_M8_SPI_CLK,0);
		udelay(delay);
		gpio_set_value(MEIZU_M8_SPI_CLK,1);
		udelay(delay);

		/*读数据24 bit or 32bit*/
		if(regs==0x04) i=23;
		else i=31;
		for(;i>=0;i--)
		{
			gpio_set_value(MEIZU_M8_SPI_CLK,0);
			udelay(delay);
			tmp = gpio_get_value(MEIZU_M8_SPI_SO);	//读数据
			val |= tmp<<i;
			gpio_set_value(MEIZU_M8_SPI_CLK,1);
			udelay(delay);
		}
		break;
	case 0x2e:	//GRAM data read
		gpio_set_value(MEIZU_M8_SPI_CLK,0);
		gpio_set_value(MEIZU_M8_SPI_SI,0);
		udelay(delay);
		gpio_set_value(MEIZU_M8_SPI_CLK,1);
		udelay(delay);

		/*dummy read*/
		for(i=7;i>=0;i--)
		{
			gpio_set_value(MEIZU_M8_SPI_CLK,0);
			udelay(delay);
			tmp = gpio_get_value(MEIZU_M8_SPI_SO);	//读数据
			gpio_set_value(MEIZU_M8_SPI_CLK,1);
			udelay(delay);
		}

		gpio_set_value(MEIZU_M8_SPI_CLK,0);
		gpio_set_value(MEIZU_M8_SPI_SI,0);
		udelay(delay);
		gpio_set_value(MEIZU_M8_SPI_CLK,1);
		udelay(delay);

		for(i=7;i>=0;i--)	//first 8 bit data
		{
			gpio_set_value(MEIZU_M8_SPI_CLK,0);
			udelay(delay);
			tmp = gpio_get_value(MEIZU_M8_SPI_SO);	//读数据
			val |= tmp<<(i+8);
			gpio_set_value(MEIZU_M8_SPI_CLK,1);
			udelay(delay);
		}

		gpio_set_value(MEIZU_M8_SPI_CLK,0);
		gpio_set_value(MEIZU_M8_SPI_SI,0);
		udelay(delay);
		gpio_set_value(MEIZU_M8_SPI_CLK,1);
		udelay(delay);

		for(i=7;i>=0;i--)	//first 8 bit data
		{
			gpio_set_value(MEIZU_M8_SPI_CLK,0);
			udelay(delay);
			tmp = gpio_get_value(MEIZU_M8_SPI_SO);	//读数据
			val |= tmp<<i;
			gpio_set_value(MEIZU_M8_SPI_CLK,1);
			udelay(delay);
		}
		
		break;
	default:		//8bit		
		for(i=7;i>=0;i--)
		{
			gpio_set_value(MEIZU_M8_SPI_CLK,0);
			udelay(delay);
			tmp = gpio_get_value(MEIZU_M8_SPI_SO);	//读数据
			val |= tmp<<i;
			gpio_set_value(MEIZU_M8_SPI_CLK,1);
			udelay(delay);
		}
		break;
	}

	gpio_set_value(MEIZU_M8_SPI_CLK,0);
	udelay(delay);
	gpio_set_value(MEIZU_M8_SPI_CS,1);
	udelay(delay);
	
	return val;
}

static int s3cfb_hx5118_reset(void)
{
	int err = 0;
	if (gpio_is_valid(MEIZU_M8_SPI_RST)) {
		err = gpio_request(MEIZU_M8_SPI_RST, "GPQ");
		if (!err) {
			gpio_direction_output(MEIZU_M8_SPI_RST,1);
			s3c_gpio_setpull(MEIZU_M8_SPI_RST,S3C_GPIO_PULL_NONE);
			
			/* module reset */
			msleep(2);
			gpio_set_value(MEIZU_M8_SPI_RST, 0);
			msleep(5);
			gpio_set_value(MEIZU_M8_SPI_RST, 1);
			msleep(25);
		}
		else 
			printk("s3cfb_reset fail!\n");
	} 
	else 
		err = 1;

	return err;	

}


static void s3cfb_init_hx5118(void)
{
	s3cfb_hx5118_spi_write(0x00, 0xBE);	//Set OTP Related Setting
	s3cfb_hx5118_spi_write(0x01, 0x00);	
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x80);
	msleep(5);

	s3cfb_hx5118_spi_write(0x00, 0xB6);	//Set Interface and Display Mode Select
	s3cfb_hx5118_spi_write(0x01, 0x02);
	msleep(5);

	s3cfb_hx5118_spi_write(0x00, 0xB1);	//Set Panel Related register
	s3cfb_hx5118_spi_write(0x01, 0x01);
	msleep(5);

	s3cfb_hx5118_spi_write(0x00, 0xB7);	//Set Display Waveform Cycle
	s3cfb_hx5118_spi_write(0x01, 0x10);
	s3cfb_hx5118_spi_write(0x01, 0x04);
	s3cfb_hx5118_spi_write(0x01, 0x03);
	s3cfb_hx5118_spi_write(0x01, 0x02);
	s3cfb_hx5118_spi_write(0x01, 0x07);
	s3cfb_hx5118_spi_write(0x01, 0x02);
	s3cfb_hx5118_spi_write(0x01, 0x27);
	s3cfb_hx5118_spi_write(0x01, 0x02);
	s3cfb_hx5118_spi_write(0x01, 0x0F);
	s3cfb_hx5118_spi_write(0x01, 0x03);
	s3cfb_hx5118_spi_write(0x01, 0x07);
	s3cfb_hx5118_spi_write(0x01, 0x0C);
	s3cfb_hx5118_spi_write(0x01, 0x04);
	s3cfb_hx5118_spi_write(0x01, 0x03);
	s3cfb_hx5118_spi_write(0x01, 0x02);
	s3cfb_hx5118_spi_write(0x01, 0x07);
	s3cfb_hx5118_spi_write(0x01, 0x02);
	s3cfb_hx5118_spi_write(0x01, 0x27);
	s3cfb_hx5118_spi_write(0x01, 0x02);
	s3cfb_hx5118_spi_write(0x01, 0x0F);
	s3cfb_hx5118_spi_write(0x01, 0x03);
	s3cfb_hx5118_spi_write(0x01, 0x07);
	s3cfb_hx5118_spi_write(0x01, 0x0C);
	msleep(5);

	s3cfb_hx5118_spi_write(0x00, 0xB5);	//Set RGB interface related register
	s3cfb_hx5118_spi_write(0x01, 0x09);
	msleep(5);

	s3cfb_hx5118_spi_write(0x00, 0xB4);	//Set Display Related register
	s3cfb_hx5118_spi_write(0x01, 0x43);
	s3cfb_hx5118_spi_write(0x01, 0x7f);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x40);
	s3cfb_hx5118_spi_write(0x01, 0xA5);
	msleep(5);

	s3cfb_hx5118_spi_write(0x00, 0xB8);	//Set Source Voltage Related Register
	s3cfb_hx5118_spi_write(0x01, 0x2c);
	s3cfb_hx5118_spi_write(0x01, 0x34);
	s3cfb_hx5118_spi_write(0x01, 0x0D);
	s3cfb_hx5118_spi_write(0x01, 0x0D);
	msleep(15);

	s3cfb_hx5118_spi_write(0x00, 0x11);	//sleep out
	msleep(200);

	s3cfb_hx5118_spi_write(0x00, 0xB2);	//Set Power
	s3cfb_hx5118_spi_write(0x01, 0xF0);
	s3cfb_hx5118_spi_write(0x01, 0x30);
	s3cfb_hx5118_spi_write(0x01, 0x2D);
	s3cfb_hx5118_spi_write(0x01, 0x44);
	s3cfb_hx5118_spi_write(0x01, 0x11);
	s3cfb_hx5118_spi_write(0x01, 0x11);
	s3cfb_hx5118_spi_write(0x01, 0x1E);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x75);
	msleep(5);

	s3cfb_hx5118_spi_write(0x00, 0x26);	//Gamma set
	s3cfb_hx5118_spi_write(0x01, 0x02);
	msleep(5);

	s3cfb_hx5118_spi_write(0x00, 0xBB);	//Set Red Gamma Curve Related Setting
	s3cfb_hx5118_spi_write(0x01, 0x12);
	s3cfb_hx5118_spi_write(0x01, 0x12);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x47);
	s3cfb_hx5118_spi_write(0x01, 0x13);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x47);
	s3cfb_hx5118_spi_write(0x01, 0x13);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x22);
	s3cfb_hx5118_spi_write(0x01, 0x38);
	s3cfb_hx5118_spi_write(0x01, 0x38);
	msleep(5);

	s3cfb_hx5118_spi_write(0x00, 0xBC);	//Set Green Gamma Curve Related Setting
	s3cfb_hx5118_spi_write(0x01, 0x13);
	s3cfb_hx5118_spi_write(0x01, 0x13);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x47);
	s3cfb_hx5118_spi_write(0x01, 0x13);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x47);
	s3cfb_hx5118_spi_write(0x01, 0x13);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x22);
	s3cfb_hx5118_spi_write(0x01, 0x38);
	s3cfb_hx5118_spi_write(0x01, 0x38);
	msleep(5);

	s3cfb_hx5118_spi_write(0x00, 0xBD);	//Set Blue Gamma Curve Related Setting
	s3cfb_hx5118_spi_write(0x01, 0x14);
	s3cfb_hx5118_spi_write(0x01, 0x14);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x47);
	s3cfb_hx5118_spi_write(0x01, 0x13);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x47);
	s3cfb_hx5118_spi_write(0x01, 0x13);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x00);
	s3cfb_hx5118_spi_write(0x01, 0x22);
	s3cfb_hx5118_spi_write(0x01, 0x38);
	s3cfb_hx5118_spi_write(0x01, 0x38);
	msleep(10);


	s3cfb_hx5118_spi_write(0x00, 0x29);	//display on
	s3cfb_hx5118_spi_write(0x00, 0x00);	//NOP
}

int s3cfb_set_gpio(void)
{
	int err = 0;
	unsigned long val;
	int i;

	/* Must be '0' for Normal-path instead of By-pass */
	writel(0x0, S3C_HOSTIFB_MIFPCON);

	/* select TFT LCD type (RGB I/F) */
	val = readl(S3C64XX_SPC_BASE);
	val &= ~0x3;
	val |= (1 << 0);
	writel(val, S3C64XX_SPC_BASE);

	/* VD */
	for (i = 0; i < 16; i++)
		s3c_gpio_cfgpin(S3C64XX_GPI(i), S3C_GPIO_SFN(2));

	for (i = 0; i < 12; i++)
		s3c_gpio_cfgpin(S3C64XX_GPJ(i), S3C_GPIO_SFN(2));
	
	if (gpio_is_valid(MEIZU_M8_SPI_CLK)) {
		err = gpio_request(MEIZU_M8_SPI_CLK, "GPQ");
		if (err) goto err_clk;
		else{
			gpio_direction_output(MEIZU_M8_SPI_CLK,1);
			s3c_gpio_setpull(MEIZU_M8_SPI_CLK,S3C_GPIO_PULL_NONE);
		}
	} else {
		err = 1;
		goto err_clk;
	}

	if (gpio_is_valid(MEIZU_M8_SPI_SI)) {
		err = gpio_request(MEIZU_M8_SPI_SI, "GPQ");
		if (err) goto err_si;
		else{
			gpio_direction_output(MEIZU_M8_SPI_SI,1);
			s3c_gpio_setpull(MEIZU_M8_SPI_SI,S3C_GPIO_PULL_NONE);
		}
	} else {
		err = 1;
		goto err_si;
	}

	if (gpio_is_valid(MEIZU_M8_SPI_CS)) {
		err = gpio_request(MEIZU_M8_SPI_CS, "GPQ");
		if (err) goto err_cs;
		else{
			gpio_direction_output(MEIZU_M8_SPI_CS,1);
			s3c_gpio_setpull(MEIZU_M8_SPI_CS,S3C_GPIO_PULL_NONE);
		}
	} else {
		err = 1;
		goto err_cs;
	}

	if (gpio_is_valid(MEIZU_M8_SPI_SO)) {
		err = gpio_request(MEIZU_M8_SPI_SO, "GPQ");
		if (err) goto err_so;
		else{
			gpio_direction_input(MEIZU_M8_SPI_SO);
			s3c_gpio_setpull(MEIZU_M8_SPI_SO,S3C_GPIO_PULL_NONE);
		}
	} else {
		err = 1;
		goto err_so;
	}
	
	s3cfb_hx5118_reset();
	s3cfb_init_hx5118();

	gpio_free(MEIZU_M8_SPI_CLK);
	gpio_free(MEIZU_M8_SPI_SI);
	gpio_free(MEIZU_M8_SPI_CS);
	gpio_free(MEIZU_M8_SPI_SO);
	gpio_free(MEIZU_M8_SPI_RST);

err_so:
	gpio_free(MEIZU_M8_SPI_CS);
err_cs:
	gpio_free(MEIZU_M8_SPI_SI);
err_si:
	gpio_free(MEIZU_M8_SPI_CLK);
err_clk:
	return err;
}

int lcd_power_ctrl(s32 value)
{
	int err;

	if (value == lcd_power)
		return 0;

	if (value) {	
		s3cfb_hx5118_spi_write(0x00, 0x01); //SWRESET
		msleep(120);
		s3cfb_init_hx5118();
	}
	else {
		s3cfb_hx5118_spi_write(0x00, 0x28); //display off
		s3cfb_hx5118_spi_write(0x00, 0x00); //NOP
		s3cfb_hx5118_spi_write(0x00, 0x10); //SLEEP IN
		msleep(120);
	}
	lcd_power = value;
	return 0;
}

static void backlight_ctrl(s32 value)
{
	int err, ret;

	if (value) {
		/* backlight ON */
		ret = lcd_power_ctrl(ON);
		if (ret != 0) {
			printk(KERN_ERR "lcd power on control is failed\n");
			return;
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
		/* backlight OFF */
		if (gpio_is_valid(S3C64XX_GPF(15))) {
			err = gpio_request(S3C64XX_GPF(15), "GPF");

			if (err) {
				printk(KERN_ERR "failed to request GPF for "
					"lcd backlight control\n");
			}

			gpio_direction_output(S3C64XX_GPF(15), 0);
		 }
		ret = lcd_power_ctrl(OFF);
		if (ret != 0) {
			printk(KERN_ERR "lcd power off control is failed\n");
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

void s3cfb_init_hw(void)
{
	printk(KERN_INFO "LCD TYPE :: LMS340KC01 will be initialized\n");

	s3cfb_set_fimd_info();
//	s3cfb_set_gpio();
	
	s3cfb_set_brightness(255/3);
}

