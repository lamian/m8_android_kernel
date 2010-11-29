/*
* smdk6410_wm8990.c  --  SoC audio for SMDK6410 with WM8990
 *
 * Copyright 2007, 2008 Wolfson Microelectronics PLC.
 * Author: Liam Girdwood
 *         lg@opensource.wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  Copyright (C) 2007, Ryu Euiyoul <ryu.real@gmail.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    28th Feb 2008   Initial version.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
//#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <plat/regs-iis.h>
#include <mach/map.h>

#include <plat/regs-gpio.h>
#include <mach/hardware.h>
#include <mach/audio.h>
#include <asm/io.h>
#include <plat/regs-clock.h>

#include "../codecs/wm8990.h"
#include "s3c-pcm.h"
#include "s3c-i2s.h"

/* define the scenarios */
#define SMDK6410_AUDIO_OFF		0
#define SMDK6410_CAPTURE_MIC1		3
#define SMDK6410_STEREO_TO_HEADPHONES	2
#define SMDK6410_CAPTURE_LINE_IN	1

#ifdef CONFIG_SND_DEBUG
#define s3cdbg(x...) printk(x)
#else
#define s3cdbg(x...)
#endif

/*
 * TODO: - We need to work out PLL values for 256FS for every rate.
 */
static int smdk6410_hifi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	unsigned int pll_out = 0;//, bclk = 0;
	int ret = 0;
	unsigned int iispsr, iismod;
	unsigned int prescaler = 4;

	u32*	regs;
	regs = ioremap(S3C64XX_PA_IIS, 0x100);

	s3cdbg("Entered %s, rate = %d\n", __FUNCTION__, params_rate(params));

	/*PCLK & SCLK gating enable*/
	writel(readl(S3C_PCLK_GATE)|S3C_CLKCON_PCLK_IIS0, S3C_PCLK_GATE);
	writel(readl(S3C_SCLK_GATE)|S3C_CLKCON_SCLK_AUDIO0, S3C_SCLK_GATE);

	iismod = readl(regs + S3C64XX_IIS0MOD);
	iismod &= ~S3C64XX_IIS0MOD_FS_MASK;
	iismod &= ~S3C64XX_IIS0MOD_BFS_MASK;

	/*Clear I2S prescaler value [13:8] and disable prescaler*/
	iispsr = readl(regs + S3C64XX_IIS0PSR);	
	iispsr &=~((0x3f<<8)|(1<<15)); 
	writel(iispsr, regs + S3C64XX_IIS0PSR);
	
	s3cdbg("%s: %d , params = %d\n", __FUNCTION__, __LINE__, params_rate(params));

	switch (params_rate(params)) {
	case 8000:
	case 16000:
	case 32000:
	case 64100:
		writel(50332, S3C_EPLL_CON1);
		writel((1<<31)|(32<<16)|(1<<8)|(3<<0) ,S3C_EPLL_CON0);
		break;
	case 11025:
	case 22050:
	case 44100:
	case 88200:
		writel(10398, S3C_EPLL_CON1);
		writel((1<<31)|(45<<16)|(1<<8)|(3<<0) ,S3C_EPLL_CON0);
		break;
	case 48000:
	case 96000:
		writel(9961, S3C_EPLL_CON1);
		writel((1<<31)|(49<<16)|(1<<8)|(3<<0) ,S3C_EPLL_CON0);
		break;
	default:
		writel(0, S3C_EPLL_CON1);
		writel((1<<31)|(128<<16)|(25<<8)|(0<<0) ,S3C_EPLL_CON0);
		break;
	}

	s3cdbg("%s, IISCON: %x IISMOD: %x,IISFIC: %x,IISPSR: %x\n",
			__FUNCTION__ , readl(S3C_IIS0CON), readl(S3C_IIS0MOD), 
			readl(S3C_IIS0FIC), readl(S3C_IIS0PSR));
	
	while(!(__raw_readl(S3C_EPLL_CON0)&(1<<30)));

	/* MUXepll : FOUTepll */
	writel(readl(S3C_CLK_SRC)|S3C_CLKSRC_EPLL_CLKSEL, S3C_CLK_SRC);
	/* AUDIO0 sel : FOUTepll */
	writel((readl(S3C_CLK_SRC)&~(0x7<<7))|(0<<7), S3C_CLK_SRC);

	/* CLK_DIV2 setting */
	writel(0x0,S3C_CLK_DIV2);

	switch (params_rate(params)) {
	case 8000:
		pll_out = 2048000;
		prescaler = 8;
		break;
	case 11025:
		pll_out = 2822400;
		prescaler = 8; 
		break;
	case 16000:
		pll_out = 4096000;
		prescaler = 4; 
		break;
	case 22050:
		pll_out = 5644800;
		prescaler = 4; 
		break;
	case 32000:
		pll_out = 8192000;
		prescaler = 2; 
		break;
	case 44100:
		pll_out = 11289600;
		//prescaler = 6; 
		prescaler = 2; 
		break;
	case 48000:
		pll_out = 12288000;
		prescaler = 2; 
		break;
	case 88200:
		pll_out = 22579200;
		prescaler = 1; 
		break;
	case 96000:
		pll_out = 24576000;
		prescaler = 1;
		break;
	default:
		/* somtimes 32000 rate comes to 96000 
		   default values are same as 32000 */
		iismod |= S3C64XX_IIS0MOD_384FS;
		pll_out = 12288000;
		break;
	}

	/* set MCLK division for sample rate */
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S8:
	case SNDRV_PCM_FORMAT_S16_LE:
		iismod |= S3C64XX_IIS0MOD_256FS | S3C64XX_IIS0MOD_32FS;
		prescaler *= 3;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		iismod |= S3C64XX_IIS0MOD_384FS | S3C64XX_IIS0MOD_48FS;
		prescaler *= 2;
		break;
	default:
		return -EINVAL;
	}

	prescaler = prescaler - 1; 
	writel(iismod , regs + S3C64XX_IIS0MOD);

	/* set codec DAI configuration */
	ret = codec_dai->dai_ops.set_fmt(codec_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS ); 
	if (ret < 0)
		return ret;

	/* set cpu DAI configuration */
	ret = cpu_dai->dai_ops.set_fmt(cpu_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS ); 
	if (ret < 0)
		return ret;

	/* set the codec system clock for DAC and ADC */
	ret = codec_dai->dai_ops.set_sysclk(codec_dai, WM8990_MCLK, pll_out,
		SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

	/* set prescaler division for sample rate */
	ret = cpu_dai->dai_ops.set_clkdiv(cpu_dai, S3C24XX_DIV_PRESCALER,
		(prescaler << 0x8));
	if (ret < 0)
		return ret;

	return 0;
}

