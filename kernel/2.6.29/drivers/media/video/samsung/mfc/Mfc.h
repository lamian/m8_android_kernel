#ifndef __SAMSUNG_SYSLSI_APDEV_MFC_H__
#define __SAMSUNG_SYSLSI_APDEV_MFC_H__

#ifdef __cplusplus
extern "C" {
#endif



//SDRAM buffer control options 
#define STREAM_ENDIAN_LITTLE 			(0<<0)
#define STREAM_ENDIAN_BIG 				(1<<0)
#define BUF_STATUS_FULL_EMPTY_CHECK_BIT (0<<1)
#define BUF_STATUS_NO_CHECK_BIT 		(1<<1)

// FRAME_BUF_CTRL (0x110)
#define FRAME_MEM_ENDIAN_LITTLE 		(0<<0)
#define FRAME_MEM_ENDIAN_BIG 			(1<<0)

// DEC_SEQ_OPTION (0x18c)
#define MP4_DBK_DISABLE 				(0<<0)
#define MP4_DBK_ENABLE 					(1<<0)
#define REORDER_DISABLE 				(0<<1)
#define REORDER_ENABLE 					(1<<1)
#define FILEPLAY_ENABLE 				(1<<2)
#define FILEPLAY_DISABLE 				(0<<2)
#define DYNBUFALLOC_ENABLE 				(1<<3)
#define DYNBUFALLOC_DISABLE 			(0<<3)

// ENC_SEQ_OPTION (0x188)
#define MB_BIT_REPORT_DISABLE			(0<<0)
#define MB_BIT_REPORT_ENABLE			(1<<0)
#define SLICE_INFO_REPORT_DISABLE		(0<<1)
#define SLICE_INFO_REPORT_ENABLE		(1<<1)
#define AUD_DISABLE						(0<<2)
#define AUD_ENABLE						(1<<2)

// ENC_SEQ_SLICE_MODE (0x1A4)
#define SLICE_MODE_ONE					(0<<0)
#define SLICE_MODE_MULTIPLE				(1<<0)

// ENC_SEQ_RC_PARA (0x1AC)
#define RC_DISABLE						(0<<0)	// RC means rate control
#define RC_ENABLE						(1<<0)
#define SKIP_DISABLE					(1<<31)
#define SKIP_ENABLE						(0<<31)

// ENC_SEQ_FMO
#define FMO_DISABLE						(0<<0)
#define FMO_ENABLE						(1<<0)

// ENC_SEQ_COD_STD (0x18C)
#define MPEG4_ENCODE					0
#define H263_ENCODE						1
#define H264_ENCODE						2

// ENC_SEQ_MP4_PARA
#define DATA_PART_DISABLE				(0<<0)
#define DATA_PART_ENABLE				(1<<0)

// ENC_SEQ_263_PARA
#define ANNEX_T_OFF						(0<<0)
#define ANNEX_T_ON						(1<<0)
#define ANNEX_K_OFF						(0<<1)
#define ANNEX_K_ON						(1<<1)
#define ANNEX_J_OFF						(0<<2)
#define ANNEX_J_ON						(1<<2)
#define ANNEX_I_OFF						(0<<3)
#define ANNEX_I_ON						(1<<3)



typedef enum __MFC_CODEC_MODE
{
	MP4_DEC	= 0,
	MP4_ENC	= 1,
	AVC_DEC	= 2,
	AVC_ENC	= 3,
	VC1_DEC	= 4,
	H263_DEC= 5,
	H263_ENC= 6
} MFC_CODECMODE;

typedef enum __MFC_COMMAND
{
	SEQ_INIT      		= 0x01,
	SEQ_END       		= 0x02,
	PIC_RUN       		= 0x03,
	SET_FRAME_BUF 		= 0x04,
	ENC_HEADER 	  		= 0x05,
	ENC_PARA_SET  		= 0x06,
	DEC_PARA_SET  		= 0x07,
	ENC_PARAM_CHANGE 	= 0x09,
	SLEEP            	= 0x0A,
	WAKEUP           	= 0x0B,
	GET_FW_VER    		= 0x0F
} MFC_COMMAND;

typedef struct tagS3C6400_MFC_PARAM_REG_DEC_SEQ_INIT
{
	///////////////////////////////////////
	// INPUT ARGUMENTS common for rnc.dec//
	// (0x180 ~ 0x1A0)					 //
	///////////////////////////////////////

	unsigned int DEC_SEQ_BIT_BUF_ADDR;	// 0x180,
	unsigned int DEC_SEQ_BIT_BUF_SIZE;	// 0x184,
	unsigned int DEC_SEQ_OPTION;		// 0x188,
	unsigned int DEC_SEQ_PRO_BUF;		// 0x18c,
	unsigned int DEC_SEQ_TMP_BUF_1;		// 0x190,
	unsigned int DEC_SEQ_TMP_BUF_2;		// 0x194,
	unsigned int DEC_SEQ_TMP_BUF_3;		// 0x198,
	unsigned int DEC_SEQ_TMP_BUF_4;		// 0x19c,
	unsigned int DEC_SEQ_TMP_BUF_5;		// 0x1a0,
	unsigned int DEC_SEQ_START_BYTE;	// 0x1a4

	unsigned int _reserved[6];		// 0x1b8, 0x1bc


	/////////////////////
	//  OUTPUT RETURN  //
	// (0x1C0 ~ 0x1D4) //
	/////////////////////
	unsigned int RET_SEQ_SUCCESS;			// 0x1c0,
	unsigned int RET_DEC_SEQ_SRC_SIZE;		// 0x1c4,
	unsigned int RET_DEC_SEQ_SRC_FRAME_RATE;	// 0x1c8,
	unsigned int RET_DEC_SEQ_FRAME_NEED_COUNT;	// 0x1cc,
	unsigned int RET_DEC_SEQ_FRAME_DELAY;		// 0x1d0,
	unsigned int RET_DEC_SEQ_INFO;			// 0x1d4,
	unsigned int RET_DEC_SEQ_TIME_RES;			// 0x1d8,
	
} S3C6400_MFC_PARAM_REG_DEC_SEQ_INIT;

typedef struct tagS3C6400_MFC_PARAM_REG_ENC_SEQ_INIT
{
	///////////////////////////////////////
	// INPUT ARGUMENTS common for encdoer//
	// (0x180 ~ 0x1B8, 0x1D0 ~ 0x1DC)	 //
	///////////////////////////////////////

	unsigned int ENC_SEQ_BIT_BUF_ADDR;	// 0x180,
	unsigned int ENC_SEQ_BIT_BUF_SIZE;	// 0x184,
	unsigned int ENC_SEQ_OPTION;		// 0x188,
	unsigned int ENC_SEQ_COD_STD;		// 0x18c,
	unsigned int ENC_SEQ_SRC_SIZE;		// 0x190,
	unsigned int ENC_SEQ_SRC_F_RATE;	// 0x194,
	unsigned int ENC_SEQ_MP4_PARA;		// 0x198,
	unsigned int ENC_SEQ_263_PARA;		// 0x19c,
	unsigned int ENC_SEQ_264_PARA;		// 0x1a0,
	unsigned int ENC_SEQ_SLICE_MODE;	// 0x1a4
	unsigned int ENC_SEQ_GOP_NUM;		// 0x1a8
	unsigned int ENC_SEQ_RC_PARA;		// 0x1ac
	unsigned int ENC_SEQ_RC_BUF_SIZE;	// 0x1b0
	unsigned int ENC_SEQ_INTRA_MB;		// 0x1b4
	unsigned int ENC_SEQ_FMO;			// 0x1b8

	unsigned int _reserved1[1];			// 0x1bc 

	/////////////////////
	//  OUTPUT RETURN  //
	//     (0x1C0)     //
	/////////////////////
	unsigned int RET_ENC_SEQ_SUCCESS;	// 0x1c0


	unsigned int _reserved2[3];			// 0x1c4, 0x1c8, 0x1cc 

	unsigned int ENC_SEQ_TMP_BUF1;		// 0x1d0
	unsigned int ENC_SEQ_TMP_BUF2;		// 0x1d4
	unsigned int ENC_SEQ_TMP_BUF3;		// 0x1d8
	unsigned int ENC_SEQ_TMP_BUF4;		// 0x1dc
} S3C6400_MFC_PARAM_REG_ENC_SEQ_INIT;

typedef struct tagS3C6400_MFC_PARAM_REG_SET_FRAME_BUF
{
	///////////////////////////////////////
	// INPUT ARGUMENTS common for rnc.dec//
	// (0x180 ~ 0x1A0)     		     //
	///////////////////////////////////////

	unsigned int SET_FRAME_BUF_NUM;		// 0x180,
	unsigned int SET_FRAME_BUF_STRIDE;	// 0x184,

/*
	//encoder
	UINT32 ENC_SEQ_SLICE_MODE;		// 0x1a4,
	UINT32 ENC_SEQ_GOP_NUM;			// 0x1a8,
	UINT32 ENC_SEQ_RC_PARA;			// 0x1ac,
	UINT32 ENC_SEQ_RC_BUF_SIZE;		// 0x1b0,
	UINT32 ENC_SEQ_INTRA_MB;		// 0x1b4,
*/

	/////////////////////
	//  OUTPUT RETURN  //
	// (0x1C0 ~ 0x1D4) //
	/////////////////////

} S3C6400_MFC_PARAM_REG_SET_FRAME_BUF;

typedef struct tagS3C6400_MFC_PARAM_REG_DEC_PIC_RUN
{
	///////////////////////////////////////
	// INPUT ARGUMENTS common for rnc.dec//
	// (0x180 ~ 0x1A0)		     //
	///////////////////////////////////////
	unsigned int DEC_PIC_ROT_MODE;		// 0x180, Display frame post-rotator mode
	unsigned int DEC_PIC_ROT_ADDR_Y;	// 0x184, Post-rotated frame store Y address
	unsigned int DEC_PIC_ROT_ADDR_CB;	// 0x188, Post-rotated frame store Cb address
	unsigned int DEC_PIC_ROT_ADDR_CR;	// 0x18c, Post-rotated frame store Cr address
	unsigned int DEC_PIC_DBK_ADDR_Y;	// 0x190, Deblocked frame store Y address
	unsigned int DEC_PIC_DBK_ADDR_CB;	// 0x194, Deblocked frame store Cb address
	unsigned int DEC_PIC_DBK_ADDR_CR;	// 0x198, Deblocked frame store Cr address
	unsigned int DEC_PIC_ROT_STRIDE;	// 0x19c, Post-rotated frame stride
	unsigned int DEC_PIC_OPTION;		// 0x1a0, Decoding option
	unsigned int _reserved1[1];			// 0x1a4
	unsigned int DEC_PIC_CHUNK_SIZE;	// 0x1a8, Frame chunk size
	unsigned int DEC_PIC_BB_START;		// 0x1ac, 4-byte aligned start address of picture stream buffer
	unsigned int DEC_PIC_START_BYTE;	// 0x1b0, Start byte of valid stream data
	unsigned int DEC_PIC_MV_ADDR;		// 0x1b4, Base address for Motion Vector data
	unsigned int DEC_PIC_MBTYPE_ADDR;	// 0x1b8, Base address for MBType data
	
	unsigned int _reserved2[1];			// 0x1bc


	/////////////////////
	//  OUTPUT RETURN  //
	// (0x1C0 ~ 0x1E0) //
	/////////////////////
	unsigned int RET_DEC_PIC_FRAME_NUM;	// 0x1c0, Decoded frame number
	unsigned int RET_DEC_PIC_IDX;		// 0x1c4, Display frame index
	unsigned int RET_DEC_PIC_ERR_MB_NUM;	// 0x1c8, Error MB number in decodec picture
	unsigned int RET_DEC_PIC_TYPE;		// 0x1cc, Decoded picture type
	unsigned int _reserved3[2];			// 0x1d0~0xd4
	unsigned int RET_DEC_PIC_SUCCESS;	// 0x1d8, Command executing result status
	unsigned int RET_DEC_PIC_CUR_IDX;	// 0x1dc, Decoded frame index
	unsigned int RET_DEC_PIC_FCODE_FWD;	// 0x1e0, FCODE value
	unsigned int RET_DEC_PIC_TRD;	// 0x1e4, TRD value
	unsigned int RET_DEC_PIC_TIME_BASE_LAST;	// 0x1e8, TIME_BASE_LAST value
	unsigned int RET_DEC_PIC_NONB_TIME_LAST;	// 0x1ec, NONB_TIME_LAST value
	unsigned int RET_DEC_PIC_BCNT;	// 0x1f0, the size of frame consumed

} S3C6400_MFC_PARAM_REG_DEC_PIC_RUN;


typedef struct tagS3C6400_MFC_PARAM_REG_ENC_PIC_RUN
{
	///////////////////////////////////////
	// INPUT ARGUMENTS common for encoder//
	// (0x180 ~ 0x194)				     //
	///////////////////////////////////////
	unsigned int ENC_PIC_SRC_ADDR_Y;	// 0x180
	unsigned int ENC_PIC_SRC_ADDR_CB;	// 0x184
	unsigned int ENC_PIC_SRC_ADDR_CR;	// 0x188
	unsigned int ENC_PIC_QS;			// 0x18c
	unsigned int ENC_PIC_ROT_MODE;		// 0x190
	unsigned int ENC_PIC_OPTION;		// 0x194
	unsigned int ENC_PIC_BB_START;		// 0x198
	unsigned int ENC_PIC_BB_SIZE;		// 0x19c

	unsigned int _reserved[8];			// 0x1a0, 0x1a4, 0x1a8, 0x1ac, 
										// 0x1b0, 0x1b4, 0x1b8, 0x1bc,

	/////////////////////
	//  OUTPUT RETURN  //
	// (0x1C0 ~ 0x1CC) //
	/////////////////////
	unsigned int RET_ENC_PIC_FRAME_NUM;	// 0x1c0
	unsigned int RET_ENC_PIC_TYPE;		// 0x1c4
	unsigned int RET_ENC_PIC_IDX;		// 0x1c8
	unsigned int RET_ENC_PIC_SLICE_NUM;	// 0x1cc
	unsigned int RET_ENC_PIC_FLAG;		// 0x1d0
} S3C6400_MFC_PARAM_REG_ENC_PIC_RUN;


typedef struct tagS3C6400_MFC_PARAM_REG_ENC_PARA_SET
{
	///////////////////////////////////////
	// INPUT ARGUMENTS common for encoder//
	// (0x180 ~ 0x194)				     //
	///////////////////////////////////////
	unsigned int ENC_PARA_SET_TYPE;		// 0X180

	unsigned int _reserved[15];			// 0x184, 0x188, 0x18c,
										// 0x190, 0x194, 0x198, 0x19c,
										// 0x1a0, 0x1a4, 0x1a8, 0x1ac,
										// 0x1b0, 0x1b4, 0x1b8, 0x1bc,

	/////////////////////
	//  OUTPUT RETURN  //
	// (0x1C0 ~ 0x1CC) //
	/////////////////////
	unsigned int RET_ENC_PARA_SET_SIZE;	// 0x1c0

} S3C6400_MFC_PARAM_REG_ENC_PARA_SET;


typedef struct tagS3C6400_MFC_PARAM_REG_ENC_HEADER
{
	///////////////////////////////////////
	// INPUT ARGUMENTS common for encoder//
	// (0x180 ~ 0x194)				     //
	///////////////////////////////////////
	unsigned int ENC_HEADER_CODE;		// 0x180
	unsigned int ENC_HEADER_BB_START;	// 0x184
	unsigned int ENC_HEADER_BB_SIZE;	// 0x188
	unsigned int ENC_HEADER_NUM;		// 0x18c
	
	unsigned int _reserved[12];			// 0x190, 0x194, 0x198, 0x19c,
										// 0x1a0, 0x1a4, 0x1a8, 0x1ac,
										// 0x1b0, 0x1b4, 0x1b8, 0x1bc,

	/////////////////////
	//  OUTPUT RETURN  //
	// (0x1C0 ~ 0x1CC) //
	/////////////////////

} S3C6400_MFC_PARAM_REG_ENC_HEADER;



typedef struct tagS3C6400_MFC_PARAM_REG_ENC_PARAM_CHANGE
{
	///////////////////////////////////////
	// INPUT ARGUMENTS common for encoder//
	// (0x180 ~ 0x194)				     //
	///////////////////////////////////////
	unsigned int ENC_PARAM_CHANGE_ENABLE;			// 0x180
	unsigned int ENC_PARAM_CHANGE_GOP_NUM;			// 0x184
	unsigned int ENC_PARAM_CHANGE_INTRA_QP;			// 0x188
	unsigned int ENC_PARAM_CHANGE_BITRATE;			// 0x18c
	unsigned int ENC_PARAM_CHANGE_F_RATE;			// 0x190
	unsigned int ENC_PARAM_CHANGE_INTRA_REFRESH;	// 0x194
	unsigned int ENC_PARAM_CHANGE_SLICE_MODE;		// 0x198
	unsigned int ENC_PARAM_CHANGE_HEC_MODE;			// 0x19c

	unsigned int _reserved[8];			// 0x1a0, 0x1a4, 0x1a8, 0x1ac, 
										// 0x1b0, 0x1b4, 0x1b8, 0x1bc,

	/////////////////////
	//  OUTPUT RETURN  //
	// (0x1C0 ~ 0x1CC) //
	/////////////////////
	unsigned int RET_ENC_PARAM_CHANGE_SUCCESS;		// 0x1c0
} S3C6400_MFC_PARAM_REG_ENC_PARAM_CHANGE;


typedef struct tagS3C6400_MFC_PARAM_REG_FIRMWARE_VER
{
	///////////////////////////////////////
	// INPUT ARGUMENTS common for rnc.dec//
	// (0x180 ~ 0x1A0)		     		 //
	///////////////////////////////////////
	unsigned int _reserved[16];		// 0x180 ~ 0x1bc


	/////////////////////
	//  OUTPUT RETURN  //
	// (0x1C0 ~ 0x1D4) //
	/////////////////////
	unsigned int RET_GET_FW_VER;		// 0x1c0,

} S3C6400_MFC_PARAM_REG_FIRMWARE_VER;

typedef struct
{
	unsigned int BITS_RD_PTR;		// 0x120,
	unsigned int BITS_WR_PTR;		// 0x124,
} ST_BITSTRM_BUF_RW_ADDR;

typedef struct tagS3C6400_MFC_HOSTIF_REG
{
	unsigned int CODE_RUN;			// 0x000,
		// [0] 1=Start the bit processor, 0=Stop.
	unsigned int CODE_DN_LOAD;		// 0x004,
		// [15:0]
		// [28:16]
	unsigned int HOST_INTR;			// 0x008,
		// [0] Write '1' to this bit to request an interrupt to BIT
	unsigned int BITS_INT_CLEAR;	// 0x00c,
		// [0]
	unsigned int BITS_INT_STAT;		// 0x010,
		// [0] 1 means that BIT interrupt to the host is asserted.
	unsigned int BITS_CODE_RESET;	// 0x014,
	unsigned int BITS_CUR_PC;		// 0x018,

	unsigned int _reserved1[57];	// 0x01c ~ 0x0fc

	unsigned int CODE_BUF_ADDR;		// 0x100,
	unsigned int WORK_BUF_ADDR;		// 0x104,
	unsigned int PARA_BUF_ADDR;		// 0x108,
	unsigned int STRM_BUF_CTRL;		// 0x10c,
	unsigned int FRME_BUF_CTRL;		// 0x110,
	unsigned int DEC_FUNC_CTRL;		// 0x114, // 7th fw
	unsigned int _reserved2[1];		// 0x118
	unsigned int WORK_BUF_CTRL;		// 0x11c, // 7th fw

	ST_BITSTRM_BUF_RW_ADDR   BIT_STR_BUF_RW_ADDR[8];	// 0x120 ~ 0x15c


	unsigned int BUSY_FLAG;			// 0x160,
	unsigned int RUN_CMD;			// 0x164,
	unsigned int RUN_INDEX;			// 0x168,
	unsigned int RUN_COD_STD;		// 0x16c,
	unsigned int INT_ENABLE;		// 0x170,
	unsigned int INT_REASON;		// 0x174,

	unsigned int _reserved3[2];		// 0x178 ,0x17c

	// Union for the parameters of the MFC commands
	union {
		S3C6400_MFC_PARAM_REG_DEC_SEQ_INIT		dec_seq_init;
		S3C6400_MFC_PARAM_REG_ENC_SEQ_INIT		enc_seq_init;
		S3C6400_MFC_PARAM_REG_SET_FRAME_BUF		set_frame_buf;
		S3C6400_MFC_PARAM_REG_DEC_PIC_RUN		dec_pic_run;
		S3C6400_MFC_PARAM_REG_ENC_PIC_RUN		enc_pic_run;

		S3C6400_MFC_PARAM_REG_ENC_PARA_SET      enc_para_set;
		S3C6400_MFC_PARAM_REG_ENC_HEADER        enc_header;

		S3C6400_MFC_PARAM_REG_FIRMWARE_VER		get_fw_ver;
	} param;

} S3C6400_MFC_SFR;

// MFC_SFR들 중에서 SW_RESET 레지스터는
// 다른 SFR들과 달리 혼자서 멀리 떨어져 있기 때문에 (0xe00 번지에 있다)
// S3C6400_MFC_SFR 구조체에 편입시키지 않고, 
// 상대 주소 값만 정의 했다.
// Memory Setup 하는 부분에서 virtual memory mapping 할때,  
// 이 SW_RESET 레지스터 영역까지 mapping을 하게 된다 
#define S3C6400_MFC_SFR_SW_RESET_ADDR	(0x0e00)


#ifdef __cplusplus
}
#endif

#endif /* __SAMSUNG_SYSLSI_APDEV_MFC_H__ */

