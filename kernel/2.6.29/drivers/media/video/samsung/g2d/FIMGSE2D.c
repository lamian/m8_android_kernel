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
*	@file		fimgse2d.cpp
*	@brief	hardware control implementation for FIMGSE-2D v2.0, This class is a kind of adapter
*	@author	Jiwon Kim
*	
*	@note This version is made for S3C6410.
*/

#include <linux/init.h>

#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <asm/uaccess.h>
#include <linux/errno.h> /* error codes */
#include <asm/div64.h>
#include <linux/tty.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>

#include <linux/version.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/signal.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/kmod.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/semaphore.h>

#include <asm/io.h>
#include <asm/page.h>
#include <asm/irq.h>
#include <linux/mm.h>
#include <linux/moduleparam.h>

#include <mach/hardware.h>
#include <mach/map.h>

#include <plat/power-clock-domain.h>
#include <plat/pm.h>

#include "regs_s3c_g2d.h"
#include "FIMGSE2D.h"
#include "regctrl_g2d.h"

#define assert ASSERT

#undef SWAP
#define SWAP(a,b,type) { type tmp=a; a=b; b=tmp; }


// Set Ternary raster operation
// Support 256 raster operation
// Refer to ternary raster operation table if you know 256 ROP

// Set Alpha Value
void SetAlphaValue(PFIMGSE2D pF, u8 ucAlphaVal)
{
	ucAlphaVal &= 0xff;
	pF->m_pG2DReg->ALPHA = (pF->m_pG2DReg->ALPHA&(~0xff)) | ucAlphaVal;
}

// Set alpha blending mode
void SetAlphaMode(PFIMGSE2D pF, ALPHAFX eMode)
{
	RequestEmptyFifo(pF->m_pG2DReg, 1);
	/*    u32 uAlphaBlend;

	uAlphaBlend =
	(eMode == G2D_NO_ALPHA_MODE) ? G2D_NO_ALPHA_BIT :
	(eMode == G2D_PP_ALPHA_SOURCE_MODE) ? G2D_PP_ALPHA_SOURCE_BIT :
	(eMode == G2D_ALPHA_MODE) ? G2D_ALPHA_BIT : 
	(eMode == G2D_FADING_MODE) ? G2D_FADING_BIT : G2D_NO_ALPHA_BIT;
	*/
	pF->m_pG2DReg->ROP = (pF->m_pG2DReg->ROP & ~(0x7<<10)) | pF->m_iAlphaModeMapper[eMode];//AlphaBlend;
}

#if 0
// Set fade value
void SetFadingValue(PFIMGSE2D pF, u8 ucFadeVal)
{
	ucFadeVal &= 0xff;
	pF->m_pG2DReg->ALPHA = (pF->m_pG2DReg->ALPHA & ~(0xff<<8)) | (ucFadeVal<<8);
}
#endif

void DisableEffect(PFIMGSE2D pF)
{
	pF->m_pG2DReg->ROP &= ~(0x7<<10);
}

#if 0
void EnablePlaneAlphaBlending(PFIMGSE2D pF, u8 ucAlphaVal)
{
	ucAlphaVal &= 0xff;

	// Set Alpha Blending Mode
	m_pG2DReg->ROP = ((m_pG2DReg->ROP) & ~(0x7<<10)) | G2D_ALPHA_MODE;


	// Set Alpha Value
	m_pG2DReg->ALPHA = ((m_pG2DReg->ALPHA) & ~(0xff)) | ucAlphaVal;

	m_ucAlphaVal = ucAlphaVal;
	m_bIsAlphaCall = true;
}

void DisablePlaneAlphaBlending(PFIMGSE2D pF)
{
	DisableEffect();
}

void EnablePixelAlphaBlending(PFIMGSE2D pF) // Only Support 24bpp and Only used in BitBlt
{
	m_pG2DReg->ROP = ((m_pG2DReg->ROP) & ~(0x7<<10)) | G2D_PP_ALPHA_SOURCE_MODE;
}

void DisablePixelAlphaBlending(PFIMGSE2D pF) // Only Support 24bpp and only used in BitBlt
{
	DisableEffect();
}

void EnableFadding(PFIMGSE2D pF, u8 ucFadingVal)
{
	u8 ucAlphaVal;

	ucAlphaVal = (m_bIsAlphaCall == true) ? m_ucAlphaVal : 255;

	ucFadingVal &= 0xff;

	// Set Fadding Mode    
	m_pG2DReg->ROP = ((m_pG2DReg->ROP) & ~(0x7<<10)) | G2D_FADING_MODE;

	// Set Fadding Value    
	m_pG2DReg->ALPHA = ((m_pG2DReg->ALPHA) & ~(0xff<<8)) | (ucFadingVal<<8) | (ucAlphaVal<<0);
}

void DisableFadding(PFIMGSE2D pF)
{
	DisableEffect();
}

/**
*    @fn    GetRotType(int m_iRotate)
*    @brief    This function convert rotation degree value to ROT_TYPE
*
*/
ROT_TYPE GetRotType(PFIMGSE2D pF, ROTATE_COEFF m_iRotate)
{
	switch(m_iRotate)
	{
	case ROTATE_0:
		return    ROT_0;
	case ROTATE_270:
		return    ROT_270;
	case ROTATE_180:
		return    ROT_180;
	case ROTATE_90:
		return    ROT_90;
	case ROTATE_XFLIP:
		return    FLIP_X;
	case ROTATE_YFLIP:
		return    FLIP_Y;
	default:
		return    ROT_0;
	}
	return ROT_0;
}
#endif

/**
*    @fn    u32 CalculateXYIncrFormat(u32 uDividend, u32 uDivisor)
*    @brief    This function returns x_incr or y_incr vaule in register format
*    @input    this function accept real pixel coordinate, ex) (0,0)~(9,9) means that 10pixel by pixel image
*    @return    Result value
*/
u32 CalculateXYIncrFormat(PFIMGSE2D pF, u32 uDividend, u32 uDivisor)
{
	int i;
	u32 uQuotient;
	u32 uUnderPoint=0;

	if(uDivisor == 0)
	{
		uDivisor = 1;    //< this will prevent data abort. but result is incorrect.
	}

	uQuotient = (u32)(uDividend/uDivisor);
	// Quotient should be less than MAX_XCOORD or MAX_YCOORD.
	if(uQuotient > MAX_2DHW_XCOORD) 
	{
// 		RETAILMSG(DISP_ZONE_WARNING, (TEXT("Increment value to stretch can not exceed %d, Value will be set as 1.0\n"), MAX_2DHW_XCOORD));
		return ((1<<11) | 0 );
	}

	uDividend-=(uQuotient*uDivisor);

	/// Now under point is calculated.
	for (i=0; i<12; i++)
	{
		uDividend <<= 1;
		uUnderPoint <<= 1;

		if (uDividend >= uDivisor)
		{
			uUnderPoint = uUnderPoint | 1;
			uDividend -= uDivisor;
		}
// 		DEBUGMSG(DISP_ZONE_2D, (TEXT("uDivend:%x(%d), uDivisor:%x(%d), uUnderPoint:%x(%d)\n"), uDividend, uDividend, uDivisor, uDivisor,uUnderPoint, uUnderPoint));
	}

	uUnderPoint = (uUnderPoint + 1) >> 1;

	return ( uUnderPoint|(uQuotient<<11) );
}

