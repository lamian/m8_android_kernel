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
 * This source file is for managing the Frame buffer.
 *
 * @name MFC DRIVER MODULE Module (FramBufMgr.c)
 * @author name(email address)
 * @date 03-28-07
 */

#include "MfcMemory.h"
#include "FramBufMgr.h"
#include "MfcTypes.h"
#include "LogMsg.h"


// The size in bytes of the BUF_SEGMENT.
// The buffers are fragemented into the segment unit of this size.
#define BUF_SEGMENT_SIZE	1024


typedef struct
{
	unsigned char *pBaseAddr;
	int            idx_commit;
} SEGMENT_INFO;


typedef struct
{
	int index_base_seg;
	int num_segs;
} COMMIT_INFO;


static SEGMENT_INFO  *_p_segment_info = NULL;
static COMMIT_INFO   *_p_commit_info  = NULL;


static unsigned char *_pBufferBase  = NULL;
static int            _nBufferSize  = 0;
static int            _nNumSegs		= 0;


//
// int FramBufMgrInit(unsigned char *pBufBase, int nBufSize)
//
// Description
//		This function initializes the MfcFramBufMgr(Buffer Segment Manager)
// Parameters
//		pBufBase [IN]: pointer to the buffer which will be managed by this MfcFramBufMgr functions.
//		nBufSize [IN]: buffer size in bytes
// Return Value
//		1 : Success
//		0 : Fail
//
BOOL FramBufMgrInit(unsigned char *pBufBase, int nBufSize)
{
	int   i;

	// check parameters
	if (pBufBase == NULL || nBufSize == 0)
		return FALSE;


	// 이미 초기화가 되어 있고
	// (1) 초기화 결과와 입력 파라미터 값이 같다면
	//     실수로 이 함수가 두 번 호출된 것이므로 그냥 바로 1을 리턴
	// (2) 같지 않다면, 
	//     Finalize 시킨 후, 다시 재 초기화 한다. 
	if ((_pBufferBase != NULL) && (_nBufferSize != 0)) {
		if ((pBufBase == _pBufferBase) && (nBufSize == _nBufferSize))
			return TRUE;

		FramBufMgrFinal();
	}


	_pBufferBase = pBufBase;
	_nBufferSize = nBufSize;
	_nNumSegs = nBufSize / BUF_SEGMENT_SIZE;

	_p_segment_info = (SEGMENT_INFO *) Mem_Alloc(_nNumSegs * sizeof(SEGMENT_INFO));
	for (i=0; i<_nNumSegs; i++) {
		_p_segment_info[i].pBaseAddr   = pBufBase  +  (i * BUF_SEGMENT_SIZE);
		_p_segment_info[i].idx_commit  = 0;
	}

	_p_commit_info  = (COMMIT_INFO *) Mem_Alloc(_nNumSegs * sizeof(COMMIT_INFO));
	for (i=0; i<_nNumSegs; i++) {
		_p_commit_info[i].index_base_seg  = -1;
		_p_commit_info[i].num_segs        = 0;
	}


	return TRUE;
}


//
// void FramBufMgrFinal()
//
// Description
//		This function finalizes the MfcFramBufMgr(Buffer Segment Manager)
// Parameters
//		None
// Return Value
//		None
//
void FramBufMgrFinal()
{
	if (_p_segment_info != NULL) {
		Mem_Free(_p_segment_info);
		_p_segment_info = NULL;
	}

	if (_p_commit_info != NULL) {
		Mem_Free(_p_commit_info);
		_p_commit_info = NULL;
	}


	_pBufferBase  = NULL;
	_nBufferSize  = 0;
	_nNumSegs       = 0;
}


//
// unsigned char *FramBufMgrCommit(int idx_commit, int commit_size)
//
// Description
//		This function requests the commit for commit_size buffer to be reserved.
// Parameters
//		idx_commit  [IN]: pointer to the buffer which will be managed by this MfcFramBufMgr functions.
//		commit_size [IN]: commit size in bytes
// Return Value
//		NULL : Failed to commit (Wrong parameters, commit_size too big, and so on.)
//		Otherwise it returns the pointer which was committed.
//
unsigned char *FramBufMgrCommit(int idx_commit, int commit_size)
{
	int  i, j;

	int  num_fram_buf_seg;		// 필요한 buffer를 위한 SEGMENT 개수 


	// 초기화 여부 체크 
	if (_p_segment_info == NULL || _p_commit_info == NULL) {
		return NULL;
	}

	// check parameters
	if (idx_commit < 0 || idx_commit >= _nNumSegs)
		return NULL;
	if (commit_size <= 0 || commit_size > _nBufferSize)
		return NULL;

	// COMMIT_INFO 배열에서 idx_commit 번째 값이
	// free인지 이미 commit 되었는지 확인한다. 
	if (_p_commit_info[idx_commit].index_base_seg != -1)
		return NULL;


	// 필요한 FRAM_BUF_SEGMENT 개수를 구한다. 
	// Instance FRAM_BUF의 필요한 크기가 FRAM_BUF_SEGMENT_SIZE 단위를 조금이라도 넘어가면
	// FRAM_BUF_SEGMENT가 통째로 하나 더 필요하게 된다.
	if ((commit_size % BUF_SEGMENT_SIZE) == 0)
		num_fram_buf_seg = commit_size / BUF_SEGMENT_SIZE;
	else
		num_fram_buf_seg = (commit_size / BUF_SEGMENT_SIZE)  +  1;

	for (i=0; i<(_nNumSegs - num_fram_buf_seg); i++) {
		// SEGMENT 중에 commit 안된 것이 있을 때까지 쭉 검색 
		if (_p_segment_info[i].idx_commit != 0)
			continue;

		for (j=0; j<num_fram_buf_seg; j++) {
			if (_p_segment_info[i+j].idx_commit != 0)
				break;
		}

		// j=0 ~ num_fram_buf_seg 까지 commit된 SEGMENT가 없다면 
		// 이 부분을 commit한다. 
		if (j == num_fram_buf_seg) {

			for (j=0; j<num_fram_buf_seg; j++) {
				_p_segment_info[i+j].idx_commit = 1;
			}

			_p_commit_info[idx_commit].index_base_seg  = i;
			_p_commit_info[idx_commit].num_segs        = num_fram_buf_seg;

			return _p_segment_info[i].pBaseAddr;
		}
		else
		{
			// instance buffer들 사이에 빈공간이 발생했을때, 또다른 instance를 위해 버퍼를 할당할
			// 크기가 되지 않으면 그 빈공간을 건너뛰고, 빈 버퍼를 찾는다. 
			i = i + j - 1;
		}
	}

	return NULL;
}


