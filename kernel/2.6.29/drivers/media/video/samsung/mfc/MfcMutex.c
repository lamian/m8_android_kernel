/*
 * Project Name MFC DRIVER
 * Copyright (c) Samsung Electronics 
 * All right reserved.
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics  ("Confidential Information").   
 * you shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics 
 *
 * This file implements MUTEX to gurad MFC multi instance operation..
 *
 * @name MFC DRIVER MODULE Module (MfcMutex.c)
 * @author Shija P S (p.s.shija@samsung.com)
 */

#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/wait.h>

#include "MfcConfig.h"
#include "MfcMutex.h"
#include "MfcTypes.h"

extern wait_queue_head_t	WaitQueue_MFC;
static struct mutex	*hMutex = NULL;


BOOL MFC_Mutex_Create(void)
{
	hMutex = (struct mutex *)kmalloc(sizeof(struct mutex), GFP_KERNEL);
	if (hMutex == NULL)
		return FALSE;

	mutex_init(hMutex);

	return TRUE;
}

void MFC_Mutex_Delete(void)
{
	if (hMutex == NULL)
		return;

	mutex_destroy(hMutex);
}

BOOL MFC_Mutex_Lock(void)
{
	mutex_lock(hMutex);

	return TRUE;
}

BOOL MFC_Mutex_Release(void)
{
	mutex_unlock(hMutex);

	return TRUE;
}

