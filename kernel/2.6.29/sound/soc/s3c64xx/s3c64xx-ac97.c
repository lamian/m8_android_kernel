/*
 * s3c6400-ac97.c  --  ALSA Soc Audio Layer
 *
 *  Copyright (C) 2007, Ryu Euiyoul <ryu.real@gmail.com>
 *
 * (c) 2007 Wolfson Microelectronics PLC.
 * Graeme Gregory graeme.gregory@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  Copyright (C) 2007, Ryu Euiyoul <ryu.real@gmail.com>
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  Revision history
 *	21st Mar 2007   Initial Version
 *	20th Sep 2007   Apply at s3c6400
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/clk.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/ac97_codec.h>
#include <sound/initval.h>
#include <sound/soc.h>

#include <mach/hardware.h>
#include <asm/io.h>
#include <plat/regs-ac97.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-bank-h.h>
#include <plat/gpio-bank-d.h>
#include <plat/regs-clock.h>
#include <mach/audio.h>
#include <mach/dma.h>
#include <asm/dma.h>

#include "s3c-pcm.h"
#include "s3c64xx-ac97.h"

#ifdef CONFIG_SND_DEBUG
#define s3cdbg(x...) printk(x)
#else
#define s3cdbg(x...)
#endif

extern struct clk *clk_get(struct device *dev, const char *id);
extern int clk_enable(struct clk *clk);
extern void clk_disable(struct clk *clk);

struct s3c24xx_ac97_info {
	void __iomem	*regs;
	struct clk	*ac97_clk;
};
static struct s3c24xx_ac97_info s3c24xx_ac97;

static u32 codec_ready;
static DEFINE_MUTEX(ac97_mutex);
static DECLARE_WAIT_QUEUE_HEAD(gsr_wq);

static unsigned short s3c6400_ac97_read(struct snd_ac97 *ac97,
	unsigned short reg)
{
	u32 ac_glbctrl;
	u32 ac_codec_cmd;
	u32 stat, addr, data;

	s3cdbg("Entered %s: reg=0x%x\n", __FUNCTION__, reg);

	mutex_lock(&ac97_mutex);

	codec_ready = S3C_AC97_GLBSTAT_CODECREADY;
	ac_codec_cmd = S3C_AC97_CODEC_CMD_READ | AC_CMD_ADDR(reg);
	writel(ac_codec_cmd, s3c24xx_ac97.regs + S3C_AC97_CODEC_CMD);

	udelay(1000);

	ac_glbctrl = readl(s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	ac_glbctrl |= S3C_AC97_GLBCTRL_CODECREADYIE;
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);

	stat = readl(s3c24xx_ac97.regs + S3C_AC97_STAT);
	addr = (stat >> 16) & 0x7f;
	data = (stat & 0xffff);

	wait_event_timeout(gsr_wq,addr==reg,1);
	if(addr!=reg){
		printk(KERN_ERR"AC97: read error (ac97_reg=%x addr=%x)\n", reg, addr);
		printk(KERN_ERR"Check audio codec jumpper settings\n\n");
		goto out;
	}

out:	mutex_unlock(&ac97_mutex);
	return (unsigned short)data;
}

static void s3c6400_ac97_write(struct snd_ac97 *ac97, unsigned short reg,
	unsigned short val)
{
	u32 ac_glbctrl;
	u32 ac_codec_cmd;
	u32 stat, data;

	s3cdbg("Entered %s: reg=0x%x, val=0x%x\n", __FUNCTION__,reg,val);

	mutex_lock(&ac97_mutex);

	codec_ready = S3C_AC97_GLBSTAT_CODECREADY;
	ac_codec_cmd = AC_CMD_ADDR(reg) | AC_CMD_DATA(val);
	writel(ac_codec_cmd, s3c24xx_ac97.regs + S3C_AC97_CODEC_CMD);

	udelay(50);

	ac_glbctrl = readl(s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	ac_glbctrl |= S3C_AC97_GLBCTRL_CODECREADYIE;
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);

	ac_codec_cmd |= S3C_AC97_CODEC_CMD_READ;
	writel(ac_codec_cmd, s3c24xx_ac97.regs + S3C_AC97_CODEC_CMD);

	stat = readl(s3c24xx_ac97.regs + S3C_AC97_CODEC_CMD);
	data = (stat & 0xffff);

	wait_event_timeout(gsr_wq,data==val,1);
	if(data!=val){
		printk("%s: write error (ac97_val=%x data=%x)\n",
				__FUNCTION__, val, data);
	}

	mutex_unlock(&ac97_mutex);
}

static void s3c6400_ac97_warm_reset(struct snd_ac97 *ac97)
{
	u32 ac_glbctrl;

	s3cdbg("Entered %s\n", __FUNCTION__);

	ac_glbctrl = readl(s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	ac_glbctrl |= S3C_AC97_GLBCTRL_WARMRESET;
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	msleep(1);

	ac_glbctrl &= ~S3C_AC97_GLBCTRL_WARMRESET;
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	msleep(1);
	
	ac_glbctrl = S3C_AC97_GLBCTRL_ACLINKON;
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	msleep(1);

	ac_glbctrl = S3C_AC97_GLBCTRL_TRANSFERDATAENABLE;
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	msleep(1);

	ac_glbctrl |= S3C_AC97_GLBCTRL_PCMOUTTM_DMA |
		S3C_AC97_GLBCTRL_PCMINTM_DMA | S3C_AC97_GLBCTRL_MICINTM_DMA;
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);

	ac_glbctrl = readl(s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	ac_glbctrl |= S3C_AC97_GLBCTRL_ACLINKON;
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	udelay(1000);
}

static void s3c6400_ac97_cold_reset(struct snd_ac97 *ac97)
{
	u32 ac_glbctrl;

	s3cdbg("Entered %s\n", __FUNCTION__);

	ac_glbctrl = S3C_AC97_GLBCTRL_COLDRESET;
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	msleep(1);

	ac_glbctrl &= ~S3C_AC97_GLBCTRL_COLDRESET;
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	msleep(1);

	ac_glbctrl = S3C_AC97_GLBCTRL_COLDRESET;
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	msleep(1);

	ac_glbctrl &= ~S3C_AC97_GLBCTRL_COLDRESET;
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	msleep(1);
}

static irqreturn_t s3c6400_ac97_irq(int irq, void *dev_id)
{
	int status;
	u32 ac_glbctrl, ac_glbstat;

	ac_glbstat = readl(s3c24xx_ac97.regs + S3C_AC97_GLBSTAT);

	s3cdbg("Entered %s: AC_GLBSTAT = 0x%x\n", __FUNCTION__, ac_glbstat);

	status = ac_glbstat & codec_ready;


	if (status) {
		ac_glbctrl = readl(s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
		ac_glbctrl &= ~S3C_AC97_GLBCTRL_CODECREADYIE;
		writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
		wake_up(&gsr_wq);
	}
	return IRQ_HANDLED;
}

struct snd_ac97_bus_ops soc_ac97_ops = {
	.read	= s3c6400_ac97_read,
	.write	= s3c6400_ac97_write,
	.warm_reset	= s3c6400_ac97_warm_reset,
	.reset	= s3c6400_ac97_cold_reset,
};

static struct s3c2410_dma_client s3c6400_dma_client_out = {
	.name = "AC97 PCM Stereo out"
};

static struct s3c24xx_pcm_dma_params s3c6400_ac97_pcm_stereo_out = {
	.client		= &s3c6400_dma_client_out,
	.channel	= DMACH_AC97_PCM_OUT,
	.dma_addr	= S3C6400_PA_AC97 + S3C_AC97_PCM_DATA,
	.dma_size	= 4,
};

#ifdef CONFIG_SOUND_WM9713_INPUT_STREAM_MIC
static struct s3c2410_dma_client s3c6400_dma_client_micin = {
	.name = "AC97 Mic Mono in"
};

static struct s3c24xx_pcm_dma_params s3c6400_ac97_mic_mono_in = {
	.client		= &s3c6400_dma_client_micin,
	.channel	= DMACH_AC97_MIC_IN,
	.dma_addr	= S3C6400_PA_AC97 + S3C_AC97_MIC_DATA,
	.dma_size	= 4,
};
#else /* Input Stream is LINE-IN */
static struct s3c2410_dma_client s3c6400_dma_client_in = {
	.name = "AC97 PCM Stereo Line in"
};

