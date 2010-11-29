//
// Copyright (c) Samsung Electronics. Co. LTD.  All rights reserved.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
--*/
/**
*	@file		fimgse2d.h
*	@brief	defines for FIMGSE-2D Graphics Accelerator
*	@author	Jiwon Kim
*	
*	@note This version is made for FIMGSE-2D v2.0
*/
// Header to define the FIMGSE-2D class

#ifndef __FIMGSE2D_H__
#define __FIMGSE2D_H__

#include "regctrl_g2d.h"
#include "s3c_g2d_driver.h"

/**
*    Define G2D Command processing and return type
*
**/
typedef enum
{
	G2D_FASTRETURN, //< Currently, this mode has some bugs, with GDI call
	G2D_INTERRUPT,
	G2D_BUSYWAITING
} G2D_CMDPROCESSING_TYPE;

/**
*    Hardware Limitation Macro
*
**/
/// For Coordinate Register
#define G2D_MAX_WIDTH           (1<<11)        //< 2048
#define G2D_MAX_HEIGHT          (1<<11)        //< 2048

#define MAX_2DHW_WIDTH  (2040)
#define MAX_2DHW_HEIGHT (2040)
#define MAX_2DHW_XCOORD (2040)
#define MAX_2DHW_YCOORD (2040)

// ROP_REG (0x410)
#define G2D_TRANSPARENT_BIT     (1<<9)
#define G2D_OPAQUE_BIT          (0<<9)

// Color_Mode_Reg (0x510[2:0])
#define G2D_COLOR_RGB_565       (0) 
#define G2D_COLOR_RGBA_5551     (1)
#define G2D_COLOR_ARGB_1555     (2)
#define G2D_COLOR_RGBA_8888     (3)
#define G2D_COLOR_ARGB_8888     (4)
#define G2D_COLOR_XRGB_8888     (5)
#define G2D_COLOR_RGBX_8888     (6)
#define G2D_COLOR_UNUSED        (7)

#define MS_NUM_SUPPORT_COLORMODE    (10)        //EGPEFormat


// CMD0_REG (Line) (0x100)
#define G2D_REND_POINT_BIT              (1<<0)
#define G2D_REND_LINE_BIT               (1<<1)
#define G2D_MAJOR_COORD_X_BIT           (1<<8)
#define G2D_MAJOR_COORD_Y_BIT           (0<<8)
#define G2D_NOT_DRAW_LAST_POINT_BIT     (1<<9)
#define G2D_DRAW_LAST_POINT_BIT         ~(1<<9)

// CMD1_REG (BitBlt) (0x104)
#define G2D_STRETCH_BITBLT_BIT          (1<<1)
#define G2D_NORMAL_BITBLT_BIT           (1<<0)

#define ABS(v)                          (((v)>=0) ? (v):(-(v)))
#define START_ASCII                     (0x20)
#define OPAQUE_ENABLE                   (0<<9)

// Set fading and alpha value
#define FADING_OFFSET_DISABLE           (0x0<<8)
#define ALPHA_VALUE_DISABLE             (0xff<<0)



#define HOST2SCREEN         (0)
#define SCREEN2SCREEN       (1)

// G2D Source           0xf0
// G2D Dest             0xcc
// G2D Pattern          0xaa
// MS ROP Pattern       0xf0
// MS ROP Source        0xcc
// MS ROP Dest          0xaa
//                       G2D     MS
// SRC_ONLY             0xf0    0xcc        // SRCCOPY : S
// DST_ONLY             0xcc    0xaa        // DSTCOPY : D
// PAT_ONLY             0xaa    0xf0        // PATCOPY : P
// SRC_OR_DST           0xfc    0xee        // SRCPAINT : S | D
// SRC_OR_PAT           0xfa    0xfc        // P | S --> 0xF0008A
// DST_OR_PAT           0xee    0xfa        // R2_MERGEPEN : P | D
// SRC_AND_DST          0xc0    0x88        // SRCAND : S & D
// SRC_AND_PAT          0xa0    0xc0        // MERGECOPY : S & P
// DST_AND_PAT          0x88    0xa0        // R2_MASKPEN : P & D
// SRC_XOR_DST          0x3c    0x66        // SRCINVERT : S ^ D
// SRC_XOR_PAT          0x5a    0x3c        //  X
// DST_XOR_PAT          0x66    0x5a        // PATINVERT : P ^ D
// NOTSRCCOPY           0x0f    0x33        // NOTSRCCOPY : ~S
// DSTINVERT            0x33    0x55        // DSTINVERT : ~D
// R2_NOTCOPYPEN        0x55    0x0f        // R2_NOTCOPYPEN : ~P
//
#define G2D_ROP_SRC_ONLY        (0xf0)
#define G2D_ROP_PAT_ONLY        (0xaa)
#define G2D_ROP_DST_ONLY        (0xcc)
#define G2D_ROP_SRC_OR_DST      (0xfc)
#define G2D_ROP_SRC_OR_PAT      (0xfa)
#define G2D_ROP_DST_OR_PAT      (0xee)
#define G2D_ROP_SRC_AND_DST     (0xc0) //(pat==1)? src:dst
#define G2D_ROP_SRC_AND_PAT     (0xa0)
#define G2D_ROP_DST_AND_PAT     (0x88)
#define G2D_ROP_SRC_XOR_DST     (0x3c)
#define G2D_ROP_SRC_XOR_PAT     (0x5a)
#define G2D_ROP_DST_XOR_PAT     (0x66)
#define G2D_ROP_NOTSRCCOPY      (0x0f)
#define G2D_ROP_DSTINVERT       (0x33)
#define G2D_ROP_R2_NOTCOPYPEN   (0x55)
#define G2D_NUM_SUPPORT_ROP     (15)

