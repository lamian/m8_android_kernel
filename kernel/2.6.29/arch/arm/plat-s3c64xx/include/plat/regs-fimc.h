/* linux/arch/arm/plat-s5pc1xx/include/plat/regs-fimc.h
 *
 * Register definition file for Samsung Camera Interface (FIMC) driver
 *
 * Jinsung Yang, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef _REGS_FIMC_H
#define _REGS_FIMC_H

#define S3C_FIMCREG(x) 	(x)

/*************************************************************************
 * Register part
 ************************************************************************/
#define S3C_CICOYSA(__x) 			S3C_FIMCREG(0x18 + (__x) * 4)
#define S3C_CICOCBSA(__x) 			S3C_FIMCREG(0x28 + (__x) * 4)
#define S3C_CICOCRSA(__x)  			S3C_FIMCREG(0x38 + (__x) * 4)
#define S3C_CIPRYSA(__x) 			S3C_FIMCREG(0x6c + (__x) * 4)
#define S3C_CIPRCBSA(__x) 			S3C_FIMCREG(0x7c + (__x) * 4)
#define S3C_CIPRCRSA(__x)  			S3C_FIMCREG(0x8c + (__x) * 4)

#define S3C_CISRCFMT				S3C_FIMCREG(0x00)	/* Input source format */
#define S3C_CIWDOFST				S3C_FIMCREG(0x04)	/* Window offset */
#define S3C_CIGCTRL				S3C_FIMCREG(0x08)	/* Global control */
#define S3C_CIWDOFST2				S3C_FIMCREG(0x14)	/* Window offset 2 */
#define S3C_CICOYSA1				S3C_FIMCREG(0x18)	/* Y 1st frame start address for output DMA */
#define S3C_CICOYSA2				S3C_FIMCREG(0x1c)	/* Y 2nd frame start address for output DMA */
#define S3C_CICOYSA3				S3C_FIMCREG(0x20)	/* Y 3rd frame start address for output DMA */
#define S3C_CICOYSA4				S3C_FIMCREG(0x24)	/* Y 4th frame start address for output DMA */
#define S3C_CICOCBSA1				S3C_FIMCREG(0x28)	/* Cb 1st frame start address for output DMA */
#define S3C_CICOCBSA2				S3C_FIMCREG(0x2c)	/* Cb 2nd frame start address for output DMA */
#define S3C_CICOCBSA3				S3C_FIMCREG(0x30)	/* Cb 3rd frame start address for output DMA */
#define S3C_CICOCBSA4				S3C_FIMCREG(0x34)	/* Cb 4th frame start address for output DMA */
#define S3C_CICOCRSA1				S3C_FIMCREG(0x38)	/* Cr 1st frame start address for output DMA */
#define S3C_CICOCRSA2				S3C_FIMCREG(0x3c)	/* Cr 2nd frame start address for output DMA */
#define S3C_CICOCRSA3				S3C_FIMCREG(0x40)	/* Cr 3rd frame start address for output DMA */
#define S3C_CICOCRSA4				S3C_FIMCREG(0x44)	/* Cr 4th frame start address for output DMA */
#define S3C_CICOTRGFMT				S3C_FIMCREG(0x48)	/* Target image format */
#define S3C_CICOCTRL				S3C_FIMCREG(0x4c)	/* Output DMA control */
#define S3C_CICOSCPRERATIO			S3C_FIMCREG(0x50)	/* Pre-scaler control 1 */
#define S3C_CICOSCPREDST			S3C_FIMCREG(0x54)	/* Pre-scaler control 2 */
#define S3C_CICOSCCTRL				S3C_FIMCREG(0x58)	/* Main scaler control */
#define S3C_CICOTAREA				S3C_FIMCREG(0x5c)	/* Target area */
#define S3C_CICOSTATUS				S3C_FIMCREG(0x64)	/* Status */
#define S3C_CIPRYSA1				S3C_FIMCREG(0x6c)	/* Y 1st frame start address for output DMA */
#define S3C_CIPRYSA2				S3C_FIMCREG(0x70)	/* Y 2nd frame start address for output DMA */
#define S3C_CIPRYSA3				S3C_FIMCREG(0x74)	/* Y 3rd frame start address for output DMA */
#define S3C_CIPRYSA4				S3C_FIMCREG(0x78)	/* Y 4th frame start address for output DMA */
#define S3C_CIPRCBSA1				S3C_FIMCREG(0x7c)	/* Cb 1st frame start address for output DMA */
#define S3C_CIPRCBSA2				S3C_FIMCREG(0x80)	/* Cb 2nd frame start address for output DMA */
#define S3C_CIPRCBSA3				S3C_FIMCREG(0x84)	/* Cb 3rd frame start address for output DMA */
#define S3C_CIPRCBSA4				S3C_FIMCREG(0x88)	/* Cb 4th frame start address for output DMA */
#define S3C_CIPRCRSA1				S3C_FIMCREG(0x8c)	/* Cr 1st frame start address for output DMA */
#define S3C_CIPRCRSA2				S3C_FIMCREG(0x90)	/* Cr 2nd frame start address for output DMA */
#define S3C_CIPRCRSA3				S3C_FIMCREG(0x94)	/* Cr 3rd frame start address for output DMA */
#define S3C_CIPRCRSA4				S3C_FIMCREG(0x98)	/* Cr 4th frame start address for output DMA */
#define S3C_CIPRTRGFMT				S3C_FIMCREG(0x9c)	/* Target image format */
#define S3C_CIPRCTRL				S3C_FIMCREG(0xa0)	/* Output DMA control */
#define S3C_CIPRSCPRERATIO			S3C_FIMCREG(0xa4)	/* Pre-scaler control 1 */
#define S3C_CIPRSCPREDST			S3C_FIMCREG(0xa8)	/* Pre-scaler control 2 */
#define S3C_CIPRSCCTRL				S3C_FIMCREG(0xac)	/* Main scaler control */
#define S3C_CIPRTAREA				S3C_FIMCREG(0xb0)	/* Target area */
#define S3C_CIPRSTATUS				S3C_FIMCREG(0xb8)	/* Status */
#define S3C_CIIMGCPT				S3C_FIMCREG(0xc0)	/* Image capture enable command */
#define S3C_CICPTSEQ				S3C_FIMCREG(0xc4)	/* Capture sequence */
#define S3C_CIIMGEFF				S3C_FIMCREG(0xd0)	/* Image effects */
#define S3C_MSCOY0SA				S3C_FIMCREG(0xd4)	/* Y frame start address for input DMA */
#define S3C_MSCOCB0SA				S3C_FIMCREG(0xd8)	/* Cb frame start address for input DMA */
#define S3C_MSCOCR0SA				S3C_FIMCREG(0xdc)	/* Cr frame start address for input DMA */
#define S3C_MSCOY0END				S3C_FIMCREG(0xe0)	/* Y frame end address for input DMA */
#define S3C_MSCOCB0END				S3C_FIMCREG(0xe4)	/* Cb frame end address for input DMA */
#define S3C_MSCOCR0END				S3C_FIMCREG(0xe8)	/* Cr frame end address for input DMA */
#define S3C_MSCOYOFF				S3C_FIMCREG(0xec)	/* Y offset */
#define S3C_MSCOCBOFF				S3C_FIMCREG(0xf0)	/* CB offset */
#define S3C_MSCOCROFF				S3C_FIMCREG(0xf4)	/* CR offset */
#define S3C_MSCOWIDTH				S3C_FIMCREG(0xf8)	/* Real input DMA image size */
#define S3C_MSCOCTRL				S3C_FIMCREG(0xfc)	/* Input DMA control */
#define S3C_MSPRY0SA				S3C_FIMCREG(0x100)	/* Y frame start address for input DMA */
#define S3C_MSPRCB0SA				S3C_FIMCREG(0x104)	/* Cb frame start address for input DMA */
#define S3C_MSPRCR0SA				S3C_FIMCREG(0x108)	/* Cr frame start address for input DMA */
#define S3C_MSPRY0END				S3C_FIMCREG(0x10c)	/* Y frame end address for input DMA */
#define S3C_MSPRCB0END				S3C_FIMCREG(0x110)	/* Cb frame end address for input DMA */
#define S3C_MSPRCR0END				S3C_FIMCREG(0x114)	/* Cr frame end address for input DMA */
#define S3C_MSPRYOFF				S3C_FIMCREG(0x118)	/* Y offset */
#define S3C_MSPRCBOFF				S3C_FIMCREG(0x11c)	/* CB offset */
#define S3C_MSPRCROFF				S3C_FIMCREG(0x120)	/* CR offset */
#define S3C_MSPRWIDTH				S3C_FIMCREG(0x124)	/* Real input DMA image size */
#define S3C_MSPRCTRL				S3C_FIMCREG(0x128)	/* Input DMA control */
#define S3C_CICOSCOSY				S3C_FIMCREG(0x12c)
#define S3C_CICOSCOSCB				S3C_FIMCREG(0x130)
#define S3C_CICOSCOSCR				S3C_FIMCREG(0x134)
#define S3C_CIPRSPRSY				S3C_FIMCREG(0x138)
#define S3C_CIPRSPRSCB				S3C_FIMCREG(0x13c)
#define S3C_CIPRSPRSCR				S3C_FIMCREG(0x140)

