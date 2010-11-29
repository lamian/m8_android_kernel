/*
 * Project Name MFC DRIVER 
 * Copyright  2007 Samsung Electronics Co, Ltd. All Rights Reserved. 
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics  ("Confidential Information").   
 * you shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics 
 *
 * This source file is for setting physical memory.
 *
 * @name MFC DRIVER MODULE Module (MfcMemory.c)
 * @author Jiun, Yu(jiun.yu@samsung.com)
 * @date 03-28-07
 */

#include <asm/io.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm/uaccess.h>

#include <linux/version.h>
#include <plat/regs-lcd.h>

#include "MfcConfig.h"
#include "LogMsg.h"


void *Phy2Vir_AddrMapping(unsigned int phy_addr, int mem_size)
{
	void	*reserved_mem;

        // from 2.8.5
	if (phy_addr == S3C6400_BASEADDR_MFC_DATA_BUF) {
		//reserved_mem = (void *)ioremap_cached( (unsigned long)phy_addr, (int)mem_size );
		reserved_mem = (void *)ioremap( (unsigned long)phy_addr, (int)mem_size );
	} else {
		//reserved_mem = (void *)ioremap_nocache( (unsigned long)phy_addr, (int)mem_size );
		reserved_mem = (void *)ioremap( (unsigned long)phy_addr, (int)mem_size );
	}
	
	if (reserved_mem == NULL) {
		LOG_MSG(LOG_ERROR, "Phy2Vir_AddrMapping", "For IOPreg: VirtualAlloc failed!\r\n");
		return NULL;
	}

	
	return reserved_mem;
}


void *Mem_Alloc(unsigned int size)
{
	void	*alloc_mem;

	alloc_mem = (void *)kmalloc(size, GFP_KERNEL);
	if (alloc_mem == NULL) {
		LOG_MSG(LOG_ERROR, "Mem_Alloc", "memory allocation failed!\r\n");
		return NULL;
	}

	return alloc_mem;
}

void Mem_Free(void *addr)
{
	kfree(addr);
}

void *Mem_Cpy(void *dst, const void *src, int size)
{
	return memcpy(dst, src, size);
}

void *Mem_Set(void *target, int val, int size)
{
	return memset(target, val, size);
}

int Copy_From_User(void *to, const void *from, unsigned long n)
{
	return copy_from_user(to, from, n);
}

int Copy_To_User(void *to, const void *from, unsigned long n)
{
	return copy_to_user(to, from, n);
}

