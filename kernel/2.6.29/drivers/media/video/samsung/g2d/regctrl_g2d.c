//
// Copyright (c) Samsung Electronics CO., LTD.  All rights reserved.
//
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:	regctrl_g2d.cpp

Abstract:		implementation of register controller for S3C6410 FIMGSE-2D

Functions:

Notes:

--*/

#include "regctrl_g2d.h"

void Reset(volatile G2D_REG* pG2DReg)
{
	pG2DReg->CONTROL = 1;  //assert G2D reset, automatic clear
}

/**
*    @fn        void SetEndian(bool bEndian)
*    @brief    Set both Source and Destination data Endian
*    @param    bEndian    Endian setting 1: Big Endian, 0: Little Endian
*/
void SetEndian(volatile G2D_REG* pG2DReg, bool bEndianSrc, bool bEndianDst)
{
	RequestEmptyFifo(pG2DReg, 1);
	pG2DReg->ENDIAN = ( (bEndianDst<<1) | (bEndianSrc<<0) );
}

/**
*    Interrupt Block
*/
void IntEnable(volatile G2D_REG* pG2DReg)
{
	IntPendingClear(pG2DReg);
	pG2DReg->INTEN = (0x1<<10);// + (0x1<<8) + (0x1);                //Enable Interrupt
}

void IntDisable(volatile G2D_REG* pG2DReg)
{
	pG2DReg->INTEN &= ~((1<<10) /*+(1<<8) + 1*/);//Disable Interrupt
}

void IntPendingClear(volatile G2D_REG* pG2DReg)
{
	pG2DReg->INTC_PEND = 0x80000701;        // Level Interrupt (Interupt Clear Enable)    // 0x80000401;??
	pG2DReg->INTC_PEND = 0x80000000;        // Level Interrupt (Interupt Clear Enable)    
}

void IntEnableForDeEngineFinish(volatile G2D_REG* pG2DReg)
{
	pG2DReg->INTEN = (pG2DReg->INTEN)&~(0x7<<8) | (1<<10);
	pG2DReg->INTC_PEND = (0x80000000|(7<<8));
	pG2DReg->INTC_PEND = 0x80000000;
}
void IntEnableForCmdFinish(volatile G2D_REG* pG2DReg)
{
	pG2DReg->INTEN = ((pG2DReg->INTEN)&~(0x7<<8)) | (1<<9);
	pG2DReg->INTC_PEND = 0x80000000;
}
void IntEnableForOverflow(volatile G2D_REG* pG2DReg, bool bFifo, u8 ucFifoLevel)
{
	if(bFifo) {
		//        pG2DReg->INTEN = ((pG2DReg->INTEN)&~(0x7<<8))|(1<<8)|1;
		pG2DReg->INTEN = ((pG2DReg->INTEN)&~(0x7<<8))|1;
		pG2DReg->FIFO_INTC = ucFifoLevel;
	}    
	else
	{
		pG2DReg->INTEN = ((pG2DReg->INTEN)&~(0x7<<8))|(1<<8);
	}
	pG2DReg->INTC_PEND = 0x80000000;
}
void InterruptDisAll(volatile G2D_REG* pG2DReg)
{
	pG2DReg->INTEN = (pG2DReg->INTEN)&~((3<<8)|1);
}

void WaitForIdleStatus(volatile G2D_REG* pG2DReg)
{
	while(1)
	{
		if((pG2DReg->FIFO_STATUS & G2D_DE_STATUS_FA_BIT))
		{
			break;
		}
	}
}

void WaitForEmptyFifo(volatile G2D_REG* pG2DReg)
{
		RequestEmptyFifo(pG2DReg, G2D_COMMANDFIFO_SIZE);
}