static struct s3c24xx_pcm_dma_params s3c6400_ac97_pcm_stereo_in = {
	.client		= &s3c6400_dma_client_in,
	.channel	= DMACH_AC97_PCM_IN,
	.dma_addr	= S3C6400_PA_AC97 + S3C_AC97_PCM_DATA,
	.dma_size	= 4,
};
#endif

static int s3c6400_ac97_probe(struct platform_device *pdev)
{
	int ret;

	s3cdbg("Entered %s\n", __FUNCTION__);

	s3c24xx_ac97.regs = ioremap(S3C6400_PA_AC97, 0x100);
	if (s3c24xx_ac97.regs == NULL)
		return -ENXIO;

	s3c24xx_ac97.ac97_clk = clk_get(&pdev->dev, "ac97");
	if (s3c24xx_ac97.ac97_clk == NULL) {
		printk(KERN_ERR "s3c6400-ac97 failed to get ac97_clock\n");
		iounmap(s3c24xx_ac97.regs);
		return -ENODEV;
	}
	clk_enable(s3c24xx_ac97.ac97_clk);
	/*
        s3c_gpio_cfgpin(S3C_GPD0,S3C_GPD0_AC97_BITCLK);
        s3c_gpio_cfgpin(S3C_GPD1,S3C_GPD1_AC97_RESET);
        s3c_gpio_cfgpin(S3C_GPD2,S3C_GPD2_AC97_SYNC);
        s3c_gpio_cfgpin(S3C_GPD3,S3C_GPD3_AC97_SDI);
        s3c_gpio_cfgpin(S3C_GPD4,S3C_GPD4_AC97_SDO);

        s3c_gpio_pullup(S3C_GPD0,0);
        s3c_gpio_pullup(S3C_GPD1,0);
        s3c_gpio_pullup(S3C_GPD2,0);
        s3c_gpio_pullup(S3C_GPD3,0);
        s3c_gpio_pullup(S3C_GPD4,0);
        */
        s3c_gpio_cfgpin(S3C64XX_GPD(0),S3C64XX_GPD0_AC97_BITCLK);
        s3c_gpio_cfgpin(S3C64XX_GPD(1),S3C64XX_GPD1_AC97_RESET);
        s3c_gpio_cfgpin(S3C64XX_GPD(2),S3C64XX_GPD2_AC97_SYNC);
        s3c_gpio_cfgpin(S3C64XX_GPD(3),S3C64XX_GPD3_AC97_SDI);
        s3c_gpio_cfgpin(S3C64XX_GPD(4),S3C64XX_GPD4_AC97_SDO);
        s3c_gpio_setpull(S3C64XX_GPD(0),S3C_GPIO_PULL_DOWN);
        s3c_gpio_setpull(S3C64XX_GPD(1),S3C_GPIO_PULL_DOWN);
        s3c_gpio_setpull(S3C64XX_GPD(2),S3C_GPIO_PULL_DOWN);
        s3c_gpio_setpull(S3C64XX_GPD(3),S3C_GPIO_PULL_DOWN);
        s3c_gpio_setpull(S3C64XX_GPD(4),S3C_GPIO_PULL_DOWN);
        
	ret = request_irq(IRQ_AC97, s3c6400_ac97_irq,
		IRQF_DISABLED, "AC97", NULL);
	if (ret < 0) {
		printk(KERN_ERR "s3c24xx-ac97: interrupt request failed.\n");
		clk_disable(s3c24xx_ac97.ac97_clk);
		clk_put(s3c24xx_ac97.ac97_clk);
		iounmap(s3c24xx_ac97.regs);
	}

	return ret;
}

