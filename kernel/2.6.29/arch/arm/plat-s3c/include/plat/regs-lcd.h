/* linux/arch/arm/plat-s3c/include/plat/regs-lcd.h
 *
 * Copyright (c) 2003 Simtec Electronics <linux@simtec.co.uk>
 *		      http://www.simtec.co.uk/products/SWLINUX/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/


#ifndef ___ASM_ARCH_REGS_LCD_H
#define ___ASM_ARCH_REGS_LCD_H

#include <plat/map-base.h>

/***************************************************************************/
/* LCD Registers for S3C2443/2450/S3C6400/6410 */
#define S3C_LCDREG(x)		((x) + S3C_VA_LCD)

/* LCD control registers */
#define S3C_VIDCON0		S3C_LCDREG(0x00)  	/* Video control 0 register */
#define S3C_VIDCON1		S3C_LCDREG(0x04)  	/* Video control 1 register */

#if defined(CONFIG_CPU_S3C2443)||defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
#define S3C_VIDTCON0		S3C_LCDREG(0x08)  	/* LCD CONTROL 1 */
#define S3C_VIDTCON1		S3C_LCDREG(0x0C)  	/* LCD CONTROL 1 */
#define S3C_VIDTCON2		S3C_LCDREG(0x10) 	/* LCD CONTROL 1 */
#define S3C_WINCON0		S3C_LCDREG(0x14)  	/* LCD CONTROL 1 */
#define S3C_WINCON1		S3C_LCDREG(0x18)  	/* LCD CONTROL 1 */
#define S3C_VIDOSD0A		S3C_LCDREG(0x28)  	/* LCD CONTROL 1 */
#define S3C_VIDOSD0B		S3C_LCDREG(0x2C)  	/* LCD CONTROL 1 */
#define S3C_VIDOSD0C		S3C_LCDREG(0x30)  	/* LCD CONTROL 1 */
#define S3C_VIDOSD1A		S3C_LCDREG(0x34)  	/* LCD CONTROL 1 */
#define S3C_VIDOSD1B		S3C_LCDREG(0x38)  	/* LCD CONTROL 1 */
#define S3C_VIDOSD1C		S3C_LCDREG(0x3C)  	/* LCD CONTROL 1 */
#define S3C_VIDW00ADD0B0	S3C_LCDREG(0x64)  	/* LCD CONTROL 1 */
#define S3C_VIDW00ADD0B1	S3C_LCDREG(0x68)  	/* LCD CONTROL 1 */
#define S3C_VIDW01ADD0		S3C_LCDREG(0x6C)  	/* LCD CONTROL 1 */
#define S3C_VIDW00ADD1B0	S3C_LCDREG(0x7C)  	/* LCD CONTROL 1 */
#define S3C_VIDW00ADD1B1	S3C_LCDREG(0x80)  	/* LCD CONTROL 1 */
#define S3C_VIDW01ADD1		S3C_LCDREG(0x84)  	/* LCD CONTROL 1 */
#define S3C_VIDW00ADD2B0	S3C_LCDREG(0x94)  	/* LCD CONTROL 1 */
#define S3C_VIDW00ADD2B1	S3C_LCDREG(0x98)  	/* LCD CONTROL 1 */
#define S3C_VIDW01ADD2		S3C_LCDREG(0x9C)  	/* LCD CONTROL 1 */
#define S3C_VIDINTCON		S3C_LCDREG(0xAC)  	/* LCD CONTROL 1 */
#define S3C_W1KEYCON0		S3C_LCDREG(0xB0)  	/* LCD CONTROL 1 */
#define S3C_W1KEYCON1		S3C_LCDREG(0xB4)  	/* LCD CONTROL 1 */
#define S3C_WIN0MAP		S3C_LCDREG(0xD0)  	/* LCD CONTROL 1 */
#define S3C_WIN1MAP		S3C_LCDREG(0xD4)  	/* LCD CONTROL 1 */
#define S3C_WPALCON		S3C_LCDREG(0xE4)  	/* LCD CONTROL 1 */
#define S3C_SYSIFCON0		S3C_LCDREG(0x130)  	/* LCD CONTROL 1 */
#define S3C_SYSIFCON1		S3C_LCDREG(0x134)  	/* LCD CONTROL 1 */
#define S3C_DITHMODE		S3C_LCDREG(0x138)  	/* LCD CONTROL 1 */
#define S3C_SIFCCON0		S3C_LCDREG(0x13C)  	/* LCD CONTROL 1 */
#define S3C_SIFCCON1		S3C_LCDREG(0x140)  	/* LCD CONTROL 1 */
#define S3C_SIFCCON2		S3C_LCDREG(0x144)  	/* LCD CONTROL 1 */
#define S3C_CPUTRIGCON2		S3C_LCDREG(0x160)  	/* LCD CONTROL 1 */

#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410) || defined(CONFIG_CPU_S5P6440) || defined(CONFIG_CPU_S5PC100) || defined(CONFIG_CPU_S5PC110)
#define S3C_VIDCON2		S3C_LCDREG(0x08)  	/* Video control 2 register */
#define S3C_PRTCON		S3C_LCDREG(0x0C)	/* Protection control register */
#define S3C_VIDTCON0		S3C_LCDREG(0x10)  	/* Video time control 0 register */
#define S3C_VIDTCON1		S3C_LCDREG(0x14)  	/* Video time control 1 register */
#define S3C_VIDTCON2		S3C_LCDREG(0x18)  	/* Video time control 2 register */
#define S3C_VIDTCON3		S3C_LCDREG(0x1C)  	/* Video time control 3 register */