typedef struct
{
	u32    dwBaseaddr;
	u32    dwHoriRes;
	u32    dwVertRes;
	u32    dwColorMode;

} SURFACE_DESCRIPTOR, *PSURFACE_DESCRIPTOR;

typedef enum
{
	ROP_SRC_ONLY = 0,            //O
	ROP_PAT_ONLY = 1,            //O
	ROP_DST_ONLY = 2,            //O
	ROP_SRC_OR_DST = 3,        //O
	ROP_SRC_OR_PAT = 4,        //O
	ROP_DST_OR_PAT = 5,        //O
	ROP_SRC_AND_DST = 6,    //O
	ROP_SRC_AND_PAT = 7,    //O
	ROP_DST_AND_PAT = 8,    //N
	ROP_SRC_XOR_DST = 9,    //N
	ROP_SRC_XOR_PAT = 10,    //O
	ROP_DST_XOR_PAT = 11,    //N
	ROP_NOTSRCCOPY = 12,        //N
	ROP_DSTINVERT = 13,        //N
	ROP_R2_NOTCOPYPEN = 14        //N
} G2D_ROP_TYPE;


// This enumerate value can be used diRECTy to set register
typedef enum
{
	G2D_NO_ALPHA_MODE =         (0<<10),
	G2D_PP_ALPHA_SOURCE_MODE =  (1<<10),
	G2D_ALPHA_MODE =            (2<<10),
	G2D_FADING_MODE =           (4<<10)
} G2D_ALPHA_BLENDING_MODE;

typedef enum
{
	QCIF, CIF/*352x288*/, 
	QQVGA, QVGA, VGA, SVGA/*800x600*/, SXGA/*1280x1024*/, UXGA/*1600x1200*/, QXGA/*2048x1536*/,
	WVGA/*854x480*/, HD720/*1280x720*/, HD1080/*1920x1080*/
} IMG_SIZE;

typedef enum
{
	ALPHAFX_NOALPHA			= 0,
	ALPHAFX_CONSTALPHA,
	ALPHAFX_PERPIXELALPHA,
	ALPHAFX_FADE,
	NUMOF_ALPHAFX
} ALPHAFX;

#define    HASBIT_COND(var,cond)        (((var&cond) == cond) ? TRUE : FALSE)

typedef struct
{
	volatile PG2D_REG m_pG2DReg;
	
	u8      m_iROPMapper[G2D_NUM_SUPPORT_ROP];
	long    m_iColorModeMapper[NUMOF_CS];
	long	m_iAlphaModeMapper[NUMOF_ALPHAFX];

	/// Source Surface Descriptor
	SURFACE_DESCRIPTOR    m_descSrcSurface;
	/// Destination Surface Descriptor
	SURFACE_DESCRIPTOR    m_descDstSurface;
	//  Max Window Size of clipping window
	RECT    m_rtClipWindow;

	u32  m_uMaxDx;
	u32  m_uMaxDy;

	// Coordinate (X, Y) of clipping window
	u32  m_uCwX1, m_uCwY1;
	u32  m_uCwX2, m_uCwY2;

	u32  m_uFgColor;
	u32  m_uBgColor;
	u32  m_uBlueScreenColor;
	u32  m_uColorVal[8];

	// Reference to Raster operation
	u32  m_uRopVal; // Raster operation value
	u32  m_uAlphaBlendMode;
	u32  m_uTransparentMode;
	u32  m_u3rdOprndSel;

	// Reference to alpha value
	u32  m_uFadingOffsetVal;
	u32  m_uAlphaVal;

	// Reference to image rotation
	u32  m_uRotOrgX, m_uRotOrgY;
	u32  m_uRotAngle;

	// reference to pattern of bitblt
	u32  m_uPatternOffsetX, m_uPatternOffsetY;

	u32  m_uu8s;

	u8   m_ucAlphaVal;
	bool m_bIsAlphaCall;

	// true: BitBlt enable in Host-To-Screen Font Drawing
	// false: BitBlt disable in Host-To-Screen Font Drawing
	bool m_bIsBitBlt;

	//        u32 m_uFontAddr;
	bool m_bIsScr2Scr;

	// N_24X24, B_24X24, N_16X16, T_8X16, N_8X8, N_8X15
	u8*  m_upFontType;
	u32  m_uFontWidth, m_uFontHeight;
} FIMGSE2D, *PFIMGSE2D;

