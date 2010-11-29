/* linux/drivers/media/video/samsung/s3c_fimc.h
 *
 * Header file for Samsung Camera Interface (FIMC) driver
 *
 * Jinsung Yang, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef _S3C_CAMIF_H
#define _S3C_CAMIF_H

#ifdef __KERNEL__
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/videodev2.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include <plat/fimc.h>
#endif

/*
 * P I X E L   F O R M A T   G U I D E
 *
 * The 'x' means 'DO NOT CARE'
 * The '*' means 'FIMC SPECIFIC'
 * For some fimc formats, we couldn't find equivalent format in the V4L2 FOURCC.
 *
 * FIMC TYPE	PLANES	ORDER		V4L2_PIX_FMT
 * ---------------------------------------------------------
 * RGB565	x	x		V4L2_PIX_FMT_RGB565
 * RGB888	x	x		V4L2_PIX_FMT_RGB24
 * YUV420	2	LSB_CBCR	V4L2_PIX_FMT_NV12
 * YUV420	2	LSB_CRCB	V4L2_PIX_FMT_NV21
 * YUV420	2	MSB_CBCR	V4L2_PIX_FMT_NV21X*
 * YUV420	2	MSB_CRCB	V4L2_PIX_FMT_NV12X*
 * YUV420	3	x		V4L2_PIX_FMT_YUV420
 * YUV422	1	YCBYCR		V4L2_PIX_FMT_YUYV
 * YUV422	1	YCRYCB		V4L2_PIX_FMT_YVYU
 * YUV422	1	CBYCRY		V4L2_PIX_FMT_UYVY
 * YUV422	1	CRYCBY		V4L2_PIX_FMT_VYUY*
 * YUV422	2	LSB_CBCR	V4L2_PIX_FMT_NV16*
 * YUV422	2	LSB_CRCB	V4L2_PIX_FMT_NV61*
 * YUV422	2	MSB_CBCR	V4L2_PIX_FMT_NV16X*
 * YUV422	2	MSB_CRCB	V4L2_PIX_FMT_NV61X*
 * YUV422	3	x		V4L2_PIX_FMT_YUV422P
 *
*/

/*
 * C O M M O N   D E F I N I T I O N S
 *
*/
#define S3C_FIMC_NAME	"s3c-fimc"

#define info(args...)	do { printk(KERN_INFO S3C_FIMC_NAME ": " args); } while (0)
#define err(args...)	do { printk(KERN_ERR  S3C_FIMC_NAME ": " args); } while (0)

#define S3C_FIMC_FRAME_SKIP		0
#define S3C_FIMC_FRAME_TAKE		1


#define S3C_FIMC_MAX_CTRLS		3
#define S3C_FIMC_MAX_FRAMES		4

/* including 1 more for test pattern */
#define S3C_FIMC_MAX_CAMS		4
#define S3C_FIMC_TPID			(S3C_FIMC_MAX_CAMS - 1)


#define S3C_FIMC_ZOOM_PIXELS		32

#define S3C_FIMC_CROP_DEF_WIDTH		352
#define S3C_FIMC_CROP_DEF_HEIGHT	272

/* S flag: global status flags */
#define S3C_FIMC_FLAG_RUNNING		0x0001
#define S3C_FIMC_FLAG_STOP		0x0002
#define S3C_FIMC_FLAG_HANDLE_IRQ	0x0004
#define S3C_FIMC_STA_MASK		0x000f

/* U flag: use case flags */
#define S3C_FIMC_FLAG_PREVIEW		0x0010
#define S3C_FIMC_FLAG_CAPTURE		0x0020
#define S3C_FIMC_USE_MASK		0x00f0

/* I flag: IRQ flags */
#define S3C_FIMC_FLAG_IRQ_NORMAL	0x0100	
#define S3C_FIMC_FLAG_IRQ_X		0x0200
#define S3C_FIMC_FLAG_IRQ_Y		0x0400
#define S3C_FIMC_FLAG_IRQ_LAST		0x0800
#define S3C_FIMC_IRQ_MASK		0x0f00