static void s3c6400_ac97_remove(struct platform_device *pdev)
{
	s3cdbg("Entered %s\n", __FUNCTION__);

	free_irq(IRQ_AC97, NULL);
	clk_disable(s3c24xx_ac97.ac97_clk);
	clk_put(s3c24xx_ac97.ac97_clk);
	iounmap(s3c24xx_ac97.regs);
}

static int s3c6400_ac97_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	
	s3cdbg("Entered %s\n", __FUNCTION__);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		cpu_dai->dma_data = &s3c6400_ac97_pcm_stereo_out;
	else
#ifdef CONFIG_SOUND_WM9713_INPUT_STREAM_MIC
		cpu_dai->dma_data = &s3c6400_ac97_mic_mono_in;
#else /* Input Stream is LINE-IN */
		cpu_dai->dma_data = &s3c6400_ac97_pcm_stereo_in;
#endif

	
	return 0;
}

static int s3c6400_ac97_hifi_prepare(struct snd_pcm_substream *substream)
{
	/*
	 * To support full duplex  
	 * Tested by cat /dev/dsp > /dev/dsp
	 */
	s3cdbg("Entered %s\n", __FUNCTION__);

	s3c6400_ac97_write(0,0x26,0x0);
	s3c6400_ac97_write(0, 0x0c, 0x0808);
	s3c6400_ac97_write(0,0x3c, 0xf803);
	s3c6400_ac97_write(0,0x3e,0xb990);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		s3c6400_ac97_write(0,0x02, 0x8080);	
		s3c6400_ac97_write(0, 0x04, 0x0606);	
		s3c6400_ac97_write(0,0x1c, 0x00aa);
	}
	else
	{
		s3c6400_ac97_write(0, 0x12, 0x0f0f);
#ifdef CONFIG_SOUND_WM9713_INPUT_STREAM_MIC
		s3c6400_ac97_write(0,0x5c,0x2);
		s3c6400_ac97_write(0,0x10,0x68);
		s3c6400_ac97_write(0,0x14,0xfe00);
#else /* Input Stream is LINE-IN */
		s3c6400_ac97_write(0, 0x14, 0xd612);
#endif
	}

	return 0;
}


