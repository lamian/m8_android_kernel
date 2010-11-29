/* drivers/media/video/s3c_camif.c
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#include <linux/videodev.h>
#include <linux/semaphore.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/irq.h>

#include <mach/map.h>
#include <mach/hardware.h>
#include <mach/gpio.h>

#include <plat/gpio-cfg.h>
#include <plat/regs-camif.h>
#include <plat/regs-gpio.h>
#include <plat/regs-lcd.h>

#include "s3c_camif.h"

static unsigned int irq_old_priority;
camif_cfg_t s3c_fimc[CAMIF_DEV_NUM];

/*************************************************************************
 * Utility part
 ************************************************************************/
camif_cfg_t *s3c_camif_get_fimc_object(int nr)
{
	camif_cfg_t *ret = NULL;

	switch (nr) {
	case CODEC_MINOR:
		ret = &s3c_fimc[FIMC_CODEC_INDEX];
		break;

	case PREVIEW_MINOR:
		ret = &s3c_fimc[FIMC_PREVIEW_INDEX];
		break;

	default:
		printk(KERN_ERR "Unknown minor number\n");
	}

	return ret;
}

int s3c_camif_get_frame_num(camif_cfg_t *cfg)
{
	unsigned int index = 0;

	if (cfg->dma_type & CAMIF_CODEC)
		index = (readl(cfg->regs + S3C_CICOSTATUS) >> 26) & 0x3;
	else {
		assert(cfg->dma_type & CAMIF_PREVIEW);
		index = (readl(cfg->regs + S3C_CIPRSTATUS) >> 26) & 0x3;
	}

	//cfg->cur_frame_num = (index + 2) % 4;    // When 4 PingPong 
	//cfg->cur_frame_num = (index + 2) & 0x03;    /* When 4 PingPong */
	cfg->cur_frame_num = ((index + (cfg->pp_num - 2)) & (cfg->pp_num - 1));
	// added by sangyub
	//printk("[fimc]%s:: next frame number = %d, current frame number = %d\n", __FUNCTION__,index, cfg->cur_frame_num);
	// pp 4 : 0 -> 2 / 1 -> 3 / 2 -> 4 / 3 -> 1
	// pp 2 : 0 -> 0 / 1 -> 1 / 2 -> 0 / 3 -> 1

	//printk("###### kcoolsw : index              : %d\n", index);
	//printk("###### kcoolsw : cfg->pp_num        : %d\n", cfg->pp_num);
	//printk("###### kcoolsw : cfg->cur_frame_num : %d\n", cfg->cur_frame_num);

	return 0;
}

unsigned char* s3c_camif_get_frame(camif_cfg_t *cfg)
{
	unsigned char *ret = NULL;
	unsigned int cnt = cfg->cur_frame_num;

	if (cfg->dma_type & CAMIF_PREVIEW)
		ret = cfg->img_buf[cnt].virt_rgb;

	if (cfg->dma_type & CAMIF_CODEC)
	{
		if ((cfg->dst_fmt & CAMIF_RGB16) || (cfg->dst_fmt & CAMIF_RGB24))
			ret = cfg->img_buf[cnt].virt_rgb;
		else
			ret = cfg->img_buf[cnt].virt_y;
	}

	return ret;
}

int s3c_camif_get_fifo_status(camif_cfg_t *cfg)
{
	unsigned int reg, val, flag;

	if (cfg->dma_type & CAMIF_CODEC) {
		flag = S3C_CICOSTATUS_OVFIY_CO | S3C_CICOSTATUS_OVFICB_CO | S3C_CICOSTATUS_OVFICR_CO;
		reg = readl(cfg->regs + S3C_CICOSTATUS);

		if (reg & flag) {
			/* FIFO Error Count ++  */
			val = readl(cfg->regs + S3C_CIWDOFST);
			val |= (S3C_CIWDOFST_CLROVCOFIY | S3C_CIWDOFST_CLROVCOFICB | S3C_CIWDOFST_CLROVCOFICR);
			writel(val, cfg->regs + S3C_CIWDOFST);

			val = readl(cfg->regs + S3C_CIWDOFST);
			val &= ~(S3C_CIWDOFST_CLROVCOFIY | S3C_CIWDOFST_CLROVCOFICB | S3C_CIWDOFST_CLROVCOFICR);
			writel(val, cfg->regs + S3C_CIWDOFST);

			return 1; /* Error */
		}
	} else if (cfg->dma_type & CAMIF_PREVIEW) {
		flag = S3C_CIPRSTATUS_OVFICB_PR | S3C_CIPRSTATUS_OVFICR_PR;
		reg = readl(cfg->regs + S3C_CIPRSTATUS);

		if (reg & flag) {
			/* FIFO Error Count ++  */
			val = readl(cfg->regs + S3C_CIWDOFST);
			val |= (S3C_CIWDOFST_CLROVPRFICB | S3C_CIWDOFST_CLROVPRFICR);
			writel(val, cfg->regs + S3C_CIWDOFST);

			val = readl(cfg->regs + S3C_CIWDOFST);
			val &= ~(S3C_CIWDOFST_CLROVPRFIY | S3C_CIWDOFST_CLROVPRFICB | S3C_CIWDOFST_CLROVPRFICR);
			writel(val, cfg->regs + S3C_CIWDOFST);

			return 1; /* Error */
		}
	}

	return 0;
}

void s3c_camif_set_polarity(camif_cfg_t *cfg)
{
	camif_cis_t *cis = cfg->cis;
	unsigned int val;
	unsigned int cmd;

	cmd = readl(cfg->regs + S3C_CIGCTRL);
	cmd &= ~(0x7 << 24);

	if (cis->polarity_pclk)
		cmd |= S3C_CIGCTRL_INVPOLPCLK;

	if (cis->polarity_vsync)
		cmd |= S3C_CIGCTRL_INVPOLVSYNC;

	if (cis->polarity_href)
		cmd |= S3C_CIGCTRL_INVPOLHREF;

	val = readl(cfg->regs + S3C_CIGCTRL);
	val |= cmd;
	writel(val, cfg->regs + S3C_CIGCTRL);
}

/*************************************************************************
 * Memory part
 ************************************************************************/
static int s3c_camif_request_memory(camif_cfg_t *cfg)
{
	unsigned int t_size = 0, i = 0;
	unsigned int area = 0;

	area = cfg->target_x * cfg->target_y;

	if (cfg->dma_type & CAMIF_CODEC)
	{
		if      (cfg->dst_fmt & CAMIF_YCBCR420)
			t_size = (area * 3) >> 1;	/* CAMIF_YCBCR420 */
		else if (cfg->dst_fmt & CAMIF_YCBCR422 || cfg->dst_fmt & CAMIF_YCBCR422I)
			t_size = (area << 1);		/* CAMIF_YCBCR422 */
		else if (cfg->dst_fmt & CAMIF_RGB16)
			t_size = (area << 1);		/* 2 bytes per one pixel */
		else if (cfg->dst_fmt & CAMIF_RGB24)
			t_size = (area << 2);		/* 4 bytes per one pixel */
		else
			printk(KERN_INFO "Invalid target format\n");

		if ((t_size % PAGE_SIZE) != 0)
		{
			i = t_size / PAGE_SIZE;
			t_size = (i + 1) * PAGE_SIZE;
		}

		t_size = t_size * cfg->pp_num;
		cfg->pp_totalsize = t_size;
		// added by sangyub
		//printk("[fimc]%s:: frame buffer size = %d, ping pong num = %d\n", __FUNCTION__, t_size , cfg->pp_num);

		// printk(KERN_INFO "Codec memory required: 0x%08X bytes\n", t_size);
	}
	else if (cfg->dma_type & CAMIF_PREVIEW)
	{
		if (cfg->dst_fmt & CAMIF_RGB16)
			t_size = (area << 1);		/*  2 bytes per two pixel*/
		else if (cfg->dst_fmt & CAMIF_RGB24)
			t_size = (area << 2);		/* 4 bytes per one pixel */
		else
			printk(KERN_ERR "Invalid target format\n");

		if ((t_size % PAGE_SIZE) != 0)
		{
			i = t_size / PAGE_SIZE;
			t_size = (i + 1) * PAGE_SIZE;
		}

		t_size = t_size * cfg->pp_num;
		cfg->pp_totalsize = t_size;

		// printk(KERN_INFO "Preview memory required: 0x%08X bytes\n", t_size);
	}

	return 0;
}

static void s3c_camif_calc_burst_length_yuv422i(unsigned int hsize, unsigned int *mburst, unsigned int *rburst)
{
	unsigned int tmp, wanted;

	tmp = (hsize >> 1) & 0xf;

	switch (tmp) {
	case 0:
		wanted = 16;
		break;

	case 4:
		wanted = 4;
		break;

	case 8:
		wanted = 8;
		break;

	default:
		wanted = 4;
		break;
	}

	*mburst = (wanted >> 1);
	*rburst = (wanted >> 1);
}

static void s3c_camif_calc_burst_length(unsigned int hsize, unsigned int *mburst, unsigned int *rburst)
{
	unsigned int tmp;

	tmp = (hsize >> 2) & 0xf;

	switch (tmp) 
	{
		case 0:
			*mburst = 16;
			*rburst = 16;
			break;

		case 4:
			*mburst = 16;
			*rburst = 4;
			break;

		case 8:
			*mburst = 16;
			*rburst = 8;
			break;

		default:
			tmp = (hsize >> 2) % 8;

			if (tmp == 0)
			{
				*mburst = 8;
				*rburst = 8;
			}
			else if (tmp == 4)
			{
				*mburst = 8;
				*rburst = 4;
			}
			else 
			{
				tmp = (hsize >> 2) % 4;
				*mburst = 4;
				*rburst = (tmp) ? tmp : 4;
			}

			break;
	}
}

