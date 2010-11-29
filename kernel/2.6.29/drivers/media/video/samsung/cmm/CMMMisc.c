
#include <stdarg.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/wait.h>

#include <asm/io.h>
#include <plat/regs-lcd.h>
#include "CMMMisc.h"

static HANDLE hMutex	= NULL;

/*----------------------------------------------------------------------------
*Function: CreateJPGmutex
*Implementation Notes: Create Mutex handle 
-----------------------------------------------------------------------------*/
HANDLE CreateCMMmutex(void)
{
	hMutex = (HANDLE)kmalloc(sizeof(struct mutex), GFP_KERNEL);
	if (hMutex == NULL)
		return NULL;
	
	mutex_init(hMutex);
	
	return hMutex;
}

/*----------------------------------------------------------------------------
*Function: LockJPGMutex
*Implementation Notes: lock mutex 
-----------------------------------------------------------------------------*/
DWORD LockCMMMutex(void)
{
    mutex_lock(hMutex);  
    return 1;
}

/*----------------------------------------------------------------------------
*Function: UnlockJPGMutex
*Implementation Notes: unlock mutex
-----------------------------------------------------------------------------*/
DWORD UnlockCMMMutex(void)
{
	mutex_unlock(hMutex);
	
    return 1;
}

/*----------------------------------------------------------------------------
*Function: DeleteJPGMutex
*Implementation Notes: delete mutex handle 
-----------------------------------------------------------------------------*/
void DeleteCMMMutex(void)
{
	if (hMutex == NULL)
		return;

	mutex_destroy(hMutex);
}