/**
*    @fn    BitBlt(PRECT prclSrc, PRECT prclDst, ROT_TYPE m_iRotate)
*    @param    prclSrc    Source Rectangle
*    @param    prclDst    Destination Rectangle
*    @param    m_iRotate    Rotatation Degree. See also ROT_TYPE type
*    @note This funciton performs real Bit blit using 2D HW. this functio can handle rotation case.
*            There's predefine macro type for presenting rotation register's setting value
*            G2D_ROTATION
@    @sa    ROT_TYPE    this can be set mixed value.
*/
void BitBlt(PFIMGSE2D pF, PRECT prclSrc, PRECT prclDst, ROT_TYPE m_iRotate)
{
	u32 uCmdRegVal=0;
	RECT    rectDst;            //< If rotation case this value must be corrected.

// 	RETAILMSG(DISP_ZONE_ENTER,(TEXT("[2DHW] BitBlt Entry\r\n")));    

	/// Always LeftTop Coordinate is less than RightBottom for Source and Destination Region
	/// assert( (prclSrc->left < prclSrc->right) && (prclSrc->top < prclSrc->bottom) );
	/// assert( (prclDst->left < prclDst->right) && (prclDst->top < prclDst->bottom) );    

	/// Set Destination's Rotation mode
	SetRotationMode(pF->m_pG2DReg, m_iRotate);
	SetCoordinateSrcBlock(pF->m_pG2DReg, prclSrc->left, prclSrc->top, prclSrc->right - 1, prclSrc->bottom - 1);

	if(m_iRotate == ROT_180)        //< origin set to (x2,y2)
	{
		rectDst.left = prclDst->right - 1;                        //< x2
		rectDst.top = prclDst->bottom - 1;                        //< y2
		rectDst.right = 2 * (prclDst->right - 1) - prclDst->left ;        //< x2 + (x2 - x1)
		rectDst.bottom = 2 * (prclDst->bottom -1) - prclDst->top;    //< y2 + (y2 - y1)
	}
	else     if(m_iRotate == ROT_90)        //<In this time, Height and Width are swapped.
	{
		rectDst.left = prclDst->right - 1;                        //< x2
		rectDst.right = prclDst->right - 1 + prclDst->bottom - 1 - prclDst->top;    //< x2 + (y2 - y1)
		rectDst.top = prclDst->top;                                        //< y1
		rectDst.bottom = prclDst->top + prclDst->right - 1 - prclDst->left;        //< y1 + (x2 - x1)
	}
	else     if(m_iRotate == ROT_270)        //<In this time, Height and Width are swapped.
	{
		rectDst.left = prclDst->left;                            //< x1
		rectDst.right = prclDst->left + prclDst->bottom - 1- prclDst->top;        //< x1 + (y2 - y1)
		rectDst.top = prclDst->bottom - 1;                                    //< y2
		rectDst.bottom = prclDst->bottom - 1 + prclDst->right - 1- prclDst->left;    //< y2 + (x2 - x1)
	}
	else     if(m_iRotate == FLIP_X)
	{
		/// Set rotation origin on destination's bottom line.
		rectDst.left = prclDst->left;                    //< x1
		rectDst.right = prclDst->right - 1;                //< x2
		rectDst.top = prclDst->bottom - 1;            //< y2
		rectDst.bottom = prclDst->bottom - 1 + prclDst->bottom - 1 - prclDst->top;    //< y2 + (y2-y1)
	}
	else     if(m_iRotate == FLIP_Y)
	{
		/// Set rotation origin on destination's right line.
		rectDst.left = prclDst->right - 1;                //< x2
		rectDst.right = prclDst->right - 1 + prclDst->right - 1 - prclDst->left;        //< x2 + (x2 - x1)
		rectDst.top = prclDst->top;                    //< y1
		rectDst.bottom = prclDst->bottom - 1;            //< y2
	}
	else        //< ROT_0
	{
		rectDst.left = prclDst->left;
		rectDst.top = prclDst->top;
		rectDst.right = prclDst->right - 1;
		rectDst.bottom = prclDst->bottom - 1;
	}

	SetRotationOrg(pF->m_pG2DReg, (u16)rectDst.left, (u16)rectDst.top);
	SetCoordinateDstBlock(pF->m_pG2DReg, rectDst.left, rectDst.top, rectDst.right, rectDst.bottom);

// 	RETAILMSG(DISP_ZONE_2D,(TEXT("ROT:%d, Src:(%d,%d)~(%d,%d), Dst:(%d,%d)~(%d,%d)\r\n"), 
// 		m_iRotate, prclSrc->left, prclSrc->top, prclSrc->right, prclSrc->bottom, 
// 		rectDst.left, rectDst.top, rectDst.right, rectDst.bottom));

	uCmdRegVal = G2D_NORMAL_BITBLT_BIT;

	DoCmd(pF, &(pF->m_pG2DReg->CMDR1), uCmdRegVal, G2D_INTERRUPT);

// 	RETAILMSG(DISP_ZONE_ENTER,(TEXT("[2DHW] BitBlt Exit\r\n")));            
	/// TODO: Resource Register clearing can be needed.

}

#if 0
/**
*    @fn    GetCompensatedOffset(PRECT prclSrc, PRECT prclDst, ROT_TYPE m_iRotate)
*    @param    prclSrc    Source Rectangle
*    @param    prclDst    Destination Rectangle
*    @param    m_iRotate    Rotatation Degree. See also ROT_TYPE type
*    @note This funciton performs getting offset for stretchblt algorithm compensation
*    @sa    ROT_TYPE
*
**/
LONG GetCompensatedOffset(PFIMGSE2D pF, u32 usSrcValue, u32 usDstValue)

{
	/// Calculate X,Y Offset
	float fIncrement;
	float fStretchRatio;
	float fReferPoint = 0.5;
	LONG i =0;

	fIncrement = (float)usSrcValue / (float)usDstValue;
	fStretchRatio = (float)usDstValue / (float)usSrcValue;

	do
	{
		if(fReferPoint > 1) break;    
		fReferPoint += fIncrement;
		i++;
	} while(1);

// 	RETAILMSG(DISP_ZONE_2D,(TEXT("\n fIncr : %5.6f, fSR: %5.6f, i : %d, Offset : %d"), fIncrement, fStretchRatio, i, (LONG)(fStretchRatio - i)));

	return (fStretchRatio < 1) ? 0 : (LONG)(fStretchRatio - i);
}

