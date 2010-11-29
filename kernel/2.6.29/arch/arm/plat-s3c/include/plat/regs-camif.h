/* linux/include/asm-arm/arch-s3c2410/regs-lcd.h
 *
 * Copyright (c) 2003 Simtec Electronics <linux@simtec.co.uk>
 *		      http://www.simtec.co.uk/products/SWLINUX/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/


#ifndef ___ASM_ARCH_REGS_CAMIF_H
#define ___ASM_ARCH_REGS_CAMIF_H

#define S3C_CAMIFREG(x) (x)

/*************************************************************************
 * Macro part
 ************************************************************************/
#define S3C_CISRCFMT_SOURCEHSIZE(x)			((x) << 16) 	
#define S3C_CISRCFMT_GET_SOURCEHSIZE(x)			(((x) >> 16) & 0x1FFFF) 	
#define S3C_CISRCFMT_SOURCEVSIZE(x)			((x) << 0) 
#define S3C_CISRCFMT_GET_SOURCEVSIZE(x)			(((x) >> 0) & 0x1FFF) 

#define S3C_CIWDOFST_WINHOROFST(x)			((x) << 16)
#define S3C_CIWDOFST_GET_WINHOROFST(x)			(((x) >> 16) & 0x7FF)
#define S3C_CIWDOFST_WINVEROFST(x)			((x) << 0)
#define S3C_CIWDOFST_GET_WINVEROFST(x)			(((x) >> 0) & 0x7FF)

#define S3C_CIDOWSFT2_WINHOROFST2(x)			((x) << 16)
#define S3C_CIDOWSFT2_GET_WINHOROFST2(x)		(((x) >> 16) & 0x7FF)
#define S3C_CIDOWSFT2_WINVEROFST2(x)			((x) << 0)
#define S3C_CIDOWSFT2_GET_WINVEROFST2(x)		(((x) >> 0) & 0x7FF)

#define S3C_CICOTRGFMT_TARGETHSIZE_CO(x)		((x) << 16)
#define S3C_CICOTRGFMT_GET_TARGETHSIZE_CO(x)		(((x) >> 16) & 0x1FFF)

#define S3C_CICOTRGFMT_TARGETVSIZE_CO(x)		((x) << 0)
#define S3C_CICOTRGFMT_GET_TARGETVSIZE_CO(x)		(((x) >> 0) & 0x1FFF)

#define S3C_CICOCTRL_YBURST1_CO(x)			((x) << 19)
#define S3C_CICOCTRL_YBURST2_CO(x)			((x) << 14)
#define S3C_CICOCTRL_CBURST1_CO(x)			((x) << 9)
#define S3C_CICOCTRL_CBURST2_CO(x)			((x) << 4)

#define S3C_CICOSCPRERATIO_SHFACTOR_CO(x)		((x) << 28)
#define S3C_CICOSCPRERATIO_GET_SHFACTOR_CO(x)		(((x) >> 28) & 0x7F)
#define S3C_CICOSCPRERATIO_PREHORRATIO_CO(x)		((x) << 16)
#define S3C_CICOSCPRERATIO_GET_PREHORRATIO_CO(x)	(((x) >> 16) & 0x7F)
#define S3C_CICOSCPRERATIO_PREVERRATIO_CO(x)		((x) << 0)
#define S3C_CICOSCPRERATIO_GET_PREVERRATIO_CO(x)	(((x) >> 0) & 0x7F)

#define S3C_CICOSCPREDST_PREDSTWIDTH_CO(x)		((x) << 16)
#define S3C_CICOSCPREDST_GET_PREDSTWIDTH_CO(x)		(((x) >> 16) & 0x7FF)
#define S3C_CICOSCPREDST_PREDSTHEIGHT_CO(x)		((x) << 0)
#define S3C_CICOSCPREDST_GET_PREDSTHEIGHT_CO(x)		(((x) >> 0) & 0x7FF)

#define S3C_CICOSCCTRL_MAINHORRATIO_CO(x)		((x) << 16)
#define S3C_CICOSCCTRL_GET_MAINHORRATIO_CO(x)		(((x) >> 16) & 0x1FF)
#define S3C_CICOSCCTRL_MAINVERRATIO_CO(x)		((x) << 0)

#define S3C_CICOSTATUS_FRAMECNT_CO(x)			((x) << 26)
#define S3C_CICOSTATUS_GET_FRAMECNT_CO(x)		(((x) >> 26) & 0x3)

#define S3C_CIPRTRGFMT_TARGETHSIZE_PR(x)		((x) << 16)
#define S3C_CIPRTRGFMT_GET_TARGETHSIZE_PR(x)		(((x) >> 16) & 0x1FFF)

