/* linux/drivers/media/video/samsung/s3c_fimc4x_regs.c
 *
 * Register interface file for Samsung Camera Interface (FIMC) driver
 *
 * Jinsung Yang, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/io.h>
#include <mach/map.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-gpio.h>
#include <plat/regs-lcd.h>
#include <plat/regs-fimc.h>
#include <plat/fimc.h>

#include "s3c_fimc.h"

void s3c_fimc_clear_irq(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIGCTRL);

	if (ctrl->id == 1)
		cfg |= S3C_CIGCTRL_IRQ_CLR_P;
	else
		cfg |= S3C_CIGCTRL_IRQ_CLR_C;

	writel(cfg, ctrl->regs + S3C_CIGCTRL);
}

static int s3c_fimc_check_fifo_co(struct s3c_fimc_control *ctrl)
{
	u32 cfg, status, flag;

	status = readl(ctrl->regs + S3C_CICOSTATUS);
	flag = S3C_CICOSTATUS_OVFIY | S3C_CICOSTATUS_OVFICB | S3C_CICOSTATUS_OVFICR;

	if (status & flag) {
		cfg = readl(ctrl->regs + S3C_CIWDOFST);
		cfg |= (S3C_CIWDOFST_CLROVCOFIY | S3C_CIWDOFST_CLROVCOFICB | \
			S3C_CIWDOFST_CLROVCOFICR);
		writel(cfg, ctrl->regs + S3C_CIWDOFST);

		cfg = readl(ctrl->regs + S3C_CIWDOFST);
		cfg &= ~(S3C_CIWDOFST_CLROVCOFIY | S3C_CIWDOFST_CLROVCOFICB | \
			S3C_CIWDOFST_CLROVCOFICR);
		writel(cfg, ctrl->regs + S3C_CIWDOFST);
	}

	return 0;
}

static int s3c_fimc_check_fifo_pr(struct s3c_fimc_control *ctrl)
{
	u32 cfg, status, flag;

	status = readl(ctrl->regs + S3C_CIPRSTATUS);
	flag = S3C_CIPRSTATUS_OVFIY | S3C_CIPRSTATUS_OVFICB | S3C_CIPRSTATUS_OVFICR;

	if (status & flag) {
		cfg = readl(ctrl->regs + S3C_CIWDOFST);
		cfg |= (S3C_CIWDOFST_CLROVPRFIY | S3C_CIWDOFST_CLROVPRFICB | \
			S3C_CIWDOFST_CLROVPRFICR);
		writel(cfg, ctrl->regs + S3C_CIWDOFST);

		cfg = readl(ctrl->regs + S3C_CIWDOFST);
		cfg &= ~(S3C_CIWDOFST_CLROVPRFIY | S3C_CIWDOFST_CLROVPRFICB | \
			S3C_CIWDOFST_CLROVPRFICR);
		writel(cfg, ctrl->regs + S3C_CIWDOFST);
	}

	return 0;
}

int s3c_fimc_check_fifo(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		return s3c_fimc_check_fifo_pr(ctrl);
	else
		return s3c_fimc_check_fifo_co(ctrl);
}

void s3c_fimc_select_camera(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIGCTRL);

	cfg &= ~S3C_CIGCTRL_TESTPATTERN_MASK;
	writel(cfg, ctrl->regs + S3C_CIGCTRL);
}

void s3c_fimc_set_test_pattern(struct s3c_fimc_control *ctrl, int type)
{
	u32 cfg = readl(ctrl->regs + S3C_CIGCTRL);

	cfg &= ~S3C_CIGCTRL_TESTPATTERN_MASK;
	cfg |= type << S3C_CIGCTRL_TESTPATTERN_SHIFT;

	writel(cfg, ctrl->regs + S3C_CIGCTRL);
}

void s3c_fimc_set_source_format(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_camera *cam = ctrl->in_cam;
	u32 cfg = 0;

	cfg |= (cam->mode | cam->order422);
	cfg |= S3C_CISRCFMT_SOURCEHSIZE(cam->width);
	cfg |= S3C_CISRCFMT_SOURCEVSIZE(cam->height);

	writel(cfg, ctrl->regs + S3C_CISRCFMT);
}

void s3c_fimc_set_window_offset(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_window_offset *offset = &ctrl->in_cam->offset;
	u32 cfg;
	
	cfg = readl(ctrl->regs + S3C_CIWDOFST);
	cfg &= ~(S3C_CIWDOFST_WINHOROFST_MASK | S3C_CIWDOFST_WINVEROFST_MASK);
	cfg |= S3C_CIWDOFST_WINHOROFST(offset->h1);
	cfg |= S3C_CIWDOFST_WINVEROFST(offset->v1);
	cfg |= S3C_CIWDOFST_WINOFSEN;
	writel(cfg, ctrl->regs + S3C_CIWDOFST);

	cfg = 0;
	cfg |= S3C_CIWDOFST2_WINHOROFST2(offset->h2);
	cfg |= S3C_CIWDOFST2_WINVEROFST2(offset->v2);
	writel(cfg, ctrl->regs + S3C_CIWDOFST2);
}

static void s3c_fimc_reset_cfg(struct s3c_fimc_control *ctrl)
{
	int i;
	u32 cfg[][2] = {
		{ 0x018, 0x00000000 }, { 0x01c, 0x00000000 },
		{ 0x020, 0x00000000 }, { 0x024, 0x00000000 },
		{ 0x028, 0x00000000 }, { 0x02c, 0x00000000 },
		{ 0x030, 0x00000000 }, { 0x034, 0x00000000 },
		{ 0x038, 0x00000000 }, { 0x03c, 0x00000000 },
		{ 0x040, 0x00000000 }, { 0x044, 0x00000000 },
		{ 0x048, 0x00000000 }, { 0x04c, 0x00000000 },
		{ 0x050, 0x00000000 }, { 0x054, 0x00000000 },
		{ 0x058, 0x18000000 }, { 0x05c, 0x00000000 },
		{ 0x068, 0x00000000 }, { 0x06c, 0x00000000 },
		{ 0x070, 0x00000000 }, { 0x074, 0x00000000 },
		{ 0x078, 0x18000000 }, { 0x07c, 0x00000000 },
		{ 0x080, 0x00000000 }, { 0x084, 0x00000000 },
		{ 0x088, 0x00000000 }, { 0x08c, 0x00000000 },
		{ 0x090, 0x00000000 }, { 0x094, 0x00000000 },
		{ 0x098, 0x18000000 }, { 0x0a0, 0x00000000 },
		{ 0x0a4, 0x00000000 }, { 0x0a8, 0x00000000 },
		{ 0x0ac, 0x18000000 }, { 0x0b0, 0x00000000 },			
		{ 0x0c0, 0x00000000 }, { 0x0c4, 0xffffffff },
		{ 0x0d0, 0x00100080 }, { 0x0d4, 0x00000000 },
		{ 0x0d8, 0x00000000 }, { 0x0e0, 0x00000000 },
		{ 0x0e4, 0x00000000 }, { 0x0e8, 0x00000000 },
		{ 0x0ec, 0x00000000 }, { 0x0f0, 0x00000000 },
		{ 0x0f4, 0x00000000 }, { 0x0f8, 0x00000000 },
		{ 0x0fc, 0x00000000 }, { 0x100, 0x00000000 },
		{ 0x104, 0x00000000 }, { 0x108, 0x00000000 },
		{ 0x10c, 0x00000000 }, { 0x110, 0x00000000 },
		{ 0x114, 0x00000000 }, { 0x118, 0x00000000 },
		{ 0x11c, 0x00000000 }, { 0x120, 0x00000000 },
		{ 0x124, 0x00000000 }, { 0x128, 0x00000000 },
		{ 0x12c, 0x00000000 }, { 0x130, 0x00000000 },
		{ 0x134, 0x00000000 }, { 0x138, 0x00000000 },
		{ 0x13c, 0x00000000 }, { 0x140, 0x00000000 },
	};

	for (i = 0; i < sizeof(cfg) / 8; i++)
		writel(cfg[i][1], ctrl->regs + cfg[i][0]);
}

void s3c_fimc_reset(struct s3c_fimc_control *ctrl)
{
	u32 cfg;

	/*
	 * we have to write 1 to the CISRCFMT[31] before
	 * getting started the sw reset
	 *
	 */
	cfg = readl(ctrl->regs + S3C_CISRCFMT);
	cfg |= S3C_CISRCFMT_ITU601_8BIT;
	writel(cfg, ctrl->regs + S3C_CISRCFMT);

	/* s/w reset */
	cfg = readl(ctrl->regs + S3C_CIGCTRL);
	cfg |= (S3C_CIGCTRL_SWRST | S3C_CIGCTRL_IRQ_LEVEL);
	writel(cfg, ctrl->regs + S3C_CIGCTRL);
	mdelay(1);

	cfg = readl(ctrl->regs + S3C_CIGCTRL);
	cfg &= ~S3C_CIGCTRL_SWRST;
	writel(cfg, ctrl->regs + S3C_CIGCTRL);

	/* in case of ITU656, CISRCFMT[31] should be 0 */
	if (ctrl->in_cam && ctrl->in_cam->mode == ITU_656_YCBCR422_8BIT) {
		cfg = readl(ctrl->regs + S3C_CISRCFMT);
		cfg &= ~S3C_CISRCFMT_ITU601_8BIT;
		writel(cfg, ctrl->regs + S3C_CISRCFMT);
	}

	s3c_fimc_reset_cfg(ctrl);
}