static int s3c6400_ac97_trigger(struct snd_pcm_substream *substream, int cmd)
{
	u32 ac_glbctrl;

	s3cdbg("Entered %s: cmd = %d\n", __FUNCTION__, cmd);

	ac_glbctrl = readl(s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	switch(cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			ac_glbctrl |= S3C_AC97_GLBCTRL_PCMINTM_DMA;
		else
			ac_glbctrl |= S3C_AC97_GLBCTRL_PCMOUTTM_DMA;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			ac_glbctrl &= ~S3C_AC97_GLBCTRL_PCMINTM_MASK;
		else
			ac_glbctrl &= ~S3C_AC97_GLBCTRL_PCMOUTTM_MASK;
		break;
	}
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);

	return 0;
}

#if CONFIG_SOUND_WM9713_INPUT_STREAM_MIC
static int s3c6400_ac97_hw_mic_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		return -ENODEV;
	else
		cpu_dai->dma_data = &s3c6400_ac97_mic_mono_in;

	return 0;
}

static int s3c6400_ac97_mic_trigger(struct snd_pcm_substream *substream,
	int cmd)
{
	u32 ac_glbctrl;

	s3cdbg("Entered %s\n", __FUNCTION__);

	ac_glbctrl = readl(s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);
	switch(cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		ac_glbctrl |= S3C_AC97_GLBCTRL_PCMINTM_DMA;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		ac_glbctrl &= ~S3C_AC97_GLBCTRL_PCMINTM_MASK;
	}
	writel(ac_glbctrl, s3c24xx_ac97.regs + S3C_AC97_GLBCTRL);

	return 0;
}
#endif

#define s3c6400_AC97_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |\
		SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 | \
		SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000)

struct snd_soc_dai s3c6400_ac97_dai[] = {
{
	.name = "s3c64xx-ac97",
	.id = 0,
	.type = SND_SOC_DAI_AC97,
	.probe = s3c6400_ac97_probe,
	.remove = s3c6400_ac97_remove,
	.playback = {
		.stream_name = "AC97 Playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = s3c6400_AC97_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
#if 1
	.capture = {
		.stream_name = "AC97 Mic Capture",
		.channels_min = 1,
		.channels_max = 1,
		.rates = s3c6400_AC97_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.ops = {
		.hw_params = s3c6400_ac97_hw_params,
		.prepare = s3c6400_ac97_hifi_prepare,
		.trigger = s3c6400_ac97_trigger},
},
#endif
#if CONFIG_SOUND_WM9713_INPUT_STREAM_MIC
#if 0
{
	.name = "s3c6400-ac97-mic",
	.id = 1,
	.type = SND_SOC_DAI_AC97,
	.capture = {
		.stream_name = "AC97 Mic Capture",
		.channels_min = 1,
		.channels_max = 1,
		.rates = s3c6400_AC97_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.ops = {
		.hw_params = s3c6400_ac97_hw_mic_params,
		.trigger = s3c6400_ac97_mic_trigger,},
},
#endif
#endif
};

EXPORT_SYMBOL_GPL(s3c6400_ac97_dai);
EXPORT_SYMBOL_GPL(soc_ac97_ops);

MODULE_AUTHOR("Ryu Euiyoul");
MODULE_DESCRIPTION("AC97 driver for the Samsung s3c6400 chip");
MODULE_LICENSE("GPL");
