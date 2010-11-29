/* linux/include/asm-arm/arch-s3c2410/regs-tvenc.h
 *
 * Copyright (c) 2007 Samsung Electronics
 *		      http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.*/

#ifndef __ASM_ARM_REGS_TVENC_H
#define __ASM_ARM_REGS_TVENC_H

#define S3C_TVCTRL		(0x00)
#define S3C_VBPORCH		(0x04)
#define S3C_HBPORCH		(0x08)
#define S3C_HENHOFFSET		(0x0C)
#define S3C_VDEMOWINSIZE	(0x10)
#define S3C_HDEMOWINSIZE	(0x14)
#define S3C_INIMAGESIZE		(0x18)
#define S3C_PEDCTRL		(0x1C)
#define S3C_YCFILTERBW		(0x20)
#define S3C_HUECTRL		(0x24)
#define S3C_FSCCTRL		(0x28)
#define S3C_FSCDTOMANCTRL	(0x2C)
#define S3C_BGCTRL		(0x34)
#define S3C_BGHVAVCTRL		(0x38)
#define S3C_CONTRABRIGHT	(0x44)
#define S3C_CBCRGAINCTRL	(0x48)
#define S3C_DEMOWINCTRL		(0x4C)
#define S3C_FTCA		(0x50)
#define S3C_BWGAIN		(0x58)
#define S3C_SHARPCTRL		(0x60)
#define S3C_GAMMACTRL		(0x64)
#define S3C_FSCAUXCTRL		(0x68)
#define S3C_SYNCSIZECTRL	(0x6C)
#define S3C_BURSTCTRL		(0x70)
#define S3C_MACROBURSTCTRL	(0x74)
#define S3C_ACTVIDPOCTRL	(0x78)
#define S3C_ENCCTRL		(0x7C)
#define S3C_MUTECTRL		(0x80)
#define S3C_MACROVISION0	(0x84)
#define S3C_MACROVISION1	(0x88)
#define S3C_MACROVISION2	(0x8C)
#define S3C_MACROVISION3	(0x90)
#define S3C_MACROVISION4	(0x94)
#define S3C_MACROVISION5	(0x98)
#define S3C_MACROVISION6	(0x9C)


#define S3C_TVCTRL_FIFOURINT_DIS    0<<16
#define S3C_TVCTRL_FIFOURINT_ENA    1<<16
#define S3C_TVCTRL_FIFOURINT_OCCUR  1<<12
#define S3C_TVCTRL_OUTTYPE_C        0<<8
#define S3C_TVCTRL_OUTTYPE_S        1<<8
#define S3C_TVCTRL_OUTFMT_NTSC_M    0<<4
#define S3C_TVCTRL_OUTFMT_NTSC_J    1<<4
#define S3C_TVCTRL_OUTFMT_PAL_BDG   2<<4
#define S3C_TVCTRL_OUTFMT_PAL_M     3<<4
#define S3C_TVCTRL_OUTFMT_PAL_NC    4<<4
#define S3C_TVCTRL_OFF              0<<0
#define S3C_TVCTRL_ON               1<<0

// vertical back porch control
#define VBP_VEFBPD(n)   (((n)&0x1FF)<<16)
#define VBP_VOFBPD(n)   (((n)&0xFF)<<0)

#define VBP_VEFBPD_NTSC     0x11C<<16
#define VBP_VEFBPD_PAL      0x14F<<16
#define VBP_VOFBPD_NTSC     0x15<<0
#define VBP_VOFBPD_PAL      0x16<<0


// horizontal back porch end point
#define HBP_HSPW(n)     (((n)&0xFF)<<16)
#define HBP_HBPD(n)     (((n)&0x7FF)<<0)

#define HBP_HSPW_NTSC       0x80<<16
#define HBP_HSPW_PAL        0x80<<16
#define HBP_HBPD_NTSC       0xF4<<0
#define HBP_HBPD_PAL        0x108<<0

// horizontal enhancer offset
#define HEO_VAWCC(n)    (((n)&0x3F)<<24)
#define HEO_HAWCC(n)    (((n)&0xFF)<<16)
#define HEO_DTO(n)      (((n)&0x7)<<8)
#define HEO_HEOV(n)     (((n)&0x1F)<<0)

