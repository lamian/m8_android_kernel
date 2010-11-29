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
 * @name JPEG DRIVER MODULE Module (JPGOpr.c)
 * @author Jiun Yu (jiun.yu@samsung.com)
 * @date 28-07-07
 */

#include <asm/memory.h>
#include <mach/memory.h>
#include "JPGMem.h"
#include "JPGMisc.h"
#include "JPGOpr.h"
#include "JPGConf.h"
#include "LogMsg.h"

extern int	jpg_irq_reason;

enum{
		UNKNOWN,
		BASELINE = 0xC0,
		EXTENDED_SEQ = 0xC1,
		PROGRESSIVE = 0xC2
}JPG_SOF_MARKER;


/*----------------------------------------------------------------------------
*Function: decodeJPG

*Parameters:	jCTX:
				input_buff:
				input_size:
				output_buff:
				output_size
*Return Value:
*Implementation Notes: 
-----------------------------------------------------------------------------*/
JPG_RETURN_STATUS decodeJPG(S3C6400_JPG_CTX *jCTX,
							JPG_DEC_PROC_PARAM *decParam)
{
	volatile int		ret;
	SAMPLE_MODE_T sampleMode;
	UINT32	width, height, orgwidth, orgheight;
	BOOL	headerFixed = FALSE;

	JPEG_LOG_MSG(LOG_TRACE, "decodeJPG", "decodeJPG function\n");
	resetJPG(jCTX);

	/////////////////////////////////////////////////////////
	// Header Parsing									   //
	/////////////////////////////////////////////////////////

	decodeHeader(jCTX);
	WaitForInterrupt();
	ret = jpg_irq_reason;
		
	if(ret != OK_HD_PARSING){
		JPEG_LOG_MSG(LOG_ERROR, "\ndecodeJPG", "DD::JPG Header Parsing Error(%d)\r\n", ret);
		return JPG_FAIL;
	}

	sampleMode = getSampleType(jCTX);
	JPEG_LOG_MSG(LOG_TRACE, "decodeJPG", "sampleMode : %d\n", sampleMode);
	if(sampleMode == JPG_SAMPLE_UNKNOWN){
		JPEG_LOG_MSG(LOG_ERROR, "decodeJPG", "DD::JPG has invalid sampleMode\r\n");
		return JPG_FAIL;
	}
	decParam->sampleMode = sampleMode;

	getXY(jCTX, &width, &height);
	JPEG_LOG_MSG(LOG_TRACE, "decodeJPG", "DD:: width : 0x%x height : 0x%x\n", width, height);
	if(width <= 0 || width > MAX_JPG_WIDTH || height <= 0 || height > MAX_JPG_HEIGHT){
		JPEG_LOG_MSG(LOG_ERROR, "decodeJPG", "DD::JPG has invalid width/height\n");
		return JPG_FAIL;
	}

	/////////////////////////////////////////////////////////
	// Header Correction								   //
	/////////////////////////////////////////////////////////

	orgwidth = width;
	orgheight = height;
	if(!isCorrectHeader(sampleMode, &width, &height)){
		rewriteHeader(jCTX, decParam->fileSize, width, height);
		headerFixed = TRUE;
	}
	

	/////////////////////////////////////////////////////////
	// Body Decoding									   //
	/////////////////////////////////////////////////////////

	if(headerFixed){
		resetJPG(jCTX);
		decodeHeader(jCTX);
		WaitForInterrupt();
		ret = jpg_irq_reason;
				
		if(ret != OK_HD_PARSING){
			JPEG_LOG_MSG(LOG_ERROR, "decodeJPG", "JPG Header Parsing Error(%d)\r\n", ret);
			return JPG_FAIL;
		}
		
		decodeBody(jCTX);
		WaitForInterrupt();
		ret = jpg_irq_reason;
			
		if(ret != OK_ENC_OR_DEC){
			JPEG_LOG_MSG(LOG_ERROR, "decodeJPG", "JPG Body Decoding Error(%d)\n", ret);
			return JPG_FAIL;
		}

		// for post processor, discard pixel
		if(orgwidth % 4 != 0)  
			orgwidth = (orgwidth/4)*4;

		JPEG_LOG_MSG(LOG_TRACE, "decodeJPG", "orgwidth : %d orgheight : %d\n", orgwidth, orgheight);
		rewriteYUV(jCTX, width, orgwidth, height, orgheight);

		// JPEG H/W IP always return YUV422
		decParam->dataSize = getYUVSize(JPG_422, orgwidth, orgheight);
		decParam->width = orgwidth;
		decParam->height = orgheight;
	}
	else{
		decodeBody(jCTX);
		WaitForInterrupt();
		ret = jpg_irq_reason;
		
		if(ret != OK_ENC_OR_DEC){
			JPEG_LOG_MSG(LOG_ERROR, "decodeJPG", "DD::JPG Body Decoding Error(%d)\n", ret);
			return JPG_FAIL;
		}

		// JPEG H/W IP always return YUV422
		decParam->dataSize = getYUVSize(JPG_422, width, height);
		decParam->width = width;
		decParam->height = height;
	}
	
	return JPG_SUCCESS;	
}

