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
 * @name JPEG DRIVER MODULE Module (JPGMisc.c)
 * @author jiun.yu (jiun.yu@samsung.com)
 * @date 05-07-07
 */
#ifndef __JPG_MISC_H__
#define __JPG_MISC_H__

#include <linux/types.h>

typedef	unsigned char	UCHAR;
typedef unsigned long	ULONG;
typedef	unsigned int	UINT;
typedef struct mutex *	HANDLE;
typedef unsigned long	DWORD;
typedef unsigned int	UINT32;
typedef unsigned char	UINT8;
typedef enum {FALSE, TRUE} BOOL;

HANDLE CreateJPGmutex(void);
DWORD LockJPGMutex(void);
DWORD UnlockJPGMutex(void);
void DeleteJPGMutex(void);
unsigned int get_fb0_addr(void);
void get_lcd_size(int *width, int *height);
void WaitForInterrupt(void);
void WaitForDecInterrupt(void);

#endif
