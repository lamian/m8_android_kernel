/* linux/arch/arm/mach-s3c6410/dma.c
 *
 * Copyright (c) 2003-2005,2006 Samsung Electronics
 *
 * S3C6410 DMA selection
 *
 * http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysdev.h>
#include <linux/serial_core.h>

#include <asm/dma.h>
#include <mach/dma.h>
#include <asm/io.h>

#include <plat/dma.h>
#include <plat/cpu.h>


/* DMAC-0 */
#define MAP0(x) { \
		[0]	= (x) | DMA_CH_VALID,	\
		[1]	= (x) | DMA_CH_VALID,	\
		[2]	= (x) | DMA_CH_VALID,	\
		[3]	= (x) | DMA_CH_VALID,	\
		[4]	= (x) | DMA_CH_VALID,	\
		[5]     = (x) | DMA_CH_VALID,	\
		[6]	= (x) | DMA_CH_VALID,	\
		[7]     = (x) | DMA_CH_VALID,	\
		[8]	= (x),	\
		[9]	= (x),	\
		[10]	= (x),	\
		[11]	= (x),	\
		[12]	= (x),	\
		[13]    = (x),	\
		[14]	= (x),	\
		[15]    = (x),	\
		[16]	= (x),	\
		[17]	= (x),	\
		[18]	= (x),	\
		[19]	= (x),	\
		[20]	= (x),	\
		[21]    = (x),	\
		[22]	= (x),	\
		[23]    = (x),	\
		[24]	= (x),	\
		[25]	= (x),	\
		[26]	= (x),	\
		[27]	= (x),	\
		[28]	= (x),	\
		[29]    = (x),	\
		[30]	= (x),	\
		[31]    = (x),	\
	}

/* DMAC-1 */
#define MAP1(x) { \
		[0]	= (x),	\
		[1]	= (x),	\
		[2]	= (x),	\
		[3]	= (x),	\
		[4]	= (x),	\
		[5]     = (x),	\
		[6]	= (x),	\
		[7]     = (x),	\
		[8]	= (x) | DMA_CH_VALID,	\
		[9]	= (x) | DMA_CH_VALID,	\
		[10]	= (x) | DMA_CH_VALID,	\
		[11]	= (x) | DMA_CH_VALID,	\
		[12]	= (x) | DMA_CH_VALID,	\
		[13]    = (x) | DMA_CH_VALID,	\
		[14]	= (x) | DMA_CH_VALID,	\
		[15]    = (x) | DMA_CH_VALID,	\
		[16]	= (x),	\
		[17]	= (x),	\
		[18]	= (x),	\
		[19]	= (x),	\
		[20]	= (x),	\
		[21]    = (x),	\
		[22]	= (x),	\
		[23]    = (x),	\
		[24]	= (x),	\
		[25]	= (x),	\
		[26]	= (x),	\
		[27]	= (x),	\
		[28]	= (x),	\
		[29]    = (x),	\
		[30]	= (x),	\
		[31]    = (x),	\
	}

/* SDMAC-0 */
#define MAP2(x) { \
		[0]	= (x),	\
		[1]	= (x),	\
		[2]	= (x),	\
		[3]	= (x),	\
		[4]	= (x),	\
		[5]     = (x),	\
		[6]	= (x),	\
		[7]     = (x),	\
		[8]	= (x),	\
		[9]	= (x),	\
		[10]	= (x),	\
		[11]	= (x),	\
		[12]	= (x),	\
		[13]    = (x),	\
		[14]	= (x),	\
		[15]    = (x),	\
		[16]	= (x) | DMA_CH_VALID,	\
		[17]	= (x) | DMA_CH_VALID,	\
		[18]	= (x) | DMA_CH_VALID,	\
		[19]	= (x) | DMA_CH_VALID,	\
		[20]	= (x) | DMA_CH_VALID,	\
		[21]    = (x) | DMA_CH_VALID,	\
		[22]	= (x) | DMA_CH_VALID,	\
		[23]    = (x) | DMA_CH_VALID,	\
		[24]	= (x),	\
		[25]	= (x),	\
		[26]	= (x),	\
		[27]	= (x),	\
		[28]	= (x),	\
		[29]    = (x),	\
		[30]	= (x),	\
		[31]    = (x),	\
	}

