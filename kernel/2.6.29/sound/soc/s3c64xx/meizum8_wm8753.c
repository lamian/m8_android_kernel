/*
 * meizum8_wm8753.c  --  SoC audio for smdk6400_wm8753.c
 *
 * Copyright 2007 Wolfson Microelectronics PLC.
 * Author: Graeme Gregory
 *         graeme.gregory@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  Copyright (C) 2007, Ryu Euiyoul <ryu.real@gmail.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    20th Jan 2007   Initial version.
 *    05th Feb 2007   Rename all to Neo1973
 *
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>

#include <plat/regs-iis.h>
#include <plat/map-base.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-timer.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-bank-h.h>
#include <plat/gpio-bank-c.h>

#include <mach/hardware.h>
#include <mach/audio.h>
#include <asm/io.h>
#include <plat/regs-clock.h>

#include "../codecs/wm8753.h"
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

static int meizum8_hifi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	unsigned int pll_out = 0, bclk = 0;
	int ret = 0;
	unsigned int iispsr, iismod;
	unsigned int prescaler = 4;
	u32*	regs;

// Add by lamian
	static bool init = false;
	if (!init) {
		init = true;
		unsigned long tcnt = __raw_readl(S3C_TCNTB(4));
//		tcnt = tcnt * 2;
//		__raw_writel(tcnt, S3C_TCNTB(4));
		printk("tcnt=%d\n", tcnt);
	}
// End

	regs = ioremap(S3C_IIS_PABASE, 0x100);

	s3cdbg("Entered %s, rate = %d\n", __FUNCTION__, params_rate(params));
	printk("Entered %s, rate = %d\n", __FUNCTION__, params_rate(params));

	/*PCLK & SCLK gating enable*/
	writel(readl(S3C_PCLK_GATE)|S3C_CLKCON_PCLK_IIS0, S3C_PCLK_GATE);
	writel(readl(S3C_SCLK_GATE)|S3C_CLKCON_SCLK_AUDIO0, S3C_SCLK_GATE);

	iismod = readl(regs+S3C64XX_IIS0MOD);
	iismod &=~S3C64XX_IIS0MOD_FS_MASK;

	/*Clear I2S prescaler value [13:8] and disable prescaler*/
	iispsr = readl(regs+S3C64XX_IIS0PSR);
	iispsr &=~((0x3f<<8)|(1<<15));
	writel(iispsr, regs + S3C64XX_IIS0PSR);

	s3cdbg("%s: %d , params = %d \n", __FUNCTION__, __LINE__, params_rate(params));
	printk("%s: %d , params = %d \n", __FUNCTION__, __LINE__, params_rate(params));

	switch (params_rate(params)) {
	case 16000:
	case 32000:
		writel(0, S3C_EPLL_CON1);
		writel((1<<31)|(128<<16)|(25<<8)|(0<<0) ,S3C_EPLL_CON0);
		printk("rate=16000 or 32000\n");
		break;
	//case 8000:
	//	prescaler = 0xe;
	case 48000:
		writel(0, S3C_EPLL_CON1);
		writel((1<<31)|(192<<16)|(25<<8)|(0<<0) ,S3C_EPLL_CON0);
		printk("rate=48000\n");
		break;
	case 11025:
		prescaler = 9;
	case 8000:
	case 22050:
	case 44100:
		writel(0, S3C_EPLL_CON1);
		writel((1<<31)|(254<<16)|(9<<8)|(2<<0) ,S3C_EPLL_CON0);
		printk("rate=8000 or 11025 or 22050 or 44100\n");
		break;
	default:
		/* somtimes 32000 rate comes to 96000
		   default values are same as 32000 */
		writel(0, S3C_EPLL_CON1);
		writel((1<<31)|(128<<16)|(25<<8)|(0<<0) ,S3C_EPLL_CON0);
		printk("other rate\n");

		/* for 96000 rate : error 0.3%
		 * prescaler = 1;
		 * writel((1<<31)|(154<<16)|(25<<8)|(0<<0) ,S3C_EPLL_CON0);
		 */
		break;
	}

	s3cdbg("%s, IISCON: %x IISMOD: %x,IISFIC: %x,IISPSR: %x",
			__FUNCTION__ , readl(regs+S3C64XX_IIS0CON), readl(regs+S3C64XX_IIS0MOD),
			readl(regs+S3C64XX_IIS0FIC), readl(regs+S3C64XX_IIS0PSR));

	while(!(__raw_readl(S3C_EPLL_CON0)&(1<<30)));

	/* MUXepll : FOUTepll */
	writel(readl(S3C_CLK_SRC)|S3C_CLKSRC_EPLL_CLKSEL, S3C_CLK_SRC);
	/* AUDIO0 sel : FOUTepll */
	writel((readl(S3C_CLK_SRC)&~(0x7<<7))|(0<<7), S3C_CLK_SRC);

	/* CLK_DIV2 setting */
	writel(0x0,S3C_CLK_DIV2);

	switch (params_rate(params)) {
	case 11025:
		iismod |= S3C64XX_IIS0MOD_768FS;
		bclk = WM8753_BCLK_DIV_16;
		pll_out = 16934400;
		break;
	case 16000:
		iismod |= S3C64XX_IIS0MOD_768FS;
		bclk = WM8753_BCLK_DIV_2;
		pll_out = 12288000;
		break;
	case 22050:
		iismod |= S3C64XX_IIS0MOD_768FS;
		bclk = WM8753_BCLK_DIV_8;
		pll_out = 16934400;
		break;
	case 32000:
		iismod |= S3C64XX_IIS0MOD_384FS;
		bclk = WM8753_BCLK_DIV_2;
		pll_out = 12288000;
		break;
	case 44100:
	case 8000:
		iismod |= S3C64XX_IIS0MOD_384FS;
		bclk = WM8753_BCLK_DIV_4;
		pll_out = 16934400;
		break;
	case 48000:
		iismod |= S3C64XX_IIS0MOD_384FS;
		bclk = WM8753_BCLK_DIV_4;
		pll_out = 18432000;
		break;
	default:
		/* somtimes 32000 rate comes to 96000
		   default values are same as 32000 */
		iismod |= S3C64XX_IIS0MOD_384FS;
		bclk = WM8753_BCLK_DIV_2;
		pll_out = 12288000;
		break;
	}
	printk("pll_out=%d\n", pll_out);

	writel(iismod , regs+S3C64XX_IIS0MOD);

	/* set codec DAI configuration */
	ret = codec_dai->dai_ops.set_fmt(codec_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS );
	if (ret < 0)
	{
		printk("fail at 1\n");
		return ret;
	}

	/* set cpu DAI configuration */
	ret = cpu_dai->dai_ops.set_fmt(cpu_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS );
	if (ret < 0)
	{
		printk("fail at 2\n");
		return ret;
	}

	/* set the codec system clock for DAC and ADC */
	ret = codec_dai->dai_ops.set_sysclk(codec_dai, WM8753_MCLK, pll_out,
		SND_SOC_CLOCK_IN);
	if (ret < 0)
	{
		printk("fail at 3\n");
		return ret;
	}

	/* set MCLK division for sample rate */
	ret = cpu_dai->dai_ops.set_clkdiv(cpu_dai, S3C_DIV_MCLK,
		32 );
	if (ret < 0)
	{
		printk("fail at 4\n");
		return ret;
	}

	/* set codec BCLK division for sample rate */
	ret = codec_dai->dai_ops.set_clkdiv(codec_dai, WM8753_BCLKDIV, bclk);
	if (ret < 0)
	{
		printk("fail at 5\n");
		return ret;
	}

	/* set prescaler division for sample rate */
	ret = cpu_dai->dai_ops.set_clkdiv(cpu_dai, S3C_DIV_PRESCALER,
		(prescaler << 0x8));
	if (ret < 0)
	{
		printk("fail at 6\n");
		return ret;
	}

	return 0;
}

