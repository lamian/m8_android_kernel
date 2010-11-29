/* linux/arch/arm/plat-s3c64xx/include/plat/regs-keypad.h
 *
 *
 * S3C6410 Key Interface register definitions
 *
 * Kim Kyoungil, Copyright (c) 2006-2009 Samsung Electronics
 *      http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef __ASM_ARCH_REGS_KEYPAD_H
#define __ASM_ARCH_REGS_KEYPAD_H

/* 
 * Keypad Interface
 */
#define S3C_KEYPADREG(x)	(x)

#define S3C_KEYIFCON		S3C_KEYPADREG(0x00)
#define S3C_KEYIFSTSCLR		S3C_KEYPADREG(0x04)
#define S3C_KEYIFCOL		S3C_KEYPADREG(0x08)
#define S3C_KEYIFROW		S3C_KEYPADREG(0x0C)
#define S3C_KEYIFFC		S3C_KEYPADREG(0x10)

#define KEYCOL_DMASK		(0xff)
#define KEYROW_DMASK		(0xff)
#define	INT_F_EN		(1<<0)	/*falling edge(key-pressed) interuppt enable*/
#define	INT_R_EN		(1<<1)	/*rising edge(key-released) interuppt enable*/
#define	DF_EN			(1<<2)	/*debouncing filter enable*/
#define	FC_EN			(1<<3)	/*filter clock enable*/
#define	KEYIFCON_INIT		(KEYIFCON_CLEAR |INT_F_EN|INT_R_EN|DF_EN|FC_EN)
#define KEYIFSTSCLR_CLEAR	(0xffff)

#endif /* __ASM_ARCH_REGS_KEYPAD_H */