void s3c_fimc_reset_camera(void)
{
	void __iomem *regs = ioremap(S3C64XX_PA_FIMC, SZ_4K);
	u32 cfg;

#if (CONFIG_VIDEO_FIMC_CAM_RESET == 1)
	cfg = readl(regs + S3C_CIGCTRL);
	cfg |= S3C_CIGCTRL_CAMRST;
	writel(cfg, regs + S3C_CIGCTRL);
	udelay(200);

	cfg = readl(regs + S3C_CIGCTRL);
	cfg &= ~S3C_CIGCTRL_CAMRST;
	writel(cfg, regs + S3C_CIGCTRL);
	udelay(2000);
#else
	cfg = readl(regs + S3C_CIGCTRL);
	cfg &= ~S3C_CIGCTRL_CAMRST;
	writel(cfg, regs + S3C_CIGCTRL);
	udelay(200);

	cfg = readl(regs + S3C_CIGCTRL);
	cfg |= S3C_CIGCTRL_CAMRST;
	writel(cfg, regs + S3C_CIGCTRL);
	udelay(2000);
#endif

	iounmap(regs);
}

void s3c_fimc_set_polarity(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_camera *cam = ctrl->in_cam;
	u32 cfg;

	cfg = readl(ctrl->regs + S3C_CIGCTRL);

	cfg &= ~(S3C_CIGCTRL_INVPOLPCLK | S3C_CIGCTRL_INVPOLVSYNC | \
		 S3C_CIGCTRL_INVPOLHREF);

	if (cam->polarity.pclk)
		cfg |= S3C_CIGCTRL_INVPOLPCLK;

	if (cam->polarity.vsync)
		cfg |= S3C_CIGCTRL_INVPOLVSYNC;

	if (cam->polarity.href)
		cfg |= S3C_CIGCTRL_INVPOLHREF;

	writel(cfg, ctrl->regs + S3C_CIGCTRL);
}

static void s3c_fimc_set_rot90(struct s3c_fimc_control *ctrl)
{
	u32 cfg = 0;

	if (ctrl->id == 1) {
		cfg = readl(ctrl->regs + S3C_CIPRCTRL);
		cfg &= ~S3C_CIPRCTRL_BURST_MASK;
		cfg |= S3C_CIPRCTRL_YBURST1(4) | S3C_CIPRCTRL_YBURST2(4);
		writel(cfg, ctrl->regs + S3C_CIPRCTRL);

		cfg = readl(ctrl->regs + S3C_CIPRTRGFMT);
		cfg |= S3C_CIPRTRGFMT_ROT90_CLOCKWISE;
		writel(cfg, ctrl->regs + S3C_CIPRTRGFMT);
	}
}

