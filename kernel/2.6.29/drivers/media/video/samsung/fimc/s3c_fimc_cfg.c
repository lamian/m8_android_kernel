/* linux/drivers/media/video/samsung/s3c_fimc_cfg.c
 *
 * Configuration support file for Samsung Camera Interface (FIMC) driver
 *
 * Jinsung Yang, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/slab.h>
#include <linux/bootmem.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <plat/media.h>

#include "s3c_fimc.h"

#if (CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC > 0)
static dma_addr_t s3c_fimc_get_dma_region(u32 bytes)
{
	dma_addr_t end, addr, *curr;

	end = s3c_fimc.dma_start + s3c_fimc.dma_total;
	curr = &s3c_fimc.dma_current;

	if (*curr + bytes > end) {
		addr = 0;
	} else {
		addr = *curr;
		*curr += bytes;
	}

	return addr;
}

static void s3c_fimc_put_dma_region(u32 bytes)
{
	s3c_fimc.dma_current -= bytes;
}

void s3c_fimc_free_output_memory(struct s3c_fimc_out_frame *info)
{
	struct s3c_fimc_frame_addr *frame;
	int i;

	for (i = 0; i < info->nr_frames; i++) {
		frame = &info->addr[i];

		if (frame->phys_y)
			s3c_fimc_put_dma_region(info->buf_size);

		memset(frame, 0, sizeof(*frame));
	}

	info->buf_size = 0;
}

static int s3c_fimc_alloc_rgb_memory(struct s3c_fimc_out_frame *info)
{
	struct s3c_fimc_frame_addr *frame;
	int i, ret, nr_frames = info->nr_frames;

	for (i = 0; i < nr_frames; i++) {
		frame = &info->addr[i];

		frame->phys_rgb = s3c_fimc_get_dma_region(info->buf_size);
		if (frame->phys_rgb == 0) {
			ret = -ENOMEM;
			goto alloc_fail;
		}
		
		frame->virt_rgb = phys_to_virt(frame->phys_rgb);
		
	}

	for (i = nr_frames; i < S3C_FIMC_MAX_FRAMES; i++) {
		frame = &info->addr[i];
		frame->phys_rgb = info->addr[i - nr_frames].phys_rgb;
		frame->virt_rgb = info->addr[i - nr_frames].virt_rgb;		
	}

	return 0;

alloc_fail:
	s3c_fimc_free_output_memory(info);
	return ret;
}

static int s3c_fimc_alloc_yuv_memory(struct s3c_fimc_out_frame *info)
{


	struct s3c_fimc_frame_addr *frame;
	int i, ret, nr_frames = info->nr_frames;
	u32 size = info->width * info->height, cbcr_size;
	
	if (info->format == FORMAT_YCBCR420)
		cbcr_size = size / 4;
	else
		cbcr_size = size / 2;
	
	

	for (i = 0; i < nr_frames; i++) {
		frame = &info->addr[i];

		frame->phys_y = s3c_fimc_get_dma_region(info->buf_size);
		if (frame->phys_y == 0) {
			ret = -ENOMEM;
			goto alloc_fail;
		}

		frame->phys_cb = frame->phys_y + size;
		frame->phys_cr = frame->phys_cb + cbcr_size;
		frame->virt_y = phys_to_virt(frame->phys_y);
		frame->virt_cb = frame->virt_y + size;
		frame->virt_cr = frame->virt_cb + cbcr_size;
	
		
	}

	for (i = nr_frames; i < S3C_FIMC_MAX_FRAMES; i++) {
		frame = &info->addr[i];
		frame->phys_y = info->addr[i - nr_frames].phys_y;
		frame->phys_cb = info->addr[i - nr_frames].phys_cb;
		frame->phys_cr = info->addr[i - nr_frames].phys_cr;
		frame->virt_y = info->addr[i - nr_frames].virt_y;
		frame->virt_cb = info->addr[i - nr_frames].virt_cb;
		frame->virt_cr = info->addr[i - nr_frames].virt_cr;
	}
	

	return 0;

alloc_fail:
	s3c_fimc_free_output_memory(info);
	return ret;
}

#else
void s3c_fimc_free_output_memory(struct s3c_fimc_out_frame *info)
{
	struct s3c_fimc_frame_addr *frame;
	int i;

	for (i = 0; i < info->nr_frames; i++) {
		frame = &info->addr[i];

		if (frame->virt_y)
			kfree(frame->virt_y);

		memset(frame, 0, sizeof(*frame));
	}

	info->buf_size = 0;
}

static int s3c_fimc_alloc_rgb_memory(struct s3c_fimc_out_frame *info)
{
	struct s3c_fimc_frame_addr *frame;
	int i, ret, nr_frames = info->nr_frames;

	for (i = 0; i < nr_frames; i++) {
		frame = &info->addr[i];

		frame->virt_rgb = kmalloc(info->buf_size, GFP_DMA);
		if (frame->virt_rgb == NULL) {
			ret = -ENOMEM;
			goto alloc_fail;
		}
		
		frame->phys_rgb = virt_to_phys(frame->virt_rgb);
	}

	for (i = nr_frames; i < S3C_FIMC_MAX_FRAMES; i++) {
		frame = &info->addr[i];
		frame->virt_rgb = info->addr[i - nr_frames].virt_rgb;
		frame->phys_rgb = info->addr[i - nr_frames].phys_rgb;
	}

	return 0;

alloc_fail:
	s3c_fimc_free_output_memory(info);
	return ret;
}

static int s3c_fimc_alloc_yuv_memory(struct s3c_fimc_out_frame *info)
{
	struct s3c_fimc_frame_addr *frame;
	int i, ret, nr_frames = info->nr_frames;
	u32 size = info->width * info->height, cbcr_size;

	if (info->format == FORMAT_YCBCR420)
		cbcr_size = size / 4;
	else
		cbcr_size = size / 2;

	for (i = 0; i < nr_frames; i++) {
		frame = &info->addr[i];

		frame->virt_y = kmalloc(info->buf_size, GFP_DMA);
		if (frame->virt_y == NULL) {
			ret = -ENOMEM;
			goto alloc_fail;
		}

		frame->virt_cb = frame->virt_y + size;
		frame->virt_cr = frame->virt_cb + cbcr_size;

		frame->phys_y = virt_to_phys(frame->virt_y);
		frame->phys_cb = frame->phys_y + size;
		frame->phys_cr = frame->phys_cb + cbcr_size;
	}

	for (i = nr_frames; i < S3C_FIMC_MAX_FRAMES; i++) {
		frame = &info->addr[i];
		frame->phys_y = info->addr[i - nr_frames].phys_y;
		frame->phys_cb = info->addr[i - nr_frames].phys_cb;
		frame->phys_cr = info->addr[i - nr_frames].phys_cr;
		frame->virt_y = info->addr[i - nr_frames].virt_y;
		frame->virt_cb = info->addr[i - nr_frames].virt_cb;
		frame->virt_cr = info->addr[i - nr_frames].virt_cr;
	}

	return 0;

alloc_fail:
	s3c_fimc_free_output_memory(info);
	return ret;
}
#endif

static u32 s3c_fimc_get_buffer_size(int width, int height, enum s3c_fimc_format_t fmt)
{
	u32 size = width * height;
	u32 cbcr_size = 0, *buf_size = NULL, one_p_size;

	switch (fmt) {
	case FORMAT_RGB565:
		size *= 2;
		buf_size = &size;
		break;

	case FORMAT_RGB666:	/* fall through */
	case FORMAT_RGB888:
		size *= 4;
		buf_size = &size;
		break;

	case FORMAT_YCBCR420:
		cbcr_size = size / 4;
		one_p_size = size + (2 * cbcr_size);
		buf_size = &one_p_size;
		break;

	case FORMAT_YCBCR422:
		cbcr_size = size / 2;
		one_p_size = size + (2 * cbcr_size);
		buf_size = &one_p_size;
		break;
	}

	if (*buf_size % PAGE_SIZE != 0)
		*buf_size = (*buf_size / PAGE_SIZE + 1) * PAGE_SIZE;

	return *buf_size;
}