int s3c_camif_setup_dma(camif_cfg_t *cfg)
{
	unsigned int width = cfg->target_x;
	unsigned int val, yburst_m, yburst_r, cburst_m, cburst_r;

	if (cfg->dma_type & CAMIF_CODEC)
	{
		if ((cfg->dst_fmt == CAMIF_RGB16) || (cfg->dst_fmt == CAMIF_RGB24))
		{
			if (cfg->dst_fmt == CAMIF_RGB24)
			{
				if(width & 0x01)
					return BURST_ERR;

				s3c_camif_calc_burst_length((width << 2), &yburst_m, &yburst_r);
			}
			else
			{
				if ((width >> 1) & 0x01)
					return BURST_ERR;

				s3c_camif_calc_burst_length((width << 1), &yburst_m, &yburst_r);
			}

			val = readl(cfg->regs + S3C_CICOCTRL);
			val &= ~(0xfffff << 4);

			if (cfg->dst_fmt == CAMIF_RGB24)
			{
				val = S3C_CICOCTRL_YBURST1_CO(yburst_m >> 1) | \
				S3C_CICOCTRL_YBURST2_CO(yburst_r >> 2) | (4 << 9) | (2 << 4);
			}
			else
			{
				val = S3C_CICOCTRL_YBURST1_CO(yburst_m >> 1) | \
					S3C_CICOCTRL_YBURST2_CO(yburst_r >> 1) | (4 << 9) | (2 << 4);
			}

			writel(val, cfg->regs + S3C_CICOCTRL);
		} else {
			// CODEC DMA WIDHT is multiple of 16 
			////if (width & 0x15)
			if(width % 16 != 0)
				return BURST_ERR;

			if (cfg->dst_fmt == CAMIF_YCBCR422I)
			{
				s3c_camif_calc_burst_length_yuv422i(width, &yburst_m, &yburst_r);
				cburst_m = (yburst_m >> 1);
				cburst_r = (yburst_r >> 1);
			} else {
				s3c_camif_calc_burst_length(width, &yburst_m, &yburst_r);
				s3c_camif_calc_burst_length(width >> 1, &cburst_m, &cburst_r);
			}

			val = readl(cfg->regs + S3C_CICOCTRL);
			val &= ~(0xfffff << 4);
			val |= (S3C_CICOCTRL_YBURST1_CO(yburst_m) | S3C_CICOCTRL_CBURST1_CO(cburst_m) | \
			       S3C_CICOCTRL_YBURST2_CO(yburst_r) | S3C_CICOCTRL_CBURST2_CO(cburst_r));
			writel(val, cfg->regs + S3C_CICOCTRL);
		}
	}
	else if (cfg->dma_type & CAMIF_PREVIEW)
	{
		if (cfg->dst_fmt == CAMIF_RGB24)
		{
			if (width & 0x01)
				return BURST_ERR;

			s3c_camif_calc_burst_length(width << 2, &yburst_m, &yburst_r);
		} else {
			if ((width >> 1) & 0x01)
				return BURST_ERR;

			s3c_camif_calc_burst_length(width << 1, &yburst_m, &yburst_r);
		}

		val = readl(cfg->regs + S3C_CIPRCTRL);
		val &= ~(0x3ff << 14);
		val |= (S3C_CICOCTRL_YBURST1_CO(yburst_m) | S3C_CICOCTRL_YBURST2_CO(yburst_r));
		writel(val, cfg->regs + S3C_CIPRCTRL);
	}

	return 0;
}

/*************************************************************************
 * Input path part
 ************************************************************************/
/*
 * 2443 MSDMA (Preview Only)
 */
#if defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
int s3c_camif_input_msdma_preview(camif_cfg_t * cfg)
{
	int ret = 0;
	unsigned int addr_start_Y = 0, addr_start_CB = 0, addr_start_CR = 0;
	unsigned int addr_end_Y = 0, addr_end_CB = 0, addr_end_CR = 0;
	unsigned int val, val_width;

	val = readl(cfg->regs + S3C_CIMSCTRL);
	val &= ~(1 << 2);
	writel(val, cfg->regs + S3C_CIMSCTRL);

	val = readl(cfg->regs + S3C_CIMSCTRL);
	val |= (1 << 2);
	writel(val, cfg->regs + S3C_CIMSCTRL);

	if (cfg->src_fmt != CAMIF_YCBCR420 && cfg->src_fmt != CAMIF_YCBCR422 && cfg->src_fmt != CAMIF_YCBCR422I)
		cfg->src_fmt = CAMIF_YCBCR420;

	switch(cfg->src_fmt) {
	case CAMIF_YCBCR420:
		val = readl(cfg->regs + S3C_CIMSCTRL);
		val = (val & ~(0x1 << 1)) | (0x1 << 1);
		writel(val, cfg->regs + S3C_CIMSCTRL);

		addr_start_Y = readl(cfg->regs + S3C_CIMSYSA);
		addr_start_CB = addr_start_Y + (cfg->cis->source_x * cfg->cis->source_y);
		addr_start_CR = addr_start_CB + (cfg->cis->source_x * cfg->cis->source_y / 4);

		addr_end_Y = addr_start_Y + (cfg->cis->source_x * cfg->cis->source_y);
		addr_end_CB = addr_start_CB + (cfg->cis->source_x * cfg->cis->source_y / 4);
		addr_end_CR = addr_start_CR + (cfg->cis->source_x * cfg->cis->source_y / 4);
		break;

	case CAMIF_YCBCR422:
	case CAMIF_YCBCR422I:
		val = readl(cfg->regs + S3C_CIMSCTRL);
		val = (val & ~(0x1 << 5)) | (0x1 << 5);	/* Interleave_MS */
		val &= ~(0x1 << 1);
		val &= ~(0x3 << 3);			/* YCbYCr */
		writel(val, cfg->regs + S3C_CIMSCTRL);

		addr_start_Y = readl(cfg->regs + S3C_CIMSYSA);
		addr_start_CB = addr_start_Y + (cfg->cis->source_x * cfg->cis->source_y);
		addr_start_CR = addr_start_CB + (cfg->cis->source_x * cfg->cis->source_y / 2);

		addr_end_Y = addr_start_Y + (cfg->cis->source_x * cfg->cis->source_y);
		addr_end_CB = addr_start_CB + (cfg->cis->source_x * cfg->cis->source_y / 2);
		addr_end_CR = addr_start_CR + (cfg->cis->source_x * cfg->cis->source_y / 2);
		break;

	default:
		break;
	}

	/* MSDMA memory */
	writel(addr_start_Y, cfg->regs + S3C_CIMSYSA);
	writel(addr_start_CB, cfg->regs + S3C_CIMSCBSA);
	writel(addr_start_CR, cfg->regs + S3C_CIMSCRSA);

	writel(addr_end_Y, cfg->regs + S3C_CIMSYEND);
	writel(addr_end_CB, cfg->regs + S3C_CIMSCBEND);
	writel(addr_end_CR, cfg->regs + S3C_CIMSCREND);

	/* MSDMA memory offset - default : 0 */
	writel(0, cfg->regs + S3C_CIMSYOFF);
	writel(0, cfg->regs + S3C_CIMSCBOFF);
	writel(0, cfg->regs + S3C_CIMSCROFF);

	/* MSDMA for codec source image width */
	val_width = readl(cfg->regs + S3C_CIMSWIDTH);
	val_width = (val_width & ~(0x1 << 31));		/* AutoLoadDisable */
	val_width |= (cfg->cis->source_y << 16);	/* MSCOHEIGHT */
	val_width |= cfg->cis->source_x;		/* MSCOWIDTH */
	val_width = cfg->cis->source_x;
	writel(val_width, cfg->regs + S3C_CIMSWIDTH);

	return ret;
}

static int s3c_camif_input_msdma(camif_cfg_t *cfg)
{
	if (cfg->input_channel == MSDMA_FROM_PREVIEW)
		s3c_camif_input_msdma_preview(cfg);

	return 0;
}

/*
 * 6400 MSDMA (Preview & Codec)
 */
#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
int s3c_camif_input_msdma_codec(camif_cfg_t * cfg)
{
	int ret = 0;
	u32 addr_start_Y = 0, addr_start_CB = 0, addr_start_CR = 0;
	u32 addr_end_Y = 0, addr_end_CB = 0, addr_end_CR = 0;
	u32 val, val_width;

	/* Codec path input data selection */
	val = readl(cfg->regs + S3C_MSCOCTRL);
	val &= ~(1 << 3);
	writel(val, cfg->regs + S3C_MSCOCTRL);

	val = readl(cfg->regs + S3C_MSCOCTRL);
	val |= (1 << 3);
	writel(val, cfg->regs + S3C_MSCOCTRL);

	if (cfg->src_fmt != CAMIF_YCBCR420 && cfg->src_fmt != CAMIF_YCBCR422 && cfg->src_fmt != CAMIF_YCBCR422I)
		cfg->src_fmt = CAMIF_YCBCR420;

	switch(cfg->src_fmt) {
	case CAMIF_YCBCR420:
		val = readl(cfg->regs + S3C_MSCOCTRL);
		val &= ~(0x3 << 1);
		writel(val, cfg->regs + S3C_MSCOCTRL);

		addr_start_Y = cfg->pp_phys_buf;
		addr_start_CB = addr_start_Y + (cfg->cis->source_x * cfg->cis->source_y);
		addr_start_CR = addr_start_CB + (cfg->cis->source_x * cfg->cis->source_y / 4);

		addr_end_Y = addr_start_Y + (cfg->cis->source_x * cfg->cis->source_y);
		addr_end_CB = addr_start_CB + (cfg->cis->source_x * cfg->cis->source_y / 4);
		addr_end_CR = addr_start_CR + (cfg->cis->source_x * cfg->cis->source_y / 4);
		break;

	case CAMIF_YCBCR422:
	case CAMIF_YCBCR422I:
		val = readl(cfg->regs + S3C_MSCOCTRL);
		val = (val & ~(0x3 << 1)) |(0x2 << 1);
		writel(val, cfg->regs + S3C_MSCOCTRL);

		addr_start_Y = cfg->pp_phys_buf;
		addr_start_CB = addr_start_Y + (cfg->cis->source_x * cfg->cis->source_y);
		addr_start_CR = addr_start_CB + (cfg->cis->source_x * cfg->cis->source_y / 2);

		addr_end_Y = addr_start_Y + (cfg->cis->source_x * cfg->cis->source_y);
		addr_end_CB = addr_start_CB + (cfg->cis->source_x * cfg->cis->source_y / 2);
		addr_end_CR = addr_start_CR + (cfg->cis->source_x * cfg->cis->source_y / 2);
		break;

	default:
		break;
	}

	/* MSDMA memory */
	writel(addr_start_Y, cfg->regs + S3C_MSCOY0SA);
	writel(addr_start_CB, cfg->regs + S3C_MSCOCB0SA);
	writel(addr_start_CR, cfg->regs + S3C_MSCOCR0SA);

	writel(addr_end_Y, cfg->regs + S3C_MSCOY0END);
	writel(addr_end_CB, cfg->regs + S3C_MSCOCB0END);
	writel(addr_end_CR, cfg->regs + S3C_MSCOCR0END);

	/* MSDMA memory offset */
	writel(0, cfg->regs + S3C_MSCOYOFF);
	writel(0, cfg->regs + S3C_MSCOCBOFF);
	writel(0, cfg->regs + S3C_MSCOCROFF);

	/* MSDMA for codec source image width */
	val_width = readl(cfg->regs + S3C_MSCOWIDTH);
	val_width = (val_width & ~(0x1 << 31))|(0x1 << 31);	/* AutoLoadEnable */
	val_width |= (cfg->cis->source_y << 16);		/* MSCOHEIGHT */
	val_width |= cfg->cis->source_x;			/* MSCOWIDTH */
	writel(val_width, cfg->regs + S3C_MSCOWIDTH);

	return ret;
}