#define UNMASK_STATUS(x)		(x->flag &= ~S3C_FIMC_STA_MASK)
#define UNMASK_USAGE(x)			(x->flag &= ~S3C_FIMC_USE_MASK)
#define UNMASK_IRQ(x)			(x->flag &= ~S3C_FIMC_IRQ_MASK)

#define FSET_RUNNING(x)			UNMASK_STATUS(x); (x->flag |= S3C_FIMC_FLAG_RUNNING)
#define FSET_STOP(x)			UNMASK_STATUS(x); (x->flag |= S3C_FIMC_FLAG_STOP)
#define FSET_HANDLE_IRQ(x)		UNMASK_STATUS(x); (x->flag |= S3C_FIMC_FLAG_HANDLE_IRQ)

#define FSET_PREVIEW(x)			UNMASK_USAGE(x); (x->flag |= S3C_FIMC_FLAG_PREVIEW)
#define FSET_CAPTURE(x)			UNMASK_USAGE(x); (x->flag |= S3C_FIMC_FLAG_CAPTURE)

#define FSET_IRQ_NORMAL(x)		UNMASK_IRQ(x); (x->flag |= S3C_FIMC_FLAG_IRQ_NORMAL)
#define FSET_IRQ_X(x)			UNMASK_IRQ(x); (x->flag |= S3C_FIMC_FLAG_IRQ_X)
#define FSET_IRQ_Y(x)			UNMASK_IRQ(x); (x->flag |= S3C_FIMC_FLAG_IRQ_Y)
#define FSET_IRQ_LAST(x)		UNMASK_IRQ(x); (x->flag |= S3C_FIMC_FLAG_IRQ_LAST)

#define IS_RUNNING(x)			(x->flag & S3C_FIMC_FLAG_RUNNING)
#define IS_IRQ_HANDLING(x)		(x->flag & S3C_FIMC_FLAG_HANDLE_IRQ)

#define IS_PREVIEW(x)			(x->flag & S3C_FIMC_FLAG_PREVIEW)
#define IS_CAPTURE(x)			(x->flag & S3C_FIMC_FLAG_CAPTURE)

#define IS_IRQ_NORMAL(x)		(x->flag & S3C_FIMC_FLAG_IRQ_NORMAL)
#define IS_IRQ_X(x)			(x->flag & S3C_FIMC_FLAG_IRQ_X)
#define IS_IRQ_Y(x)			(x->flag & S3C_FIMC_FLAG_IRQ_Y)
#define IS_IRQ_LAST(x)			(x->flag & S3C_FIMC_FLAG_IRQ_LAST)

#define PAT_CB(x)			((x >> 8) & 0xff)
#define PAT_CR(x)			(x & 0xff)


/*
 * E N U M E R A T I O N S
 *
*/
enum s3c_fimc_cam_t {
	CAM_TYPE_ITU	= 0,
	CAM_TYPE_MIPI	= 1,
};

enum s3c_fimc_cam_mode_t {
	ITU_601_YCBCR422_8BIT = (1 << 31),
	ITU_656_YCBCR422_8BIT = (0 << 31),
	ITU_601_YCBCR422_16BIT = (1 << 29),
	MIPI_CSI_YCBCR422_8BIT = 0x1e,
	MIPI_CSI_RAW8 = 0x2a,
	MIPI_CSI_RAW10 = 0x2b,
	MIPI_CSI_RAW12 = 0x2c,
};

enum s3c_fimc_order422_cam_t {
	CAM_ORDER422_8BIT_YCBYCR = (0 << 14),
	CAM_ORDER422_8BIT_YCRYCB = (1 << 14),
	CAM_ORDER422_8BIT_CBYCRY = (2 << 14),
	CAM_ORDER422_8BIT_CRYCBY = (3 << 14),
	CAM_ORDER422_16BIT_Y4CBCRCBCR = (0 << 14),
	CAM_ORDER422_16BIT_Y4CRCBCRCB = (1 << 14),
};

enum s3c_fimc_order422_in_t {
	IN_ORDER422_CRYCBY = (0 << 4),
	IN_ORDER422_YCRYCB = (1 << 4),
	IN_ORDER422_CBYCRY = (2 << 4),
	IN_ORDER422_YCBYCR = (3 << 4),
};