/*----------------------------------------------------------------------------
*Function: isCorrectHeader

*Parameters:	sampleMode:
				width:
				height:
*Return Value:
*Implementation Notes: 
-----------------------------------------------------------------------------*/
BOOL isCorrectHeader(SAMPLE_MODE_T sampleMode, UINT32 *width, UINT32 *height)
{
	BOOL result = FALSE;

	JPEG_LOG_MSG(LOG_TRACE, "isCorrectHeader", "Header is not multiple of MCU\n");

	switch(sampleMode){
		case JPG_400 : 
		case JPG_444 : if((*width % 8 == 0) && (*height % 8 == 0))
						   result = TRUE;
					   if(*width % 8 != 0)
						   *width += 8 - (*width % 8);
					   if(*height % 8 != 0)
						   *height += 8 - (*height % 8);						
						break;
		case JPG_422 : if((*width % 16 == 0) && (*height % 8 == 0))
						   result = TRUE;
					   if(*width % 16 != 0)
						   *width += 16 - (*width % 16);
					   if(*height % 8 != 0)
						   *height += 8 - (*height % 8);						
						break; 
		case JPG_420 : 
		case JPG_411 : if((*width % 16 == 0) && (*height % 16 == 0))
						   result = TRUE;
					   if(*width % 16 != 0)
						   *width += 16 - (*width % 16);
					   if(*height % 16 != 0)
						   *height += 16 - (*height % 16);						
						break;
		default : break;
	}

	JPEG_LOG_MSG(LOG_TRACE, "isCorrectHeader", "after error correction : width(%x) height(%x)\n", *width, *height);
	return(result);
}

/*----------------------------------------------------------------------------
*Function: rewriteHeader

*Parameters:	jCTX:
				file_size:
				width:
				height:
*Return Value:
*Implementation Notes: 
-----------------------------------------------------------------------------*/
void rewriteHeader(S3C6400_JPG_CTX *jCTX, UINT32 file_size, UINT32 width, UINT32 height)
{
	UINT32	i;
	UINT8	*ptr = (UINT8 *)jCTX->v_pJPGData_Buff;
	UINT8	*SOF1 = NULL, *SOF2 = NULL;
	UINT8	*header = NULL;

	JPEG_LOG_MSG(LOG_TRACE, "rewriteHeader", "file size : %d, v_pJPGData_Buff : 0x%X\n", file_size, ptr);
	
	for(i=0; i < file_size; i++){
		if(*ptr++ == 0xFF){
			if((*ptr == BASELINE) || (*ptr == EXTENDED_SEQ) || (*ptr == PROGRESSIVE)){
				JPEG_LOG_MSG(LOG_TRACE, "rewriteHeader", "match FFC0(i : %d)\n", i);
				if(SOF1 == NULL)
					SOF1 = ++ptr;
				else{
					SOF2 = ++ptr;
					break;
				}
			}
		}
	}

	JPEG_LOG_MSG(LOG_TRACE, "rewriteHeader", "start header correction\n");
	if(i <= file_size){
		header = (SOF2 == NULL) ? (SOF1) : (SOF2);
		header += 3; //length(2) + sampling bit(1)
		*header = (height>>8) & 0xFF;
		header++;
		*header = height & 0xFF;
		header++;
		*header = (width>>8) & 0xFF;
		header++;
		*header = (width & 0xFF);

	}
}