int s3c_camif_input_msdma_preview(camif_cfg_t * cfg)
{
	int ret = 0;
	unsigned int addr_start_Y = 0, addr_start_CB = 0, addr_start_CR = 0;
	unsigned int addr_end_Y = 0, addr_end_CB = 0, addr_end_CR = 0;
	unsigned int val, val_width;

	val = readl(cfg->regs + S3C_CIMSCTRL);
	val &= ~(0x1 << 3);
	writel(val, cfg->regs + S3C_CIMSCTRL);

	val = readl(cfg->regs + S3C_CIMSCTRL);
	val |= (0x1 << 3);
	writel(val, cfg->regs + S3C_CIMSCTRL);

	if (cfg->src_fmt != CAMIF_YCBCR420 && cfg->src_fmt != CAMIF_YCBCR422 && cfg->src_fmt != CAMIF_YCBCR422I)
		cfg->src_fmt = CAMIF_YCBCR420;

	switch(cfg->src_fmt) {
	case CAMIF_YCBCR420:
		val = readl(cfg->regs + S3C_CIMSCTRL);
		val &= ~(0x3 << 1);
		writel(val, cfg->regs + S3C_CIMSCTRL);

		addr_start_Y = readl(cfg->regs + S3C_MSPRY0SA);
		addr_start_CB = addr_start_Y + (cfg->cis->source_x * cfg->cis->source_y);
		addr_start_CR = addr_start_CB + (cfg->cis->source_x * cfg->cis->source_y / 4);

		addr_end_Y = addr_start_Y + (cfg->cis->source_x * cfg->cis->source_y);
		addr_end_CB = addr_start_CB + (cfg->cis->source_x * cfg->cis->source_y / 4);
		addr_end_CR = addr_start_CR + (cfg->cis->source_x * cfg->cis->source_y / 4);
		break;

	case CAMIF_YCBCR422:
	case CAMIF_YCBCR422I:
		val = readl(cfg->regs + S3C_CIMSCTRL);
		val = (val & ~(0x3 << 1)) | (0x2 << 1);	/* YCbCr 422 Interleave */
		val = (val & ~(0x3 << 4)) | (0x3 << 4);	/* YCbYCr */
		writel(val, cfg->regs + S3C_CIMSCTRL);

		addr_start_Y = readl(cfg->regs + S3C_MSPRY0SA);
		addr_start_CB = addr_start_Y + (cfg->cis->source_x * cfg->cis->source_y);
		addr_start_CR = addr_start_CB + (cfg->cis->source_x * cfg->cis->source_y / 2);

		addr_end_Y = addr_start_Y + (cfg->cis->source_x * cfg->cis->source_y);
		addr_end_CB = addr_start_CB + (cfg->cis->source_x * cfg->cis->source_y / 2);
		addr_end_CR = addr_start_CR + (cfg->cis->source_x * cfg->cis->source_y / 2);
		break;

	default:
		break;
	}

	/* MSDMA memory */
	writel(addr_start_Y, cfg->regs + S3C_MSPRY0SA);
	writel(addr_start_CB, cfg->regs + S3C_MSPRCB0SA);
	writel(addr_start_CR, cfg->regs + S3C_MSPRCR0SA);

	writel(addr_end_Y, cfg->regs + S3C_MSPRY0END);
	writel(addr_end_CB, cfg->regs + S3C_MSPRCB0END);
	writel(addr_end_CR, cfg->regs + S3C_MSPRCR0END);

	/* MSDMA memory offset */
	writel(0, cfg->regs + S3C_MSPRYOFF);
	writel(0, cfg->regs + S3C_MSPRCBOFF);
	writel(0, cfg->regs + S3C_MSPRCROFF);

	/* MSDMA for codec source image width */
	val_width = readl(cfg->regs + S3C_MSPRWIDTH);
	val_width = (val_width & ~(0x1 << 31));		/* AutoLoadEnable */
	val_width |= (cfg->cis->source_y << 16);	/* MSCOHEIGHT */
	val_width |= cfg->cis->source_x;		/* MSCOWIDTH */
	writel(val_width, cfg->regs + S3C_MSPRWIDTH);

	return ret;
}

static int s3c_camif_input_msdma(camif_cfg_t *cfg)
{
	if (cfg->input_channel == MSDMA_FROM_PREVIEW)
		s3c_camif_input_msdma_preview(cfg);
	else if (cfg->input_channel == MSDMA_FROM_CODEC)
		s3c_camif_input_msdma_codec(cfg);

	return 0;
}
#endif

static int s3c_camif_input_camera(camif_cfg_t *cfg)
{
	unsigned int val;

	s3c_camif_set_offset(cfg->cis);

	if (cfg->dma_type & CAMIF_CODEC)
	{
		#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
			val = readl(cfg->regs + S3C_MSCOCTRL);
			val &= ~(1 << 3);
			writel(val, cfg->regs + S3C_MSCOCTRL);
		#endif
	}
	else if (cfg->dma_type & CAMIF_PREVIEW)
	{
		#if defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450)
			val = readl(cfg->regs + S3C_CIMSCTRL);
			val &= ~(1 << 2);
			writel(val, cfg->regs + S3C_CIMSCTRL);
		#elif defined(CONFIG_CPU_S3C6400)
			val = readl(cfg->regs + S3C_CIMSCTRL);	
			val &= ~(1 << 3);
			writel(val, cfg->regs + S3C_CIMSCTRL);
		#endif
	}
	else
		printk(KERN_ERR "Invalid DMA type\n");

	return 0;
}

static int s3c_camif_setup_input_path(camif_cfg_t *cfg)
{
	if (cfg->input_channel == CAMERA_INPUT)
		s3c_camif_input_camera(cfg);
	else
		s3c_camif_input_msdma(cfg);

	return 0;
}

/*************************************************************************
 * Output path part
 ************************************************************************/
static int s3c_camif_output_pp_codec_rgb(camif_cfg_t *cfg)
{
	int i;
	unsigned int val;
	unsigned int area = cfg->target_x * cfg->target_y;
	
	if (cfg->dst_fmt & CAMIF_RGB24)
		area = (area << 2);
	else
	{
		assert (cfg->dst_fmt & CAMIF_RGB16);
		area = (area << 1);
	}

	if ((area % PAGE_SIZE) != 0)
	{
		i = area / PAGE_SIZE;
		area = (i + 1) * PAGE_SIZE;
	}

	cfg->buffer_size = area;
	// added by sangyub
	//printk("[fimc]%s::cfg->buffer_size = %d \n", __FUNCTION__,area);

	if (cfg->input_channel == MSDMA_FROM_CODEC)
	{
		val = readl(S3C_VIDW00ADD0B0);

		for (i = 0; i < 4; i++)
			writel(val, cfg->regs + S3C_CICOYSA(i));
	}
	else
	{
		for (i = 0; i < MAX_PPNUM; i++)
		{
			if(cfg->flag_img_buf_user_set == 1)
			{
				// nop
			}
			else
			{
				cfg->img_buf[i].virt_rgb = cfg->pp_virt_buf;
				cfg->img_buf[i].phys_rgb = cfg->pp_phys_buf;
			}
//			printk("[fimc]%s::cfg->img_buf[%d].phys_rgb = 0x%x \n", __FUNCTION__,i, cfg->img_buf[i].phys_rgb);
			writel(cfg->img_buf[i].phys_rgb, cfg->regs + S3C_CICOYSA(i));
		}
	}

	return 0;
}

static int s3c_camif_output_pp_codec(camif_cfg_t *cfg)
{
	unsigned int i, cbcr_size = 0;
	unsigned int area = cfg->target_x * cfg->target_y;
	unsigned int one_p_size;
	unsigned int real_one_p_size = 0;
	
	if (cfg->dst_fmt & CAMIF_YCBCR420)
		cbcr_size = (area >> 2);
	else if (cfg->dst_fmt & CAMIF_YCBCR422 || cfg->dst_fmt & CAMIF_YCBCR422I)
		cbcr_size = (area >> 1);
	else if ((cfg->dst_fmt & CAMIF_RGB16) || (cfg->dst_fmt & CAMIF_RGB24))
	{
		s3c_camif_output_pp_codec_rgb(cfg);
		return 0;
	}
	else
		printk(KERN_ERR "Invalid target format %d\n", cfg->dst_fmt);

	one_p_size = area + (cbcr_size << 1);

	if ((one_p_size % PAGE_SIZE) != 0)
	{
		i = one_p_size / PAGE_SIZE;
		one_p_size = (i + 1) * PAGE_SIZE;

		// printk("######### changed one_p_size : %d\n", one_p_size);
	}

	cfg->buffer_size = one_p_size;
	
	if(cfg->pp_num == 0 || MAX_PPNUM < cfg->pp_num)
		printk(KERN_ERR "Invalid pingpong number %d\n", cfg->pp_num);
	
	for (i = 0; i < MAX_PPNUM; i++)
	{
		if(cfg->flag_img_buf_user_set == 1)
		{			
			if (i >= cfg->pp_num)
			{
				cfg->img_buf[i].virt_y = 0;
				cfg->img_buf[i].phys_y = 0;
				cfg->img_buf[i].virt_cb = 0;
				cfg->img_buf[i].phys_cb = 0;
				cfg->img_buf[i].virt_cr = 0;
				cfg->img_buf[i].phys_cr = 0;
			}
			else
			{
				if (cfg->dst_fmt & CAMIF_YCBCR422I)
				{
					cfg->img_buf[i].virt_cb = 0;
					cfg->img_buf[i].phys_cb = 0;
					cfg->img_buf[i].virt_cr = 0;
					cfg->img_buf[i].phys_cr = 0;
				}
				else if (cfg->dst_fmt & CAMIF_YCBCR420 || cfg->dst_fmt & CAMIF_YCBCR422)
				{
					cfg->img_buf[i].virt_cb = cfg->img_buf[i].virt_y + area;
					cfg->img_buf[i].phys_cb = cfg->img_buf[i].phys_y + area;
					cfg->img_buf[i].virt_cr = cfg->img_buf[i].virt_y + area + cbcr_size;
					cfg->img_buf[i].phys_cr = cfg->img_buf[i].phys_y + area + cbcr_size;
				}
			}
		}
		else
		{
			real_one_p_size = one_p_size * (i % cfg->pp_num);

			cfg->img_buf[i].virt_y  = cfg->pp_virt_buf                    + real_one_p_size;
			cfg->img_buf[i].phys_y  = cfg->pp_phys_buf                    + real_one_p_size;
			cfg->img_buf[i].virt_cb = cfg->pp_virt_buf + area             + real_one_p_size;
			cfg->img_buf[i].phys_cb = cfg->pp_phys_buf + area             + real_one_p_size;
			cfg->img_buf[i].virt_cr = cfg->pp_virt_buf + area + cbcr_size + real_one_p_size;
			cfg->img_buf[i].phys_cr = cfg->pp_phys_buf + area + cbcr_size + real_one_p_size;
		}
		
		writel(cfg->img_buf[i].phys_y,  cfg->regs + S3C_CICOYSA(i));
		writel(cfg->img_buf[i].phys_cb, cfg->regs + S3C_CICOCBSA(i));
		writel(cfg->img_buf[i].phys_cr, cfg->regs + S3C_CICOCRSA(i));
//		printk("#### user set : cfg->img_buf[%d].phys_y : 0x%x,", i, cfg->img_buf[i].phys_y);
//		printk("phys_cb : 0x%x,", cfg->img_buf[i].phys_cb);
//		printk("phys_cr : 0x%x\n", cfg->img_buf[i].phys_cr);
	}

	return 0;
}