bool WaitForFinish(volatile G2D_REG* pG2DReg)
{
	volatile unsigned uPendVal;

	uPendVal = pG2DReg->INTC_PEND;

	if( (uPendVal>>8) & 0x7){
		switch( (uPendVal>>8) & 0x7) {
			case 1:
				pG2DReg->INTC_PEND = ((1<<31)|(1<<8));        // Overflow
				break;
			case 2:
				pG2DReg->INTC_PEND = ((1<<31)|(1<<9));        // Command All Finish, Engine IDLE
				break;
			case 4:
				pG2DReg->INTC_PEND = ((1<<31)|(1<<10));    // Drawing Engine Finish
				return false;
			default:
				pG2DReg->INTC_PEND = ((1<<31)|(0x7<<8));    // All Clear
				break;
		}
		pG2DReg->INTC_PEND = (u32)(1<<31); // to clear pending.        


		return true;
	}
	return false;
}

/**
*    Command Block
*/

/**
*    @fn    RequestEmptyFifo(u32 uEmptyFifo)
*    @param    uEmptyFifo    Requested Empty FIFO size
*    @return    int        Return Available FIFO Size
*    @note This function will do busy-waiting until requested fifo is empty.
*/
int RequestEmptyFifo(volatile G2D_REG* pG2DReg, u32 uEmptyFifo)
{
	if(uEmptyFifo > G2D_COMMANDFIFO_SIZE)
	{
		uEmptyFifo = G2D_COMMANDFIFO_SIZE;
	}
	while( (u32)CheckFifo(pG2DReg) > (G2D_COMMANDFIFO_SIZE - uEmptyFifo) ); 

	return (G2D_COMMANDFIFO_SIZE - CheckFifo(pG2DReg));

}

/**
*    @fn    CheckFifo()
*    @return    int        Return Used FIFO Size
*/
int CheckFifo(volatile G2D_REG* pG2DReg)
{
	return (((pG2DReg->FIFO_STATUS)& (0x3f<<1))>>1);
}


/*
*     @fn    void SetFirstBitBLTData(u32 uFirstData)
*    @brief    Set First Source Data to FIMG2D HW Register using CMD register 2, Next Data must be issued by CMD Register 3 continously
*    @param    uFirstData    32bit surface Surface data
*/
void SetFirstBitBLTData(volatile G2D_REG* pG2DReg, u32 uFirstData)
{
	RequestEmptyFifo(pG2DReg, 1);
	pG2DReg->CMDR2 = uFirstData;
}

/*
*     @fn    void SetNextBitBLTData(u32 uFirstData)
*    @brief    Set Next Source Data to FIMG2D HW Register using CMD register 2, Next Data must be issued by CMD Register 3
*    @param    uNextData    32bit surface Surface data
*/
void SetNextBitBLTData(volatile G2D_REG* pG2DReg, u32 uNextData)
{
	RequestEmptyFifo(pG2DReg, 1);
	pG2DReg->CMDR3 = uNextData;
}

/**
*    @fn    void SetRotationMode(u32 uRotationType)
*    @param    uRotationType    This is register value for each rotation. It can handle 0, 90, 180, 270, Xflip, Yflip
*    @note G2D's Rotation Register has only 1 bit as enabling.
*/
void SetRotationMode(volatile G2D_REG* pG2DReg, ROT_TYPE uRotationType)
{
	RequestEmptyFifo(pG2DReg, 1);
	pG2DReg->ROT_MODE = ((pG2DReg->ROT_MODE) & ~0x3f)|(uRotationType);    
}

/**
*    @fn    void SetRotationOrg(u16 usRotOrg, u16 usRotOrgY)
*    @param    usRotOrgX    X value of Rotation Origin.
*    @param    usRotOrgY    Y value of Rotation Origin.
*    @sa    SetRotationOrgX()
*    @sa SetRotationOrgY()
*    @brief    this function sets rotation origin.
*/
void SetRotationOrg(volatile G2D_REG* pG2DReg, u16 usRotOrgX, u16 usRotOrgY)
{
	SetRotationOrgX(pG2DReg, usRotOrgX);
	SetRotationOrgY(pG2DReg, usRotOrgY);
}

void SetRotationOrgX(volatile G2D_REG* pG2DReg, u16 usRotOrgX)
{
	RequestEmptyFifo(pG2DReg, 1);
	pG2DReg->ROT_OC_X = (u32) (usRotOrgX & 0x000007FF);
}