#define S3C_CIPRTRGFMT_GET_ROT90_PR(x)			(((x) >> 13) & 0x1)

#define S3C_CIPRTRGFMT_TARGETVSIZE_PR(x)		((x) << 0)
#define S3C_CIPRTRGFMT_GET_TARGETVSIZE_PR(x)		(((x) >> 0) & 0x1FFF)

#define S3C_CIPRSCPRERATIO_SHFACTOR_PR(x)		((x) << 28)
#define S3C_CIPRSCPRERATIO_GET_SHFACTOR_PR(x)		(((x) >> 28) & 0xF)
#define S3C_CIPRSCPRERATIO_PREHORRATIO_PR(x)		((x) << 16)
#define S3C_CIPRSCPRERATIO_GET_PREHORRATIO_PR(x)	(((x) >> 16) & 0x7F)
#define S3C_CIPRSCPRERATIO_PREVERRATIO_PR(x)		((x) << 0)
#define S3C_CIPRSCPRERATIO_GET_PREVERRATIO_PR(x)	(((x) >> 0) & 0x7F)

#define S3C_CIPRSCPREDST_PREDSTWIDTH_PR(x)		((x) << 16)
#define S3C_CIPRSCPREDST_GET_PREDSTWIDTH_PR(x)		(((x) >> 16) & 0xFFF)
#define S3C_CIPRSCPREDST_PREDSTHEIGHT_PR(x)		((x) << 0)
#define S3C_CIPRSCPREDST_GET_PREDSTHEIGHT_PR(x)		(((x) >> 0) & 0xFFF)

#define S3C_CIPRSCCTRL_MAINHORRATIO_PR(x)		((x) << 16)
#define S3C_CIPRSCCTRL_GET_MAINHORRATIO_PR(x)		(((x) >> 16) && 0x1FF)
#define S3C_CIPRSCCTRL_MAINVERRATIO_PR(x)		((x) << 0)
#define S3C_CIPRSCCTRL_GET_MAINVERRATIO_PR(x)		(((x) >> 0) && 0x1FF)

/*************************************************************************
 * Bit definition part
 ************************************************************************/
/* Windows Offset Register */
#define S3C_CIWDOFST_WINOFSEN				(1 << 31)
#define S3C_CIWDOFST_CLROVCOFIY				(1 << 30)
#define S3C_CIWDOFST_CLROVRLB_CO			(1 << 29)
#define S3C_CIWDOFST_CLROVRLB_PR			(1 << 28)
#define S3C_CIWDOFST_CLROVPRFIY				(1 << 27)
#define S3C_CIWDOFST_CLROVCOFICB			(1 << 15)
#define S3C_CIWDOFST_CLROVCOFICR			(1 << 14)
#define S3C_CIWDOFST_CLROVPRFICB			(1 << 13)
#define S3C_CIWDOFST_CLROVPRFICR			(1 << 12)

/* Global Control Register */
#define S3C_CIGCTRL_SWRST				(1 << 31)
#define S3C_CIGCTRL_CAMRST				(1 << 30)

#if defined (CONFIG_CPU_S3C6400) || defined (CONFIG_CPU_S3C6410) 
#define	S3C_CIGCTRL_IRQ_LEVEL				(1 << 20)
#endif

#define S3C_CIGCTRL_TESTPATTERN_VER_INC			(3 << 27)
#define S3C_CIGCTRL_TESTPATTERN_HOR_INC			(2 << 27)
#define S3C_CIGCTRL_TESTPATTERN_COLOR_BAR		(1 << 27)
#define S3C_CIGCTRL_TESTPATTERN_NORMAL			(0 << 27)

#define S3C_CIGCTRL_INVPOLPCLK				(1 << 26)
#define S3C_CIGCTRL_INVPOLVSYNC				(1 << 25)
#define S3C_CIGCTRL_INVPOLHREF				(1 << 24)
#define S3C_CIGCTRL_IRQ_OVFEN				(1 << 22)
#define S3C_CIGCTRL_HREF_MASK				(1 << 21)
#define S3C_CIGCTRL_IRQ_LEVEL				(1 << 20)
#define S3C_CIGCTRL_IRQ_CLR_C				(1 << 19)
#define S3C_CIGCTRL_IRQ_CLR_P				(1 << 18)

/* Codec Target Format Register */
#if defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450)
#define S3C_CICOTRGFMT_IN422_422			(1 << 31)
#define S3C_CICOTRGFMT_IN422_420			(0 << 31)
#define S3C_CICOTRGFMT_OUT422_422			(1 << 30)
#define S3C_CICOTRGFMT_OUT422_420			(0 << 30)