/*----------------------------------------------------------------------------
*Function: resetJPG

*Parameters:	jCTX:
*Return Value:
*Implementation Notes: 
-----------------------------------------------------------------------------*/
void resetJPG(S3C6400_JPG_CTX *jCTX)
{
	JPEG_LOG_MSG(LOG_TRACE, "resetJPG", "resetJPG function\n");
	jCTX->v_pJPG_REG->JPGSoftReset = 0; //ENABLE
}

/*----------------------------------------------------------------------------
*Function: decodeHeader

*Parameters:	jCTX:	
*Return Value:
*Implementation Notes: 
-----------------------------------------------------------------------------*/
void decodeHeader(S3C6400_JPG_CTX *jCTX)
{
	JPEG_LOG_MSG(LOG_TRACE, "decodeHeader", "decodeHeader function\n");
	jCTX->v_pJPG_REG->JPGFileAddr0 = jCTX->p_pJPGData_Buff;
	jCTX->v_pJPG_REG->JPGFileAddr1 = jCTX->p_pJPGData_Buff;
	
//	jCTX->v_pJPG_REG->JPGFileAddr0 = JPG_DATA_BASE_ADDR;
//	jCTX->v_pJPG_REG->JPGFileAddr1 = JPG_DATA_BASE_ADDR;

	jCTX->v_pJPG_REG->JPGMod = 0x08; //decoding mode
	jCTX->v_pJPG_REG->JPGIRQ = ENABLE_IRQ;
	jCTX->v_pJPG_REG->JPGCntl = DISABLE_HW_DEC;
	jCTX->v_pJPG_REG->JPGMISC = (NORMAL_DEC | YCBCR_MEMORY);
	jCTX->v_pJPG_REG->JPGStart = 1;
}

/*----------------------------------------------------------------------------
*Function: decodeBody

*Parameters:	jCTX:
*Return Value:
*Implementation Notes: 
-----------------------------------------------------------------------------*/
void decodeBody(S3C6400_JPG_CTX *jCTX)
{
	JPEG_LOG_MSG(LOG_TRACE, "decodeBody", "decodeBody function\n");
	jCTX->v_pJPG_REG->JPGYUVAddr0 = jCTX->p_pYUVData_Buff;
	jCTX->v_pJPG_REG->JPGYUVAddr1 = jCTX->p_pYUVData_Buff;
//	jCTX->v_pJPG_REG->JPGYUVAddr0 = JPG_DATA_BASE_ADDR + JPG_STREAM_BUF_SIZE + JPG_STREAM_THUMB_BUF_SIZE;
//	jCTX->v_pJPG_REG->JPGYUVAddr1 = JPG_DATA_BASE_ADDR + JPG_STREAM_BUF_SIZE + JPG_STREAM_THUMB_BUF_SIZE;

	jCTX->v_pJPG_REG->JPGCntl = 0;
	jCTX->v_pJPG_REG->JPGMISC = 0;
	jCTX->v_pJPG_REG->JPGReStart = 1;
}