int s3c_fimc_alloc_output_memory(struct s3c_fimc_out_frame *info)
{
	int ret;

	info->buf_size = s3c_fimc_get_buffer_size(info->width, info->height, \
							info->format);

	printk("DEBUG : %s have buffer size(%x) \n", __FUNCTION__, info->buf_size);

	if (info->format == FORMAT_YCBCR420 || info->format == FORMAT_YCBCR422)
		ret = s3c_fimc_alloc_yuv_memory(info);
	else
		ret = s3c_fimc_alloc_rgb_memory(info);

	return ret;
}

int s3c_fimc_alloc_input_memory(struct s3c_fimc_in_frame *info, dma_addr_t addr)
{
	struct s3c_fimc_frame_addr *frame;
	u32 size = info->width * info->height, cbcr_size;
	
	if (info->format == FORMAT_YCBCR420)
		cbcr_size = size / 4;
	else
		cbcr_size = size / 2;

	info->buf_size = s3c_fimc_get_buffer_size(info->width, info->height, \
							info->format);

	switch (info->format) {
	case FORMAT_RGB565:	/* fall through */
	case FORMAT_RGB666:	/* fall through */
	case FORMAT_RGB888:
		info->addr.phys_rgb = addr;
		break;

	case FORMAT_YCBCR420:	/* fall through */
	case FORMAT_YCBCR422:
		frame = &info->addr;
		frame->phys_y = addr;
		frame->phys_cb = frame->phys_y + size;
		frame->phys_cr = frame->phys_cb + cbcr_size;
		break;
	}

	return 0;
}