#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410 )
#define S3C_CICOTRGFMT_OUTFORMAT_RGBOUT			(3 << 29)
#define S3C_CICOTRGFMT_OUTFORMAT_YCBCR422OUTINTERLEAVE	(2 << 29)
#define S3C_CICOTRGFMT_OUTFORMAT_YCBCR422OUT		(1 << 29)
#define S3C_CICOTRGFMT_OUTFORMAT_YCBCR420OUT		(0 << 29)
#endif

#define S3C_CICOTRGFMT_INTERLEAVE_ON			(1 << 29)
#define S3C_CICOTRGFMT_INTERLEAVE_OFF			(0 << 29)

#define S3C_CICOTRGFMT_FLIP_180				(3 << 14)
#define S3C_CICOTRGFMT_FLIP_Y_MIRROR			(2 << 14)
#define S3C_CICOTRGFMT_FLIP_X_MIRROR			(1 << 14)
#define S3C_CICOTRGFMT_FLIP_NORMAL			(0 << 14)

/* Codec DMA Control Register */
#define S3C_CICOCTRL_LASTIRQEN				(1 << 2)
#define S3C_CICOCTRL_ORDER422_CRYCBY			(3 << 0)
#define S3C_CICOCTRL_ORDER422_CBYCRY			(2 << 0)
#define S3C_CICOCTRL_ORDER422_YCRYCB			(1 << 0)
#define S3C_CICOCTRL_ORDER422_YCBYCR			(0 << 0)

/* Codec Main-Scaler Control Register */
#define S3C_CICOSCCTRL_SCALERBYPASS_CO			(1 << 31)	
#define S3C_CICOSCCTRL_SCALEUP_H			(1 << 30)
#define S3C_CICOSCCTRL_SCALEUP_V			(1 << 29)

#define S3C_CICOSCCTRL_CSCR2Y_WIDE			(1 << 28)
#define S3C_CICOSCCTRL_CSCR2Y_NARROW			(0 << 28)

#define S3C_CICOSCCTRL_CSCY2R_WIDE			(1 << 27)
#define S3C_CICOSCCTRL_CSCY2R_NARROW			(0 << 27)

#define S3C_CICOSCCTRL_LCDPATHEN_FIFO			(1 << 26)
#define S3C_CICOSCCTRL_LCDPATHEN_DMA			(0 << 26)

#define S3C_CICOSCCTRL_INTERLACE_INTERLACE		(1 << 25)
#define S3C_CICOSCCTRL_INTERLACE_PROGRESSIVE		(0 << 25)

#define S3C_CICOSCCTRL_COSCALERSTART			(1 << 15)

#define S3C_CICOSCCTRL_INRGB_FMT_RGB888			(2 << 13)
#define S3C_CICOSCCTRL_INRGB_FMT_RGB666			(1 << 13)
#define S3C_CICOSCCTRL_INRGB_FMT_RGB565			(0 << 13)

#define S3C_CICOSCCTRL_OUTRGB_FMT_RGB888		(2 << 11)
#define S3C_CICOSCCTRL_OUTRGB_FMT_RGB666		(1 << 11)
#define S3C_CICOSCCTRL_OUTRGB_FMT_RGB565		(0 << 11)

#define S3C_CICOSCCTRL_EXTRGB_EXTENSION			(1 << 10)
#define S3C_CICOSCCTRL_EXTRGB_NORMAL			(0 << 10)

/* Codec Status Register */
#define S3C_CICOSTATUS_OVFIY_CO				(1 << 31)
#define S3C_CICOSTATUS_OVFICB_CO			(1 << 30)
#define S3C_CICOSTATUS_OVFICR_CO			(1 << 29)
#define S3C_CICOSTATUS_VSYNC				(1 << 28)
#define S3C_CICOSTATUS_WINOFSTEN_CO			(1 << 25)
#define S3C_CICOSTATUS_IMGCPTEN_CAMIF			(1 << 22)
#define S3C_CICOSTATUS_IMGCPTEN_COSC			(1 << 21)
#define S3C_CICOSTATUS_VSYNC_A				(1 << 20)
#define S3C_CICOSTATUS_VSYNC_B				(1 << 19)
#define S3C_CICOSTATUS_OVRLB_CO				(1 << 18)
#define S3C_CICOSTATUS_FRAMEEND_CO			(1 << 17)

/* Preview Target Format Register */
#define S3C_CIPRTRGFMT_FLIPMD_180ROT			(3 << 14)
#define S3C_CIPRTRGFMT_FLIPMD_YMIRROR			(2 << 14)
#define S3C_CIPRTRGFMT_FLIPMD_XMIRROR			(1 << 14)
#define S3C_CIPRTRGFMT_FLIPMD_NORMAL			(0 << 14)

