/* linux/drivers/media/video/samsung/s3c_fimc_v4l2.c
 *
 * V4L2 interface support file for Samsung Camera Interface (FIMC) driver
 *
 * Jinsung Yang, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/videodev2.h>
#include <media/v4l2-ioctl.h>

#include "s3c_fimc.h"

static struct v4l2_input s3c_fimc_input_types[] = {
	{
		.index		= 0,
		.name		= "External Camera Input",
		.type		= V4L2_INPUT_TYPE_CAMERA,
		.audioset	= 1,
		.tuner		= 0,
		.std		= V4L2_STD_PAL_BG | V4L2_STD_NTSC_M,
		.status		= 0,
	}, 
	{
		.index		= 1,
		.name		= "Memory Input",
		.type		= V4L2_INPUT_TYPE_MEMORY,
		.audioset	= 2,
		.tuner		= 0,
		.std		= V4L2_STD_PAL_BG | V4L2_STD_NTSC_M,
		.status		= 0,
	}
};

static struct v4l2_output s3c_fimc_output_types[] = {
	{
		.index		= 0,
		.name		= "Memory Output",
		.type		= V4L2_OUTPUT_TYPE_MEMORY,
		.audioset	= 0,
		.modulator	= 0, 
		.std		= 0,
	}, 
	{
		.index		= 1,
		.name		= "LCD FIFO Output",
		.type		= V4L2_OUTPUT_TYPE_LCDFIFO,
		.audioset	= 0,
		.modulator	= 0,
		.std		= 0,
	} 
};

const static struct v4l2_fmtdesc s3c_fimc_capture_formats[] = {
	{
		.index		= 0,
		.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags		= FORMAT_FLAGS_PLANAR,
		.description	= "4:2:0, planar, Y-Cb-Cr",
		.pixelformat	= V4L2_PIX_FMT_YUV420,
	},
	{
		.index		= 1,
		.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags		= FORMAT_FLAGS_PLANAR,
		.description	= "4:2:2, planar, Y-Cb-Cr",
		.pixelformat	= V4L2_PIX_FMT_YUV422P,

	},	
	{
		.index		= 2,
		.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags		= FORMAT_FLAGS_PACKED,
		.description	= "4:2:2, packed, YCBYCR",
		.pixelformat	= V4L2_PIX_FMT_YUYV,
	},
	{
		.index		= 3,
		.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags		= FORMAT_FLAGS_PACKED,
		.description	= "4:2:2, packed, CBYCRY",
		.pixelformat	= V4L2_PIX_FMT_UYVY,
	}
};

const static struct v4l2_fmtdesc s3c_fimc_overlay_formats[] = {
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
	//add sangyub.kim
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

#define S3C_FIMC_MAX_INPUT_TYPES	ARRAY_SIZE(s3c_fimc_input_types)
#define S3C_FIMC_MAX_OUTPUT_TYPES	ARRAY_SIZE(s3c_fimc_output_types)
#define S3C_FIMC_MAX_CAPTURE_FORMATS	ARRAY_SIZE(s3c_fimc_capture_formats)
#define S3C_FIMC_MAX_OVERLAY_FORMATS	ARRAY_SIZE(s3c_fimc_overlay_formats)

static int s3c_fimc_v4l2_querycap(struct file *filp, void *fh,
					struct v4l2_capability *cap)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;

	strcpy(cap->driver, "Samsung FIMC Driver");
	strlcpy(cap->card, ctrl->vd->name, sizeof(cap->card));
	sprintf(cap->bus_info, "FIMC AHB-bus");

	cap->version = 0;
	cap->capabilities = (V4L2_CAP_VIDEO_OVERLAY | \
				V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING);

	return 0;
}

static int s3c_fimc_v4l2_g_fbuf(struct file *filp, void *fh,
					struct v4l2_framebuffer *fb)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;

	*fb = ctrl->v4l2.frmbuf;

	fb->base = ctrl->v4l2.frmbuf.base;
	fb->capability = V4L2_FBUF_CAP_LIST_CLIPPING;

	fb->fmt.pixelformat  = ctrl->v4l2.frmbuf.fmt.pixelformat;
	fb->fmt.width = ctrl->v4l2.frmbuf.fmt.width;
	fb->fmt.height = ctrl->v4l2.frmbuf.fmt.height;
	fb->fmt.bytesperline = ctrl->v4l2.frmbuf.fmt.bytesperline;

	return 0;
}

static int s3c_fimc_v4l2_s_fbuf(struct file *filp, void *fh,
					struct v4l2_framebuffer *fb)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;
	struct v4l2_framebuffer *frmbuf = &(ctrl->v4l2.frmbuf);
	int i, bpp;

	for (i = 0; i < S3C_FIMC_MAX_OVERLAY_FORMATS; i++) {
		if (s3c_fimc_overlay_formats[i].pixelformat == fb->fmt.pixelformat)
			break;
	}

	if (i == S3C_FIMC_MAX_OVERLAY_FORMATS)
		return -EINVAL;

	bpp = s3c_fimc_set_output_frame(ctrl, &fb->fmt);
	

	frmbuf->base  = fb->base;
	frmbuf->flags = fb->flags;
	frmbuf->capability = fb->capability;
	frmbuf->fmt.width = fb->fmt.width;
	frmbuf->fmt.height = fb->fmt.height;
	frmbuf->fmt.field = fb->fmt.field;
	frmbuf->fmt.pixelformat = fb->fmt.pixelformat;
	frmbuf->fmt.bytesperline = fb->fmt.width * bpp / 8;
	frmbuf->fmt.sizeimage = fb->fmt.width * frmbuf->fmt.bytesperline;

	return 0;
}

static int s3c_fimc_v4l2_enum_fmt_vid_cap(struct file *filp, void *fh,
					struct v4l2_fmtdesc *f)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;
	int index = f->index;

	if (index >= S3C_FIMC_MAX_CAPTURE_FORMATS)
		return -EINVAL;

	memset(f, 0, sizeof(*f));
	memcpy(f, ctrl->v4l2.fmtdesc + index, sizeof(*f));

	return 0;
}

static int s3c_fimc_v4l2_g_fmt_vid_cap(struct file *filp, void *fh,
					struct v4l2_format *f)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;
	int size = sizeof(struct v4l2_pix_format);

	memset(&f->fmt.pix, 0, size);
	memcpy(&f->fmt.pix, &(ctrl->v4l2.frmbuf.fmt), size);

	return 0;
}

static int s3c_fimc_v4l2_s_fmt_vid_cap(struct file *filp, void *fh,
					struct v4l2_format *f)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;

	ctrl->v4l2.frmbuf.fmt = f->fmt.pix;

	if (f->fmt.pix.priv == V4L2_FMT_IN)
		s3c_fimc_set_input_frame(ctrl, &f->fmt.pix);
	else
		s3c_fimc_set_output_frame(ctrl, &f->fmt.pix);

	return 0;
}

static int s3c_fimc_v4l2_try_fmt_vid_cap(struct file *filp, void *fh,
					  struct v4l2_format *f)
{
	return 0;
}

static int s3c_fimc_v4l2_try_fmt_overlay(struct file *filp, void *fh,
					  struct v4l2_format *f)
{
	return 0;
}

static int s3c_fimc_v4l2_overlay(struct file *filp, void *fh, unsigned int i)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;

	if (i) {
		if (ctrl->in_type != PATH_IN_DMA)
			s3c_fimc_init_camera(ctrl);

		FSET_PREVIEW(ctrl);
		s3c_fimc_start_dma(ctrl);
	} else {
		s3c_fimc_stop_dma(ctrl);

		if (ctrl->out_type != PATH_OUT_LCDFIFO) {
			s3c_fimc_free_output_memory(&ctrl->out_frame);
			s3c_fimc_set_output_address(ctrl);
		}
	}
	
	return 0;
}

static int s3c_fimc_v4l2_g_ctrl(struct file *filp, void *fh,
					struct v4l2_control *c)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;

	switch (c->id) {
	case V4L2_CID_OUTPUT_ADDR:
		c->value = frame->addr[c->value].phys_y;
		break;

	default:
		err("invalid control id: %d\n", c->id);
		return -EINVAL;
	}
	
	return 0;
}

static int s3c_fimc_v4l2_s_ctrl(struct file *filp, void *fh,
					struct v4l2_control *c)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	struct s3c_fimc_window_offset *offset = &ctrl->in_cam->offset;

	switch (c->id) {
	case V4L2_CID_EFFECT_ORIGINAL:
		frame->effect.type = EFFECT_ORIGINAL;
		s3c_fimc_change_effect(ctrl);
		break;

	case V4L2_CID_EFFECT_NEGATIVE:
		frame->effect.type = EFFECT_NEGATIVE;
		s3c_fimc_change_effect(ctrl);
		break;

	case V4L2_CID_EFFECT_EMBOSSING:
		frame->effect.type = EFFECT_EMBOSSING;
		s3c_fimc_change_effect(ctrl);
		break;

	case V4L2_CID_EFFECT_ARTFREEZE:
		frame->effect.type = EFFECT_ARTFREEZE;
		s3c_fimc_change_effect(ctrl);
		break;

	case V4L2_CID_EFFECT_SILHOUETTE:
		frame->effect.type = EFFECT_SILHOUETTE;
		s3c_fimc_change_effect(ctrl);
		break;

	case V4L2_CID_EFFECT_ARBITRARY:
		frame->effect.type = EFFECT_ARBITRARY;
		
		frame->effect.pat_cb = PAT_CB(c->value);
		frame->effect.pat_cr = PAT_CR(c->value);
		
		s3c_fimc_change_effect(ctrl);
		break;

	case V4L2_CID_ROTATE_ORIGINAL:
		frame->flip = FLIP_ORIGINAL;
		ctrl->rot90 = 0;
		s3c_fimc_change_rotate(ctrl);
		break;

	case V4L2_CID_HFLIP:
		frame->flip = FLIP_X_AXIS;
		ctrl->rot90 = 0;
		s3c_fimc_change_rotate(ctrl);
		break;

	case V4L2_CID_VFLIP:
		frame->flip = FLIP_Y_AXIS;
		ctrl->rot90 = 0;
		s3c_fimc_change_rotate(ctrl);
		break;

	case V4L2_CID_ROTATE_180:
		frame->flip = FLIP_XY_AXIS;
		ctrl->rot90 = 0;
		s3c_fimc_change_rotate(ctrl);
		break;

	case V4L2_CID_ROTATE_90:
		frame->flip = FLIP_ORIGINAL;
		ctrl->rot90 = 1;
		s3c_fimc_change_rotate(ctrl);
		break;

	case V4L2_CID_ROTATE_270:
		frame->flip = FLIP_XY_AXIS;
		ctrl->rot90 = 1;
		s3c_fimc_change_rotate(ctrl);
		break;

	case V4L2_CID_ROTATE_90_HFLIP:
		frame->flip = FLIP_X_AXIS;
		ctrl->rot90 = 1;
		s3c_fimc_change_rotate(ctrl);
		break;

	case V4L2_CID_ROTATE_90_VFLIP:
		frame->flip = FLIP_Y_AXIS;
		ctrl->rot90 = 1;
		s3c_fimc_change_rotate(ctrl);
		break;

	case V4L2_CID_ZOOM_IN:
		if (s3c_fimc_check_zoom(ctrl, c->id) == 0) {
			offset->h1 += S3C_FIMC_ZOOM_PIXELS;
			offset->h2 += S3C_FIMC_ZOOM_PIXELS;
			offset->v1 += S3C_FIMC_ZOOM_PIXELS;
			offset->v2 += S3C_FIMC_ZOOM_PIXELS;
			s3c_fimc_restart_dma(ctrl);
		}

		break;

	case V4L2_CID_ZOOM_OUT:
		if (s3c_fimc_check_zoom(ctrl, c->id) == 0) {
			offset->h1 -= S3C_FIMC_ZOOM_PIXELS;
			offset->h2 -= S3C_FIMC_ZOOM_PIXELS;
			offset->v1 -= S3C_FIMC_ZOOM_PIXELS;
			offset->v2 -= S3C_FIMC_ZOOM_PIXELS;
			s3c_fimc_restart_dma(ctrl);
		}

		break;

	case V4L2_CID_AUTO_WHITE_BALANCE:
		s3c_fimc_i2c_command(ctrl, I2C_CAM_WB, c->value);
		break;

	case V4L2_CID_ACTIVE_CAMERA:
		s3c_fimc_set_active_camera(ctrl, c->value);
		s3c_fimc_i2c_command(ctrl, I2C_CAM_WB, WB_AUTO);
		break;

	case V4L2_CID_TEST_PATTERN:
		s3c_fimc_set_active_camera(ctrl, S3C_FIMC_TPID);
		s3c_fimc_set_test_pattern(ctrl, c->value);
		break;

	case V4L2_CID_NR_FRAMES:
		s3c_fimc_set_nr_frames(ctrl, c->value);
		break;

	case V4L2_CID_INPUT_ADDR:
		s3c_fimc_alloc_input_memory(&ctrl->in_frame, \
						(dma_addr_t) c->value);
		s3c_fimc_set_input_address(ctrl);
		break;

	case V4L2_CID_INPUT_ADDR_Y:
	case V4L2_CID_INPUT_ADDR_RGB:
		s3c_fimc_alloc_y_memory(&ctrl->in_frame, \
						(dma_addr_t) c->value);
		s3c_fimc_set_input_address(ctrl);
		break;

	case V4L2_CID_INPUT_ADDR_CB:	/* fall through */
	case V4L2_CID_INPUT_ADDR_CBCR:
		s3c_fimc_alloc_cb_memory(&ctrl->in_frame, \
						(dma_addr_t) c->value);
		s3c_fimc_set_input_address(ctrl);
		break;

	case V4L2_CID_INPUT_ADDR_CR:
		s3c_fimc_alloc_cr_memory(&ctrl->in_frame, \
						(dma_addr_t) c->value);
		s3c_fimc_set_input_address(ctrl);
		break;

	case V4L2_CID_RESET:
		ctrl->rot90 = 0;
		ctrl->in_frame.flip = FLIP_ORIGINAL;
		ctrl->out_frame.flip = FLIP_ORIGINAL;
		ctrl->out_frame.effect.type = EFFECT_ORIGINAL;
		ctrl->scaler.bypass = 0;
		s3c_fimc_reset(ctrl);
		break;

	case V4L2_CID_JPEG_INPUT:	/* fall through */
	case V4L2_CID_SCALER_BYPASS:
		ctrl->scaler.bypass = 1;
		break;

	default:
		err("invalid control id: %d\n", c->id);
		return -EINVAL;
	}

	return 0;
}