static int meizum8_hifi_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;

	/* disable the PLL */
	return codec_dai->dai_ops.set_pll(codec_dai, WM8753_PLL1, 0, 0);
}

/*
 * Neo1973 WM8753 HiFi DAI opserations.
 */
static struct snd_soc_ops meizum8_hifi_ops = {
	.hw_params = meizum8_hifi_hw_params,
	.hw_free = meizum8_hifi_hw_free,
};

static int meizum8_scenario = 0;

static int meizum8_get_scenario(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = meizum8_scenario;
	return 0;
}

static int set_scenario_endpoints(struct snd_soc_codec *codec, int scenario)
{
	switch(meizum8_scenario) {
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

static int meizum8_set_scenario(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	if (meizum8_scenario == ucontrol->value.integer.value[0])
		return 0;

	meizum8_scenario = ucontrol->value.integer.value[0];
	set_scenario_endpoints(codec, meizum8_scenario);
	return 1;
}

static const struct snd_soc_dapm_widget wm8753_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_MIC("Mic1 Jack", NULL),
	SND_SOC_DAPM_LINE("Line In Jack", NULL),
};


/* example machine audio_mapnections */
static const struct snd_soc_dapm_route audio_map[] = {

	{"Headphone Jack", NULL, "LOUT1"},
	{"Headphone Jack", NULL, "ROUT1"},

	/* mic is connected to mic1 - with bias */
	/* mic is connected to mic1 - with bias */
	{"MIC1", NULL, "Mic1 Jack"},

	{"LINE1", NULL, "Line In Jack"},
	{"LINE2", NULL, "Line In Jack"},

	/* Connect the ALC pins */
	{"ACIN", NULL, "ACOP"},

	/*add by cefanty*/
	#if 0
	{"Playback SPK", NULL, "LOUT2"},
	{"Playback SPK", NULL, "ROUT2"},

	{"RA Reciever", NULL, "OUT2"},

	{"MIC2", NULL, "Mic2 Jack"},
	#endif
	/*add end*/
};

static const char *meizu_scenarios[] = {
	"Off",
	"Capture Line In",
	"Headphones",
	"Capture Mic1",
	/*add by cefanty*/
	#if 0
	"Capture Mic2",
	"Playback Spk",
	"RA reciever"
	#endif
	/*add end*/
};

static const struct soc_enum meizu_scenario_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(meizu_scenarios),meizu_scenarios),
};

