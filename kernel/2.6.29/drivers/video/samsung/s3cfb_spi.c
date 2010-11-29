/*
 * drivers/video/s3c/s3c24xxfb_spi.c
 *
 * $Id: s3cfb_spi.c,v 1.1 2008/11/17 11:12:08 jsgood Exp $
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

#include <linux/delay.h>

#include <asm/mach/map.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-clock.h>
#include <plat/regs-lcd.h>

#if 0 //defined(CONFIG_PLAT_S3C24XX)

#define S3C_FB_SPI_CLK(x)	(S3C2443_GPL10 + (ch * 0))
#define S3C_FB_SPI_MOSI(x)	(S3C2443_GPL11 + (ch * 0))
#define S3C_FB_SPI_CS(x)	(S3C2443_GPL14 + (ch * 0))

static inline void s3cfb_spi_lcd_dclk(int ch, int value)
{
	s3c2410_gpio_setpin(S3C_FB_SPI_CLK(ch), value);
}

static inline void s3cfb_spi_lcd_dseri(int ch, int value)
{
	s3c2410_gpio_setpin(S3C_FB_SPI_MOSI(ch), value);
}

static inline void s3cfb_spi_lcd_den(int ch, int value)
{
	s3c2410_gpio_setpin(S3C_FB_SPI_CS(ch), value);
}

static inline void s3cfb_spi_set_lcd_data(int ch)
{
	s3c2410_gpio_cfgpin(S3C_FB_SPI_CLK(ch), 1);
	s3c2410_gpio_cfgpin(S3C_FB_SPI_MOSI(ch), 1);
	s3c2410_gpio_cfgpin(S3C_FB_SPI_CS(ch), 1);

	s3c2410_gpio_pullup(S3C_FB_SPI_CLK(ch), 2);
	s3c2410_gpio_pullup(S3C_FB_SPI_MOSI(ch), 2);
	s3c2410_gpio_pullup(S3C_FB_SPI_CS(ch), 2);
}

#elif defined(CONFIG_PLAT_S3C64XX)

#define S3C_FB_SPI_CLK(x)	(S3C64XX_GPC(1 + (ch * 4)))
#define S3C_FB_SPI_MOSI(x)	(S3C64XX_GPC(2 + (ch * 4)))
#define S3C_FB_SPI_CS(x)	(S3C64XX_GPC(3 + (ch * 4)))

int s3cfb_spi_gpio_request(int ch)
{
	int err = 0;

	if (gpio_is_valid(S3C_FB_SPI_CLK(ch))) {
		err = gpio_request(S3C_FB_SPI_CLK(ch), "GPC");

		if (err)
			goto err_clk;
	} else {
		err = 1;
		goto err_clk;
	}

	if (gpio_is_valid(S3C_FB_SPI_MOSI(ch))) {
		err = gpio_request(S3C_FB_SPI_MOSI(ch), "GPC");

		if (err)
			goto err_mosi;
	} else {
		err = 1;
		goto err_mosi;
	}

	if (gpio_is_valid(S3C_FB_SPI_CS(ch))) {
		err = gpio_request(S3C_FB_SPI_CS(ch), "GPC");

		if (err)
			goto err_cs;
	} else {
		err = 1;
		goto err_cs;
	}

err_cs:
	gpio_free(S3C_FB_SPI_MOSI(ch));

err_mosi:
	gpio_free(S3C_FB_SPI_CLK(ch));

err_clk:
	return err;
	
}

inline void s3cfb_spi_lcd_dclk(int ch, int value)
{
	gpio_set_value(S3C_FB_SPI_CLK(ch), value);
}

inline void s3cfb_spi_lcd_dseri(int ch, int value)
{
	gpio_set_value(S3C_FB_SPI_MOSI(ch), value);
}

inline void s3cfb_spi_lcd_den(int ch, int value)
{
	gpio_set_value(S3C_FB_SPI_CS(ch), value);
}

inline void s3cfb_spi_set_lcd_data(int ch)
{
	gpio_direction_output(S3C_FB_SPI_CLK(ch), 1);
	gpio_direction_output(S3C_FB_SPI_MOSI(ch), 1);
	gpio_direction_output(S3C_FB_SPI_CS(ch), 1);

	s3c_gpio_setpull(S3C_FB_SPI_CLK(ch), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C_FB_SPI_MOSI(ch), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C_FB_SPI_CS(ch), S3C_GPIO_PULL_NONE);
}

void s3cfb_spi_gpio_free(int ch)
{
	gpio_free(S3C_FB_SPI_CLK(ch));
	gpio_free(S3C_FB_SPI_MOSI(ch));
	gpio_free(S3C_FB_SPI_CS(ch));
}

#elif 0 //defined(CONFIG_PLAT_S5PC1XX)

#define S5P_FB_SPI_MISO(x)	(S5P_GPB0 + (ch * 4))
#define S5P_FB_SPI_CLK(x)	(S5P_GPB1 + (ch * 4))
#define S5P_FB_SPI_MOSI(x)	(S5P_GPB2 + (ch * 4))
#define S5P_FB_SPI_nSS(x)	(S5P_GPB3 + (ch * 4))

inline void s3cfb_spi_lcd_dclk(int ch, int value)
{
	gpio_set_value(S5P_FB_SPI_CLK(ch), value);
}

inline void s3cfb_spi_lcd_dseri(int ch, int value)
{
	gpio_set_value(S5P_FB_SPI_MOSI(ch), value);
}

inline void s3cfb_spi_lcd_den(int ch, int value)
{
	gpio_set_value(S5P_FB_SPI_nSS(ch), value);
}

inline void s3cfb_spi_set_lcd_data(int ch)
{
	gpio_direction_output(S5P_FB_SPI_CLK(ch), 1);
	gpio_direction_output(S5P_FB_SPI_MOSI(ch), 1);
	gpio_direction_output(S5P_FB_SPI_nSS(ch), 1);

	gpio_pullup(S5P_FB_SPI_CLK(ch), S5P_GPIO_PUD_DISABLE);
	gpio_pullup(S5P_FB_SPI_MOSI(ch), S5P_GPIO_PUD_DISABLE);
	gpio_pullup(S5P_FB_SPI_nSS(ch), S5P_GPIO_PUD_DISABLE);
}

#endif