enum s3c_fimc_order422_out_t {
	OUT_ORDER422_YCBYCR = (0 << 0),
	OUT_ORDER422_YCRYCB = (1 << 0),
	OUT_ORDER422_CBYCRY = (2 << 0),
	OUT_ORDER422_CRYCBY = (3 << 0),
};

enum s3c_fimc_2plane_order_t {
	LSB_CBCR = 0,
	LSB_CRCB = 1,
	MSB_CRCB = 2,
	MSB_CBCR = 3,
};

enum s3c_fimc_itu_cam_ch_t {
	ITU_CAM_A = 1,
	ITU_CAM_B = 0,
};

enum s3c_fimc_scan_t {
	SCAN_TYPE_PROGRESSIVE	= 0,
	SCAN_TYPE_INTERLACE	= 1,
};

enum s3c_fimc_format_t {
	FORMAT_RGB565,
	FORMAT_RGB666,
	FORMAT_RGB888,
	FORMAT_YCBCR420,
	FORMAT_YCBCR422,
};

enum s3c_fimc_flip_t {
	FLIP_ORIGINAL = 0,
	FLIP_X_AXIS = 1,
	FLIP_Y_AXIS = 2,
	FLIP_XY_AXIS = 3,
};

enum s3c_fimc_path_in_t {
	PATH_IN_ITU_CAMERA,
	PATH_IN_MIPI_CAMERA,
	PATH_IN_DMA,
};

enum s3c_fimc_path_out_t {
	PATH_OUT_DMA,
	PATH_OUT_LCDFIFO,
};

enum s3c_fimc_effect_t {
	EFFECT_ORIGINAL = (0 << 26),
	EFFECT_ARBITRARY = (1 << 26),
	EFFECT_NEGATIVE = (2 << 26),
	EFFECT_ARTFREEZE = (3 << 26),
	EFFECT_EMBOSSING = (4 << 26),
	EFFECT_SILHOUETTE = (5 << 26),
};

enum s3c_fimc_wb_t {
	WB_AUTO = 0,
	WB_INDOOR_3001 = 1,
	WB_OUTDOOR_5100 = 2,
	WB_INDOOR_2000 = 3,
	WB_HALT = 4,
	WB_CLOUDY = 5,
	WB_SUNNY = 6,
};

enum s3c_fimc_i2c_cmd_t {
	I2C_CAM_INIT,
	I2C_CAM_RESOLUTION,	
	I2C_CAM_WB,
};

enum s3c_fimc_cam_res_t {
	CAM_RES_DEFAULT,
	CAM_RES_QSVGA,
	CAM_RES_VGA,
	CAM_RES_SVGA,
	CAM_RES_SXGA,
	CAM_RES_UXGA,
	CAM_RES_MAX,
};


/*
 * F I M C   S T R U C T U R E S
 *
*/

/*
 * struct s3c_fimc_frame_addr
 * @phys_rgb:	physical start address of rgb buffer
 * @phys_y:	physical start address of y buffer
 * @phys_cb:	physical start address of u buffer
 * @phys_cr:	physical start address of v buffer
 * @virt_y:	virtual start address of y buffer
 * @virt_rgb:	virtual start address of rgb buffer
 * @virt_cb:	virtual start address of u buffer
 * @virt_cr:	virtual start address of v buffer
*/
struct s3c_fimc_frame_addr {
	union {
		dma_addr_t	phys_rgb;
		dma_addr_t	phys_y;		
	};

	dma_addr_t		phys_cb;
	dma_addr_t		phys_cr;

	union {
		u8		*virt_rgb;
		u8		*virt_y;
	};

	u8			*virt_cb;
	u8			*virt_cr;
};

/*
 * struct s3c_fimc_window_offset
 * @h1:	left side offset of source
 * @h2: right side offset of source
 * @v1: upper side offset of source
 * @v2: lower side offset of source
*/
struct s3c_fimc_window_offset {
	int	h1;
	int	h2;
	int	v1;
	int	v2;
};