/* SDMAC-1 */
#define MAP3(x) { \
		[0]	= (x),	\
		[1]	= (x),	\
		[2]	= (x),	\
		[3]	= (x),	\
		[4]	= (x),	\
		[5]     = (x),	\
		[6]	= (x),	\
		[7]     = (x),	\
		[8]	= (x),	\
		[9]	= (x),	\
		[10]	= (x),	\
		[11]	= (x),	\
		[12]	= (x),	\
		[13]    = (x),	\
		[14]	= (x),	\
		[15]    = (x),	\
		[16]	= (x),	\
		[17]	= (x),	\
		[18]	= (x),	\
		[19]	= (x),	\
		[20]	= (x),	\
		[21]    = (x),	\
		[22]	= (x),	\
		[23]    = (x),	\
		[24]	= (x) | DMA_CH_VALID,	\
		[25]	= (x) | DMA_CH_VALID,	\
		[26]	= (x) | DMA_CH_VALID,	\
		[27]	= (x) | DMA_CH_VALID,	\
		[28]	= (x) | DMA_CH_VALID,	\
		[29]    = (x) | DMA_CH_VALID,	\
		[30]	= (x) | DMA_CH_VALID,	\
		[31]    = (x) | DMA_CH_VALID,	\
	}

/* SDMAC-0 */
#define MAP2_ONENAND(x) { \
		[0]	= (x),	\
		[1]	= (x),	\
		[2]	= (x),	\
		[3]	= (x) ,	\
		[4]	= (x),	\
		[5]     = (x),	\
		[6]	= (x),	\
		[7]     = (x),	\
		[8]	= (x),	\
		[9]	= (x),	\
		[10]	= (x),	\
		[11]	= (x),	\
		[12]	= (x),	\
		[13]    = (x),	\
		[14]	= (x),	\
		[15]    = (x),	\
		[16]	= (x),	\
		[17]	= (x),	\
		[18]	= (x) ,	\
		[19]	= (x) | DMA_CH_VALID,	\
		[20]	= (x),	\
		[21]    = (x),	\
		[22]	= (x),	\
		[23]    = (x),	\
		[24]	= (x),	\
		[25]	= (x),	\
		[26]	= (x),	\
		[27]	= (x),	\
		[28]	= (x),	\
		[29]    = (x),	\
		[30]	= (x),	\
		[31]    = (x),	\
	}


/* DMAC0 DMA request sources */
#define S3C_DMA0_UART0CH0	0
#define S3C_DMA0_UART0CH1	1
#define S3C_DMA0_UART1CH0	2
#define S3C_DMA0_UART1CH1	3
#define S3C_DMA0_ONENAND_RX	3	/* Memory to Memory DMA */
#define S3C_DMA0_UART2CH0	4
#define S3C_DMA0_UART2CH1	5
#define S3C_DMA0_UART3CH0	6
#define S3C_DMA0_UART3CH1	7
#define S3C_DMA0_PCM0_TX	8
#define S3C_DMA0_PCM0_RX	9
#define S3C_DMA0_I2S0_TX	10
#define S3C_DMA0_I2S0_RX	11
#define S3C_DMA0_SPI0_TX	12
#define S3C_DMA0_SPI0_RX	13
#define S3C_DMA0_HSI_TX		14
#define S3C_DMA0_HSI_RX		15

/* DMAC1 DMA request sources */
#define S3C_DMA1_PCM1_TX	0
#define S3C_DMA1_PCM1_RX	1
#define S3C_DMA1_I2S1_TX	2
#define S3C_DMA1_I2S1_RX	3
#define S3C_DMA1_SPI1_TX	4
#define S3C_DMA1_SPI1_RX	5
#define S3C_DMA1_AC97_PCMOUT	6
#define S3C_DMA1_AC97_PCMIN	7
#define S3C_DMA1_AC97_MICIN	8
#define S3C_DMA1_PWM		9
#define S3C_DMA1_IRDA		10
#define S3C_DMA1_EXT		11


static struct s3c_dma_map __initdata s3c6410_dma_mappings[] = {