/**
*    @fn    StretchBlt_Bilinear(PRECT prclSrc, PRECT prclDst, ROT_TYPE m_iRotate)
*    @param    prclSrc    Source Rectangle
*    @param    prclDst    Destination Rectangle
*    @param    m_iRotate    Rotatation Degree. See also ROT_TYPE type
*    @note This funciton performs real Stretched Bit blit using 2D HW. this functio can handle rotation case.
*            There's predefine macro type for presenting rotation register's setting value
*            G2D_ROTATION
*    @note This function can not support Multiple Operation ex) mirrored + rotation because of HW
*    @sa    ROT_TYPE
**/
void StretchBlt_Bilinear(PFIMGSE2D pF, PRECT prclSrc, PRECT prclDst, ROT_TYPE m_iRotate)
{
	POINT ptCompensatedOffset;
	u16 usSrcWidth = 0;
	u16 usSrcHeight = 0;
	u16 usDstWidth = 0;
	u16 usDstHeight = 0;

	u32 uXIncr = 0;
	u32 uYIncr = 0;
	u32 uCmdRegVal=0;

	RECT    rectDst;
	RECT    rectDstRT;            //< If rotation case this value must be corrected.
	RECT    rectDstLT;
	RECT    rectDstRB;
	RECT    rectDstLB;

// 	RETAILMSG(DISP_ZONE_ENTER,(TEXT("\n[2DHW] StretchBlt Entry")));    

	/// Always LeftTop Coordinate is less than RightBottom for Source and Destination Region
	assert( (prclSrc->left < prclSrc->right) && (prclSrc->top < prclSrc->bottom) );
	assert( (prclDst->left < prclDst->right) && (prclDst->top < prclDst->bottom) );    

	/// Set Stretch parameter
	/// most right and bottom line does not be drawn.
	usSrcWidth=(u16) ABS( prclSrc->right  - prclSrc->left);
	usDstWidth=(u16) ABS( prclDst->right  - prclDst->left);
	usSrcHeight=(u16) ABS( prclSrc->bottom  - prclSrc->top);
	usDstHeight=(u16) ABS( prclDst->bottom  - prclDst->top);

	if((m_iRotate == ROT_90) ||(m_iRotate == ROT_270) )
	{
		SWAP(usDstHeight, usDstWidth, u16);
	}

	/// Stretch ratio calculation, width and height is include last line
	/// ex) 10x10 to 30x30
	/// Given Coordinate parameter
	/// SrcCoord. (0,0)~(10,10) :  srcwidth = 10-0 = 10, srcheight = 10-0 = 10
	/// DstCoord. (30,30)~(60,60) : dstwidth = 60-30 = 30, dstheight = 60-30 = 30
	/// Actual using coordinate
	/// src (0,0)~(9,9)
	/// Dst (30,30)~(59,59)
	/// Increment calculation : srcwidth/dstwidth = 10/30 = 0.33333...

	if(usSrcWidth == usDstWidth && usSrcHeight == usDstHeight)
	{
// 		RETAILMSG(DISP_ZONE_2D, (TEXT("\nThis is not stretch or shrink BLT, redirect to BitBlt, R:%d"), m_iRotate));
		BitBlt(prclSrc, prclDst, m_iRotate);
		return;
	}

	/// calculate horizontal length        
	uXIncr = CalculateXYIncrFormat(usSrcWidth , usDstWidth );    
	SetXIncr(uXIncr);
// 	RETAILMSG(DISP_ZONE_2D,(TEXT("\nXIncr : %d.%d"), (uXIncr&0x003ff800)>>11, (uXIncr & 0x000007ff)));        

	/// calculate vertical length
	uYIncr = CalculateXYIncrFormat(usSrcHeight  , usDstHeight );
	SetYIncr(uYIncr);
// 	RETAILMSG(DISP_ZONE_2D,(TEXT("\nYIncr : %d.%d"), (uYIncr&0x003ff800)>>11, (uYIncr & 0x000007ff)));            

	/// Set Source Region Coordinate
	SetCoordinateSrcBlock(prclSrc->left, prclSrc->top, prclSrc->right - 1, prclSrc->bottom - 1);    

	/// Now We divide destination region by 4 logically.
	/// 1. LeftTop Vertical Bar, 2. RightTop Block, 3. LeftBottom Small Block, 4.RightBottom Horizontal Bar

	/// 1. LeftTop Vertical Bar
	/// This region has destination's height - RightBottom Horizontal bar's height
	/// and has width value can be gotten by this fomula : 
	///     Refered source surface's pixel coordinate = 0.5 + Increment ratio(Src/Dst) * destination's coordinate
	/// In stretchBlt, Destination's coordinate always starts from (0,0)
	/// Here, Bar's width is (max desitnation's coordinate+1) until refered source surface's pixel coordinate is not over 1.0.
	/// ex) 10x10 to 30x30
	///    Increment ratio = 10/30 = 0.33333
	///    Refered source surface's pixel coordinate = 0.5+0.333*0, 0.5+0.333*1, 0.5+0.333*2
	///    0.5, 0.833  meets this condition. so max destination's coordnate is 1. width is 2
	/// then each block of destination's has this region
	/// LT = (0,0)~(1,27), LB(0,28)~(1,29), RT(2,0)~(29,27), RB(2,28)~(29,29)
	///  real stretch ratio is 30/10 = 3. so 2 is less than 3. we need add offset 1
	/// ex) 10x10 to 50x50
	///    Increment ratio = 10/50 = 0.2
	///    Refered source surface's pixel coordinate = 0.5+0.2*0, 0.5+0.2*1, 0.5+0.2*2, 0.5+0.2*3
	///   0.5, 0.7, 0.9 meets this condition. so max destination's coordinate is 2. width is 3
	/// then each block of desitnation's has this region
	/// LT = (0,0)~(2,46), LB(0,47)~(2,49), RT(3,0)~(49,47), RB(3,47)~(49,49)
	///  real stretch ratio is 50/10 = 5. so 3 is less than 5, we need add offset 2
	ptCompensatedOffset.x = GetCompensatedOffset(usSrcWidth, usDstWidth);
	ptCompensatedOffset.y = GetCompensatedOffset(usSrcHeight, usDstHeight);

	///    

	/// Calculate Destination Region Coordinate for each rotation degree
	if(m_iRotate == ROT_180)        //< origin set to (x2,y2)
	{
		rectDst.left = prclDst->right - 1;                        //< x2
		rectDst.top = prclDst->bottom - 1;                        //< y2
		rectDst.right = 2 * (prclDst->right - 1) - prclDst->left ;        //< x2 + (x2 - x1)
		rectDst.bottom = 2 * (prclDst->bottom -1) - prclDst->top;    //< y2 + (y2 - y1)
	}
	else     if(m_iRotate == ROT_90)        //<In this time, Height and Width are swapped.    
	{
		rectDst.left = prclDst->right - 1;                        //< x2
		rectDst.right = prclDst->right - 1 + prclDst->bottom - 1 - prclDst->top;    //< x2 + (y2 - y1)
		rectDst.top = prclDst->top;                                        //< y1
		rectDst.bottom = prclDst->top + prclDst->right - 1 - prclDst->left;        //< y1 + (x2 - x1)
	}
	else     if(m_iRotate == ROT_270)        //<In this time, Height and Width are swapped.    
	{
		rectDst.left = prclDst->left;                            //< x1
		rectDst.right = prclDst->left + prclDst->bottom - 1- prclDst->top;        //< x1 + (y2 - y1)
		rectDst.top = prclDst->bottom - 1;                                    //< y2
		rectDst.bottom = prclDst->bottom - 1 + prclDst->right - 1- prclDst->left;    //< y2 + (x2 - x1)
	}
	else     if(m_iRotate == FLIP_X)
	{
		/// Set rotation origin on destination's bottom line.
		rectDst.left = prclDst->left;                    //< x1
		rectDst.right = prclDst->right - 1;                //< x2
		rectDst.top = prclDst->bottom - 1;            //< y2
		rectDst.bottom = prclDst->bottom - 1 + prclDst->bottom - 1 - prclDst->top;    //< y2 + (y2-y1)
	}
	else     if(m_iRotate == FLIP_Y)
	{
		/// Set rotation origin on destination's right line.
		rectDst.left = prclDst->right - 1;                //< x2
		rectDst.right = prclDst->right - 1 + prclDst->right - 1 - prclDst->left;        //< x2 + (x2 - x1)
		rectDst.top = prclDst->top;                    //< y1
		rectDst.bottom = prclDst->bottom - 1;            //< y2
	}
	else        //< ROT_0
	{
		rectDst.left = prclDst->left;
		rectDst.top = prclDst->top;        
		rectDst.right = prclDst->right - 1;
		rectDst.bottom = prclDst->bottom - 1;
	}

// 	RETAILMSG(DISP_ZONE_2D,(TEXT("\nROT:%d, Src:(%d,%d)~(%d,%d), Dst:(%d,%d)~(%d,%d), OC:(%d,%d)"), 
// 		m_iRotate, prclSrc->left, prclSrc->top, prclSrc->right, prclSrc->bottom, 
// 		rectDst.left, rectDst.top, rectDst.right, rectDst.bottom, rectDst.left, rectDst.top));

	/// Set Destination's Rotation mode
	SetRotationMode(m_iRotate);        
	SetRotationOrg((u16)rectDst.left, (u16)rectDst.top);    
	uCmdRegVal = G2D_STRETCH_BITBLT_BIT;

	if(ptCompensatedOffset.x != 0 || ptCompensatedOffset.y != 0)
	{
		rectDstRB.left = rectDst.left + ptCompensatedOffset.x;
		rectDstRB.right = rectDst.right;
		rectDstRB.top = rectDst.top + ptCompensatedOffset.y;
		rectDstRB.bottom = rectDst.bottom;
		SetClipWindow(&rectDstRB);    //< Reconfigure clip region as Each Block's region    
		SetCoordinateDstBlock(rectDstRB.left, rectDstRB.top, rectDstRB.right + ptCompensatedOffset.x, rectDstRB.bottom + ptCompensatedOffset.y);

		/// First Issuing for Right Bottom Big Region.
		RequestEmptyFifo(1);

// 		RETAILMSG(DISP_ZONE_2D,(TEXT("\nRight Bottom Block : Dst:(%d,%d)~(%d,%d)"), 
// 			rectDstRB.left, rectDstRB.top, rectDstRB.right, rectDstRB.bottom));    

		m_pG2DReg->CMDR1 = uCmdRegVal;

		rectDstRT.left = rectDstRB.left;
		rectDstRT.right = rectDst.right;
		rectDstRT.top = rectDst.top;
		rectDstRT.bottom = rectDstRB.top - 1;
		SetClipWindow(&rectDstRT);
		SetCoordinateDstBlock(rectDstRT.left, rectDst.top, rectDst.right + ptCompensatedOffset.x, rectDst.bottom);

		/// Second Issuing for Right Top Horizontal Bar Region.(in 0, 180 degree)
		RequestEmptyFifo(1);
// 		RETAILMSG(DISP_ZONE_2D,(TEXT("\nRight Top Block : Dst:(%d,%d)~(%d,%d)"), 
// 			rectDstRT.left, rectDstRT.top, rectDstRT.right, rectDstRT.bottom));    

		m_pG2DReg->CMDR1 = uCmdRegVal;


		rectDstLB.left = rectDst.left;
		rectDstLB.right = rectDstRB.left - 1;
		rectDstLB.top = rectDstRB.top;
		rectDstLB.bottom = rectDst.bottom;
		SetClipWindow(&rectDstLB);
		SetCoordinateDstBlock(rectDst.left, rectDstLB.top, rectDst.right, rectDstLB.bottom + ptCompensatedOffset.y);    
		/// Third Issuing for Left Bottom Vertical Bar(in 0,180 degree)
		RequestEmptyFifo(1);
// 		RETAILMSG(DISP_ZONE_2D,(TEXT("\nLeft Bottom Block : Dst:(%d,%d)~(%d,%d)"), 
// 			rectDstLB.left, rectDstLB.top, rectDstLB.right, rectDstLB.bottom));    

		m_pG2DReg->CMDR1 = uCmdRegVal;


		rectDstLT.left = rectDst.left;
		rectDstLT.right = rectDstLB.right;
		rectDstLT.top = rectDst.top;
		rectDstLT.bottom = rectDstRT.bottom;
		SetClipWindow(&rectDstLT);
// 		RETAILMSG(DISP_ZONE_2D,(TEXT("\nLeft Top Block : Dst:(%d,%d)~(%d,%d)"), 
// 			rectDstLT.left, rectDstLT.top, rectDstLT.right, rectDstLT.bottom));    

	}

	SetCoordinateDstBlock(rectDst.left, rectDst.top, rectDst.right, rectDst.bottom);    
	/// Last Issuing for Left Top Small Region(in 0,180 degree)

	DoCmd(&(m_pG2DReg->CMDR1), uCmdRegVal, G2D_BUSYWAITING);    

// 	RETAILMSG(DISP_ZONE_ENTER,(TEXT("\n[2DHW] StretchBlt Exit")));
	/// TODO: Resource Register clearing can be needed.

}
#endif

