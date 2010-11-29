/*
 * smdk6400_wm9713.c  --  SoC audio for smdk6400
 *
 * Copyright (C) 2007, Ryu Euiyoul <ryu.real@gmail.com>
 *
 * Copyright 2007 Wolfson Microelectronics PLC.
 * Author: Graeme Gregory
 *         graeme.gregory@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    8th  Mar 2007   Initial version.
 *    20th Sep 2007   Apply at smdk6400
 *
 */

#include <linux/module.h>
#include <linux/device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include "../codecs/wm9713.h"
#include "s3c-pcm.h"
#include "s3c64xx-ac97.h"

static struct snd_soc_machine smdk6400;

static struct snd_soc_dai_link smdk6400_dai[] = {
{
	.name = "AC97",
	.stream_name = "AC97 HiFi",
	.cpu_dai = &s3c6400_ac97_dai[0],
	.codec_dai = &wm9713_dai[WM9713_DAI_AC97_HIFI],
},
};

static struct snd_soc_machine smdk6400 = {
	.name = "SMDK6400",
	.dai_link = smdk6400_dai,
	.num_links = ARRAY_SIZE(smdk6400_dai),
};

static struct snd_soc_device smdk6400_snd_ac97_devdata = {
	.machine = &smdk6400,
	.platform = &s3c24xx_soc_platform,
	.codec_dev = &soc_codec_dev_wm9713,
};

static struct platform_device *smdk6400_snd_ac97_device;

static int __init smdk6400_init(void)
{
	int ret;

	smdk6400_snd_ac97_device = platform_device_alloc("soc-audio", -1);
	if (!smdk6400_snd_ac97_device)
		return -ENOMEM;

	platform_set_drvdata(smdk6400_snd_ac97_device,
				&smdk6400_snd_ac97_devdata);
	smdk6400_snd_ac97_devdata.dev = &smdk6400_snd_ac97_device->dev;
	ret = platform_device_add(smdk6400_snd_ac97_device);

	if (ret)
		platform_device_put(smdk6400_snd_ac97_device);

	return ret;
}

static void __exit smdk6400_exit(void)
{
	platform_device_unregister(smdk6400_snd_ac97_device);
}

module_init(smdk6400_init);
module_exit(smdk6400_exit);

/* Module information */
MODULE_AUTHOR("Samsung: Ryu");
MODULE_DESCRIPTION("ALSA SoC WM9713 SMDK6400");
MODULE_LICENSE("GPL");
