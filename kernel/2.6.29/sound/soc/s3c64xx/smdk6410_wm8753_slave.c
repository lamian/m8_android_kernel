/*
 * smdk6400_wm8753.c  --  SoC audio for Neo1973
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
#include <plat/regs-gpio.h>
#include <plat/gpio-bank-h.h>
#include <plat/gpio-bank-c.h>

#include <mach/hardware.h>
#include <mach/audio.h>
#include <mach/map.h>
#include <asm/io.h>
#include <plat/regs-clock.h>
#include <plat/s3c6410.h>

#include "../codecs/wm8753.h"
#include "s3c-pcm.h"
#include "s3c-i2s.h"

#include <linux/irq.h>
#include <linux/workqueue.h>
#include <asm/io.h>
#include <linux/delay.h>

//#define CONFIG_SND_DEBUG
#ifdef CONFIG_SND_DEBUG
#define s3cdbg(x...) printk(x)
#else
#define s3cdbg(x...)
#endif

static int hp_jack = 0;
static int hp_mic = 0;
static int spk = 0;
static int ear = 0;
static int mic = 0;
static int gsm = 0;

static int endpoint_set(struct snd_soc_codec *codec, int val, int* pval, const char* name)
{
	if (*pval == val)
		return 0;

	*pval = val;
	
	if (*pval) {
		snd_soc_dapm_enable_pin(codec, name);
	} else {
		snd_soc_dapm_disable_pin(codec, name);
	};

	return 1;
}

static void set_m8_amp(int value)
{
	s3c_gpio_cfgpin(S3C64XX_GPL(14),S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S3C64XX_GPL(14), S3C_GPIO_PULL_NONE);
	gpio_set_value(S3C64XX_GPL(14), value);
}

static int smdk6400_hifi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	int bfs, rfs, psr, ret;
	unsigned int src_clk;
	unsigned int param_fmt = params_format(params);
	unsigned int param_rate = params_rate(params);
	
	s3cdbg("%s\n", __FUNCTION__);

	/* Choose BFS and RFS values combination that is supported by
	 * both the WM8753 codec as well as the S3C AP
	 *
	 * WM8753 codec supports only S16_LE, S20_3LE, S24_LE & S32_LE.
	 * S3C AP supports only S8, S16_LE & S24_LE.
	 * We implement all for completeness but only S16_LE & S24_LE bit-lengths 
	 * are possible for this AP-Codec combination.
	 */
	switch (param_fmt) {
	case SNDRV_PCM_FORMAT_S8:
		bfs = 16;
		rfs = 256;		/* Can take any RFS value for AP */
 		break;
 	case SNDRV_PCM_FORMAT_S16_LE:
		bfs = 32;
		rfs = 256;		/* Can take any RFS value for AP */
 		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
 	case SNDRV_PCM_FORMAT_S24_LE:
		bfs = 48;
		rfs = 512;		/* B'coz 48-BFS needs atleast 512-RFS acc to *S5P6440* UserManual */
 		break;
 	case SNDRV_PCM_FORMAT_S32_LE:	/* Impossible, as the AP doesn't support 64fs or more BFS */
	default:
		return -EINVAL;
 	}
 
	/* Select the AP Sysclk */
	ret = snd_soc_dai_set_sysclk(cpu_dai, S3C_CDCLKSRC_INT,param_rate, SND_SOC_CLOCK_OUT);
	if (ret < 0)
		return ret;


#if USE_AP_MASTER

#ifdef USE_CLKAUDIO
	ret = snd_soc_dai_set_sysclk(cpu_dai, S3C_CLKSRC_CLKAUDIO,param_rate, SND_SOC_CLOCK_OUT);
#else
	ret = snd_soc_dai_set_sysclk(cpu_dai, S3C_CLKSRC_PCLK, 0, SND_SOC_CLOCK_OUT);
#endif

#else
	s3cdbg("CLKSRC:SLVPCLK\n");
	ret = snd_soc_dai_set_sysclk(cpu_dai, S3C_IISMOD_SLVI2SCLK, 0, SND_SOC_CLOCK_OUT);
#endif

	if (ret < 0)
		return ret;

	/* Set the AP DAI configuration */
#if USE_AP_MASTER
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
#else
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM);
#endif

	if (ret < 0)
		return ret;

	/* Set the AP RFS */
	ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_DIV_MCLK, rfs);
	if (ret < 0)
		return ret;

	/* Set the AP BFS */
	ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_DIV_BCLK, bfs);
	if (ret < 0)
		return ret;

	src_clk = s3c_i2s_get_clockrate();
	
	switch (params_rate(params)) {
	case 8000:
	case 11025:
	case 16000:
	case 22050:
	case 32000:
	case 44100: 
	case 48000:
	case 64000:
	case 88200:
	case 96000:
		psr = src_clk / rfs / param_rate;
		ret = src_clk / rfs - psr * param_rate;
		if(ret >= param_rate/2)	// round off
		   psr += 1;
		psr -= 1;
		break;
	default:
		return -EINVAL;
	}

	s3cdbg("SRC_CLK=%d PSR=%d RFS=%d BFS=%d\n", src_clk, psr, rfs, bfs);

	/* Set the AP Prescalar */
	ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_DIV_PRESCALER, psr);
	if (ret < 0)
	{
		printk("%s:%d\n", __FILE__, __LINE__);
		return ret;
	}

		/* Set the Codec DAI configuration */