/*************************************************************************
 * Macro part
 ************************************************************************/
#define S3C_CISRCFMT_SOURCEHSIZE(x)		((x) << 16)
#define S3C_CISRCFMT_SOURCEVSIZE(x)		((x) << 0)

#define S3C_CIWDOFST_WINHOROFST(x)		((x) << 16)
#define S3C_CIWDOFST_WINVEROFST(x)		((x) << 0)

#define S3C_CIWDOFST2_WINHOROFST2(x)		((x) << 16)
#define S3C_CIWDOFST2_WINVEROFST2(x)		((x) << 0)

#define S3C_CICOTRGFMT_TARGETHSIZE(x)		((x) << 16)
#define S3C_CICOTRGFMT_TARGETVSIZE(x)		((x) << 0)

#define S3C_CICOCTRL_YBURST1(x)			((x) << 19)
#define S3C_CICOCTRL_YBURST2(x)			((x) << 14)
#define S3C_CICOCTRL_CBURST1(x)			((x) << 9)
#define S3C_CICOCTRL_CBURST2(x)			((x) << 4)

#define S3C_CICOSCPRERATIO_SHFACTOR(x)		((x) << 28)
#define S3C_CICOSCPRERATIO_PREHORRATIO(x)	((x) << 16)
#define S3C_CICOSCPRERATIO_PREVERRATIO(x)	((x) << 0)

