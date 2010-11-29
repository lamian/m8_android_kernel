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
 * This source file is for initializing the MFC instance.
 *
 * @name MFC DRIVER MODULE Module (MFC_Inst_Init.c)
 * @author name(email address)
 * @date 03-28-07
 */
#include "Mfc.h"
#include "MFC_Instance.h"
#include "MfcMemory.h"
#include "DataBuf.h"
#include "FramBufMgr.h"
#include "LogMsg.h"
#include "MfcConfig.h"
#include "MfcSfr.h"
#include "BitProcBuf.h"
#include "MFC_Inst_Pool.h"

#include <asm/io.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <asm/memory.h>

// RainAde : timing issue
#include <linux/delay.h>

static MFCInstCtx _mfcinst_ctx[MFC_NUM_INSTANCES_MAX];


MFCInstCtx *MFCInst_GetCtx(int inst_no)
{
	if ((inst_no < 0) || (inst_no >= MFC_NUM_INSTANCES_MAX))
		return NULL;

	if (MFCINST_STATE(&(_mfcinst_ctx[inst_no])) >= MFCINST_STATE_CREATED)
		return &(_mfcinst_ctx[inst_no]);
	else
		return NULL;
}



// Filling the pStrmBuf and phyadrStrmBuf variables of the MfcInstCtx structure
// (pStrmBuf and phyadrStrmBuf are the virtual and physical address of STRM_BUF(stream buffer) respectively.)
static void Get_MfcStrmBufAddr(MFCInstCtx *ctx, int isLineBuf)
{
	// LINE BUF
	if (isLineBuf) {
		ctx->pStrmBuf		= (unsigned char *) ( GetDataBufVirAddr() + (ctx->inst_no * MFC_LINE_BUF_SIZE_PER_INSTANCE) );
		ctx->phyadrStrmBuf	= (PHYADDR_VAL) ( GetDataBufPhyAddr() + (ctx->inst_no * MFC_LINE_BUF_SIZE_PER_INSTANCE) );
		ctx->nStrmBufSize   = MFC_LINE_BUF_SIZE_PER_INSTANCE;
	}
	// RING BUF
	else {
#if (MFC_LINE_RING_SHARE == 1)
		ctx->pStrmBuf		= (unsigned char *) GetDataBufVirAddr();
		ctx->phyadrStrmBuf	= (PHYADDR_VAL) GetDataBufPhyAddr();
#elif (MFC_LINE_RING_SHARE == 0)
		ctx->pStrmBuf		= (unsigned char *) ( GetDataBufVirAddr() + MFC_LINE_BUF_SIZE);
		ctx->phyadrStrmBuf	= (PHYADDR_VAL) ( GetDataBufPhyAddr() + MFC_LINE_BUF_SIZE);
#endif
		ctx->nStrmBufSize   = MFC_RING_BUF_SIZE;
	}

}

// Filling the pFramBuf and phyadrFramBuf variables of the MfcInstCtx structure
// (pFramBuf and phyadrFramBuf are the virtual and physical address of FRAM_BUF(frame buffer) respectively.)
static BOOL Get_MfcFramBufAddr(MFCInstCtx *ctx, int buf_size)
{
	unsigned char	*pInstFramBuf;

	pInstFramBuf	= FramBufMgrCommit(ctx->inst_no, buf_size);
	if (pInstFramBuf == NULL) {
		LOG_MSG(LOG_ERROR, "Get_MfcFramBufAddr", "Frame buffer allocation was failed!\r\n");
		return FALSE;
	}

	FramBufMgrPrintCommitInfo();

	ctx->pFramBuf		= pInstFramBuf;	// virtual address of frame buffer	
	ctx->phyadrFramBuf	= S3C6400_BASEADDR_MFC_DATA_BUF + ( (int)pInstFramBuf - (int)GetDataBufVirAddr() );
	ctx->nFramBufSize   = buf_size;

	// RainAde 090907 : flush old frame buffer
	Mem_Set((char *)(ctx->pFramBuf), 0, ctx->nFramBufSize);
	
	LOG_MSG(LOG_TRACE, "Get_MfcFramBufAddr", "ctx->inst_no : %d\r\n", ctx->inst_no);
	LOG_MSG(LOG_TRACE, "Get_MfcFramBufAddr", "ctx->pFramBuf : 0x%X\r\n", ctx->pFramBuf);
	LOG_MSG(LOG_TRACE, "Get_MfcFramBufAddr", "ctx->phyadrFramBuf : 0x%X\r\n", ctx->phyadrFramBuf);

	return TRUE;
}


void MFCInst_RingBufAddrCorrection(MFCInstCtx *ctx)
{
	Get_MfcStrmBufAddr(ctx, 0);
}



int MFCInst_GetLineBuf(MFCInstCtx *ctx, unsigned char **ppBuf, int *size)
{
	////////////////////////////
	///    STATE checking    ///
	////////////////////////////
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_DELETED)) {
		LOG_MSG(LOG_ERROR, "MFCInst_GetRingBuf", "MFC instance is deleted.\r\n");
		return MFCINST_ERR_STATE_DELETED;
	}

	if (ctx->inbuf_type == DEC_INBUF_NOT_SPECIFIED)
		ctx->inbuf_type = DEC_INBUF_LINE_BUF;
	else if (ctx->inbuf_type == DEC_INBUF_RING_BUF)
		return MFCINST_ERR_ETC;

	*ppBuf = ctx->pStrmBuf;
	*size  = MFC_LINE_BUF_SIZE_PER_INSTANCE;

	return MFCINST_RET_OK;
}

int MFCInst_GetRingBuf(MFCInstCtx *ctx, unsigned char **ppBuf, int *size)
{
	unsigned char  *pRD_PTR, *pWR_PTR;
	int             num_bytes_strm_buf;	// Number of bytes in STRM_BUF (used in IOCTL_MFC_GET_RING_BUF_ADDR)

#if (MFC_LINE_RING_SHARE == 0)
	MFCInstCtx     *pMfcInstTmp;
	int             inst_no = 0;
#endif

	////////////////////////////
	///    STATE checking    ///
	////////////////////////////
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_DELETED)) {
		LOG_MSG(LOG_ERROR, "MFCInst_GetRingBuf", "MFC instance is deleted.\r\n");
		return MFCINST_ERR_STATE_DELETED;
	}

	
	if (ctx->inbuf_type == DEC_INBUF_NOT_SPECIFIED)
		ctx->inbuf_type = DEC_INBUF_RING_BUF;
	else if (ctx->inbuf_type == DEC_INBUF_LINE_BUF)
		return MFCINST_ERR_ETC;


	// state variable checking
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_CREATED)) {

#if (MFC_LINE_RING_SHARE == 1)
		// If LINE_BUF and RING_BUF are shared,
		// there must be no instances other than itself.
		if (MfcInstPool_NumAvail() != (MFC_NUM_INSTANCES_MAX-1))
			return MFCINST_ERR_ETC;

		// Occupy all the instances
		// so that the other instances won't be activated
		// until this VC-1 decode instance will be closed.
		MfcInstPool_OccupyAll();

#elif (MFC_LINE_RING_SHARE == 0)
		// If LINE_BUF and RING_BUF are separated.
		// there must be no other instances using the RING_BUF other than itself.
		if (MfcInstPool_NumAvail() < (MFC_NUM_INSTANCES_MAX-1)) {
			for (inst_no=0; inst_no<MFC_NUM_INSTANCES_MAX; inst_no++) {
				if (inst_no == ctx->inst_no)
					continue;

				pMfcInstTmp = MFCInst_GetCtx(inst_no);
				if (pMfcInstTmp == NULL)
					continue;
				if (pMfcInstTmp->inbuf_type == DEC_INBUF_RING_BUF) {
					return MFCINST_ERR_ETC;
				}
			}
		}

		// If LINE_BUF & RING_BUF are not shared,
		// the RING_BUF address must be different from the LINE_BUF.
		// For this reason, the address needs correction at first.
		MFCInst_RingBufAddrCorrection(ctx);
#endif
	}

	// Get the current positions of RD_PTR and WR_PTR
	MFCInst_GetStreamRWPtrs(ctx, &pRD_PTR, &pWR_PTR);

	// When MFC instance is in the state of MFCINST_STATE_CREATED,
	// the size of data to be filled in RING_BUF is double the PARTUNIT size.
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_CREATED)) {
		*ppBuf = pWR_PTR;
		*size  = MFC_RING_BUF_PARTUNIT_SIZE << 1;

		MFCINST_STATE_BUF_FILL_REQ_SET(ctx);

		return MFCINST_RET_OK;
	}


	*ppBuf = pWR_PTR;
	num_bytes_strm_buf = (int)pWR_PTR - (int)pRD_PTR;
	if (num_bytes_strm_buf <= 0)	// if WR_PTR is behind the RD_PTR.
		num_bytes_strm_buf  += MFC_RING_BUF_SIZE;

	// Num of bytes in RING_BUF is less than MFC_RING_BUF_PARTUNIT_SIZE,
	// then load one more MFC_RING_BUF_PARTUNIT_SIZE unit.
	if ((num_bytes_strm_buf < MFC_RING_BUF_PARTUNIT_SIZE)
			&& MFCINST_STATE_CHECK(ctx, MFCINST_STATE_DEC_PIC_RUN_RING_BUF)) {
		*size = MFC_RING_BUF_PARTUNIT_SIZE;
		MFCINST_STATE_BUF_FILL_REQ_SET(ctx);
	}
	else
		*size = 0;


	return MFCINST_RET_OK;
}

int MFCInst_GetFramBuf(MFCInstCtx *ctx, unsigned char **ppBuf, int *size)
{
	////////////////////////////
	///    STATE checking    ///
	////////////////////////////
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_DELETED)) {
		LOG_MSG(LOG_ERROR, "MFCInst_GetFramBuf", "MFC instance is deleted.\r\n");
		return MFCINST_ERR_STATE_DELETED;
	}
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_CREATED)) {
		LOG_MSG(LOG_ERROR, "MFCInst_GetFramBuf", "MFC instance is not initialized.\r\n");
		return MFCINST_ERR_STATE_CHK;
	}

	if (ctx->pFramBuf == NULL) {
		LOG_MSG(LOG_ERROR, "MFCInst_GetFramBuf", "MFC Frame buffer is not internally allocated yet.\n");
		return MFCINST_ERR_ETC;
	}


	*size  = (ctx->buf_width * ctx->buf_height * 3) >> 1;	// YUV420 frame size

	if (ctx->idx < 0)	// RET_DEC_PIC_IDX == -3  (No picture to be displayed)
		*ppBuf = NULL;
	else {
		*ppBuf = ctx->pFramBuf + (ctx->idx) * (*size);
#if (MFC_ROTATE_ENABLE == 1)
		// RainAde : added condition for compitibility with code of outside
		//if (ctx->PostRotMode & 0x0010) 
		if( (ctx->codec_mode != VC1_DEC) && (ctx->PostRotMode & 0x0010) )
			*ppBuf = ctx->pFramBuf + (ctx->frambufCnt) * (*size);
#endif
	}


	return MFCINST_RET_OK;
}