#define S3C_CIPRTRGFMT_ROT90_ROTATE			(1 << 13)
#define S3C_CIPRTRGFMT_ROT90_BYPASS			(0 << 13)

/* Preview DMA Control Register */
#define S3C_CIPRCTRL_LASTIRQEN_ENABLE			(1 << 2)
#define S3C_CIPRCTRL_LASTIRQEN_NORMAL			(0 << 2)

#define S3C_CIPRCTRL_ORDER422_CRYCBY			(3 << 0)
#define S3C_CIPRCTRL_ORDER422_CBYCRY			(2 << 0)
#define S3C_CIPRCTRL_ORDER422_YCRYCB			(1 << 0)
#define S3C_CIPRCTRL_ORDER422_YCBYCR			(0 << 0)

/* Preview Main-Scaler Control Register */
#define S3C_CIPRSCCTRL_SAMPLE_PR			(1 << 31)

#define S3C_CIPRSCCTRL_RGBFORMAT_24			(1 << 30)
#define S3C_CIPRSCCTRL_RGBFORMAT_16			(0 << 30)

#define S3C_CIPRSCCTRL_START				(1 << 15)

#define S3C_CIPRSCCTRL_INRGB_FMT_PR_RGB888		(2 << 13)
#define S3C_CIPRSCCTRL_INRGB_FMT_PR_RGB666		(1 << 13)
#define S3C_CIPRSCCTRL_INRGB_FMT_PR_RGB565		(0 << 13)

#define S3C_CIPRSCCTRL_OUTRGB_FMT_PR_RGB888		(2 << 11)
#define S3C_CIPRSCCTRL_OUTRGB_FMT_PR_RGB666		(1 << 11)
#define S3C_CIPRSCCTRL_OUTRGB_FMT_PR_RGB565		(0 << 11)

/* Preview Status Register */
#if defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450)
#define S3C_CIPRSTATUS_OVFICB_PR			(1 << 31)
#define S3C_CIPRSTATUS_OVFICR_PR			(1 << 30)

#elif defined CONFIG_CPU_S3C6400 || defined CONFIG_CPU_S3C6410 
#define S3C_CIPRSTATUS_OVFIY_PR				(1 << 31)
#define S3C_CIPRSTATUS_OVFICB_PR			(1 << 30)
#define S3C_CIPRSTATUS_OVFICR_PR			(1 << 29)
#endif

/* Image Capture Enable Register */
#define S3C_CIIMGCPT_IMGCPTEN				(1 << 31)
#define S3C_CIIMGCPT_IMGCPTEN_COSC			(1 << 30)
#define S3C_CIIMGCPT_IMGCPTEN_PRSC			(1 << 29)

#define S3C_CIIMGCPT_CPT_CODMA_SEL_RGB			(1 << 26)
#define S3C_CIIMGCPT_CPT_CODMA_SEL_YUV			(0 << 26)

#if defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450)
#define S3C_CIIMGCPT_CPT_CODMA_RGBFMT_24		(1 << 25)
#define S3C_CIIMGCPT_CPT_CODMA_RGBFMT_16		(0 << 25)
#define S3C_CIIMGCPT_CPT_CODMA_ENABLE			(1 << 24)
#define S3C_CIIMGCPT_CPT_CODMA_DISABLE			(0 << 24)
#define S3C_CIIMGCPT_CPT_CODMA_MOD_CNT			(1 << 18)
#define S3C_CIIMGCPT_CPT_CODMA_MOD_EN			(0 << 18)

#elif defined CONFIG_CPU_S3C6400 || defined CONFIG_CPU_S3C6410 
#define S3C_CIIMGCPT_CPT_FREN_CO_ENABLE			(1 << 25)
#define S3C_CIIMGCPT_CPT_FREN_CO_DISABLE		(0 << 25)
#define S3C_CIIMGCPT_CPT_FREN_PR_ENABLE			(1 << 24)
#define S3C_CIIMGCPT_CPT_FREN_PR_DISABLE		(0 << 24)
#define S3C_CIIMGCPT_CPT_FRMOD_CNT			(1 << 18)
#define S3C_CIIMGCPT_CPT_FRMOD_EN			(0 << 18)
#endif

/* Image Effects Register */
#define S3C_CIIMGEFF_IE_ON_PR_ENABLE			(1 << 31)
#define S3C_CIIMGEFF_IE_ON_PR_DISABLE			(0 << 31)