/*----------------------------------------------------------------------------
*Function: rewriteYUV

*Parameters:	jCTX:
				width:
				orgwidth:
				height:
				orgheight:
*Return Value:
*Implementation Notes: 
-----------------------------------------------------------------------------*/
void rewriteYUV(S3C6400_JPG_CTX *jCTX, UINT32 width, UINT32 orgwidth, UINT32 height, UINT32 orgheight)
{
	UINT32	src, dst;
	UINT32	i;
	UINT8	*streamPtr;

	JPEG_LOG_MSG(LOG_TRACE, "rewriteYUV", "rewriteYUV function\n");

//	streamPtr = (UINT8 *)(jCTX->v_pJPGData_Buff + JPG_STREAM_BUF_SIZE + JPG_STREAM_THUMB_BUF_SIZE);
	streamPtr = (UINT8 *)(jCTX->v_pYUVData_Buff);
	src = 2*width;
	dst = 2*orgwidth;
	for(i = 1; i < orgheight; i++){
		MemMove(&streamPtr[dst], &streamPtr[src], 2*orgwidth);
		src += 2*width;
		dst += 2*orgwidth;
	}
}

/*----------------------------------------------------------------------------
*Function:	getSampleType

*Parameters:	jCTX:
*Return Value:
*Implementation Notes: 
-----------------------------------------------------------------------------*/
SAMPLE_MODE_T getSampleType(S3C6400_JPG_CTX *jCTX)
{	
	ULONG	jpgMode;
	SAMPLE_MODE_T	sampleMode;	

	jpgMode = jCTX->v_pJPG_REG->JPGMod;

	sampleMode = 
		((jpgMode&0x7) == 0) ? JPG_444 :
		((jpgMode&0x7) == 1) ? JPG_422 :
		((jpgMode&0x7) == 2) ? JPG_420 :
		((jpgMode&0x7) == 3) ? JPG_400 :
		((jpgMode&0x7) == 6) ? JPG_411 : JPG_SAMPLE_UNKNOWN;

	return(sampleMode);
}

/*----------------------------------------------------------------------------
*Function:	getXY

*Parameters:	jCTX:
				x:
				y:
*Return Value:
*Implementation Notes: 
-----------------------------------------------------------------------------*/
void getXY(S3C6400_JPG_CTX *jCTX, UINT32 *x, UINT32 *y)
{
	*x = jCTX->v_pJPG_REG->JPGX;
	*y = jCTX->v_pJPG_REG->JPGY;
}

/*----------------------------------------------------------------------------
*Function: getYUVSize

*Parameters:	sampleMode:
				width:
				height:
*Return Value:
*Implementation Notes: 
-----------------------------------------------------------------------------*/
UINT32 getYUVSize(SAMPLE_MODE_T sampleMode, UINT32 width, UINT32 height)
{
	switch(sampleMode){
		case JPG_444 : return(width*height*3);
		case JPG_422 : return(width*height*2);
		case JPG_420 : 
		case JPG_411 : return((width*height*3)>>1);
		case JPG_400 : return(width*height);
		default : return(0);
	}

}