//
// void FramBufMgrFree(int idx_commit)
//
// Description
//		This function frees the committed region of buffer.
// Parameters
//		idx_commit  [IN]: pointer to the buffer which will be managed by this MfcFramBufMgr functions.
// Return Value
//		None
//
void FramBufMgrFree(int idx_commit)
{
	int  i;

	int  index_base_seg;		// 해당 commmit 부분의 base segment의 index
	int  num_fram_buf_seg;		// 필요한 buffer를 위한 SEGMENT의 개수 


	// 초기화 여부 체크 
	if (_p_segment_info == NULL || _p_commit_info == NULL)
		return;

	// check parameters
	if (idx_commit < 0 || idx_commit >= _nNumSegs)
		return;

	// COMMIT_INFO 배열에서 idx_commit 번째 값이 
	// free인지 이미 commit 되었는지 확인한다 
	if (_p_commit_info[idx_commit].index_base_seg == -1)
		return;


	index_base_seg    =  _p_commit_info[idx_commit].index_base_seg;
	num_fram_buf_seg  =  _p_commit_info[idx_commit].num_segs;

	for (i=0; i<num_fram_buf_seg; i++) {
		_p_segment_info[index_base_seg + i].idx_commit = 0;
	}


	_p_commit_info[idx_commit].index_base_seg  =  -1;
	_p_commit_info[idx_commit].num_segs        =  0;

}




//
// unsigned char *FramBufMgrGetBuf(int idx_commit)
//
// Description
//		This function obtains the committed buffer of 'idx_commit'.
// Parameters
//		idx_commit  [IN]: commit index of the buffer which will be obtained
// Return Value
//		NULL : Failed to get the indicated buffer (Wrong parameters, not committed, and so on.)
//		Otherwise it returns the pointer which was committed.
//
unsigned char *FramBufMgrGetBuf(int idx_commit)
{
	int index_base_seg;

	// 초기화 여부 체크 
	if (_p_segment_info == NULL || _p_commit_info == NULL)
		return NULL;

	// check parameters
	if (idx_commit < 0 || idx_commit >= _nNumSegs)
		return NULL;

	// COMMIT_INFO 배열에서 idx_commit 번째 값이 
	// free인지 이미 commit 되었는지 확인한다. 
	if (_p_commit_info[idx_commit].index_base_seg == -1)
		return NULL;


	index_base_seg  =  _p_commit_info[idx_commit].index_base_seg;

	return _p_segment_info[index_base_seg].pBaseAddr;
}

//
// int FramBufMgrGetBufSize(int idx_commit)
//
// Description
//		This function obtains the size of the committed buffer of 'idx_commit'.
// Parameters
//		idx_commit  [IN]: commit index of the buffer which will be obtained
// Return Value
//		0 : Failed to get the size of indicated buffer (Wrong parameters, not committed, and so on.)
//		Otherwise it returns the size of the buffer.
//		Note that the size is multiples of the BUF_SEGMENT_SIZE.
//
int FramBufMgrGetBufSize(int idx_commit)
{
	// 초기화 여부 체크 
	if (_p_segment_info == NULL || _p_commit_info == NULL)
		return 0;

	// check parameters
	if (idx_commit < 0 || idx_commit >= _nNumSegs)
		return 0;

	// COMMIT_INFO 배열에서 idx_commit 번째 값이 
	// free인지 이미 commit 되었는지 확인한다. 
	if (_p_commit_info[idx_commit].index_base_seg == -1)
		return 0;


	return (_p_commit_info[idx_commit].num_segs * BUF_SEGMENT_SIZE);
}


//
// void FramBufMgrPrintCommitInfo()
//
// Description
//		This function prints the commited information on the console screen.
// Parameters
//		None
// Return Value
//		None
//
void FramBufMgrPrintCommitInfo()
{
	int  i;

	// 초기화 여부 체크 
	if (_p_segment_info == NULL || _p_commit_info == NULL) {
		LOG_MSG(LOG_TRACE, "FramBufMgrPrintCommitInfo", "\n The FramBufMgr is not initialized.\n");
		return;
	}


	for (i=0; i<_nNumSegs; i++) {
		if (_p_commit_info[i].index_base_seg != -1)  {
			LOG_MSG(LOG_TRACE, "FramBufMgrPrintCommitInfo", "\n\nCOMMIT INDEX = [%03d], BASE_SEG_IDX = %d", i, _p_commit_info[i].index_base_seg);
			LOG_MSG(LOG_TRACE, "FramBufMgrPrintCommitInfo", "\nCOMMIT INDEX = [%03d], NUM OF SEGS  = %d", i, _p_commit_info[i].num_segs);
		}
	}
}