static void s3c_fimc_set_target_format_pr(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	u32 cfg = 0;

	switch (frame->format) {
	case FORMAT_RGB565: /* fall through */
	case FORMAT_RGB666: /* fall through */
	case FORMAT_RGB888:
		cfg |= S3C_CIPRTRGFMT_OUTFORMAT_RGB;
		break;

	case FORMAT_YCBCR420:
		cfg |= S3C_CIPRTRGFMT_OUTFORMAT_YCBCR420;
		break;

	case FORMAT_YCBCR422:
		if (frame->planes == 1)
			cfg |= S3C_CIPRTRGFMT_OUTFORMAT_YCBCR422I;
		else
			cfg |= S3C_CIPRTRGFMT_OUTFORMAT_YCBCR422;

		break;
	}

	cfg |= S3C_CIPRTRGFMT_TARGETHSIZE(frame->width);
	cfg |= S3C_CIPRTRGFMT_TARGETVSIZE(frame->height);
	cfg |= (frame->flip << S3C_CIPRTRGFMT_FLIP_SHIFT);

	writel(cfg, ctrl->regs + S3C_CIPRTRGFMT);

	if (ctrl->rot90)
		s3c_fimc_set_rot90(ctrl);

	cfg = S3C_CIPRTAREA_TARGET_AREA(frame->width * frame->height);
	writel(cfg, ctrl->regs + S3C_CIPRTAREA);
}

static void s3c_fimc_set_target_format_co(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	u32 cfg = 0;

	switch (frame->format) {
	case FORMAT_RGB565: /* fall through */
	case FORMAT_RGB666: /* fall through */
	case FORMAT_RGB888:
		cfg |= S3C_CICOTRGFMT_OUTFORMAT_RGB;
		break;

	case FORMAT_YCBCR420:
		cfg |= S3C_CICOTRGFMT_OUTFORMAT_YCBCR420;
		break;

	case FORMAT_YCBCR422:
		if (frame->planes == 1)
			cfg |= S3C_CICOTRGFMT_OUTFORMAT_YCBCR422I;
		else
			cfg |= S3C_CICOTRGFMT_OUTFORMAT_YCBCR422;

		break;
	}

	cfg |= S3C_CICOTRGFMT_TARGETHSIZE(frame->width);
	cfg |= S3C_CICOTRGFMT_TARGETVSIZE(frame->height);
	cfg |= (frame->flip << S3C_CICOTRGFMT_FLIP_SHIFT);

	writel(cfg, ctrl->regs + S3C_CICOTRGFMT);

	cfg = S3C_CICOTAREA_TARGET_AREA(frame->width * frame->height);
	writel(cfg, ctrl->regs + S3C_CICOTAREA);
}

void s3c_fimc_set_target_format(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_set_target_format_pr(ctrl);
	else
		s3c_fimc_set_target_format_co(ctrl);
}

static void s3c_fimc_get_burst_422i(u32 width, u32 *mburst, u32 *rburst)
{
	unsigned int tmp, wanted;

	tmp = (width / 2) & 0xf;

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

	*mburst = wanted / 2;
	*rburst = wanted / 2;
}

static void s3c_fimc_get_burst(u32 width, u32 *mburst, u32 *rburst)
{
	unsigned int tmp;

	tmp = (width / 4) & 0xf;

	switch (tmp) {
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
		tmp = (width / 4) % 8;

		if (tmp == 0) {
			*mburst = 8;
			*rburst = 8;
		} else if (tmp == 4) {
			*mburst = 8;
			*rburst = 4;
		} else {
			tmp = (width / 4) % 4;
			*mburst = 4;
			*rburst = (tmp) ? tmp : 4;
		}

		break;
	}
}

static void s3c_fimc_set_output_dma_pr(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	// edit sangyub.kim
	u32 cfg, yburst_m, yburst_r, cburst_m, cburst_r;
	//u32 cfg, yburst_m, yburst_r;

	/* for output dma control */
	cfg = readl(ctrl->regs + S3C_CIPRCTRL);

	cfg &= ~(S3C_CIPRCTRL_BURST_MASK | S3C_CIPRCTRL_ORDER422_MASK);
	cfg |= frame->order_1p;

	if (ctrl->rot90) {
		yburst_m = 4;
		yburst_r = 4;
	} 
	#if 0
	else {
		if (frame->format == FORMAT_RGB888)
			s3c_fimc_get_burst(frame->width * 4, &yburst_m, &yburst_r);
		else
			s3c_fimc_get_burst(frame->width * 2, &yburst_m, &yburst_r);
	}

	cfg |= (S3C_CIPRCTRL_YBURST1(yburst_m) | S3C_CIPRCTRL_YBURST2(yburst_r));
	

	writel(cfg, ctrl->regs + S3C_CIPRCTRL);
	#endif
	else {
		if (frame->format == FORMAT_RGB888)
			s3c_fimc_get_burst(frame->width * 4, &yburst_m, &yburst_r);
		else if(frame->format == FORMAT_RGB565)
			s3c_fimc_get_burst(frame->width * 2, &yburst_m, &yburst_r);
		else if (frame->format == FORMAT_YCBCR422 && frame->planes == 1) {
			s3c_fimc_get_burst_422i(frame->width, &yburst_m, &yburst_r);
			cburst_m = yburst_m / 2;
			cburst_r = yburst_r / 2;
			cfg |= (S3C_CICOCTRL_CBURST1(cburst_m) | S3C_CICOCTRL_CBURST2(cburst_r));
		} 
		else if (frame->format == FORMAT_YCBCR420){
			s3c_fimc_get_burst(frame->width, &yburst_m, &yburst_r);
			s3c_fimc_get_burst(frame->width / 2, &cburst_m, &cburst_r);
			cfg |= (S3C_CICOCTRL_CBURST1(cburst_m) | S3C_CICOCTRL_CBURST2(cburst_r));
		}
		else
			printk("DEBUG : preview format is not supported\n");
	}
	cfg |= (S3C_CICOCTRL_YBURST1(yburst_m) | S3C_CICOCTRL_YBURST2(yburst_r));


	writel(cfg, ctrl->regs + S3C_CIPRCTRL);
}