int MFCInst_GetFramBufPhysical(MFCInstCtx *ctx, unsigned char **ppBuf, int *size)
{
	////////////////////////////
	///    STATE checking    ///
	////////////////////////////
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_DELETED)) {
		LOG_MSG(LOG_ERROR, "MFCInst_GetFramBuf", "MFC instance is deleted.\r\n");
		return MFCINST_ERR_STATE_DELETED;
	}
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_CREATED)) {
		LOG_MSG(LOG_ERROR, "MFCInst_GetFramBuf", "MFC instance is not initialized.\r\n");
		return MFCINST_ERR_STATE_CHK;
	}

	if (ctx->pFramBuf == NULL) {
		LOG_MSG(LOG_ERROR, "MFCInst_GetFramBuf", "MFC Frame buffer is not internally allocated yet.\n");
		return MFCINST_ERR_ETC;
	}

#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))
    *size  = (ctx->buf_width* ctx->buf_height* 3) >> 1;    // YUV420 frame size
#else
	*size  = (ctx->width * ctx->height * 3) >> 1;	// YUV420 frame size
#endif

	if (ctx->idx < 0)	// RET_DEC_PIC_IDX == -3  (No picture to be displayed)
		*ppBuf = NULL;
	else
		*ppBuf = (unsigned char *) ( ctx->phyadrFramBuf + (ctx->idx) * (*size) );


	return MFCINST_RET_OK;
}

//
// Function Name: MFCInst_GetInstNo
// Description
//      It returns the instance number of the 6400 MFC instance context.
// Parameters
//      ctx[IN]: MFCInstCtx
//
int MFCInst_GetInstNo(MFCInstCtx *ctx)
{
	return ctx->inst_no;
}

//
// Function Name: MFCInst_GetStreamRWPtrs
// Description
//      It returns the virtual address of RD_PTR and WR_PTR.
// Parameters
//      ctx[IN]: MFCInstCtx
//      ppRD_PTR[OUT]: RD_PTR
//      ppWR_PTR[OUT]: WR_PTR
//
BOOL MFCInst_GetStreamRWPtrs(MFCInstCtx *ctx, unsigned char **ppRD_PTR, unsigned char **ppWR_PTR)
{
	S3C6400_MFC_SFR	*mfc_sfr;		// MFC SFR pointer
	int              diff_vir_phy;


	if (MFCINST_STATE(ctx) < MFCINST_STATE_CREATED)
		return FALSE;

	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_CREATED)) {
		// If MFCInstCtx is just created and not initialized by MFCInst_Init,
		// then the initial RD_PTR and WR_PTR are the start address of STRM_BUF.
		*ppRD_PTR = ctx->pStrmBuf;
		*ppWR_PTR = ctx->pStrmBuf;
	}
	else {
		// The physical to virtual address conversion of RD_PTR and WR_PTR.
		diff_vir_phy  =  (int) (ctx->pStrmBuf - ctx->phyadrStrmBuf);
		mfc_sfr = (S3C6400_MFC_SFR *) GetMfcSfrVirAddr();
		*ppRD_PTR = (unsigned char *) (diff_vir_phy + mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_RD_PTR);
		*ppWR_PTR = (unsigned char *) (diff_vir_phy + mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_WR_PTR);
	}

	return TRUE;
}

BOOL MFCInst_SetInbufType(MFCInstCtx *ctx, MFCINST_DEC_INBUF_TYPE inbuf_type)
{
	ctx->inbuf_type = inbuf_type;

	return TRUE;
}


unsigned int MFCInst_Set_PostRotate(MFCInstCtx *ctx, unsigned int post_rotmode)
{
	unsigned int old_post_rotmode;

	old_post_rotmode = ctx->PostRotMode;

	if (post_rotmode & 0x0010) {
		ctx->PostRotMode = post_rotmode;
	}
	else
		ctx->PostRotMode = 0;


	return old_post_rotmode;
}


MFCInstCtx *MFCInst_Create(void)
{
	MFCInstCtx *ctx;
	int		inst_no;

	// Occupy the 'inst_no'.
	// If it fails, it returns NULL.
	inst_no = MfcInstPool_Occupy();
	if (inst_no == -1)
		return NULL;


	ctx = &(_mfcinst_ctx[inst_no]);

	Mem_Set(ctx, 0, sizeof(MFCInstCtx));

	ctx->inst_no     = inst_no;
	ctx->inbuf_type  = DEC_INBUF_NOT_SPECIFIED;
// RainAde : for initialize QS default value in case this value is not setted properly
	ctx->pictureQs = 8;

	MFCINST_STATE_TRANSITION(ctx, MFCINST_STATE_CREATED);

	// At first, since it is not known whether it is LINE_BUF or RING_BUF,
	// it assumes to be LINE_BUF.
	// Later, if it turns out to be RING_BUF, correction will be made.
	Get_MfcStrmBufAddr(ctx, 1);

	LOG_MSG(LOG_TRACE, "s3c_mfc_open", "state : %d\n", ctx->state_var);


	return ctx;
}


//
// Function Name: MFCInst_Delete
// Description
//      It deletes the 6400 MFC instance.
// Parameters
//      ctx[IN]: MFCInstCtx
//
void MFCInst_Delete(MFCInstCtx *ctx)
{
	////////////////////////////
	///    STATE checking    ///
	////////////////////////////
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_DELETED)) {
		LOG_MSG(LOG_ERROR, "MFCInst_Delete", "MFC instance is already deleted.\r\n");
		return;
	}

	MfcInstPool_Release(ctx->inst_no);
	FramBufMgrFree(ctx->inst_no);
	FramBufMgrFree(ctx->inst_no + MFC_NUM_INSTANCES_MAX);

	MFCINST_STATE_TRANSITION(ctx, MFCINST_STATE_DELETED);
}


//
// Function Name: MFCInst_PowerOffState
// Description
//      It turns on the flag indicating 6400 MFC's power-off.
// Parameters
//      ctx[IN]: MFCInstCtx
//
void MFCInst_PowerOffState(MFCInstCtx *ctx)
{
	MFCINST_STATE_PWR_OFF_FLAG_SET(ctx);
}

//
// Function Name: MFCInst_PowerOnState
// Description
//      It turns on the flag indicating 6400 MFC's power-off.
// Parameters
//      ctx[IN]: MFCInstCtx
//
void MFCInst_PowerOnState(MFCInstCtx *ctx)
{
	MFCINST_STATE_PWR_OFF_FLAG_CLEAR(ctx);
}


