/* linux/drivers/input/keyboard/s3c-keypad.h 
 *
 * Driver header for Samsung SoC keypad.
 *
 * Kim Kyoungil, Copyright (c) 2006-2009 Samsung Electronics
 *      http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef _S3C_KEYPAD_H_
#define _S3C_KEYPAD_H_

static void __iomem *key_base;

#define GET_KEYCODE(x)                  (x+1)

#define KEYCODE_DUMPKEY	247
#define KEYCODE_SENDEND	248
#define KEYCODE_ENDCALL	249
#define KEYCODE_FOCUS	250

#define KEYPAD_ROW_GPIOCON      S3C64XX_GPKCON1
#define KEYPAD_ROW_GPIOPUD      S3C64XX_GPKPUD
#define KEYPAD_COL_GPIOCON      S3C64XX_GPLCON
#define KEYPAD_COL_GPIOPUD      S3C64XX_GPLPUD

#define KEYPAD_DELAY		(50)
#define	KEYIFCOL_CLEAR		(readl(key_base+S3C_KEYIFCOL) & ~0xffff)
#define	KEYIFCON_CLEAR		(readl(key_base+S3C_KEYIFCON) & ~0x1f)
#define KEYIFFC_DIV		(readl(key_base+S3C_KEYIFFC) | 0x1)

struct s3c_keypad_slide
{
        int     eint;
        int     gpio;
        int     gpio_af;
        int     state_upset;
};

struct s3c_keypad_special_key
{
        int     mask_low;
        int     mask_high;
        int     keycode;
};
 
struct s3c_keypad_gpio_key
{
        int     eint;
        int     gpio;
        int     gpio_af;
	int	keycode;
	int     state_upset;
};

struct s3c_keypad_extra 
{
	int 				board_num;
	struct s3c_keypad_slide		*slide;
	struct s3c_keypad_special_key	*special_key;
	int				special_key_num;
	struct s3c_keypad_gpio_key	*gpio_key;
	int				gpio_key_num;
	int				wakeup_by_keypad;
};

struct s3c_keypad {
	struct input_dev *dev;
	struct s3c_keypad_extra *extra;
};

extern void s3c_setup_keypad_cfg_gpio(int rows, int columns);

#endif				/* _S3C_KEYIF_H_ */