static int s3c_fimc_v4l2_streamon(struct file *filp, void *fh,
					enum v4l2_buf_type i)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;
	
	if (i != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	if (ctrl->in_type != PATH_IN_DMA)
		s3c_fimc_init_camera(ctrl);

	ctrl->out_frame.skip_frames = 0;
	FSET_CAPTURE(ctrl);
	FSET_IRQ_NORMAL(ctrl);
	s3c_fimc_start_dma(ctrl);

	return 0;
}

static int s3c_fimc_v4l2_streamoff(struct file *filp, void *fh,
					enum v4l2_buf_type i)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;
	
	if (i != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	FSET_STOP(ctrl);
	UNMASK_USAGE(ctrl);
	UNMASK_IRQ(ctrl);

	s3c_fimc_stop_dma(ctrl);
	s3c_fimc_free_output_memory(&ctrl->out_frame);
	s3c_fimc_set_output_address(ctrl);

	return 0;
}

static int s3c_fimc_v4l2_g_input(struct file *filp, void *fh,
					unsigned int *i)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;

	*i = ctrl->v4l2.input->index;

	return 0;
}

static int s3c_fimc_v4l2_s_input(struct file *filp, void *fh,
					unsigned int i)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;

	if (i >= S3C_FIMC_MAX_INPUT_TYPES)
		return -EINVAL;

	ctrl->v4l2.input = &s3c_fimc_input_types[i];

	if (s3c_fimc_input_types[i].type == V4L2_INPUT_TYPE_CAMERA)
		ctrl->in_type = PATH_IN_ITU_CAMERA;
	else
		ctrl->in_type = PATH_IN_DMA;

	return 0;
}