/*
 * Neo1973 WM8990 HiFi DAI opserations.
 */
static struct snd_soc_ops smdk6410_hifi_ops = {
	.hw_params = smdk6410_hifi_hw_params,
};

static int smdk6410_scenario = 0;

static int smdk6410_get_scenario(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = smdk6410_scenario;
	return 0;
}

static int set_scenario_endpoints(struct snd_soc_codec *codec, int scenario)
{
	smdk6410_scenario = scenario;
	switch (smdk6410_scenario) {
	case SMDK6410_AUDIO_OFF:
		snd_soc_dapm_set_endpoint(codec, "Headphone Jack",    0);
		snd_soc_dapm_set_endpoint(codec, "Mic1 Jack",  0);
		snd_soc_dapm_set_endpoint(codec, "Line In Jack",  0);
		break;
	case SMDK6410_STEREO_TO_HEADPHONES:
		snd_soc_dapm_set_endpoint(codec, "Headphone Jack",    1);
		snd_soc_dapm_set_endpoint(codec, "Mic1 Jack",  0);
		snd_soc_dapm_set_endpoint(codec, "Line In Jack",  0);
		break;
	case SMDK6410_CAPTURE_MIC1:
		snd_soc_dapm_set_endpoint(codec, "Headphone Jack",    0);
		snd_soc_dapm_set_endpoint(codec, "Mic1 Jack",  1);
		snd_soc_dapm_set_endpoint(codec, "Line In Jack",  0);
		break;
	case SMDK6410_CAPTURE_LINE_IN:
		snd_soc_dapm_set_endpoint(codec, "Headphone Jack",    0);
		snd_soc_dapm_set_endpoint(codec, "Mic1 Jack",  0);
		snd_soc_dapm_set_endpoint(codec, "Line In Jack",  1);
		break;
	default:
		snd_soc_dapm_set_endpoint(codec, "Headphone Jack",    1);
		snd_soc_dapm_set_endpoint(codec, "Mic1 Jack",  1);
		snd_soc_dapm_set_endpoint(codec, "Line In Jack",  1);
		break;
	}

	snd_soc_dapm_sync_endpoints(codec);
	return 0;
}

static int smdk6410_set_scenario(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	if (smdk6410_scenario == ucontrol->value.integer.value[0])
		return 0;

	set_scenario_endpoints(codec, ucontrol->value.integer.value[0]);
	return 1;
}

static const struct snd_soc_dapm_widget wm8990_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_MIC("Mic1 Jack", NULL),
	SND_SOC_DAPM_LINE("Line In Jack", NULL),
};

/* example machine audio_mapnections */
static const struct snd_soc_dapm_route audio_map[] = {

	/* no irq jack detect */
	{"Headphone Jack", NULL, "LOUT"},
	{"Headphone Jack", NULL, "ROUT"},

	/* mic is connected to LIN1 and LIN2 - with bias */
	{"LIN1", NULL, "Mic1 Jack"},
	//{"LIN2", NULL, "Mic1 Jack"},

	{"LIN2", NULL, "Line In Jack"},
	{"RIN2", NULL, "Line In Jack"},
		
};

