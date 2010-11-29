/* linux/include/asm-arm/arch-s3c2410/regs-mfc.h
 *
 * Copyright (c) 2009 Samsung Electronics 
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * S3C MFC Controller
*/

#ifndef __ASM_ARCH_REGS_MFC_H
#define __ASM_ARCH_REGS_MFC_H __FILE__

/*
 * MFC Interface
 */
#define S3C_MFC(x)			(x)

#define S3C_MFC_CODE_RUN				S3C_MFC(0x000)	/* [0] 1=Start the bit processor, 0=Stop. */
#define S3C_MFC_CODE_DN_LOAD				S3C_MFC(0x004)	/* [15:0] [28:16] */
#define S3C_MFC_HOST_INTR				S3C_MFC(0x008)	/* [0] Write '1' to this bit to request an interrupt to BIT */
#define S3C_MFC_BITS_INT_CLEAR				S3C_MFC(0x00c)
#define S3C_MFC_BITS_INT_STAT				S3C_MFC(0x010)	/* [0] 1 means that BIT interrupt to the host is asserted. */
#define S3C_MFC_BITS_CODE_RESET				S3C_MFC(0x014)
#define S3C_MFC_BITS_CUR_PC				S3C_MFC(0x018)
#define S3C_MFC_RESERVED1				S3C_MFC(0x01c)	/* 0x01c ~ 0x0fc */
#define S3C_MFC_CODE_BUF_ADDR				S3C_MFC(0x100)
#define S3C_MFC_WORK_BUF_ADDR				S3C_MFC(0x104)
#define S3C_MFC_PARA_BUF_ADDR				S3C_MFC(0x108)
#define S3C_MFC_STRM_BUF_CTRL				S3C_MFC(0x10c)
#define S3C_MFC_FRME_BUF_CTRL				S3C_MFC(0x110)
#define S3C_MFC_DEC_FUNC_CTRL				S3C_MFC(0x114)	/* 7th fw */
#define S3C_MFC_RESERVED2				S3C_MFC(0x118)	/* 0x118 */
#define S3C_MFC_WORK_BUF_CTRL				S3C_MFC(0x11c)	/* 7th fw */

#define S3C_MFC_BIT_STR_BASE_PTR0			S3C_MFC(0x120)
#define S3C_MFC_BIT_STR_RD_PTR0				S3C_MFC(0x120)
#define S3C_MFC_BIT_STR_WR_PTR0				S3C_MFC(0x124)

#define S3C_MFC_BIT_STR_BASE_PTR1			S3C_MFC(0x128)
#define S3C_MFC_BIT_STR_RD_PTR1				S3C_MFC(0x128)
#define S3C_MFC_BIT_STR_WR_PTR1				S3C_MFC(0x12c)

#define S3C_MFC_BIT_STR_BASE_PTR2			S3C_MFC(0x130)
#define S3C_MFC_BIT_STR_RD_PTR2				S3C_MFC(0x130)
#define S3C_MFC_BIT_STR_WR_PTR2				S3C_MFC(0x134)

#define S3C_MFC_BIT_STR_BASE_PTR3			S3C_MFC(0x138)
#define S3C_MFC_BIT_STR_RD_PTR3				S3C_MFC(0x138)
#define S3C_MFC_BIT_STR_WR_PTR3				S3C_MFC(0x13c)

#define S3C_MFC_BIT_STR_BASE_PTR4			S3C_MFC(0x140)
#define S3C_MFC_BIT_STR_RD_PTR4				S3C_MFC(0x140)
#define S3C_MFC_BIT_STR_WR_PTR4				S3C_MFC(0x144)

#define S3C_MFC_BIT_STR_BASE_PTR5			S3C_MFC(0x148)
#define S3C_MFC_BIT_STR_RD_PTR5				S3C_MFC(0x148)
#define S3C_MFC_BIT_STR_WR_PTR5				S3C_MFC(0x14c)

#define S3C_MFC_BIT_STR_BASE_PTR6			S3C_MFC(0x150)
#define S3C_MFC_BIT_STR_RD_PTR6				S3C_MFC(0x150)
#define S3C_MFC_BIT_STR_WR_PTR6				S3C_MFC(0x154)