#define S3C_CICOSCPREDST_PREDSTWIDTH(x)		((x) << 16)
#define S3C_CICOSCPREDST_PREDSTHEIGHT(x)	((x) << 0)

#define S3C_CICOSCCTRL_MAINHORRATIO(x)		((x) << 16)
#define S3C_CICOSCCTRL_MAINVERRATIO(x)		((x) << 0)

#define S3C_CICOTAREA_TARGET_AREA(x)		((x) << 0)

#define S3C_CICOSTATUS_GET_FRAME_COUNT(x)	(((x) >> 26) & 0x3)
#define S3C_CICOSTATUS_GET_FRAME_END(x)		(((x) >> 17) & 0x1)

#define S3C_CIPRTRGFMT_TARGETHSIZE(x)		((x) << 16)
#define S3C_CIPRTRGFMT_TARGETVSIZE(x)		((x) << 0)

#define S3C_CIPRCTRL_YBURST1(x)			((x) << 19)
#define S3C_CIPRCTRL_YBURST2(x)			((x) << 14)
#define S3C_CIPRCTRL_CBURST1(x)			((x) << 9)
#define S3C_CIPRCTRL_CBURST2(x)			((x) << 4)

#define S3C_CIPRSCPRERATIO_SHFACTOR(x)		((x) << 28)
#define S3C_CIPRSCPRERATIO_PREHORRATIO(x)	((x) << 16)
#define S3C_CIPRSCPRERATIO_PREVERRATIO(x)	((x) << 0)

