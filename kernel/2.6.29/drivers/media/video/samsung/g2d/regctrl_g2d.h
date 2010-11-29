//
// Copyright (c) Samsung Electronics CO., LTD.  All rights reserved.
//
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:    regctrl_g2d.h

Abstract:       defines for FIMGSE-2D Graphics Accelerator Register Controller

Notes:
// Header to define the FIMGSE-2D Register Controller class

--*/

#ifndef __REGCTRL_G2D_H__
#define __REGCTRL_G2D_H__

#include "g2d_reg.h"

#define S3C6410_BASE_REG_PA_2DGRAPHICS	0x76100000

/**
*    For Hardware Specific Macro
**/
#define G2D_DE_STATUS_FA_BIT            (1<<9)

/// Each command will be issued to 2D engine.
/// But if 2D engine is not idle, command will be pushed to COMMAND FIFO,
/// and Related Parameter registers also will be pushed to COMMAND FIFO
/// For each command, required FIFO size can be variable(?)
#define    G2D_COMMANDFIFO_SIZE        (32)            //< 32-word size

#define G2D_OPERAND3_PAT_BIT            (0<<13)
#define G2D_OPERAND3_FG_BIT             (1<<13)

/**
*    @brief    Rotation Degree and flip setting for register setting. FLIP_X means that up-down inversion, FLIP_Y means that left-right inversion
*/
typedef enum
{
	ROT_0=(0x1<<0), ROT_90=(0x1<<1), ROT_180=(0x1<<2), ROT_270=(0x1<<3), FLIP_X=(0x1<<4), FLIP_Y=(0x1<<5)
} ROT_TYPE;

typedef enum
{
	G2D_OPERAND3_PAT,  G2D_OPERAND3_FG 
} G2D_OPERAND3;

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
}RECT, *PRECT;

/// For HW Control
void WaitForIdleStatus(volatile G2D_REG* pG2DReg);
bool WaitForFinish(volatile G2D_REG* pG2DReg);
void WaitForEmptyFifo(volatile G2D_REG* pG2DReg);

int CheckFifo(volatile G2D_REG* pG2DReg);
void IntEnable(volatile G2D_REG* pG2DReg);
void IntDisable(volatile G2D_REG* pG2DReg);
void IntPendingClear(volatile G2D_REG* pG2DReg);

void SetClipWindow(volatile G2D_REG* pG2DReg, PRECT prtClipWindow);
void Set3rdOperand(volatile G2D_REG* pG2DReg, G2D_OPERAND3 e3rdOp);

void Reset(volatile G2D_REG* pG2DReg);
void SetEndian(volatile G2D_REG* pG2DReg, bool bEndianSrc, bool bEndianDst);
void SetRopValue(volatile G2D_REG* pG2DReg, u32 uRopVal);

/// For Interrupt Handling
void IntEnableForDeEngineFinish(volatile G2D_REG* pG2DReg);
void IntEnableForCmdFinish(volatile G2D_REG* pG2DReg);
void IntEnableForOverflow(volatile G2D_REG* pG2DReg, bool bFifo, u8 ucFifoLevel);
void InterruptDisAll(volatile G2D_REG* pG2DReg);

int RequestEmptyFifo(volatile G2D_REG* pG2DReg, u32 uEmptyFifo);

void SetFirstBitBLTData(volatile G2D_REG* pG2DReg, u32 uFirstData);
void SetNextBitBLTData(volatile G2D_REG* pG2DReg, u32 uNextData);

/// For Rotation Setting
void SetRotationMode(volatile G2D_REG* pG2DReg, ROT_TYPE uRotationType);
void SetRotationOrg(volatile G2D_REG* pG2DReg, u16 usRotOrgX, u16 usRotOrgY);
void SetRotationOrgX(volatile G2D_REG* pG2DReg, u16 usRotOrgX);
void SetRotationOrgY(volatile G2D_REG* pG2DReg, u16 usRotOrgY);

/// For Stretching
void SetXIncr(volatile G2D_REG* pG2DReg, u32 uXIncr);
void SetYIncr(volatile G2D_REG* pG2DReg, u32 uYIncr);

/// For Transfer data region setting
void SetCoordinateSrcBlock(volatile G2D_REG* pG2DReg, u32 uStartX, u32 uStartY, u32 uEndX, u32 uEndY);
void SetCoordinateDstBlock(volatile G2D_REG* pG2DReg, u32 uStartX, u32 uStartY, u32 uEndX, u32 uEndY);

#endif //__REGCTRL_G2D_H__