#define S3C_CIIMGEFF_IE_ON_CO_ENABLE			(1 << 30)
#define S3C_CIIMGEFF_IE_ON_CO_DISABLE			(0 << 30)

#define S3C_CIIMGEFF_IE_AFTER_SC_BEFORE			(0 << 29)
#define S3C_CIIMGEFF_IE_AFTER_SC_AFTER			(1 << 29)

#define S3C_CIIMGEFF_FIN_SILHOUETTE			(5 << 26)
#define S3C_CIIMGEFF_FIN_EMBOSSING			(4 << 26)
#define S3C_CIIMGEFF_FIN_ARTFREEZE			(3 << 26)
#define S3C_CIIMGEFF_FIN_NEGATIVE			(2 << 26)
#define S3C_CIIMGEFF_FIN_ARBITRARY			(1 << 26)
#define S3C_CIIMGEFF_FIN_BYPASS				(0 << 26)

/* MSDMA for Codec Source Image Width Register */
#define S3C_MSCOWIDTH_AUTOLOAD_ENABLE			(1 << 31)
#define S3C_MSCOWIDTH_AUTOLOAD_DISABLE			(0 << 31)

#define S3C_MSCOWIDTH_ADDR_CH_ENABLE			(1 << 30)
#define S3C_MSCOWIDTH_ADDR_CH_DISABLE			(0 << 30)

/* MSDMA Control Register */
#if defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450)
#define S3C_CIMSCTRL_INTERLEAVE_MS_INTERLEAVE		(1 << 5)
#define S3C_CIMSCTRL_INTERLEAVE_MS_NONINTERLEAVE	(0 << 5)
#define S3C_CIMSCTRL_ORDER422_MS_CRYCBY			(3 << 3)
#define S3C_CIMSCTRL_ORDER422_MS_CBYCRY			(2 << 3)
#define S3C_CIMSCTRL_ORDER422_MS_YCRYCB			(1 << 3)
#define S3C_CIMSCTRL_ORDER422_MS_YCBYCR			(0 << 3)
#define S3C_CIMSCTRL_SEL_DMA_CAM_MEMORY			(1 << 2)
#define S3C_CIMSCTRL_SEL_DMA_CAM_EXTCAM			(0 << 2)
#define S3C_CIMSCTRL_SRC420_MS_420			(1 << 1)
#define S3C_CIMSCTRL_SRC420_MS_422			(0 << 1)
#define S3C_CIMSCTRL_ENVID_MS_SET			(1 << 0)

#elif defined CONFIG_CPU_S3C6400 || defined CONFIG_CPU_S3C6410 
#define S3C_MSCOCTRL_BC_SEL_FRAME			(0 << 10)
#define S3C_MSCOCTRL_BC_SEL_FIELD			(1 << 10)
#define S3C_MSCOCTRL_BUFFER_INI_0			(0 << 8)
#define S3C_MSCOCTRL_BUFFER_INI_1			(1 << 8)
#define S3C_MSCOCTRL_TRG_MODE_SOFT			(0 << 7)
#define S3C_MSCOCTRL_TRG_MODE_HARD			(1 << 7)
#define S3C_MSCOCTRL_ORDER422_M_C_YCBYCR		(0 << 4)
#define S3C_MSCOCTRL_ORDER422_M_C_YCRYCB		(1 << 4)
#define S3C_MSCOCTRL_ORDER422_M_C_CBYCRY		(2 << 4)
#define S3C_MSCOCTRL_ORDER422_M_C_CRYCBY		(3 << 4)
#define S3C_MSCOCTRL_SEL_DMA_CAM_C_EXTCAM		(0 << 3)
#define S3C_MSCOCTRL_SEL_DMA_CAM_C_MEMORY		(1 << 3)
#define S3C_MSCOCTRL_INFORMAT_M_C_420			(0 << 1)
#define S3C_MSCOCTRL_INFORMAT_M_C_422			(1 << 1)
#define S3C_MSCOCTRL_INFORMAT_M_C_422_INT		(2 << 1)
#define S3C_MSCOCTRL_INFORMAT_M_C_RGB			(3 << 1)
#define S3C_MSCOCTRL_ENVID_M_C_SET			(1 << 0)
#define S3C_MSPRCTRL_BC_SEL_FIELD			(0 << 10)
#define S3C_MSPRCTRL_BC_SEL_FRAME			(1 << 10)
#define S3C_MSPRCTRL_BUFFER_INI_0			(0 << 8)
#define S3C_MSPRCTRL_BUFFER_INI_1			(1 << 8)
#define S3C_MSPRCTRL_TRG_MODE_SOFT			(0 << 7)
#define S3C_MSPRCTRL_TRG_MODE_HARD			(1 << 7)
#define S3C_MSPRCTRL_ORDER422_M_P_YCBYCR		(0 << 4)
#define S3C_MSPRCTRL_ORDER422_M_P_YCRYCB		(1 << 4)
#define S3C_MSPRCTRL_ORDER422_M_P_CBYCRY		(2 << 4)
#define S3C_MSPRCTRL_ORDER422_M_P_CRYCBY		(3 << 4)
#define S3C_MSPRCTRL_SEL_DMA_CAM_P_EXTCAM		(0 << 3)
#define S3C_MSPRCTRL_SEL_DMA_CAM_P_MEMORY		(1 << 3)
#define S3C_MSPRCTRL_INFORMAT_M_P_420			(0 << 1)
#define S3C_MSPRCTRL_INFORMAT_M_P_422			(1 << 1)
#define S3C_MSPRCTRL_INFORMAT_M_P_422_INT		(2 << 1)
#define S3C_MSPRCTRL_INFORMAT_M_P_RGB			(3 << 1)
#define S3C_MSPRCTRL_ENVID_M_P_SET			(1 << 0)
#endif