#define S3C_WINCON0		S3C_LCDREG(0x20)  	/* Window control 0 register */
#define S3C_WINCON1		S3C_LCDREG(0x24)  	/* Window control 1 register */
#define S3C_WINCON2		S3C_LCDREG(0x28)  	/* Window control 2 register */
#define S3C_WINCON3		S3C_LCDREG(0x2C)  	/* Window control 3 register */
#define S3C_WINCON4		S3C_LCDREG(0x30)  	/* Window control 4 register*/


#define S3C_VIDOSD0A		S3C_LCDREG(0x40)  	/* Video Window 0 position control register */
#define S3C_VIDOSD0B		S3C_LCDREG(0x44)  	/* Video Window 0 position control register1 */
#define S3C_VIDOSD0C		S3C_LCDREG(0x48)  	/* Video Window 0 position control register */

#define S3C_VIDOSD1A		S3C_LCDREG(0x50)  	/* Video Window 1 position control register */
#define S3C_VIDOSD1B		S3C_LCDREG(0x54)  	/* Video Window 1 position control register */
#define S3C_VIDOSD1C		S3C_LCDREG(0x58)  	/* Video Window 1 position control register */
#define S3C_VIDOSD1D		S3C_LCDREG(0x5C)  	/* Video Window 1 position control register */

#define S3C_VIDOSD2A		S3C_LCDREG(0x60)  	/* Video Window 2 position control register */
#define S3C_VIDOSD2B		S3C_LCDREG(0x64)  	/* Video Window 2 position control register */
#define S3C_VIDOSD2C		S3C_LCDREG(0x68)  	/* Video Window 2 position control register */
#define S3C_VIDOSD2D		S3C_LCDREG(0x6C)  	/* Video Window 2 position control register */

#define S3C_VIDOSD3A		S3C_LCDREG(0x70)  	/* Video Window 3 position control register */
#define S3C_VIDOSD3B		S3C_LCDREG(0x74)  	/* Video Window 3 position control register */
#define S3C_VIDOSD3C		S3C_LCDREG(0x78)  	/* Video Window 3 position control register */

#define S3C_VIDOSD4A		S3C_LCDREG(0x80)  	/* Video Window 4 position control register */
#define S3C_VIDOSD4B		S3C_LCDREG(0x84)  	/* Video Window 4 position control register */
#define S3C_VIDOSD4C		S3C_LCDREG(0x88)  	/* Video Window 4 position control register */

#define S3C_VIDW00ADD2B0	S3C_LCDREG(0x94)  	/* LCD CONTROL 1 */
#define S3C_VIDW00ADD2B1	S3C_LCDREG(0x98)  	/* LCD CONTROL 1 */

#define S3C_VIDW00ADD0B0	S3C_LCDREG(0x0A0) 	/* Window 0 buffer start address register, buffer 0 */
#define S3C_VIDW00ADD0B1	S3C_LCDREG(0x0A4) 	/* Window 0 buffer start address register, buffer 1 */
#define S3C_VIDW01ADD0B0	S3C_LCDREG(0x0A8) 	/* Window 1 buffer start address register, buffer 0 */
#define S3C_VIDW01ADD0B1	S3C_LCDREG(0x0AC) 	/* Window 1 buffer start address register, buffer 1 */
#define S3C_VIDW02ADD0		S3C_LCDREG(0x0B0) 	/* Window 2 buffer start address register */
#define S3C_VIDW03ADD0		S3C_LCDREG(0x0B8) 	/* Window 3 buffer start address register */
#define S3C_VIDW04ADD0		S3C_LCDREG(0x0C0) 	/* Window 4 buffer start address register */
#define S3C_VIDW00ADD1B0	S3C_LCDREG(0x0D0) 	/* Window 0 buffer end address register, buffer 0 */
#define S3C_VIDW00ADD1B1	S3C_LCDREG(0x0D4) 	/* Window 0 buffer end address register, buffer 1 */
#define S3C_VIDW01ADD1B0	S3C_LCDREG(0x0D8) 	/* Window 1 buffer end address register, buffer 0 */
#define S3C_VIDW01ADD1B1	S3C_LCDREG(0x0DC) 	/* Window 1 buffer end address register, buffer 1 */
#define S3C_VIDW02ADD1		S3C_LCDREG(0x0E0) 	/* Window 2 buffer end address register */
#define S3C_VIDW03ADD1		S3C_LCDREG(0x0E8) 	/* Window 3 buffer end address register */
#define S3C_VIDW04ADD1		S3C_LCDREG(0x0F0) 	/* Window 4 buffer end address register */
#define S3C_VIDW00ADD2		S3C_LCDREG(0x100) 	/* Window 0 buffer size register */
#define S3C_VIDW01ADD2		S3C_LCDREG(0x104) 	/* Window 1 buffer size register */

#define S3C_VIDW02ADD2		S3C_LCDREG(0x108) 	/* Window 2 buffer size register */
#define S3C_VIDW03ADD2		S3C_LCDREG(0x10C) 	/* Window 3 buffer size register */
#define S3C_VIDW04ADD2		S3C_LCDREG(0x110) 	/* Window 4 buffer size register */

#define S3C_VIDINTCON0		S3C_LCDREG(0x130)	/* Indicate the Video interrupt control register */
#define S3C_VIDINTCON1		S3C_LCDREG(0x134) 	/* Video Interrupt Pending register */
#define S3C_W1KEYCON0		S3C_LCDREG(0x140) 	/* Color key control register */
#define S3C_W1KEYCON1		S3C_LCDREG(0x144) 	/* Color key value ( transparent value) register */
#define S3C_W2KEYCON0		S3C_LCDREG(0x148) 	/* Color key control register */
#define S3C_W2KEYCON1		S3C_LCDREG(0x14C) 	/* Color key value (transparent value) register */

