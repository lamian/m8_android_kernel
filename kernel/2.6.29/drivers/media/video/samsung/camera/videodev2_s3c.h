#ifndef __VIDEODEV2_S3C_H_
#define __VIDEODEV2_S3C_H_

#include <linux/videodev2.h>

#define V4L2_INPUT_TYPE_MSDMA		3
#define V4L2_INPUT_TYPE_INTERLACE	4

/****************************************************************
* struct v4l2_control
* Control IDs defined by S3C
*****************************************************************/
/* Image Effect */
#define V4L2_CID_ORIGINAL		(V4L2_CID_PRIVATE_BASE + 0)
#define V4L2_CID_ARBITRARY		(V4L2_CID_PRIVATE_BASE + 1)
#define V4L2_CID_NEGATIVE 		(V4L2_CID_PRIVATE_BASE + 2)
#define V4L2_CID_ART_FREEZE		(V4L2_CID_PRIVATE_BASE + 3)
#define V4L2_CID_EMBOSSING		(V4L2_CID_PRIVATE_BASE + 4)
#define V4L2_CID_SILHOUETTE		(V4L2_CID_PRIVATE_BASE + 5)

/* Image Rotate */
#define V4L2_CID_ROTATE_90		(V4L2_CID_PRIVATE_BASE + 6)
#define V4L2_CID_ROTATE_180		(V4L2_CID_PRIVATE_BASE + 7)
#define V4L2_CID_ROTATE_270		(V4L2_CID_PRIVATE_BASE + 8)
#define V4L2_CID_ROTATE_BYPASS	(V4L2_CID_PRIVATE_BASE + 9)

/* Zoom-in, Zoom-out */
#define	V4L2_CID_ZOOMIN			(V4L2_CID_PRIVATE_BASE + 10)
#define V4L2_CID_ZOOMOUT		(V4L2_CID_PRIVATE_BASE + 11)

/* Flash Control */
#define	V4L2_CID_FLASH_CAMERA	(V4L2_CID_PRIVATE_BASE + 12)
#define V4L2_CID_FLASH_MOVIE	(V4L2_CID_PRIVATE_BASE + 13)

/* Set Mode */
#define V4L2_CID_MODE_SET   	(V4L2_CID_PRIVATE_BASE + 14)


/****************************************************************
*	I O C T L   C O D E S   F O R   V I D E O   D E V I C E S
*    	 It's only for S3C
*****************************************************************/
#define VIDIOC_S_CAMERA_START 	_IO  ('V', BASE_VIDIOC_PRIVATE + 0)
#define VIDIOC_S_CAMERA_STOP	_IO  ('V', BASE_VIDIOC_PRIVATE + 1)
#define VIDIOC_MSDMA_START		_IOW ('V', BASE_VIDIOC_PRIVATE + 2, struct v4l2_msdma_format)
#define VIDIOC_MSDMA_STOP		_IOW ('V', BASE_VIDIOC_PRIVATE + 3, struct v4l2_msdma_format)
#define VIDIOC_S_MSDMA			_IOW ('V', BASE_VIDIOC_PRIVATE + 4, struct v4l2_msdma_format)
#define VIDIOC_S_INTERLACE_MODE _IOW ('V', BASE_VIDIOC_PRIVATE + 5, struct v4l2_interlace_format)

/*
 *	INTERLACE MODE
 */
#define	S3C_VIDEO_DECODER_PAL		1	/* can decode PAL signal */
#define	S3C_VIDEO_DECODER_NTSC		2	/* can decode NTSC */
#define	S3C_VIDEO_DECODER_SECAM		4	/* can decode SECAM */
#define	S3C_VIDEO_DECODER_AUTO		8	/* can autosense norm */
#define	S3C_VIDEO_DECODER_CCIR		16	/* CCIR-601 pixel rate (720 pixels per line) instead of square pixel rate */

#define S3C_DECODER_INIT				_IOW ('V', BASE_VIDIOC_PRIVATE + 14, struct s3c_video_decoder_init)	/* init internal registers at once */
#define	S3C_DECODER_GET_CAPABILITIES	_IOR ('V', BASE_VIDIOC_PRIVATE + 6,  struct s3c_video_decoder_capability)
#define	S3C_DECODER_GET_STATUS    		_IOR ('V', BASE_VIDIOC_PRIVATE + 7,  int)
#define	S3C_DECODER_SET_NORM			_IOW ('V', BASE_VIDIOC_PRIVATE + 8,  int)
#define	S3C_DECODER_SET_INPUT			_IOW ('V', BASE_VIDIOC_PRIVATE + 9,  int)					/* 0 <= input < #inputs */
#define	S3C_DECODER_SET_OUTPUT			_IOW ('V', BASE_VIDIOC_PRIVATE + 10, int)					/* 0 <= output < #outputs */
#define	S3C_DECODER_ENABLE_OUTPUT		_IOW ('V', BASE_VIDIOC_PRIVATE + 11, int)					/* boolean output enable control */
#define	S3C_DECODER_SET_PICTURE   		_IOW ('V', BASE_VIDIOC_PRIVATE + 12, struct video_picture)
#define	S3C_DECODER_SET_GPIO			_IOW ('V', BASE_VIDIOC_PRIVATE + 13, int)					/* switch general purpose pin */
#define	S3C_DECODER_SET_VBI_BYPASS		_IOW ('V', BASE_VIDIOC_PRIVATE + 15, int)					/* switch vbi bypass */
#define	S3C_DECODER_DUMP				_IO  ('V', BASE_VIDIOC_PRIVATE + 16)					/* debug hook */

