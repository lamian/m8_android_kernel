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
 * @name MFC DRIVER MODULE Module (MfcIntrNotification.c)
 * @author Simon Chun (simon.chun@samsung.com)
 */

#include "s3c-mfc.h"
#include "MfcIntrNotification.h"
#include "MfcSfr.h"


extern wait_queue_head_t	WaitQueue_MFC;
static unsigned int  		gIntrType = 0;

int SendInterruptNotification(int intr_type)
{
	gIntrType = intr_type;
	wake_up_interruptible(&WaitQueue_MFC);
	
	return 0;
}

int WaitInterruptNotification(void)
{
	if(interruptible_sleep_on_timeout(&WaitQueue_MFC, 500) == 0)
	{
		MfcStreamEnd();
		return WAIT_INT_NOTI_TIMEOUT; 
	}
	
	return gIntrType;
}