//
// Function Name: MFCInst_Init
// Description
//      It initializes the 6400 MFC instance with the appropriate config stream.
//      The config stream must be copied into STRM_BUF before this function.
// Parameters
//      ctx[IN]: MFCInstCtx
//      codec_mode[IN]: codec mode specifying H.264/MPEG4/H.263.
//      strm_leng[IN]: stream size (especially it should be the config stream.)
//
int MFCInst_Init(MFCInstCtx *ctx, MFC_CODECMODE codec_mode, unsigned long strm_leng)
{
	unsigned int    i;

	S3C6400_MFC_SFR	*mfc_sfr;		// MFC SFR pointer

	unsigned char	*pPARAM_BUF;	// PARAM_BUF in BITPROC_BUF
	int              nFramBufSize;	// Required size in FRAM_BUF
	int              frame_size;	// width * height

	
	static S3C6400_MFC_PARAM_REG_DEC_SEQ_INIT    *pPARAM_SEQ_INIT;		// Parameter of SEQ_INIT command
	static S3C6400_MFC_PARAM_REG_SET_FRAME_BUF   *pPARAM_SET_FRAME_BUF;	// Parameter of SET_FRAME_BUF command
	volatile S3C6400_MFC_PARAM_REG_DEC_PIC_RUN	*pPARAM_DEC_PIC_RUN;	// yj

	int				last_unit = 0;
	int				garbage_size = 0;
	

	////////////////////////////
	///    STATE checking    ///
	////////////////////////////
	if (!MFCINST_STATE_CHECK(ctx, MFCINST_STATE_CREATED)) {
		LOG_MSG(LOG_ERROR, "MFCInst_Init", "Init function was called at an incorrect point.\r\n");
		return MFCINST_ERR_STATE_CHK;
	}

	
	// codec_mode
	ctx->codec_mode = codec_mode;

	// inbuf_type checking
	if (ctx->inbuf_type == DEC_INBUF_NOT_SPECIFIED) {
		LOG_MSG(LOG_ERROR, "MFCInst_Init", "Input buffer type is not specified.\r\n");
		return MFCINST_ERR_ETC;
	}


	//////////////////////////////////////////////
	//                                          //
	// 2. Copy the Config Stream into STRM_BUF. //
	//                                          //
	//////////////////////////////////////////////
	// config stream needs to be in the STRM_BUF a priori.
	// RD_PTR & WR_PTR is set to point the start and end address of STRM_BUF.
	// If WR_PTR is set to [start + config_leng] instead of [end address of STR_BUF],
	// then MFC is not initialized when MPEG4 decoding.
	mfc_sfr = (S3C6400_MFC_SFR *) GetMfcSfrVirAddr();
	mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_RD_PTR = ctx->phyadrStrmBuf;
	if (ctx->inbuf_type == DEC_INBUF_LINE_BUF)	// In 'fileplay' mode, set the WR_PTR to the end of STRM_BUF
		strm_leng = MFC_LINE_BUF_SIZE_PER_INSTANCE;
	else if (ctx->inbuf_type == DEC_INBUF_RING_BUF) {
		if (strm_leng < (MFC_RING_BUF_PARTUNIT_SIZE << 1)) {
			if (strm_leng & 0x1FF) {
				garbage_size = strm_leng;
				strm_leng = (strm_leng + 512) & 0xFFFFFE00;
				garbage_size = strm_leng - garbage_size;
				Mem_Set((char *)(ctx->pStrmBuf + strm_leng - garbage_size), 0, garbage_size);
			}
			last_unit = 1;
		}

		// In the SEQ_INIT phase, the stream length should be more than 5
		// since it must contain the CONFIG stream at the least.
		if (strm_leng < 5) {
			LOG_MSG(LOG_ERROR, "MFCInst_Init", "Size of buffer fill is too small.\r\n");
			return MFCINST_ERR_DEC_BUF_FILL_SIZE_WRONG;
		}

		MFCINST_STATE_BUF_FILL_REQ_CLEAR(ctx);
	}
	
	mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_WR_PTR = ctx->phyadrStrmBuf  +  strm_leng;

	LOG_MSG(LOG_TRACE, "MFCInst_Init", "  ctx->phyadrStrmBuf   = 0x%X\n", ctx->phyadrStrmBuf);
	LOG_MSG(LOG_TRACE, "MFCInst_Init", "  ctx->phyadrStrmBuf + = 0x%X\n", ctx->phyadrStrmBuf + strm_leng);


	///////////////////////////////////////////////////////////////////
	//                                                               //
	// 3. Issue the SEQ_INIT command                                 //
	//     (width/height of frame will be obtained)                  //
	//                                                               //
	///////////////////////////////////////////////////////////////////
	// Set the Parameters for SEQ_INIT command.
	pPARAM_SEQ_INIT = (S3C6400_MFC_PARAM_REG_DEC_SEQ_INIT *)  MfcGetCmdParamRegion();
	pPARAM_SEQ_INIT->DEC_SEQ_BIT_BUF_ADDR   = ctx->phyadrStrmBuf;
	if (ctx->inbuf_type == DEC_INBUF_LINE_BUF) {	// Other than VC-1 decode
		pPARAM_SEQ_INIT->DEC_SEQ_BIT_BUF_SIZE   = MFC_LINE_BUF_SIZE_PER_INSTANCE / 1024;
		
		// yj: Enable 'Mp4DbkOn' bit
		if(ctx->isMp4DbkOn == 1) 
			pPARAM_SEQ_INIT->DEC_SEQ_OPTION         = FILEPLAY_ENABLE | DYNBUFALLOC_ENABLE | MP4_DBK_ENABLE;
		else
		pPARAM_SEQ_INIT->DEC_SEQ_OPTION         = FILEPLAY_ENABLE | DYNBUFALLOC_ENABLE;
		
		pPARAM_SEQ_INIT->DEC_SEQ_START_BYTE     = 0;
	}
	else if (ctx->inbuf_type == DEC_INBUF_RING_BUF) {		// VC-1 decode case
		pPARAM_SEQ_INIT->DEC_SEQ_BIT_BUF_SIZE   = MFC_RING_BUF_SIZE / 1024;
		pPARAM_SEQ_INIT->DEC_SEQ_OPTION         = MP4_DBK_ENABLE; 
		pPARAM_SEQ_INIT->DEC_SEQ_START_BYTE     = 0;
	}
	else {
		LOG_MSG(LOG_ERROR, "MFCInst_Init", "Input buffer type is not specified.\r\n");
		return MFCINST_ERR_ETC;
	}

	LOG_MSG(LOG_TRACE, "MFCInst_Init", "  ctx->inst_no     = %d\n", ctx->inst_no);
	LOG_MSG(LOG_TRACE, "MFCInst_Init", "  ctx->codec_mode  = %d\n", ctx->codec_mode);
	LOG_MSG(LOG_TRACE, "MFCInst_Init", "  SEQ_BIT_BUF_SIZE = %d (KB)\n", pPARAM_SEQ_INIT->DEC_SEQ_BIT_BUF_SIZE);


	MfcSetEos(0);
	
	// SEQ_INIT command
	if (MfcIssueCmd(ctx->inst_no, ctx->codec_mode, SEQ_INIT) == FALSE) {
		LOG_MSG(LOG_ERROR, "MFCInst_Init", "SEQ_INIT failed.\n");
		MfcStreamEnd();
		return MFCINST_ERR_DEC_INIT_CMD_FAIL;
	}

	MfcStreamEnd();

	if (pPARAM_SEQ_INIT->RET_SEQ_SUCCESS == TRUE) {

		LOG_MSG(LOG_TRACE, "MFCInst_Init", "\tRET_DEC_SEQ_SRC_SIZE         = %d\n", pPARAM_SEQ_INIT->RET_DEC_SEQ_SRC_SIZE);
		LOG_MSG(LOG_TRACE, "MFCInst_Init", "\tRET_DEC_SEQ_SRC_FRAME_RATE   = %d\n", pPARAM_SEQ_INIT->RET_DEC_SEQ_SRC_FRAME_RATE);
		LOG_MSG(LOG_TRACE, "MFCInst_Init", "\tRET_DEC_SEQ_FRAME_NEED_COUNT = %d\n", pPARAM_SEQ_INIT->RET_DEC_SEQ_FRAME_NEED_COUNT);
		LOG_MSG(LOG_TRACE, "MFCInst_Init", "\tRET_DEC_SEQ_FRAME_DELAY      = %d\n", pPARAM_SEQ_INIT->RET_DEC_SEQ_FRAME_DELAY);
	}
	else {
		LOG_MSG(LOG_ERROR, "MFCInst_Init", "SEQ_INIT failed. [%d]\n", pPARAM_SEQ_INIT->RET_SEQ_SUCCESS);
		return MFCINST_ERR_DEC_INIT_CMD_FAIL;
	}

	// width & height are obtained from return value of SEQ_INIT command
	// stride value is multiple of 16.
	ctx->height = (pPARAM_SEQ_INIT->RET_DEC_SEQ_SRC_SIZE      ) & 0x03FF;
	ctx->width  = (pPARAM_SEQ_INIT->RET_DEC_SEQ_SRC_SIZE >> 10) & 0x03FF;
	if ((ctx->width & 0x0F) == 0)	// 16 aligned (ctx->width%16 == 0)
		ctx->buf_width  = ctx->width;
	else
		ctx->buf_width  = (ctx->width  & 0xFFFFFFF0) + 16;
	if ((ctx->height & 0x0F) == 0)	// 16 aligned (ctx->height%16 == 0)
		ctx->buf_height = ctx->height;
	else
		ctx->buf_height = (ctx->height & 0xFFFFFFF0) + 16;

	LOG_MSG(LOG_TRACE, "MFCInst_Init", "width : %d, height : %d, buf_width : %d, buf_height : %d\n", ctx->width, ctx->height, ctx->buf_width, ctx->buf_height);
	
	// If codec mode is VC1_DEC,
	// width & height value are not from return value of SEQ_INIT command
	// but extracting from config stream.
	if (ctx->codec_mode == VC1_DEC) {
		Mem_Cpy(&(ctx->height), ctx->pStrmBuf + 12, 4);
		Mem_Cpy(&(ctx->width), ctx->pStrmBuf + 16, 4);
		ctx->buf_width = ctx->width;
	}

#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))
    ctx->buf_width  += 2*ctx->paddingSize;
    ctx->buf_height += 2*ctx->paddingSize;

    ctx->RET_DEC_SEQ_INIT_BAK_MP4ASP_VOP_TIME_RES   = pPARAM_SEQ_INIT->RET_DEC_SEQ_TIME_RES;
#endif

	LOG_MSG(LOG_TRACE, "MFCInst_Init", "SEQ_SRC_SIZE, (width=%d) (height=%d) (stride=%d)\r\n", 	\
			ctx->width, ctx->height, ctx->buf_width);


	////////////////////////////////////////////////
	//                                            //
	// 4. Getting FRAME_BUF for this instance     //
	//                                            //
	////////////////////////////////////////////////
	// nFramBufSize is (YUV420 frame size) * (required frame buffer count)
#if (MFC_ROTATE_ENABLE == 1)
	// If rotation is enabled, one more YUV buffer is required.
	nFramBufSize = ((ctx->buf_width * ctx->buf_height * 3) >> 1)  *  (pPARAM_SEQ_INIT->RET_DEC_SEQ_FRAME_NEED_COUNT + 1);
#else
	nFramBufSize = ((ctx->buf_width * ctx->buf_height * 3) >> 1)  *  pPARAM_SEQ_INIT->RET_DEC_SEQ_FRAME_NEED_COUNT;
#endif
    nFramBufSize += 60000;//27540;    // 27,540 bytes = MV(25,920) + MBTypes(1,620)
	if ( Get_MfcFramBufAddr(ctx, nFramBufSize) == FALSE ) {
		LOG_MSG(LOG_ERROR, "MFCInst_Init", "MFC Instance init failed! (required frame buffer size = %d)\r\n", nFramBufSize);
		return MFCINST_ERR_ETC;
	}
	
	ctx->framBufAllocated	= 1;
	ctx->frambufCnt			= pPARAM_SEQ_INIT->RET_DEC_SEQ_FRAME_NEED_COUNT;
	ctx->mv_mbyte_addr    = ctx->phyadrFramBuf + (nFramBufSize - 60000/*27540*/);


	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// 5. Set the Parameters in the PARA_BUF for SET_FRAME_BUF command. //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	// Buffer address of Y, Cb, Cr will be set in PARAM_BUF before issuing SET_FRAME_BUF command.

	pPARAM_BUF = (unsigned char *)GetParamBufVirAddr();
    frame_size = ctx->buf_width * ctx->buf_height;
#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))
	// s/w divx use padding area, so mfc frame buffer must have stride.
	for (i=0; i<pPARAM_SEQ_INIT->RET_DEC_SEQ_FRAME_NEED_COUNT; i++)
	{
		*((int *) (pPARAM_BUF + i*3*4))      = ctx->phyadrFramBuf + i * ((frame_size * 3) >> 1) + (ctx->buf_width)*ctx->paddingSize+ ctx->paddingSize;
        *((int *) (pPARAM_BUF + i*3*4 + 4))  = ctx->phyadrFramBuf + i * ((frame_size * 3) >> 1) + frame_size + ((ctx->buf_width)/2)*(ctx->paddingSize/2)+ (ctx->paddingSize/2);
        *((int *) (pPARAM_BUF + i*3*4 + 8))  = ctx->phyadrFramBuf + i * ((frame_size * 3) >> 1) + frame_size + (frame_size >> 2) + ((ctx->buf_width)/2)*(ctx->paddingSize/2)+ (ctx->paddingSize/2);
	}
#else	
	for (i=0; i<pPARAM_SEQ_INIT->RET_DEC_SEQ_FRAME_NEED_COUNT; i++)
	{
		*((int *) (pPARAM_BUF + i*3*4))      = ctx->phyadrFramBuf + i * ((frame_size * 3) >> 1);
		*((int *) (pPARAM_BUF + i*3*4 + 4))  = ctx->phyadrFramBuf + i * ((frame_size * 3) >> 1) + frame_size;
		*((int *) (pPARAM_BUF + i*3*4 + 8))  = ctx->phyadrFramBuf + i * ((frame_size * 3) >> 1) + frame_size + (frame_size >> 2);
	}
#endif


	////////////////////////////////////////
	//                                    //
	// 6. Issue the SET_FRAME_BUF command //
	//                                    //
	////////////////////////////////////////
	// 'SET_FRAME_BUF_NUM' must be greater than or equal to RET_DEC_SEQ_FRAME_NEED_COUNT.
	pPARAM_SET_FRAME_BUF = (S3C6400_MFC_PARAM_REG_SET_FRAME_BUF *)  MfcGetCmdParamRegion();
	pPARAM_SET_FRAME_BUF->SET_FRAME_BUF_NUM    = pPARAM_SEQ_INIT->RET_DEC_SEQ_FRAME_NEED_COUNT;
	pPARAM_SET_FRAME_BUF->SET_FRAME_BUF_STRIDE = ctx->buf_width;


	MfcIssueCmd(ctx->inst_no, ctx->codec_mode, SET_FRAME_BUF);

	/*
	  * If Mp4DbkOn bit is enabled, set the dbk-address.
	  * De-blocking filtered image stored to the address.
	  *
	  * 2009.5.12 by yj (yunji.kim@samsung.com)
	  */
	if((pPARAM_SEQ_INIT->DEC_SEQ_OPTION  & MP4_DBK_ENABLE) == 0x1)
	{
		frame_size = ctx->buf_width * ctx->buf_height;

		pPARAM_DEC_PIC_RUN = (S3C6400_MFC_PARAM_REG_DEC_PIC_RUN *)  MfcGetCmdParamRegion();
		pPARAM_DEC_PIC_RUN->DEC_PIC_DBK_ADDR_Y = GetDbkBufPhyAddr();
		pPARAM_DEC_PIC_RUN->DEC_PIC_DBK_ADDR_CB = pPARAM_DEC_PIC_RUN->DEC_PIC_DBK_ADDR_Y + frame_size;
		pPARAM_DEC_PIC_RUN->DEC_PIC_DBK_ADDR_CR = pPARAM_DEC_PIC_RUN->DEC_PIC_DBK_ADDR_CB + (frame_size >> 2);
	}

	////////////////////////////
	///    STATE changing    ///
	////////////////////////////
	// Change the state to MFCINST_STATE_DEC_INITIALIZED
	// If the input stream data is less than the 2 PARTUNITs size,
	// then the state is changed to MFCINST_STATE_DEC_PIC_RUN_RING_BUF_LAST_UNITS.
	MFCINST_STATE_TRANSITION(ctx, MFCINST_STATE_DEC_INITIALIZED);
	if (last_unit) {
		MFCINST_STATE_TRANSITION(ctx, MFCINST_STATE_DEC_PIC_RUN_RING_BUF_LAST_UNITS);
		MfcSetEos(0);
	}

	LOG_MSG(LOG_TRACE, "MFCInst_Init", "After init state : %d\n", ctx->state_var);


	return MFCINST_RET_OK;
}