/*************************************************************************
 * Register part
 ************************************************************************/
#define S3C_CICOYSA(__x) 	S3C_CAMIFREG(0x18 + (__x) * 4)
#define S3C_CICOCBSA(__x) 	S3C_CAMIFREG(0x28 + (__x) * 4)
#define S3C_CICOCRSA(__x)  	S3C_CAMIFREG(0x38 + (__x) * 4)
#define S3C_CIPRCLRSA(__x)  	S3C_CAMIFREG(0x6C + (__x) * 4)
#define S3C_CIPRYSA(__x)     	S3C_CAMIFREG(0x6C + (__x) * 4)
#define S3C_CIPRCBSA(__x)   	S3C_CAMIFREG(0x7C + (__x) * 4)
#define S3C_CIPRCRSA(__x)   	S3C_CAMIFREG(0x8C + (__x) * 4)

#define S3C_CISRCFMT		S3C_CAMIFREG(0x00)
#define S3C_CIWDOFST		S3C_CAMIFREG(0x04)
#define S3C_CIGCTRL		S3C_CAMIFREG(0x08)
#define S3C_CIDOWSFT2		S3C_CAMIFREG(0x14)
#define S3C_CICOYSA1		S3C_CAMIFREG(0x18)
#define S3C_CICOYSA2		S3C_CAMIFREG(0x1C)
#define S3C_CICOYSA3		S3C_CAMIFREG(0x20)
#define S3C_CICOYSA4		S3C_CAMIFREG(0x24)
#define S3C_CICOCBSA1		S3C_CAMIFREG(0x28)
#define S3C_CICOCBSA2		S3C_CAMIFREG(0x2C)
#define S3C_CICOCBSA3		S3C_CAMIFREG(0x30)
#define S3C_CICOCBSA4		S3C_CAMIFREG(0x34)
#define S3C_CICOCRSA1		S3C_CAMIFREG(0x38)
#define S3C_CICOCRSA2		S3C_CAMIFREG(0x3C)
#define S3C_CICOCRSA3		S3C_CAMIFREG(0x40)
#define S3C_CICOCRSA4		S3C_CAMIFREG(0x44)
#define S3C_CICOTRGFMT		S3C_CAMIFREG(0x48)	/* CODEC target format */
#define S3C_CICOCTRL		S3C_CAMIFREG(0x4C)	/* CODEC DMA control register */
#define S3C_CICOSCPRERATIO	S3C_CAMIFREG(0x50)	/* CODEC pre-scaler control register 1 */
#define S3C_CICOSCPREDST	S3C_CAMIFREG(0x54)	/* CODEC pre-scaler control register 2 */
#define S3C_CICOSCCTRL		S3C_CAMIFREG(0x58)	/* CODEC main-scaler control */
#define S3C_CICOTAREA		S3C_CAMIFREG(0x5C)	/* CODEC DMA target area register */
#define S3C_CICOSTATUS		S3C_CAMIFREG(0x64)	/* CODEC status register */