static void s3c_fimc_set_output_dma_co(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	u32 cfg, yburst_m, yburst_r, cburst_m, cburst_r;

	/* for output dma control */
	cfg = readl(ctrl->regs + S3C_CICOCTRL);

	cfg &= ~(S3C_CICOCTRL_BURST_MASK | S3C_CICOCTRL_ORDER422_MASK);
	cfg |= frame->order_1p;
#if 1
	if (frame->format == FORMAT_YCBCR422 && frame->planes == 1) {
		s3c_fimc_get_burst_422i(frame->width, &yburst_m, &yburst_r);
		cburst_m = yburst_m / 2;
		cburst_r = yburst_r / 2;
	} else {
		s3c_fimc_get_burst(frame->width, &yburst_m, &yburst_r);
		s3c_fimc_get_burst(frame->width / 2, &cburst_m, &cburst_r);
	}
#else
	if (frame->format == FORMAT_RGB888)
		s3c_fimc_get_burst(frame->width * 4, &yburst_m, &yburst_r);
	else if(frame->format == FORMAT_RGB565)
		s3c_fimc_get_burst(frame->width * 2, &yburst_m, &yburst_r);
	else if (frame->format == FORMAT_YCBCR422 && frame->planes == 1) {
		s3c_fimc_get_burst_422i(frame->width, &yburst_m, &yburst_r);
		cburst_m = yburst_m / 2;
		cburst_r = yburst_r / 2;
	} 
	else if (frame->format == FORMAT_YCBCR420){
		s3c_fimc_get_burst(frame->width, &yburst_m, &yburst_r);
		s3c_fimc_get_burst(frame->width / 2, &cburst_m, &cburst_r);
		printk("DEBUG : %s FORMAT_YCBCR420 yburst_m(%d), yburst_r(%d)\n",__FUNCTION__, yburst_m, yburst_r);
		printk("DEBUG : %s FORMAT_YCBCR420 cburst_m(%d), cburst_r(%d)\n",__FUNCTION__, cburst_m, cburst_r);
	}
	else
		printk("DEBUG : codec format is not supported\n");
#endif

	cfg |= (S3C_CICOCTRL_YBURST1(yburst_m) | S3C_CICOCTRL_YBURST2(yburst_r));
	cfg |= (S3C_CICOCTRL_CBURST1(cburst_m) | S3C_CICOCTRL_CBURST2(cburst_r));

	writel(cfg, ctrl->regs + S3C_CICOCTRL);
}

void s3c_fimc_set_output_dma(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_set_output_dma_pr(ctrl);
	else
		s3c_fimc_set_output_dma_co(ctrl);
}

static void s3c_fimc_enable_lastirq_pr(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIPRCTRL);

	cfg |= S3C_CIPRCTRL_LASTIRQ_ENABLE;
	writel(cfg, ctrl->regs + S3C_CIPRCTRL);
}

static void s3c_fimc_enable_lastirq_co(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CICOCTRL);

	cfg |= S3C_CICOCTRL_LASTIRQ_ENABLE;
	writel(cfg, ctrl->regs + S3C_CICOCTRL);
}

void s3c_fimc_enable_lastirq(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_enable_lastirq_pr(ctrl);
	else
		s3c_fimc_enable_lastirq_co(ctrl);
}

static void s3c_fimc_disable_lastirq_pr(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIPRCTRL);

	cfg &= ~S3C_CIPRCTRL_LASTIRQ_ENABLE;
	writel(cfg, ctrl->regs + S3C_CIPRCTRL);
}

static void s3c_fimc_disable_lastirq_co(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CICOCTRL);

	cfg &= ~S3C_CICOCTRL_LASTIRQ_ENABLE;
	writel(cfg, ctrl->regs + S3C_CICOCTRL);
}

void s3c_fimc_disable_lastirq(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_disable_lastirq_pr(ctrl);
	else
		s3c_fimc_disable_lastirq_co(ctrl);
}

static void s3c_fimc_set_prescaler_pr(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_scaler *sc = &ctrl->scaler;
	u32 cfg = 0, shfactor;

	shfactor = 10 - (sc->hfactor + sc->vfactor);

	cfg |= S3C_CIPRSCPRERATIO_SHFACTOR(shfactor);
	cfg |= S3C_CIPRSCPRERATIO_PREHORRATIO(sc->pre_hratio);
	cfg |= S3C_CIPRSCPRERATIO_PREVERRATIO(sc->pre_vratio);

	writel(cfg, ctrl->regs + S3C_CIPRSCPRERATIO);

	cfg = 0;
	cfg |= S3C_CIPRSCPREDST_PREDSTWIDTH(sc->pre_dst_width);
	cfg |= S3C_CIPRSCPREDST_PREDSTHEIGHT(sc->pre_dst_height);

	writel(cfg, ctrl->regs + S3C_CIPRSCPREDST);
}

static void s3c_fimc_set_prescaler_co(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_scaler *sc = &ctrl->scaler;
	u32 cfg = 0, shfactor;

	shfactor = 10 - (sc->hfactor + sc->vfactor);

	cfg |= S3C_CICOSCPRERATIO_SHFACTOR(shfactor);
	cfg |= S3C_CICOSCPRERATIO_PREHORRATIO(sc->pre_hratio);
	cfg |= S3C_CICOSCPRERATIO_PREVERRATIO(sc->pre_vratio);

	writel(cfg, ctrl->regs + S3C_CICOSCPRERATIO);

	cfg = 0;
	cfg |= S3C_CICOSCPREDST_PREDSTWIDTH(sc->pre_dst_width);
	cfg |= S3C_CICOSCPREDST_PREDSTHEIGHT(sc->pre_dst_height);

	writel(cfg, ctrl->regs + S3C_CICOSCPREDST);
}

