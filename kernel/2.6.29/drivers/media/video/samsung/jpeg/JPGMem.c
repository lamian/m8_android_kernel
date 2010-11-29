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
 * @name JPEG DRIVER MODULE Module (JPGMem.c)
 * @author Jiun Yu (jiun.yu@samsung.com)
 * @date 04-07-07
 */
#include <asm/io.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/types.h>

#include "JPGMem.h"
#include "JPGMisc.h"
#include "LogMsg.h"


/*----------------------------------------------------------------------------
*Function: Phy2VirAddr

*Parameters: 		dwContext		:
*Return Value:		True/False
*Implementation Notes: memory mapping from physical addr to virtual addr 
-----------------------------------------------------------------------------*/
void *Phy2VirAddr(UINT32 phy_addr, int mem_size)
{
	void	*reserved_mem;

	reserved_mem = (void *)ioremap( (unsigned long)phy_addr, (int)mem_size );		

	if (reserved_mem == NULL) {
		JPEG_LOG_MSG(LOG_ERROR, "Phy2VirAddr", "DD::Phyical to virtual memory mapping was failed!\r\n");
		return NULL;
	}

	return reserved_mem;
}

/*----------------------------------------------------------------------------
*Function: JPGMemMapping

*Parameters: 		dwContext		:
*Return Value:		True/False
*Implementation Notes: JPG register mapping from physical addr to virtual addr 
-----------------------------------------------------------------------------*/
BOOL JPGMemMapping(S3C6400_JPG_CTX *base)
{
	// JPG HOST Register
	base->v_pJPG_REG = (volatile S3C6400_JPG_HOSTIF_REG *)Phy2VirAddr(JPG_REG_BASE_ADDR, sizeof(S3C6400_JPG_HOSTIF_REG));
	if (base->v_pJPG_REG == NULL)
	{
		JPEG_LOG_MSG(LOG_ERROR, "JPGMemMapping", "DD::v_pJPG_REG: VirtualAlloc failed!\r\n");
		return FALSE;
	}
	
	return TRUE;
}


void JPGMemFree(S3C6400_JPG_CTX *base)
{
	iounmap((void *)base->v_pJPG_REG);
	base->v_pJPG_REG = NULL;
}

/*----------------------------------------------------------------------------
*Function: JPGBuffMapping

*Parameters: 		dwContext		:
*Return Value:		True/False
*Implementation Notes: JPG Buffer mapping from physical addr to virtual addr 
-----------------------------------------------------------------------------*/
/*
BOOL JPGBuffMapping(S3C6400_JPG_CTX *base)
{
	// JPG Data Buffer
	base->v_pJPGData_Buff = (UINT8 *)Phy2VirAddr(JPG_DATA_BASE_ADDR, JPG_TOTAL_BUF_SIZE);

	if (base->v_pJPGData_Buff == NULL)
	{
		JPEG_LOG_MSG(LOG_ERROR, "JPGBuffMapping", "DD::v_pJPGData_Buff: VirtualAlloc failed!\r\n");
		return FALSE;
	}

	return TRUE;
}

void JPGBuffFree(S3C6400_JPG_CTX *base)
{
	iounmap( (void *)base->v_pJPGData_Buff );
	base->v_pJPGData_Buff = NULL;
}
*/

void *MemMove(void *dst, const void *src, unsigned int size)
{
	return memmove(dst, src, size);
}

void *MemAlloc(unsigned int size)
{
	void	*alloc_mem;

	alloc_mem = (void *)kmalloc((int)size, GFP_KERNEL);
	if (alloc_mem == NULL) {
		JPEG_LOG_MSG(LOG_ERROR, "Mem_Alloc", "memory allocation failed!\r\n");
		return NULL;
	}

	return alloc_mem;
}

int JPEG_Copy_From_User(void *to, const void *from, unsigned long n)
{
	return copy_from_user(to, from, n);
}

int JPEG_Copy_To_User(void *to, const void *from, unsigned long n)
{
	return copy_to_user(to, from, n);
}

