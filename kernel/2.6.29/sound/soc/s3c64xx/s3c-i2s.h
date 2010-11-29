/*
 * s3c24xx-i2s.c  --  ALSA Soc Audio Layer
 *
 * Copyright 2005 Wolfson Microelectronics PLC.
 * Author: Graeme Gregory
 *         graeme.gregory@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    10th Nov 2006   Initial version.
 */

#ifndef S3C_I2S_H_
#define S3C_I2S_H_

#define USE_CLKAUDIO	0
#define USE_AP_MASTER   1 	/*1: AP Master Mode, 0: Codec Master Mode*/

/* Clock dividers */
#define S3C_DIV_MCLK	0
#define S3C_DIV_BCLK	1
#define S3C_DIV_PRESCALER	2

#define S3C_IISCON		(0x00)
#define S3C_IISMOD		(0x04)
#define S3C_IISFIC		(0x08)
#define S3C_IISPSR		(0x0C)
#define S3C_IISTXD		(0x10)
#define S3C_IISRXD		(0x14)

#define S3C_IISCON_I2SACTIVE	(0x1<<0)
#define S3C_IISCON_RXDMACTIVE	(0x1<<1)
#define S3C_IISCON_TXDMACTIVE	(0x1<<2)
#define S3C_IISCON_RXCHPAUSE	(0x1<<3)
#define S3C_IISCON_TXCHPAUSE	(0x1<<4)
#define S3C_IISCON_RXDMAPAUSE	(0x1<<5)
#define S3C_IISCON_TXDMAPAUSE	(0x1<<6)
#define S3C_IISCON_FRXFULL		(0x1<<7)

#if defined(CONFIG_SND_S3C_I2S_V40) || defined(CONFIG_SND_S3C_I2S_V40_MODULE)
#define S3C_IISCON_FTX0FULL		(0x1<<8)
#else
#define S3C_IISCON_FTXFULL		(0x1<<8)
#endif
#define S3C_IISCON_FRXEMPT		(0x1<<9)
#define S3C_IISCON_FTX0EMPT		(0x1<<10)
#define S3C_IISCON_LRI		(0x1<<11)
#if defined(CONFIG_SND_S3C_I2S_V40) || defined(CONFIG_SND_S3C_I2S_V40_MODULE)
#define S3C_IISCON_FTX1FULL		(0x1<<12)
#define S3C_IISCON_FTX2FULL		(0x1<<13)
#define S3C_IISCON_FTX1EMPT		(0x1<<14)
#define S3C_IISCON_FTX2EMPT		(0x1<<15)
#endif
#define S3C_IISCON_FTXURINTEN	(0x1<<16)
#define S3C_IISCON_FTXURSTATUS	(0x1<<17)
#if defined(CONFIG_SND_S3C_I2S_V32) || defined(CONFIG_SND_S3C_I2S_V32_MODULE)
#define S3C_IISCON_FRXORINTEN	(0x1<<18)
#define S3C_IISCON_FRXORSTATUS	(0x1<<19)
#endif
#if defined(CONFIG_SND_S3C_I2S_V50) || defined(CONFIG_SND_S3C_I2S_V50_MODULE)
#define S3C_IISCON_SWRESET	(0x1<<31)
#else
#define S3C_IISCON_SWRESET	S3C_IISCON_I2SACTIVE /* Indifferent for others */
#endif

#define S3C_IISMOD_BFSMASK		(3<<1)
#define S3C_IISMOD_32FS		(0<<1)
#define S3C_IISMOD_48FS		(1<<1)
#define S3C_IISMOD_16FS		(2<<1)
#define S3C_IISMOD_24FS		(3<<1)

#define S3C_IISMOD_RFSMASK		(3<<3)
#define S3C_IISMOD_256FS		(0<<3)
#define S3C_IISMOD_512FS		(1<<3)
#define S3C_IISMOD_384FS		(2<<3)
#define S3C_IISMOD_768FS		(3<<3)

#define S3C_IISMOD_SDFMASK		(3<<5)
#define S3C_IISMOD_IIS		(0<<5)
#define S3C_IISMOD_MSB		(1<<5)
#define S3C_IISMOD_LSB		(2<<5)

#define S3C_IISMOD_LRP		(1<<7)

#define S3C_IISMOD_TXRMASK		(3<<8)
#define S3C_IISMOD_TX		(0<<8)
#define S3C_IISMOD_RX		(1<<8)
#define S3C_IISMOD_TXRX		(2<<8)

#define S3C_IISMOD_IMSMASK		(3<<10)
#define S3C_IISMOD_MSTPCLK		(0<<10)
#define S3C_IISMOD_MSTCLKAUDIO	(1<<10)
#define S3C_IISMOD_SLVPCLK		(2<<10)
#define S3C_IISMOD_SLVI2SCLK	(3<<10)

#define S3C_IISMOD_CDCLKCON		(1<<12)

#define S3C_IISMOD_BLCMASK		(3<<13)
#define S3C_IISMOD_16BIT		(0<<13)
#define S3C_IISMOD_8BIT		(1<<13)
#define S3C_IISMOD_24BIT		(2<<13)

#if defined(CONFIG_SND_S3C_I2S_V40) || defined(CONFIG_SND_S3C_I2S_V40_MODULE)

#define S3C_IISMOD_SD1EN		(1<<16)
#define S3C_IISMOD_SD2EN		(1<<17)

#define S3C_IISMOD_CCD1MASK		(3<<18)
#define S3C_IISMOD_CCD1ND		(0<<18)
#define S3C_IISMOD_CCD11STD		(1<<18)
#define S3C_IISMOD_CCD12NDD		(2<<18)