void s3c_fimc_set_prescaler(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_set_prescaler_pr(ctrl);
	else
		s3c_fimc_set_prescaler_co(ctrl);
}

static void s3c_fimc_set_scaler_pr(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_scaler *sc = &ctrl->scaler;
	u32 cfg = (S3C_CIPRSCCTRL_CSCR2Y_WIDE | S3C_CIPRSCCTRL_CSCY2R_WIDE);

	if (sc->bypass)
		cfg |= S3C_CIPRSCCTRL_SCALERBYPASS;

	if (sc->scaleup_h)
		cfg |= S3C_CIPRSCCTRL_SCALEUP_H;

	if (sc->scaleup_v)
		cfg |= S3C_CIPRSCCTRL_SCALEUP_V;

	if (ctrl->in_type == PATH_IN_DMA) {
		if (ctrl->in_frame.format == FORMAT_RGB565)
			cfg |= S3C_CIPRSCCTRL_INRGB_FMT_RGB565;
		else if (ctrl->in_frame.format == FORMAT_RGB666)
			cfg |= S3C_CIPRSCCTRL_INRGB_FMT_RGB666;
		else if (ctrl->in_frame.format == FORMAT_RGB888)
			cfg |= S3C_CIPRSCCTRL_INRGB_FMT_RGB888;
	}

	if (ctrl->out_type == PATH_OUT_DMA) {
		if (ctrl->out_frame.format == FORMAT_RGB565)
			cfg |= S3C_CIPRSCCTRL_OUTRGB_FMT_RGB565;
		else if (ctrl->out_frame.format == FORMAT_RGB666)
			cfg |= S3C_CIPRSCCTRL_OUTRGB_FMT_RGB666;
		else if (ctrl->out_frame.format == FORMAT_RGB888)
			cfg |= S3C_CIPRSCCTRL_OUTRGB_FMT_RGB888;
	} else {
		cfg |= S3C_CIPRSCCTRL_OUTRGB_FMT_RGB888;

		if (ctrl->out_frame.scan == SCAN_TYPE_INTERLACE)
			cfg |= S3C_CIPRSCCTRL_INTERLACE;
		else
			cfg |= S3C_CIPRSCCTRL_PROGRESSIVE;
	}

	cfg |= S3C_CIPRSCCTRL_MAINHORRATIO(sc->main_hratio);
	cfg |= S3C_CIPRSCCTRL_MAINVERRATIO(sc->main_vratio);

	writel(cfg, ctrl->regs + S3C_CIPRSCCTRL);
}

static void s3c_fimc_set_scaler_co(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_scaler *sc = &ctrl->scaler;
	u32 cfg = (S3C_CICOSCCTRL_CSCR2Y_WIDE | S3C_CICOSCCTRL_CSCY2R_WIDE);

	if (sc->bypass)
		cfg |= S3C_CICOSCCTRL_SCALERBYPASS;

	if (sc->scaleup_h)
		cfg |= S3C_CICOSCCTRL_SCALEUP_H;

	if (sc->scaleup_v)
		cfg |= S3C_CICOSCCTRL_SCALEUP_V;

	if (ctrl->in_type == PATH_IN_DMA) {
		if (ctrl->in_frame.format == FORMAT_RGB565)
			cfg |= S3C_CICOSCCTRL_INRGB_FMT_RGB565;
		else if (ctrl->in_frame.format == FORMAT_RGB666)
			cfg |= S3C_CICOSCCTRL_INRGB_FMT_RGB666;
		else if (ctrl->in_frame.format == FORMAT_RGB888)
			cfg |= S3C_CICOSCCTRL_INRGB_FMT_RGB888;
	}

	if (ctrl->out_type == PATH_OUT_DMA) {
		if (ctrl->out_frame.format == FORMAT_RGB565)
			cfg |= S3C_CICOSCCTRL_OUTRGB_FMT_RGB565;
		else if (ctrl->out_frame.format == FORMAT_RGB666)
			cfg |= S3C_CICOSCCTRL_OUTRGB_FMT_RGB666;
		else if (ctrl->out_frame.format == FORMAT_RGB888)
			cfg |= S3C_CICOSCCTRL_OUTRGB_FMT_RGB888;
	} else {
		cfg |= S3C_CICOSCCTRL_OUTRGB_FMT_RGB888;

		if (ctrl->out_frame.scan == SCAN_TYPE_INTERLACE)
			cfg |= S3C_CICOSCCTRL_INTERLACE;
		else
			cfg |= S3C_CICOSCCTRL_PROGRESSIVE;
	}

	cfg |= S3C_CICOSCCTRL_MAINHORRATIO(sc->main_hratio);
	cfg |= S3C_CICOSCCTRL_MAINVERRATIO(sc->main_vratio);

	writel(cfg, ctrl->regs + S3C_CICOSCCTRL);
}

void s3c_fimc_set_scaler(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_set_scaler_pr(ctrl);
	else
		s3c_fimc_set_scaler_co(ctrl);
}

static void s3c_fimc_start_scaler_pr(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIPRSCCTRL);

	cfg |= S3C_CIPRSCCTRL_SCALERSTART;
	writel(cfg, ctrl->regs + S3C_CIPRSCCTRL);

	if (ctrl->out_type == PATH_OUT_LCDFIFO)
		ctrl->open_lcdfifo(1, 0, S3C_WINCON1_LOCALSEL_CAMERA);
}

static void s3c_fimc_start_scaler_co(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CICOSCCTRL);

	cfg |= S3C_CICOSCCTRL_SCALERSTART;
	writel(cfg, ctrl->regs + S3C_CICOSCCTRL);

	if (ctrl->out_type == PATH_OUT_LCDFIFO)
		ctrl->open_lcdfifo(2, 1, S3C_WINCON2_LOCALSEL_CAMERA);
}