int s3c_fimc_alloc_y_memory(struct s3c_fimc_in_frame *info, 
					dma_addr_t addr)
{
	info->addr.phys_y = addr;
	info->buf_size = s3c_fimc_get_buffer_size(info->width, \
					info->height, info->format);

	return 0;
}

int s3c_fimc_alloc_cb_memory(struct s3c_fimc_in_frame *info, 
					dma_addr_t addr)
{
	info->addr.phys_cb = addr;
	info->buf_size = s3c_fimc_get_buffer_size(info->width, \
					info->height, info->format);

	return 0;
}

int s3c_fimc_alloc_cr_memory(struct s3c_fimc_in_frame *info, 
					dma_addr_t addr)
{
	info->addr.phys_cr = addr;
	info->buf_size = s3c_fimc_get_buffer_size(info->width, \
					info->height, info->format);

	return 0;
}

void s3c_fimc_set_nr_frames(struct s3c_fimc_control *ctrl, int nr)
{
	if (nr == 3)
		ctrl->out_frame.nr_frames = 2;
	else
		ctrl->out_frame.nr_frames = nr;
}

static void s3c_fimc_set_input_format(struct s3c_fimc_control *ctrl,
					struct v4l2_pix_format *fmt)
{
	struct s3c_fimc_in_frame *frame = &ctrl->in_frame;

	frame->width = fmt->width;
	frame->height = fmt->height;

	switch (fmt->pixelformat) {
	case V4L2_PIX_FMT_RGB565:
		frame->format = FORMAT_RGB565;
		frame->planes = 1;
		break;

	case V4L2_PIX_FMT_RGB24:
		frame->format = FORMAT_RGB888;
		frame->planes = 1;
		break;

	case V4L2_PIX_FMT_NV12:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 2;
		frame->order_2p = LSB_CBCR;
		break;

	case V4L2_PIX_FMT_NV21:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 2;
		frame->order_2p = LSB_CRCB;
		break;

	case V4L2_PIX_FMT_NV12X:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 2;
		frame->order_2p = MSB_CBCR;
		break;

	case V4L2_PIX_FMT_NV21X:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 2;
		frame->order_2p = MSB_CRCB;
		break;

	case V4L2_PIX_FMT_YUV420:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 3;
		break;

	case V4L2_PIX_FMT_YUYV:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 1;
		frame->order_1p = IN_ORDER422_YCBYCR;
		break;

	case V4L2_PIX_FMT_YVYU:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 1;
		frame->order_1p = IN_ORDER422_YCRYCB;
		break;

	case V4L2_PIX_FMT_UYVY:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 1;
		frame->order_1p = IN_ORDER422_CBYCRY;
		break;

	case V4L2_PIX_FMT_VYUY:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 1;
		frame->order_1p = IN_ORDER422_CRYCBY;
		break;

	case V4L2_PIX_FMT_NV16:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 2;
		frame->order_1p = LSB_CBCR;
		break;

	case V4L2_PIX_FMT_NV61:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 2;
		frame->order_1p = LSB_CRCB;
		break;

	case V4L2_PIX_FMT_NV16X:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 2;
		frame->order_1p = MSB_CBCR;
		break;

	case V4L2_PIX_FMT_NV61X:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 2;
		frame->order_1p = MSB_CRCB;
		break;

	case V4L2_PIX_FMT_YUV422P:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 3;
		break;
	}
}