#define S3C_W3KEYCON0		S3C_LCDREG(0x150)	/* Color key control register	*/
#define S3C_W3KEYCON1		S3C_LCDREG(0x154)	/* Color key value (transparent value) register	*/
#define S3C_W4KEYCON0		S3C_LCDREG(0x158)	/* Color key control register	*/
#define S3C_W4KEYCON1		S3C_LCDREG(0x15C)	/* Color key value (transparent value) register	*/
#define S3C_DITHMODE		S3C_LCDREG(0x170)	/* Dithering mode register.	*/

#define S3C_WIN0MAP		S3C_LCDREG(0x180)	/* Window color control	*/
#define S3C_WIN1MAP		S3C_LCDREG(0x184)	/* Window color control	*/
#define S3C_WIN2MAP		S3C_LCDREG(0x188)	/* Window color control	*/
#define S3C_WIN3MAP		S3C_LCDREG(0x18C)	/* Window color control	*/
#define S3C_WIN4MAP		S3C_LCDREG(0x190)	/* Window color control	*/
#define S3C_WPALCON		S3C_LCDREG(0x1A0)	/* Window Palette control register	*/

#define S3C_TRIGCON		S3C_LCDREG(0x1A4)	/* I80 / RGB Trigger Control Regiter	*/
#define S3C_I80IFCONA0		S3C_LCDREG(0x1B0)	/* I80 Interface control 0 for Main LDI	*/
#define S3C_I80IFCONA1		S3C_LCDREG(0x1B4)	/* I80 Interface control 0 for Sub LDI	*/
#define S3C_I80IFCONB0		S3C_LCDREG(0x1B8)	/* I80 Inteface control 1 for Main LDI	*/
#define S3C_I80IFCONB1		S3C_LCDREG(0x1BC)	/* I80 Inteface control 1 for Sub LDI	*/
#define S3C_LDI_CMDCON0		S3C_LCDREG(0x1D0)	/* I80 Interface LDI Command Control 0	*/
#define S3C_LDI_CMDCON1		S3C_LCDREG(0x1D4)	/* I80 Interface LDI Command Control 1	*/
#define S3C_SIFCCON0		S3C_LCDREG(0x1E0)	/* LCD i80 System Interface Command Control 0	*/
#define S3C_SIFCCON1		S3C_LCDREG(0x1E4)	/* LCD i80 System Interface Command Control 1	*/
#define S3C_SIFCCON2		S3C_LCDREG(0x1E8)	/* LCD i80 System Interface Command Control 2	*/

#define S3C_LDI_CMD0		S3C_LCDREG(0x280)	/* I80 Inteface LDI Command 0	*/
#define S3C_LDI_CMD1		S3C_LCDREG(0x284)	/* I80 Inteface LDI Command 1	*/
#define S3C_LDI_CMD2		S3C_LCDREG(0x288)	/* I80 Inteface LDI Command 2	*/
#define S3C_LDI_CMD3		S3C_LCDREG(0x28C)	/* I80 Inteface LDI Command 3	*/
#define S3C_LDI_CMD4		S3C_LCDREG(0x290)	/* I80 Inteface LDI Command 4	*/
#define S3C_LDI_CMD5		S3C_LCDREG(0x294)	/* I80 Inteface LDI Command 5	*/
#define S3C_LDI_CMD6		S3C_LCDREG(0x298)	/* I80 Inteface LDI Command 6	*/
#define S3C_LDI_CMD7		S3C_LCDREG(0x29C)	/* I80 Inteface LDI Command 7	*/
#define S3C_LDI_CMD8		S3C_LCDREG(0x2A0)	/* I80 Inteface LDI Command 8	*/
#define S3C_LDI_CMD9		S3C_LCDREG(0x2A4)	/* I80 Inteface LDI Command 9	*/
#define S3C_LDI_CMD10		S3C_LCDREG(0x2A8)	/* I80 Inteface LDI Command 10	*/
#define S3C_LDI_CMD11		S3C_LCDREG(0x2AC)	/* I80 Inteface LDI Command 11	*/

#define S3C_W2PDATA01		S3C_LCDREG(0x300)	/* Window 2 Palette Data of the Index 0,1	*/
#define S3C_W2PDATA23		S3C_LCDREG(0x304)	/* Window 2 Palette Data of the Index 2,3	*/
#define S3C_W2PDATA45		S3C_LCDREG(0x308)	/* Window 2 Palette Data of the Index 4,5	*/
#define S3C_W2PDATA67		S3C_LCDREG(0x30C)	/* Window 2 Palette Data of the Index 6,7	*/
#define S3C_W2PDATA89		S3C_LCDREG(0x310)	/* Window 2 Palette Data of the Index 8,9	*/
#define S3C_W2PDATAAB		S3C_LCDREG(0x314)	/* Window 2 Palette Data of the Index A, B	*/
#define S3C_W2PDATACD		S3C_LCDREG(0x318)	/* Window 2 Palette Data of the Index C, D	*/
#define S3C_W2PDATAEF		S3C_LCDREG(0x31C)	/* Window 2 Palette Data of the Index E, F	*/
#define S3C_W3PDATA01		S3C_LCDREG(0x320)	/* Window 3 Palette Data of the Index 0,1	*/
#define S3C_W3PDATA23		S3C_LCDREG(0x324)	/* Window 3 Palette Data of the Index 2,3	*/
#define S3C_W3PDATA45		S3C_LCDREG(0x328)	/* Window 3 Palette Data of the Index 4,5	*/
#define S3C_W3PDATA67		S3C_LCDREG(0x32C)	/* Window 3 Palette Data of the Index 6,7	*/
#define S3C_W3PDATA89		S3C_LCDREG(0x330)	/* Window 3 Palette Data of the Index 8,9	*/
#define S3C_W3PDATAAB		S3C_LCDREG(0x334)	/* Window 3 Palette Data of the Index A, B	*/
#define S3C_W3PDATACD		S3C_LCDREG(0x338)	/* Window 3 Palette Data of the Index C, D	*/
#define S3C_W3PDATAEF		S3C_LCDREG(0x33C)	/* Window 3 Palette Data of the Index E, F	*/
#define S3C_W4PDATA01		S3C_LCDREG(0x340)	/* Window 3 Palette Data of the Index 0,1	*/
#define S3C_W4PDATA23		S3C_LCDREG(0x344)	/* Window 3 Palette Data of the Index 2,3	*/
#endif