/// This implementation has distorted stretch result.
/**
*   @fn StretchBlt(PRECT prclSrc, PRECT prclDst, ROT_TYPE m_iRotate)
*   @param  prclSrc Source Rectangle
*   @param  prclDst Destination Rectangle
*   @param  m_iRotate   Rotatation Degree. See also ROT_TYPE type
*   @note This funciton performs real Stretched Bit blit using 2D HW. this functio can handle rotation case.
*       There's predefine macro type for presenting rotation register's setting value
*       G2D_ROTATION
*   @note This function can not support Multiple Operation ex) mirrored + rotation because of HW
*   @sa ROT_TYPE
**/
void StretchBlt(PFIMGSE2D pF, PRECT prclSrc, PRECT prclDst, ROT_TYPE m_iRotate)
{
	u16 usSrcWidth = 0;
	u16 usSrcHeight = 0;
	u16 usDstWidth = 0;
	u16 usDstHeight = 0;
	u32 uXYIncr = 0;
	u32 uCmdRegVal=0;

	RECT    rectDst;            //< If rotation case this value must be corrected.

// 	RETAILMSG(DISP_ZONE_ENTER,(TEXT("[2DHW] StretchBlt Entry\r\n")));    

	/// Always LeftTop Coordinate is less than RightBottom for Source and Destination Region
	/// assert( (prclSrc->left < prclSrc->right) && (prclSrc->top < prclSrc->bottom) );
	/// assert( (prclDst->left < prclDst->right) && (prclDst->top < prclDst->bottom) );    

	/// Set Stretch parameter
	/// Stretch ratio calculation, width and height is not include last line
	usSrcWidth=(u16) ABS( prclSrc->right  - prclSrc->left);
	usDstWidth=(u16) ABS( prclDst->right  - prclDst->left);
	usSrcHeight=(u16) ABS( prclSrc->bottom  - prclSrc->top);
	usDstHeight=(u16) ABS( prclDst->bottom  - prclDst->top);

	if((m_iRotate == ROT_90) ||(m_iRotate == ROT_270) )
	{
		SWAP(usDstHeight, usDstWidth, u16);
	}    

	/// When Orthogonally Rotated operation is conducted,     
	if(usSrcWidth == usDstWidth && usSrcHeight == usDstHeight)
	{
// 		RETAILMSG(DISP_ZONE_2D, (TEXT("This is not stretch or shrink BLT, redirect to BitBlt, R:%d\n"), m_iRotate));
		BitBlt(pF, prclSrc, prclDst, m_iRotate);
		return;
	}

	uXYIncr = CalculateXYIncrFormat(pF, usSrcWidth -1, usDstWidth -1);
	SetXIncr(pF->m_pG2DReg, uXYIncr);    
// 	RETAILMSG(DISP_ZONE_2D,(TEXT("\nXIncr : %d.%x"), (uXYIncr&0x003ff800)>>11, (uXYIncr & 0x000007ff)));    

	uXYIncr = CalculateXYIncrFormat(pF, usSrcHeight -1, usDstHeight -1);
	SetYIncr(pF->m_pG2DReg, uXYIncr);
// 	RETAILMSG(DISP_ZONE_2D,(TEXT("\nYIncr : %d.%x"), (uXYIncr&0x003ff800)>>11, (uXYIncr & 0x000007ff)));        

	SetCoordinateSrcBlock(pF->m_pG2DReg, prclSrc->left, prclSrc->top, prclSrc->right - 1, prclSrc->bottom - 1);    

	if(m_iRotate == ROT_180)        //< origin set to (x2,y2)
	{
		rectDst.left = prclDst->right - 1;                        //< x2
		rectDst.top = prclDst->bottom - 1;                        //< y2
		rectDst.right = 2 * (prclDst->right - 1) - prclDst->left ;        //< x2 + (x2 - x1)
		rectDst.bottom = 2 * (prclDst->bottom -1) - prclDst->top;    //< y2 + (y2 - y1)
	}
	else     if(m_iRotate == ROT_90)        //<In this time, Height and Width are swapped.
	{
		rectDst.left = prclDst->right - 1;                        //< x2
		rectDst.right = prclDst->right - 1 + prclDst->bottom - 1 - prclDst->top;    //< x2 + (y2 - y1)
		rectDst.top = prclDst->top;                                        //< y1
		rectDst.bottom = prclDst->top + prclDst->right - 1 - prclDst->left;        //< y1 + (x2 - x1)
	}
	else     if(m_iRotate == ROT_270)        //<In this time, Height and Width are swapped.
	{
		rectDst.left = prclDst->left;                            //< x1
		rectDst.right = prclDst->left + prclDst->bottom - 1- prclDst->top;        //< x1 + (y2 - y1)
		rectDst.top = prclDst->bottom - 1;                                    //< y2
		rectDst.bottom = prclDst->bottom - 1 + prclDst->right - 1- prclDst->left;    //< y2 + (x2 - x1)
	}
	else     if(m_iRotate == FLIP_X)
	{
		/// Set rotation origin on destination's bottom line.
		rectDst.left = prclDst->left;                    //< x1
		rectDst.right = prclDst->right - 1;                //< x2
		rectDst.top = prclDst->bottom - 1;            //< y2
		rectDst.bottom = prclDst->bottom - 1 + prclDst->bottom - 1 - prclDst->top;    //< y2 + (y2-y1)
	}
	else     if(m_iRotate == FLIP_Y)
	{
		/// Set rotation origin on destination's right line.
		rectDst.left = prclDst->right - 1;                //< x2
		rectDst.right = prclDst->right - 1 + prclDst->right - 1 - prclDst->left;        //< x2 + (x2 - x1)
		rectDst.top = prclDst->top;                    //< y1
		rectDst.bottom = prclDst->bottom - 1;            //< y2
	}
	else        //< ROT_0
	{
		rectDst.left = prclDst->left;
		rectDst.top = prclDst->top;
		rectDst.right = prclDst->right - 1;
		rectDst.bottom = prclDst->bottom - 1;
	}

	/// Set Destination's Rotation mode
	SetRotationMode(pF->m_pG2DReg, m_iRotate);
	SetRotationOrg(pF->m_pG2DReg, (u16)rectDst.left, (u16)rectDst.top);
	SetCoordinateDstBlock(pF->m_pG2DReg, rectDst.left, rectDst.top, rectDst.right, rectDst.bottom);

// 	RETAILMSG(DISP_ZONE_2D,(TEXT("ROT:%d, Src:(%d,%d)~(%d,%d), Dst:(%d,%d)~(%d,%d), OC:(%d,%d)\r\n"), 
// 		m_iRotate, prclSrc->left, prclSrc->top, prclSrc->right, prclSrc->bottom, 
// 		rectDst.left, rectDst.top, rectDst.right, rectDst.bottom, rectDst.left, rectDst.top));

	uCmdRegVal = G2D_STRETCH_BITBLT_BIT;

	DoCmd(pF, &(pF->m_pG2DReg->CMDR1), uCmdRegVal, G2D_INTERRUPT);

// 	RETAILMSG(DISP_ZONE_ENTER,(TEXT("[2DHW] StretchBlt Exit\r\n")));            
	/// TODO: Resource Register clearing can be needed.
}