/*
 * struct s3c_fimc_dma_offset
 * @y_h:	y value horizontal offset
 * @y_v:	y value vertical offset
 * @cb_h:	cb value horizontal offset
 * @cb_v:	cb value vertical offset
 * @cr_h:	cr value horizontal offset
 * @cr_v:	cr value vertical offset
 *
*/
struct s3c_fimc_dma_offset {
	int	y_h;
	int	y_v;
	int	cb_h;
	int	cb_v;
	int	cr_h;
	int	cr_v;
};

/*
 * struct s3c_fimc_polarity
 * @pclk:	1 if PCLK polarity is inverse
 * @vsync:	1 if VSYNC polarity is inverse
 * @href:	1 if HREF polarity is inverse
 * @hsync:	1 if HSYNC polarity is inverse
*/
struct s3c_fimc_polarity {
	u32	pclk;
	u32	vsync;
	u32	href;
	u32	hsync;
};

/*
 * struct s3c_fimc_effect
 * @type:	effect type
 * @pat_cb:	cr value when type == arbitrary
 * @pat_cR:	cr value when type == arbitrary
 *
*/
struct s3c_fimc_effect {
	enum s3c_fimc_effect_t	type;
	u8			pat_cb;
	u8			pat_cr;
};

/*
 * struct s3c_fimc_scaler
 * @bypass:		1 when pass the original source with no scaling
 * @hfactor:		horizontal shift factor to scale up/down
 * @vfactor:		vertical shift factor to scale up/down
 * @pre_hratio:		horizontal ratio for pre-scaler
 * @pre_vratio:		vertical ratio for pre-scaler
 * @pre_dst_width:	destination width for pre-scaler
 * @pre_dst_height:	destination height for pre-scaler
 * @scaleup_h:		1 if we have to scale up for the horizontal
 * @scaleup_v:		1 if we have to scale up for the vertical
 * @main_hratio:	horizontal ratio for main scaler
 * @main_vratio:	vertical ratio for main scaler
 * @real_width:		src_width - offset
 * @real_height:	src_height - offset
 * @line_length:	line buffer length from platform_data
 * @zoom_depth:		current zoom depth (0 = original)
*/
struct s3c_fimc_scaler {
	u32		bypass;
	u32		hfactor;
	u32		vfactor;
	u32		pre_hratio;
	u32		pre_vratio;
	u32		pre_dst_width;
	u32		pre_dst_height;
	u32		scaleup_h;
	u32		scaleup_v;
	u32		main_hratio;
	u32		main_vratio;
	u32		real_width;
	u32		real_height;
	u32		line_length;
	u32		zoom_depth;
};

/*
 * struct s3c_fimc_camera: abstraction for input camera
 * @id:			cam id (0-2)
 * @type:		type of camera (ITU or MIPI)
 * @mode:		mode of input source
 * @order422:		YCBCR422 order
 * @clockrate:		camera clockrate
 * @width:		source width
 * @height:		source height
 * @offset:		offset information
 * @polarity		polarity information
 * @reset_type:		reset type (high or low)
 * @reset_delay:	delay time in microseconds (udelay)
 * @client:		i2c client
 * @initialized:	whether already initialized
*/
struct s3c_fimc_camera {
	int				id;
	enum s3c_fimc_cam_t		type;
	enum s3c_fimc_cam_mode_t	mode;
	enum s3c_fimc_order422_cam_t	order422;
	u32				clockrate;
	int				width;
	int				height;
	struct s3c_fimc_window_offset	offset;
	struct s3c_fimc_polarity	polarity;
	struct i2c_client		*client;
	int				initialized;
};

/*
 * struct s3c_fimc_in_frame: abstraction for frame data
 * @addr:		address information of frame data
 * @width:		width
 * @height:		height
 * @offset:		dma offset
 * @format:		pixel format
 * @planes:		YCBCR planes (1, 2 or 3)
 * @order_1p		1plane YCBCR order
 * @order_2p:		2plane YCBCR order
 * @flip:		flip mode
*/
struct s3c_fimc_in_frame {
	u32				buf_size;
	struct s3c_fimc_frame_addr	addr;
	int				width;
	int				height;
	struct s3c_fimc_dma_offset	offset;
	enum s3c_fimc_format_t		format;
	int				planes;
	enum s3c_fimc_order422_in_t	order_1p;
	enum s3c_fimc_2plane_order_t	order_2p;
	enum s3c_fimc_flip_t		flip;
};