static int s3c_fimc_v4l2_g_output(struct file *filp, void *fh,
					unsigned int *i)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;

	*i = ctrl->v4l2.output->index;

	return 0;
}

static int s3c_fimc_v4l2_s_output(struct file *filp, void *fh,
					unsigned int i)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;

	if (i >= S3C_FIMC_MAX_OUTPUT_TYPES)
		return -EINVAL;

	ctrl->v4l2.output = &s3c_fimc_output_types[i];

	if (s3c_fimc_output_types[i].type == V4L2_OUTPUT_TYPE_MEMORY)
		ctrl->out_type = PATH_OUT_DMA;
	else
		ctrl->out_type = PATH_OUT_LCDFIFO;

	return 0;
}

static int s3c_fimc_v4l2_enum_input(struct file *filp, void *fh,
					struct v4l2_input *i)
{
	if (i->index >= S3C_FIMC_MAX_INPUT_TYPES)
		return -EINVAL;

	memcpy(i, &s3c_fimc_input_types[i->index], sizeof(struct v4l2_input));

	return 0;
}

static int s3c_fimc_v4l2_enum_output(struct file *filp, void *fh,
					struct v4l2_output *o)
{
	if ((o->index) >= S3C_FIMC_MAX_OUTPUT_TYPES)
		return -EINVAL;

