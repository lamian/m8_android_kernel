#ifndef __S3CTVENC_H_
#define __S3CTVENC_H_

#include "s3c-tvscaler.h"


#define TVENC_IOCTL_MAGIC 'T'

typedef struct {

} s3c_tvenc_info;

#define TV_ON			_IO(TVENC_IOCTL_MAGIC, 0)
#define TV_OFF			_IO(TVENC_IOCTL_MAGIC, 1)
#define SELECT_TV_OUT_FORMAT	_IO(TVENC_IOCTL_MAGIC, 2)

#define TVENC_IOCTL_MAXNR	6

#define TVENC_MINOR  14                     // Just some number

typedef enum {
	OFF,
	ON
} tv_enc_switch_t;

typedef enum {
	NTSC_M,
	PAL_M,
	PAL_BGHID,
	PAL_N,
	PAL_NC,
	PAL_60,
	NTSC_443,
	NTSC_J
} tv_standard_t;

typedef enum {
	QCIF, CIF/*352x288*/, 
	QQVGA, QVGA, VGA, SVGA/*800x600*/, SXGA/*1280x1024*/, UXGA/*1600x1200*/, QXGA/*2048x1536*/,
	WVGA/*854x480*/, HD720/*1280x720*/, HD1080/*1920x1080*/
} img_size_t;

typedef enum {
	BLACKSTRETCH, WHITESTRETCH, BLUESTRETCH
} stretch_color_t;

typedef enum {
	COMPOSITE, S_VIDEO
} tv_conn_type_t;

typedef enum {
	BLACK, BLUE, RED, MAGENTA, GREEN, CYAN, YELLOW, WHITE
} bg_color_t;

typedef enum {
	MUTE_Y, MUTE_CB, MUTE_CR
} mute_type_t;

typedef enum {
	AGC4L, AGC2L, N01, N02, P01, P02
} macro_pattern_t;

typedef enum {
	LCD_RGB, LCD_TV, LCD_I80F, LCD_I80S,
	LCD_TVRGB, LCD_TVI80F, LCD_TVI80S	
} lcd_local_output_t;

/* when  App want to change v4l2 parameter,
 * we instantly store it into v4l2_t v2 
 * and then reflect it to hardware
 */	
typedef struct v4l2 {
	struct v4l2_fmtdesc     *fmtdesc;
//	struct v4l2_framebuffer  frmbuf; /* current frame buffer */
	struct v4l2_pix_format	pixfmt;
	struct v4l2_input        *input;
	struct v4l2_output        *output;
//	enum v4l2_status         status;
} v4l2_t;


typedef struct {
	tv_standard_t sig_type;
	tv_conn_type_t connect;
	/* Width of input image. The input value is twice original output image
	*  width. For example, you must set 1440 when the image width is 720.
	*  Max value is 1440
	*/
	unsigned int in_width;
	/* Height of input image
	*  Max value is 576
	*/
	unsigned int in_height;

	// Setting value of VIDOUT[28:26] in Display
	// controller(VIDCON0)
	lcd_local_output_t lcd_output_mode;
	// Set CLKVAL_F[13:6] of VIDCON0 with
	// this value
	unsigned int lcd_clkval_f;

	// Flag of lcd rgb port
	// 0 : disable, 1 : enable
	unsigned int lcd_rgb_port_flag;

	scaler_params_t sp;

	struct video_device      *v;
	v4l2_t v2;
		
} tv_out_params_t;

#define V4L2_INPUT_TYPE_MSDMA		3
#define V4L2_INPUT_TYPE_FIFO		4
#define V4L2_OUTPUT_TYPE_MSDMA		4

#define FORMAT_FLAGS_DITHER       0x01
#define FORMAT_FLAGS_PACKED       0x02
#define FORMAT_FLAGS_PLANAR       0x04
#define FORMAT_FLAGS_RAW          0x08
#define FORMAT_FLAGS_CrCb         0x10

/****************************************************************
* struct v4l2_control
* Control IDs defined by S3C
*****************************************************************/

/* TV-OUT connector type */
#define V4L2_CID_CONNECT_TYPE		(V4L2_CID_PRIVATE_BASE+0)

/****************************************************************
*	I O C T L   C O D E S   F O R   V I D E O   D E V I C E S
*    	 It's only for S3C
*****************************************************************/
#define VIDIOC_S_TVOUT_ON 		_IO ('V', BASE_VIDIOC_PRIVATE+0)
#define VIDIOC_S_TVOUT_OFF		_IO ('V', BASE_VIDIOC_PRIVATE+1)
#define VIDIOC_S_SCALER_TEST 		_IO ('V', BASE_VIDIOC_PRIVATE+3)


extern void s3c_tvscaler_config(scaler_params_t * sp);
extern void s3c_tvscaler_int_enable(unsigned int int_type);
extern void s3c_tvscaler_int_disable(void);
extern void s3c_tvscaler_start(void);
extern void s3c_tvscaler_stop_freerun(void);
extern void s3c_tvscaler_init(void);
extern void s3c_tvscaler_set_interlace(unsigned int on_off);
extern int video_exclusive_release(struct inode * inode, struct file * file);
extern int video_exclusive_open(struct inode * inode, struct file * file);

#if 0	// peter for 2.6.21 kernel
extern void s3c_fb_start_lcd(void);
extern void s3c_fb_stop_lcd(void);
extern void s3c_fb_set_output_path(int out);
extern void s3c_fb_set_clkval(unsigned int clkval);
extern void s3c_fb_enable_rgbport(unsigned int on_off);
#else	// peter for 2.6.24 kernel
extern void s3cfb_start_lcd(void);
extern void s3cfb_stop_lcd(void);
extern void s3cfb_set_output_path(int out);
extern void s3cfb_set_clock(unsigned int clkval);
extern void s3cfb_enable_rgbport(unsigned int on_off);
#endif


#endif // __S3CTVENC_H_