#if 0
/**
*    @fn    FlipBlt(PRECT prclSrc, PRECT prclDst, ROT_TYPE m_iRotate)
*    @param    prclSrc    Source Rectangle
*    @param    prclDst    Destination Rectangle
*    @param    m_iRotate    Flip Setting. See also ROT_TYPE type
*    @note This funciton performs ONLY FLIP Bit blit using 2D HW. this function cannot handle rotation case.
*            There's predefine macro type for presenting rotation register's setting value
*            This function requires Scratch Memory for Destination.
*            This function don't support X&Y flipping
*    @sa    ROT_TYPE
**/
BOOL FlipBlt(PFIMGSE2D pF, PRECT prclSrc, PRECT prclDst, ROT_TYPE m_iRotate)
{
	u32 uCmdRegVal=0;
	BOOL bRetVal = FALSE;
	RECT    rectDst;            //< If rotation case this value must be corrected.    

// 	RETAILMSG(DISP_ZONE_ENTER,(TEXT("[2DHW] FlipBlt Entry\r\n")));                

	/// Always LeftTop Coordinate is less than RightBottom for Source and Destination Region
	assert( (prclSrc->left < prclSrc->right) && (prclSrc->top < prclSrc->bottom) );
	assert( (prclDst->left < prclDst->right) && (prclDst->top < prclDst->bottom) );

	/// Check Flip Option, we only do care about only flip, don't care about rotation option although it set.
	if(HASBIT_COND(m_iRotate, FLIP_X))
	{
		SetRotationMode(FLIP_X);
		/// Set rotation origin on destination's bottom line.
		rectDst.left = prclDst->left;                    //< x1
		rectDst.right = prclDst->right - 1;                //< x2
		rectDst.top = prclDst->bottom - 1;            //< y2
		rectDst.bottom = prclDst->bottom - 1 + prclDst->bottom - 1 - prclDst->top;    //< y2 + (y2-y1)
	}
	else if(HASBIT_COND(m_iRotate, FLIP_Y))
	{
		SetRotationMode(FLIP_Y);
		/// Set rotation origin on destination's right line.
		rectDst.left = prclDst->right - 1;                //< x2
		rectDst.right = prclDst->right - 1 + prclDst->right - 1 - prclDst->left;        //< x2 + (x2 - x1)
		rectDst.top = prclDst->top;                    //< y1
		rectDst.bottom = prclDst->bottom - 1;            //< y2
	}
	else
	{
		/// Do not need to do Flip operation.
		return FALSE;
	}

	SetCoordinateSrcBlock(prclSrc->left, prclSrc->top, prclSrc->right - 1, prclSrc->bottom - 1);
	SetRotationOrg((u16)rectDst.left, (u16)rectDst.top);
	SetCoordinateDstBlock(rectDst.left, rectDst.top, rectDst.right, rectDst.bottom);

	uCmdRegVal = G2D_NORMAL_BITBLT_BIT;

	bRetVal = DoCmd(&(m_pG2DReg->CMDR1), uCmdRegVal, G2D_BUSYWAITING);

// 	RETAILMSG(DISP_ZONE_ENTER,(TEXT("[2DHW] FlipBlt Exit\r\n")));            
	/// TODO: Resource Register clearing can be needed.

	return bRetVal;

}
#endif

// if ucTransMode is '1', Transparent Mode
// else '0', Opaque Mode
void SetTransparentMode(PFIMGSE2D pF, bool bIsTransparent, u32 uBsColor)
{
	u32 uRopRegVal;

	RequestEmptyFifo(pF->m_pG2DReg, 2);

	uRopRegVal = pF->m_pG2DReg->ROP;

	uRopRegVal =
		(bIsTransparent == 1) ? (uRopRegVal | G2D_TRANSPARENT_BIT) : (uRopRegVal & ~(G2D_TRANSPARENT_BIT));

	pF->m_pG2DReg->ROP = uRopRegVal;

	// register Blue Screen Color
	pF->m_pG2DReg->BS_COLOR = uBsColor;
}

#if 0
// if ucTransMode is '1', Transparent Mode
// else '0', Opaque Mode
void SetColorKeyOn(PFIMGSE2D pF, u32 uBsColor)
{
	RequestEmptyFifo(2);

	m_pG2DReg->ROP = m_pG2DReg->ROP | G2D_TRANSPARENT_BIT;

	// register Blue Screen Color
	m_pG2DReg->BS_COLOR = uBsColor;
}
#endif

void SetColorKeyOff(PFIMGSE2D pF)
{
	RequestEmptyFifo(pF->m_pG2DReg, 2);

	// Blue screen off
	pF->m_pG2DReg->ROP =  pF->m_pG2DReg->ROP & ~(G2D_TRANSPARENT_BIT);

	// color key off
	pF->m_pG2DReg->COLORKEY_CNTL = (pF->m_pG2DReg->COLORKEY_CNTL & ~(0x1U<<31));
}

#if 0
void SetFgColor(PFIMGSE2D pF, u32 uFgColor)
{
	RequestEmptyFifo(1);
	uFgColor &= 0x00ffffff;        //< Remove Alpha value
	m_pG2DReg->FG_COLOR = uFgColor;
}

void SetBgColor(PFIMGSE2D pF, u32 uBgColor)
{
	RequestEmptyFifo(1);
	uBgColor &= 0x00ffffff;        //< Remove Alpha value
	m_pG2DReg->BG_COLOR = uBgColor;
}

void SetBsColor(PFIMGSE2D pF, u32 uBsColor)
{
	RequestEmptyFifo(1);
	uBsColor &= 0x00ffffff;        //< Remove Alpha value
	m_pG2DReg->BS_COLOR = uBsColor;
}


/**
*    @fn    void FillRect(PRECT prtDst, u32 uColor)
*    @param    prtDst    Destination Rectangle
*    @param    uColor    Filled Color
*    @attention    prtDst must have positive value.
*    @brief    prclDst must be rotated when screen is rotated.
*/
void FillRect(PFIMGSE2D pF, PRECT prclDst, COLORREF uColor)
{
	SetFgColor(uColor);
	Set3rdOperand(G2D_OPERAND3_FG);
	SetRopEtype(ROP_PAT_ONLY);
	BitBlt(prclDst, prclDst, ROT_0);        // Fill Rect doesn't care about screen rotation,
}
#endif

/*
*     @fn    void SetSrcSurface(PSURFACE_DESCRIPTOR desc_surface)
*    @brief    Set Source Surface Information to FIMG2D HW Register
*    @param    desc_surface    Surface Information : Base Address, Horizontal&Vertical Resolution, Color mode
*/
void SetSrcSurface(PFIMGSE2D pF, PSURFACE_DESCRIPTOR desc_surface)
{
	RequestEmptyFifo(pF->m_pG2DReg, 4);

	pF->m_pG2DReg->SRC_BASE_ADDR = desc_surface->dwBaseaddr;

	pF->m_pG2DReg->SRC_COLOR_MODE = pF->m_iColorModeMapper[desc_surface->dwColorMode];

	pF->m_pG2DReg->SRC_HORI_RES = desc_surface->dwHoriRes;
	pF->m_pG2DReg->SRC_VERT_RES = desc_surface->dwVertRes;
}