#if defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
static int s3c_camif_io_duplex_preview(camif_cfg_t *cfg)
{
	unsigned int cbcr_size = 0;
	unsigned int area = cfg->cis->source_x * cfg->cis->source_y;
	unsigned int val;
	int i;

	val = readl(S3C_VIDW01ADD0);

	if (!((cfg->dst_fmt & CAMIF_RGB16) || (cfg->dst_fmt & CAMIF_RGB24)))
		printk(KERN_ERR "Invalid target format\n");

	for (i = 0; i < 4; i++)
		writel(val, cfg->regs + S3C_CIPRYSA(i));

	if(cfg->flag_img_buf_user_set == 1)
	{
		// set by user	
	}
	else
	{
		cfg->img_buf[0].virt_y = cfg->pp_virt_buf;
		cfg->img_buf[0].phys_y = cfg->pp_phys_buf;
	}

	if (cfg->src_fmt & CAMIF_YCBCR420)
	{
		cbcr_size = (area >> 2);

		if(cfg->flag_img_buf_user_set == 1)
		{
			cfg->img_buf[0].virt_cb = cfg->img_buf[0].virt_y + area;
			cfg->img_buf[0].phys_cb = cfg->img_buf[0].phys_y + area;
			cfg->img_buf[0].virt_cr = cfg->img_buf[0].virt_y + area + cbcr_size;
			cfg->img_buf[0].phys_cr = cfg->img_buf[0].phys_y + area + cbcr_size;
		}
		else
		{
			cfg->img_buf[0].virt_cb = cfg->pp_virt_buf + area;
			cfg->img_buf[0].phys_cb = cfg->pp_phys_buf + area;
			cfg->img_buf[0].virt_cr = cfg->pp_virt_buf + area + cbcr_size;
			cfg->img_buf[0].phys_cr = cfg->pp_phys_buf + area + cbcr_size;
		}
		
	}
	else if (cfg->src_fmt & CAMIF_YCBCR422 || cfg->dst_fmt & CAMIF_YCBCR422I)
	{
		area = (area << 1);
		cfg->img_buf[0].virt_cb = 0;
		cfg->img_buf[0].phys_cb = 0;
		cfg->img_buf[0].virt_cr = 0;
		cfg->img_buf[0].phys_cr = 0;
	}

	writel(cfg->img_buf[0].phys_y, cfg->regs + S3C_CIMSYSA);
	writel(cfg->img_buf[0].phys_y + area, cfg->regs + S3C_CIMSYEND);

	writel(cfg->img_buf[0].phys_cb, cfg->regs + S3C_CIMSCBSA);
	writel(cfg->img_buf[0].phys_cb + cbcr_size, cfg->regs + S3C_CIMSCBEND);

	writel(cfg->img_buf[0].phys_cr, cfg->regs + S3C_CIMSCRSA);
	writel(cfg->img_buf[0].phys_cr + cbcr_size, cfg->regs + S3C_CIMSCREND);

	writel(cfg->cis->source_x, cfg->regs + S3C_CIMSWIDTH);

	return 0;
}
#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
static int s3c_camif_io_duplex_preview(camif_cfg_t *cfg)
{
	unsigned int cbcr_size = 0;
	unsigned int area = cfg->cis->source_x * cfg->cis->source_y;
	unsigned int val;
	int i;

	val = readl(S3C_VIDW01ADD0B0);

	if (!((cfg->dst_fmt & CAMIF_RGB16) || (cfg->dst_fmt & CAMIF_RGB24)))
		printk(KERN_ERR "Invalid target format\n");

	for (i = 0; i < 4; i++)
		writel(val, cfg->regs + S3C_CIPRYSA(i));


	if(cfg->flag_img_buf_user_set == 1)
	{
	}
	else
	{
		cfg->img_buf[0].virt_y = cfg->pp_virt_buf;
		cfg->img_buf[0].phys_y = cfg->pp_phys_buf;
	}

	if (cfg->src_fmt & CAMIF_YCBCR420)
	{
		cbcr_size = (area >> 2);

		if(cfg->flag_img_buf_user_set == 1)
		{
			cfg->img_buf[0].virt_cb = cfg->img_buf[0].virt_y + area;
			cfg->img_buf[0].phys_cb = cfg->img_buf[0].phys_y + area;
			cfg->img_buf[0].virt_cr = cfg->img_buf[0].virt_y + area + cbcr_size;
			cfg->img_buf[0].phys_cr = cfg->img_buf[0].phys_y + area + cbcr_size;
		}
		else
		{
			cfg->img_buf[0].virt_cb = cfg->pp_virt_buf + area;
			cfg->img_buf[0].phys_cb = cfg->pp_phys_buf + area;
			cfg->img_buf[0].virt_cr = cfg->pp_virt_buf + area + cbcr_size;
			cfg->img_buf[0].phys_cr = cfg->pp_phys_buf + area + cbcr_size;
		}
		
	}
	else if (cfg->src_fmt & CAMIF_YCBCR422 || cfg->dst_fmt & CAMIF_YCBCR422I)
	{
		area = (area << 1);
		cfg->img_buf[0].virt_cb = 0;
		cfg->img_buf[0].phys_cb = 0;
		cfg->img_buf[0].virt_cr = 0;
		cfg->img_buf[0].phys_cr = 0;
	}



	writel(cfg->img_buf[0].phys_y, cfg->regs + S3C_MSPRY0SA);
	writel(cfg->img_buf[0].phys_y + area, cfg->regs + S3C_MSPRY0END);

	writel(cfg->img_buf[0].phys_cb, cfg->regs + S3C_MSPRCB0SA);
	writel(cfg->img_buf[0].phys_cb + cbcr_size, cfg->regs + S3C_MSPRCB0END);

	writel(cfg->img_buf[0].phys_cr, cfg->regs + S3C_MSPRCR0SA);
	writel(cfg->img_buf[0].phys_cr + cbcr_size, cfg->regs + S3C_MSPRCR0END);

	val = readl(cfg->regs + S3C_MSCOWIDTH);
	val = (val & ~(0x1 << 31)) | (0x1 << 31);
	val |= (cfg->cis->source_y << 16);
	val |= cfg->cis->source_x;
	writel(val, cfg->regs + S3C_MSPRWIDTH);

	return 0;
}
#endif

static int s3c_camif_output_pp_preview(camif_cfg_t *cfg)
{
	int i;
	unsigned int area            = cfg->target_x * cfg->target_y;
	unsigned int one_p_size      = 0;
	unsigned int real_one_p_size = 0;
	
	if (cfg->input_channel)
	{
		s3c_camif_io_duplex_preview(cfg);
		return 0;
	}

	if (cfg->dst_fmt & CAMIF_YCBCR420)
		area = ((area * 3) >> 1);
	else if (cfg->dst_fmt & CAMIF_YCBCR422 || cfg->dst_fmt & CAMIF_YCBCR422I)
		area = (area << 1);
	else if (cfg->dst_fmt & CAMIF_RGB24)
		area = (area << 2);
	else if (cfg->dst_fmt & CAMIF_RGB16)
		area = (area << 1);
	else
		printk(KERN_ERR "Invalid target format %d\n", cfg->dst_fmt);

	
	if ((area % PAGE_SIZE) != 0)
	{
		i = area / PAGE_SIZE;
		area = (i + 1) * PAGE_SIZE;
	}
	
	cfg->buffer_size = area;
	one_p_size       = area;	

	if(MAX_PPNUM < cfg->pp_num)
		printk(KERN_ERR "Invalid pingpong number %d\n", cfg->pp_num);

	for (i = 0; i < MAX_PPNUM; i++)
	{
		if(cfg->flag_img_buf_user_set == 1)
		{
			//cfg->img_buf[i].virt_rgb = cfg->pp_virt_buf + real_one_p_size;
			//cfg->img_buf[i].phys_rgb = cfg->pp_phys_buf + real_one_p_size;
		}
		else
		{

			real_one_p_size = one_p_size * (i % cfg->pp_num);
				
			cfg->img_buf[i].virt_rgb = cfg->pp_virt_buf + real_one_p_size;
			cfg->img_buf[i].phys_rgb = cfg->pp_phys_buf + real_one_p_size;
		}

		writel(cfg->img_buf[i].phys_rgb, cfg->regs + S3C_CIPRYSA(i));
	}

	return 0;
}

static int s3c_camif_output_pp(camif_cfg_t *cfg)
{
	if (cfg->dma_type & CAMIF_CODEC)
		s3c_camif_output_pp_codec(cfg);
	else if ( cfg->dma_type & CAMIF_PREVIEW)
		s3c_camif_output_pp_preview(cfg);

	return 0;
}

static int s3c_camif_output_lcd(camif_cfg_t *cfg)
{
	/* To Be Implemented */
	return 0;
}

static int s3c_camif_setup_output_path(camif_cfg_t *cfg)
{
	if (cfg->output_channel == CAMIF_OUT_FIFO)
		s3c_camif_output_lcd(cfg);
	else
		s3c_camif_output_pp(cfg);

	return 0;
}

/*************************************************************************
 * Scaler part
 ************************************************************************/
static int s3c_camif_set_target_area(camif_cfg_t *cfg)
{
	unsigned int rect = cfg->target_x * cfg->target_y;

	if (cfg->dma_type & CAMIF_CODEC)
		writel(rect, cfg->regs + S3C_CICOTAREA);
	else if (cfg->dma_type & CAMIF_PREVIEW)
		writel(rect, cfg->regs + S3C_CIPRTAREA);

	return 0;
}