#if defined (CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450)
#define S3C_CIPRCLRSA1		S3C_CAMIFREG(0x6C)	/* RGB 1st frame start address for preview DMA */
#define S3C_CIPRCLRSA2		S3C_CAMIFREG(0x70)	/* RGB 2nd frame start address for preview DMA */
#define S3C_CIPRCLRSA3		S3C_CAMIFREG(0x74)	/* RGB 3rd frame start address for preview DMA */
#define S3C_CIPRCLRSA4		S3C_CAMIFREG(0x78)	/* RGB 4th frame start address for preview DMA */
#define S3C_CIPRTRGFMT		S3C_CAMIFREG(0x7C)	/* PREVIEW target format register */
#define S3C_CIPRCTRL		S3C_CAMIFREG(0x80)	/* PREVIEW DMA control register */
#define S3C_CIPRSCPRERATIO	S3C_CAMIFREG(0x84)	/* PREVIEW pre-scaler control register 1 */
#define S3C_CIPRSCPREDST	S3C_CAMIFREG(0x88)	/* PREVIEW pre-scaler control register 2 */
#define S3C_CIPRSCCTRL		S3C_CAMIFREG(0x8C)	/* PREVIEW main-scaler control register */
#define S3C_CIPRTAREA		S3C_CAMIFREG(0x90)	/* PREVIEW DMA target area register */
#define S3C_CIPRSTATUS		S3C_CAMIFREG(0x98)	/* PREVIEW status register */
#define S3C_CIIMGCPT		S3C_CAMIFREG(0xA0)	/* image capture enable register */
#define S3C_CICOCPTSEQ		S3C_CAMIFREG(0xA4)	/* CODEC capture sequence register */
#define S3C_CICOSCOS		S3C_CAMIFREG(0xA8)	/* CODEC scan line offset register */
#define S3C_CIIMGEFF		S3C_CAMIFREG(0xB0)	/* image effect register */
#define S3C_CIMSYSA		S3C_CAMIFREG(0xB4)	/* MSDMA Y start address register */
#define S3C_CIMSCBSA		S3C_CAMIFREG(0xB8)	/* MSDMA CB start address register */
#define S3C_CIMSCRSA		S3C_CAMIFREG(0xBC)	/* MSDMA CR start address register */
#define S3C_CIMSYEND		S3C_CAMIFREG(0xC0)	/* MSDMA Y end address register */
#define S3C_CIMSCBEND		S3C_CAMIFREG(0xC4)	/* MSDMA CB end address register */
#define S3C_CIMSCREND		S3C_CAMIFREG(0xC8)	/* MSDMA CR end address register */
#define S3C_CIMSYOFF		S3C_CAMIFREG(0xCC)	/* MSDMA Y offset register */
#define S3C_CIMSCBOFF		S3C_CAMIFREG(0xD0)	/* MSDMA CB offset register */
#define S3C_CIMSCROFF		S3C_CAMIFREG(0xD4)	/* MSDMA CR offset register */
#define S3C_CIMSWIDTH		S3C_CAMIFREG(0xD8)	/* MSDMA source image width register */
#define S3C_CIMSCTRL		S3C_CAMIFREG(0xDC)	/* MSDMA control register */

