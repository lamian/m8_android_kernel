/* linux/include/asm-arm/arch-s3c2410/regs-tvscaler.h
 *
 * Copyright (c) 2007 Samsung Electronics
 *		      http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * S3C6400 TV SCALER register definitions
*/

#ifndef __ASM_ARM_REGS_TVSCALER
#define __ASM_ARM_REGS_TVSCALER "regs-tvscaler.h"

#define S3C_MODE		(0x00)
#define S3C_PRESCALE_RATIO 	(0x04)
#define S3C_PRESCALEIMGSIZE 	(0x08)
#define S3C_SRCIMGSIZE 		(0x0C)
#define S3C_MAINSCALE_H_RATIO 	(0x10)
#define S3C_MAINSCALE_V_RATIO 	(0x14)
#define S3C_DSTIMGSIZE 		(0x18)
#define S3C_PRESCALE_SHFACTOR 	(0x1C)
#define S3C_ADDRSTART_Y 	(0x20)
#define S3C_ADDRSTART_CB 	(0x24)
#define S3C_ADDRSTART_CR 	(0x28)
#define S3C_ADDRSTART_RGB 	(0x2C)
#define S3C_ADDREND_Y 		(0x30)
#define S3C_ADDREND_CB 		(0x34)
#define S3C_ADDREND_CR 		(0x38)
#define S3C_ADDREND_RGB 	(0x3C)	
#define S3C_OFFSET_Y 		(0x40)
#define S3C_OFFSET_CB 		(0x44)
#define S3C_OFFSET_CR 		(0x48)
#define S3C_OFFSET_RGB 		(0x4C)	
#define S3C_NXTADDRSTART_Y 	(0x54)
#define S3C_NXTADDRSTART_CB 	(0x58)
#define S3C_NXTADDRSTART_CR 	(0x5C)
#define S3C_NXTADDRSTART_RGB 	(0x60)
#define S3C_NXTADDREND_Y 	(0x64)
#define S3C_NXTADDREND_CB 	(0x68)
#define S3C_NXTADDREND_CR 	(0x6C)
#define S3C_NXTADDREND_RGB 	(0x70)	
#define S3C_ADDRSTART_OCB 	(0x74)
#define S3C_ADDRSTART_OCR 	(0x78)
#define S3C_ADDREND_OCB 	(0x7C)
#define S3C_ADDREND_OCR 	(0x80)
#define S3C_OFFSET_OCB 		(0x84)
#define S3C_OFFSET_OCR 		(0x88)
#define S3C_NXTADDRSTART_OCB 	(0x8C)
#define S3C_NXTADDRSTART_OCR 	(0x90)
#define S3C_NXTADDREND_OCB 	(0x94)
#define S3C_NXTADDREND_OCR 	(0x98)
#define S3C_POSTENVID 		(0x9C)
#define S3C_MODE2 		(0xA0)

/*************************************************************************
 * Bit definition part
 ************************************************************************/
//POSTENVID
#define S3C_POSTENVID_ENABLE			(0x1<<31)
#define S3C_POSTENVID_DISABLE			(0x0<<31)

//MODE Control register
#define S3C_MODE_AUTOLOAD_ENABLE		(0x1<<14)
#define S3C_MODE_TV_INT_ENABLE			(0x1<<7)
#define S3C_MODE_TV_PENDING			(0x1<<6)
#define S3C_MODE_IRQ_LEVEL			(0x1<<5)
#define S3C_MODE_H_CLK_INPUT			(0x0<<2)
#define S3C_MODE_EXT_CLK_0_INPUT		(0x1<<2)
#define S3C_MODE_EXT_CLK_1_INPUT		(0x3<<2)
#define S3C_TV_MODE_R2YSEL_WIDE			(0x1<<16)
#define S3C_TV_MODE_NARROW			(0x1<<10)
#define S3C_TV_MODE_WIDE			(0x2<<10)
#define S3C_TV_MODE_CLKSEL_F_MASK		(0x3<<21)
#define S3C_TV_MODE_CLKDIR_MASK			(0x1<<23)
#define S3C_TV_MODE_CLKVAL_F_MASK		(0x3F<<24)
#define S3C_TV_MODE_INTERLACE			(0x1<<12)
#define S3C_TV_MODE_FIFO			(0x1<<13)
#define S3C_TV_MODE_FREERUN			(0x1<<14)
#define S3C_TV_POSTENVID_ENABLE			(0x1<<31)
#define S3C_TV_POSTENVID_DISABLE		(0x0<<31)
#define S3C_TV_MODE_FORMAT_MASK			(0x1E811F)

//MODE Control register 2
#define S3C_MODE2_ADDR_CHANGE_ENABLE      (0x0<<4)
#define S3C_MODE2_ADDR_CHANGE_DISABLE     (0x1<<4)
#define S3C_MODE2_CHANGE_AT_FIELD_END     (0x0<<3)
#define S3C_MODE2_CHANGE_AT_FRAME_END     (0x1<<3)
#define S3C_MODE2_SOFTWARE_TRIGGER        (0x0<<0)
#define S3C_MODE2_HARDWARE_TRIGGER        (0x1<<0)

/*************************************************************************
 * Macro part
 ************************************************************************/
#define S3C_TV_POSTENVID(x)			((x) << 31)
#define S3C_TV_INT_ENABLE(x)			((x) << 7)

#define S3C_TV_PRE_RATIO_V(x)			((x) << 7)
#define S3C_TV_PRE_RATIO_H(x)			((x) << 0)

#define S3C_TV_PRE_DST_H(x)			((x) << 12)
#define S3C_TV_PRE_DST_W(x)			((x) << 0)

#define S3C_TV_SRC_H(x)				((x) << 12)
#define S3C_TV_SRC_W(x)				((x) << 0)

#define S3C_TV_DST_H(x)				((x) << 12)
#define S3C_TV_DST_W(x)				((x) << 0)

#define S3C_TV_CLKSEL_F(x)			((x) << 21)
#define S3C_TV_CLKDIR(x)			((x) << 23)
#define S3C_TV_CLKVAL_F(x)			((x) << 24)

#endif /* __ASM_ARM_REGS_TVSCALER */