#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
static inline int s3c_camif_set_ratio(camif_cfg_t *cfg)
{
	unsigned int cmd = (S3C_CICOSCCTRL_CSCR2Y_WIDE | S3C_CICOSCCTRL_CSCY2R_WIDE);

	if (cfg->dma_type & CAMIF_CODEC)
	{
		writel(S3C_CICOSCPRERATIO_SHFACTOR_CO(cfg->sc.shfactor) | \
			S3C_CICOSCPRERATIO_PREHORRATIO_CO(cfg->sc.prehratio) | \
			S3C_CICOSCPRERATIO_PREVERRATIO_CO(cfg->sc.prevratio), cfg->regs + S3C_CICOSCPRERATIO);

		writel(S3C_CICOSCPREDST_PREDSTWIDTH_CO(cfg->sc.predst_x) | \
			S3C_CICOSCPREDST_PREDSTHEIGHT_CO(cfg->sc.predst_y), cfg->regs + S3C_CICOSCPREDST);

		/* Differ from Preview */
		// (091124 / kcoolsw) : for scalerbypass mode
		//if (cfg->sc.scalerbypass)
		//	cmd |= S3C_CICOSCCTRL_SCALERBYPASS_CO;	// - mihee 090811

		/* Differ from Codec */
		if (cfg->dst_fmt & CAMIF_RGB24)
			cmd |= S3C_CICOSCCTRL_OUTRGB_FMT_RGB888;
		else
			cmd |= S3C_CICOSCCTRL_OUTRGB_FMT_RGB565;

		if (cfg->sc.scaleup_h & cfg->sc.scaleup_v)
			cmd |= (S3C_CICOSCCTRL_SCALEUP_H | S3C_CICOSCCTRL_SCALEUP_V);

		writel(cmd | S3C_CICOSCCTRL_MAINHORRATIO_CO(cfg->sc.mainhratio) | \
			S3C_CICOSCCTRL_MAINVERRATIO_CO(cfg->sc.mainvratio), cfg->regs + S3C_CICOSCCTRL);

	}
	else if (cfg->dma_type & CAMIF_PREVIEW)
	{
		writel(S3C_CIPRSCPRERATIO_SHFACTOR_PR(cfg->sc.shfactor) | \
			S3C_CIPRSCPRERATIO_PREHORRATIO_PR(cfg->sc.prehratio) | \
			S3C_CIPRSCPRERATIO_PREVERRATIO_PR(cfg->sc.prevratio), cfg->regs + S3C_CIPRSCPRERATIO);

		writel(S3C_CIPRSCPREDST_PREDSTWIDTH_PR(cfg->sc.predst_x) | \
			S3C_CIPRSCPREDST_PREDSTHEIGHT_PR(cfg->sc.predst_y), cfg->regs + S3C_CIPRSCPREDST);

		if (cfg->dst_fmt & CAMIF_RGB24)
			cmd |= S3C_CIPRSCCTRL_OUTRGB_FMT_PR_RGB888;
		else
			cmd |= S3C_CIPRSCCTRL_OUTRGB_FMT_PR_RGB565;

		if (cfg->sc.scaleup_h & cfg->sc.scaleup_v)
			cmd |= ((1 << 30) | (1 << 29));

		writel(cmd | S3C_CIPRSCCTRL_MAINHORRATIO_PR(cfg->sc.mainhratio) | \
			S3C_CIPRSCCTRL_MAINVERRATIO_PR(cfg->sc.mainvratio), cfg->regs + S3C_CIPRSCCTRL);

	} else
		printk(KERN_ERR "Invalid DMA type\n");

	return 0;
}
#elif defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
static inline int s3c_camif_set_ratio(camif_cfg_t *cfg)
{
	u32 cmd = 0;

	if (cfg->dma_type & CAMIF_CODEC) {

		writel(S3C_CICOSCPRERATIO_SHFACTOR_CO(cfg->sc.shfactor) | \
			S3C_CICOSCPRERATIO_PREHORRATIO_CO(cfg->sc.prehratio) | \
			S3C_CICOSCPRERATIO_PREVERRATIO_CO(cfg->sc.prevratio), cfg->regs + S3C_CICOSCPRERATIO);

		writel(S3C_CICOSCPREDST_PREDSTWIDTH_CO(cfg->sc.predst_x) | \
			S3C_CICOSCPREDST_PREDSTHEIGHT_CO(cfg->sc.predst_y), cfg->regs + S3C_CICOSCPREDST);

		if (cfg->sc.scalerbypass)
			cmd |= S3C_CICOSCCTRL_SCALERBYPASS_CO;

		if (cfg->sc.scaleup_h & cfg->sc.scaleup_v)
			cmd |= (S3C_CICOSCCTRL_SCALEUP_H | S3C_CICOSCCTRL_SCALEUP_V);

		writel(cmd | S3C_CICOSCCTRL_MAINHORRATIO_CO(cfg->sc.mainhratio) | \
			S3C_CICOSCCTRL_MAINVERRATIO_CO(cfg->sc.mainvratio), cfg->regs + S3C_CICOSCCTRL);

	} else if (cfg->dma_type & CAMIF_PREVIEW) {

		cmd |= S3C_CIPRSCCTRL_SAMPLE_PR;

		writel(S3C_CIPRSCPRERATIO_SHFACTOR_PR(cfg->sc.shfactor) | \
			S3C_CIPRSCPRERATIO_PREHORRATIO_PR(cfg->sc.prehratio) | \
			S3C_CIPRSCPRERATIO_PREVERRATIO_PR(cfg->sc.prevratio), cfg->regs + S3C_CIPRSCPRERATIO);

		writel(S3C_CIPRSCPREDST_PREDSTWIDTH_PR(cfg->sc.predst_x) | \
			S3C_CIPRSCPREDST_PREDSTHEIGHT_PR(cfg->sc.predst_y), cfg->regs + S3C_CIPRSCPREDST);

		if (cfg->dst_fmt & CAMIF_RGB24)
			cmd |= S3C_CIPRSCCTRL_RGBFORMAT_24;

		if (cfg->sc.scaleup_h & cfg->sc.scaleup_v)
			cmd |= ((1 << 29) | (1 << 28));

		writel(cmd | S3C_CIPRSCCTRL_MAINHORRATIO_PR(cfg->sc.mainhratio) | \
			S3C_CIPRSCCTRL_MAINVERRATIO_PR(cfg->sc.mainvratio), cfg->regs + S3C_CIPRSCCTRL);

	} else
		printk(KERN_ERR "Invalid DMA type\n");

	return 0;
}
#endif

static int s3c_camif_calc_ratio(unsigned int src_width, unsigned int dst_width, unsigned int *ratio, unsigned int *shift)
{
	// kcoolsw
	//if (src_width >= 64 * dst_width)
	if (src_width >= (dst_width << 6))
	{
		printk(KERN_ERR "Out of pre-scaler range: src_width / dst_width = %d (< 64)\n", src_width / dst_width);
        *ratio = 32;
        *shift = 5;
		return 1;
	}
	//else if (src_width >= 32 * dst_width)
	else if (src_width >= (dst_width << 5))
	{
		*ratio = 32;
		*shift = 5;
	}
	//else if (src_width >= 16 * dst_width)
	else if (src_width >= (dst_width << 4))
	{
		*ratio = 16;
		*shift = 4;
	}
	//else if (src_width >= 8 * dst_width)
	else if (src_width >= (dst_width << 3))
	{
		*ratio = 8;
		*shift = 3;
	}
	//else if (src_width >= 4 * dst_width)
	else if (src_width >= (dst_width << 2))
	{
		*ratio = 4;
		*shift = 2;
	}
	//else if (src_width >= 2 * dst_width)
	else if (src_width >= (dst_width << 1))
	{
		*ratio = 2;
		*shift = 1;
	}
	else
	{
		*ratio = 1;
		*shift = 0;
	}

	return 0;
}

static int s3c_camif_setup_scaler(camif_cfg_t *cfg)
{
	int tx = cfg->target_x, ty=cfg->target_y;
	int sx, sy;

	if (tx <= 0 || ty <= 0) {
		printk(KERN_ERR "Invalid target size\n");
		return -1;
	}

	sx = cfg->cis->source_x - (cfg->cis->win_hor_ofst + cfg->cis->win_hor_ofst2);
	sy = cfg->cis->source_y - (cfg->cis->win_ver_ofst + cfg->cis->win_ver_ofst2);	// mihee 080623	

	if (sx <= 0 || sy <= 0)	{
		printk(KERN_ERR "Invalid source size\n");
		return -1;
	}

	cfg->sc.modified_src_x = sx;
	cfg->sc.modified_src_y = sy;

	// added by sangyub 
	//printk("[fimc]%s:: source x = %d, source y = %d, target x = %d, target y = %d\n", __FUNCTION__,sx,sy,tx,ty);

	/* Pre-scaler control register 1 */
	s3c_camif_calc_ratio(sx, tx, &cfg->sc.prehratio, &cfg->sc.hfactor);
	s3c_camif_calc_ratio(sy, ty, &cfg->sc.prevratio, &cfg->sc.vfactor);

#if defined(CONFIG_CPU_S3C6410)
	if (cfg->dma_type & CAMIF_PREVIEW)
	{
		if ((sx / cfg->sc.prehratio) > 720)
		{
			printk(KERN_INFO "Internal preview line buffer length is 720 pixels\n");
			printk(KERN_INFO "Decrease the resolution or adjust window offset values appropriately\n");
		}
	}
#else
	if (cfg->dma_type & CAMIF_PREVIEW)
	{
		if ((sx / cfg->sc.prehratio) > 640)
		{
			printk(KERN_INFO "Internal preview line buffer length is 640 pixels\n");
			printk(KERN_INFO "Decrease the resolution or adjust window offset values appropriately\n");
		}
	}
#endif

	cfg->sc.shfactor = 10 - (cfg->sc.hfactor + cfg->sc.vfactor);

	/* Pre-scaler control register 2 */
	cfg->sc.predst_x = sx / cfg->sc.prehratio;
	cfg->sc.predst_y = sy / cfg->sc.prevratio;

	/* Main-scaler control register */
	cfg->sc.mainhratio = (sx << 8) / (tx << cfg->sc.hfactor);
	cfg->sc.mainvratio = (sy << 8) / (ty << cfg->sc.vfactor);

	if((cfg->sc.mainhratio & 0x01) && (cfg->sc.mainhratio != 0x01))
		cfg->sc.mainhratio--;

	if((cfg->sc.mainvratio & 0x01) && (cfg->sc.mainvratio != 0x01))
		cfg->sc.mainvratio--;

	cfg->sc.scaleup_h  = (sx <= tx) ? 1 : 0;
	cfg->sc.scaleup_v  = (sy <= ty) ? 1 : 0;

	s3c_camif_set_ratio(cfg);
	s3c_camif_set_target_area(cfg);

	return 0;
}

/*************************************************************************
 * Format part
 ************************************************************************/