#define S3C_CIPRSCPREDST_PREDSTWIDTH(x)		((x) << 16)
#define S3C_CIPRSCPREDST_PREDSTHEIGHT(x)	((x) << 0)

#define S3C_CIPRSCCTRL_MAINHORRATIO(x)		((x) << 16)
#define S3C_CIPRSCCTRL_MAINVERRATIO(x)		((x) << 0)

#define S3C_CIPRTAREA_TARGET_AREA(x)		((x) << 0)

#define S3C_CIPRSTATUS_GET_FRAME_COUNT(x)	(((x) >> 26) & 0x3)
#define S3C_CIPRSTATUS_GET_FRAME_END(x)		(((x) >> 19) & 0x1)

#define S3C_CIIMGEFF_PAT_CB(x)			((x) << 13)
#define S3C_CIIMGEFF_PAT_CR(x)			((x) << 0)

#define S3C_MSCO_HEIGHT(x)			((x) << 16)
#define S3C_MSCO_WIDTH(x)			((x) << 0)

#define S3C_MSPR_HEIGHT(x)			((x) << 16)
#define S3C_MSPR_WIDTH(x)			((x) << 0)

/*************************************************************************
 * Bit definition part
 ************************************************************************/
/* Source format register */
#define S3C_CISRCFMT_ITU601_8BIT		(1 << 31)
#define S3C_CISRCFMT_ITU656_8BIT		(0 << 31)
#define S3C_CISRCFMT_ORDER422_YCBYCR		(0 << 14)
#define S3C_CISRCFMT_ORDER422_YCRYCB		(1 << 14)
#define S3C_CISRCFMT_ORDER422_CBYCRY		(2 << 14)
#define S3C_CISRCFMT_ORDER422_CRYCBY		(3 << 14)

/* Window offset register */
#define S3C_CIWDOFST_WINOFSEN			(1 << 31)
#define S3C_CIWDOFST_CLROVCOFIY			(1 << 30)
#define S3C_CIWDOFST_CLROVRLB_PR		(1 << 28)
#define S3C_CIWDOFST_CLROVPRFIY			(1 << 27)
#define S3C_CIWDOFST_CLROVCOFICB		(1 << 15)
#define S3C_CIWDOFST_CLROVCOFICR		(1 << 14)
#define S3C_CIWDOFST_CLROVPRFICB		(1 << 13)
#define S3C_CIWDOFST_CLROVPRFICR		(1 << 12)
#define S3C_CIWDOFST_WINHOROFST_MASK		(0x7ff << 16)
#define S3C_CIWDOFST_WINVEROFST_MASK		(0x7ff << 0)

/* Global control register */
#define S3C_CIGCTRL_SWRST			(1 << 31)
#define S3C_CIGCTRL_CAMRST			(1 << 30)
#define S3C_CIGCTRL_TESTPATTERN_NORMAL		(0 << 27)
#define S3C_CIGCTRL_TESTPATTERN_COLOR_BAR	(1 << 27)
#define S3C_CIGCTRL_TESTPATTERN_HOR_INC		(2 << 27)
#define S3C_CIGCTRL_TESTPATTERN_VER_INC		(3 << 27)
#define S3C_CIGCTRL_TESTPATTERN_MASK		(3 << 27)
#define S3C_CIGCTRL_TESTPATTERN_SHIFT		(27)
#define S3C_CIGCTRL_INVPOLPCLK			(1 << 26)
#define S3C_CIGCTRL_INVPOLVSYNC			(1 << 25)
#define S3C_CIGCTRL_INVPOLHREF			(1 << 24)
#define S3C_CIGCTRL_IRQ_OVFEN			(1 << 22)
#define S3C_CIGCTRL_HREF_MASK			(1 << 21)
#define S3C_CIGCTRL_IRQ_EDGE			(0 << 20)
#define S3C_CIGCTRL_IRQ_LEVEL			(1 << 20)
#define S3C_CIGCTRL_IRQ_CLR_C			(1 << 19)
#define S3C_CIGCTRL_IRQ_CLR_P			(1 << 18)
#define S3C_CIGCTRL_PROGRESSIVE			(0 << 0)
#define S3C_CIGCTRL_INTERLACE			(1 << 0)