void s3c_fimc_start_scaler(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_start_scaler_pr(ctrl);
	else
		s3c_fimc_start_scaler_co(ctrl);
}

static void s3c_fimc_stop_scaler_pr(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIPRSCCTRL);

	if (ctrl->out_type == PATH_OUT_LCDFIFO)
		ctrl->close_lcdfifo(ctrl->id);

	cfg &= ~S3C_CIPRSCCTRL_SCALERSTART;
	writel(cfg, ctrl->regs + S3C_CIPRSCCTRL);
}

static void s3c_fimc_stop_scaler_co(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CICOSCCTRL);

	if (ctrl->out_type == PATH_OUT_LCDFIFO)
		ctrl->close_lcdfifo(ctrl->id);

	cfg &= ~S3C_CICOSCCTRL_SCALERSTART;
	writel(cfg, ctrl->regs + S3C_CICOSCCTRL);
}

void s3c_fimc_stop_scaler(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_stop_scaler_pr(ctrl);
	else
		s3c_fimc_stop_scaler_co(ctrl);
}


void s3c_fimc_enable_capture(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIIMGCPT);

	if (ctrl->id == 1) {
		cfg &= ~S3C_CIIMGCPT_CPT_FREN_ENABLE_PR;
		cfg |= S3C_CIIMGCPT_IMGCPTEN;

		if (!ctrl->scaler.bypass)
			cfg |= S3C_CIIMGCPT_IMGCPTEN_PRSC;
	} else {
		cfg &= ~S3C_CIIMGCPT_CPT_FREN_ENABLE_CO;
		cfg |= S3C_CIIMGCPT_IMGCPTEN;

		if (!ctrl->scaler.bypass)
			cfg |= S3C_CIIMGCPT_IMGCPTEN_COSC;
	}

	writel(cfg, ctrl->regs + S3C_CIIMGCPT);
}

void s3c_fimc_disable_capture(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIIMGCPT);

	if (ctrl->id == 1)
		cfg &= ~S3C_CIIMGCPT_IMGCPTEN_PRSC;
	else
		cfg &= ~S3C_CIIMGCPT_IMGCPTEN_COSC;

	if (!(cfg & (S3C_CIIMGCPT_IMGCPTEN_PRSC | S3C_CIIMGCPT_IMGCPTEN_COSC)))
		cfg &= ~S3C_CIIMGCPT_IMGCPTEN;

	writel(cfg, ctrl->regs + S3C_CIIMGCPT);
}

void s3c_fimc_set_effect(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_effect *effect = &ctrl->out_frame.effect;
	u32 cfg = S3C_CIIMGEFF_IE_SC_AFTER;

	if (ctrl->id == 1)
		cfg |= S3C_CIIMGEFF_IE_ENABLE_PR;
	else
		cfg |= S3C_CIIMGEFF_IE_ENABLE_CO;

	cfg |= effect->type;

	if (effect->type == EFFECT_ARBITRARY) {
		cfg |= S3C_CIIMGEFF_PAT_CB(effect->pat_cb);
		cfg |= S3C_CIIMGEFF_PAT_CR(effect->pat_cr);
	}

	writel(cfg, ctrl->regs + S3C_CIIMGEFF);
}

static void s3c_fimc_set_input_dma_size_pr(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_in_frame *frame = &ctrl->in_frame;
	int ofs_h = frame->offset.y_h * 2;
	int ofs_v = frame->offset.y_v * 2;
	u32 cfg = S3C_MSPRWIDTH_AUTOLOAD_ENABLE;

	cfg |= S3C_MSPR_WIDTH(frame->width - ofs_h);
	cfg |= S3C_MSPR_HEIGHT(frame->height - ofs_v);

	writel(cfg, ctrl->regs + S3C_MSPRWIDTH);
}

static void s3c_fimc_set_input_dma_pr(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_in_frame *frame = &ctrl->in_frame;
	u32 cfg;

	/* for original & real size */
	s3c_fimc_set_input_dma_size_pr(ctrl);

	/* for input dma control */
	cfg = S3C_MSPRCTRL_INPUT_MEMORY;

	switch (frame->format) {
	case FORMAT_RGB565: /* fall through */
	case FORMAT_RGB666: /* fall through */
	case FORMAT_RGB888:
		cfg |= S3C_MSPRCTRL_INFORMAT_RGB;
		break;

	case FORMAT_YCBCR420:
		cfg |= S3C_MSPRCTRL_INFORMAT_YCBCR420;
		break;

	case FORMAT_YCBCR422:
		if (frame->planes == 1) {
			cfg |= S3C_MSPRCTRL_INFORMAT_YCBCR422I;
			cfg |= frame->order_1p;
		} else
			cfg |= S3C_MSPRCTRL_INFORMAT_YCBCR422;

		break;
	}

	writel(cfg, ctrl->regs + S3C_MSPRCTRL);
}

static void s3c_fimc_set_input_dma_size_co(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_in_frame *frame = &ctrl->in_frame;
	int ofs_h = frame->offset.y_h * 2;
	int ofs_v = frame->offset.y_v * 2;
	u32 cfg = S3C_MSCOWIDTH_AUTOLOAD_ENABLE;

	cfg |= S3C_MSCO_WIDTH(frame->width - ofs_h);
	cfg |= S3C_MSCO_HEIGHT(frame->height - ofs_v);

	writel(cfg, ctrl->regs + S3C_MSCOWIDTH);
}

static void s3c_fimc_set_input_dma_co(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_in_frame *frame = &ctrl->in_frame;
	u32 cfg;

	/* for original & real size */
	s3c_fimc_set_input_dma_size_co(ctrl);

	/* for input dma control */
	cfg = S3C_MSCOCTRL_INPUT_MEMORY;

	switch (frame->format) {
	case FORMAT_RGB565: /* fall through */
	case FORMAT_RGB666: /* fall through */
	case FORMAT_RGB888:
		cfg |= S3C_MSCOCTRL_INFORMAT_RGB;
		break;

	case FORMAT_YCBCR420:
		cfg |= S3C_MSCOCTRL_INFORMAT_YCBCR420;
		break;

	case FORMAT_YCBCR422:
		if (frame->planes == 1)
			cfg |= S3C_MSCOCTRL_INFORMAT_YCBCR422I;
		else
			cfg |= S3C_MSCOCTRL_INFORMAT_YCBCR422;

		break;
	}

	writel(cfg, ctrl->regs + S3C_MSCOCTRL);
}