int s3c_camif_set_source_format(camif_cis_t *cis)
{
	camif_cfg_t *cfg = s3c_camif_get_fimc_object(CODEC_MINOR);
	unsigned int cmd = 0;

	/* Configure CISRCFMT --Source Format */
	if (cis->itu_fmt & CAMIF_ITU601)
		cmd = CAMIF_ITU601;
	else {
		assert(cis->itu_fmt & CAMIF_ITU656);
		cmd = CAMIF_ITU656;
	}

	cmd |= (S3C_CISRCFMT_SOURCEHSIZE(cis->source_x) | S3C_CISRCFMT_SOURCEVSIZE(cis->source_y));

	/* Order422 */
	cmd |= cis->order422;
	writel(cmd, cfg->regs + S3C_CISRCFMT);

#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
#if 0	/* Fixed to YCBYCR (Output order memory storing style) */
	cmd = (cis->order422 >> 14);
	writel((readl(cfg->regs + S3C_CICOCTRL) & ~(0x3 << 0)) | cmd, cfg->regs + S3C_CICOCTRL);
#else
/* throughc : for jpeg capture CICOCTRL 00 replace 11 
 *            Order422_Co [1:0]   : 11 Cr0Y0Cb0Y1
 *                                  00 Y0Cb0Y1Cr0
 * */
	if(cfg->sc.scalerbypass)
	{	cmd = (cis->order422 >> 14);
		writel((readl(cfg->regs + S3C_CICOCTRL) & ~(0x3 << 0)) | cmd, cfg->regs + S3C_CICOCTRL);
	}
	else
	{
		cmd = CAMIF_YCBYCR;		// + mihee 090725
		writel((readl(cfg->regs + S3C_CICOCTRL) & ~(0x3 << 0)) | cmd, cfg->regs + S3C_CICOCTRL);
	}
#endif
#endif

	return 0;
}

#if defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
static int s3c_camif_set_target_format(camif_cfg_t *cfg)
{
	unsigned int cmd = 0;

	if (cfg->dma_type & CAMIF_CODEC)
	{
		cmd |= S3C_CICOTRGFMT_TARGETHSIZE_CO(cfg->target_x) | S3C_CICOTRGFMT_TARGETVSIZE_CO(cfg->target_y);

		if (cfg->dst_fmt & CAMIF_YCBCR420)
		{
			cmd |= (S3C_CICOTRGFMT_OUT422_420 | S3C_CICOTRGFMT_IN422_422);
			writel(cmd, cfg->regs + S3C_CICOTRGFMT);
		}
		else if (cfg->dst_fmt & CAMIF_YCBCR422)
		{
			cmd |= (S3C_CICOTRGFMT_OUT422_422 | S3C_CICOTRGFMT_IN422_422);
			writel(cmd, cfg->regs + S3C_CICOTRGFMT);
		}
		else if ((cfg->dst_fmt & CAMIF_RGB24) || (cfg->dst_fmt & CAMIF_RGB16))
		{
			cmd |= (S3C_CICOTRGFMT_OUT422_422 | S3C_CICOTRGFMT_IN422_422);
			writel(cmd | (1 << 29), cfg->regs + S3C_CICOTRGFMT);
		}
		else
			printk(KERN_ERR "Invalid target format\n");
	}
	else
	{
		assert(cfg->dma_type & CAMIF_PREVIEW);

		cmd = readl(cfg->regs + S3C_CIPRTRGFMT);
		cmd &= (S3C_CIPRTRGFMT_TARGETHSIZE_PR(0) | S3C_CIPRTRGFMT_TARGETVSIZE_PR(0));
		cmd |= (S3C_CIPRTRGFMT_TARGETHSIZE_PR(cfg->target_x) | S3C_CIPRTRGFMT_TARGETVSIZE_PR(cfg->target_y));

		writel(cmd | (2 << 30), cfg->regs + S3C_CIPRTRGFMT);
	}

	return 0;
}
#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
static int s3c_camif_set_target_format(camif_cfg_t *cfg)
{
	unsigned int cmd = 0;

	if (cfg->dma_type & CAMIF_CODEC)
	{
		cmd |= S3C_CICOTRGFMT_TARGETHSIZE_CO(cfg->target_x);
		cmd |= S3C_CICOTRGFMT_TARGETVSIZE_CO(cfg->target_y);

		if (cfg->dst_fmt & CAMIF_YCBCR420)
		{
			cmd |= S3C_CICOTRGFMT_OUTFORMAT_YCBCR420OUT;
			writel(cmd, cfg->regs + S3C_CICOTRGFMT);
		}
		else if (cfg->dst_fmt & CAMIF_YCBCR422)
		{
			cmd |= S3C_CICOTRGFMT_OUTFORMAT_YCBCR422OUT;
			writel(cmd, cfg->regs + S3C_CICOTRGFMT);
		}
		else if (cfg->dst_fmt & CAMIF_YCBCR422I)
		{
			cmd |= S3C_CICOTRGFMT_OUTFORMAT_YCBCR422OUTINTERLEAVE;
			writel(cmd, cfg->regs + S3C_CICOTRGFMT);
		}
		else if ((cfg->dst_fmt & CAMIF_RGB24) || (cfg->dst_fmt & CAMIF_RGB16))
		{
			cmd |= S3C_CICOTRGFMT_OUTFORMAT_RGBOUT;
			writel(cmd, cfg->regs + S3C_CICOTRGFMT);
		}
		else
			printk(KERN_ERR "Invalid target format\n");
	}
	else
	{
		assert(cfg->dma_type & CAMIF_PREVIEW);

		cmd = readl(cfg->regs + S3C_CIPRTRGFMT);
		cmd &= (S3C_CIPRTRGFMT_TARGETHSIZE_PR(0)             | S3C_CIPRTRGFMT_TARGETVSIZE_PR(0));
		cmd |= (S3C_CIPRTRGFMT_TARGETHSIZE_PR(cfg->target_x) | S3C_CIPRTRGFMT_TARGETVSIZE_PR(cfg->target_y));

		if (cfg->dst_fmt & CAMIF_YCBCR420)
			cmd |= S3C_CICOTRGFMT_OUTFORMAT_YCBCR420OUT;
		else if (cfg->dst_fmt & CAMIF_YCBCR422)
			cmd |= S3C_CICOTRGFMT_OUTFORMAT_YCBCR422OUT;
		else if (cfg->dst_fmt & CAMIF_YCBCR422I)
			cmd |= S3C_CICOTRGFMT_OUTFORMAT_YCBCR422OUTINTERLEAVE;
		else if ((cfg->dst_fmt & CAMIF_RGB24) || (cfg->dst_fmt & CAMIF_RGB16))
			cmd |= S3C_CICOTRGFMT_OUTFORMAT_RGBOUT;
		else
			printk(KERN_ERR "Invalid target format\n");

		writel(cmd, cfg->regs + S3C_CIPRTRGFMT);
	}

	return 0;
}
#endif

/*************************************************************************
 * Control part
 ************************************************************************/
int s3c_camif_control_fimc(camif_cfg_t *cfg)
{
	if (s3c_camif_request_memory(cfg)) {
		printk(KERN_ERR "Instead of using consistent_alloc(). Let me use dedicated mem for DMA\n");
		return -1;
	}

	s3c_camif_setup_input_path(cfg);

	if (s3c_camif_setup_scaler(cfg)) {
		printk(KERN_ERR "Preview scaler fault: change WinHorOfset or target size\n");
		return 1;
	}

	s3c_camif_set_target_format(cfg);

	if (s3c_camif_setup_dma(cfg)) {
		printk(KERN_ERR "DMA burst length error\n");
		return 1;
	}

	s3c_camif_setup_output_path(cfg);

	return 0;
}

int s3c_camif_start_dma(camif_cfg_t *cfg)
{
	unsigned int n_cmd = readl(cfg->regs + S3C_CIIMGCPT);
	unsigned int val;
	unsigned int timeout = 300;

restart_dma:
	switch(cfg->capture_enable) {
	case CAMIF_BOTH_DMA_ON:
		s3c_camif_reset(CAMIF_RESET, 0); /* Flush Camera Core Buffer */

		// (091124 / kcoolsw) : for scalerbypass mode
		/* For Codec */
		if (!cfg->sc.scalerbypass)
		{
			val = readl(cfg->regs + S3C_CICOSCCTRL);
			val |= S3C_CICOSCCTRL_COSCALERSTART;
			writel(val, cfg->regs + S3C_CICOSCCTRL);
		}		

		/* For Preview */
		val = readl(cfg->regs + S3C_CIPRSCCTRL);
		val |= S3C_CIPRSCCTRL_START;
		writel(val, cfg->regs + S3C_CIPRSCCTRL);

		if (!cfg->sc.scalerbypass)
			n_cmd |= S3C_CIIMGCPT_IMGCPTEN_COSC | S3C_CIIMGCPT_IMGCPTEN_PRSC;

		break;

	case CAMIF_DMA_ON:

		s3c_camif_reset(CAMIF_RESET, 0); /* Flush Camera Core Buffer */	// - mihee 090730 multishot lockup
		
		if (cfg->dma_type & CAMIF_CODEC) {

			// (091124 / kcoolsw) : for scalerbypass mode
			/*
			val = readl(cfg->regs + S3C_CICOSCCTRL);
			val |= S3C_CICOSCCTRL_COSCALERSTART;
			writel(val, cfg->regs + S3C_CICOSCCTRL);

			if (!cfg->sc.scalerbypass)
				n_cmd |= S3C_CIIMGCPT_IMGCPTEN_COSC;
			*/
			
			if (!cfg->sc.scalerbypass) {			// + mihee 090415
				val = readl(cfg->regs + S3C_CICOSCCTRL);
				val |= S3C_CICOSCCTRL_COSCALERSTART;
				writel(val, cfg->regs + S3C_CICOSCCTRL);
				n_cmd |= S3C_CIIMGCPT_IMGCPTEN_COSC;
			}
			else {
				val = readl(cfg->regs + S3C_CICOSCCTRL);
				val |= S3C_CICOSCCTRL_SCALERBYPASS_CO;	// + mihee 090811
				writel(val, cfg->regs + S3C_CICOSCCTRL);
			}

#if defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
			n_cmd |= (1 << 24);
#endif
		} else {
			val = readl(cfg->regs + S3C_CIPRSCCTRL);
			val |= S3C_CIPRSCCTRL_START;
			writel(val, cfg->regs + S3C_CIPRSCCTRL);

			if (!cfg->sc.scalerbypass)
				n_cmd |= S3C_CIIMGCPT_IMGCPTEN_PRSC;
		}

		/* wait until Sync Time expires */
		/* First settting, to wait VSYNC fall  */
		/* By VESA spec,in 640x480 @60Hz
		   MAX Delay Time is around 64us which "while" has.*/

		while(1)
		{
			if((S3C_CICOSTATUS_VSYNC & readl(cfg->regs + S3C_CICOSTATUS)))
				break;
			msleep(1);
			timeout--;
	//		printk("timeout = %d\n", timeout);
			if(!timeout){
				n_cmd = readl(cfg->regs + S3C_CIIMGCPT);
				timeout = 300;
				goto restart_dma;
			}

		}
		break;

	default:
		break;
	}

#if defined(CONFIG_CPU_S3C2443)
	if (cfg->dma_type & CAMIF_CODEC) {
		if (cfg->dst_fmt & CAMIF_RGB24)
			n_cmd |= (3 << 25);
		else if (cfg->dst_fmt & CAMIF_RGB16)
			n_cmd |= (1 << 25);
		else if (cfg->dst_fmt & CAMIF_YCBCR420)
			n_cmd |= (2 << 25);
	}
#endif

	val = readl(cfg->regs + S3C_CIIMGCPT);
	val &= ~(0x7 << 29);
	writel(val | n_cmd | S3C_CIIMGCPT_IMGCPTEN, cfg->regs + S3C_CIIMGCPT);

	return 0;
}