	memcpy(o, &s3c_fimc_output_types[o->index], sizeof(struct v4l2_output));

	return 0;
}

static int s3c_fimc_v4l2_reqbufs(struct file *filp, void *fh,
					struct v4l2_requestbuffers *b)
{
	if (b->memory != V4L2_MEMORY_MMAP) {
		err("V4L2_MEMORY_MMAP is only supported\n");
		return -EINVAL;
	}

	/* control user input */
	if (b->count > 4)
		b->count = 4;
	else if (b->count < 1)
		b->count = 1;

	return 0;
}

static int s3c_fimc_v4l2_querybuf(struct file *filp, void *fh,
					struct v4l2_buffer *b)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;
	
	if (b->type != V4L2_BUF_TYPE_VIDEO_OVERLAY && \
		b->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	if (b->memory != V4L2_MEMORY_MMAP)
		return -EINVAL;

	b->length = ctrl->out_frame.buf_size;

	/*
	 * NOTE: we use the m.offset as an index for multiple frames out.
	 * Because all frames are not contiguous, we cannot use it as
	 * original purpose.
	 * The index value used to find out which frame user wants to mmap.
	 */
	b->m.offset = b->index * PAGE_SIZE;

	return 0;
}

static int s3c_fimc_v4l2_qbuf(struct file *filp, void *fh,
				struct v4l2_buffer *b)
{
	return 0;
}