#define S3C_TFTPAL2(x)		S3C_LCDREG((0x300 + (x)*4))
#define S3C_TFTPAL3(x) 		S3C_LCDREG((0x320 + (x)*4))
#define S3C_TFTPAL4(x)		S3C_LCDREG((0x340 + (x)*4))
#define S3C_TFTPAL0(x)		S3C_LCDREG((0x400 + (x)*4))
#define S3C_TFTPAL1(x)		S3C_LCDREG((0x800 + (x)*4))

/*--------------------------------------------------------------*/
/* Video Main Control 0 register - VIDCON0 */
#define S3C_VIDCON0_INTERLACE_F_PROGRESSIVE		(0<<29)
#define S3C_VIDCON0_INTERLACE_F_INTERLACE		(1<<29)
#define S3C_VIDCON0_INTERLACE_F_MASK			(1<<29)
#define S3C_VIDCON0_VIDOUT(x)  				(((x)&0x7)<<26)
#define S3C_VIDCON0_VIDOUT_RGB_IF			(0<<26)
#define S3C_VIDCON0_VIDOUT_TV				(1<<26)
#define S3C_VIDCON0_VIDOUT_I80IF0			(2<<26)
#define S3C_VIDCON0_VIDOUT_I80IF1			(3<<26)
#define S3C_VIDCON0_VIDOUT_TVNRGBIF 			(4<<26)
#define S3C_VIDCON0_VIDOUT_TVNI80IF0			(6<<26)
#define S3C_VIDCON0_VIDOUT_TVNI80IF1			(7<<26)
#define S3C_VIDCON0_VIDOUT_MASK				(7<<26)
#define S3C_VIDCON0_L1_DATA16(x)  			(((x)&0x7)<<23)
#define S3C_VIDCON0_L1_DATA16_SUB_16_MODE		(0<<23)
#define S3C_VIDCON0_L1_DATA16_SUB_16PLUS2_MODE		(1<<23)
#define S3C_VIDCON0_L1_DATA16_SUB_9PLUS9_MODE 		(2<<23)
#define S3C_VIDCON0_L1_DATA16_SUB_16PLUS8_MODE		(3<<23)
#define S3C_VIDCON0_L1_DATA16_SUB_18_MODE		(4<<23)
#define S3C_VIDCON0_L0_DATA16(x)  			(((x)&0x7)<<20)
#define S3C_VIDCON0_L0_DATA16_MAIN_16_MODE		(0<<20)
#define S3C_VIDCON0_L0_DATA16_MAIN_16PLUS2_MODE		(1<<20)
#define S3C_VIDCON0_L0_DATA16_MAIN_9PLUS9_MODE		(2<<20)
#define S3C_VIDCON0_L0_DATA16_MAIN_16PLUS8_MODE		(3<<20)
#define S3C_VIDCON0_L0_DATA16_MAIN_18_MODE		(4<<20)
#define S3C_VIDCON0_PNRMODE(x)  			(((x)&0x3)<<17)
#define S3C_VIDCON0_PNRMODE_RGB_P			(0<<17)
#define S3C_VIDCON0_PNRMODE_BGR_P			(1<<17)
#define S3C_VIDCON0_PNRMODE_RGB_S			(2<<17)
#define S3C_VIDCON0_PNRMODE_BGR_S			(3<<17)
#define S3C_VIDCON0_PNRMODE_MASK			(3<<17)
#define S3C_VIDCON0_CLKVALUP_ALWAYS 			(0<<16)
#define S3C_VIDCON0_CLKVALUP_ST_FRM 			(1<<16)
#define S3C_VIDCON0_CLKVAL_F(x)				(((x)&0xFF)<<6)
#define S3C_VIDCON0_VCLKEN_ENABLE			(1<<5)
#define S3C_VIDCON0_CLKDIR_DIVIDED   			(1<<4)
#define S3C_VIDCON0_CLKDIR_DIRECTED  			(0<<4)
#define S3C_VIDCON0_CLKSEL(x)   			(((x)&0x3)<<2)
#define S3C_VIDCON0_CLKSEL_F_HCLK	  		(0<<2)
#define S3C_VIDCON0_ENVID_ENABLE	    		(1 << 1)	/* 0:Disable 1:Enable LCD video output and logic immediatly */
#define S3C_VIDCON0_ENVID_DISABLE	    		(0 << 1)	/* 0:Disable 1:Enable LCD video output and logic immediatly */
#define S3C_VIDCON0_ENVID_F_ENABLE     			(1 << 0)	/* 0:Dis 1:Ena wait until Current frame end. */
#define S3C_VIDCON0_ENVID_F_DISABLE     		(0 << 0)	/* 0:Dis 1:Ena wait until Current frame end. */