#define S3C_MFC_BIT_STR_BASE_PTR7			S3C_MFC(0x158)
#define S3C_MFC_BIT_STR_RD_PTR7				S3C_MFC(0x158)
#define S3C_MFC_BIT_STR_WR_PTR7				S3C_MFC(0x15c)

#define S3C_MFC_BUSY_FLAG				S3C_MFC(0x160)
#define S3C_MFC_RUN_CMD					S3C_MFC(0x164)
#define S3C_MFC_RUN_INDEX				S3C_MFC(0x168)
#define S3C_MFC_RUN_COD_STD				S3C_MFC(0x16c)
#define S3C_MFC_INT_ENABLE				S3C_MFC(0x170)
#define S3C_MFC_INT_REASON				S3C_MFC(0x174)

#define S3C_MFC_RESERVED3				S3C_MFC(0x178)	/* 0x178 ,0x17c */

#define S3C_MFC_PARAM					S3C_MFC(0x180)

/* Parameter regester decode sequence init */
#define S3C_MFC_PARAM_DEC_SEQ_INIT			S3C_MFC_PARAM
#define S3C_MFC_PARAM_DEC_SEQ_BIT_BUF_ADDR		S3C_MFC(0x180)
#define S3C_MFC_PARAM_DEC_SEQ_BIT_BUF_SIZE		S3C_MFC(0x184)
#define S3C_MFC_PARAM_DEC_SEQ_OPTION			S3C_MFC(0x188)
#define S3C_MFC_PARAM_DEC_SEQ_PRO_BUF			S3C_MFC(0x18c)
#define S3C_MFC_PARAM_DEC_SEQ_TMP_BUF_1			S3C_MFC(0x190)
#define S3C_MFC_PARAM_DEC_SEQ_TMP_BUF_2			S3C_MFC(0x194)
#define S3C_MFC_PARAM_DEC_SEQ_TMP_BUF_3			S3C_MFC(0x198)
#define S3C_MFC_PARAM_DEC_SEQ_TMP_BUF_4			S3C_MFC(0x19c)
#define S3C_MFC_PARAM_DEC_SEQ_TMP_BUF_5			S3C_MFC(0x1a0)
#define S3C_MFC_PARAM_DEC_SEQ_START_BYTE		S3C_MFC(0x1a4)
#define S3C_MFC_PARAM_DEC_SEQ_RESERVED			S3C_MFC(0x1a8)
/* output return */
#define S3C_MFC_PARAM_RET_DEC_SEQ_SUCCESS		S3C_MFC(0x1c0) 
#define S3C_MFC_PARAM_RET_DEC_SEQ_SRC_SIZE		S3C_MFC(0x1c4) 
#define S3C_MFC_PARAM_RET_DEC_SEQ_SRC_FRAME_RATE	S3C_MFC(0x1c8) 
#define S3C_MFC_PARAM_RET_DEC_SEQ_FRAME_NEED_COUNT	S3C_MFC(0x1cc) 
#define S3C_MFC_PARAM_RET_DEC_SEQ_FRAME_DELAY		S3C_MFC(0x1d0) 
#define S3C_MFC_PARAM_RET_DEC_SEQ_INFO			S3C_MFC(0x1d4) 
#define S3C_MFC_PARAM_RET_DEC_SEQ_TIME_RES		S3C_MFC(0x1d8) 
#define S3C_MFC_PARAM_RET_DEC_SEQ_CROP_LEFT_RIGHT	S3C_MFC(0x1dC) 
#define S3C_MFC_PARAM_RET_DEC_SEQ_CROP_TOP_BOTTOM	S3C_MFC(0x1E0) 