/* Window offset2 register */
#define S3C_CIWDOFST_WINHOROFST2_MASK		(0xfff << 16)
#define S3C_CIWDOFST_WINVEROFST2_MASK		(0xfff << 16)

/* Target format register */
#define S3C_CICOTRGFMT_OUTFORMAT_YCBCR420	(0 << 29)
#define S3C_CICOTRGFMT_OUTFORMAT_YCBCR422	(1 << 29)
#define S3C_CICOTRGFMT_OUTFORMAT_YCBCR422I	(2 << 29)
#define S3C_CICOTRGFMT_OUTFORMAT_RGB		(3 << 29)
#define S3C_CICOTRGFMT_FLIP_SHIFT		(14)
#define S3C_CICOTRGFMT_FLIP_NORMAL		(0 << 14)
#define S3C_CICOTRGFMT_FLIP_X_MIRROR		(1 << 14)
#define S3C_CICOTRGFMT_FLIP_Y_MIRROR		(2 << 14)
#define S3C_CICOTRGFMT_FLIP_180			(3 << 14)
#define S3C_CICOTRGFMT_FLIP_MASK		(3 << 14)

/* Output DMA control register */
#define S3C_CICOCTRL_BURST_MASK			(0xfffff << 4)
#define S3C_CICOCTRL_LASTIRQ_ENABLE		(1 << 2)
#define S3C_CICOCTRL_ORDER422_MASK		(3 << 0)

/* Main scaler control register */
#define S3C_CICOSCCTRL_SCALERBYPASS		(1 << 31)
#define S3C_CICOSCCTRL_SCALEUP_H		(1 << 30)
#define S3C_CICOSCCTRL_SCALEUP_V		(1 << 29)
#define S3C_CICOSCCTRL_CSCR2Y_NARROW		(0 << 28)
#define S3C_CICOSCCTRL_CSCR2Y_WIDE		(1 << 28)
#define S3C_CICOSCCTRL_CSCY2R_NARROW		(0 << 27)
#define S3C_CICOSCCTRL_CSCY2R_WIDE		(1 << 27)
#define S3C_CICOSCCTRL_LCDPATHEN_FIFO		(1 << 26)
#define S3C_CICOSCCTRL_PROGRESSIVE		(0 << 25)
#define S3C_CICOSCCTRL_INTERLACE		(1 << 25)
#define S3C_CICOSCCTRL_SCALERSTART		(1 << 15)
#define S3C_CICOSCCTRL_INRGB_FMT_RGB565		(0 << 13)
#define S3C_CICOSCCTRL_INRGB_FMT_RGB666		(1 << 13)
#define S3C_CICOSCCTRL_INRGB_FMT_RGB888		(2 << 13)
#define S3C_CICOSCCTRL_OUTRGB_FMT_RGB565	(0 << 11)
#define S3C_CICOSCCTRL_OUTRGB_FMT_RGB666	(1 << 11)
#define S3C_CICOSCCTRL_OUTRGB_FMT_RGB888	(2 << 11)
#define S3C_CICOSCCTRL_EXTRGB_NORMAL		(0 << 10)
#define S3C_CICOSCCTRL_EXTRGB_EXTENSION		(1 << 10)
#define S3C_CICOSCCTRL_ONE2ONE			(1 << 9)

/* Status register */
#define S3C_CICOSTATUS_OVFIY			(1 << 31)
#define S3C_CICOSTATUS_OVFICB			(1 << 30)
#define S3C_CICOSTATUS_OVFICR			(1 << 29)
#define S3C_CICOSTATUS_VSYNC			(1 << 28)
#define S3C_CICOSTATUS_WINOFSTEN		(1 << 25)
#define S3C_CICOSTATUS_IMGCPTEN			(1 << 22)
#define S3C_CICOSTATUS_IMGCPTENSC		(1 << 21)
#define S3C_CICOSTATUS_VSYNC_A			(1 << 20)
#define S3C_CICOSTATUS_FRAMEEND			(1 << 17)