int s3c_camif_stop_dma(camif_cfg_t *cfg)
{
	unsigned int n_cmd = readl(cfg->regs + S3C_CIIMGCPT);
	unsigned int val;

	switch(cfg->capture_enable) {
	case CAMIF_BOTH_DMA_OFF:
		val = readl(cfg->regs + S3C_CICOSCCTRL);
		val &= ~S3C_CICOSCCTRL_COSCALERSTART;
		writel(val, cfg->regs + S3C_CICOSCCTRL);

		val = readl(cfg->regs + S3C_CIPRSCCTRL);
		val &= ~S3C_CIPRSCCTRL_START;
		writel(val, cfg->regs + S3C_CIPRSCCTRL);

		n_cmd = 0;
		break;

	case CAMIF_DMA_OFF_L_IRQ: /* fall thru */
	case CAMIF_DMA_OFF:
		if (cfg->dma_type & CAMIF_CODEC) {
			//printk("[fimc]%s:: CODEC DMA OFF \n", __FUNCTION__);
			val = readl(cfg->regs + S3C_CICOSCCTRL);

			// (091124 / kcoolsw) : for scalerbypass mode
			//val &= ~S3C_CICOSCCTRL_COSCALERSTART;
			val &= ~(S3C_CICOSCCTRL_COSCALERSTART | S3C_CICOSCCTRL_SCALERBYPASS_CO);    // + mihee 090811

			writel(val, cfg->regs + S3C_CICOSCCTRL);
			n_cmd &= ~S3C_CIIMGCPT_IMGCPTEN_COSC;

			if (!(n_cmd & S3C_CIIMGCPT_IMGCPTEN_PRSC))
				n_cmd = 0;
		} else {
			val = readl(cfg->regs + S3C_CIPRSCCTRL);
			val &= ~S3C_CIPRSCCTRL_START;
			writel(val, cfg->regs + S3C_CIPRSCCTRL);

			n_cmd &= ~S3C_CIIMGCPT_IMGCPTEN_PRSC;

			if (!(n_cmd & S3C_CIIMGCPT_IMGCPTEN_COSC))
				n_cmd = 0;
		}

		break;

	default:
		printk(KERN_ERR "Unexpected DMA control\n");
	}

	writel(n_cmd, cfg->regs + S3C_CIIMGCPT);

	if (cfg->capture_enable == CAMIF_DMA_OFF_L_IRQ) { /* Last IRQ  */
		if (cfg->dma_type & CAMIF_CODEC) {
			val = readl(cfg->regs + S3C_CICOCTRL);
			val |= S3C_CICOCTRL_LASTIRQEN;
			writel(val, cfg->regs + S3C_CICOCTRL);
		} else {
			val = readl(cfg->regs + S3C_CIPRCTRL);
			val |= S3C_CIPRCTRL_LASTIRQEN_ENABLE;
			writel(val, cfg->regs + S3C_CIPRCTRL);
		}
	}

	return 0;
}

#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
int s3c_camif_start_codec_msdma(camif_cfg_t *cfg)
{
	int ret = 0;
	u32 val;

	val = readl(cfg->regs + S3C_MSCOCTRL);
	val &= ~(1 << 0);
	writel(val, cfg->regs + S3C_MSCOCTRL);

	val = readl(cfg->regs + S3C_MSCOCTRL);
	val |= (1 << 0);
	writel(val, cfg->regs + S3C_MSCOCTRL);

	return ret;
}
#endif

int s3c_camif_start_preview_msdma(camif_cfg_t * cfg)
{
	unsigned int val;
	int ret = 0;

#if !defined(CONFIG_CPU_S3C6400) && !defined(CONFIG_CPU_S3C6410)
	val = readl(cfg->regs + S3C_CIMSCTRL);
	val &= ~(1 << 0);
	writel(val, cfg->regs + S3C_CIMSCTRL);
#endif
	val = readl(cfg->regs + S3C_CIMSCTRL);
	val |= (1 << 0);
	writel(val, cfg->regs + S3C_CIMSCTRL);

	while((readl(cfg->regs + S3C_CIMSCTRL) & (1 << 6)) == 0);

	return ret;
}

void s3c_camif_change_flip(camif_cfg_t *cfg)
{
	unsigned int cmd = 0;

	if (cfg->dma_type & CAMIF_CODEC) {
		cmd  = readl(cfg->regs + S3C_CICOTRGFMT);
		cmd &= ~((1 << 14) | (1 << 15));
		cmd |= cfg->flip;
		writel(cmd, cfg->regs + S3C_CICOTRGFMT);
	} else {
		/* if ROT90_Pr == 1, dma burst length must be 4 */
		if (cfg->flip == CAMIF_ROTATE_90 || cfg->flip == CAMIF_FLIP_ROTATE_270) {
			cmd = readl(cfg->regs + S3C_CIPRCTRL);
			cmd &= ~(0x3ff << 14);
			cmd |= (S3C_CICOCTRL_YBURST1_CO(4) | S3C_CICOCTRL_YBURST2_CO(4));
			writel(cmd, cfg->regs + S3C_CIPRCTRL);
		}

		cmd  = readl(cfg->regs + S3C_CIPRTRGFMT);
		cmd &= ~(0x7 << 13);
		cmd |= cfg->flip;
		writel(cmd, cfg->regs + S3C_CIPRTRGFMT);
	}
}

void s3c_camif_change_effect(camif_cfg_t *cfg)
{
	unsigned int val = readl(cfg->regs + S3C_CIIMGEFF);
	val &= ~((1 << 28) | (1 << 27) | (1 << 26));

#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
	val |= ((1 << 31) | (1 << 30));
#endif

	switch(cfg->effect) {
	case CAMIF_SILHOUETTE:
		val |= S3C_CIIMGEFF_FIN_SILHOUETTE;
		break;

	case CAMIF_EMBOSSING:
		val |= S3C_CIIMGEFF_FIN_EMBOSSING;
		break;

	case CAMIF_ART_FREEZE:
		val |= S3C_CIIMGEFF_FIN_ARTFREEZE;
		break;

	case CAMIF_NEGATIVE:
		val |= S3C_CIIMGEFF_FIN_NEGATIVE;
		break;

	case CAMIF_ARBITRARY_CB_CR:
		val |= S3C_CIIMGEFF_FIN_ARBITRARY;
		break;

	case CAMIF_BYPASS:
	default:
		break;
	}

	writel(val, cfg->regs + S3C_CIIMGEFF);
}

int s3c_camif_do_postprocess(camif_cfg_t *cfg)
{
	unsigned int val= readl(cfg->regs + S3C_CIMSCTRL);

	if (cfg->dst_fmt & CAMIF_YCBCR420)
		val |= (1 << 1);
	else
		val &= ~(1 << 1);

	val &= ~(1 << 0);
	writel(val, cfg->regs + S3C_CIMSCTRL);

	val |= (1 << 0);
	writel(val, cfg->regs + S3C_CIMSCTRL);

	printk(KERN_INFO "Postprocessing started\n");

	while((readl(cfg->regs + S3C_CIMSCTRL) & (1 << 6)) == 0);

	printk(KERN_INFO "Postprocessing finished\n");

	return 0;
}

int s3c_camif_set_offset(camif_cis_t *cis)
{
	camif_cfg_t *cfg = s3c_camif_get_fimc_object(CODEC_MINOR);
	unsigned int h = cis->win_hor_ofst;     /* Camera input offset ONLY */
	unsigned int v = cis->win_ver_ofst;     /* Camera input offset ONLY */
	unsigned int h2 = cis->win_hor_ofst2;	/* Camera input offset ONLY */
	unsigned int v2 = cis->win_ver_ofst2;	/* Camera input offset ONLY */

	/*Clear Overflow */
	writel(S3C_CIWDOFST_CLROVCOFIY | S3C_CIWDOFST_CLROVCOFICB | \
		S3C_CIWDOFST_CLROVCOFICR | S3C_CIWDOFST_CLROVPRFICB | \
		S3C_CIWDOFST_CLROVPRFICR, cfg->regs + S3C_CIWDOFST);

	writel(0, cfg->regs + S3C_CIWDOFST);

	if (!h && !v) {
		writel(0, cfg->regs + S3C_CIWDOFST);
		writel(0, cfg->regs + S3C_CIDOWSFT2);
		return 0;
	}

	writel(S3C_CIWDOFST_WINOFSEN | S3C_CIWDOFST_WINHOROFST(h) | S3C_CIWDOFST_WINVEROFST(v), cfg->regs + S3C_CIWDOFST);
	writel(S3C_CIDOWSFT2_WINHOROFST2(h) | S3C_CIDOWSFT2_WINVEROFST2(v), cfg->regs + S3C_CIDOWSFT2);
	writel(S3C_CIDOWSFT2_WINHOROFST2(h2) | S3C_CIDOWSFT2_WINVEROFST2(v2), cfg->regs + S3C_CIDOWSFT2);

	return 0;
}

void s3c_camif_set_priority(int flag)
{
	unsigned int val;

	if (flag) {
		irq_old_priority = readl(S3C_PRIORITY);
		val = irq_old_priority;
		val &= ~(3 << 7);
		writel(val, S3C_PRIORITY);

		/* Arbiter 1, REQ2 first */
		val |=  (1 << 7);
		writel(val, S3C_PRIORITY);

		/* Disable Priority Rotate */
		val &= ~(1 << 1);
		writel(val, S3C_PRIORITY);
	} else
		writel(irq_old_priority, S3C_PRIORITY);
}

/*************************************************************************
 * Interrupt part
 ************************************************************************/
void s3c_camif_enable_lastirq(camif_cfg_t *cfg)
{
	unsigned int val;

	if (cfg->capture_enable == CAMIF_BOTH_DMA_ON) {
		val = readl(cfg->regs + S3C_CICOCTRL);
		val |= S3C_CICOCTRL_LASTIRQEN;
		writel(val, cfg->regs + S3C_CICOCTRL);

		val = readl(cfg->regs + S3C_CIPRCTRL);
		val |= S3C_CIPRCTRL_LASTIRQEN_ENABLE;
		writel(val, cfg->regs + S3C_CIPRCTRL);
	} else {
		if (cfg->dma_type & CAMIF_CODEC) {
			val = readl(cfg->regs + S3C_CICOCTRL);
			val |= S3C_CICOCTRL_LASTIRQEN;
			writel(val, cfg->regs + S3C_CICOCTRL);
		} else {
			val = readl(cfg->regs + S3C_CIPRCTRL);
			val |= S3C_CIPRCTRL_LASTIRQEN_ENABLE;
			writel(val, cfg->regs + S3C_CIPRCTRL);
		}
	}
}