/// For StretchBlt
u32 CalculateXYIncrFormat(PFIMGSE2D pF, u32 uDividend, u32 uDivisor);
/// For StretchBlt Algorithm Compensation
/// LONG GetCompensatedOffset(PFIMGSE2D pF, u32 usSrcValue, u32 usDstValue);
/// Submit command and wait
bool DoCmd(PFIMGSE2D pF, volatile u32 *CmdReg, u32 CmdValue, G2D_CMDPROCESSING_TYPE eCmdType);

void Init(PFIMGSE2D pF, volatile PG2D_REG pG2DReg);

/// For Common Resource Setting
void SetRopEtype(PFIMGSE2D pF, G2D_ROP_TYPE eRopType);
/// For Rotation Setting
/// ROT_TYPE GetRotType(PFIMGSE2D pF, ROTATE_COEFF m_iRotate)        ;

void SetTransparentMode(PFIMGSE2D pF, bool bIsTransparent, u32 uBsColor);        

void SetColorKeyOn(PFIMGSE2D pF, u32 uColorKey);
void SetColorKeyOff(PFIMGSE2D pF);

void SetFgColor(PFIMGSE2D pF, u32 uFgColor);
void SetBgColor(PFIMGSE2D pF, u32 uBgColor);
void SetBsColor(PFIMGSE2D pF, u32 uBsColor);

void SetSrcSurface(PFIMGSE2D pF, PSURFACE_DESCRIPTOR desc_surface);
void SetDstSurface(PFIMGSE2D pF, PSURFACE_DESCRIPTOR desc_surface);        
void TranslateCoordinateToZero(PFIMGSE2D pF, PSURFACE_DESCRIPTOR pdescDstSurface, PRECT prclDst, PRECT prclClip);        

/// For Bitblt
void BitBlt(PFIMGSE2D pF, PRECT prclSrc, PRECT prclDst, ROT_TYPE m_iRotate);
void StretchBlt(PFIMGSE2D pF, PRECT prclSrc, PRECT prclDst, ROT_TYPE m_iRotate);
void StretchBlt_Bilinear(PFIMGSE2D pF, PRECT prclSrc, PRECT prclDst, ROT_TYPE m_iRotate);
/// BOOL FlipBlt(PFIMGSE2D pF, PRECT prclSrc, PRECT prclDst, ROT_TYPE m_iRotate);
void FillRect(PFIMGSE2D pF, PRECT prclDst, u32 uColor);

/// For Additional Effect Setting
void EnablePlaneAlphaBlending(PFIMGSE2D pF, u8 ucAlphaVal);
void DisablePlaneAlphaBlending(PFIMGSE2D pF);

void EnablePixelAlphaBlending(PFIMGSE2D pF);
void DisablePixelAlphaBlending(PFIMGSE2D pF);

void EnableFadding(PFIMGSE2D pF, u8 ucFadingVal);
void DisableFadding(PFIMGSE2D pF);

void SetAlphaMode(PFIMGSE2D pF, ALPHAFX eMode);
void SetAlphaValue(PFIMGSE2D pF, u8 ucAlphaVal);
void SetFadingValue(PFIMGSE2D pF, u8 ucFadeVal);

/// For Line Drawing
void PutPixel(PFIMGSE2D pF, u32 uPosX, u32 uPosY, u32 uColor);
void PutLine(PFIMGSE2D pF, u32 uPosX1, u32 uPosY1, u32 uPosX2, u32 uPosY2, u32 uColor, bool bIsDrawLastPoint);

/// Advanced Macro Function
void WaitForIdle(PFIMGSE2D pF);

void GetRotationOrgXY(PFIMGSE2D pF, u16 usSrcX1, u16 usSrcY1, u16 usSrcX2, u16 usSrcY2,    u16 usDestX1, u16 usDestY1, ROT_TYPE eRotDegree, u16* usOrgX, u16* usOrgY);

void DisableEffect(PFIMGSE2D pF);

void SetStencilKey(PFIMGSE2D pF, u32 uIsColorKeyOn, u32 uIsInverseOn, u32 uIsSwapOn);
void SetStencilMinMax(PFIMGSE2D pF, u32 uRedMin, u32 uRedMax, u32 uGreenMin, u32 uGreenMax, u32 uBlueMin, u32 uBlueMax);

void SetColorExpansionMethod(PFIMGSE2D pF, bool bIsScr2Scr);

void BlendingOut(PFIMGSE2D pF, u32 uSrcData, u32 uDstData, u8 ucAlphaVal, bool bFading, u8 ucFadingOffset, u32 *uBlendingOut);
void GetRotateCoordinate(PFIMGSE2D pF, u32 uDstX, u32 uDstY, u32 uOrgX, u32 uOrgY, u32 uRType, u32 *uRsltX, u32 *uRsltY);

#endif