int MFCInst_Enc_Init(MFCInstCtx *ctx, MFC_CODECMODE codec_mode, enc_info_t *enc_info)
{
	int              i;

	S3C6400_MFC_SFR	*mfc_sfr;		// MFC SFR pointer

	unsigned char	*pPARAM_BUF;	// PARAM_BUF in BITPROC_BUF
	int              nFramBufSize;	// Required size in FRAM_BUF
	int              frame_size;	// width * height
	int              num_mbs;		// Number of MBs
	int              slices_mb;		// MB number of slice (only if SLICE_MODE_MULTIPLE is selected.)
	
	static S3C6400_MFC_PARAM_REG_ENC_SEQ_INIT    *pPARAM_SEQ_INIT;		// Parameter of SEQ_INIT command
	static S3C6400_MFC_PARAM_REG_SET_FRAME_BUF   *pPARAM_SET_FRAME_BUF;	// Parameter of SET_FRAME_BUF command


	// check parameters from user application
	if ( (enc_info->width & 0x0F) || (enc_info->height & 0x0F) ) {
		LOG_MSG(LOG_ERROR, "MFCInst_Enc_Init", "Source picture width and height must be a multiple of 16. width : %d, height : %d\n", \
			enc_info->width, enc_info->height);
		
		return MFCINST_ERR_INVALID_PARAM;
	}


	if (enc_info->gopNum > 60) {
		LOG_MSG(LOG_ERROR, "MFCInst_Enc_Init", "Maximum GOP number is 60.  GOP number = %d\n", enc_info->gopNum);

		return MFCINST_ERR_INVALID_PARAM;
	}

	ctx->width			= enc_info->width;
	ctx->height			= enc_info->height;
	ctx->frameRateRes	= enc_info->frameRateRes;
	ctx->frameRateDiv	= enc_info->frameRateDiv;
	ctx->gopNum			= enc_info->gopNum;
	ctx->bitrate		= enc_info->bitrate;

	/*
	 * At least 2 frame buffers are needed. 
	 * These buffers are used for input buffer in encoder case
	 */
	ctx->frambufCnt		= 2;


	// This part is not required since the width and the height are checked to be multiples of 16
	// in the beginning of this function.
	if ((ctx->width & 0x0F) == 0)	// 16 aligned (ctx->width%16 == 0)
		ctx->buf_width = ctx->width;
	else
		ctx->buf_width = (ctx->width & 0xFFFFFFF0) + 16;



	// codec_mode
	ctx->codec_mode = codec_mode;
	
	LOG_MSG(LOG_TRACE, "MFCInst_Enc_Init", "  ctx->inst_no = %d\n", ctx->inst_no);
	LOG_MSG(LOG_TRACE, "MFCInst_Enc_Init", "  ctx->codec_mode = %d\n", ctx->codec_mode);


	/*
	 * set stream buffer read/write pointer
	 * At first, stream buffer is empty. so write pointer points start of buffer and read pointer points end of buffer
	 */
	mfc_sfr = (S3C6400_MFC_SFR *) GetMfcSfrVirAddr();
	mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_WR_PTR = ctx->phyadrStrmBuf;
	mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_RD_PTR = ctx->phyadrStrmBuf  +  MFC_LINE_BUF_SIZE_PER_INSTANCE;
	mfc_sfr->STRM_BUF_CTRL = 0x1C;	// bit stream buffer is reset at every picture encoding / decoding command

	LOG_MSG(LOG_TRACE, "MFCInst_Enc_Init", "  ctx->phyadrStrmBuf = 0x%X\n", ctx->phyadrStrmBuf);
	LOG_MSG(LOG_TRACE, "MFCInst_Enc_Init", "  ctx->phyadrStrmBuf + = 0x%X\n", ctx->phyadrStrmBuf + MFC_LINE_BUF_SIZE_PER_INSTANCE);


	///////////////////////////////////////////////////////////////////
	//                                                               //
	// 3. Issue the SEQ_INIT command                                 //
	//     (frame width/height will be returned.)                    //
	//                                                               //
	///////////////////////////////////////////////////////////////////
	// Set the Parameters for SEQ_INIT command.
	pPARAM_SEQ_INIT = (S3C6400_MFC_PARAM_REG_ENC_SEQ_INIT *) MfcGetCmdParamRegion();
	memset(pPARAM_SEQ_INIT, 0, sizeof(S3C6400_MFC_PARAM_REG_ENC_SEQ_INIT));
	
	pPARAM_SEQ_INIT->ENC_SEQ_BIT_BUF_ADDR	= ctx->phyadrStrmBuf;
	pPARAM_SEQ_INIT->ENC_SEQ_BIT_BUF_SIZE   = MFC_LINE_BUF_SIZE_PER_INSTANCE / 1024;
	pPARAM_SEQ_INIT->ENC_SEQ_OPTION			= MB_BIT_REPORT_DISABLE | SLICE_INFO_REPORT_DISABLE | AUD_DISABLE;
	pPARAM_SEQ_INIT->ENC_SEQ_SRC_SIZE		= (ctx->width << 10) | ctx->height;
	pPARAM_SEQ_INIT->ENC_SEQ_SRC_F_RATE		= (ctx->frameRateDiv << 16) | ctx->frameRateRes;
	pPARAM_SEQ_INIT->ENC_SEQ_SLICE_MODE		= SLICE_MODE_ONE;
	pPARAM_SEQ_INIT->ENC_SEQ_GOP_NUM		= ctx->gopNum;
	pPARAM_SEQ_INIT->ENC_SEQ_RC_PARA		= ctx->rcPara | (ctx->bitrate << 1);	// set rate control
	pPARAM_SEQ_INIT->ENC_SEQ_INTRA_MB		= 0x0;		// Intra MB refresh is not used
	pPARAM_SEQ_INIT->ENC_SEQ_FMO			= FMO_DISABLE;
	pPARAM_SEQ_INIT->ENC_SEQ_SLICE_MODE		= SLICE_MODE_ONE;

	LOG_MSG(LOG_TRACE, "MFCInst_Enc_Init", "pPARAM_SEQ_INIT->ENC_SEQ_RC_PARA = %x\r\n", pPARAM_SEQ_INIT->ENC_SEQ_RC_PARA);
	
	switch(ctx->codec_mode) {
		case MP4_ENC:
			pPARAM_SEQ_INIT->ENC_SEQ_COD_STD	= MPEG4_ENCODE;
			pPARAM_SEQ_INIT->ENC_SEQ_MP4_PARA	= DATA_PART_DISABLE;

			break;

		case H263_ENC:
			pPARAM_SEQ_INIT->ENC_SEQ_COD_STD	= H263_ENCODE;
			pPARAM_SEQ_INIT->ENC_SEQ_263_PARA 	= ctx->h263_annex;

			if (ctx->multiple_slice == 1) {
				pPARAM_SEQ_INIT->ENC_SEQ_SLICE_MODE = SLICE_MODE_MULTIPLE;
				pPARAM_SEQ_INIT->ENC_SEQ_263_PARA 	= ANNEX_K_ON;	
			}
			else if (ctx->enc_num_slices){
				// MB size is 16x16. -> width & height are divided by 16 to get number of MBs. (Division by 16 == shift right by 4-bit)
				num_mbs   = (enc_info->width >> 4) * (enc_info->height >> 4);
				slices_mb = (num_mbs / ctx->enc_num_slices);
				pPARAM_SEQ_INIT->ENC_SEQ_SLICE_MODE = SLICE_MODE_MULTIPLE | (1 << 1) | (slices_mb<< 2);
				pPARAM_SEQ_INIT->ENC_SEQ_263_PARA 	= ctx->h263_annex;
			}
			else if ( ((enc_info->width == 704) && (enc_info->height == 576)) || ((enc_info->width == 352) && (enc_info->height == 288)) || \
					((enc_info->width == 176) && (enc_info->height == 144))  || ((enc_info->width == 128) && (enc_info->height == 96)) ) {
				LOG_MSG(LOG_TRACE, "MFCInst_Enc_Init", "ENC_SEQ_263_PARA = 0x%X\n", pPARAM_SEQ_INIT->ENC_SEQ_263_PARA);
			}
			else if (ctx->h263_annex == 0) {
				LOG_MSG(LOG_ERROR, "MFCInst_Enc_Init", "H.263 Encoder supports 4CIF, CIF, QCIF and Sub-QCIF, when all Annex were off\n");
				return MFCINST_ERR_INVALID_PARAM;
			}
			
			break;

		case AVC_ENC:
			pPARAM_SEQ_INIT->ENC_SEQ_COD_STD	= H264_ENCODE;
			pPARAM_SEQ_INIT->ENC_SEQ_264_PARA	= ~(0xFFFF);

			if (ctx->enc_num_slices) {
				// MB size is 16x16. -> width & height are divided by 16 to get number of MBs. (Division by 16 == shift right by 4-bit)
				num_mbs   = (enc_info->width >> 4) * (enc_info->height >> 4);
				slices_mb = (num_mbs / ctx->enc_num_slices);
				pPARAM_SEQ_INIT->ENC_SEQ_SLICE_MODE = SLICE_MODE_MULTIPLE | (1 << 1) | (slices_mb<< 2);
			}

			break;
		
		default:
			LOG_MSG(LOG_ERROR, "MFCInst_Enc_Init", "MFC encoder supports MPEG4, H.264 and H.263.\n");			
			return MFCINST_ERR_INVALID_PARAM;
	}


	// SEQ_INIT command
	LOG_MSG(LOG_TRACE, "MFCInst_Enc_Init", "ANNEX K : 0x%X,  SLICE MODE : 0x%X\n", pPARAM_SEQ_INIT->ENC_SEQ_263_PARA, pPARAM_SEQ_INIT->ENC_SEQ_SLICE_MODE);
	MfcIssueCmd(ctx->inst_no, ctx->codec_mode, SEQ_INIT);

	if (pPARAM_SEQ_INIT->RET_ENC_SEQ_SUCCESS == TRUE)
	{
		LOG_MSG(LOG_TRACE, "MFCInst_Enc_Init", "ENC SEQ INIT success\n");			
	}
	else {
		LOG_MSG(LOG_ERROR, "MFCInst_Enc_Init", "ENC SEQ INIT Fail\n");			
		return MFCINST_ERR_ENC_INIT_CMD_FAIL;
	}

	// nFramBufSize is (YUV420 frame size) * (required frame buffer count)
	nFramBufSize = ((ctx->width * ctx->height * 3) >> 1) * (ctx->frambufCnt + 1);
	if ( Get_MfcFramBufAddr(ctx, nFramBufSize) == FALSE ) {
		LOG_MSG(LOG_ERROR, "MFCInst_Enc_Init", "Initialization of MFC Instance failed!\r\n");
		LOG_MSG(LOG_ERROR, "MFCInst_Enc_Init", "MFC Instance init failed! (required frame buffer size = %d)\r\n", nFramBufSize);
		return MFCINST_ERR_ETC;
	}
	ctx->framBufAllocated = 1;

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// 5. Set the Parameters in the PARA_BUF for SET_FRAME_BUF command. //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	// Buffer address of Y, Cb, Cr will be set in PARAM_BUF.
	pPARAM_BUF = (unsigned char *)GetParamBufVirAddr();
	frame_size = ctx->width * ctx->height;
	for (i=0; i < ctx->frambufCnt; i++)
	{
		*((int *) (pPARAM_BUF + i*3*4))      = ctx->phyadrFramBuf + (i + 1) * ((frame_size * 3) >> 1);
		*((int *) (pPARAM_BUF + i*3*4 + 4))  = ctx->phyadrFramBuf + (i + 1) * ((frame_size * 3) >> 1) + frame_size;
		*((int *) (pPARAM_BUF + i*3*4 + 8))  = ctx->phyadrFramBuf + (i + 1) * ((frame_size * 3) >> 1) + frame_size + (frame_size >> 2);
	}


	////////////////////////////////////////
	//                                    //
	// 6. Issue the SET_FRAME_BUF command //
	//                                    //
	////////////////////////////////////////
	// 'SET_FRAME_BUF_NUM' must be greater than or equal to RET_DEC_SEQ_FRAME_NEED_COUNT.
	pPARAM_SET_FRAME_BUF = (S3C6400_MFC_PARAM_REG_SET_FRAME_BUF *)  MfcGetCmdParamRegion();
	pPARAM_SET_FRAME_BUF->SET_FRAME_BUF_NUM    = ctx->frambufCnt;
	pPARAM_SET_FRAME_BUF->SET_FRAME_BUF_STRIDE = ctx->buf_width;
	MfcIssueCmd(ctx->inst_no, ctx->codec_mode, SET_FRAME_BUF);


	////////////////////////////
	///    STATE changing    ///
	////////////////////////////
	// state change to MFCINST_STATE_ENC_INITIALIZED
	MFCINST_STATE_TRANSITION(ctx, MFCINST_STATE_ENC_INITIALIZED);

	ctx->multiple_slice 	= 0;


	return MFCINST_RET_OK;
}