/* Paramete register encode sequence init */
#define S3C_MFC_PARAM_ENC_SEQ_INIT			S3C_MFC_PARAM
#define S3C_MFC_PARAM_ENC_SEQ_BIT_BUF_ADDR		S3C_MFC(0x180)
#define S3C_MFC_PARAM_ENC_SEQ_BIT_BUF_SIZE		S3C_MFC(0x184)
#define S3C_MFC_PARAM_ENC_SEQ_OPTION			S3C_MFC(0x188)
#define S3C_MFC_PARAM_ENC_SEQ_COD_STD			S3C_MFC(0x18c)
#define S3C_MFC_PARAM_ENC_SEQ_SRC_SIZE			S3C_MFC(0x190)
#define S3C_MFC_PARAM_ENC_SEQ_SRC_F_RATE		S3C_MFC(0x194)
#define S3C_MFC_PARAM_ENC_SEQ_MP4_PARA			S3C_MFC(0x198)
#define S3C_MFC_PARAM_ENC_SEQ_263_PARA			S3C_MFC(0x19c)
#define S3C_MFC_PARAM_ENC_SEQ_264_PARA			S3C_MFC(0x1a0)
#define S3C_MFC_PARAM_ENC_SEQ_SLICE_MODE		S3C_MFC(0x1a4)
#define S3C_MFC_PARAM_ENC_SEQ_GOP_NUM			S3C_MFC(0x1a8)
#define S3C_MFC_PARAM_ENC_SEQ_RC_PARA			S3C_MFC(0x1ac)
#define S3C_MFC_PARAM_ENC_SEQ_RC_BUF_SIZE		S3C_MFC(0x1b0)
#define S3C_MFC_PARAM_ENC_SEQ_INTRA_MB			S3C_MFC(0x1b4)
#define S3C_MFC_PARAM_ENC_SEQ_FMO			S3C_MFC(0x1b8)
#define S3C_MFC_PARAM_ENC_SEQ_INTRA_QP			S3C_MFC(0x1bc)
/* output return */
#define  S3C_MFC_PARAM_RET_ENC_SEQ_SUCCESS		S3C_MFC(0x1c0)

#define S3C_MFC_PARAM_ENC_SEQ_RC_OPTION			S3C_MFC(0x1c4)
#define S3C_MFC_PARAM_ENC_SEQ_RC_QP_MAX			S3C_MFC(0x1c8)
#define S3C_MFC_PARAM_ENC_SEQ_RC_GAMMA			S3C_MFC(0x1cc)	/* 0x1cc float? */
#define S3C_MFC_PARAM_ENC_SEQ_TMP_BUF1			S3C_MFC(0x1d0)
#define S3C_MFC_PARAM_ENC_SEQ_TMP_BUF2			S3C_MFC(0x1d4)
#define S3C_MFC_PARAM_ENC_SEQ_TMP_BUF3			S3C_MFC(0x1d8)
#define S3C_MFC_PARAM_ENC_SEQ_TMP_BUF4			S3C_MFC(0x1dc)

/* Parameter register set frame buf */
#define S3C_MFC_PARAM_REG_SET_FRAME_BUF			S3C_MFC_PARAM
#define S3C_MFC_PARAM_SET_FRAME_BUF_NUM			S3C_MFC(0x180)
#define S3C_MFC_PARAM_SET_FRAME_BUF_STRIDE		S3C_MFC(0x184)


