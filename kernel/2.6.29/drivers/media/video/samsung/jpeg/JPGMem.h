/*
 * Project Name JPEG DRIVER IN Linux
 * Copyright  2007 Samsung Electronics Co, Ltd. All Rights Reserved. 
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics  ("Confidential Information").   
 * you shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics 
 *
 * This file implements JPEG driver.
 *
 * @name JPEG DRIVER MODULE Module (JPGMem.h)
 * @author Jiun Yu (jiun.yu@samsung.com)
 * @date 04-07-07
 */

#ifndef __JPG_MEM_H__
#define __JPG_MEM_H__

#include "JPGMisc.h"

#include <linux/version.h>
#include <mach/hardware.h>
#include <plat/reserved_mem.h>


#define JPG_REG_BASE_ADDR          (0x78800000)

#define MAX_JPG_WIDTH              (4096)
#define MAX_JPG_HEIGHT             (4096)

#define MAX_JPG_THUMBNAIL_WIDTH	   (320)
#define MAX_JPG_THUMBNAIL_HEIGHT   (240)

//#define JPG_STREAM_BUF_SIZE			(MAX_JPG_WIDTH * MAX_JPG_HEIGHT)
//#define JPG_STREAM_THUMB_BUF_SIZE	(MAX_JPG_THUMBNAIL_WIDTH * MAX_JPG_THUMBNAIL_HEIGHT)
//#define JPG_FRAME_BUF_SIZE			(MAX_JPG_WIDTH * MAX_JPG_HEIGHT * 3)
//#define JPG_FRAME_THUMB_BUF_SIZE	(MAX_JPG_THUMBNAIL_WIDTH * MAX_JPG_THUMBNAIL_HEIGHT * 3)

//#define JPG_TOTAL_BUF_SIZE			(JPG_STREAM_BUF_SIZE + JPG_STREAM_THUMB_BUF_SIZE + JPG_FRAME_BUF_SIZE + JPG_FRAME_THUMB_BUF_SIZE)

#define COEF1_RGB_2_YUV          0x4d971e
#define COEF2_RGB_2_YUV          0x2c5783
#define COEF3_RGB_2_YUV          0x836e13

#define ENABLE_MOTION_ENC       (0x1<<3)
#define DISABLE_MOTION_ENC      (0x0<<3)

#define ENABLE_MOTION_DEC       (0x1<<0)
#define DISABLE_MOTION_DEC      (0x0<<0)

#define ENABLE_HW_DEC           (0x1<<2)
#define DISABLE_HW_DEC          (0x0<<2)

#define INCREMENTAL_DEC         (0x1<<3)
#define NORMAL_DEC              (0x0<<3)
#define YCBCR_MEMORY			(0x1<<5)

#define	ENABLE_IRQ				(0xf<<3)

typedef struct tagS3C6400_JPG_HOSTIF_REG
{
	UINT32		JPGMod;			//0x000
	UINT32		JPGStatus;		//0x004
	UINT32		JPGQTblNo;		//0x008
	UINT32		JPGRSTPos;		//0x00C
	UINT32		JPGY;			//0x010
	UINT32		JPGX;			//0x014
	UINT32		JPGDataSize;	//0x018
	UINT32		JPGIRQ;			//0x01C
	UINT32		JPGIRQStatus;	//0x020
	UINT32		dummy0[247];

	UINT32		JQTBL0[64];		//0x400
	UINT32		JQTBL1[64];		//0x500
	UINT32		JQTBL2[64];		//0x600
	UINT32		JQTBL3[64];		//0x700
	UINT32		JHDCTBL0[16];	//0x800
	UINT32		JHDCTBLG0[12];	//0x840
	UINT32		dummy1[4];
	UINT32		JHACTBL0[16];	//0x880
	UINT32		JHACTBLG0[162];	//0x8c0
	UINT32		dummy2[46];
	UINT32		JHDCTBL1[16];	//0xc00
	UINT32		JHDCTBLG1[12];	//0xc40
	UINT32		dummy3[4];
	UINT32		JHACTBL1[16];	//0xc80
	UINT32		JHACTBLG1[162];	//0xcc0
	UINT32		dummy4[46];

	UINT32		JPGYUVAddr0;	//0x1000
	UINT32		JPGYUVAddr1;	//0x1004
	UINT32		JPGFileAddr0;	//0x1008
	UINT32		JPGFileAddr1;	//0x100c
	UINT32		JPGStart;		//0x1010
	UINT32		JPGReStart;		//0x1014
	UINT32		JPGSoftReset;	//0x1018
	UINT32		JPGCntl;		//0x101c
	UINT32		JPGCOEF1;		//0x1020
	UINT32		JPGCOEF2;		//0x1024
	UINT32		JPGCOEF3;		//0x1028
	UINT32		JPGMISC;		//0x102c
	UINT32		JPGFrameIntv;	//0x1030
}S3C6400_JPG_HOSTIF_REG;

typedef struct tagS3C6400_JPG_CTX
{
	volatile S3C6400_JPG_HOSTIF_REG	*v_pJPG_REG;
	volatile UINT8					*v_pJPGData_Buff;
	volatile UINT8					*v_pYUVData_Buff;
	unsigned int 					p_pJPGData_Buff;
	unsigned int 					p_pYUVData_Buff;

	int								callerProcess;
	unsigned char					*strUserBuf;
	unsigned char					*frmUserBuf;
	unsigned char					*strUserThumbBuf;
	unsigned int					 p_strUserThumbBuf;
	unsigned char					*frmUserThumbBuf;
	unsigned int					 p_frmUserThumbBuf;
}S3C6400_JPG_CTX;

void *Phy2VirAddr(UINT32 phy_addr, int mem_size);
BOOL JPGMemMapping(S3C6400_JPG_CTX *base);
void JPGMemFree(S3C6400_JPG_CTX *base);
BOOL JPGBuffMapping(S3C6400_JPG_CTX *base);
void JPGBuffFree(S3C6400_JPG_CTX *base);
BOOL HWPostMemMapping(void);
void HWPostMemFree(void);
void *MemMove(void *dst, const void *src, unsigned int size);
void *MemAlloc(unsigned int size);
int JPEG_Copy_From_User(void *to, const void *from, unsigned long n);
int JPEG_Copy_To_User(void *to, const void *from, unsigned long n);

#endif

