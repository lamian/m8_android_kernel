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
 * This source file is for controlling the Data buffer.
 *
 * @name MFC DRIVER MODULE Module (DataBuf.c)
 * @author name(email address)
 * @date 03-28-07
 */

#include "Mfc.h"
#include "MfcTypes.h"
#include "MfcMemory.h"
#include "LogMsg.h"
#include "DataBuf.h"
#include "MfcConfig.h"

static volatile unsigned char     *vir_pDATA_BUF      = NULL;

static unsigned int                phyDATA_BUF     = 0;


BOOL MfcDataBufMemMapping()
{
	BOOL	ret = FALSE;

	// STREAM BUFFER, FRAME BUFFER  <-- virtual data buffer address mapping
	vir_pDATA_BUF = (volatile unsigned char *)Phy2Vir_AddrMapping(S3C6400_BASEADDR_MFC_DATA_BUF, MFC_DATA_BUF_SIZE);
	if (vir_pDATA_BUF == NULL)
	{
		LOG_MSG(LOG_ERROR, "MfcDataBufMapping", "For DATA_BUF: VirtualAlloc failed!\r\n");
		return ret;
	}
	LOG_MSG(LOG_TRACE, "MfcDataBufMapping", "VIRTUAL ADDR DATA BUF : 0x%X\n", vir_pDATA_BUF);

	// Physical register address mapping
	phyDATA_BUF	= S3C6400_BASEADDR_MFC_DATA_BUF;


	ret = TRUE;

	return ret;
}

volatile unsigned char *GetDataBufVirAddr()
{
	volatile unsigned char	*pDataBuf;

	pDataBuf	= vir_pDATA_BUF;

	return pDataBuf;	
}

volatile unsigned char *GetFramBufVirAddr()
{
	volatile unsigned char	*pFramBuf;

	pFramBuf	= vir_pDATA_BUF + MFC_STRM_BUF_SIZE;

	return pFramBuf;	
}

/*
  * virtual address of DBK_BUF is returned
  *
  * 2009.5.12 by yj (yunji.kim@samsung.com)
  */
volatile unsigned char *GetDbkBufVirAddr()
{
	volatile unsigned char	*pDbkBuf;

	pDbkBuf	= vir_pDATA_BUF + MFC_STRM_BUF_SIZE + MFC_FRAM_BUF_SIZE;

	return pDbkBuf;	
}

unsigned int GetDataBufPhyAddr()
{
	unsigned int	phyDataBuf;

	phyDataBuf	= phyDATA_BUF;

	return phyDataBuf;
}

unsigned int GetFramBufPhyAddr()
{
	unsigned int	phyFramBuf;

	phyFramBuf	= phyDATA_BUF + MFC_STRM_BUF_SIZE;

	return phyFramBuf;
}

/*
  * physical address of DBK_BUF is returned
  *
  * 2009.5.12 by yj (yunji.kim@samsung.com)
  */
unsigned int GetDbkBufPhyAddr()
{
	unsigned int	phyDbkBuf;

	phyDbkBuf	= phyDATA_BUF + MFC_STRM_BUF_SIZE + MFC_FRAM_BUF_SIZE;

	return phyDbkBuf;	
}