/* Video Main Control 1 register - VIDCON1 */
#define S3C_VIDCON1_IVCLK_FALL_EDGE 			(0<<7)
#define S3C_VIDCON1_IVCLK_RISE_EDGE 			(1<<7)
#define S3C_VIDCON1_IHSYNC_NORMAL			(0<<6)
#define S3C_VIDCON1_IHSYNC_INVERT			(1<<6)
#define S3C_VIDCON1_IVSYNC_NORMAL			(0<<5)
#define S3C_VIDCON1_IVSYNC_INVERT			(1<<5)
#define S3C_VIDCON1_IVDEN_NORMAL			(0<<4)
#define S3C_VIDCON1_IVDEN_INVERT			(1<<4)

/* Video Main Control 2 register - VIDCON2 */
#define S3C_VIDCON2_EN601_DISABLE			(0<<23)
#define S3C_VIDCON2_EN601_ENABLE			(1<<23)
#define S3C_VIDCON2_EN601_MASK				(1<<23)
#define S3C_VIDCON2_TVFORMATSEL0_HARDWARE		(0<<14)
#define S3C_VIDCON2_TVFORMATSEL0_SOFTWARE		(1<<14)
#define S3C_VIDCON2_TVFORMATSEL0_MASK			(1<<14)
#define S3C_VIDCON2_TVFORMATSEL1_RGB			(0<<12)
#define S3C_VIDCON2_TVFORMATSEL1_YUV422			(1<<12)
#define S3C_VIDCON2_TVFORMATSEL1_YUV444			(2<<12)
#define S3C_VIDCON2_TVFORMATSEL1_MASK			(0x3<<12)
#define S3C_VIDCON2_ORGYUV_YCBCR			(0<<8)
#define S3C_VIDCON2_ORGYUV_CBCRY			(1<<8)
#define S3C_VIDCON2_ORGYUV_MASK				(1<<8)
#define S3C_VIDCON2_YUVORD_CBCR				(0<<7)
#define S3C_VIDCON2_YUVORD_CRCB				(1<<7)
#define S3C_VIDCON2_YUVORD_MASK				(1<<7)

/* VIDEO Time Control 0 register - VIDTCON0 */
#define S3C_VIDTCON0_VBPDE(x)				(((x)&0xFF)<<24)
#define S3C_VIDTCON0_VBPD(x)				(((x)&0xFF)<<16)
#define S3C_VIDTCON0_VFPD(x) 				(((x)&0xFF)<<8)
#define S3C_VIDTCON0_VSPW(x) 				(((x)&0xFF)<<0)

/* VIDEO Time Control 1 register - VIDTCON1 */
#define S3C_VIDTCON1_VFPDE(x)				(((x)&0xFF)<<24)
#define S3C_VIDTCON1_HBPD(x) 				(((x)&0xFF)<<16)
#define S3C_VIDTCON1_HFPD(x) 				(((x)&0xFF)<<8)
#define S3C_VIDTCON1_HSPW(x) 				(((x)&0xFF)<<0)

/* VIDEO Time Control 2 register - VIDTCON2 */
#define S3C_VIDTCON2_LINEVAL(x)  			(((x)&0x7FF)<<11) /* these bits determine the vertical size of lcd panel */
#define S3C_VIDTCON2_HOZVAL(x)   			(((x)&0x7FF)<<0) /* these bits determine the horizontal size of lcd panel*/


/* Window 0~4 Control register - WINCONx */
#define S3C_WINCONx_WIDE_NARROW(x)			(((x)&0x3)<<26)
#define S3C_WINCONx_ENLOCAL_DMA				(0<<22)
#define S3C_WINCONx_ENLOCAL				(1<<22)
#define S3C_WINCONx_ENLOCAL_MASK			(1<<22)
#define S3C_WINCONx_BUFSEL_0				(0<<20)
#define S3C_WINCONx_BUFSEL_1				(1<<20)
#define S3C_WINCONx_BUFSEL_MASK				(1<<20)
#define S3C_WINCONx_BUFAUTOEN_DISABLE			(0<<19)
#define S3C_WINCONx_BUFAUTOEN_ENABLE			(1<<19)
#define S3C_WINCONx_BUFAUTOEN_MASK			(1<<19)
#define S3C_WINCONx_BITSWP_DISABLE			(0<<18)
#define S3C_WINCONx_BITSWP_ENABLE			(1<<18)
#define S3C_WINCONx_BYTSWP_DISABLE			(0<<17)
#define S3C_WINCONx_BYTSWP_ENABLE			(1<<17)
#define S3C_WINCONx_HAWSWP_DISABLE			(0<<16)
#define S3C_WINCONx_HAWSWP_ENABLE			(1<<16)
#define S3C_WINCONx_WSWP_DISABLE			(0<<15)
#define S3C_WINCONx_WSWP_ENABLE				(1<<15)
#define S3C_WINCONx_INRGB_RGB		   		(0<<13)
#define S3C_WINCONx_INRGB_YUV		 		(1<<13)
#define S3C_WINCONx_INRGB_MASK				(1<<13)
#define S3C_WINCONx_BURSTLEN_16WORD			(0<<9)
#define S3C_WINCONx_BURSTLEN_8WORD			(1<<9)
#define S3C_WINCONx_BURSTLEN_4WORD			(2<<9)
#define S3C_WINCONx_BURSTLEN_MASK			(3<<9)
#define S3C_WINCONx_BLD_PIX_PLANE			(0<<6)
#define S3C_WINCONx_BLD_PIX_PIXEL			(1<<6)
#define S3C_WINCONx_BLD_PIX_MASK			(1<<6)
#define S3C_WINCONx_BPPMODE_F_1BPP			(0<<2)
#define S3C_WINCONx_BPPMODE_F_2BPP			(1<<2)
#define S3C_WINCONx_BPPMODE_F_4BPP			(2<<2)
#define S3C_WINCONx_BPPMODE_F_8BPP_PAL			(3<<2)
#define S3C_WINCONx_BPPMODE_F_8BPP_NOPAL		(4<<2)
#define S3C_WINCONx_BPPMODE_F_16BPP_565			(5<<2)
#define S3C_WINCONx_BPPMODE_F_16BPP_A555		(6<<2)
#define S3C_WINCONx_BPPMODE_F_18BPP_666			(8<<2)
#define S3C_WINCONx_BPPMODE_F_24BPP_888			(11<<2)
#define S3C_WINCONx_BPPMODE_F_24BPP_A887		(0xc<<2)
#define S3C_WINCONx_BPPMODE_F_25BPP_A888		(0xd<<2)
#define S3C_WINCONx_BPPMODE_F_28BPP_A888		(0xd<<2)
#define S3C_WINCONx_BPPMODE_F_MASK			(0xf<<2)
#define S3C_WINCONx_ALPHA_SEL_0				(0<<1)
#define S3C_WINCONx_ALPHA_SEL_1				(1<<1)
#define S3C_WINCONx_ALPHA_SEL_MASK			(1<<1)
#define S3C_WINCONx_ENWIN_F_DISABLE 			(0<<0)
#define S3C_WINCONx_ENWIN_F_ENABLE			(1<<0)

