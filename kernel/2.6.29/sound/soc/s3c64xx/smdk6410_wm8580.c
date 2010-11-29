/*
 * smdk6400_wm8580.c
 *
 * Copyright 2007, 2008 Wolfson Microelectronics PLC.
 *
 * Copyright (C) 2007, Ryu Euiyoul <ryu.real@gmail.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
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
#include <plat/regs-gpio.h>
#include <plat/gpio-bank-h.h>
#include <plat/gpio-bank-c.h>

#include <mach/hardware.h>
#include <mach/audio.h>
#include <asm/io.h>
#include <plat/regs-clock.h>

#include "../codecs/wm8580.h"
#include "s3c-pcm.h"
#include "s3c-i2s.h"

#ifdef CONFIG_SND_DEBUG
#define s3cdbg(x...) printk(x)
#else
#define s3cdbg(x...)
#endif

static int smdk6410_hifi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	//struct snd_soc_codec_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	unsigned int pll_out = 0; /*bclk = 0; */
	int ret = 0;
	unsigned int prescaler;

	s3cdbg("Entered %s, rate = %d\n", __FUNCTION__, params_rate(params));

	/*PCLK & SCLK gating enable*/
	writel(readl(S3C_PCLK_GATE)|S3C6410_CLKCON_PCLK_IIS2, S3C_PCLK_GATE);
	writel(readl(S3C_SCLK_GATE)|S3C_CLKCON_SCLK_AUDIO0, S3C_SCLK_GATE);

	/*Clear I2S prescaler value [13:8] and disable prescaler*/
	/* set prescaler division for sample rate */
	ret = cpu_dai->dai_ops.set_clkdiv(cpu_dai, S3C24XX_DIV_PRESCALER, 0);
	if (ret < 0)
		return ret;
	
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
		/* K=10398, M=45, P=1, S=3 -- Fout=67.738 */
		writel(10398, S3C_EPLL_CON1);
		writel((1<<31)|(45<<16)|(1<<8)|(3<<0) ,S3C_EPLL_CON0);
		break;
	case 48000:
	case 96000:
		/* K=9961, M=49, P=1, S=3 -- Fin=12, Fout=73.728; r=1536 */
		writel(9961, S3C_EPLL_CON1);
		writel((1<<31)|(49<<16)|(1<<8)|(3<<0) ,S3C_EPLL_CON0);
		break;
	default:
		writel(0, S3C_EPLL_CON1);
		writel((1<<31)|(128<<16)|(25<<8)|(0<<0) ,S3C_EPLL_CON0);
		break;
	}

	while(!(__raw_readl(S3C_EPLL_CON0)&(1<<30)));

	/* MUXepll : FOUTepll */
	writel(readl(S3C_CLK_SRC)|S3C6400_CLKSRC_EPLL_MOUT, S3C_CLK_SRC);

	/* AUDIO2 sel : FOUTepll */
	writel((readl(S3C_CLK_SRC2)&~(0x7<<0))|(0<<0), S3C_CLK_SRC2);

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
		/* Fout=73.728 */
		pll_out = 11289600;
		prescaler = 2;
		break;
	case 48000:
		/* Fout=67.738 */
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
		prescaler = 4;
		pll_out = 12288000;
		break;
	}

	/* set MCLK division for sample rate */
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S8:
	case SNDRV_PCM_FORMAT_S16_LE:
		prescaler *= 3;
		break;
	case SNDRV_PCM_FORMAT_S24_3LE:
		prescaler *= 2;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		prescaler *= 2;
		break;
	default:
		return -EINVAL;
	}

	prescaler = prescaler - 1; 

	/* set cpu DAI configuration */
	ret = cpu_dai->dai_ops.set_fmt(cpu_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS); 
	if (ret < 0)
		return ret;

	ret = cpu_dai->dai_ops.set_clkdiv(cpu_dai, S3C24XX_DIV_BCLK,
		S3C64XX_IIS0MOD_256FS);
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
 * WM8580 HiFi DAI opserations.
 */
static struct snd_soc_ops smdk6410_hifi_ops = {
	.hw_params = smdk6410_hifi_hw_params,
};

static const struct snd_soc_dapm_widget wm8580_dapm_widgets[] = {
	SND_SOC_DAPM_LINE("I2S Front Jack", NULL),
	SND_SOC_DAPM_LINE("I2S Center Jack", NULL),
	SND_SOC_DAPM_LINE("I2S Rear Jack", NULL),
	SND_SOC_DAPM_LINE("Line In Jack", NULL),
};

/* example machine audio_mapnections */
static const struct snd_soc_dapm_route audio_map[] = {

	{ "I2S Front Jack", NULL, "VOUT1L" },
	{ "I2S Front Jack", NULL, "VOUT1R" },

	{ "I2S Center Jack", NULL, "VOUT2L" },
	{ "I2S Center Jack", NULL, "VOUT2R" },

	{ "I2S Rear Jack", NULL, "VOUT3L" },
	{ "I2S Rear Jack", NULL, "VOUT3R" },

	{ "AINL", NULL, "Line In Jack" },
	{ "AINR", NULL, "Line In Jack" },
		
};

static int smdk6410_wm8580_init(struct snd_soc_codec *codec)
{
	int i;

	/* Add smdk6410 specific widgets */
		snd_soc_dapm_new_controls(codec, wm8580_dapm_widgets,ARRAY_SIZE(wm8580_dapm_widgets));

	/* set up smdk6410 specific audio paths */
		snd_soc_dapm_add_routes(codec, audio_map,ARRAY_SIZE(audio_map));

	/* No jack detect - mark all jacks as enabled */
	for (i = 0; i < ARRAY_SIZE(wm8580_dapm_widgets); i++)
		snd_soc_dapm_set_endpoint(codec,
					  wm8580_dapm_widgets[i].name, 1);

	snd_soc_dapm_sync_endpoints(codec);

	return 0;
}

static struct snd_soc_dai_link smdk6410_dai[] = {
{
	.name = "WM8580",
	.stream_name = "WM8580 HiFi Playback",
	.cpu_dai = &s3c_i2s_v40_dai,
	.codec_dai = &wm8580_dai[WM8580_DAI_PAIFRX],
	.init = smdk6410_wm8580_init,
	.ops = &smdk6410_hifi_ops,
},
};

static struct snd_soc_machine smdk6410 = {
	.name = "smdk6410",
	.dai_link = smdk6410_dai,
	.num_links = ARRAY_SIZE(smdk6410_dai),
};

static struct wm8580_setup_data smdk6410_wm8580_setup = {
	.i2c_address = 0x1b,
};

static struct snd_soc_device smdk6410_snd_devdata = {
	.machine = &smdk6410,
	.platform = &s3c24xx_soc_platform,
	.codec_dev = &soc_codec_dev_wm8580,
	.codec_data = &smdk6410_wm8580_setup,
};

static struct platform_device *smdk6410_snd_device;

static int __init smdk6410_init(void)
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
MODULE_AUTHOR("Mark Brown");
MODULE_DESCRIPTION("ALSA SoC SMDK6410 WM8580");
MODULE_LICENSE("GPL");