void MFCInst_Get_Frame_size(MFCInstCtx *ctx, int arg)
{
	MFC_DECODED_FRAME_INFO tmp;

	tmp.width	= ctx->width;
	tmp.height	= ctx->height;

	Copy_To_User( (MFC_DECODED_FRAME_INFO *)arg, &tmp, sizeof(MFC_DECODED_FRAME_INFO) );
}



//
// Function Name: MFCInst_Decode
// Description
//      This function decodes the input stream and put the decoded frame into the FRAM_BUF.
// Parameters
//      ctx[IN]: MFCInstCtx
//      strm_leng[IN]: stream size
//
int MFCInst_Decode(MFCInstCtx *ctx, unsigned long strm_leng)
{
	volatile S3C6400_MFC_PARAM_REG_DEC_PIC_RUN	*pPARAM_DEC_PIC_RUN;
	S3C6400_MFC_SFR						*mfc_sfr;

#if (MFC_ROTATE_ENABLE == 1)
	int              frame_size;	// width * height
#endif
	int				frm_size;

	////////////////////////////
	///    STATE checking    ///
	////////////////////////////
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_DELETED)) {
		LOG_MSG(LOG_ERROR, "MFCInst_Decode", "MFC instance is deleted.\r\n");
		return MFCINST_ERR_STATE_DELETED;
	}
	if (MFCINST_STATE_PWR_OFF_FLAG_CHECK(ctx)) {
		LOG_MSG(LOG_ERROR, "MFCInst_Decode", "MFC instance is in Power-Off state.\r\n");
		return MFCINST_ERR_STATE_POWER_OFF;
	}
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_CREATED)) {
		LOG_MSG(LOG_ERROR, "MFCInst_Decode", "MFC instance is not initialized.\r\n");
		return MFCINST_ERR_STATE_CHK;
	}


	pPARAM_DEC_PIC_RUN = (S3C6400_MFC_PARAM_REG_DEC_PIC_RUN *)  MfcGetCmdParamRegion();

	// (strm_leng > 0) means that the video stream is waiting for being decoded in the STRM_LINE_BUF.
	// Otherwise, no more video streams are available and the decode command will flush the decoded YUV data
	// which are postponed because of B-frame (VC-1) or reordering (H.264).
	if (strm_leng > 0) {

		mfc_sfr = (S3C6400_MFC_SFR *) GetMfcSfrVirAddr();
		mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_RD_PTR = ctx->phyadrStrmBuf;
		mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_WR_PTR = ctx->phyadrStrmBuf  +  strm_leng;


	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// 13. Set the Parameters in the PARAM_BUF for PIC_RUN command. 	//
	//                                                                  //
	//////////////////////////////////////////////////////////////////////

	pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_MODE = ctx->PostRotMode;
#if (MFC_ROTATE_ENABLE == 1)
	if (ctx->PostRotMode & 0x0010) {	// The bit of 'PostRotEn' is 1.

		frame_size = ctx->buf_width * ctx->buf_height;

		pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_ADDR_Y  = ctx->phyadrFramBuf + ctx->frambufCnt * ((frame_size * 3) >> 1);
		pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_ADDR_CB = pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_ADDR_Y + frame_size;
		pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_ADDR_CR = pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_ADDR_Y + frame_size + (frame_size >> 2);

		// Rotate Angle
		switch (ctx->PostRotMode & 0x0003) {
		case 0:	// 0   degree counterclockwise rotate
		case 2:	// 180 degree counterclockwise rotate
			pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_STRIDE  = ctx->buf_width;
			break;

		case 1:	// 90  degree counterclockwise rotate
		case 3:	// 270 degree counterclockwise rotate
			pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_STRIDE  = ctx->buf_height;
			break;
		}
	}
#endif

#if 1
	// DEC_PIC_OPTION was newly added for MP4ASP.
	frm_size = ctx->buf_width * ctx->buf_height;
	pPARAM_DEC_PIC_RUN->DEC_PIC_OPTION       = 0x7;//(ctx->dec_pic_option & 0x00000007);
    pPARAM_DEC_PIC_RUN->DEC_PIC_MV_ADDR      = ctx->mv_mbyte_addr;
    pPARAM_DEC_PIC_RUN->DEC_PIC_MBTYPE_ADDR  = pPARAM_DEC_PIC_RUN->DEC_PIC_MV_ADDR + 25920;// '25920' is the maximum MV size (=45*36*16)
#endif

	pPARAM_DEC_PIC_RUN->DEC_PIC_BB_START    = ctx->phyadrStrmBuf & 0xFFFFFFFC;
	pPARAM_DEC_PIC_RUN->DEC_PIC_START_BYTE  = ctx->phyadrStrmBuf & 0x00000003;
	pPARAM_DEC_PIC_RUN->DEC_PIC_CHUNK_SIZE  = strm_leng;

	}
	else {
		pPARAM_DEC_PIC_RUN->DEC_PIC_CHUNK_SIZE  = strm_leng;

		MfcSetEos(1);

	}

	////////////////////////////////////////
	//                                    //
	// 14. Issue the PIC_RUN command	  //
	//                                    //
	////////////////////////////////////////
	if (!MfcIssueCmd(ctx->inst_no, ctx->codec_mode, PIC_RUN)) {
		return MFCINST_ERR_DEC_PIC_RUN_CMD_FAIL;
	}
	if (pPARAM_DEC_PIC_RUN->RET_DEC_PIC_SUCCESS != 1) {
        LOG_MSG(LOG_WARNING, "Decode", "RET_DEC_PIC_SUCCESS is not value of 1(=SUCCESS).\r\n", pPARAM_DEC_PIC_RUN->RET_DEC_PIC_SUCCESS);
    }
	if (pPARAM_DEC_PIC_RUN->RET_DEC_PIC_IDX > 30) {

		if (pPARAM_DEC_PIC_RUN->RET_DEC_PIC_IDX == 0xFFFFFFFF) {		// RET_DEC_PIC_IDX == -1
			LOG_MSG(LOG_WARNING, "Decode", "End of Stream.\r\n");
			return MFCINST_ERR_DEC_EOS;
		}
		else if (pPARAM_DEC_PIC_RUN->RET_DEC_PIC_IDX == 0xFFFFFFFD) {	// RET_DEC_PIC_IDX == -3
			LOG_MSG(LOG_TRACE, "Decode", "No picture to be displayed.\r\n");
		}
		else {
			LOG_MSG(LOG_ERROR, "Decode", "Decoding Fail, ret = %d\r\n", pPARAM_DEC_PIC_RUN->RET_DEC_PIC_IDX);
			return MFCINST_ERR_DEC_DECODE_FAIL_ETC;
		}
	}

	ctx->idx = pPARAM_DEC_PIC_RUN->RET_DEC_PIC_IDX;