/* Window 1-2 Control register - WINCON1 */
#define S3C_WINCON1_LOCALSEL_TV				(0<<23)
#define S3C_WINCON1_LOCALSEL_CAMERA			(1<<23)
#define S3C_WINCON1_LOCALSEL_MASK			(1<<23)
#define S3C_WINCON2_LOCALSEL_TV				(0<<23)
#define S3C_WINCON2_LOCALSEL_CAMERA			(1<<23)
#define S3C_WINCON2_LOCALSEL_MASK			(1<<23)

/* Window 0~4 Position Control A register - VIDOSDxA */
#define S3C_VIDOSDxA_OSD_LTX_F(x)			(((x)&0x7FF)<<11)
#define S3C_VIDOSDxA_OSD_LTY_F(x)			(((x)&0x7FF)<<0)

/* Window 0~4 Position Control B register - VIDOSDxB */
#define S3C_VIDOSDxB_OSD_RBX_F(x)			(((x)&0x7FF)<<11)
#define S3C_VIDOSDxB_OSD_RBY_F(x)			(((x)&0x7FF)<<0)

/* Window 0 Position Control C register - VIDOSD0C */
#define  S3C_VIDOSD0C_OSDSIZE(x)			(((x)&0xFFFFFF)<<0)

/* Window 1~4 Position Control C register - VIDOSDxC */
#define S3C_VIDOSDxC_ALPHA0_R(x)			(((x)&0xF)<<20)
#define S3C_VIDOSDxC_ALPHA0_G(x)			(((x)&0xF)<<16)
#define S3C_VIDOSDxC_ALPHA0_B(x)			(((x)&0xF)<<12)
#define S3C_VIDOSDxC_ALPHA1_R(x)			(((x)&0xF)<<8)
#define S3C_VIDOSDxC_ALPHA1_G(x)			(((x)&0xF)<<4)
#define S3C_VIDOSDxC_ALPHA1_B(x)			(((x)&0xF)<<0)

/* Window 1~2 Position Control D register - VIDOSDxD */
#define  S3C_VIDOSDxD_OSDSIZE(x)			(((x)&0xFFFFFF)<<0)

/* Frame buffer Start Address register - VIDWxxADD0 */
#define S3C_VIDWxxADD0_VBANK_F(x) 			(((x)&0xFF)<<23) /* the end address of the LCD frame buffer. */
#define S3C_VIDWxxADD0_VBASEU_F(x)			(((x)&0xFFFFFF)<<0) /* Virtual screen offset size (the number of byte). */

/* Frame buffer End Address register - VIDWxxADD1 */
#define S3C_VIDWxxADD1_VBASEL_F(x) 			(((x)&0xFFFFFF)<<0)  /* the end address of the LCD frame buffer. */

/* Frame buffer Size register - VIDWxxADD2 */
#define S3C_VIDWxxADD2_OFFSIZE_F(x)  			(((x)&0x1FFF)<<13) /* Virtual screen offset size (the number of byte). */
#define S3C_VIDWxxADD2_PAGEWIDTH_F(x)			(((x)&0x1FFF)<<0) /* Virtual screen page width (the number of byte). */

