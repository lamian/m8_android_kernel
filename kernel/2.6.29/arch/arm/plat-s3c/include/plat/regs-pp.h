/* linux/include/asm-arm/arch-s3c2410/regs-hsmmc.h
 *
 * Copyright (c) 2004 Samsung Electronics 
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * S3C HSMMC Controller
*/

#ifndef __ASM_ARCH_REGS_PP_H
#define __ASM_ARCH_REGS_PP_H __FILE__

#define S3C_VPP(x)	((x))

#define S3C_VPP_MODE 				S3C_VPP(0x00)
#define S3C_VPP_PRESCALE_RATIO 		S3C_VPP(0x04)
#define S3C_VPP_PRESCALEIMGSIZE 	S3C_VPP(0x08)
#define S3C_VPP_SRCIMGSIZE 			S3C_VPP(0x0C)
#define S3C_VPP_MAINSCALE_H_RATIO 	S3C_VPP(0x10)
#define S3C_VPP_MAINSCALE_V_RATIO 	S3C_VPP(0x14)
#define S3C_VPP_DSTIMGSIZE 			S3C_VPP(0x18)
#define S3C_VPP_PRESCALE_SHFACTOR 	S3C_VPP(0x1C)
#define S3C_VPP_ADDRSTART_Y 		S3C_VPP(0x20)
#define S3C_VPP_ADDRSTART_CB 		S3C_VPP(0x24)
#define S3C_VPP_ADDRSTART_CR 		S3C_VPP(0x28)
#define S3C_VPP_ADDRSTART_RGB 		S3C_VPP(0x2C)
#define S3C_VPP_ADDREND_Y 			S3C_VPP(0x30)
#define S3C_VPP_ADDREND_CB 			S3C_VPP(0x34)
#define S3C_VPP_ADDREND_CR 			S3C_VPP(0x38)
#define S3C_VPP_ADDREND_RGB 		S3C_VPP(0x3C)	
#define S3C_VPP_OFFSET_Y 			S3C_VPP(0x40)
#define S3C_VPP_OFFSET_CB 			S3C_VPP(0x44)
#define S3C_VPP_OFFSET_CR 			S3C_VPP(0x48)
#define S3C_VPP_OFFSET_RGB 			S3C_VPP(0x4C)	
#define S3C_VPP_NXTADDRSTART_Y 		S3C_VPP(0x54)
#define S3C_VPP_NXTADDRSTART_CB 	S3C_VPP(0x58)
#define S3C_VPP_NXTADDRSTART_CR 	S3C_VPP(0x5C)
#define S3C_VPP_NXTADDRSTART_RGB 	S3C_VPP(0x60)
#define S3C_VPP_NXTADDREND_Y 		S3C_VPP(0x64)
#define S3C_VPP_NXTADDREND_CB 		S3C_VPP(0x68)
#define S3C_VPP_NXTADDREND_CR 		S3C_VPP(0x6C)
#define S3C_VPP_NXTADDREND_RGB 		S3C_VPP(0x70)	
#define S3C_VPP_ADDRSTART_OCB 		S3C_VPP(0x74)
#define S3C_VPP_ADDRSTART_OCR 		S3C_VPP(0x78)
#define S3C_VPP_ADDREND_OCB 		S3C_VPP(0x7C)
#define S3C_VPP_ADDREND_OCR 		S3C_VPP(0x80)
#define S3C_VPP_OFFSET_OCB 			S3C_VPP(0x84)
#define S3C_VPP_OFFSET_OCR 			S3C_VPP(0x88)
#define S3C_VPP_NXTADDRSTART_OCB 	S3C_VPP(0x8C)
#define S3C_VPP_NXTADDRSTART_OCR 	S3C_VPP(0x90)
#define S3C_VPP_NXTADDREND_OCB 		S3C_VPP(0x94)
#define S3C_VPP_NXTADDREND_OCR 		S3C_VPP(0x98)
#define S3C_VPP_POSTENVID 			S3C_VPP(0x9C)
#define S3C_VPP_MODE_2 				S3C_VPP(0xA0)


/*************************************************************************
 * Macro part
 ************************************************************************/
#define S3C_PP_POSTENVID(x)			((x) << 31)
#define S3C_PP_INT_ENABLE(x)			((x) << 7)

#define S3C_PP_PRE_RATIO_V(x)			((x) << 7)
#define S3C_PP_PRE_RATIO_H(x)			((x) << 0)

#define S3C_PP_PRE_DST_H(x)			((x) << 12)
#define S3C_PP_PRE_DST_W(x)			((x) << 0)

#define S3C_PP_SRC_H(x)				((x) << 12)
#define S3C_PP_SRC_W(x)				((x) << 0)

#define S3C_PP_DST_H(x)				((x) << 12)
#define S3C_PP_DST_W(x)				((x) << 0)

#define S3C_PP_CLKSEL_F(x)			((x) << 21)
#define S3C_PP_CLKDIR(x)			((x) << 23)
#define S3C_PP_CLKVAL_F(x)			((x) << 24)

/*************************************************************************
 * Bit definition part
 ************************************************************************/
//POSTENVID
#define S3C_POSTENVID_ENABLE	(0x1<<31) // khlee
#define S3C_POSTENVID_DISABLE	(0x0<<31) 

//MODE Control register
#define S3C_MODE_AUTOLOAD_ENABLE         (0x1<<14)
#define S3C_MODE_POST_INT_ENABLE         (0x1<<7)
#define S3C_MODE_POST_PENDING            (0x1<<6)
#define S3C_MODE_IRQ_LEVEL               (0x1<<5)
#define S3C_MODE_H_CLK_INPUT              (0x0<<2)
#define S3C_MODE_EXT_CLK_0_INPUT          (0x1<<2)
#define S3C_MODE_EXT_CLK_1_INPUT          (0x3<<2)
#define S3C_PP_MODE_R2YSEL_WIDE			(0x1<<16)
#define S3C_PP_MODE_NARROW			(0x1<<10)
#define S3C_PP_MODE_WIDE			(0x2<<10)
#define S3C_PP_MODE_CLKSEL_F_MASK		(0x3<<21)
#define S3C_PP_MODE_CLKDIR_MASK			(0x1<<23)
#define S3C_PP_MODE_CLKVAL_F_MASK		(0x3F<<24)
#define S3C_PP_MODE_INTERLACE			(0x1<<12)
#define S3C_PP_MODE_FIFO			(0x1<<13)
#define S3C_PP_MODE_FREERUN			(0x1<<14)
#define S3C_PP_POSTENVID_ENABLE			(0x1<<31)
#define S3C_PP_POSTENVID_DISABLE		(0x0<<31)
#define S3C_PP_MODE_FORMAT_MASK			(0x1E811F)

//MODE Control register 2
#define S3C_MODE2_ADDR_CHANGE_ENABLE      (0x0<<4)
#define S3C_MODE2_ADDR_CHANGE_DISABLE     (0x1<<4)
#define S3C_MODE2_CHANGE_AT_FIELD_END     (0x0<<3)
#define S3C_MODE2_CHANGE_AT_FRAME_END     (0x1<<3)
#define S3C_MODE2_SOFTWARE_TRIGGER        (0x0<<0)
#define S3C_MODE2_HARDWARE_TRIGGER        (0x1<<0)

#endif /* __ASM_ARCH_REGS_HSMMC_H */