#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))
    ctx->RET_DEC_PIC_RUN_BAK_BYTE_CONSUMED          = pPARAM_DEC_PIC_RUN->RET_DEC_PIC_BCNT;
    ctx->RET_DEC_PIC_RUN_BAK_MP4ASP_FCODE           = pPARAM_DEC_PIC_RUN->RET_DEC_PIC_FCODE_FWD;
    ctx->RET_DEC_PIC_RUN_BAK_MP4ASP_TIME_BASE_LAST  = pPARAM_DEC_PIC_RUN->RET_DEC_PIC_TIME_BASE_LAST;
    ctx->RET_DEC_PIC_RUN_BAK_MP4ASP_NONB_TIME_LAST  = pPARAM_DEC_PIC_RUN->RET_DEC_PIC_NONB_TIME_LAST;
    ctx->RET_DEC_PIC_RUN_BAK_MP4ASP_MP4ASP_TRD      = pPARAM_DEC_PIC_RUN->RET_DEC_PIC_TRD;
#endif

#ifdef CNM_PM	
	if( pPARAM_DEC_PIC_RUN->RET_DEC_PIC_ERR_MB_NUM > 0 )
		RETAILMSG( 1, (_T("Report MB Error num=%d\r\n"), pPARAM_DEC_PIC_RUN->RET_DEC_PIC_ERR_MB_NUM ) );
#endif	

	////////////////////////////
	///    STATE changing    ///
	////////////////////////////
	// state change to MFCINST_STATE_DEC_PIC_RUN_LINE_BUF
	MFCINST_STATE_TRANSITION(ctx, MFCINST_STATE_DEC_PIC_RUN_LINE_BUF);

	return MFCINST_RET_OK;
}


//
// Function Name: MFCInst_Decode_Stream
// Description
//      This function decodes the input stream and put the decoded frame into the FRAM_BUF.
// Parameters
//      ctx[IN]: MFCInstCtx
//      strm_leng[IN]: stream size
//
int MFCInst_Decode_Stream(MFCInstCtx *ctx, unsigned long strm_leng)
{
	volatile S3C6400_MFC_PARAM_REG_DEC_PIC_RUN	*pPARAM_DEC_PIC_RUN;
	S3C6400_MFC_SFR						*mfc_sfr;
	int									garbage_size = 0;
	unsigned int 						offset = 0;
	
#if (MFC_ROTATE_ENABLE == 1)
	int              frame_size;	// width * height
#endif
	
	////////////////////////////
	///    STATE checking    ///
	////////////////////////////
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_DELETED)) {
		LOG_MSG(LOG_ERROR, "Decode_Stream", "MFC instance is deleted.\r\n");
		return MFCINST_ERR_STATE_DELETED;
	}
	if (MFCINST_STATE_PWR_OFF_FLAG_CHECK(ctx)) {
		LOG_MSG(LOG_ERROR, "Decode_Stream", "MFC instance is in Power-Off state.\r\n");
		return MFCINST_ERR_STATE_POWER_OFF;
	}
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_CREATED)) {
		LOG_MSG(LOG_ERROR, "Decode_Stream", "MFC instance is not initialized.\r\n");
		return MFCINST_ERR_STATE_CHK;
	}
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_DEC_PIC_END_RING_BUF)) {
		LOG_MSG(LOG_ERROR, "Decode_Stream", "Decode cannot executed after DEC_PIC_END state.\r\n");
		return MFCINST_ERR_STATE_CHK;
	}
	
	if (strm_leng && !MFCINST_STATE_BUF_FILL_REQ_CHECK(ctx)) {
		LOG_MSG(LOG_ERROR, "Decode_Stream", "Buffer should not be filled.\r\n");
		return MFCINST_ERR_STATE_CHK;
	}


	////////////////////////////
	///    STATE changing    ///
	////////////////////////////
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_DEC_INITIALIZED))
		MFCINST_STATE_TRANSITION(ctx, MFCINST_STATE_DEC_PIC_RUN_RING_BUF);


	if (strm_leng) {

		// If the input stream length is less than 'MFC_RING_BUF_PARTUNIT_SIZE',
		// this block of input stream is considered as the last unit.
		// MFC instance will not accept any more input stream data,
		// when the state is changed to MFCINST_STATE_DEC_PIC_RUN_RING_BUF_LAST_UNITS.
		// The state is now changing to MFCINST_STATE_DEC_PIC_RUN_RING_BUF_LAST_UNITS.
		if (strm_leng < MFC_RING_BUF_PARTUNIT_SIZE) {
			if (strm_leng & 0x1FF) {
				garbage_size = strm_leng;
				strm_leng = (strm_leng + 512) & 0xFFFFFE00;
				garbage_size = strm_leng - garbage_size;
			}
			MFCINST_STATE_TRANSITION(ctx, MFCINST_STATE_DEC_PIC_RUN_RING_BUF_LAST_UNITS);
			MfcSetEos(0);
		}
		else if (strm_leng == MFC_RING_BUF_PARTUNIT_SIZE) {
			// state change to MFCINST_STATE_DEC_PIC_RUN_RING_BUF
			MFCINST_STATE_TRANSITION(ctx, MFCINST_STATE_DEC_PIC_RUN_RING_BUF);
		}
		else {	// strm_leng > MFC_RING_BUF_PARTUNIT_SIZE   => ERROR !!
			LOG_MSG(LOG_ERROR, "Decode_Stream", "Size of buffer fill is more than requested.\r\n");
			return MFCINST_ERR_DEC_BUF_FILL_SIZE_WRONG;
		}

		MFCINST_STATE_BUF_FILL_REQ_CLEAR(ctx);
	}


	// Moving the BITS_WR_PTR value by the amount of strm_leng
	mfc_sfr = (S3C6400_MFC_SFR *) GetMfcSfrVirAddr();
	mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_WR_PTR += strm_leng;
	if (strm_leng > 0 &&
		((mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_WR_PTR - ctx->phyadrStrmBuf)
			>=  MFC_RING_BUF_SIZE)) {

		mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_WR_PTR -= MFC_RING_BUF_SIZE;	
	}

	if ( (strm_leng > 0) && (strm_leng < MFC_RING_BUF_PARTUNIT_SIZE) ) {
		mfc_sfr = (S3C6400_MFC_SFR *) GetMfcSfrVirAddr();
		offset = mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_WR_PTR - GetDataBufPhyAddr();
		Mem_Set((char *)(GetDataBufVirAddr() + offset - garbage_size), 0, garbage_size);
	}

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// 13. Set the Parameters in the PARAM_BUF for PIC_RUN command. 	//
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	pPARAM_DEC_PIC_RUN = (S3C6400_MFC_PARAM_REG_DEC_PIC_RUN *)  MfcGetCmdParamRegion();
	pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_MODE = ctx->PostRotMode;

#if (MFC_ROTATE_ENABLE == 1)
	if (ctx->PostRotMode & 0x0010) {	// The bit of 'PostRotEn' is 1.

		frame_size = ctx->buf_width * ctx->buf_height;

		pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_ADDR_Y  = ctx->phyadrFramBuf + ctx->frambufCnt * ((frame_size * 3) >> 1);
		pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_ADDR_CB = pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_ADDR_Y + frame_size;
		pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_ADDR_CR = pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_ADDR_Y + frame_size + (frame_size >> 2);

		// Rotate Angle
		switch (ctx->PostRotMode & 0x0003) {
		case 0:	// 0   degree counterclockwise rotate
		case 2:	// 180 degree counterclockwise rotate
			pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_STRIDE  = ctx->buf_width;
			break;

		case 1:	// 90  degree counterclockwise rotate
		case 3:	// 270 degree counterclockwise rotate
			pPARAM_DEC_PIC_RUN->DEC_PIC_ROT_STRIDE  = ctx->buf_height;
			break;
		}
	}