/* Parameter register decode pic run */
#define S3C_MFC_PARAM_DEC_PIC_RUN			S3C_MFC_PARAM
#define S3C_MFC_PARAM_DEC_PIC_ROT_MODE			S3C_MFC(0x180) /* Display frame post-rotator mode */
#define S3C_MFC_PARAM_DEC_PIC_ROT_ADDR_Y		S3C_MFC(0x184) /* Post-rotated frame store Y address */
#define S3C_MFC_PARAM_DEC_PIC_ROT_ADDR_CB		S3C_MFC(0x188) /* Post-rotated frame store Cb address */
#define S3C_MFC_PARAM_DEC_PIC_ROT_ADDR_CR		S3C_MFC(0x18c) /* Post-rotated frame store Cr address */
#define S3C_MFC_PARAM_DEC_PIC_DBK_ADDR_Y		S3C_MFC(0x190) /* Deblocked frame store Y address */
#define S3C_MFC_PARAM_DEC_PIC_DBK_ADDR_CB		S3C_MFC(0x194) /* Deblocked frame store Cb address */
#define S3C_MFC_PARAM_DEC_PIC_DBK_ADDR_CR		S3C_MFC(0x198) /* Deblocked frame store Cr address */
#define S3C_MFC_PARAM_DEC_PIC_ROT_STRIDE		S3C_MFC(0x19c) /* Post-rotated frame stride */
#define S3C_MFC_PARAM_DEC_PIC_OPTION			S3C_MFC(0x1a0) /* Decoding option */
#define S3C_MFC_PARAM_DEC_PIC_RESERVED1			S3C_MFC(0x1a4)
#define S3C_MFC_PARAM_DEC_PIC_CHUNK_SIZE		S3C_MFC(0x1a8) /* Frame chunk size */
#define S3C_MFC_PARAM_DEC_PIC_BB_START			S3C_MFC(0x1ac) /* 4-byte aligned start address of picture stream buffer */
#define S3C_MFC_PARAM_DEC_PIC_START_BYTE		S3C_MFC(0x1b0) /* Start byte of valid stream data */
#define S3C_MFC_PARAM_DEC_PIC_MV_ADDR			S3C_MFC(0x1b4) /* Base address for Motion Vector data */
#define S3C_MFC_PARAM_DEC_PIC_MBTYPE_ADDR		S3C_MFC(0x1b8) /* Base address for MBType data */
#define S3C_MFC_PARAM_DEC_PIC_RESERVED2			S3C_MFC(0x1bc)
/* output return */
#define S3C_MFC_PARAM_RET_DEC_PIC_FRAME_NUM		S3C_MFC(0x1c0) /* Decoded frame number */
#define S3C_MFC_PARAM_RET_DEC_PIC_IDX			S3C_MFC(0x1c4) /* Display frame index */
#define S3C_MFC_PARAM_RET_DEC_PIC_ERR_MB_NUM		S3C_MFC(0x1c8) /* Error MB number in decodec picture */
#define S3C_MFC_PARAM_RET_DEC_PIC_TYPE			S3C_MFC(0x1cc) /* Decoded picture type */
#define S3C_MFC_PARAM_DEC_PIC_RESERVED3			S3C_MFC(0x1d0) /* 0x1d0 ~ 0x1d4 */
#define S3C_MFC_PARAM_RET_DEC_PIC_SUCCESS		S3C_MFC(0x1d8) /* Command executing result status */
#define S3C_MFC_PARAM_RET_DEC_PIC_CUR_IDX		S3C_MFC(0x1dc) /* Decoded frame index */
#define S3C_MFC_PARAM_RET_DEC_PIC_FCODE_FWD		S3C_MFC(0x1e0) /* FCODE value */
#define S3C_MFC_PARAM_RET_DEC_PIC_TRD			S3C_MFC(0x1e4) /* TRD value */
#define S3C_MFC_PARAM_RET_DEC_PIC_TIME_BASE_LAST	S3C_MFC(0x1e8) /* TIME_BASE_LAST value */
#define S3C_MFC_PARAM_RET_DEC_PIC_NONB_TIME_LAST	S3C_MFC(0x1ec) /* NONB_TIME_LAST value */
#define S3C_MFC_PARAM_RET_DEC_PIC_BCNT			S3C_MFC(0x1f0) /* the size of frame consumed */

/* Parameter register encode pic run */
#define S3C_MFC_PARAM_ENC_PIC_RUN			S3C_MFC_PARAM
#define S3C_MFC_PARAM_ENC_PIC_SRC_ADDR_Y		S3C_MFC(0x180)
#define S3C_MFC_PARAM_ENC_PIC_SRC_ADDR_CB		S3C_MFC(0x184)
#define S3C_MFC_PARAM_ENC_PIC_SRC_ADDR_CR		S3C_MFC(0x188)
#define S3C_MFC_PARAM_ENC_PIC_QS			S3C_MFC(0x18c)
#define S3C_MFC_PARAM_ENC_PIC_ROT_MODE			S3C_MFC(0x190)
#define S3C_MFC_PARAM_ENC_PIC_OPTION			S3C_MFC(0x194)
#define S3C_MFC_PARAM_ENC_PIC_BB_START			S3C_MFC(0x198)
#define S3C_MFC_PARAM_ENC_PIC_BB_SIZE			S3C_MFC(0x19c)
#define S3C_MFC_PARAM_ENC_PIC_RESERVED			S3C_MFC(0x1a0) /* 0x1a0, 0x1a4, 0x1a8, 0x1ac, 0x1b0, 0x1b4, 0x1b8, 0x1bc */
/* output return */
#define S3C_MFC_PARAM_RET_ENC_PIC_FRAME_NUM		S3C_MFC(0x1c0)
#define S3C_MFC_PARAM_RET_ENC_PIC_TYPE			S3C_MFC(0x1c4)
#define S3C_MFC_PARAM_RET_ENC_PIC_IDX			S3C_MFC(0x1c8)
#define S3C_MFC_PARAM_RET_ENC_PIC_SLICE_NUM		S3C_MFC(0x1cc)
#define S3C_MFC_PARAM_RET_ENC_PIC_FLAG			S3C_MFC(0x1d0)