void s3c_fimc_set_input_dma(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_set_input_dma_pr(ctrl);
	else
		s3c_fimc_set_input_dma_co(ctrl);
}

static void s3c_fimc_start_input_dma_pr(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_MSPRCTRL);

	cfg |= S3C_MSPRCTRL_ENVID;
	writel(cfg, ctrl->regs + S3C_MSPRCTRL);
}

static void s3c_fimc_start_input_dma_co(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_MSCOCTRL);

	cfg |= S3C_MSCOCTRL_ENVID;
	writel(cfg, ctrl->regs + S3C_MSCOCTRL);
}

void s3c_fimc_start_input_dma(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_start_input_dma_pr(ctrl);
	else
		s3c_fimc_start_input_dma_co(ctrl);
}

static void s3c_fimc_stop_input_dma_pr(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_MSPRCTRL);

	cfg &= ~S3C_MSPRCTRL_ENVID;
	writel(cfg, ctrl->regs + S3C_MSPRCTRL);
}

static void s3c_fimc_stop_input_dma_co(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_MSCOCTRL);

	cfg &= ~S3C_MSCOCTRL_ENVID;
	writel(cfg, ctrl->regs + S3C_MSCOCTRL);
}

void s3c_fimc_stop_input_dma(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_stop_input_dma_pr(ctrl);
	else
		s3c_fimc_stop_input_dma_co(ctrl);
}

static void s3c_fimc_set_input_path_pr(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_MSPRCTRL);

	cfg &= ~S3C_MSPRCTRL_INPUT_MASK;

	if (ctrl->in_type == PATH_IN_DMA)
		cfg |= S3C_MSPRCTRL_INPUT_MEMORY;
	else
		cfg |= S3C_MSPRCTRL_INPUT_EXTCAM;

	writel(cfg, ctrl->regs + S3C_MSPRCTRL);
}

static void s3c_fimc_set_input_path_co(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_MSCOCTRL);

	cfg &= ~S3C_MSCOCTRL_INPUT_MASK;

	if (ctrl->in_type == PATH_IN_DMA)
		cfg |= S3C_MSCOCTRL_INPUT_MEMORY;
	else
		cfg |= S3C_MSCOCTRL_INPUT_EXTCAM;

	writel(cfg, ctrl->regs + S3C_MSCOCTRL);
}

void s3c_fimc_set_input_path(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_set_input_path_pr(ctrl);
	else
		s3c_fimc_set_input_path_co(ctrl);
}

static void s3c_fimc_set_output_path_pr(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIPRSCCTRL);

	cfg &= ~S3C_CIPRSCCTRL_LCDPATHEN_FIFO;

	if (ctrl->out_type == PATH_OUT_LCDFIFO)
		cfg |= S3C_CIPRSCCTRL_LCDPATHEN_FIFO;

	writel(cfg, ctrl->regs + S3C_CIPRSCCTRL);
}

static void s3c_fimc_set_output_path_co(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CICOSCCTRL);

	cfg &= ~S3C_CICOSCCTRL_LCDPATHEN_FIFO;

	if (ctrl->out_type == PATH_OUT_LCDFIFO)
		cfg |= S3C_CICOSCCTRL_LCDPATHEN_FIFO;

	writel(cfg, ctrl->regs + S3C_CICOSCCTRL);
}

void s3c_fimc_set_output_path(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_set_output_path_pr(ctrl);
	else
		s3c_fimc_set_output_path_co(ctrl);
}

void s3c_fimc_set_input_address(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_in_frame *frame = &ctrl->in_frame;
	struct s3c_fimc_frame_addr *addr = &ctrl->in_frame.addr;
	u32 width = frame->width;
	u32 height = frame->height;
	dma_addr_t start_y = addr->phys_y, start_cb = 0, start_cr = 0;
	dma_addr_t end_y = 0, end_cb = 0, end_cr = 0;

	if (frame->planes == 1)
		end_y = start_y + frame->buf_size;
	else {
		start_cb = addr->phys_cb;
		start_cr = addr->phys_cr;

		if (frame->format == FORMAT_YCBCR420) {
			end_y = start_y + (width * height);
			end_cb = start_cb + (width * height / 4);
			end_cr = start_cr + (width * height / 4);
		} else {
			end_y = start_y + (width * height);
			end_cb = start_cb + (width * height / 2);
			end_cr = start_cr + (width * height / 2);
		}
	}

	addr->phys_y = start_y;
	addr->phys_cb = start_cb;
	addr->phys_cr = start_cr;

	if (ctrl->id == 1) {
		writel(start_y, ctrl->regs + S3C_MSPRY0SA);
		writel(start_cb, ctrl->regs + S3C_MSPRCB0SA);
		writel(start_cr, ctrl->regs + S3C_MSPRCR0SA);
		writel(end_y, ctrl->regs + S3C_MSPRY0END);
		writel(end_cb, ctrl->regs + S3C_MSPRCB0END);
		writel(end_cr, ctrl->regs + S3C_MSPRCR0END);
	} else {
		writel(start_y, ctrl->regs + S3C_MSCOY0SA);
		writel(start_cb, ctrl->regs + S3C_MSCOCB0SA);
		writel(start_cr, ctrl->regs + S3C_MSCOCR0SA);
		writel(end_y, ctrl->regs + S3C_MSCOY0END);
		writel(end_cb, ctrl->regs + S3C_MSCOCB0END);
		writel(end_cr, ctrl->regs + S3C_MSCOCR0END);
	}
}