/*----------------------------------------------------------------------------
*Function: resetJPG

*Parameters:	jCTX:
*Return Value:
*Implementation Notes: 
-----------------------------------------------------------------------------*/
JPG_RETURN_STATUS encodeJPG(S3C6400_JPG_CTX *jCTX, 
							JPG_ENC_PROC_PARAM	*EncParam)
{
	UINT	i, ret;

	if(EncParam->width <= 0 || EncParam->width > MAX_JPG_WIDTH
		|| EncParam->height <=0 || EncParam->height > MAX_JPG_HEIGHT){
			JPEG_LOG_MSG(LOG_ERROR, "encodeJPG", "DD::Encoder : Invalid width/height\r\n");
			return JPG_FAIL;
	}

	resetJPG(jCTX);

	jCTX->v_pJPG_REG->JPGMod = (EncParam->sampleMode == JPG_422) ? (0x1<<0) : (0x2<<0);
	jCTX->v_pJPG_REG->JPGRSTPos = 2; // MCU inserts RST marker
	jCTX->v_pJPG_REG->JPGQTblNo = (1<<12) | (1<<14);
	jCTX->v_pJPG_REG->JPGX = EncParam->width;
	jCTX->v_pJPG_REG->JPGY = EncParam->height;

	JPEG_LOG_MSG(LOG_TRACE, "encodeJPG", "EncParam->encType : %d\n", EncParam->encType);
	if(EncParam->encType == JPG_MAIN)
	{
		jCTX->v_pJPG_REG->JPGYUVAddr0  = jCTX->p_pYUVData_Buff; // Address of input image
		jCTX->v_pJPG_REG->JPGYUVAddr1  = jCTX->p_pYUVData_Buff; // Address of input image
		jCTX->v_pJPG_REG->JPGFileAddr0 = jCTX->p_pJPGData_Buff; // Address of JPEG stream
		jCTX->v_pJPG_REG->JPGFileAddr1 = jCTX->p_pJPGData_Buff; // next address of motion JPEG stream
	}
	
	// thumbnail encoding
	else
	{
		jCTX->v_pJPG_REG->JPGYUVAddr0  = jCTX->p_frmUserThumbBuf; // Address of input image
		jCTX->v_pJPG_REG->JPGYUVAddr1  = jCTX->p_frmUserThumbBuf; // Address of input image
		jCTX->v_pJPG_REG->JPGFileAddr0 = jCTX->p_strUserThumbBuf; // Address of JPEG stream
		jCTX->v_pJPG_REG->JPGFileAddr1 = jCTX->p_strUserThumbBuf; // next address of motion JPEG stream
	}

	jCTX->v_pJPG_REG->JPGCOEF1 = COEF1_RGB_2_YUV; // Coefficient value 1 for RGB to YCbCr
	jCTX->v_pJPG_REG->JPGCOEF2 = COEF2_RGB_2_YUV; // Coefficient value 2 for RGB to YCbCr
	jCTX->v_pJPG_REG->JPGCOEF3 = COEF3_RGB_2_YUV; // Coefficient value 3 for RGB to YCbCr

	jCTX->v_pJPG_REG->JPGMISC = (1<<5) | (0<<2);
	jCTX->v_pJPG_REG->JPGCntl = DISABLE_MOTION_ENC;


	// Quantiazation and Huffman Table setting
	for (i=0; i<64; i++)
		jCTX->v_pJPG_REG->JQTBL0[i] = (UINT32)QTBL_Luminance[EncParam->quality][i];

	for (i=0; i<64; i++)
		jCTX->v_pJPG_REG->JQTBL1[i] = (UINT32)QTBL_Chrominance[EncParam->quality][i];

	for (i=0; i<16; i++)
		jCTX->v_pJPG_REG->JHDCTBL0[i] = (UINT32)HDCTBL0[i];

	for (i=0; i<12; i++)
		jCTX->v_pJPG_REG->JHDCTBLG0[i] = (UINT32)HDCTBLG0[i];

	for (i=0; i<16; i++)
		jCTX->v_pJPG_REG->JHACTBL0[i] = (UINT32)HACTBL0[i];

	for (i=0; i<162; i++)
		jCTX->v_pJPG_REG->JHACTBLG0[i] = (UINT32)HACTBLG0[i];

	jCTX->v_pJPG_REG->JPGStart = 0;

	WaitForInterrupt();
	ret = jpg_irq_reason;

	if(ret != OK_ENC_OR_DEC){
		JPEG_LOG_MSG(LOG_ERROR, "encodeJPG", "DD::JPG Encoding Error(%d)\n", ret);
		return JPG_FAIL;
	}

	EncParam->fileSize = jCTX->v_pJPG_REG->JPGDataSize;
	return JPG_SUCCESS;

}