#if USE_AP_MASTER
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
#else
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM);
#endif
	if (ret < 0)
		return ret;

#if USE_AP_MASTER
	ret = snd_soc_dai_set_sysclk(codec_dai,WM8753_MCLK,src_clk,SND_SOC_CLOCK_IN);
#else
	ret = snd_soc_dai_set_sysclk(codec_dai,WM8753_MCLK,src_clk,SND_SOC_CLOCK_OUT);
#endif
	if (ret < 0)
	{
		printk("%s:%d\n", __FILE__, __LINE__);
		return ret;
	}

	/* set VXDOSR, ADCOSR and DACOSR to 1 for saving power*/
	ret = snd_soc_dai_set_clkdiv(codec_dai, WM8753_OSRCLR, 0x00);
	if (ret < 0){
		printk("%s: %d , excute wm8753_set_dai_clkdiv  failure\n", __FUNCTION__, __LINE__);
		return ret;
	}
	return 0;
}

static int smdk6400_hifi_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;

	/* disable the PLL */
	return snd_soc_dai_set_pll(codec_dai, WM8753_PLL1, 0, 0, 0);
}

/*
 * Neo1973 WM8753 HiFi DAI opserations.
 */
static struct snd_soc_ops smdk6400_hifi_ops = {
	.hw_params = smdk6400_hifi_hw_params,
	.hw_free = smdk6400_hifi_hw_free,
};

static const struct snd_soc_dapm_widget wm8753_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_MIC("Headphone Mic", NULL),
	SND_SOC_DAPM_SPK("Phone Speaker", NULL),
	SND_SOC_DAPM_SPK("Phone Earpiece", NULL),
	SND_SOC_DAPM_MIC("Phone Mic", NULL),
	SND_SOC_DAPM_LINE("GSM Speaker", NULL),
};


/* example machine audio_mapnections */
//static const char* audio_map[][3] = {
static const struct snd_soc_dapm_route audio_map[]={
	// Headphone connectors
	{"Headphone Jack", NULL, "LOUT1"},
	{"Headphone Jack", NULL, "ROUT1"},
	{"MIC2", NULL, "Headphone Mic"},
	{"MIC2N", NULL, "Headphone Mic"},

	// Phone connectors
	{"Phone Speaker", NULL, "LOUT2"},
	{"Phone Speaker", NULL, "ROUT2"},
	{"Phone Earpiece", NULL, "OUT3"},
	{"MIC1", NULL, "Phone Mic"},
	{"MIC1N", NULL, "Phone Mic"},

	{"RXP", NULL, "GSM Speaker"},
	{"RXN", NULL, "GSM Speaker"},
};

static const char *endpoint_cfgs[] = {
	"Off",
	"On",
};

static const struct soc_enum m8fe_endpoint_cfg[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(endpoint_cfgs),endpoint_cfgs),
};

static int hp_jack_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = hp_jack;
	return 0;
}

static int hp_jack_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret = endpoint_set(snd_kcontrol_chip(kcontrol), ucontrol->value.integer.value[0], &hp_jack, "Headphone Jack");
	snd_soc_dapm_sync(snd_kcontrol_chip(kcontrol));
	return ret;
}

static int hp_mic_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = hp_mic;
	return 0;
}

static int hp_mic_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret = endpoint_set(snd_kcontrol_chip(kcontrol), ucontrol->value.integer.value[0], &hp_mic, "Headphone Mic");
	snd_soc_dapm_sync(snd_kcontrol_chip(kcontrol));
	return ret;
}

static int spk_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = spk;
	return 0;
}

static int spk_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret = endpoint_set(snd_kcontrol_chip(kcontrol), ucontrol->value.integer.value[0], &spk, "Phone Speaker");
	snd_soc_dapm_sync(snd_kcontrol_chip(kcontrol));
	if (ret) {
		set_m8_amp(ucontrol->value.integer.value[0]);
	}
	return ret;
}

static int ear_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = ear;
	return 0;
}

static int ear_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret = endpoint_set(snd_kcontrol_chip(kcontrol), ucontrol->value.integer.value[0], &ear, "Phone Earpiece");
	snd_soc_dapm_sync(snd_kcontrol_chip(kcontrol));
	return ret;
}

static int mic_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = mic;
	return 0;
}

static int mic_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret = endpoint_set(snd_kcontrol_chip(kcontrol), ucontrol->value.integer.value[0], &mic, "Phone Mic");
	snd_soc_dapm_sync(snd_kcontrol_chip(kcontrol));
	return ret;
}