enum v4l2_msdma_input {
	V4L2_MSDMA_CODEC = 1,
	V4L2_MSDMA_PREVIEW = 2,
};

struct v4l2_msdma_format
{
	__u32         		width;		/* MSDMA INPUT : Source X size */
	__u32			height;		/* MSDMA INPUT : Source Y size */
	__u32			pixelformat;
	enum v4l2_msdma_input  	input_path;
};

struct v4l2_interlace_format
{
	__u32 width;	/* INTERLACE INPUT : Source X size */
	__u32 height;	/* INTERLACE INPUT : Source Y size */
};

struct s3c_video_decoder_init {
	unsigned char len;
	const unsigned char *data;
};

struct s3c_video_decoder_capability {	/* this name is too long */
	__u32	flags;
	int	inputs;			/* number of inputs */
	int	outputs;		/* number of outputs */
};

const struct v4l2_fmtdesc fimc_codec_formats[] = {
	{
		.index		= 0,
		.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.description	= "16 bpp RGB, le",
		.pixelformat	= V4L2_PIX_FMT_RGB565,
		.flags		= FORMAT_FLAGS_PACKED,
	},
	{
		.index		= 1,
		.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags		= FORMAT_FLAGS_PACKED,
		.description	= "32 bpp RGB, le",
		.pixelformat	= V4L2_PIX_FMT_BGR32,
	},
	{
		.index		= 2,
		.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags		= FORMAT_FLAGS_PLANAR,
		.description	= "4:2:2, planar, Y-Cb-Cr",
		.pixelformat	= V4L2_PIX_FMT_YUV422P,

	},
	{
		.index		= 3,
		.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags		= FORMAT_FLAGS_PLANAR,
		.description	= "4:2:0, planar, Y-Cb-Cr",
		.pixelformat	= V4L2_PIX_FMT_YUV420,
	}
};

const struct v4l2_fmtdesc fimc_preview_formats[] = {
	{
		.index		= 0,
		.type		= V4L2_BUF_TYPE_VIDEO_OVERLAY,
		.flags		= FORMAT_FLAGS_PACKED,
		.description	= "16 bpp RGB, le",
		.pixelformat	= V4L2_PIX_FMT_RGB565,		
	},
	{
		.index		= 1,
		.type		= V4L2_BUF_TYPE_VIDEO_OVERLAY,
		.flags		= FORMAT_FLAGS_PACKED,
		.description	= "24 bpp RGB, le",
		.pixelformat	= V4L2_PIX_FMT_RGB24,		
	},
	{
		.index		= 2,
		.type		= V4L2_BUF_TYPE_VIDEO_OVERLAY,
		.flags		= FORMAT_FLAGS_PACKED,
		.description	= "32 bpp RGB, le",
		.pixelformat	= V4L2_PIX_FMT_BGR32,
	},
	{
		.index		= 3,
		.type		= V4L2_BUF_TYPE_VIDEO_OVERLAY,
		.flags		= FORMAT_FLAGS_PLANAR,
		.description	= "4:2:2, planar, Y-Cb-Cr",
		.pixelformat	= V4L2_PIX_FMT_YUV422P,

	},
	{
		.index		= 4,
		.type		= V4L2_BUF_TYPE_VIDEO_OVERLAY,
		.flags		= FORMAT_FLAGS_PLANAR,
		.description	= "4:2:0, planar, Y-Cb-Cr",
		.pixelformat	= V4L2_PIX_FMT_YUV420,
	}
};

#define NUMBER_OF_PREVIEW_FORMATS  	ARRAY_SIZE(fimc_preview_formats)
#define NUMBER_OF_CODEC_FORMATS		ARRAY_SIZE(fimc_codec_formats)

#define VIDIOC_G_CAMPP				_IOR  ('V', 82, void *)
#define VIDIOC_S_ROTATE_ANGLE		_IOW  ('V', 83, void *)

#define VIDIOC_I2C_WRITE			_IOW  ('V', 84, void *)
#define VIDIOC_I2C_READ				_IOR  ('V', 85, void *)

#define VIDIOC_S_SENSOR				_IOW  ('V', 86, void *)
#define VIDIOC_S_FOCUS_AUTO			_IOW  ('V', 87, void *)
#define VIDIOC_S_DIG_ZOOM			_IOW  ('V', 88, void *)
#define VIDIOC_S_SCENE_MODE			_IOW  ('V', 89, void *)
#define VIDIOC_S_PHOTOMETRY			_IOW  ('V', 90, void *)
#define VIDIOC_S_ISO				_IOW  ('V', 91, void *)
#define VIDIOC_S_CONTRAST			_IOW  ('V', 92, void *)
#define VIDIOC_S_SATURATION			_IOW  ('V', 93, void *)
#define VIDIOC_S_SHARPNESS			_IOW  ('V', 94, void *)
#define VIDIOC_S_WHITE_BALANCE		_IOW  ('V', 95, void *)
#define VIDIOC_S_BRIGHTNESS			_IOW  ('V', 96, void *)
#define VIDIOC_S_EFFECT				_IOW  ('V', 97, void *)
#define VIDIOC_S_FLASH_CAMERA		_IOW  ('V', 98, void *)
#define VIDIOC_S_FLASH_MOVIE		_IOW  ('V', 99, void *)
#define VIDIOC_S_EXIF_DATA			_IOW  ('V',100, void *)

#endif