#endif

	////////////////////////////////////////
	//                                    //
	// 14. Issue the PIC_RUN command	  //
	//                                    //
	////////////////////////////////////////
	if (!MfcIssueCmd(ctx->inst_no, ctx->codec_mode, PIC_RUN)) {
		LOG_MSG(LOG_TRACE, "Decode_Stream", "Frame num : %d, pPARAM_DEC_PIC_RUN->RET_DEC_PIC_IDX : %d\n", pPARAM_DEC_PIC_RUN->RET_DEC_PIC_FRAME_NUM, pPARAM_DEC_PIC_RUN->RET_DEC_PIC_IDX);
		return MFCINST_ERR_DEC_PIC_RUN_CMD_FAIL;	// BUFFER empty or full interrupt is raised.
	}



	if (pPARAM_DEC_PIC_RUN->RET_DEC_PIC_IDX > 30) {
		if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_DEC_PIC_RUN_RING_BUF_LAST_UNITS)
			&& (pPARAM_DEC_PIC_RUN->RET_DEC_PIC_IDX == 0xFFFFFFFF)) {	// RET_DEC_PIC_IDX == -1

			LOG_MSG(LOG_WARNING, "Decode_Stream", "End of Stream.\r\n");
			return MFCINST_ERR_DEC_EOS;
		}
		else if (pPARAM_DEC_PIC_RUN->RET_DEC_PIC_IDX == 0xFFFFFFFD) {	// RET_DEC_PIC_IDX == -3
			LOG_MSG(LOG_TRACE, "Decode_Stream", "No picture to be displayed.\r\n");
//			return MFCINST_ERR_DEC_DECODE_NO_DISP;
		}
		else {
			LOG_MSG(LOG_ERROR, "Decode_Stream", "Decoding Fail, ret = %d\r\n", pPARAM_DEC_PIC_RUN->RET_DEC_PIC_IDX);
			return MFCINST_ERR_DEC_DECODE_FAIL_ETC;
		}
	}

	ctx->frame_num = pPARAM_DEC_PIC_RUN->RET_DEC_PIC_FRAME_NUM;
	ctx->idx       = pPARAM_DEC_PIC_RUN->RET_DEC_PIC_IDX;

#ifdef CNM_PM
	if( pPARAM_DEC_PIC_RUN->RET_DEC_PIC_ERR_MB_NUM > 0 )
		RETAILMSG( 1, (_T("Report2 MB Error num=%d\r\n"), pPARAM_DEC_PIC_RUN->RET_DEC_PIC_ERR_MB_NUM ) );
#endif

	return MFCINST_RET_OK;
}

// modified by RainAde for composer interface
//int MFCInst_Encode(MFCInstCtx *ctx, int *enc_data_size, int *header_size, int *header0, int *header1)
// modified by RainAde for header type of mpeg4 (+ VOS/VO)
int MFCInst_Encode(MFCInstCtx *ctx, int *enc_data_size, int *header_size, int *header0, int *header1, int *header2)
{
	volatile S3C6400_MFC_PARAM_REG_ENC_PIC_RUN	*pPARAM_ENC_PIC_RUN;
	S3C6400_MFC_SFR						*mfc_sfr;

	int                                hdr_size;

	// added by RainAde for composer interface
	int                                hdr_size0 = 0, hdr_size1 = 0;	
	// modified by RainAde for header type of mpeg4 (+ VOS/VO)
	int                                hdr_size2 = 0;	

	unsigned char                      *hdr_buf_tmp=NULL;

	////////////////////////////
	///    STATE checking    ///
	////////////////////////////
	if (!MFCINST_STATE_CHECK(ctx, MFCINST_STATE_ENC_INITIALIZED) && !MFCINST_STATE_CHECK(ctx, MFCINST_STATE_ENC_PIC_RUN_LINE_BUF)) {

		LOG_MSG(LOG_ERROR, "MFCInst_Encode", "MFC Encoder instance is not initialized or not using the line buffer.\r\n");
		return MFCINST_ERR_STATE_CHK;
	}

	mfc_sfr = (S3C6400_MFC_SFR *) GetMfcSfrVirAddr();


	// The 1st call of this function (MFCInst_Encode) will generate the stream header (mpeg4: VOL, h264: SPS/PPS).
	if (MFCINST_STATE_CHECK(ctx, MFCINST_STATE_ENC_INITIALIZED)) {

#if 0
        // modified by RainAde for composer interface
		if (ctx->codec_mode == MP4_ENC) {

			//  ENC_HEADER command  //
			MFCInst_EncHeader(ctx, 0, 0, ctx->phyadrStrmBuf, ctx->nStrmBufSize, &hdr_size);	// VOL
			
			// Backup the stream header in the temporary header buffer.
			hdr_buf_tmp = (unsigned char *) Mem_Alloc(hdr_size);
			Mem_Cpy(hdr_buf_tmp, ctx->pStrmBuf, hdr_size);
			
                         // from 2.8.5
			//dmac_clean_range(ctx->pStrmBuf, (ctx->pStrmBuf + hdr_size));
			//dmac_flush_range(ctx->pStrmBuf, (ctx->pStrmBuf + hdr_size));
		}
#else
		// modified by RainAde for header type of mpeg4 (+ VOS/VO)
		if (ctx->codec_mode == MP4_ENC) {

			//  ENC_HEADER command  //
			MFCInst_EncHeader(ctx, 1, 0, ctx->phyadrStrmBuf, ctx->nStrmBufSize, &hdr_size1);	// VOS (Visual Object Sequence)
			MFCInst_EncHeader(ctx, 2, 0, ctx->phyadrStrmBuf + (hdr_size1 + 3), ctx->nStrmBufSize-(hdr_size1+3), &hdr_size2);	// VO (Visual Object)
			MFCInst_EncHeader(ctx, 0, 0, ctx->phyadrStrmBuf + (hdr_size1 + 3) + (hdr_size2 + 3), ctx->nStrmBufSize-(hdr_size1+3) - (hdr_size2+3), &hdr_size0);	// VOL (Video Object Layer)

			LOG_MSG(LOG_TRACE, "MFCInst_Encode", "VOS] hdr_size1=%d\r\n", hdr_size1);
			LOG_MSG(LOG_TRACE, "MFCInst_Encode", "VO] hdr_size2=%d\r\n", hdr_size2);	
			LOG_MSG(LOG_TRACE, "MFCInst_Encode", "VOL] hdr_size0=%d\r\n", hdr_size0);
			
			// Backup the stream header in the temporary header buffer.
			hdr_buf_tmp = (unsigned char *) Mem_Alloc(hdr_size1 + 3 + hdr_size2 + 3 +hdr_size0);

			Mem_Cpy(hdr_buf_tmp, ctx->pStrmBuf, hdr_size1);

                         // from 2.8.5
			//dmac_clean_range(ctx->pStrmBuf, (ctx->pStrmBuf + hdr_size0));
			//dmac_flush_range(ctx->pStrmBuf, (ctx->pStrmBuf + hdr_size0));
			Mem_Cpy(hdr_buf_tmp + hdr_size1, (unsigned char *)((unsigned int)(ctx->pStrmBuf + (hdr_size1 + 3)) & 0xFFFFFFFC), hdr_size2);

			Mem_Cpy(hdr_buf_tmp + hdr_size1 + hdr_size2, (unsigned char *)((unsigned int)(ctx->pStrmBuf + (hdr_size1 + 3) + (hdr_size2 + 3)) & 0xFFFFFFFC), hdr_size0);

			hdr_size = hdr_size1 + hdr_size2 + hdr_size0;

                        // from 2.8.5
			//dmac_clean_range(ctx->pStrmBuf, (ctx->pStrmBuf + hdr_size));
			//dmac_flush_range(ctx->pStrmBuf, (ctx->pStrmBuf + hdr_size));
		}
#endif
		else if (ctx->codec_mode == AVC_ENC) {


			//  ENC_HEADER command  //
			MFCInst_EncHeader(ctx, 0, 0, ctx->phyadrStrmBuf, ctx->nStrmBufSize, &hdr_size0);	// SPS
			MFCInst_EncHeader(ctx, 1, 0, ctx->phyadrStrmBuf + (hdr_size0 + 3), ctx->nStrmBufSize-(hdr_size0+3), &hdr_size1);	// PPS

			LOG_MSG(LOG_TRACE, "MFCInst_Encode", "SPS] hdr_size0=%d\r\n", hdr_size0);
			LOG_MSG(LOG_TRACE, "MFCInst_Encode", "PPS] hdr_size1=%d\r\n", hdr_size1);
			
			// Backup the stream header in the temporary header buffer.
			hdr_buf_tmp = (unsigned char *) Mem_Alloc(hdr_size0 + 3 + hdr_size1);
			Mem_Cpy(hdr_buf_tmp, ctx->pStrmBuf, hdr_size0);

                         // from 2.8.5
			//dmac_clean_range(ctx->pStrmBuf, (ctx->pStrmBuf + hdr_size0));
			//dmac_flush_range(ctx->pStrmBuf, (ctx->pStrmBuf + hdr_size0));
			Mem_Cpy(hdr_buf_tmp + hdr_size0, (unsigned char *)((unsigned int)(ctx->pStrmBuf + (hdr_size0 + 3)) & 0xFFFFFFFC), hdr_size1);

			hdr_size = hdr_size0 + hdr_size1;

                        // from 2.8.5
			//dmac_clean_range(ctx->pStrmBuf, (ctx->pStrmBuf + hdr_size));
			//dmac_flush_range(ctx->pStrmBuf, (ctx->pStrmBuf + hdr_size));
		}
	}


	// SEI message with recovery point
	if ((ctx->enc_pic_option & 0x0F000000) && (ctx->codec_mode == AVC_ENC))
	{
		LOG_MSG(LOG_TRACE, "MFCInst_Encode", "SEIr\n");

		//  ENC_HEADER command  //
		MFCInst_EncHeader(ctx, 4, ((ctx->enc_pic_option & 0x0F000000) >> 24), ctx->phyadrStrmBuf, ctx->nStrmBufSize, &hdr_size);	// SEI
		// Backup the stream header in the temporary header buffer.
		hdr_buf_tmp = (unsigned char *) Mem_Alloc(hdr_size);
		Mem_Cpy(hdr_buf_tmp, ctx->pStrmBuf, hdr_size);

	        // from 2.8.5	
                //dmac_clean_range(ctx->pStrmBuf, (ctx->pStrmBuf + hdr_size));
		//dmac_flush_range(ctx->pStrmBuf, (ctx->pStrmBuf + hdr_size));
	}

	/*
	 * Set the address of each component of YUV420
	 */
	pPARAM_ENC_PIC_RUN = (S3C6400_MFC_PARAM_REG_ENC_PIC_RUN *) MfcGetCmdParamRegion();
	pPARAM_ENC_PIC_RUN->ENC_PIC_SRC_ADDR_Y	= ctx->phyadrFramBuf;
	pPARAM_ENC_PIC_RUN->ENC_PIC_SRC_ADDR_CB	= ctx->phyadrFramBuf  +   ctx->buf_width * ctx->height;
	pPARAM_ENC_PIC_RUN->ENC_PIC_SRC_ADDR_CR	= ctx->phyadrFramBuf  + ((ctx->buf_width * ctx->height * 5) >> 2);

	// RainAde : for setting Picture Quantization Step on RC disable
	pPARAM_ENC_PIC_RUN->ENC_PIC_QS = ctx->pictureQs;
	
	pPARAM_ENC_PIC_RUN->ENC_PIC_ROT_MODE	= 0;
	pPARAM_ENC_PIC_RUN->ENC_PIC_OPTION		= (ctx->enc_pic_option & 0x0000FFFF);
	pPARAM_ENC_PIC_RUN->ENC_PIC_BB_START    = ctx->phyadrStrmBuf;
	pPARAM_ENC_PIC_RUN->ENC_PIC_BB_SIZE     = ctx->nStrmBufSize;


	if (!MfcIssueCmd(ctx->inst_no, ctx->codec_mode, PIC_RUN)) {
		return MFCINST_ERR_ENC_PIC_RUN_CMD_FAIL;
	}
// RainAde for Encoding pic type (I/P)
	ctx->enc_pic_type = pPARAM_ENC_PIC_RUN->RET_ENC_PIC_TYPE;

	ctx->enc_pic_option = 0;	// Reset the encoding picture option at every picture
	ctx->idx = 0;

	*enc_data_size = mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_WR_PTR - ctx->phyadrStrmBuf;	// calculte encoded data size
	*header_size   = 0;

	if (hdr_buf_tmp) {
		memmove(ctx->pStrmBuf + hdr_size, ctx->pStrmBuf, *enc_data_size);

                // from 2.8.5
		//dmac_clean_range(ctx->pStrmBuf, (ctx->pStrmBuf + *enc_data_size));
		//dmac_flush_range(ctx->pStrmBuf, (ctx->pStrmBuf + *enc_data_size));
		Mem_Cpy(ctx->pStrmBuf, hdr_buf_tmp, hdr_size);

                // from 2.8.5		
                //dmac_clean_range(ctx->pStrmBuf, (ctx->pStrmBuf + *enc_data_size));
		//dmac_flush_range(ctx->pStrmBuf, (ctx->pStrmBuf + *enc_data_size));
		Mem_Free(hdr_buf_tmp);

		*enc_data_size += hdr_size;
		*header_size    = hdr_size;

		// modified by RainAde for composer interface
		*header0 = hdr_size0;
		*header1 = hdr_size1;
		// modified by RainAde for header type of mpeg4 (+ VOS/VO)		
		*header2 = hdr_size2;		
		
	}

	////////////////////////////
	///    STATE changing    ///
	////////////////////////////
	// state change to MFCINST_STATE_ENC_PIC_RUN_LINE_BUF
	MFCINST_STATE_TRANSITION(ctx, MFCINST_STATE_ENC_PIC_RUN_LINE_BUF);

	
	return MFCINST_RET_OK;
}