void SetRotationOrgY(volatile G2D_REG* pG2DReg, u16 usRotOrgY)
{
	RequestEmptyFifo(pG2DReg, 1);
	pG2DReg->ROT_OC_Y = (u32) (usRotOrgY & 0x000007FF);
}

/**
*    @fn        void SetCoordinateSrcBlockEndian(u32 uStartX, u32 uStartY, u32 uEndX, u32 uEndY)
*    @brief    Set Source Data Area that will be read
*    @param    uStartX    left X 
*            uStartY    top Y
*            uEndX    right X
*            uEndY    bottom Y
*/
void SetCoordinateSrcBlock(volatile G2D_REG* pG2DReg, u32 uStartX, u32 uStartY, u32 uEndX, u32 uEndY)
{
	RequestEmptyFifo(pG2DReg, 4);
	pG2DReg->COORD0_X = uStartX;
	pG2DReg->COORD0_Y = uStartY;
	pG2DReg->COORD1_X = uEndX;
	pG2DReg->COORD1_Y = uEndY;    
}

/**
*    @fn        void SetCoordinateDstBlockEndian(u32 uStartX, u32 uStartY, u32 uEndX, u32 uEndY)
*    @brief    Set Destination Data Area that will be written
*    @param    uStartX    left X 
*            uStartY    top Y
*            uEndX    right X
*            uEndY    bottom Y
*/
void SetCoordinateDstBlock(volatile G2D_REG* pG2DReg, u32 uStartX, u32 uStartY, u32 uEndX, u32 uEndY)
{
	RequestEmptyFifo(pG2DReg, 4);
	pG2DReg->COORD2_X = uStartX;
	pG2DReg->COORD2_Y = uStartY;
	pG2DReg->COORD3_X = uEndX;
	pG2DReg->COORD3_Y = uEndY;    
}

/**
*    @fn        void SetRoptype(u32 uRopVal)
*    @note    Set Ternary Raster Operation into G2D register diRECTy
*/
void SetRopValue(volatile G2D_REG* pG2DReg, u32 uRopVal)
{
	RequestEmptyFifo(pG2DReg, 1);
	pG2DReg->ROP = ((pG2DReg->ROP)&(~0xff)) | uRopVal;
}

/**
*    @fn        void Set3rdOperand(G2D_OPERAND3 e3rdOp)
*    @brief    Set thrid operand as Pattern or Foreground color
*    @param    e3rdOp can be pattern or foreground color
*/
void Set3rdOperand(volatile G2D_REG* pG2DReg, G2D_OPERAND3 e3rdOp)
{
	RequestEmptyFifo(pG2DReg, 1);
	u32 u3rdOpSel =
		(e3rdOp == G2D_OPERAND3_PAT) ? G2D_OPERAND3_PAT_BIT :
		(e3rdOp == G2D_OPERAND3_FG) ? G2D_OPERAND3_FG_BIT :     0xffffffff;

	pG2DReg->ROP = ((pG2DReg->ROP) & ~(0x1<<13)) | u3rdOpSel;
}

/**
*    @fn        void SetClipWindow(PRECT prtClipWindow)
*    @brief    Set Clipping Data Area that will be not cropped.
*    @param    uStartX    left X 
*            uStartY    top Y
*            uEndX    right X
*            uEndY    bottom Y
*/
void SetClipWindow(volatile G2D_REG* pG2DReg, PRECT prtClipWindow)
{
	RequestEmptyFifo(pG2DReg, 4);
	pG2DReg->CW_LEFT_TOP_X = prtClipWindow->left;
	pG2DReg->CW_LEFT_TOP_Y =  prtClipWindow->top;
	pG2DReg->CW_RIGHT_BOTTOM_X = prtClipWindow->right;
	pG2DReg->CW_RIGHT_BOTTOM_Y = prtClipWindow->bottom;
}

void SetXIncr(volatile G2D_REG* pG2DReg, u32 uXIncr)
{
		pG2DReg->X_INCR = uXIncr;
}

void SetYIncr(volatile G2D_REG* pG2DReg, u32 uYIncr)
{
		pG2DReg->Y_INCR = uYIncr;
}