static const char *smdk_scenarios[] = {
	"Off",
	"Capture Line In",
	"Headphones",
	"Capture Mic1",
};

static const struct soc_enum smdk_scenario_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(smdk_scenarios),smdk_scenarios),
};

static const struct snd_kcontrol_new wm8990_smdk6410_controls[] = {
	SOC_ENUM_EXT("SMDK Mode", smdk_scenario_enum[0],
		smdk6410_get_scenario, smdk6410_set_scenario),
};

/*
 * This is an example machine initialisation for a wm8990 connected to a
 * smdk6410. It is missing logic to detect hp/mic insertions and logic
 * to re-route the audio in such an event.
 */
static int smdk6410_wm8990_init(struct snd_soc_codec *codec)
{
	int i, err;

	/* Add smdk6410 specific widgets */
	for (i = 0; i < ARRAY_SIZE(wm8990_dapm_widgets); i++)
		snd_soc_dapm_new_control(codec, &wm8990_dapm_widgets[i]);

	/* add smdk6410 specific controls */
	for (i = 0; i < ARRAY_SIZE(wm8990_smdk6410_controls); i++) {
		err = snd_ctl_add(codec->card,
				snd_soc_cnew(&wm8990_smdk6410_controls[i],
				codec, NULL));
		if (err < 0)
			return err;
	}

	/* set up smdk6410 specific audio paths */
	snd_soc_dapm_add_routes(codec, audio_map,ARRAY_SIZE(audio_map));

	/* not connected */
	snd_soc_dapm_set_endpoint(codec, "RIN1", 0);
	snd_soc_dapm_set_endpoint(codec, "LIN3", 0);
	snd_soc_dapm_set_endpoint(codec, "LIN4", 0);
	snd_soc_dapm_set_endpoint(codec, "RIN3", 0);
	snd_soc_dapm_set_endpoint(codec, "RIN4", 0);
	snd_soc_dapm_set_endpoint(codec, "OUT3", 0);
	snd_soc_dapm_set_endpoint(codec, "OUT4", 0);
	snd_soc_dapm_set_endpoint(codec, "SPKP", 0);
	snd_soc_dapm_set_endpoint(codec, "SPKN", 0);
	snd_soc_dapm_set_endpoint(codec, "ROP", 0);
	snd_soc_dapm_set_endpoint(codec, "RON", 0);
	snd_soc_dapm_set_endpoint(codec, "LOP", 0);
	snd_soc_dapm_set_endpoint(codec, "LON", 0);

	/* set endpoints to default mode & sync with DAPM */
	set_scenario_endpoints(codec, SMDK6410_STEREO_TO_HEADPHONES);
		
	return 0;
}

static struct snd_soc_dai_link smdk6410_dai[] = {
{ /* Hifi Playback - for similatious use with voice below */
	.name = "WM8990",
	.stream_name = "WM8990 HiFi",
	.cpu_dai = &s3c_i2s_dai,
	.codec_dai = &wm8990_dai,
	.init = smdk6410_wm8990_init,
	.ops = &smdk6410_hifi_ops,
},
};

static struct snd_soc_machine smdk6410 = {
	.name = "smdk6410",
	.dai_link = smdk6410_dai,
	.num_links = ARRAY_SIZE(smdk6410_dai),
};

static struct wm8990_setup_data smdk6410_wm8990_setup = {
	.i2c_address = 0x1b,
};

static struct snd_soc_device smdk6410_snd_devdata = {
	.machine = &smdk6410,
	.platform = &s3c24xx_soc_platform,
	.codec_dev = &soc_codec_dev_wm8990,
	.codec_data = &smdk6410_wm8990_setup,
};

static struct platform_device *smdk6410_snd_device;

static int __init smdk6410_init(void)
{
	int ret;

	smdk6410_snd_device = platform_device_alloc("soc-audio", -1);
	if (!smdk6410_snd_device)
		return -ENOMEM;

	platform_set_drvdata(smdk6410_snd_device, &smdk6410_snd_devdata);
	smdk6410_snd_devdata.dev = &smdk6410_snd_device->dev;
	ret = platform_device_add(smdk6410_snd_device);

	if (ret)
		platform_device_put(smdk6410_snd_device);
	
	return ret;
}

static void __exit smdk6410_exit(void)
{
	platform_device_unregister(smdk6410_snd_device);
}

module_init(smdk6410_init);
module_exit(smdk6410_exit);

/* Module information */
MODULE_AUTHOR("Liam Girdwood");
MODULE_DESCRIPTION("ALSA SoC WM8990 SMDK6410");
MODULE_LICENSE("GPL");