int s3c_fimc_set_input_frame(struct s3c_fimc_control *ctrl,
				struct v4l2_pix_format *fmt)
{
	s3c_fimc_set_input_format(ctrl, fmt);

	return 0;
}

static int s3c_fimc_set_output_format(struct s3c_fimc_control *ctrl,
					struct v4l2_pix_format *fmt)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	int depth = 0;

	frame->width = fmt->width;
	frame->height = fmt->height;

	switch (fmt->pixelformat) {
	case V4L2_PIX_FMT_RGB565:
		frame->format = FORMAT_RGB565;
		frame->planes = 1;
		depth = 16;
		break;

	case V4L2_PIX_FMT_RGB24:
		frame->format = FORMAT_RGB888;
		frame->planes = 1;
		depth = 24;
		break;

	case V4L2_PIX_FMT_NV12:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 2;
		frame->order_2p = LSB_CBCR;
		depth = 12;
		break;

	case V4L2_PIX_FMT_NV21:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 2;
		frame->order_2p = LSB_CRCB;
		depth = 12;
		break;

	case V4L2_PIX_FMT_NV12X:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 2;
		frame->order_2p = MSB_CBCR;
		depth = 12;
		break;

	case V4L2_PIX_FMT_NV21X:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 2;
		frame->order_2p = MSB_CRCB;
		depth = 12;
		break;

	case V4L2_PIX_FMT_YUV420:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 3;
		depth = 12;
		break;

	case V4L2_PIX_FMT_YUYV:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 1;
		frame->order_1p = OUT_ORDER422_YCBYCR;
		depth = 16;
		break;

	case V4L2_PIX_FMT_YVYU:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 1;
		frame->order_1p = OUT_ORDER422_YCRYCB;
		depth = 16;
		break;

	case V4L2_PIX_FMT_UYVY:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 1;
		frame->order_1p = OUT_ORDER422_CBYCRY;
		depth = 16;
		break;

	case V4L2_PIX_FMT_VYUY:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 1;
		frame->order_1p = OUT_ORDER422_CRYCBY;
		depth = 16;
		break;

	case V4L2_PIX_FMT_NV16:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 2;
		frame->order_1p = LSB_CBCR;
		depth = 16;
		break;

	case V4L2_PIX_FMT_NV61:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 2;
		frame->order_1p = LSB_CRCB;
		depth = 16;
		break;

	case V4L2_PIX_FMT_NV16X:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 2;
		frame->order_1p = MSB_CBCR;
		depth = 16;
		break;

	case V4L2_PIX_FMT_NV61X:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 2;
		frame->order_1p = MSB_CRCB;
		depth = 16;
		break;

	case V4L2_PIX_FMT_YUV422P:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 3;
		depth = 16;
		break;
	}

	switch (fmt->field) {
	case V4L2_FIELD_INTERLACED:
	case V4L2_FIELD_INTERLACED_TB:
	case V4L2_FIELD_INTERLACED_BT:
		frame->scan = SCAN_TYPE_INTERLACE;
		break;

	default:
		frame->scan = SCAN_TYPE_PROGRESSIVE;
		break;
	}

	return depth;
}

int s3c_fimc_set_output_frame(struct s3c_fimc_control *ctrl,
				struct v4l2_pix_format *fmt)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	int depth = 0;

	
	depth = s3c_fimc_set_output_format(ctrl, fmt);

	if (ctrl->out_type == PATH_OUT_DMA && frame->addr[0].virt_y == NULL) {
		if (s3c_fimc_alloc_output_memory(frame))
			err("cannot allocate memory\n");
	}

	return depth;
}

