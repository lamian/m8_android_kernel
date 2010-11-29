/* drivers/media/video/s3c_camif.h
 *
 * Copyright (c) 2008 Samsung Electronics
 *
 * Samsung S3C Camera driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __S3C_CAMIF_H_
#define __S3C_CAMIF_H_

#ifdef __KERNEL__
#include <linux/videodev.h>
#include <linux/videodev2.h>
#include <asm/types.h>
#include <linux/i2c.h>
#include <linux/video_decoder.h>
#include <plat/reserved_mem.h>
#endif	/* __KERNEL__ */

#if !defined(O_NONCAP)
#define O_NONCAP O_TRUNC
#endif

#if defined(CAMIF_DEBUG)
#define DPRINTK(fmt, args...) printk(KERN_DEBUG "%s: " fmt, __FUNCTION__ , ## args)
#else
#define DPRINTK(fmt, args...)
#endif

#if defined(CAMIF_DEBUG)
#define assert(expr) \
	if(!(expr)) { \
	printk( "Assertion failed! %s,%s,%s,line=%d\n", \
	#expr,__FILE__,__FUNCTION__,__LINE__);		\
	}
#else
#define assert(expr)
#endif

#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
#define FIMC_VER	"3.0"
#elif defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
#define CAM_MEM_SIZE	0x04000000
#define FIMC_VER	"2.3"
#else
#define CAM_MEM_SIZE	0x04000000
#define FIMC_VER	"2.x"
#endif

#undef FSM_ON_PREVIEW
#define FSM_ON_CODEC

#undef USE_LAST_IRQ			/* turn on if pp count is 1 */

#define CODEC_DEV_NAME			"CAMIF_CODEC"
#define PREVIEW_DEV_NAME		"CAMIF_PREVIEW"

#define CAMIF_DEV_NUM			2
#define FIMC_CODEC_INDEX		0
#define FIMC_PREVIEW_INDEX		1

#define BURST_ERR			1

#if defined (CONFIG_MACH_BONANZA)
#define CAMERA_MEM_SIZE       RESERVED_PMEM_PICTURE_SIZE // RESERVED_MEM_CAMERA
#else
#define CAMERA_MEM_SIZE       RESERVED_PMEM_PICTURE // RESERVED_MEM_CAMERA
#endif
#define CAMERA_MEM_START      PICTURE_RESERVED_PMEM_START


#define YUV_MEM				CAMERA_MEM_SIZE
#define RGB_MEM				(CAMERA_MEM_SIZE - YUV_MEM)

#define CODEC_DEFAULT_WIDTH	        1024
#define CODEC_DEFAULT_HEIGHT		 768
#define PREVIEW_DEFAULT_WIDTH		640
#define PREVIEW_DEFAULT_HEIGHT		480

#define CROP_DEFAULT_WIDTH		352
#define CROP_DEFAULT_HEIGHT		272

#define MAX_PPNUM              4
#define CODEC_DEFAULT_PPNUM    4
#define PREVIEW_DEFAULT_PPNUM  2

//#define CODEC_MINOR			12
//#define PREVIEW_MINOR			13
#define CODEC_MINOR			0
#define PREVIEW_MINOR			1

#define CHECK_FREQ			5
#define INSTANT_SKIP			0
#define INSTANT_GO			1

#define ZOOM_AT_A_TIME_IN_PIXELS	16 //32
#define ZOOM_IN_MAX			640

/* Codec or Preview Status */
#define CAMIF_STARTED			(1 << 1)
#define CAMIF_STOPPED			(1 << 2)
#define CAMIF_INT_HAPPEN		(1 << 3)

/* Codec or Preview  : Interrupt FSM */
#define CAMIF_1st_INT			(1 << 7)
#define CAMIF_Xth_INT			(1 << 8)
#define CAMIF_Yth_INT			(1 << 9)
#define CAMIF_Zth_INT			(1 << 10)
#define CAMIF_NORMAL_INT		(1 << 11)
#define CAMIF_DUMMY_INT			(1 << 12)
#define CAMIF_CONTINUOUS_INT	(1 << 13)
#define CAMIF_SET_LAST_INT		(1 << 14)
#define CAMIF_STOP_CAPTURE		(1 << 15)
#define CAMIF_LAST_IRQ			(1 << 16)
#define CAMIF_PENDING_INT		0

//skip frame(5->1) is modified by sangyub
// when the preview start, 1 frame is skipped.(5->1)*/
#define CAMIF_CAPTURE_SKIP_FRAMES	1

/* CAMIF RESET Definition */
#define CAMIF_RESET			(1 << 0)
#define CAMIF_EX_RESET_AL	(1 << 1)	/* Active Low */
#define CAMIF_EX_RESET_AH	(1 << 2)	/* Active High */


/* Sensor Command Defintion */
#define SENSOR_DEFAULT			0

#define SENSOR_INIT			(1 << 0)
#define USER_ADD			(1 << 1)
#define USER_EXIT			(1 << 2)

#define SENSOR_CIF			(1 << 3)
#define SENSOR_QVGA			(1 << 4)
#define SENSOR_VGA			(1 << 5)
#define SENSOR_QSVGA		(1 << 6)
#define SENSOR_SVGA			(1 << 7)
#define SENSOR_XGA			(1 << 8)
#define SENSOR_SXGA			(1 << 9)
#define SENSOR_UXGA			(1 << 10)
#define SENSOR_QSXGA		(1 << 11)
#define SENSOR_QXGA			(1 << 12)

#define SENSOR_MIRROR		(1 << 13)
#define SENSOR_ZOOMOUT		(1 << 14)
#define SENSOR_ZOOMIN		(1 << 15)

#define SENSOR_USER_READ	(1 << 16)
#define SENSOR_USER_WRITE	(1 << 17)

#define SENSOR_MODE_SET		(1 << 18)
#define SENSOR_AF			(1 << 19)
#define SENSOR_SCENE_MODE	(1 << 20)
#define SENSOR_PHOTOMETRY	(1 << 21)
#define SENSOR_ISO			(1 << 22)
#define SENSOR_CONTRAST		(1 << 23)
#define SENSOR_SATURATION	(1 << 24)
#define SENSOR_SHARPNESS	(1 << 25)
#define SENSOR_WB			(1 << 26)
#define SENSOR_BRIGHTNESS	(1 << 27)
#define SENSOR_EFFECT		(1 << 28)
#define SENSOR_FLASH_CAMERA	(1 << 29)
#define SENSOR_FLASH_MOVIE	(1 << 30)
#define SENSOR_EXIF_DATA	(1 << 31)

#define SENSOR_MAX			0xffffffff

/* Global Status Definition */
#define PWANT2START			(1 << 0)
#define CWANT2START			(1 << 1)
#define BOTH_STARTED		(PWANT2START | CWANT2START)
#define P_NOT_WORKING		(1 << 4)
#define C_WORKING			(1 << 5)
#define P_WORKING			(1 << 6)
#define C_NOT_WORKING		(1 << 7)

#define FORMAT_FLAGS_DITHER		0x01
#define FORMAT_FLAGS_PACKED		0x02
#define FORMAT_FLAGS_PLANAR		0x04
#define FORMAT_FLAGS_RAW		0x08
#define FORMAT_FLAGS_CrCb		0x10

enum camif_itu_fmt {
	CAMIF_ITU601 = (1 << 31),
	CAMIF_ITU656 = 0,
};

/* It is possbie to use two device simultaneously */
enum camif_dma_type {
	CAMIF_PREVIEW = (1 << 0),
	CAMIF_CODEC   = (1 << 1),
};

enum camif_order422 {
	CAMIF_YCBYCR = 0,
	CAMIF_YCRYCB = (1 << 14),
	CAMIF_CBYCRY = (1 << 15),
	CAMIF_CRYCBY = (1 << 15) | (1 << 14),
};

enum flip_mode {
	CAMIF_FLIP = 0,
	CAMIF_ROTATE_90	= (1 << 13),
	CAMIF_FLIP_X = (1 << 14),
	CAMIF_FLIP_Y = (1 << 15),
	CAMIF_FLIP_MIRROR = (1 << 15) | (1 << 14),
	CAMIF_FLIP_ROTATE_270 = (1 << 15) | (1 << 14) | (1 << 13),
};

enum camif_fmt {
	CAMIF_YCBCR420 = (1 << 0),
	CAMIF_YCBCR422 = (1 << 1),
	CAMIF_YCBCR422I = (1 << 2),
	CAMIF_RGB16 = (1 << 3),
	CAMIF_RGB24 = (1 << 4),
	CAMIF_RGB32 = (1 << 5),
};

enum camif_capturing {
	CAMIF_BOTH_DMA_ON = (1 << 4),
	CAMIF_DMA_ON = (1 << 3),
	CAMIF_BOTH_DMA_OFF = (1 << 1),
	CAMIF_DMA_OFF = (1 << 0),
	CAMIF_DMA_OFF_L_IRQ = (1 << 5),
};

enum image_effect {
	CAMIF_BYPASS,
	CAMIF_ARBITRARY_CB_CR,
	CAMIF_NEGATIVE,
	CAMIF_ART_FREEZE,
	CAMIF_EMBOSSING ,
	CAMIF_SILHOUETTE,
};

enum input_channel{
	CAMERA_INPUT,
	MSDMA_FROM_CODEC,
	MSDMA_FROM_PREVIEW,
};

enum output_channel{
	CAMIF_OUT_PP,
	CAMIF_OUT_FIFO,
};

typedef struct camif_performance
{
	int	frames;
	int	framesdropped;
	__u64	bytesin;
	__u64	bytesout;
	__u32	reserved[4];
} camif_perf_t;

typedef struct {
	// modified by sangyub 
	// phys_y and phys_rgb is shared using union structure
	// vir_rgb and virt_y is shared using union structure
	union{
		dma_addr_t  phys_y;
		dma_addr_t  phys_rgb;
	};
	
	dma_addr_t  phys_cb;
	dma_addr_t  phys_cr;
	u8 *        virt_cb;
	u8 *        virt_cr;
	union{
		u8 *        virt_rgb;
		u8 *        virt_y;
	};
} img_buf_t;

/* this structure convers the CIWDOFFST, prescaler, mainscaler */
typedef struct {
	u32 modified_src_x;	/* After windows applyed to source_x */
	u32 modified_src_y;
	u32 hfactor;
	u32 vfactor;
	u32 shfactor;		/* SHfactor = 10 - ( hfactor + vfactor ) */
	u32 prehratio;
	u32 prevratio;
	u32 predst_x;
	u32 predst_y;
	u32 scaleup_h;
	u32 scaleup_v;
	u32 mainhratio;
	u32 mainvratio;
	u32 scalerbypass;	/* only codec */
	u32 zoom_in_cnt;
} scaler_t;

enum v4l2_status {
	CAMIF_V4L2_INIT	= (1 << 0),
	CAMIF_v4L2_DIRTY = (1 << 1),
};

typedef struct {
	struct mutex		lock;
	enum camif_itu_fmt	itu_fmt;
	enum camif_order422	order422;
	struct i2c_client	*sensor;
	u32			win_hor_ofst;
	u32			win_ver_ofst;
	u32			win_hor_ofst2;
	u32			win_ver_ofst2;
	u32			camclk; 		/* External Image Sensor Camera Clock */
	u32			source_x;
	u32			source_y;
	u32			polarity_pclk;
	u32			polarity_vsync;
	u32			polarity_href;
	u32			user;			/* MAX 2 (codec, preview) */
	u32			irq_old_priority;	/* BUS PRIORITY register */
	u32			status;
	u32			init_sensor;		/* initializing sensor */
	u32			reset_type;		/* External Sensor Reset  Type */
	u32			reset_udelay;
	u32			zoom_in_cnt;
} camif_cis_t;

/* when  App want to change v4l2 parameter,
 * we instantly store it into v4l2_t v2
 * and then reflect it to hardware
 */
typedef struct v4l2 {
	struct v4l2_fmtdesc	*fmtdesc;
	struct v4l2_framebuffer	frmbuf; /* current frame buffer */
	struct v4l2_input	*input;
	struct v4l2_output	*output;
	enum v4l2_status	status;

	/* crop */
	struct v4l2_rect	crop_bounds;
	struct v4l2_rect	crop_defrect;
	struct v4l2_rect	crop_current;

} v4l2_t;


typedef struct camif_c_t {
	struct video_device	*v;

	/* V4L2 param only for v4l2 driver */
	v4l2_t			v2;
	camif_cis_t		*cis;			/* Common between Codec and Preview */

	/* logical parameter */
	wait_queue_head_t	waitq;
	u32			status; 		/* Start/Stop */
	u32			fsm;			/* Start/Stop */
	u32			open_count;		/* duplicated */
	int			irq;
	char			shortname[16];
	u32			target_x;
	u32			target_y;
	scaler_t		sc;
	enum flip_mode		flip;
	enum image_effect	effect;
	enum camif_dma_type	dma_type;

	/* 4 pingpong Frame memory */
	u8 *            pp_virt_buf;
	dma_addr_t      pp_phys_buf;
	u32             pp_totalsize;
	u32             pp_num; 		// used pingpong memory number */
	img_buf_t       img_buf[4];
	int             flag_img_buf_user_set;
	enum camif_fmt  src_fmt;
	enum camif_fmt  dst_fmt;
	enum camif_capturing	capture_enable;
	camif_perf_t		perf;
	u32			cur_frame_num;
	u32			auto_restart;		/* Only For Preview */
	int			input_channel;
	int			output_channel;
	int			buffer_size;
	void			*other; 		/* other camif_cfg_t */
	u32			msdma_status;		/* 0 : stop, 1 : start */
	void __iomem		*regs;
} camif_cfg_t;

/*  Test Application Usage */
typedef struct {
	int src_x;
	int src_y;
	int dst_x;
	int dst_y;
	int src_fmt;
	int dst_fmt;
	int flip;
	int awb;
	int effect;
	int input_channel;
	int output_channel;
	unsigned int h_offset;
	unsigned int v_offset;
	unsigned int h_offset2;
	unsigned int v_offset2;
} camif_param_t;

enum sensor_cap_mode{
	SENSOR_PREVIEW = (1 << 0), 
	SENSOR_CAPTURE = (1 << 1), 
	SENSOR_CAMCORDER = (1 << 2),
	SENSOR_NIGHTMODE = (1 << 4),
	SENSOR_FLASH_CAPTURE= (1 << 8),
	SENSOR_FLASH_CAP_LOW= (1 << 16),
};

typedef struct {
	unsigned int width;
	unsigned int height;
	unsigned int opt_zoom;
	unsigned int dig_zoom;
	unsigned int white_balance;
	unsigned int bright;
	unsigned int auto_focus;
	unsigned int rotate_angle;
} CAM_PP;

struct v4l2_input_handler {
	int (*init_input_handler)(void);
	void (*exit_input_handler)(void);
};

/* Externs */
extern camif_cfg_t* s3c_camif_get_fimc_object(int);
extern int s3c_camif_start_dma(camif_cfg_t *);
extern int s3c_camif_stop_dma(camif_cfg_t *);
extern int s3c_camif_get_frame_num(camif_cfg_t *);
extern unsigned char* s3c_camif_get_frame(camif_cfg_t *);
extern int s3c_camif_control_fimc(camif_cfg_t *);
extern void s3c_camif_reset(int, int);
extern void s3c_camif_init(void);
extern int s3c_camif_get_fifo_status(camif_cfg_t *);
extern void s3c_camif_enable_lastirq(camif_cfg_t *);
extern void s3c_camif_disable_lastirq(camif_cfg_t *);
extern void s3c_camif_change_flip(camif_cfg_t *);
extern void s3c_camif_change_effect(camif_cfg_t *);
extern int s3c_camif_start_codec_msdma(camif_cfg_t *);
extern int s3c_camif_set_clock(unsigned int camclk);
extern void s3c_camif_disable_clock(void);
extern int s3c_camif_start_preview_msdma(camif_cfg_t *);
extern camif_cis_t* get_initialized_cis(void);
extern void s3c_camif_clear_irq(int);
extern int s3c_camif_set_source_format(camif_cis_t *);
extern void s3c_camif_register_sensor(camif_cis_t *);
extern void s3c_camif_unregister_sensor(camif_cis_t *);
extern int s3c_camif_setup_dma(camif_cfg_t *);
extern int s3c_camif_init_sensor(camif_cfg_t *);
extern int s3c_camif_set_offset(camif_cis_t *);
extern void s3c_camif_set_priority(int);
extern void s3c_camif_open_sensor(camif_cis_t *);
extern void s3c_camif_set_polarity(camif_cfg_t *cfg);
extern struct clk *cam_clock;
extern struct clk *cam_hclk;
extern int s3c_camif_add_sensor(struct v4l2_input*, struct v4l2_input_handler *);
extern void s3c_camif_remove_sensor(struct v4l2_input *, struct v4l2_input_handler *);
#endif