static int s3c_fimc_v4l2_dqbuf(struct file *filp, void *fh,
				struct v4l2_buffer *b)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;

	ctrl->out_frame.cfn = s3c_fimc_get_frame_count(ctrl);
	b->index = (frame->cfn + 2) % frame->nr_frames;


	return 0;
}

static int s3c_fimc_v4l2_cropcap(struct file *filp, void *fh,
					struct v4l2_cropcap *a)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
		a->type != V4L2_BUF_TYPE_VIDEO_OVERLAY)
		return -EINVAL;

	/* crop limitations */
	ctrl->v4l2.crop_bounds.left = 0;
	ctrl->v4l2.crop_bounds.top = 0;
	ctrl->v4l2.crop_bounds.width = ctrl->in_cam->width;
	ctrl->v4l2.crop_bounds.height = ctrl->in_cam->height;

	/* crop default values */
	ctrl->v4l2.crop_defrect.left = \
			(ctrl->in_cam->width - S3C_FIMC_CROP_DEF_WIDTH) / 2;

	ctrl->v4l2.crop_defrect.top = \
			(ctrl->in_cam->height - S3C_FIMC_CROP_DEF_HEIGHT) / 2;

	ctrl->v4l2.crop_defrect.width = S3C_FIMC_CROP_DEF_WIDTH;
	ctrl->v4l2.crop_defrect.height = S3C_FIMC_CROP_DEF_HEIGHT;

	a->bounds = ctrl->v4l2.crop_bounds;
	a->defrect = ctrl->v4l2.crop_defrect;

	return 0;
}