/* Target format register */
#define S3C_CIPRTRGFMT_OUTFORMAT_YCBCR420	(0 << 29)
#define S3C_CIPRTRGFMT_OUTFORMAT_YCBCR422	(1 << 29)
#define S3C_CIPRTRGFMT_OUTFORMAT_YCBCR422I	(2 << 29)
#define S3C_CIPRTRGFMT_OUTFORMAT_RGB		(3 << 29)
#define S3C_CIPRTRGFMT_FLIP_SHIFT		(14)
#define S3C_CIPRTRGFMT_FLIP_NORMAL		(0 << 14)
#define S3C_CIPRTRGFMT_FLIP_X_MIRROR		(1 << 14)
#define S3C_CIPRTRGFMT_FLIP_Y_MIRROR		(2 << 14)
#define S3C_CIPRTRGFMT_FLIP_180			(3 << 14)
#define S3C_CIPRTRGFMT_FLIP_MASK		(3 << 14)
#define S3C_CIPRTRGFMT_ROT90_CLOCKWISE		(1 << 13)

/* Output DMA control register */
#define S3C_CIPRCTRL_BURST_MASK			(0xfffff << 4)
#define S3C_CIPRCTRL_LASTIRQ_ENABLE		(1 << 2)
#define S3C_CIPRCTRL_ORDER422_MASK		(3 << 0)

/* Main scaler control register */
#define S3C_CIPRSCCTRL_SCALERBYPASS		(1 << 31)
#define S3C_CIPRSCCTRL_SCALEUP_H		(1 << 30)
#define S3C_CIPRSCCTRL_SCALEUP_V		(1 << 29)
#define S3C_CIPRSCCTRL_CSCR2Y_NARROW		(0 << 28)
#define S3C_CIPRSCCTRL_CSCR2Y_WIDE		(1 << 28)
#define S3C_CIPRSCCTRL_CSCY2R_NARROW		(0 << 27)
#define S3C_CIPRSCCTRL_CSCY2R_WIDE		(1 << 27)
#define S3C_CIPRSCCTRL_LCDPATHEN_FIFO		(1 << 26)
#define S3C_CIPRSCCTRL_PROGRESSIVE		(0 << 25)
#define S3C_CIPRSCCTRL_INTERLACE		(1 << 25)
#define S3C_CIPRSCCTRL_SCALERSTART		(1 << 15)
#define S3C_CIPRSCCTRL_INRGB_FMT_RGB565		(0 << 13)
#define S3C_CIPRSCCTRL_INRGB_FMT_RGB666		(1 << 13)
#define S3C_CIPRSCCTRL_INRGB_FMT_RGB888		(2 << 13)
#define S3C_CIPRSCCTRL_OUTRGB_FMT_RGB565	(0 << 11)
#define S3C_CIPRSCCTRL_OUTRGB_FMT_RGB666	(1 << 11)
#define S3C_CIPRSCCTRL_OUTRGB_FMT_RGB888	(2 << 11)
#define S3C_CIPRSCCTRL_EXTRGB_NORMAL		(0 << 10)
#define S3C_CIPRSCCTRL_EXTRGB_EXTENSION		(1 << 10)
#define S3C_CIPRSCCTRL_ONE2ONE			(1 << 9)

/* Status register */
#define S3C_CIPRSTATUS_OVFIY			(1 << 31)
#define S3C_CIPRSTATUS_OVFICB			(1 << 30)
#define S3C_CIPRSTATUS_OVFICR			(1 << 29)
#define S3C_CIPRSTATUS_VSYNC			(1 << 28)
#define S3C_CIPRSTATUS_WINOFSTEN		(1 << 25)
#define S3C_CIPRSTATUS_IMGCPTEN			(1 << 22)
#define S3C_CIPRSTATUS_IMGCPTENSC		(1 << 21)
#define S3C_CIPRSTATUS_VSYNC_A			(1 << 20)
#define S3C_CIPRSTATUS_FRAMEEND			(1 << 19)