static int gsm_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = gsm;
	return 0;
}

static int gsm_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret = endpoint_set(snd_kcontrol_chip(kcontrol), ucontrol->value.integer.value[0], &gsm, "GSM Speaker");
	snd_soc_dapm_sync(snd_kcontrol_chip(kcontrol));
	return ret;
}

static const struct snd_kcontrol_new m8fe_controls[] = {
	SOC_ENUM_EXT("Headphone_Jack", m8fe_endpoint_cfg[0],
		hp_jack_get, hp_jack_set),
	SOC_ENUM_EXT("Headphone_Mic", m8fe_endpoint_cfg[0],
		hp_mic_get, hp_mic_set),
	SOC_ENUM_EXT("Phone_Speaker", m8fe_endpoint_cfg[0],
		spk_get, spk_set),
	SOC_ENUM_EXT("Phone_Earpiece", m8fe_endpoint_cfg[0],
		ear_get, ear_set),
	SOC_ENUM_EXT("Phone_Mic", m8fe_endpoint_cfg[0],
		mic_get, mic_set),
	SOC_ENUM_EXT("GSM_Speaker", m8fe_endpoint_cfg[0],
		gsm_get, gsm_set),
};

/*
 * This is an example machine initialisation for a wm8753 connected to a
 * smdk6400. It is missing logic to detect hp/mic insertions and logic
 * to re-route the audio in such an event.
 */
static int smdk6400_wm8753_init(struct snd_soc_codec *codec)
{
	int i, err;

	/* Add smdk6400 specific widgets */
	for (i = 0; i < ARRAY_SIZE(wm8753_dapm_widgets); i++)
		snd_soc_dapm_new_control(codec, &wm8753_dapm_widgets[i]);

	/* set up smdk6400 specific audio path audio_mapnects */
	err = snd_soc_dapm_add_routes(codec, audio_map,
				      ARRAY_SIZE(audio_map));

	/* add smdk6410 specific controls */
	for (i = 0; i < ARRAY_SIZE(m8fe_controls); i++) {
		err = snd_ctl_add(codec->card,
				snd_soc_cnew(&m8fe_controls[i],
				codec, NULL));
		if (err < 0)
			return err;
	}

	endpoint_set(codec, 0, &hp_jack, "Headphone Jack");
	endpoint_set(codec, 0, &hp_mic, "Headphone Mic");
	endpoint_set(codec, 0, &spk, "Phone Speaker");
	endpoint_set(codec, 0, &ear, "Phone Earpiece");
	endpoint_set(codec, 0, &mic, "Phone Mic");
	endpoint_set(codec, 0, &gsm, "GSM Speaker");

	snd_soc_dapm_sync(codec);

	return 0;
}

static struct snd_soc_dai_link smdk6400_dai[] = {
{ /* Hifi Playback - for similatious use with voice below */
	.name = "WM8753",
	.stream_name = "WM8753 HiFi",
	.cpu_dai = &s3c_i2s_dai,
	.codec_dai = &wm8753_dai[WM8753_DAI_HIFI],
	.init = smdk6400_wm8753_init,
	.ops = &smdk6400_hifi_ops,
},
};

static struct snd_soc_card smdk6400 = {
	.name = "smdk6400",
	.dai_link = smdk6400_dai,
	.num_links = ARRAY_SIZE(smdk6400_dai),
	.platform = &s3c24xx_soc_platform,
};

static struct wm8753_setup_data smdk6400_wm8753_setup = {
	.i2c_address = 0x1a,
};

static struct snd_soc_device smdk6400_snd_devdata = {
	.card = &smdk6400,
	.codec_dev = &soc_codec_dev_wm8753,
	.codec_data = &smdk6400_wm8753_setup,
};

static struct platform_device *smdk6400_snd_device;

static int __init smdk6400_init(void)
{
	int ret;
	
	if (m8_checkse()) {
		return 0;
	}

	smdk6400_snd_device = platform_device_alloc("soc-audio", -1);
	if (!smdk6400_snd_device)
		return -ENOMEM;

	printk("%s:%d\n", __FILE__, __LINE__);
	platform_set_drvdata(smdk6400_snd_device, &smdk6400_snd_devdata);
	smdk6400_snd_devdata.dev = &smdk6400_snd_device->dev;
	ret = platform_device_add(smdk6400_snd_device);

	if (ret)
		platform_device_put(smdk6400_snd_device);

	return ret;
}

static void __exit smdk6400_exit(void)
{
	platform_device_unregister(smdk6400_snd_device);
}

module_init(smdk6400_init);
module_exit(smdk6400_exit);

/* Module information */
MODULE_AUTHOR("Ryu Euiyoul");
MODULE_DESCRIPTION("ALSA SoC WM8753 Neo1973");
MODULE_LICENSE("GPL");