void s3c_camif_disable_lastirq(camif_cfg_t *cfg)
{
	unsigned int val;

	if (cfg->capture_enable == CAMIF_BOTH_DMA_ON) {
		val = readl(cfg->regs + S3C_CICOCTRL);
		val &= ~S3C_CICOCTRL_LASTIRQEN;
		writel(val, cfg->regs + S3C_CICOCTRL);

		val = readl(cfg->regs + S3C_CIPRCTRL);
		val &= ~S3C_CIPRCTRL_LASTIRQEN_ENABLE;
		writel(val, cfg->regs + S3C_CIPRCTRL);
	} else {
		if (cfg->dma_type & CAMIF_CODEC) {
			val = readl(cfg->regs + S3C_CICOCTRL);
			val &= ~S3C_CICOCTRL_LASTIRQEN;
			writel(val, cfg->regs + S3C_CICOCTRL);
		} else {
			val = readl(cfg->regs + S3C_CIPRCTRL);
			val &= ~S3C_CIPRCTRL_LASTIRQEN_ENABLE;
			writel(val, cfg->regs + S3C_CIPRCTRL);
		}
	}
}

#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
void s3c_camif_clear_irq(int irq)
{
	camif_cfg_t *cfg = s3c_camif_get_fimc_object(CODEC_MINOR);
	unsigned int val = 0;

	if (irq == IRQ_CAMIF_C) {
		val = readl(cfg->regs + S3C_CIGCTRL);
		val |= (1 << 19);
	} else if (irq == IRQ_CAMIF_P) {
		val = readl(cfg->regs + S3C_CIGCTRL);
		val |= (1 << 18);
	}

	writel(val, cfg->regs + S3C_CIGCTRL);
}
#else
void s3c_camif_clear_irq(int irq)
{
}
#endif

/*************************************************************************
 * Initialize part
 ************************************************************************/
#if defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
static int s3c_camif_set_gpio(void)
{
	s3c2410_gpio_cfgpin(S3C2440_GPJ0, S3C2440_GPJ0_CAMDATA0);
	s3c2410_gpio_cfgpin(S3C2440_GPJ1, S3C2440_GPJ1_CAMDATA1);
	s3c2410_gpio_cfgpin(S3C2440_GPJ2, S3C2440_GPJ2_CAMDATA2);
	s3c2410_gpio_cfgpin(S3C2440_GPJ3, S3C2440_GPJ3_CAMDATA3);
	s3c2410_gpio_cfgpin(S3C2440_GPJ4, S3C2440_GPJ4_CAMDATA4);
	s3c2410_gpio_cfgpin(S3C2440_GPJ5, S3C2440_GPJ5_CAMDATA5);
	s3c2410_gpio_cfgpin(S3C2440_GPJ6, S3C2440_GPJ6_CAMDATA6);
	s3c2410_gpio_cfgpin(S3C2440_GPJ7, S3C2440_GPJ7_CAMDATA7);
	s3c2410_gpio_cfgpin(S3C2440_GPJ8, S3C2440_GPJ8_CAMPCLK);
	s3c2410_gpio_cfgpin(S3C2440_GPJ9, S3C2440_GPJ9_CAMVSYNC);
	s3c2410_gpio_cfgpin(S3C2440_GPJ10, S3C2440_GPJ10_CAMHREF);
	s3c2410_gpio_cfgpin(S3C2440_GPJ11, S3C2440_GPJ11_CAMCLKOUT);
	s3c2410_gpio_cfgpin(S3C2440_GPJ12, S3C2440_GPJ12_CAMRESET);

	writel(0x1fff, S3C2443_GPJDN);

	return 0;
}
#elif defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
static int s3c_camif_set_gpio(void)
{
	int i;

	s3c_gpio_cfgpin(S3C64XX_GPF(0), S3C64XX_GPF0_CAMIF_CLK);
	s3c_gpio_cfgpin(S3C64XX_GPF(1), S3C64XX_GPF1_CAMIF_HREF);
	s3c_gpio_cfgpin(S3C64XX_GPF(2), S3C64XX_GPF2_CAMIF_PCLK);
	s3c_gpio_cfgpin(S3C64XX_GPF(3), S3C64XX_GPF3_CAMIF_nRST);
	s3c_gpio_cfgpin(S3C64XX_GPF(4), S3C64XX_GPF4_CAMIF_VSYNC);
	s3c_gpio_cfgpin(S3C64XX_GPF(5), S3C64XX_GPF5_CAMIF_YDATA0);
	s3c_gpio_cfgpin(S3C64XX_GPF(6), S3C64XX_GPF6_CAMIF_YDATA1);
	s3c_gpio_cfgpin(S3C64XX_GPF(7), S3C64XX_GPF7_CAMIF_YDATA2);
	s3c_gpio_cfgpin(S3C64XX_GPF(8), S3C64XX_GPF8_CAMIF_YDATA3);
	s3c_gpio_cfgpin(S3C64XX_GPF(9), S3C64XX_GPF9_CAMIF_YDATA4);
	s3c_gpio_cfgpin(S3C64XX_GPF(10), S3C64XX_GPF10_CAMIF_YDATA5);
	s3c_gpio_cfgpin(S3C64XX_GPF(11), S3C64XX_GPF11_CAMIF_YDATA6);
	s3c_gpio_cfgpin(S3C64XX_GPF(12), S3C64XX_GPF12_CAMIF_YDATA7);
	s3c_gpio_cfgpin(S3C64XX_GPB(4), S3C64XX_GPB4_CAM_FIELD);

	for (i = 0; i < 12; i++)
		s3c_gpio_setpull(S3C64XX_GPF(i), S3C_GPIO_PULL_UP);

	s3c_gpio_setpull(S3C64XX_GPB(4), S3C_GPIO_PULL_UP);

/*
	s3c_gpio_cfgpin(S3C64XX_GPF(5), S3C_GPF5_CAMIF_YDATA0);
	s3c_gpio_cfgpin(S3C64XX_GPF(6), S3C_GPF6_CAMIF_YDATA1);
	s3c_gpio_cfgpin(S3C64XX_GPF(7), S3C_GPF7_CAMIF_YDATA2);
	s3c_gpio_cfgpin(S3C64XX_GPF(8), S3C_GPF8_CAMIF_YDATA3);
	s3c_gpio_cfgpin(S3C64XX_GPF(9), S3C_GPF9_CAMIF_YDATA4);
	s3c_gpio_cfgpin(S3C64XX_GPF(10), S3C_GPF10_CAMIF_YDATA5);
	s3c_gpio_cfgpin(S3C64XX_GPF(11), S3C_GPF11_CAMIF_YDATA6);
	s3c_gpio_cfgpin(S3C64XX_GPF(12), S3C_GPF12_CAMIF_YDATA7);
	s3c_gpio_cfgpin(S3C64XX_GPF(2), S3C_GPF2_CAMIF_CLK);
	s3c_gpio_cfgpin(S3C64XX_GPF(4), S3C_GPF4_CAMIF_VSYNC);
	s3c_gpio_cfgpin(S3C64XX_GPF(1), S3C_GPF1_CAMIF_HREF);
	s3c_gpio_cfgpin(S3C64XX_GPF(0), S3C_GPF0_CAMIF_CLK);
	s3c_gpio_cfgpin(S3C64XX_GPF(3), S3C_GPF3_CAMIF_RST);

	writel(0, S3C64XX_GPFPUD);
*/
	return 0;
}
#endif

void s3c_camif_reset(int is, int delay)
{
	camif_cfg_t *cfg = s3c_camif_get_fimc_object(CODEC_MINOR);
	unsigned int val;
	unsigned int tmp;

	switch (is) {
	case CAMIF_RESET:
		tmp = readl(cfg->regs + S3C_CISRCFMT);

		// (091124 / kcoolsw) : for continuous zooming
		//if (tmp &= (1 << 31)) {
		if (tmp & (1 << 31)) {
			//tmp |= (1 << 31);
			//writel(tmp, cfg->regs + S3C_CISRCFMT);
			
			/* ITU-R BT 601 */
			val = readl(cfg->regs + S3C_CIGCTRL);
			val |= S3C_CIGCTRL_SWRST;

#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
			val |= S3C_CIGCTRL_IRQ_LEVEL;
#endif
			// (091124 / kcoolsw) : for removing lock-up when streamoff/on repeatly
			//                      this is the root cause about lock-up...
			msleep(50);
												
			writel(val, cfg->regs + S3C_CIGCTRL);
//			mdelay(1);
			msleep(1);

			val = readl(cfg->regs + S3C_CIGCTRL);
			val &= ~S3C_CIGCTRL_SWRST;
			writel(val, cfg->regs + S3C_CIGCTRL);
		} else {
			/* ITU-R BT 656 */
			tmp = readl(cfg->regs + S3C_CISRCFMT);
			tmp |= (1 << 31);
			writel(tmp, cfg->regs + S3C_CISRCFMT);

			val = readl(cfg->regs + S3C_CIGCTRL);
			val |= S3C_CIGCTRL_SWRST;

#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
			val |= S3C_CIGCTRL_IRQ_LEVEL;
#endif
			writel(val, cfg->regs + S3C_CIGCTRL);
//			mdelay(1);
			msleep(1);

			val = readl(cfg->regs + S3C_CIGCTRL);
			val &= ~S3C_CIGCTRL_SWRST;
			writel(val, cfg->regs + S3C_CIGCTRL);

			tmp = readl(cfg->regs + S3C_CISRCFMT);
			tmp &= ~(1 << 31);
			writel(tmp, cfg->regs + S3C_CISRCFMT);
		}

		break;

	case CAMIF_EX_RESET_AH:
		val = readl(cfg->regs + S3C_CIGCTRL);
		val |= S3C_CIGCTRL_CAMRST;
		writel(val, cfg->regs + S3C_CIGCTRL);
	//	udelay(200);
		msleep(1);	

		val = readl(cfg->regs + S3C_CIGCTRL);
		val &= ~S3C_CIGCTRL_CAMRST;
		writel(val, cfg->regs + S3C_CIGCTRL);
		tmp = (delay + 999)/1000;
//		udelay(delay);
		if(tmp)
		   msleep(tmp);
		else
		  msleep(1);		
		break;

	case CAMIF_EX_RESET_AL:
		val = readl(cfg->regs + S3C_CIGCTRL);
		val &= ~S3C_CIGCTRL_CAMRST;
		writel(val, cfg->regs + S3C_CIGCTRL);
	//	udelay(200);
		msleep(1);	

		val = readl(cfg->regs + S3C_CIGCTRL);
		val |= S3C_CIGCTRL_CAMRST;
		writel(val, cfg->regs + S3C_CIGCTRL);
		tmp = (delay + 999)/1000;
                if(tmp)
                  msleep(tmp);
                else
                  msleep(1);
//		udelay(delay);
				
		break;

	default:
		break;
	}
}

void s3c_camif_init(void)
{
	s3c_camif_reset(CAMIF_RESET, 0);
	s3c_camif_set_gpio();
}