/* Image capture enable register */
#define S3C_CIIMGCPT_IMGCPTEN			(1 << 31)
#define S3C_CIIMGCPT_IMGCPTEN_COSC		(1 << 30)
#define S3C_CIIMGCPT_IMGCPTEN_PRSC		(1 << 29)
#define S3C_CIIMGCPT_CPT_FREN_ENABLE_CO		(1 << 25)
#define S3C_CIIMGCPT_CPT_FREN_ENABLE_PR		(1 << 24)
#define S3C_CIIMGCPT_CPT_FRMOD_EN		(0 << 18)
#define S3C_CIIMGCPT_CPT_FRMOD_CNT		(1 << 18)

/* Image effects register */
#define S3C_CIIMGEFF_IE_DISABLE_PR		(0 << 31)
#define S3C_CIIMGEFF_IE_ENABLE_PR		(1 << 31)
#define S3C_CIIMGEFF_IE_DISABLE_CO		(0 << 30)
#define S3C_CIIMGEFF_IE_ENABLE_CO		(1 << 30)
#define S3C_CIIMGEFF_IE_SC_BEFORE		(0 << 29)
#define S3C_CIIMGEFF_IE_SC_AFTER		(1 << 29)
#define S3C_CIIMGEFF_FIN_BYPASS			(0 << 26)
#define S3C_CIIMGEFF_FIN_ARBITRARY		(1 << 26)
#define S3C_CIIMGEFF_FIN_NEGATIVE		(2 << 26)
#define S3C_CIIMGEFF_FIN_ARTFREEZE		(3 << 26)
#define S3C_CIIMGEFF_FIN_EMBOSSING		(4 << 26)
#define S3C_CIIMGEFF_FIN_SILHOUETTE		(5 << 26)
#define S3C_CIIMGEFF_FIN_MASK			(7 << 26)
#define S3C_CIIMGEFF_PAT_CBCR_MASK		((0xff < 13) | (0xff < 0))

/* Real input DMA size register */
#define S3C_MSCOWIDTH_AUTOLOAD_ENABLE		(1 << 31)

/* Input DMA control register */
#define S3C_MSCOCTRL_ORDER422_YCBYCR		(0 << 4)
#define S3C_MSCOCTRL_ORDER422_YCRYCB		(1 << 4)
#define S3C_MSCOCTRL_ORDER422_CBYCRY		(2 << 4)
#define S3C_MSCOCTRL_ORDER422_CRYCBY		(3 << 4)
#define S3C_MSCOCTRL_INPUT_EXTCAM		(0 << 3)
#define S3C_MSCOCTRL_INPUT_MEMORY		(1 << 3)
#define S3C_MSCOCTRL_INPUT_MASK			(1 << 3)
#define S3C_MSCOCTRL_INFORMAT_YCBCR420		(0 << 1)
#define S3C_MSCOCTRL_INFORMAT_YCBCR422		(1 << 1)
#define S3C_MSCOCTRL_INFORMAT_YCBCR422I		(2 << 1)
#define S3C_MSCOCTRL_INFORMAT_RGB		(3 << 1)
#define S3C_MSCOCTRL_ENVID			(1 << 0)

/* Real input DMA size register */
#define S3C_MSPRWIDTH_AUTOLOAD_ENABLE		(1 << 31)

/* Input DMA control register */
#define S3C_MSPRCTRL_ORDER422_YCBYCR		(0 << 4)
#define S3C_MSPRCTRL_ORDER422_YCRYCB		(1 << 4)
#define S3C_MSPRCTRL_ORDER422_CBYCRY		(2 << 4)
#define S3C_MSPRCTRL_ORDER422_CRYCBY		(3 << 4)
#define S3C_MSPRCTRL_INPUT_EXTCAM		(0 << 3)
#define S3C_MSPRCTRL_INPUT_MEMORY		(1 << 3)
#define S3C_MSPRCTRL_INPUT_MASK			(1 << 3)
#define S3C_MSPRCTRL_INFORMAT_YCBCR420		(0 << 1)
#define S3C_MSPRCTRL_INFORMAT_YCBCR422		(1 << 1)
#define S3C_MSPRCTRL_INFORMAT_YCBCR422I		(2 << 1)
#define S3C_MSPRCTRL_INFORMAT_RGB		(3 << 1)
#define S3C_MSPRCTRL_ENVID			(1 << 0)

#endif /* _REGS_FIMC_H */