/*
*     @fn    void SetDstSurface(PSURFACE_DESCRIPTOR desc_surface)
*    @brief    Set Destination Surface Information to FIMG2D HW Register
*    @param    desc_surface    Surface Information : Base Address, Horizontal&Vertical Resolution, Color mode
*/
void SetDstSurface(PFIMGSE2D pF, PSURFACE_DESCRIPTOR desc_surface)
{
	RequestEmptyFifo(pF->m_pG2DReg, 4);

	pF->m_pG2DReg->DST_BASE_ADDR = desc_surface->dwBaseaddr;

	pF->m_pG2DReg->DST_COLOR_MODE = pF->m_iColorModeMapper[desc_surface->dwColorMode];

	pF->m_pG2DReg->SC_HORI_RES = desc_surface->dwHoriRes;
	pF->m_pG2DReg->SC_VERT_RES = desc_surface->dwVertRes;
}

#if 0
/*
*  @fn     void TranslateCoordinateToZero(PSURFACE_DESCRIPTOR pdesc_surface, LPRECT prclDst)
*  @brief  Adjust Coordinate from (x1,y1)~(x2,y2) to (0,0)~(x2-x1,y2-y1)
*          Recalculate BaseAddress
*  @param  desc_surface    Surface Information : Base Address, Horizontal&Vertical Resolution, Color mode
*          prclDst         Surface Target Region coordinate
*          prclCLip        Surface Clipping region coordinate
*/
void TranslateCoordinateToZero(PFIMGSE2D pF, PSURFACE_DESCRIPTOR pdesc_surface, PRECT prclDst, PRECT prclClip)
{
	RECT   rtNew;
	RECT   rtNewClip;
	/// NewAddress = OriginalAddress + (Y1 * Hori.Res + X1) * BPP
	/// NewRect : NewRect.Left & top = (0,0), Right & bottom = (X2-X1, Y2-Y1)
	switch(pdesc_surface->dwColorMode)
	{
	case G2D_COLOR_RGB_565:
	case G2D_COLOR_RGBA_5551:
	case G2D_COLOR_ARGB_1555:
		/// 2u8s per pixel
		pdesc_surface->dwBaseaddr = pdesc_surface->dwBaseaddr + (prclDst->top * pdesc_surface->dwHoriRes + prclDst->left) * 2;
		break;
	case G2D_COLOR_RGBA_8888:
	case G2D_COLOR_ARGB_8888:
	case G2D_COLOR_XRGB_8888:
	case G2D_COLOR_RGBX_8888:
		/// 4u8s per pixel
		pdesc_surface->dwBaseaddr = pdesc_surface->dwBaseaddr + (prclDst->top * pdesc_surface->dwHoriRes + prclDst->left) * 4;
		break;
	default:
// 		RETAILMSG(DISP_ZONE_ERROR,(TEXT("[TCTZ] Unsupported Color Format\r\n")));
		break;
	}
	rtNew.left = 0;
	rtNew.top = 0;
	rtNew.right = prclDst->right - prclDst->left;
	rtNew.bottom = prclDst->bottom - prclDst->top;

	if(prclClip)
	{
		rtNewClip.left = 0;
		rtNewClip.top = 0;
		rtNewClip.right = prclClip->right - prclDst->left;
		rtNewClip.bottom = prclClip->bottom - prclDst->top;
		CopyRect((LPRECT)prclClip, (LPRECT) &rtNewClip);
	}

	CopyRect((LPRECT)prclDst, (LPRECT)&rtNew);
	pdesc_surface->dwVertRes -= prclDst->bottom - prclDst->top;
}
#endif

/**
*    Initialize 2D HW
*/
void Init(PFIMGSE2D pF, volatile PG2D_REG pG2DReg) 
{
	//    Reset();
	pF->m_iROPMapper[ROP_SRC_ONLY] = G2D_ROP_SRC_ONLY;
	pF->m_iROPMapper[ROP_PAT_ONLY] = G2D_ROP_PAT_ONLY;
	pF->m_iROPMapper[ROP_DST_ONLY] = G2D_ROP_DST_ONLY;
	pF->m_iROPMapper[ROP_SRC_OR_DST] = G2D_ROP_SRC_OR_DST;
	pF->m_iROPMapper[ROP_SRC_OR_PAT] = G2D_ROP_SRC_OR_PAT;
	pF->m_iROPMapper[ROP_DST_OR_PAT] = G2D_ROP_DST_OR_PAT;
	pF->m_iROPMapper[ROP_SRC_AND_DST] = G2D_ROP_SRC_AND_DST;
	pF->m_iROPMapper[ROP_SRC_AND_PAT] = G2D_ROP_SRC_AND_PAT;
	pF->m_iROPMapper[ROP_DST_AND_PAT] = G2D_ROP_DST_AND_PAT;
	pF->m_iROPMapper[ROP_SRC_XOR_DST] = G2D_ROP_SRC_XOR_DST;
	pF->m_iROPMapper[ROP_SRC_XOR_PAT] = G2D_ROP_SRC_XOR_PAT;
	pF->m_iROPMapper[ROP_DST_XOR_PAT] = G2D_ROP_DST_XOR_PAT;
	pF->m_iROPMapper[ROP_NOTSRCCOPY] = G2D_ROP_NOTSRCCOPY;
	pF->m_iROPMapper[ROP_DSTINVERT] = G2D_ROP_DSTINVERT;
	pF->m_iROPMapper[ROP_R2_NOTCOPYPEN] = G2D_ROP_R2_NOTCOPYPEN;

	pF->m_iColorModeMapper[G2D_RGB16] = S3C_G2D_COLOR_RGB_565;
	pF->m_iColorModeMapper[G2D_RGBA16] = S3C_G2D_COLOR_RGBA_5551;
	pF->m_iColorModeMapper[G2D_ARGB16] = S3C_G2D_COLOR_ARGB_1555;
	pF->m_iColorModeMapper[G2D_RGBA32] = S3C_G2D_COLOR_RGBA_8888;
	pF->m_iColorModeMapper[G2D_ARGB32] = S3C_G2D_COLOR_ARGB_8888;
	pF->m_iColorModeMapper[G2D_XRGB32] = S3C_G2D_COLOR_XRGB_8888;
	pF->m_iColorModeMapper[G2D_RGBX32] = S3C_G2D_COLOR_RGBX_8888;

	pF->m_iAlphaModeMapper[ALPHAFX_NOALPHA] = G2D_NO_ALPHA_MODE;
	pF->m_iAlphaModeMapper[ALPHAFX_PERPIXELALPHA] = G2D_PP_ALPHA_SOURCE_MODE | G2D_ALPHA_MODE;
	pF->m_iAlphaModeMapper[ALPHAFX_CONSTALPHA] = G2D_ALPHA_MODE;
	pF->m_iAlphaModeMapper[ALPHAFX_FADE] = G2D_FADING_MODE;

	pF->m_pG2DReg = pG2DReg;

	//RequestEmptyFifo(pF->m_pG2DReg, 4);

	/// Font Operation Related
	/// pF->m_bIsBitBlt = true;
	/// pF->m_bIsScr2Scr = false;
	//DisableEffect(pF); // Disable per-pixel/per-plane alpha blending and fading
	//SetColorKeyOff(pF);

	/// pF->m_pG2DReg->ALPHA = (FADING_OFFSET_DISABLE | ALPHA_VALUE_DISABLE);
	/// pF->m_pG2DReg->ROP = (G2D_OPERAND3_FG_BIT | G2D_NO_ALPHA_MODE | OPAQUE_ENABLE | G2D_ROP_SRC_ONLY);
	//SetRotationOrg(pF->m_pG2DReg, 0, 0);
	/// pF->m_pG2DReg->ROT_MODE = ROT_0;
	/// pF->m_pG2DReg->ALPHA = 0;
}

#if 0
void PutPixel(PFIMGSE2D pF, u32 uPosX, u32 uPosY, u32 uColor) //modification
{
	SetRotationMode(ROT_0);    

	RequestEmptyFifo(4);

	m_pG2DReg->COORD0_X = uPosX;
	m_pG2DReg->COORD0_Y = uPosY;
	m_pG2DReg->FG_COLOR = uColor;

	DoCmd(&(m_pG2DReg->CMDR0), G2D_REND_POINT_BIT, G2D_BUSYWAITING);
}