/*
 * struct s3c_fimc_out_frame: abstraction for frame data
 * @cfn:		current frame number
 * @buf_size:		1 buffer size
 * @addr[]:		address information of frames
 * @nr_frams:		how many output frames used
 * @skip_frames:	current streamed frames (for capture)
 * @width:		width
 * @height:		height
 * @offset:		offset for output dma
 * @format:		pixel format
 * @planes:		YCBCR planes (1, 2 or 3)
 * @order_1p		1plane YCBCR order
 * @order_2p:		2plane YCBCR order
 * @scan:		output scan method (progressive, interlace)
 * @flip:		flip mode
 * @effect:		output effect
*/
struct s3c_fimc_out_frame {
	int				cfn;
	u32				buf_size;
	struct s3c_fimc_frame_addr	addr[S3C_FIMC_MAX_FRAMES];
	int				nr_frames;
	int				skip_frames;
	int				width;
	int				height;
	struct s3c_fimc_dma_offset	offset;
	enum s3c_fimc_format_t		format;
	int				planes;
	enum s3c_fimc_order422_out_t	order_1p;
	enum s3c_fimc_2plane_order_t	order_2p;
	enum s3c_fimc_scan_t		scan;
	enum s3c_fimc_flip_t		flip;
	struct s3c_fimc_effect		effect;
};

/*
 * struct s3c_fimc_v4l2
*/
struct s3c_fimc_v4l2 {
	struct v4l2_fmtdesc	*fmtdesc;
	struct v4l2_framebuffer	frmbuf;
	struct v4l2_input	*input;
	struct v4l2_output	*output;
	struct v4l2_rect	crop_bounds;
	struct v4l2_rect	crop_defrect;
	struct v4l2_rect	crop_current;
};

/*
 * struct s3c_fimc_control: abstraction for FIMC controller
 * @id:		id number (= minor number)
 * @name:	name for video_device
 * @flag:	status, usage, irq flag (S, U, I flag)
 * @lock:	mutex lock
 * @waitq:	waitqueue
 * @pdata:	platform data
 * @clock:	fimc clock
 * @regs:	virtual address of SFR
 * @in_use:	1 when resource is occupied
 * @irq:	irq number
 * @vd:		video_device
 * @v4l2:	v4l2 info
 * @scaler:	scaler related information
 * @in_type:	type of input
 * @in_cam:	camera structure pointer if input is camera else null
 * @in_frame:	frame structure pointer if input is dma else null
 * @out_type:	type of output
 * @out_frame:	frame structure pointer if output is dma
 * @rot90:	1 if clockwise 90 degree for output
 *
 * @open_lcdfifo:	function pointer to open lcd fifo path (display driver)
 * @close_lcdfifo:	function pointer to close fifo path (display driver)
*/
struct s3c_fimc_control {
	/* general */
	int				id;
	char				name[16];
	u32				flag;
	struct mutex			lock;
	wait_queue_head_t		waitq;
	struct device			*dev;
	struct clk			*clock;	
	void __iomem			*regs;
	atomic_t			in_use;
	int				irq;
	struct video_device		*vd;
	struct s3c_fimc_v4l2		v4l2;
	struct s3c_fimc_scaler		scaler;

	/* input */
	enum s3c_fimc_path_in_t		in_type;
	struct s3c_fimc_camera		*in_cam;
	struct s3c_fimc_in_frame	in_frame;

	/* output */
	enum s3c_fimc_path_out_t	out_type;
	struct s3c_fimc_out_frame	out_frame;
	int				rot90;

	/* functions */
	void (*open_lcdfifo)(int win, int in_yuv, int sel);
	void (*close_lcdfifo)(int win);
};

/*
 * struct s3c_fimc_config
*/
struct s3c_fimc_config {
	struct s3c_fimc_control	ctrl[S3C_FIMC_MAX_CTRLS];
	struct s3c_fimc_camera	*camera[S3C_FIMC_MAX_CAMS];
	struct clk		*cam_clock;
	dma_addr_t		dma_start;
	dma_addr_t		dma_current;
	u32			dma_total;
};