/* Parameter register encode parameter set */
#define S3C_MFC_PARAM_ENC_PARA_SET			S3C_MFC_PARAM
#define S3C_MFC_PARAM_ENC_PARA_SET_TYPE			S3C_MFC(0x180)
#define S3C_MFC_PARAM_ENC_RESERVED			S3C_MFC(0x184)
/* output return */
#define S3C_MFC_PARAM_RET_ENC_PARA_SET_SIZE		S3C_MFC(0x1c0)

/* Parameter register encode header */
#define S3C_MFC_PARAM_ENC_HEADER			S3C_MFC_PARAM
#define S3C_MFC_PARAM_ENC_HEADER_CODE			S3C_MFC(0x180)
#define S3C_MFC_PARAM_ENC_HEADER_BB_START		S3C_MFC(0x184)
#define S3C_MFC_PARAM_ENC_HEADER_BB_SIZE		S3C_MFC(0x188)
#define S3C_MFC_PARAM_ENC_HEADER_NUM			S3C_MFC(0x18c)
#define S3C_MFC_PARAM_ENC_HEADER_RESERVED		S3C_MFC(0x190)


/* Parameter register encode parameter change */
#define S3C_MFC_PARAM_ENC_CHANGE			S3C_MFC_PARAM
#define S3C_MFC_PARAM_ENC_CHANGE_ENABLE			S3C_MFC(0x180)
#define S3C_MFC_PARAM_ENC_CHANGE_GOP_NUM		S3C_MFC(0x184)
#define S3C_MFC_PARAM_ENC_CHANGE_INTRA_QP		S3C_MFC(0x188)
#define S3C_MFC_PARAM_ENC_CHANGE_BITRATE		S3C_MFC(0x18c)
#define S3C_MFC_PARAM_ENC_CHANGE_F_RATE			S3C_MFC(0x190)
#define S3C_MFC_PARAM_ENC_CHANGE_INTRA_REFRESH		S3C_MFC(0x194)
#define S3C_MFC_PARAM_ENC_CHANGE_SLICE_MODE		S3C_MFC(0x198)
#define S3C_MFC_PARAM_ENC_CHANGE_HEC_MODE		S3C_MFC(0x19c)
#define S3C_MFC_PARAM_ENC_CHANGE_RESERVED		S3C_MFC(0x1a0) /* 0x1a0, 0x1a4, 0x1a8, 0x1ac, 0x1b0, 0x1b4, 0x1b8, 0x1bc */
#define S3C_MFC_PARAM_ENC_CHANGE_RESERVED0		S3C_MFC(0x1a0)	
#define S3C_MFC_PARAM_ENC_CHANGE_RESERVED1		S3C_MFC(0x1a4)
#define S3C_MFC_PARAM_ENC_CHANGE_RESERVED2		S3C_MFC(0x1a8)
#define S3C_MFC_PARAM_ENC_CHANGE_RESERVED3		S3C_MFC(0x1ac)
#define S3C_MFC_PARAM_ENC_CHANGE_RESERVED4		S3C_MFC(0x1b0)
#define S3C_MFC_PARAM_ENC_CHANGE_RESERVED5		S3C_MFC(0x1b4)
#define S3C_MFC_PARAM_ENC_CHANGE_RESERVED6		S3C_MFC(0x1b8)
#define S3C_MFC_PARAM_ENC_CHANGE_RESERVED7		S3C_MFC(0x1bc)

#define S3C_MFC_PARAM_RET_ENC_CHANGE_SUCCESS		S3C_MFC(0x1c0)

/* Parameter register firmware version */
#define S3C_MFC_PARAM_FIRMWARE_VER			S3C_MFC_PARAM
#define S3C_MFC_PARAM_FIRMWARE_VER_RESERVED		S3C_MFC(0x180) /* 0x180 ~ 0x1bc */
#define S3C_MFC_PARAM_FIRMWARE_VER_GET_FW_VER		S3C_MFC(0x1c0)