/**
* Draw Line
* (usPosX1, usPosY1) ~ (usPosX2, usPosY2)
* Do not draw last point
*   0 < usPosX, usPosY1, usPosX2, usPosY2 < 2040
* X-INCR is calculated by (End_X - Start_X)/ ABS(End_Y - Start Y), when Y-axis is the major axis(Y length>X length)
* Y-INCR is calculated by (End_Y - Start_Y)/ ABS(End_X - Start X), When X-axis is the major axis
*/
#define INCR_INTEGER_PART_LENGTH    (11)
#define INCR_FRACTION_PART_LENGTH   (11)
void PutLine(PFIMGSE2D pF, u32 usPosX1, u32 usPosY1, u32 usPosX2, u32 usPosY2, u32 uColor, bool bIsDrawLastPoint) //modification
{
	int nMajorCoordX;
	u32 uHSz, uVSz;
	int i;
	int nIncr=0;
	u32 uCmdRegVal;

	SetRotationMode(ROT_0);        

	RequestEmptyFifo(7);

// 	RETAILMSG(DISP_ZONE_LINE,(TEXT("(%d,%d)~(%d,%d):Color:0x%x, LasT:%d\n"),
// 		usPosX1, usPosY1, usPosX2, usPosY2, uColor, bIsDrawLastPoint));

	m_pG2DReg->COORD0_X = usPosX1;
	m_pG2DReg->COORD0_Y = usPosY1;
	m_pG2DReg->COORD2_X = usPosX2;
	m_pG2DReg->COORD2_Y = usPosY2;

	/// Vertical Length
	uVSz = ABS((u16)usPosY1 - (u16)usPosY2);
	/// Horizontal Length
	uHSz = ABS((u16)usPosX1 - (u16)usPosX2);

	nMajorCoordX = (uHSz>=uVSz);

	if(nMajorCoordX)
	{
		// X length is longer than Y length
		// YINCR = (EY-SY)/ABS(EX-SX)
		for (i=0; i<INCR_FRACTION_PART_LENGTH+1; i++)
		{
			uVSz <<= 1;
			nIncr <<= 1;
			if (uVSz >= uHSz)
			{
				nIncr = nIncr | 1;
				uVSz -= uHSz;
			}
		}
		nIncr = (nIncr + 1) >> 1;
		if (usPosY1 > usPosY2)
		{
			nIncr = (~nIncr) + 1; // 2's complement
		}
// 		RETAILMSG(DISP_ZONE_LINE, (TEXT("YINCR: %x  "), nIncr ));
	}
	else
	{
		// Y length is longer than Y length    
		// XINCR = (EX-SX)/ABS(EY-SY)
		for (i=0; i<INCR_FRACTION_PART_LENGTH+1; i++)
		{
			uHSz <<= 1;
			nIncr <<= 1;
			if (uHSz >= uVSz)
			{
				nIncr = nIncr | 1;
				uHSz -= uVSz;
			}
		}
		nIncr = (nIncr + 1) >> 1;
		if (usPosX1 > usPosX2)
		{
			nIncr = (~nIncr) + 1; // 2's complement
		}
// 		RETAILMSG(DISP_ZONE_LINE, (TEXT("XINCR: %x  "), nIncr ));
	}

	m_pG2DReg->FG_COLOR = uColor;

	uCmdRegVal = 0;

	SetAlphaMode(ALPHAFX_NOALPHA);   //< Constant Alpha
	SetAlphaValue(0xff);


	if(nMajorCoordX)
	{
		SetYIncr(nIncr);

		uCmdRegVal =
			(bIsDrawLastPoint == true) ? (G2D_REND_LINE_BIT | G2D_MAJOR_COORD_X_BIT & G2D_DRAW_LAST_POINT_BIT) :
			(G2D_REND_LINE_BIT | G2D_MAJOR_COORD_X_BIT | G2D_NOT_DRAW_LAST_POINT_BIT);

// 		RETAILMSG(DISP_ZONE_LINE,(TEXT("m_pG2DReg:0x%x, CMD: %x, XINCR: %x, YINCR: %x\n"), m_pG2DReg, uCmdRegVal, m_pG2DReg->X_INCR, m_pG2DReg->Y_INCR ));
	}
	else
	{
		SetXIncr(nIncr);

		uCmdRegVal =
			(bIsDrawLastPoint == true) ? (G2D_REND_LINE_BIT | G2D_MAJOR_COORD_Y_BIT & G2D_DRAW_LAST_POINT_BIT) :
			(G2D_REND_LINE_BIT | G2D_MAJOR_COORD_Y_BIT | G2D_NOT_DRAW_LAST_POINT_BIT);

// 		RETAILMSG(DISP_ZONE_LINE,(TEXT("CMD: %x, XINCR: %x, YINCR: %x\n"), uCmdRegVal, m_pG2DReg->X_INCR, m_pG2DReg->Y_INCR ));
	}

	DoCmd(&(m_pG2DReg->CMDR0), uCmdRegVal, G2D_BUSYWAITING);
}
#endif

/**
*    @fn    DoCmd(u32 *CmdRegister, u32 CmdValue, u32 CmdType)
*    @note  Submit the 2D Processing Command
*    @note  3 processing return Style is supported 
*           Asynchornous Return -> Not Recommended for all GDI call, very small transfer or just with DDraw
*           Synchornous Wait for interrupt -> Recommended for most case, Big Transfer
*           Synchrnous polling -> Recommended for very short process time, ex) LineDrawing, under tick(1 millisecond)
*/
bool DoCmd(PFIMGSE2D pF, volatile u32 *CmdReg, u32 CmdValue, G2D_CMDPROCESSING_TYPE eCmdType)
{
	u32 bRetVal = 0;
	switch(eCmdType)
	{
	case G2D_FASTRETURN:
// 		DispPerfBeginWait();
		WaitForEmptyFifo(pF->m_pG2DReg);        //< This is check fully empty command fifo.
// 		DispPerfEndWait();

		*CmdReg = CmdValue;

		break;
	case G2D_INTERRUPT:
		RequestEmptyFifo(pF->m_pG2DReg, 1);
		IntPendingClear(pF->m_pG2DReg);
		IntEnable(pF->m_pG2DReg);    

		*CmdReg = CmdValue;

// 		DispPerfBeginWait();        
// 		bRetVal = WaitForSingleObject(m_hInterrupt2D, 10000L);    //< Set Timeout as 10seconds.
// 		DispPerfEndWait();        

//		if(bRetVal == WAIT_TIMEOUT)
//		{
// 			RETAILMSG(DISP_ZONE_ERROR, (TEXT("2D Command take too long time. This command cannot be processed properly\r\n")));
//		}

//		IntDisable(pF->m_pG2DReg);
//		IntPendingClear(pF->m_pG2DReg);

// 		InterruptDone(m_dwSysIntr2D);    
		break;
	case G2D_BUSYWAITING:
		RequestEmptyFifo(pF->m_pG2DReg, 1);
// 		IntDisable();    

		*CmdReg = CmdValue;

// 		DispPerfBeginWait();
		//        while(!WaitForFinish());      
		WaitForIdleStatus(pF->m_pG2DReg);                        // Polling Style    
// 		DispPerfEndWait();

		//        IntDisable();    

		break;
	default:
// 		RETAILMSG(DISP_ZONE_ERROR,(TEXT("CMDPROCESSING TYPE is invalid : %d\n"), eCmdType));
		return FALSE;
	}
	return TRUE;
}


/**
*    @fn    SetRopEtype(G2D_ROP_TYPE eRopType)
*    @note    Set Ternary Raster Operation
*    @note    Only support 7 raster operation (most used Rop)
*/
void SetRopEtype(PFIMGSE2D pF, G2D_ROP_TYPE eRopType)
{
	u32 uRopVal;

	uRopVal =
		(eRopType == ROP_SRC_ONLY) ? G2D_ROP_SRC_ONLY :
		(eRopType == ROP_DST_ONLY) ? G2D_ROP_DST_ONLY :
		(eRopType == ROP_PAT_ONLY) ? G2D_ROP_PAT_ONLY :
		(eRopType == ROP_SRC_AND_DST) ? G2D_ROP_SRC_AND_DST:
		(eRopType == ROP_SRC_AND_PAT) ? G2D_ROP_SRC_AND_PAT :
		(eRopType == ROP_DST_AND_PAT) ? G2D_ROP_DST_AND_PAT :
		(eRopType == ROP_SRC_OR_DST) ? G2D_ROP_SRC_OR_DST :
		(eRopType == ROP_SRC_OR_PAT) ? G2D_ROP_SRC_OR_PAT :
		(eRopType == ROP_DST_OR_PAT) ? G2D_ROP_DST_OR_PAT :
		(eRopType == ROP_SRC_XOR_DST) ? G2D_ROP_SRC_XOR_DST :
		(eRopType == ROP_SRC_XOR_PAT) ? G2D_ROP_SRC_XOR_PAT :
		(eRopType == ROP_DST_XOR_PAT) ? G2D_ROP_DST_XOR_PAT :
		G2D_ROP_SRC_ONLY;

	SetRopValue(pF->m_pG2DReg, uRopVal);

}