// hdr_code == 0: SPS
// hdr_code == 1: PPS
// hdr_code == 4: SEI

int MFCInst_EncHeader(MFCInstCtx *ctx, int hdr_code, int hdr_num, unsigned int outbuf_physical_addr, int outbuf_size, int *hdr_size)
{
	volatile S3C6400_MFC_PARAM_REG_ENC_HEADER	*pPARAM_ENC_HEADER;
	S3C6400_MFC_SFR						*mfc_sfr;
	
	if (!MFCINST_STATE_CHECK(ctx, MFCINST_STATE_ENC_INITIALIZED) && !MFCINST_STATE_CHECK(ctx, MFCINST_STATE_ENC_PIC_RUN_LINE_BUF)) {

		LOG_MSG(LOG_ERROR, "MFCInst_EncHeader", "MFC Encoder instance is not initialized or not using the line buffer.\r\n");
		return MFCINST_ERR_STATE_CHK;
	}

	if ((ctx->codec_mode != MP4_ENC) && (ctx->codec_mode != AVC_ENC)) {
		return MFCINST_ERR_WRONG_CODEC_MODE;
	}


	/*
	 * Set the address of each component of YUV420
	 */
	pPARAM_ENC_HEADER = (S3C6400_MFC_PARAM_REG_ENC_HEADER *) MfcGetCmdParamRegion();
	pPARAM_ENC_HEADER->ENC_HEADER_CODE      = hdr_code;
	pPARAM_ENC_HEADER->ENC_HEADER_BB_START  = outbuf_physical_addr & 0xFFFFFFFC;
	pPARAM_ENC_HEADER->ENC_HEADER_BB_SIZE   = outbuf_size; //ctx->nStrmBufSize;
	if (hdr_code == 4)	// SEI recovery point
		pPARAM_ENC_HEADER->ENC_HEADER_NUM   = hdr_num; // recovery point value

	mfc_sfr = (S3C6400_MFC_SFR *) GetMfcSfrVirAddr();


	if (!MfcIssueCmd(ctx->inst_no, ctx->codec_mode, ENC_HEADER)) {
		return MFCINST_ERR_ENC_HEADER_CMD_FAIL;
	}

	// RainAde : timing issue
	mdelay(1);
	*hdr_size = mfc_sfr->BIT_STR_BUF_RW_ADDR[ctx->inst_no].BITS_WR_PTR - pPARAM_ENC_HEADER->ENC_HEADER_BB_START;

	return MFCINST_RET_OK;
	
}


int MFCInst_EncParamChange(MFCInstCtx *ctx, unsigned int param_change_enable, unsigned int param_change_val)
{
	volatile S3C6400_MFC_PARAM_REG_ENC_PARAM_CHANGE    *pPARAM_ENC_PARAM_CHANGE;


	int              num_mbs;		// Number of MBs
	int              slices_mb;		// MB number of slice (only if SLICE_MODE_MULTIPLE is selected.)


	pPARAM_ENC_PARAM_CHANGE = (S3C6400_MFC_PARAM_REG_ENC_PARAM_CHANGE *) MfcGetCmdParamRegion();

	Mem_Set((void *)pPARAM_ENC_PARAM_CHANGE, 0, sizeof(S3C6400_MFC_PARAM_REG_ENC_PARAM_CHANGE));


	pPARAM_ENC_PARAM_CHANGE->ENC_PARAM_CHANGE_ENABLE      = param_change_enable;

	// GOP_NUM
	if (param_change_enable == (1 << 0)) {
		if (param_change_val > 60) {
			LOG_MSG(LOG_ERROR, "MFCInst_EncParamChange", "MFC Encoder Parameter change value is invalid.\r\n");
			return MFCINST_ERR_ENC_PARAM_CHANGE_INVALID_VALUE;
		}
		pPARAM_ENC_PARAM_CHANGE->ENC_PARAM_CHANGE_GOP_NUM        = param_change_val;
	}
	// INTRA_QP
	else if (param_change_enable == (1 << 1)) {
		if (((ctx->codec_mode == MP4_DEC || ctx->codec_mode == H263_DEC) && (param_change_val == 0 || param_change_val > 31))
				|| (ctx->codec_mode == AVC_DEC && param_change_val > 51)) {
			LOG_MSG(LOG_ERROR, "MFCInst_EncParamChange", "MFC Encoder Parameter change value is invalid.\r\n");
			return MFCINST_ERR_ENC_PARAM_CHANGE_INVALID_VALUE;
		}
		pPARAM_ENC_PARAM_CHANGE->ENC_PARAM_CHANGE_INTRA_QP       = param_change_val;
	}
	// BITRATE
	else if (param_change_enable == (1 << 2)) {
		if (param_change_val > 0x07FFF) {
			LOG_MSG(LOG_ERROR, "MFCInst_EncParamChange", "MFC Encoder Parameter change value is invalid.\r\n");
			return MFCINST_ERR_ENC_PARAM_CHANGE_INVALID_VALUE;
		}
		pPARAM_ENC_PARAM_CHANGE->ENC_PARAM_CHANGE_BITRATE        = param_change_val;
	}
	// F_RATE
	else if (param_change_enable == (1 << 3)) {
		pPARAM_ENC_PARAM_CHANGE->ENC_PARAM_CHANGE_F_RATE         = param_change_val;
	}
	// INTRA_REFRESH
	else if (param_change_enable == (1 << 4)) {
		if (param_change_val > ((ctx->width * ctx->height) >> 8)) {
			LOG_MSG(LOG_ERROR, "MFCInst_EncParamChange", "MFC Encoder Parameter change value is invalid.\r\n");
			return MFCINST_ERR_ENC_PARAM_CHANGE_INVALID_VALUE;
		}
		pPARAM_ENC_PARAM_CHANGE->ENC_PARAM_CHANGE_INTRA_REFRESH  = param_change_val;
	}
	// SLICE_MODE
	else if (param_change_enable == (1 << 5)) {

		// MB size is 16x16. -> width & height are divided by 16 to get number of MBs. (Division by 16 == shift right by 4-bit)
		num_mbs   = (ctx->width >> 4) * (ctx->height >> 4);

		if (param_change_val > 256 || param_change_val > num_mbs) {
			LOG_MSG(LOG_ERROR, "MFCInst_EncParamChange", "MFC Encoder Parameter change value is invalid.\r\n");
			return MFCINST_ERR_ENC_PARAM_CHANGE_INVALID_VALUE;
		}

		if (param_change_val == 0) {
			pPARAM_ENC_PARAM_CHANGE->ENC_PARAM_CHANGE_SLICE_MODE   = SLICE_MODE_ONE;
		}
		else {

			slices_mb = (num_mbs / param_change_val);
			ctx->enc_num_slices = param_change_val;

			pPARAM_ENC_PARAM_CHANGE->ENC_PARAM_CHANGE_SLICE_MODE   = SLICE_MODE_MULTIPLE | (1 << 1) | (slices_mb<< 2);;
		}
	}

	if (!MfcIssueCmd(ctx->inst_no, ctx->codec_mode, ENC_PARAM_CHANGE)) {
		return MFCINST_ERR_ENC_HEADER_CMD_FAIL;
	}

	return MFCINST_RET_OK;
}