static void s3c_fimc_set_output_address_pr(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	struct s3c_fimc_frame_addr *addr;
	int i;

	switch (frame->format) {
	case FORMAT_RGB565: /* fall through */
	case FORMAT_RGB666: /* fall through */
	case FORMAT_RGB888:
		printk("DEBUG : %s Format is FORMAT_RGB\n", __FUNCTION__);
		for (i = 0; i < S3C_FIMC_MAX_FRAMES; i++) {
		addr = &frame->addr[i];
		writel(addr->phys_rgb, ctrl->regs + S3C_CIPRYSA(i));
		writel(0, ctrl->regs + S3C_CIPRCBSA(i));
		writel(0, ctrl->regs + S3C_CIPRCRSA(i));
		}
		break;

	case FORMAT_YCBCR420:  /* fall through */
	case FORMAT_YCBCR422:
		printk("DEBUG :  %s Format is FORMAT_YCBCR\n", __FUNCTION__);
		for (i = 0; i < S3C_FIMC_MAX_FRAMES; i++) {
		addr = &frame->addr[i];
		writel(addr->phys_y, ctrl->regs + S3C_CIPRYSA(i));
		writel(addr->phys_cb, ctrl->regs + S3C_CIPRCBSA(i));
		writel(addr->phys_cr, ctrl->regs + S3C_CIPRCRSA(i));
		}
		break;
	}


	#if 0
	for (i = 0; i < S3C_FIMC_MAX_FRAMES; i++) {
		addr = &frame->addr[i];
		writel(addr->phys_rgb, ctrl->regs + S3C_CIPRYSA(i));
		writel(0, ctrl->regs + S3C_CIPRCBSA(i));
		writel(0, ctrl->regs + S3C_CIPRCRSA(i));
	}

	for (i = 0; i < S3C_FIMC_MAX_FRAMES; i++) {
	addr = &frame->addr[i];
	writel(addr->phys_y, ctrl->regs + S3C_CIPRYSA(i));
	writel(addr->phys_cb, ctrl->regs + S3C_CIPRCBSA(i));
	writel(addr->phys_cr, ctrl->regs + S3C_CIPRCRSA(i));
	}
	#endif
}

static void s3c_fimc_set_output_address_co(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	struct s3c_fimc_frame_addr *addr;
	int i;

	for (i = 0; i < S3C_FIMC_MAX_FRAMES; i++) {
		addr = &frame->addr[i];
		writel(addr->phys_y, ctrl->regs + S3C_CICOYSA(i));
		writel(addr->phys_cb, ctrl->regs + S3C_CICOCBSA(i));
		writel(addr->phys_cr, ctrl->regs + S3C_CICOCRSA(i));
	}
}

void s3c_fimc_set_output_address(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1)
		s3c_fimc_set_output_address_pr(ctrl);
	else
		s3c_fimc_set_output_address_co(ctrl);
}

int s3c_fimc_get_frame_count(struct s3c_fimc_control *ctrl)
{
	if (ctrl->id == 1) {
		return S3C_CIPRSTATUS_GET_FRAME_COUNT( \
			readl(ctrl->regs + S3C_CIPRSTATUS));
	} else {
		return S3C_CICOSTATUS_GET_FRAME_COUNT( \
			readl(ctrl->regs + S3C_CICOSTATUS));
	}
}

// add sangyub.kim
void s3c_fimc_wait_frame_end(struct s3c_fimc_control *ctrl)
{
        unsigned long timeo = jiffies;
	unsigned int frame_cnt = 0;
	u32 cfg;

        timeo += 20;    /* waiting for 100mS */

	if (ctrl->id == 1) {
		while (time_before(jiffies, timeo)) {
			cfg = readl(ctrl->regs + S3C_CIPRSTATUS);
			
			if (S3C_CIPRSTATUS_GET_FRAME_END(cfg)) {
				cfg &= ~S3C_CIPRSTATUS_FRAMEEND;
				writel(cfg, ctrl->regs + S3C_CIPRSTATUS);

				if (frame_cnt == 2)
					break;
				else
					frame_cnt++;
			}
			cond_resched();
		}
	} else {
		while (time_before(jiffies, timeo)) {
			cfg = readl(ctrl->regs + S3C_CICOSTATUS);

			if (S3C_CICOSTATUS_GET_FRAME_END(cfg)) {
				cfg &= ~S3C_CICOSTATUS_FRAMEEND;
				writel(cfg, ctrl->regs + S3C_CICOSTATUS);
				
				if (frame_cnt == 2)
					break;
				else
					frame_cnt++;
			}
			cond_resched();
		}
	}
}

void s3c_fimc_change_effect(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_effect *effect = &ctrl->out_frame.effect;
	u32 cfg = readl(ctrl->regs + S3C_CIIMGEFF);

	cfg &= ~S3C_CIIMGEFF_FIN_MASK;
	cfg |= effect->type;

	if (ctrl->id == 1)
		cfg |= S3C_CIIMGEFF_IE_ENABLE_PR;
	else
		cfg |= S3C_CIIMGEFF_IE_ENABLE_CO;

	if (effect->type == EFFECT_ARBITRARY) {
		cfg &= ~S3C_CIIMGEFF_PAT_CBCR_MASK;
		cfg |= S3C_CIIMGEFF_PAT_CB(effect->pat_cb);
		cfg |= S3C_CIIMGEFF_PAT_CR(effect->pat_cr);
	}

	writel(cfg, ctrl->regs + S3C_CIIMGEFF);
}

void s3c_fimc_change_rotate(struct s3c_fimc_control *ctrl)
{
	u32 cfg;

	if (ctrl->rot90)
		s3c_fimc_set_rot90(ctrl);

	if (ctrl->out_type == PATH_OUT_DMA) {
		cfg = readl(ctrl->regs + S3C_CIPRTRGFMT);
		cfg &= ~S3C_CIPRTRGFMT_FLIP_MASK;
		cfg |= (ctrl->out_frame.flip << S3C_CIPRTRGFMT_FLIP_SHIFT);

		writel(cfg, ctrl->regs + S3C_CIPRTRGFMT);
	}
}
