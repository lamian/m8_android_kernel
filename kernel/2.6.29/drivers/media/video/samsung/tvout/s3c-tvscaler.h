#ifndef __S3CTVSCALER_H_
#define __S3CTVSCALER_H_

#include <plat/reserved_mem.h>

#define TVSCALER_IOCTL_MAGIC 'S'

#define PPROC_SET_PARAMS	_IO(TVSCALER_IOCTL_MAGIC, 0)
#define PPROC_START		_IO(TVSCALER_IOCTL_MAGIC, 1)
#define PPROC_STOP		_IO(TVSCALER_IOCTL_MAGIC, 2)
#define PPROC_INTERLACE_MODE	_IO(TVSCALER_IOCTL_MAGIC, 3)
#define PPROC_PROGRESSIVE_MODE	_IO(TVSCALER_IOCTL_MAGIC, 4)


#define QVGA_XSIZE      320
#define QVGA_YSIZE      240

#define LCD_XSIZE       320
#define LCD_YSIZE       240

#define SCALER_MINOR  251                     // Just some number

// 2009.04.01 modified by hyunkyung
//#define SYSTEM_RAM		0x08000000	// 128mb
//#define SYSTEM_RAM		0x07800000	// 120mb
//#define RESERVE_POST_MEM	8*1024*1024	// 8mb
#define RESERVE_POST_MEM	1*1024*1024	// 1mb
//#define PRE_BUFF_SIZE		4*1024*1024	//4 // 4mb
//#define POST_BUFF_SIZE		( RESERVE_POST_MEM - PRE_BUFF_SIZE )
#if 0
#define POST_BUFF_BASE_ADDR	(0x50000000 + (SYSTEM_RAM - RESERVE_POST_MEM))
#else	// TV_RESERVED_MEM_START is defined in the s3c-linux-2.6.21_dev_4_4_15
#define POST_BUFF_BASE_ADDR	TV_RESERVED_MEM_START
#endif

#define USE_DEDICATED_MEM	1

typedef enum {
	INTERLACE_MODE,
	PROGRESSIVE_MODE
} s3c_scaler_scan_mode_t;

typedef enum {
	POST_DMA, POST_FIFO
} s3c_scaler_path_t;

typedef enum {
	ONE_SHOT, FREE_RUN
} s3c_scaler_run_mode_t;

typedef enum {
	PAL1, PAL2, PAL4, PAL8,
	RGB8, ARGB8, RGB16, ARGB16, RGB18, RGB24, RGB30, ARGB24,
	YC420, YC422, // Non-interleave
	CRYCBY, CBYCRY, YCRYCB, YCBYCR, YUV444 // Interleave
} cspace_t;

typedef enum
{
	HCLK = 0, PLL_EXT = 1, EXT_27MHZ = 3
} scaler_clk_src_t;

typedef struct{
	unsigned int SrcFullWidth; 		// Source Image Full Width(Virtual screen size)
	unsigned int SrcFullHeight; 		// Source Image Full Height(Virtual screen size)
	unsigned int SrcStartX; 			// Source Image Start width offset
	unsigned int SrcStartY; 			// Source Image Start height offset
	unsigned int SrcWidth;			// Source Image Width
	unsigned int SrcHeight; 			// Source Image Height
	unsigned int SrcFrmSt; 			// Base Address of the Source Image : Physical Address
	cspace_t SrcCSpace; 		// Color Space ot the Source Image
	
	unsigned int DstFullWidth; 		// Source Image Full Width(Virtual screen size)
	unsigned int DstFullHeight; 		// Source Image Full Height(Virtual screen size)
	unsigned int DstStartX; 			// Source Image Start width offset
	unsigned int DstStartY; 			// Source Image Start height offset
	unsigned int DstWidth; 			// Source Image Width
	unsigned int DstHeight; 			// Source Image Height
	unsigned int DstFrmSt; 			// Base Address of the Source Image : Physical Address
	cspace_t DstCSpace; 		// Color Space ot the Source Image
	
	unsigned int SrcFrmBufNum; 		// Frame buffer number
	s3c_scaler_run_mode_t Mode; 	// POST running mode(PER_FRAME or FREE_RUN)
	s3c_scaler_path_t InPath; 	// Data path of the source image
	s3c_scaler_path_t OutPath; 	// Data path of the desitination image

}scaler_params_t;

typedef struct{
	unsigned int pre_phy_addr;
	unsigned char *pre_virt_addr;

	unsigned int post_phy_addr;
	unsigned char *post_virt_addr;
} buff_addr_t;

#endif //__S3CTVSCALER_H_