#define S3C_IISMOD_CCD2MASK		(3<<20)
#define S3C_IISMOD_CCD2ND		(0<<20)
#define S3C_IISMOD_CCD21STD		(1<<20)
#define S3C_IISMOD_CCD22NDD		(2<<20)

#endif

#define S3C_IISFIC_FRXCNTMSK	(0xf<<0)
#define S3C_IISFIC_RFLUSH		(1<<7)
#define S3C_IISFIC_FTX0CNTMSK	(0xf<<8)
#define S3C_IISFIC_TFLUSH		(1<<15)
#if defined(CONFIG_SND_S3C_I2S_V40) || defined(CONFIG_SND_S3C_I2S_V40_MODULE)
#define S3C_IISFIC_FTX1CNTMSK	(0xf<<16)
#define S3C_IISFIC_FTX2CNTMSK	(0xf<<24)
#endif

#define S3C_IISPSR_PSVALA		(0x3f<<8)
#define S3C_IISPSR_PSRAEN		(1<<15)

/* clock sources */
#define S3C_CLKSRC_PCLK		S3C_IISMOD_MSTPCLK
#define S3C_CLKSRC_CLKAUDIO	S3C_IISMOD_MSTCLKAUDIO
#define S3C_CLKSRC_SLVPCLK	S3C_IISMOD_SLVPCLK
#define S3C_CLKSRC_I2SEXT	S3C_IISMOD_SLVI2SCLK
#define S3C_CDCLKSRC_INT	(4<<10)
#define S3C_CDCLKSRC_EXT	(5<<10)

#ifndef S3C64XX_PA_IIS1
#define S3C64XX_PA_IIS0	   	(0x7F002000)
#define S3C64XX_PA_IIS1	   	(0x7F003000)		/*add by cefanty for i2S bus1*/
#endif
#if defined(CONFIG_CPU_S3C6410)
#ifndef CONFIG_SND_S3C64XX_I2S_BUS1
#define S3C_PA_IIS_V32		S3C64XX_PA_IIS0
#else
#define S3C_PA_IIS_V32		S3C64XX_PA_IIS1
#endif
#define S3C_PA_IIS_V40		S3C64XX_PA_IIS_V40
#define IRQ_S3C_IISV32		IRQ_S3C6410_IIS
#define IRQ_S3C_IISV40		IRQ_S3C6410_IIS
#elif defined(CONFIG_CPU_S5P6440)
#define S3C_PA_IIS_V40		S5P64XX_PA_IIS_V40
#define IRQ_S3C_IISV40		IRQ_IISV40
#elif defined(CONFIG_CPU_S5PC100)
#define S3C_PA_IIS_V32		S5PC1XX_PA_IIS_V32
#define S3C_PA_IIS_V50		S5PC1XX_PA_IIS_V50
#define IRQ_S3C_IISV32		IRQ_I2S1
#define IRQ_S3C_IISV50		IRQ_I2S0
#endif

#if defined(CONFIG_SND_S3C_I2S_V32) || defined(CONFIG_SND_S3C_I2S_V32_MODULE)

#ifndef CONFIG_SND_S3C64XX_I2S_BUS1
#define S3C_DMACH_I2S_OUT	DMACH_I2S_OUT
#define S3C_DMACH_I2S_IN	DMACH_I2S_IN
#else
#define S3C_DMACH_I2S_OUT	DMACH_I2S1_OUT
#define S3C_DMACH_I2S_IN	DMACH_I2S1_IN
#endif
#define S3C_IIS_PABASE		S3C_PA_IIS_V32
#define S3C_IISIRQ		IRQ_S3C_IISV32
#ifndef CONFIG_SND_S3C64XX_I2S_BUS1
#define PCLKCLK			"i2s0_v32"
#else
#define PCLKCLK			"i2s1_v32"
#endif
#if defined(CONFIG_CPU_S5PC100)
#define EXTCLK			"sclk_audio1"
#else
#define EXTCLK			"sclk_audio0"
#endif
#define PLBK_CHAN		2
#define S3C_DESC		"S3C AP I2S-V3.2 Interface"

#elif defined(CONFIG_SND_S3C_I2S_V40) || defined(CONFIG_SND_S3C_I2S_V40_MODULE)

#define S3C_DMACH_I2S_OUT	DMACH_I2S_V40_OUT
#define S3C_DMACH_I2S_IN	DMACH_I2S_V40_IN
#define S3C_IIS_PABASE		S3C_PA_IIS_V40
#define S3C_IISIRQ		IRQ_S3C_IISV40
#define PCLKCLK			"i2s_v40"
#define EXTCLK			"sclk_audio2"
#define PLBK_CHAN		6
#define S3C_DESC		"S3C AP I2S-V4.0 Interface"

#elif defined(CONFIG_SND_S3C_I2S_V50) || defined(CONFIG_SND_S3C_I2S_V50_MODULE)

#define S3C_DMACH_I2S_OUT	DMACH_I2S_V50_OUT
#define S3C_DMACH_I2S_IN	DMACH_I2S_V50_IN
#define S3C_IIS_PABASE		S3C_PA_IIS_V50
#define S3C_IISIRQ		IRQ_S3C_IISV50
#define PCLKCLK			"i2s_v50"
#define EXTCLK			"sclk_audio0"
#define PLBK_CHAN		6
#define S3C_DESC		"S3C AP I2S-V5.0 Interface"

#endif

u32 s3c_i2s_get_clockrate(void);

extern struct snd_soc_dai s3c_i2s_dai;

#endif /*S3C_I2S_H_*/