int s3c_fimc_frame_handler(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	int ret;

	frame->skip_frames++;
	dev_dbg(ctrl->dev, "irq is being handled by frame hander\n");

	switch (ctrl->flag & S3C_FIMC_IRQ_MASK) {
	case S3C_FIMC_FLAG_IRQ_NORMAL:
		dev_dbg(ctrl->dev, "irq flag is normal\n");
		FSET_RUNNING(ctrl);
		FSET_IRQ_X(ctrl);
		ret = S3C_FIMC_FRAME_SKIP;
		break;

	case S3C_FIMC_FLAG_IRQ_X:
		dev_dbg(ctrl->dev, "irq flag is x\n");
		s3c_fimc_enable_lastirq(ctrl);
		s3c_fimc_disable_lastirq(ctrl);
		FSET_HANDLE_IRQ(ctrl);
		FSET_IRQ_LAST(ctrl);
		ret = S3C_FIMC_FRAME_SKIP;
		break;

	case S3C_FIMC_FLAG_IRQ_LAST:
		dev_dbg(ctrl->dev, "irq flag is last\n");
		FSET_HANDLE_IRQ(ctrl);
		FSET_IRQ_X(ctrl);
		ret = S3C_FIMC_FRAME_TAKE;
		break;

	default:
		dev_dbg(ctrl->dev, "unknown irq state\n");
		ret = S3C_FIMC_FRAME_SKIP;
		break;
	}

	return ret;
}

u8 *s3c_fimc_get_current_frame(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;

	return frame->addr[frame->cfn].virt_y;
}

static int s3c_fimc_get_scaler_factor(u32 src, u32 tar, u32 *ratio, u32 *shift)
{
	if (src >= tar * 64) {
		err("out of pre-scaler range\n");
		return -EINVAL;
	} else if (src >= tar * 32) {
		*ratio = 32;
		*shift = 5;
	} else if (src >= tar * 16) {
		*ratio = 16;
		*shift = 4;
	} else if (src >= tar * 8) {
		*ratio = 8;
		*shift = 3;
	} else if (src >= tar * 4) {
		*ratio = 4;
		*shift = 2;
	} else if (src >= tar * 2) {
		*ratio = 2;
		*shift = 1;
	} else {
		*ratio = 1;
		*shift = 0;
	}

	return 0;
}

int s3c_fimc_set_scaler_info(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_scaler *sc = &ctrl->scaler;
	struct s3c_fimc_window_offset *w_ofs = &ctrl->in_cam->offset;
	struct s3c_fimc_dma_offset *d_ofs = &ctrl->in_frame.offset;
	int ret, tx, ty, sx, sy;
	int width, height, h_ofs, v_ofs;

	if (ctrl->in_type == PATH_IN_DMA) {
		/* input rotator case */
		if (ctrl->rot90 && ctrl->out_type == PATH_OUT_LCDFIFO) {
			width = ctrl->in_frame.height;
			height = ctrl->in_frame.width;
			h_ofs = d_ofs->y_v * 2;
			v_ofs = d_ofs->y_h * 2;
		} else {
			width = ctrl->in_frame.width;
			height = ctrl->in_frame.height;
			h_ofs = d_ofs->y_h * 2;
			v_ofs = d_ofs->y_v * 2;
		}
	} else {
		width = ctrl->in_cam->width;
		height = ctrl->in_cam->height;
		h_ofs = w_ofs->h1 + w_ofs->h2;
		v_ofs = w_ofs->v1 + w_ofs->v2;
	}

	tx = ctrl->out_frame.width;
	ty = ctrl->out_frame.height;

	if (tx <= 0 || ty <= 0) {
		err("invalid target size\n");
		ret = -EINVAL;
		goto err_size;
	}

	sx = width - h_ofs;
	sy = height - v_ofs;

	sc->real_width = sx;
	sc->real_height = sy;

	if (sx <= 0 || sy <= 0) {
		err("invalid source size\n");
		ret = -EINVAL;
		goto err_size;
	}

	s3c_fimc_get_scaler_factor(sx, tx, &sc->pre_hratio, &sc->hfactor);
	s3c_fimc_get_scaler_factor(sy, ty, &sc->pre_vratio, &sc->vfactor);

	if (IS_PREVIEW(ctrl) && (sx / sc->pre_hratio > sc->line_length))
		info("line buffer size overflow sx(%d) / sc->pre_hratio = %d > sc->line_length=%d \n",sx, sc->pre_hratio , sc->line_length);
	


	sc->pre_dst_width = sx / sc->pre_hratio;
	sc->pre_dst_height = sy / sc->pre_vratio;

	
	sc->main_hratio = (sx << 8) / (tx << sc->hfactor);
	sc->main_vratio = (sy << 8) / (ty << sc->vfactor);

	sc->scaleup_h = (tx >= sx) ? 1 : 0;
	sc->scaleup_v = (ty >= sy) ? 1 : 0;

	s3c_fimc_set_prescaler(ctrl);
	s3c_fimc_set_scaler(ctrl);

	return 0;

err_size:
	return ret;
}