/*
 * V 4 L 2   F I M C   E X T E N S I O N S
 *
*/
#define V4L2_INPUT_TYPE_MEMORY		10
#define V4L2_OUTPUT_TYPE_MEMORY		20
#define V4L2_OUTPUT_TYPE_LCDFIFO	21

#define FORMAT_FLAGS_PACKED		1
#define FORMAT_FLAGS_PLANAR		2

#define V4L2_FMT_IN			0
#define V4L2_FMT_OUT			1

// add sangyub.kim
#define TPATTERN_COLORBAR		1
#define TPATTERN_HORIZONTAL		2
#define TPATTERN_VERTICAL		3

/* FOURCC for FIMC specific */
#define V4L2_PIX_FMT_NV12X		v4l2_fourcc('N', '1', '2', 'X')
#define V4L2_PIX_FMT_NV21X		v4l2_fourcc('N', '2', '1', 'X')
#define V4L2_PIX_FMT_VYUY		v4l2_fourcc('V', 'Y', 'U', 'Y')
#define V4L2_PIX_FMT_NV16		v4l2_fourcc('N', 'V', '1', '6')
#define V4L2_PIX_FMT_NV61		v4l2_fourcc('N', 'V', '6', '1')
#define V4L2_PIX_FMT_NV16X		v4l2_fourcc('N', '1', '6', 'X')
#define V4L2_PIX_FMT_NV61X		v4l2_fourcc('N', '6', '1', 'X')

/* CID extensions */
#define V4L2_CID_ACTIVE_CAMERA		(V4L2_CID_PRIVATE_BASE + 0)
#define V4L2_CID_NR_FRAMES		(V4L2_CID_PRIVATE_BASE + 1)
#define V4L2_CID_RESET			(V4L2_CID_PRIVATE_BASE + 2)
// add sangyub.kim
#define V4L2_CID_TEST_PATTERN		(V4L2_CID_PRIVATE_BASE + 3)
#define V4L2_CID_SCALER_BYPASS		(V4L2_CID_PRIVATE_BASE + 4)
#define V4L2_CID_JPEG_INPUT		(V4L2_CID_PRIVATE_BASE + 5)
//
#define V4L2_CID_OUTPUT_ADDR		(V4L2_CID_PRIVATE_BASE + 10)
#define V4L2_CID_INPUT_ADDR		(V4L2_CID_PRIVATE_BASE + 20)
#define V4L2_CID_INPUT_ADDR_RGB		(V4L2_CID_PRIVATE_BASE + 21)
#define V4L2_CID_INPUT_ADDR_Y		(V4L2_CID_PRIVATE_BASE + 22)
#define V4L2_CID_INPUT_ADDR_CB		(V4L2_CID_PRIVATE_BASE + 23)
#define V4L2_CID_INPUT_ADDR_CBCR	(V4L2_CID_PRIVATE_BASE + 24)
#define V4L2_CID_INPUT_ADDR_CR		(V4L2_CID_PRIVATE_BASE + 25)
#define V4L2_CID_EFFECT_ORIGINAL	(V4L2_CID_PRIVATE_BASE + 30)
#define V4L2_CID_EFFECT_ARBITRARY	(V4L2_CID_PRIVATE_BASE + 31)
#define V4L2_CID_EFFECT_NEGATIVE 	(V4L2_CID_PRIVATE_BASE + 33)
#define V4L2_CID_EFFECT_ARTFREEZE	(V4L2_CID_PRIVATE_BASE + 34)
#define V4L2_CID_EFFECT_EMBOSSING	(V4L2_CID_PRIVATE_BASE + 35)
#define V4L2_CID_EFFECT_SILHOUETTE	(V4L2_CID_PRIVATE_BASE + 36)
#define V4L2_CID_ROTATE_ORIGINAL	(V4L2_CID_PRIVATE_BASE + 40)
#define V4L2_CID_ROTATE_90		(V4L2_CID_PRIVATE_BASE + 41)
#define V4L2_CID_ROTATE_180		(V4L2_CID_PRIVATE_BASE + 42)
#define V4L2_CID_ROTATE_270		(V4L2_CID_PRIVATE_BASE + 43)
#define V4L2_CID_ROTATE_90_HFLIP	(V4L2_CID_PRIVATE_BASE + 44)
#define V4L2_CID_ROTATE_90_VFLIP	(V4L2_CID_PRIVATE_BASE + 45)
#define	V4L2_CID_ZOOM_IN		(V4L2_CID_PRIVATE_BASE + 51)
#define V4L2_CID_ZOOM_OUT		(V4L2_CID_PRIVATE_BASE + 52)