/*
 * Because SW_RESET register is located apart(address 0xe00), unlike other MFC_SFR registers, 
 * I have excluded it in S3C_MFC_SFR struct and defined relative address only.  
 * When do virtual memory mapping in setting up memory, we have to map until this SW_RESET register.
 */
#define S3C_MFC_SFR_SW_RESET_ADDR			S3C_MFC(0x0e00)
#define S3C_MFC_SFR_SIZE				S3C_MFC(0x0e00)


/*************************************************************************
 * Bit definition part
 ************************************************************************/

/* SDRAM buffer control options */
#define STREAM_ENDIAN_LITTLE		(0<<0)
#define STREAM_ENDIAN_BIG		(1<<0)
#define BUF_STATUS_FULL_EMPTY_CHECK_BIT	(0<<1)
#define BUF_STATUS_NO_CHECK_BIT		(1<<1)
                                         
/* FRAME_BUF_CTRL (0x110) */
#define FRAME_MEM_ENDIAN_LITTLE		(0<<0)
#define FRAME_MEM_ENDIAN_BIG		(1<<0)

/*
 *    PRiSM-CX Video Codec IP's Register
 *    V178
 */

/* DEC_SEQ_INIT Parameter Register */
/* DEC_SEQ_OPTION (0x18c) */
#define MP4_DBK_DISABLE			(0<<0)
#define MP4_DBK_ENABLE			(1<<0)
#define REORDER_DISABLE			(0<<1)
#define REORDER_ENABLE			(1<<1)
#define FILEPLAY_ENABLE			(1<<2)
#define FILEPLAY_DISABLE		(0<<2)
#define DYNBUFALLOC_ENABLE		(1<<3)
#define DYNBUFALLOC_DISABLE		(0<<3)

/* ENC_SEQ_INIT Parameter Register */
/* ENC_SEQ_OPTION (0x188) */
#define MB_BIT_REPORT_DISABLE		(0<<0)
#define MB_BIT_REPORT_ENABLE		(1<<0)
#define SLICE_INFO_REPORT_DISABLE	(0<<1)
#define SLICE_INFO_REPORT_ENABLE	(1<<1)
#define AUD_DISABLE			(0<<2)
#define AUD_ENABLE			(1<<2)
#define MB_QP_REPORT_DISABLE		(0<<3)
#define MB_QP_REPORT_ENBLE		(1<<3)
#define CONST_QP_DISABLE		(0<<5)
#define CONST_QP_ENBLE			(1<<5)

/* ENC_SEQ_COD_STD (0x18C) */
#define MPEG4_ENCODE			0
#define H263_ENCODE			1
#define H264_ENCODE			2

/* ENC_SEQ_MP4_PARA (0x198) */
#define DATA_PART_DISABLE		(0<<0)
#define DATA_PART_ENABLE		(1<<0)

/* ENC_SEQ_263_PARA (0x19C) */
#define ANNEX_T_OFF			(0<<0)
#define ANNEX_T_ON			(1<<0)
#define ANNEX_K_OFF			(0<<1)
#define ANNEX_K_ON			(1<<1)
#define ANNEX_J_OFF			(0<<2)
#define ANNEX_J_ON			(1<<2)
#define ANNEX_I_OFF			(0<<3)
#define ANNEX_I_ON			(1<<3)

/* ENC_SEQ_SLICE_MODE (0x1A4) */
#define SLICE_MODE_ONE			(0<<0)
#define SLICE_MODE_MULTIPLE		(1<<0)

/* ENC_SEQ_RC_PARA (0x1AC) */
#define RC_DISABLE			(0<<0)    /* RC means rate control */
#define RC_ENABLE			(1<<0)
#define SKIP_DISABLE			(1<<31)
#define SKIP_ENABLE			(0<<31)

/* ENC_SEQ_FMO (0x1B8) */
#define FMO_DISABLE			(0<<0)
#define FMO_ENABLE			(1<<0)

/* ENC_SEQ_RC_OPTION (0x1C4) */
#define USER_QP_MAX_DISABLE		(0<<0)
#define USER_QP_MAX_ENABLE		(1<<0)
#define USE_GAMMA_DISABLE		(0<<1)
#define USE_GAMMA_ENABLE		(1<<1)



#endif /* __ASM_ARCH_REGS_MFC_H */