#elif defined CONFIG_CPU_S3C6400 || defined CONFIG_CPU_S3C6410 
#define S3C_CIPRYSA1		S3C_CAMIFREG(0x6C)	/* 1st frame start address for preview DMA */
#define S3C_CIPRYSA2		S3C_CAMIFREG(0x70)	/* 2nd frame start address for preview DMA */
#define S3C_CIPRYSA3		S3C_CAMIFREG(0x74)	/* 3rd frame start address for preview DMA */
#define S3C_CIPRYSA4		S3C_CAMIFREG(0x78)	/* 4th frame start address for preview DMA */
#define S3C_CIPRCBSA1		S3C_CAMIFREG(0x7C)	/* 1st frame start address for preview DMA */
#define S3C_CIPRCBSA2		S3C_CAMIFREG(0x80)	/* 2nd frame start address for preview DMA */
#define S3C_CIPRCBSA3		S3C_CAMIFREG(0x84)	/* 3rd frame start address for preview DMA */
#define S3C_CIPRCBSA4		S3C_CAMIFREG(0x88)	/* 4th frame start address for preview DMA */
#define S3C_CIPRCRSA1		S3C_CAMIFREG(0x8C)	/* 1st frame start address for preview DMA */
#define S3C_CIPRCRSA2		S3C_CAMIFREG(0x90)	/* 2nd frame start address for preview DMA */
#define S3C_CIPRCRSA3		S3C_CAMIFREG(0x94)	/* 3rd frame start address for preview DMA */
#define S3C_CIPRCRSA4		S3C_CAMIFREG(0x98)	/* 4th frame start address for preview DMA */
#define S3C_CIPRTRGFMT		S3C_CAMIFREG(0x9C)	/* PREVIEW target format register */
#define S3C_CIPRCTRL		S3C_CAMIFREG(0xA0)	/* PREVIEW DMA control register */
#define S3C_CIPRSCPRERATIO	S3C_CAMIFREG(0xA4)	/* PREVIEW pre-scaler control register 1 */
#define S3C_CIPRSCPREDST	S3C_CAMIFREG(0xA8)	/* PREVIEW pre-scaler control register 2 */
#define S3C_CIPRSCCTRL		S3C_CAMIFREG(0xAC)	/* PREVIEW main-scaler control register */
#define S3C_CIPRTAREA		S3C_CAMIFREG(0xB0)	/* PREVIEW DMA target area register */
#define S3C_CIPRSTATUS		S3C_CAMIFREG(0xB8)	/* PREVIEW status register */
#define S3C_CIIMGCPT		S3C_CAMIFREG(0xC0)	/* image capture enable register */
#define S3C_CICOCPTSEQ		S3C_CAMIFREG(0xC4)	/* CODEC capture sequence register */
#define S3C_CIIMGEFF		S3C_CAMIFREG(0xD0)	/* image effect register */
#define S3C_MSCOY0SA		S3C_CAMIFREG(0xD4)	/* MSDMA for CODEC Y start address register */
#define S3C_MSCOCB0SA		S3C_CAMIFREG(0xD8)	/* MSDMA for CODEC CB start address register */
#define S3C_MSCOCR0SA		S3C_CAMIFREG(0xDC)	/* MSDMA for CODEC CR start address register */
#define S3C_MSCOY0END		S3C_CAMIFREG(0xE0)	/* MSDMA for CODEC Y end address register */
#define S3C_MSCOCB0END		S3C_CAMIFREG(0xE4)	/* MSDMA for CODEC CB end address register */
#define S3C_MSCOCR0END		S3C_CAMIFREG(0xE8)	/* MSDMA for CODEC CR end address register */
#define S3C_MSCOYOFF		S3C_CAMIFREG(0xEC)	/* MSDMA for CODEC Y offset register */
#define S3C_MSCOCBOFF		S3C_CAMIFREG(0xF0)	/* MSDMA for CODEC CB offset register */
#define S3C_MSCOCROFF		S3C_CAMIFREG(0xF4)	/* MSDMA for CODEC CR offset register */
#define S3C_MSCOWIDTH		S3C_CAMIFREG(0xF8)	/* MSDMA for CODEC source image width register */
#define S3C_MSCOCTRL		S3C_CAMIFREG(0xFC)	/* MSDMA for CODEC control register */
#define S3C_MSPRY0SA		S3C_CAMIFREG(0x100)	/* MSDMA for PREVIEW Y0 start address register */
#define S3C_MSPRCB0SA		S3C_CAMIFREG(0x104)	/* MSDMA for PREVIEW CB0 start address register */
#define S3C_MSPRCR0SA		S3C_CAMIFREG(0x108)	/* MSDMA for PREVIEW CR0 start address register */
#define S3C_MSPRY0END		S3C_CAMIFREG(0x10C)	/* MSDMA for PREVIEW Y0 end address register */
#define S3C_MSPRCB0END		S3C_CAMIFREG(0x110)	/* MSDMA for PREVIEW CB0 end address register */
#define S3C_MSPRCR0END		S3C_CAMIFREG(0x114)	/* MSDMA for PREVIEW CR0 end address register */
#define S3C_MSPRYOFF		S3C_CAMIFREG(0x118)	/* MSDMA for PREVIEW Y offset register */
#define S3C_MSPRCBOFF		S3C_CAMIFREG(0x11C)	/* MSDMA for PREVIEW CB offset register */
#define S3C_MSPRCROFF		S3C_CAMIFREG(0x120)	/* MSDMA for PREVIEW CR offset register */
#define S3C_MSPRWIDTH		S3C_CAMIFREG(0x124)	/* MSDMA for PREVIEW source image width register */
#define S3C_CIMSCTRL		S3C_CAMIFREG(0x128)	/* MSDMA for PREVIEW control register */
#define S3C_CICOSCOSY		S3C_CAMIFREG(0x12C)	/* CODEC scan line Y offset register */
#define S3C_CICOSCOSCB		S3C_CAMIFREG(0x130)	/* CODEC scan line CB offset register */
#define S3C_CICOSCOSCR		S3C_CAMIFREG(0x134)	/* CODEC scan line CR offset register */
#define S3C_CIPRSCOSY		S3C_CAMIFREG(0x138)	/* PREVIEW scan line Y offset register */
#define S3C_CIPRSCOSCB		S3C_CAMIFREG(0x13C)	/* PREVIEW scan line CB offset register */
#define S3C_CIPRSCOSCR		S3C_CAMIFREG(0x140)	/* PREVIEW scan line CR offset register */
#endif

#endif /* ___ASM_ARCH_REGS_CAMIF_H */