/* VIDEO Interrupt Control 0 register - VIDINTCON0 */
#define S3C_VIDINTCON0_FIFOINTERVAL(x)			(((x)&0x3F)<<20)
#define S3C_VIDINTCON0_SYSMAINCON_DISABLE		(0<<19)
#define S3C_VIDINTCON0_SYSMAINCON_ENABLE		(1<<19)
#define S3C_VIDINTCON0_SYSSUBCON_DISABLE		(0<<18)
#define S3C_VIDINTCON0_SYSSUBCON_ENABLE			(1<<18)
#define S3C_VIDINTCON0_SYSIFDONE_DISABLE		(0<<17)
#define S3C_VIDINTCON0_SYSIFDONE_ENABLE			(1<<17)
#define S3C_VIDINTCON0_FRAMESEL0_BACK			(0<<15)
#define S3C_VIDINTCON0_FRAMESEL0_VSYNC 			(1<<15)
#define S3C_VIDINTCON0_FRAMESEL0_ACTIVE			(2<<15)
#define S3C_VIDINTCON0_FRAMESEL0_FRONT 			(3<<15)
#define S3C_VIDINTCON0_FRAMESEL0_MASK 			(3<<15)
#define S3C_VIDINTCON0_FRAMESEL1_NONE			(0<<13)
#define S3C_VIDINTCON0_FRAMESEL1_BACK			(1<<13)
#define S3C_VIDINTCON0_FRAMESEL1_VSYNC 			(2<<13)
#define S3C_VIDINTCON0_FRAMESEL1_FRONT 			(3<<13)
#define S3C_VIDINTCON0_INTFRMEN_DISABLE			(0<<12)
#define S3C_VIDINTCON0_INTFRMEN_ENABLE 			(1<<12)
#define S3C_VIDINTCON0_FRAMEINT_MASK			(0x1F<<12)
#define S3C_VIDINTCON0_FIFOSEL_WIN4			(1<<11)
#define S3C_VIDINTCON0_FIFOSEL_WIN3			(1<<10)
#define S3C_VIDINTCON0_FIFOSEL_WIN2			(1<<9)
#define S3C_VIDINTCON0_FIFOSEL_WIN1			(1<<6)
#define S3C_VIDINTCON0_FIFOSEL_WIN0			(1<<5)
#define S3C_VIDINTCON0_FIFOSEL_ALL			(0x73<<5)
#define S3C_VIDINTCON0_FIFOLEVEL_25			(0<<2)
#define S3C_VIDINTCON0_FIFOLEVEL_50			(1<<2)
#define S3C_VIDINTCON0_FIFOLEVEL_75			(2<<2)
#define S3C_VIDINTCON0_FIFOLEVEL_EMPTY 			(3<<2)
#define S3C_VIDINTCON0_FIFOLEVEL_FULL			(4<<2)
#define S3C_VIDINTCON0_INTFIFOEN_DISABLE		(0<<1)
#define S3C_VIDINTCON0_INTFIFOEN_ENABLE			(1<<1)
#define S3C_VIDINTCON0_INTEN_DISABLE			(0<<0)
#define S3C_VIDINTCON0_INTEN_ENABLE			(1<<0)
#define S3C_VIDINTCON0_INTEN_MASK			(1<<0)

/* VIDEO Interrupt Control 1 register - VIDINTCON1 */
#define S3C_VIDINTCON1_INTI80PEND			(0<<2)
#define S3C_VIDINTCON1_INTFRMPEND			(1<<1)
#define S3C_VIDINTCON1_INTFIFOPEND			(1<<0)

/* WIN 1~4 Color Key 0 register - WxKEYCON0 */
#define S3C_WxKEYCON0_KEYBLEN_DISABLE 			(0<<26)
#define S3C_WxKEYCON0_KEYBLEN_ENABLE			(1<<26)
#define S3C_WxKEYCON0_KEYEN_F_DISABLE 			(0<<25)
#define S3C_WxKEYCON0_KEYEN_F_ENABLE			(1<<25)
#define S3C_WxKEYCON0_DIRCON_MATCH_FG_IMAGE		(0<<24)
#define S3C_WxKEYCON0_DIRCON_MATCH_BG_IMAGE		(1<<24)
#define S3C_WxKEYCON0_COMPKEY(x)			(((x)&0xFFFFFF)<<0)

/* WIN 1~4 Color Key 1 register - WxKEYCON1 */
#define S3C_WxKEYCON1_COLVAL(x)				(((x)&0xFFFFFF)<<0)

/* Dithering Control 1 register - DITHMODE */
#define S3C_DITHMODE_RDITHPOS_8BIT			(0<<5)
#define S3C_DITHMODE_RDITHPOS_6BIT			(1<<5)
#define S3C_DITHMODE_RDITHPOS_5BIT			(2<<5)
#define S3C_DITHMODE_GDITHPOS_8BIT			(0<<3)
#define S3C_DITHMODE_GDITHPOS_6BIT			(1<<3)
#define S3C_DITHMODE_GDITHPOS_5BIT			(2<<3)
#define S3C_DITHMODE_BDITHPOS_8BIT			(0<<1)
#define S3C_DITHMODE_BDITHPOS_6BIT			(1<<1)
#define S3C_DITHMODE_BDITHPOS_5BIT			(2<<1)
#define S3C_DITHMODE_RGB_DITHPOS_MASK			(0x3f<<1)
#define S3C_DITHMODE_DITHERING_DISABLE			(0<<0)
#define S3C_DITHMODE_DITHERING_ENABLE			(1<<0)
#define S3C_DITHMODE_DITHERING_MASK			(1<<0)

/* Window 0~4 Color map register - WINxMAP */
#define S3C_WINxMAP_MAPCOLEN_F_ENABLE			(1<<24)
#define S3C_WINxMAP_MAPCOLEN_F_DISABLE			(0<<24)
#define S3C_WINxMAP_MAPCOLOR				(((x)&0xFFFFFF)<<0)

