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


#ifndef _S3C_KEYPAD_BOARD_H_
#define _S3C_KEYPAD_BOARD_H_

#include <mach/gpio.h>
static void __iomem *key_base;

#define KEYPAD_COLUMNS	(8)	/* GPIO_KEYSENSE_0 ~ GPIO_KEYSENSE_3 */
#define KEYPAD_ROWS	(8)	/* GPIO_KEYSCAN_0 ~ GPIO_KEYSCAN_3 */
#define MAX_KEYPAD_NR   (KEYPAD_COLUMNS * KEYPAD_ROWS)
#define MAX_KEYMASK_NR	32

#define KEYCODE_HOME	251

struct s3c_keypad_slide slide_smdk6410 = {IRQ_EINT(11), S3C64XX_GPN(11), 2, 1};

struct s3c_keypad_special_key special_key_smdk6410[] = {
	{0x00000000, 0x00000200, KEYCODE_FOCUS},
};

struct s3c_keypad_gpio_key gpio_key_smdk6410[] = {
	{IRQ_EINT(10),  S3C64XX_GPN(10),   2,      KEYCODE_ENDCALL, 1},
	{IRQ_EINT(9),  S3C64XX_GPN(9),   2,      KEYCODE_HOME, 1},
};

struct s3c_keypad_extra s3c_keypad_extra[] = {
	{0x0000, &slide_smdk6410, &special_key_smdk6410[0], 1,  &gpio_key_smdk6410[0], 2, 0},
};

#endif				/* _S3C_KEYIF_H_ */
