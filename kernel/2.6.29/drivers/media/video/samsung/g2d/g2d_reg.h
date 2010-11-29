//
// Copyright (c) Samsung Electronics. Co. LTD.  All rights reserved.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
--*/
/**
*	@file		g2d_reg.h
*	@brief	Defines the FIMGSE-2D Graphics Accerlerator's register layout and definitions.
*	@author	Jiwon Kim
*	
*	@note This version is made for S3C6410
*	@note IP version is v2.0
*/

#ifndef __G2D_REG_H__
#define __G2D_REG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <linux/types.h>

//------------------------------------------------------------------------------
//  Type: G2D_REG
//
//  2D Graphics Acclerator registers. This register bank is located by the constant
//  <CPU>_BASE_REG_XX_2DGRAPHICS in the configuration file <cpu>_base_regs.h.
//

typedef struct _reg_g2d_v2
{
	// General Registers
	volatile u32 CONTROL;						//   	(G2D_BASE+0x00)
	volatile u32 INTEN;							//    (G2D_BASE+0x04)
	volatile u32 FIFO_INTC;			//    (G2D_BASE+0x08)
	volatile u32 INTC_PEND;					//		(G2D_BASE+0x0c)
	volatile u32 FIFO_STATUS;		//    (G2D_BASE+0x10)
	volatile u32 PAD1[0x3B];

	// Command Registers
	volatile u32 CMDR0;							//    (G2D_BASE+0x100)
	volatile u32 CMDR1;							//    (G2D_BASE+0x104)
	volatile u32 CMDR2;							//    (G2D_BASE+0x108)
	volatile u32 CMDR3;							//    (G2D_BASE+0x10c)
	volatile u32 CMDR4;							//    (G2D_BASE+0x110)
	volatile u32 CMDR5;							//    (G2D_BASE+0x114)
	volatile u32 CMDR6;							//    (G2D_BASE+0x118)
	volatile u32 CMDR7;							//    (G2D_BASE+0x11c)

	volatile u32 PAD2[0x38];
	// Common Resource Registers
	volatile u32 SRC_RES;						//    (G2D_BASE+0x200)
	volatile u32 SRC_HORI_RES;			//    (G2D_BASE+0x204)
	volatile u32 SRC_VERT_RES;			//    (G2D_BASE+0x208)
	volatile u32 PAD3[1];
	volatile u32 SC_RES;						//		(G2D_BASE+0x210)
	volatile u32 SC_HORI_RES;				//    (G2D_BASE+0x214)
	volatile u32 SC_VERT_RES;				//   	(G2D_BASE+0x218)
	volatile u32 PAD4[1];
	volatile u32 CW_LEFT_TOP;				//    (G2D_BASE+0x220)
	volatile u32 CW_LEFT_TOP_X;			//   	(G2D_BASE+0x224)
	volatile u32 CW_LEFT_TOP_Y;			//   	(G2D_BASE+0x228)
	volatile u32 PAD5[1];
	volatile u32 CW_RIGHT_BOTTOM;		//    (G2D_BASE+0x230)
	volatile u32 CW_RIGHT_BOTTOM_X;	//   	(G2D_BASE+0x234)
	volatile u32 CW_RIGHT_BOTTOM_Y; //   	(G2D_BASE+0x238)
	volatile u32 PAD6[0x31];
	volatile u32 COORD0;						//   	(G2D_BASE+0x300)
	volatile u32 COORD0_X;					//    (G2D_BASE+0x304)
	volatile u32 COORD0_Y;					//    (G2D_BASE+0x308)
	volatile u32 PAD7[1];
	volatile u32 COORD1;						//   	(G2D_BASE+0x310)
	volatile u32 COORD1_X;					//    (G2D_BASE+0x314)
	volatile u32 COORD1_Y;					//    (G2D_BASE+0x318)
	volatile u32 PAD8[1];
	volatile u32 COORD2;						//   	(G2D_BASE+0x320)
	volatile u32 COORD2_X;					//    (G2D_BASE+0x324)
	volatile u32 COORD2_Y;					//    (G2D_BASE+0x328)
	volatile u32 PAD9[1];
	volatile u32 COORD3;						//   	(G2D_BASE+0x330)
	volatile u32 COORD3_X;					//    (G2D_BASE+0x334)
	volatile u32 COORD3_Y;					//    (G2D_BASE+0x338)
	volatile u32 PAD10[1];
	volatile u32 ROT_OC;						//   	(G2D_BASE+0x340)
	volatile u32 ROT_OC_X;					//    (G2D_BASE+0x344)
	volatile u32 ROT_OC_Y;					//    (G2D_BASE+0x348)
	volatile u32 ROT_MODE;					//    (G2D_BASE+0x34c)
	volatile u32 ENDIAN;						//   	(G2D_BASE+0x350)
	volatile u32 PAD11[0x2b];
	volatile u32 X_INCR;						//    (G2D_BASE+0x400)
	volatile u32 Y_INCR;						//    (G2D_BASE+0x404)
	volatile u32 PAD12[2];
	volatile u32 ROP;								//   	(G2D_BASE+0x410)
	volatile u32 PAD13[3];
	volatile u32 ALPHA;							//    (G2D_BASE+0x420)
	volatile u32 PAD14[0x37];

	volatile u32 FG_COLOR;					//    (G2D_BASE+0x500)
	volatile u32 BG_COLOR;					//    (G2D_BASE+0x504)
	volatile u32 BS_COLOR;					//    (G2D_BASE+0x508)
	volatile u32 PAD141[0x1];	
	volatile u32 SRC_COLOR_MODE;		//		(G2D_BASE+0x510)
	volatile u32 DST_COLOR_MODE;		//		(G2D_BASE+0x514)
	volatile u32 PAD15[0x3a];

	volatile u32 PATTERN_ADDR[0x20];//    (G2D_BASE+0x600~0x67c)
	volatile u32 PAD16[0x20];
	volatile u32 PAT_OFF_XY;				//   	(G2D_BASE+0x700)
	volatile u32 PAT_OFF_X;					//    (G2D_BASE+0x704)
	volatile u32 PAT_OFF_Y;					//    (G2D_BASE+0x708)                                      																																								
	volatile u32 PAD17[5];
	volatile u32 COLORKEY_CNTL;			//    (G2D_BASE+0x720)	
	volatile u32 COLORKEY_DR_MIN;		//    (G2D_BASE+0x724)
	volatile u32 COLORKEY_DR_MAX;		//    (G2D_BASE+0x728)

	volatile u32 PAD18[1];
	volatile u32 SRC_BASE_ADDR;			//    (G2D_BASE+0x730)
	volatile u32 DST_BASE_ADDR;			//    (G2D_BASE+0x734)	

} G2D_REG, *PG2D_REG;

 
#ifdef __cplusplus
}
#endif


#endif /*__G2D_REG_H__*/