/* Window Palette Control register - WPALCON */
#define S3C_WPALCON_PALUPDATEEN				(1<<9)
#define S3C_WPALCON_W4PAL_16BIT_A	 		(1<<8)		/* A:5:5:5 */
#define S3C_WPALCON_W4PAL_16BIT	 			(0<<8)		/*  5:6:5 */
#define S3C_WPALCON_W3PAL_16BIT_A	 		(1<<7)		/* A:5:5:5 */
#define S3C_WPALCON_W3PAL_16BIT	 			(0<<7)		/*  5:6:5 */
#define S3C_WPALCON_W2PAL_16BIT_A	 		(1<<6)		/* A:5:5:5 */
#define S3C_WPALCON_W2PAL_16BIT	 			(0<<6)		/*  5:6:5 */
#define S3C_WPALCON_W1PAL_25BIT_A	 		(0<<3)		/* A:8:8:8 */
#define S3C_WPALCON_W1PAL_24BIT				(1<<3)		/*  8:8:8 */
#define S3C_WPALCON_W1PAL_19BIT_A			(2<<3)		/* A:6:6:6 */
#define S3C_WPALCON_W1PAL_18BIT_A	 		(3<<3)		/* A:6:6:5 */
#define S3C_WPALCON_W1PAL_18BIT				(4<<3)		/*  6:6:6 */
#define S3C_WPALCON_W1PAL_16BIT_A	 		(5<<3)		/* A:5:5:5 */
#define S3C_WPALCON_W1PAL_16BIT	 			(6<<3)		/*  5:6:5 */
#define S3C_WPALCON_W0PAL_25BIT_A	 		(0<<0)		/* A:8:8:8 */
#define S3C_WPALCON_W0PAL_24BIT				(1<<0)		/*  8:8:8 */
#define S3C_WPALCON_W0PAL_19BIT_A			(2<<0)		/* A:6:6:6 */
#define S3C_WPALCON_W0PAL_18BIT_A	 		(3<<0)		/* A:6:6:5 */
#define S3C_WPALCON_W0PAL_18BIT				(4<<0)		/*  6:6:6 */
#define S3C_WPALCON_W0PAL_16BIT_A	 		(5<<0)		/* A:5:5:5 */
#define S3C_WPALCON_W0PAL_16BIT	 			(6<<0)		/*  5:6:5 */

/* I80/RGB Trigger Control register - TRIGCON */
#define S3C_TRIGCON_SWFRSTATUS_REQUESTED		(1<<2)
#define S3C_TRIGCON_SWFRSTATUS_NOT_REQUESTED		(0<<2)
#define S3C_TRIGCON_SWTRGCMD				(1<<1)
#define S3C_TRIGCON_TRGMODE_ENABLE			(1<<0)
#define S3C_TRIGCON_TRGMODE_DISABLE			(0<<0)

/* LCD I80 Interface Control 0 register - I80IFCONA0 */
#define S3C_I80IFCONAx_LCD_CS_SETUP(x) 			(((x)&0xF)<<16)
#define S3C_I80IFCONAx_LCD_WR_SETUP(x) 			(((x)&0xF)<<12)
#define S3C_I80IFCONAx_LCD_WR_ACT(x)			(((x)&0xF)<<8)
#define S3C_I80IFCONAx_LCD_WR_HOLD(x)			(((x)&0xF)<<4)


/***************************************************************************/
/*HOST IF registers */
/* Host I/F A - */
#define S3C_HOSTIFAREG(x)				((x) + S3C64XX_VA_HOSTIFA)
#define S3C_HOSTIFAREG_PHYS(x)				((x) + S3C64XX_PA_HOSTIFA)

/* Host I/F B - Modem I/F */
#define S3C_HOSTIFBREG(x)				((x) + S3C64XX_VA_HOSTIFB)
#define S3C_HOSTIFBREG_PHYS(x)				((x) + S3C64XX_PA_HOSTIFB)

#define S3C_HOSTIFB_INT2AP				S3C_HOSTIFBREG(0x8000)
#define S3C_HOSTIFB_INT2MSM				S3C_HOSTIFBREG(0x8004)
#define S3C_HOSTIFB_MIFCON				S3C_HOSTIFBREG(0x8008)
#define S3C_HOSTIFB_MIFPCON				S3C_HOSTIFBREG(0x800C)
#define S3C_HOSTIFB_MSMINTCLR				S3C_HOSTIFBREG(0x8010)

#define S3C_HOSTIFB_MIFCON_INT2MSM_DIS			(0x0<<3)
#define S3C_HOSTIFB_MIFCON_INT2MSM_EN			(0x1<<3)
#define S3C_HOSTIFB_MIFCON_INT2AP_DIS			(0x0<<2)
#define S3C_HOSTIFB_MIFCON_INT2AP_EN			(0x1<<2)
#define S3C_HOSTIFB_MIFCON_WAKEUP_DIS			(0x0<<1)
#define S3C_HOSTIFB_MIFCON_WAKEUP_EN			(0x1<<1)

#define S3C_HOSTIFB_MIFPCON_SEL_VSYNC_DIR_OUT		(0x0<<5)
#define S3C_HOSTIFB_MIFPCON_SEL_VSYNC_DIR_IN		(0x1<<5)
#define S3C_HOSTIFB_MIFPCON_INT2M_LEVEL_DIS		(0x0<<4)
#define S3C_HOSTIFB_MIFPCON_INT2M_LEVEL_EN		(0x1<<4)
#define S3C_HOSTIFB_MIFPCON_SEL_NORMAL			(0x0<<3)
#define S3C_HOSTIFB_MIFPCON_SEL_BYPASS			(0x1<<3)

#define S3C_HOSTIFB_MIFPCON_SEL_RS0			0
#define S3C_HOSTIFB_MIFPCON_SEL_RS1			1
#define S3C_HOSTIFB_MIFPCON_SEL_RS2			2
#define S3C_HOSTIFB_MIFPCON_SEL_RS3			3
#define S3C_HOSTIFB_MIFPCON_SEL_RS4			4
#define S3C_HOSTIFB_MIFPCON_SEL_RS5			5
#define S3C_HOSTIFB_MIFPCON_SEL_RS6			6

#define S3C_WINCONx_ENLOCAL_POST                    	(1<<22)
#endif