#define HEO_DTO_NTSC        0x4<<8
#define HEO_DTO_PAL         0x4<<8	
#define HEO_HEOV_NTSC       0x1A<<0
#define HEO_HEOV_PAL        0x1A<<0


// vertical demo window size
#define VDW_VDWS(n)     (((n)&0x1FF)<<16)
#define VDW_VDWSP(n)    (((n)&0x1FF)<<0)

#define VDW_VDWS_DEF        0xF0<<16
#define VDW_VDWSP_DEF       0x0<<0


// horizontal demo window size
#define HDW_HDWEP(n)    (((n)&0x7FF)<<16)
#define HDW_HDWSP(n)    (((n)&0x7FF)<<0)

#define HDW_HDWEP_DEF       0x5A0<<16
#define HDW_HDWSP_DEF       0x0<<0


// input image size
#define IIS_HEIGHT(n)   (((n)&0x3FF)<<16)
#define IIS_WIDTH(n)    (((n)&0x7FF)<<0)

// encoder pedestal control
#define EPC_PED_ON          0<<0
#define EPC_PED_OFF         1<<0

// yc filter bandwidth control
#define YFB_YBW_60          0<<4
#define YFB_YBW_38          1<<4
#define YFB_YBW_31          2<<4
#define YFB_YBW_26          3<<4
#define YFB_YBW_21          4<<4
#define YFB_CBW_12          0<<0
#define YFB_CBW_10          1<<0
#define YFB_CBW_08          2<<0
#define YFB_CBW_06          3<<0

// hue control
#define HUE_CTRL(n)     (((n)&0xFF)<<0)

// fsc control
#define FSC_CTRL(n)     (((n)&0x7FFF)<<0)

// fsc dto manually control enable
#define FDM_CTRL(n)     (((n)&0x7FFFFFFF)<<0)

// background control
#define BGC_BGYOFS(n)   (((n)&0xF)<<0)

#define BGC_SME_DIS         0<<8
#define BGC_SME_ENA         1<<8
#define BGC_BGCS_BLACK      0<<4
#define BGC_BGCS_BLUE       1<<4
#define BGC_BGCS_RED        2<<4
#define BGC_BGCS_MAGENTA    3<<4
#define BGC_BGCS_GREEN      4<<4
#define BGC_BGCS_CYAN       5<<4
#define BGC_BGCS_YELLOW     6<<4
#define BGC_BGCS_WHITE      7<<4

// background vav & hav control
#define BVH_BG_HL(n)    (((n)&0xFF)<<24)
#define BVH_BG_HS(n)    (((n)&0xFF)<<16)
#define BVH_BG_VL(n)    (((n)&0xFF)<<8)
#define BVH_BG_VS(n)    (((n)&0xFF)<<0)

// sync size control
#define SSC_HSYNC(n)    (((n)&0x3FF)<<0)

#define SSC_HSYNC_NTSC      0x3D<<0
#define SSC_HSYNC_PAL       0x3E<<0

// burst signal control
#define BSC_BEND(n)     (((n)&0x3FF)<<16)
#define BSC_BSTART(n)   (((n)&0x3FF)<<0)

#define BSC_BEND_NTSC       0x69<<16
#define BSC_BEND_PAL        0x6A<<16
#define BSC_BSTART_NTSC     0x49<<0
#define BSC_BSTART_PAL      0x4A<<0

// macrovision burst signal control
#define MBS_BSTART(n)   (((n)&0x3FF)<<0)

#define MBS_BSTART_NTSC     0x41<<0
#define MBS_BSTART_PAL      0x42<<0

// active video position control
#define AVP_AVEND(n)    (((n)&0x3FF)<<16)
#define AVP_AVSTART(n)  (((n)&0x3FF)<<0)

#define AVP_AVEND_NTSC      0x348<<16
#define AVP_AVEND_PAL       0x352<<16
#define AVP_AVSTART_NTSC    0x78<<0
#define AVP_AVSTART_PAL     0x82<<0

// encoder control
#define ENC_BGEN_DIS        0<<0
#define ENC_BGEN_ENA        1<<0

#define NTSC_WIDTH		(720)
#define NTSC_HEIGHT		(480)
#define PAL_WIDTH		(720)
#define PAL_HEIGHT		(576)

#endif /* __ASM_ARM_REGS_TVENC_H */