static const struct snd_kcontrol_new wm8753_meizum8_controls[] = {
	SOC_ENUM_EXT("SMDK Mode", meizu_scenario_enum[0],
		meizum8_get_scenario, meizum8_set_scenario),
};

/*
 * This is an example machine initialisation for a wm8753 connected to a
 * smdk6410. It is missing logic to detect hp/mic insertions and logic
 * to re-route the audio in such an event.
 */
static int meizum8_wm8753_init(struct snd_soc_codec *codec)
{
	int i, err;

	/* set endpoints to default mode */
	set_scenario_endpoints(codec, SMDK6410_AUDIO_OFF);

	/* Add smdk6400 specific widgets */
	for (i = 0; i < ARRAY_SIZE(wm8753_dapm_widgets); i++)
		snd_soc_dapm_new_control(codec, &wm8753_dapm_widgets[i]);

	/* add smdk6400 specific controls */
	for (i = 0; i < ARRAY_SIZE(wm8753_meizum8_controls); i++) {
		err = snd_ctl_add(codec->card,
				snd_soc_cnew(&wm8753_meizum8_controls[i],
				codec, NULL));
		if (err < 0)
			return err;
	}

	/* set up smdk6400 specific audio path audio_mapnects */
	snd_soc_dapm_add_routes(codec, audio_map,ARRAY_SIZE(audio_map));

	/* always connected */
	snd_soc_dapm_set_endpoint(codec, "Mic1 Jack", 1);
	snd_soc_dapm_set_endpoint(codec, "Headphone Jack", 1);
	snd_soc_dapm_set_endpoint(codec, "Line In Jack", 1);

	snd_soc_dapm_sync_endpoints(codec);
	return 0;
}

static struct snd_soc_dai_link meizum8_dai[] = {
{ /* Hifi Playback - for similatious use with voice below */
	.name = "WM8753LGEB/RV",
	.stream_name = "WM8753 HiFi",
	.cpu_dai = &s3c_i2s_dai,
	.codec_dai = &wm8753_dai[WM8753_DAI_HIFI],
	.init = meizum8_wm8753_init,
	.ops = &meizum8_hifi_ops,
},
};

static struct snd_soc_machine meizum8 = {
	.name = "MEIZUM8-WM8753",
	.dai_link = meizum8_dai,
	.num_links = ARRAY_SIZE(meizum8_dai),
};

static struct wm8753_setup_data meizum8_wm8753_setup = {
	.i2c_address = 0x1a,		/*default:0x1a*/
};

static struct snd_soc_device meizum8_snd_devdata = {
	.machine = &meizum8,
	.platform = &s3c24xx_soc_platform,
	.codec_dev = &soc_codec_dev_wm8753,
	.codec_data = &meizum8_wm8753_setup,
};

static struct platform_device *meizum8_snd_device;

static int __init meizum8_init(void)
{
	int ret;
	unsigned int *reg_GPHCON1;
	unsigned int *reg_GPCCON;

	reg_GPHCON1 = ioremap(0x7f0080e4,0x100);
	reg_GPCCON = ioremap(0x7f008040,0x100);

	s3c_gpio_cfgpin(S3C64XX_GPH(6), S3C64XX_GPH6_I2S_V40_BCLK);
	s3c_gpio_cfgpin(S3C64XX_GPH(7), S3C64XX_GPH7_I2S_V40_CDCLK);
	writel(0x50550000, reg_GPCCON);
	writel(0x00000055, reg_GPHCON1);

	s3c_gpio_cfgpin(S3C64XX_GPC(4), S3C64XX_GPC4_I2S_V40_DO0);
	s3c_gpio_cfgpin(S3C64XX_GPC(5), S3C64XX_GPC5_I2S_V40_DO1);
	s3c_gpio_cfgpin(S3C64XX_GPC(7), S3C64XX_GPC7_I2S_V40_DO2);


	/* pull-up-enable, pull-down-disable*/
	s3c_gpio_setpull(S3C64XX_GPH(6), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(S3C64XX_GPH(7), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(S3C64XX_GPH(8), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(S3C64XX_GPH(9), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(S3C64XX_GPC(4), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(S3C64XX_GPC(5), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(S3C64XX_GPC(7), S3C_GPIO_PULL_UP);


	meizum8_snd_device = platform_device_alloc("soc-audio", -1);
	if (!meizum8_snd_device)
		return -ENOMEM;

	platform_set_drvdata(meizum8_snd_device, &meizum8_snd_devdata);
	meizum8_snd_devdata.dev = &meizum8_snd_device->dev;
	ret = platform_device_add(meizum8_snd_device);

	if (ret)
		platform_device_put(meizum8_snd_device);

	return ret;
}

static void __exit meizum8_exit(void)
{
	platform_device_unregister(meizum8_snd_device);
}

module_init(meizum8_init);
module_exit(meizum8_exit);

/* Module information */
MODULE_AUTHOR("Cefanty");
MODULE_DESCRIPTION("ALSA SoC MeizuM8 WM8753");
MODULE_LICENSE("GPL");