/*
 * E X T E R N S
 *
*/
extern struct s3c_fimc_config s3c_fimc;
extern const struct v4l2_ioctl_ops s3c_fimc_v4l2_ops;
extern struct video_device s3c_fimc_video_device[];

extern struct s3c_platform_fimc *to_fimc_plat(struct device *dev);
extern u8 s3c_fimc_i2c_read(struct i2c_client *client, u8 subaddr);
extern int s3c_fimc_i2c_write(struct i2c_client *client, u8 subaddr, u8 val);
extern void s3c_fimc_i2c_command(struct s3c_fimc_control *ctrl, u32 cmd, int arg);
extern void s3c_fimc_register_camera(struct s3c_fimc_camera *cam);
extern void s3c_fimc_set_active_camera(struct s3c_fimc_control *ctrl, int id);
extern void s3c_fimc_init_camera(struct s3c_fimc_control *ctrl);
extern int s3c_fimc_alloc_input_memory(struct s3c_fimc_in_frame *info, dma_addr_t addr);
extern int s3c_fimc_alloc_output_memory(struct s3c_fimc_out_frame *info);
extern int s3c_fimc_alloc_y_memory(struct s3c_fimc_in_frame *info, dma_addr_t addr);
extern int s3c_fimc_alloc_cb_memory(struct s3c_fimc_in_frame *info, dma_addr_t addr);
extern int s3c_fimc_alloc_cr_memory(struct s3c_fimc_in_frame *info, dma_addr_t addr);
extern void s3c_fimc_free_output_memory(struct s3c_fimc_out_frame *info);
extern int s3c_fimc_set_input_frame(struct s3c_fimc_control *ctrl, struct v4l2_pix_format *fmt);
extern int s3c_fimc_set_output_frame(struct s3c_fimc_control *ctrl, struct v4l2_pix_format *fmt);
extern int s3c_fimc_frame_handler(struct s3c_fimc_control *ctrl);
extern u8 *s3c_fimc_get_current_frame(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_nr_frames(struct s3c_fimc_control *ctrl, int nr);
extern int s3c_fimc_set_scaler_info(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_start_dma(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_stop_dma(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_restart_dma(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_change_resolution(struct s3c_fimc_control *ctrl, enum s3c_fimc_cam_res_t res);
extern int s3c_fimc_check_zoom(struct s3c_fimc_control *ctrl, int type);
extern void s3c_fimc_clear_irq(struct s3c_fimc_control *ctrl);
extern int s3c_fimc_check_fifo(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_select_camera(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_test_pattern(struct s3c_fimc_control *ctrl, int type);// add sangyub.kim
extern void s3c_fimc_set_source_format(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_window_offset(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_reset(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_polarity(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_target_format(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_output_dma(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_enable_lastirq(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_disable_lastirq(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_prescaler(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_scaler(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_start_scaler(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_stop_scaler(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_enable_capture(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_disable_capture(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_effect(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_input_dma(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_start_input_dma(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_stop_input_dma(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_input_path(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_output_path(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_input_address(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_set_output_address(struct s3c_fimc_control *ctrl);
extern int s3c_fimc_get_frame_count(struct s3c_fimc_control *ctrl);

extern void s3c_fimc_wait_frame_end(struct s3c_fimc_control *ctrl);// add sangyub.kim
extern void s3c_fimc_change_effect(struct s3c_fimc_control *ctrl);
extern void s3c_fimc_change_rotate(struct s3c_fimc_control *ctrl);

/* FIMD externs */
extern void s3cfb_enable_local(int win, int in_yuv, int sel);
extern void s3cfb_enable_dma(int win);

#endif /* _S3C_CAMIF_H */