	[DMACH_I2S_IN] = {
		.name		= "i2s0-in",
		.channels	= MAP0(S3C_DMA0_I2S0_RX),
		.hw_addr.from	= S3C_DMA0_I2S0_RX,
		.sdma_sel	= 1 << S3C_DMA0_I2S0_RX,
	},
	[DMACH_I2S_OUT] = {
		.name		= "i2s0-out",
		.channels	= MAP0(S3C_DMA0_I2S0_TX),
		.hw_addr.to	= S3C_DMA0_I2S0_TX,
		.sdma_sel	= 1 << S3C_DMA0_I2S0_TX,
	},
	[DMACH_I2S1_IN] = {
		.name		= "i2s1-in",
		.channels	= MAP1(S3C_DMA1_I2S1_RX),
		.hw_addr.from	= S3C_DMA1_I2S1_RX,
		.sdma_sel	= 1 << (S3C_DMA1_I2S1_RX+S3C_DMA1),
	},
	[DMACH_I2S1_OUT] = {
		.name		= "i2s1-out",
		.channels	= MAP1(S3C_DMA1_I2S1_TX),
		.hw_addr.to	= S3C_DMA1_I2S1_TX,
		.sdma_sel	= 1 << (S3C_DMA1_I2S1_TX+S3C_DMA1),
	},
	[DMACH_SPI0_IN] = {
		.name		= "spi0-in",
		.channels	= MAP0(S3C_DMA0_SPI0_RX),
		.hw_addr.from	= S3C_DMA0_SPI0_RX,
		.sdma_sel	= 1 << S3C_DMA0_SPI0_RX,
	},
	[DMACH_SPI0_OUT] = {
		.name		= "spi0-out",
		.channels	= MAP0(S3C_DMA0_SPI0_TX),
		.hw_addr.to	= S3C_DMA0_SPI0_TX,
		.sdma_sel	= 1 << S3C_DMA0_SPI0_TX,
	},
	[DMACH_SPI1_IN] = {
		.name		= "spi1-in",
		.channels	= MAP1(S3C_DMA1_SPI1_RX),
		.hw_addr.from	= S3C_DMA1_SPI1_RX,
		.sdma_sel	= 1 << (S3C_DMA1_SPI1_RX+S3C_DMA1),
	},
	[DMACH_SPI1_OUT] = {
		.name		= "spi1-out",
		.channels	= MAP1(S3C_DMA1_SPI1_TX),
		.hw_addr.to	= S3C_DMA1_SPI1_TX,
		.sdma_sel	= 1 << (S3C_DMA1_SPI1_TX+S3C_DMA1),
	},
	[DMACH_AC97_PCM_OUT] = {
		.name		= "ac97-pcm-out",
		.channels	= MAP1(S3C_DMA1_AC97_PCMOUT),
		.hw_addr.to	= S3C_DMA1_AC97_PCMOUT,
		.sdma_sel	= 1 << (S3C_DMA1_AC97_PCMOUT+S3C_DMA1),
	},
	[DMACH_AC97_PCM_IN] = {
		.name		= "ac97-pcm-in",
		.channels	= MAP1(S3C_DMA1_AC97_PCMIN),
		.hw_addr.from	= S3C_DMA1_AC97_PCMIN,
		.sdma_sel	= 1 << (S3C_DMA1_AC97_PCMIN+S3C_DMA1),
	},
	[DMACH_AC97_MIC_IN] = {
		.name		= "ac97-mic-in",
		.channels	= MAP1(S3C_DMA1_AC97_MICIN),
		.hw_addr.from	= S3C_DMA1_AC97_MICIN,
		.sdma_sel	= 1 << (S3C_DMA1_AC97_MICIN+S3C_DMA1),
	},
	[DMACH_ONENAND_IN] = {
		.name		= "onenand-in",
		.channels	= MAP2_ONENAND(S3C_DMA0_ONENAND_RX),
		.hw_addr.from	= S3C_DMA0_ONENAND_RX,
	},
	[DMACH_3D_M2M] = {
		.name		= "3D-M2M",
		.channels	= MAP1(S3C_DMA1_EXT),
		.hw_addr.from	= S3C_DMA1_EXT,
	},
        [DMACH_I2S_V40_IN] = {                                                           
		.name           = "i2s-v40-in",                                          
		.channels       = MAP0(S3C_DMA0_HSI_RX),                                 
		.hw_addr.from   = S3C_DMA0_HSI_RX,                                       
		.sdma_sel       = 1 << S3C_DMA0_HSI_RX,                                  
	},                                                                               
	[DMACH_I2S_V40_OUT] = {                                                          
		.name           = "i2s-v40-out",                                         
		.channels       = MAP0(S3C_DMA0_HSI_TX),                                 
		.hw_addr.to     = S3C_DMA0_HSI_TX,                                       
		.sdma_sel       = 1 << S3C_DMA0_HSI_TX,                                  
	}, 
};

static void s3c6410_dma_select(struct s3c2410_dma_chan *chan,
			       struct s3c_dma_map *map)
{
	chan->map = map;
}

static struct s3c_dma_selection __initdata s3c6410_dma_sel = {
	.select		= s3c6410_dma_select,
	.dcon_mask	= 0,
	.map		= s3c6410_dma_mappings,
	.map_size	= ARRAY_SIZE(s3c6410_dma_mappings),
};

static int __init s3c6410_dma_add(struct sys_device *sysdev)
{
	s3c_dma_init(S3C_DMA_CHANNELS, IRQ_DMA0, 0x20);
	return s3c_dma_init_map(&s3c6410_dma_sel);
}

static struct sysdev_driver s3c6410_dma_driver = {
	.add	= s3c6410_dma_add,
};

static int __init s3c6410_dma_init(void)
{
	return sysdev_driver_register(&s3c6410_sysclass, &s3c6410_dma_driver);
}

arch_initcall(s3c6410_dma_init);