/* CAUTION: many sequence dependencies */
void s3c_fimc_start_dma(struct s3c_fimc_control *ctrl)
{
	s3c_fimc_set_input_path(ctrl);

	if (ctrl->in_type == PATH_IN_DMA) {
		s3c_fimc_set_input_address(ctrl);
		s3c_fimc_set_input_dma(ctrl);
	} else {
		s3c_fimc_set_source_format(ctrl);
		s3c_fimc_set_window_offset(ctrl);
		s3c_fimc_set_polarity(ctrl);
	}

	s3c_fimc_set_scaler_info(ctrl);
	s3c_fimc_set_target_format(ctrl);
	s3c_fimc_set_output_path(ctrl);

	if (ctrl->out_type == PATH_OUT_DMA) {
		s3c_fimc_set_output_address(ctrl);
		s3c_fimc_set_output_dma(ctrl);
	}


	if (!ctrl->scaler.bypass)
		s3c_fimc_start_scaler(ctrl);

	s3c_fimc_enable_capture(ctrl);

	if (ctrl->in_type == PATH_IN_DMA)
		s3c_fimc_start_input_dma(ctrl);
}

void s3c_fimc_stop_dma(struct s3c_fimc_control *ctrl)
{
	if (ctrl->in_type == PATH_IN_DMA)
		s3c_fimc_stop_input_dma(ctrl);

	s3c_fimc_stop_scaler(ctrl);
	s3c_fimc_disable_capture(ctrl);
	s3c_fimc_wait_frame_end(ctrl); // add sangyub.kim
}

void s3c_fimc_restart_dma(struct s3c_fimc_control *ctrl)
{
	s3c_fimc_stop_dma(ctrl);
	s3c_fimc_start_dma(ctrl);
}

void s3c_fimc_change_resolution(struct s3c_fimc_control *ctrl,
					enum s3c_fimc_cam_res_t res)
{
	struct s3c_fimc_camera *cam = ctrl->in_cam;

	s3c_fimc_stop_scaler(ctrl);
	s3c_fimc_i2c_command(ctrl, I2C_CAM_RESOLUTION, res);

	switch (res) {
	case CAM_RES_QSVGA:
		info("resolution changed to QSVGA (400x300) mode\n");
		cam->width = 400;
		cam->height = 300;
		break;

	case CAM_RES_VGA:
		info("resolution changed to VGA (640x480) mode\n");
		cam->width = 640;
		cam->height = 480;
		break;

	case CAM_RES_SVGA:
		info("resolution changed to SVGA (800x600) mode\n");
		cam->width = 800;
		cam->height = 600;
		break;

	case CAM_RES_SXGA:
		info("resolution changed to SXGA (1280x1024) mode\n");
		cam->width = 1280;
		cam->height = 1024;
		break;

	case CAM_RES_UXGA:
		info("resolution changed to UXGA (1600x1200) mode\n");
		cam->width = 1600;
		cam->height = 1200;
		break;

	case CAM_RES_DEFAULT:	/* fall through */
	case CAM_RES_MAX:
		/* nothing to do */
		break;
	}
}

int s3c_fimc_check_zoom(struct s3c_fimc_control *ctrl, int type)
{
	struct s3c_fimc_scaler *sc = &ctrl->scaler;
	struct s3c_fimc_window_offset *offset = &ctrl->in_cam->offset;
	int sx = sc->real_width;
	int zoom_pixels = S3C_FIMC_ZOOM_PIXELS * 2;
	int zoom_size = sx - (offset->h1 + offset->h2 + zoom_pixels);
	
	switch (type) {
	case V4L2_CID_ZOOM_IN:
		if (zoom_size / sc->pre_hratio > sc->line_length) {
			err("already reached to zoom-in boundary\n");
			return -EINVAL;
		}

		sc->zoom_depth++;
		break;

	case V4L2_CID_ZOOM_OUT:
		if (sc->zoom_depth > 0)
			sc->zoom_depth--;
		else {
			err("already reached to zoom-out boundary\n");
			return -EINVAL;
		}

		break;
	}

	return 0;
}