static int s3c_fimc_v4l2_g_crop(struct file *filp, void *fh,
				struct v4l2_crop *a)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
		a->type != V4L2_BUF_TYPE_VIDEO_OVERLAY)
		return -EINVAL;

	a->c = ctrl->v4l2.crop_current;

	return 0;
}

static int s3c_fimc_v4l2_s_crop(struct file *filp, void *fh,
				struct v4l2_crop *a)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;
	struct s3c_fimc_camera *cam = ctrl->in_cam;

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
		a->type != V4L2_BUF_TYPE_VIDEO_OVERLAY)
		return -EINVAL;

	if (a->c.height < 0)
		return -EINVAL;

	if (a->c.width < 0)
		return -EINVAL;

	if ((a->c.left + a->c.width > cam->width) || \
		(a->c.top + a->c.height > cam->height))
		return -EINVAL;

	ctrl->v4l2.crop_current = a->c;

	cam->offset.h1 = (cam->width - a->c.width) / 2;
	cam->offset.v1 = (cam->height - a->c.height) / 2;

	cam->offset.h2 = cam->offset.h1;
	cam->offset.v2 = cam->offset.v1;

	s3c_fimc_restart_dma(ctrl);

	return 0;
}

static int s3c_fimc_v4l2_s_parm(struct file *filp, void *fh,
				struct v4l2_streamparm *a)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) fh;

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	if (a->parm.capture.capturemode == V4L2_MODE_HIGHQUALITY) {
		info("changing to max resolution\n");
		s3c_fimc_change_resolution(ctrl, CAM_RES_MAX);
	} else {
		info("changing to default resolution\n");
		s3c_fimc_change_resolution(ctrl, CAM_RES_DEFAULT);
	}

	s3c_fimc_restart_dma(ctrl);

	return 0;
}

const struct v4l2_ioctl_ops s3c_fimc_v4l2_ops = {
	.vidioc_querycap		= s3c_fimc_v4l2_querycap,
	.vidioc_g_fbuf			= s3c_fimc_v4l2_g_fbuf,
	.vidioc_s_fbuf			= s3c_fimc_v4l2_s_fbuf,
	.vidioc_enum_fmt_vid_cap	= s3c_fimc_v4l2_enum_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap		= s3c_fimc_v4l2_g_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap		= s3c_fimc_v4l2_s_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap		= s3c_fimc_v4l2_try_fmt_vid_cap,
	.vidioc_try_fmt_vid_overlay	= s3c_fimc_v4l2_try_fmt_overlay,
	.vidioc_overlay			= s3c_fimc_v4l2_overlay,
	.vidioc_g_ctrl			= s3c_fimc_v4l2_g_ctrl,
	.vidioc_s_ctrl			= s3c_fimc_v4l2_s_ctrl,
	.vidioc_streamon		= s3c_fimc_v4l2_streamon,
	.vidioc_streamoff		= s3c_fimc_v4l2_streamoff,
	.vidioc_g_input			= s3c_fimc_v4l2_g_input,
	.vidioc_s_input			= s3c_fimc_v4l2_s_input,
	.vidioc_g_output		= s3c_fimc_v4l2_g_output,
	.vidioc_s_output		= s3c_fimc_v4l2_s_output,
	.vidioc_enum_input		= s3c_fimc_v4l2_enum_input,
	.vidioc_enum_output		= s3c_fimc_v4l2_enum_output,
	.vidioc_reqbufs			= s3c_fimc_v4l2_reqbufs,
	.vidioc_querybuf		= s3c_fimc_v4l2_querybuf,
	.vidioc_qbuf			= s3c_fimc_v4l2_qbuf,
	.vidioc_dqbuf			= s3c_fimc_v4l2_dqbuf,
	.vidioc_cropcap			= s3c_fimc_v4l2_cropcap,
	.vidioc_g_crop			= s3c_fimc_v4l2_g_crop,
	.vidioc_s_crop			= s3c_fimc_v4l2_s_crop,
	.vidioc_s_parm			= s3c_fimc_v4l2_s_parm,
};