#if 0
void SetStencilKey(PFIMGSE2D pF, u32 uIsColorKeyOn, u32 uIsInverseOn, u32 uIsSwapOn)
{
	RequestEmptyFifo(1);
	m_pG2DReg->COLORKEY_CNTL = ((uIsColorKeyOn&1)<<31)|((uIsInverseOn&1)<<23)|(uIsSwapOn&1);
}

void SetStencilMinMax(PFIMGSE2D pF, u32 uRedMin, u32 uRedMax, u32 uGreenMin, u32 uGreenMax, u32 uBlueMin, u32 uBlueMax)
{
	RequestEmptyFifo(2);
	m_pG2DReg->COLORKEY_DR_MIN = ((uRedMin&0xff)<<16)|((uGreenMin&0xff)<<8)|(uBlueMin&0xff);
	m_pG2DReg->COLORKEY_DR_MAX = ((0xffU<<24)|(uRedMax&0xff)<<16)|((uGreenMax&0xff)<<8)|(uBlueMax&0xff);
}

void SetColorExpansionMethod(PFIMGSE2D pF, bool bIsScr2Scr)
{
	m_bIsScr2Scr  = bIsScr2Scr;
}

void BlendingOut(PFIMGSE2D pF, u32 uSrcData, u32 uDstData, u8 ucAlphaVal, bool bFading, u8 ucFadingOffset, u32 *uBlendingOut)
{

	u32 uSrcRed, uSrcGreen, uSrcBlue;
	u32 uDstRed, uDstGreen, uDstBlue;
	u32 uBldRed, uBldGreen, uBldBlue;

	uSrcRed= (uSrcData & 0x00ff0000)>>16;  // Mask R
	uSrcGreen = (uSrcData & 0x0000ff00)>>8;     // Mask G
	uSrcBlue = uSrcData & 0x000000ff;         // Mask B

	uDstRed = (uDstData & 0x00ff0000)>>16; // Mask R
	uDstGreen = (uDstData & 0x0000ff00)>>8;  // Mask G
	uDstBlue = uDstData & 0x000000ff;         // Mask B

	if(bFading) {
		uBldRed= ((uSrcRed*(ucAlphaVal+1))>>8) + ucFadingOffset; // R output
		uBldGreen= ((uSrcGreen*(ucAlphaVal+1))>>8) + ucFadingOffset; // G output
		uBldBlue= ((uSrcBlue*(ucAlphaVal+1)>>8)) + ucFadingOffset; // B output
		if(uBldRed>=256) uBldRed=255;
		if(uBldGreen>=256) uBldGreen=255;
		if(uBldBlue>=256) uBldBlue=255;
	}
	else {
		uBldRed= ((uSrcRed*(ucAlphaVal+1)) + (uDstRed*(256-ucAlphaVal)))>>8; // R output
		uBldGreen= ((uSrcGreen*(ucAlphaVal+1)) + (uDstGreen*(256-ucAlphaVal)))>>8; // G output
		uBldBlue= ((uSrcBlue*(ucAlphaVal+1)) + (uDstBlue*(256-ucAlphaVal)))>>8; // B output
	}

	*uBlendingOut = (uBldRed<<16) | (uBldGreen<<8) | uBldBlue;
}

void GetRotateCoordinate(PFIMGSE2D pF, u32 uDstX, u32 uDstY, u32 uOrgX, u32 uOrgY, u32 uRType, u32 *uRsltX, u32 *uRsltY)
{

	switch(uRType) {
		case  1 : // No Rotate. bypass.
			*uRsltX = uDstX;
			*uRsltY = uDstY;
			break;
		case  2 : // 90 degree Rotation
			*uRsltX = uOrgX + uOrgY - uDstY;
			*uRsltY = uDstX - uOrgX + uOrgY;
			break;
		case  4 : // 180 degree Rotation
			*uRsltX = 2*uOrgX - uDstX;
			*uRsltY = 2*uOrgY - uDstY;
			break;
		case  8 : // 270 degree Rotation
			*uRsltX = uDstY + uOrgX - uOrgY;
			*uRsltY = uOrgX + uOrgY - uDstX;
			break;
		case 16 : // X-flip
			*uRsltX = uDstX;
			*uRsltY = 2*uOrgY - uDstY;
			break;
		case 32 : // Y-flip
			*uRsltX = 2*uOrgX - uDstX;
			*uRsltY = uDstY;
			break;
		default :
// 			RETAILMSG(DISP_ZONE_ERROR, (TEXT("Invalid Rotation Type : %d"), uRType));
			break;
	}
}
#endif

void WaitForIdle(PFIMGSE2D pF)
{
	WaitForIdleStatus(pF->m_pG2DReg);
}

/*
void Convert24bpp(u32 uSrcData, EGPEFormat eBpp, bool bSwap, u32 *uConvertedData)
{

	u32 uRed, uGreen, uBlue;

	switch(eBpp) {
		case  gpe8Bpp: // 15 bit color mode(ARGB:1555)
			if(bSwap == 1) {  // pde_state == 2(BitBlt)
				uRed = uSrcData & 0x00007c00;  // R
				uGreen = uSrcData & 0x000003e0;  // G
				uBlue = uSrcData & 0x0000001f;  // B

				*uConvertedData = uRed<<9 | uGreen<<6 | uBlue<<3; // SUM
			}
			else { //hsel = 0
				uRed = uSrcData & 0x7c000000;
				uGreen = uSrcData & 0x03e00000;
				uBlue = uSrcData & 0x001f0000;

				*uConvertedData = uRed>>7 | uGreen>>10 | uBlue>>13;
			} 
			break;
		case gpe16Bpp : // 16 bit color mode
			if(bSwap == 1) {
				uRed = uSrcData & 0x0000f800;
				uGreen = uSrcData & 0x000007e0;
				uBlue = uSrcData & 0x0000001f;

				*uConvertedData = uRed<<8 | uGreen<<5 | uBlue<<3;
			}
			else {
				uRed = uSrcData & 0xf8000000;
				uGreen = uSrcData & 0x07e00000;
				uBlue = uSrcData & 0x001f0000;

				*uConvertedData = uRed>>8 | uGreen>>11 | uBlue>>13;
			}
			break;    
		case gpe32Bpp : // 24 bit color mode
			*uConvertedData = uSrcData;
			break;
	} // End of switch
} // End of g2d_cvt24bpp function


BOOL InitializeInterrupt(void)
{
	u32 dwIRQ;

	dwIRQ = IRQ_2D;                    // 2D Accelerator IRQ
	m_dwSysIntr2D = SYSINTR_UNDEFINED;
	m_hInterrupt2D = NULL;

	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIRQ, sizeof(u32), &m_dwSysIntr2D, sizeof(u32), NULL))
	{
		m_dwSysIntr2D = SYSINTR_UNDEFINED;
		return FALSE;
	}
	RETAILMSG(DISP_ZONE_INIT, (TEXT("2D Sysintr : %d\r\n"),m_dwSysIntr2D));

	m_hInterrupt2D = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(NULL == m_hInterrupt2D)
	{
		return FALSE;
	}

	if (!(InterruptInitialize(m_dwSysIntr2D, m_hInterrupt2D, 0, 0)))
	{
		return FALSE;
	}
	return TRUE;
}

void DeinitInterrupt(void)
{
	if (m_dwSysIntr2D != SYSINTR_UNDEFINED)
	{
		InterruptDisable(m_dwSysIntr2D);
	}

	if (m_hInterrupt2D != NULL)
	{
		CloseHandle(m_hInterrupt2D);
	}

	if (m_dwSysIntr2D != SYSINTR_UNDEFINED)
	{
		KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_dwSysIntr2D, sizeof(u32), NULL, 0, NULL);
	}
}
*/
