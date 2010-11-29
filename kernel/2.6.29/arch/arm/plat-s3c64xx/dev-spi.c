/* linux/arch/arm/plat-s3c64xx/dev-spi.c
 *
 * Copyright 2009 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/err.h>

#include <mach/map.h>
#include <asm/io.h>

#include <plat/spi.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/irqs.h>
#include <plat/regs-gpio.h>

#if defined(CONFIG_SPICLK_SCLK48M)
#define SPICLK "sclk_spi_48"

#elif defined(CONFIG_SPICLK_EPLL) || defined(CONFIG_SPICLK_SPIEXT)
#define SPICLK "spi_epll"

#if defined(CONFIG_SPICLK_MOUTEPLL)
#define SPICLK_SRC "mout_epll"

#elif defined(CONFIG_SPICLK_EPLL_DOUT)
#if defined (CONFIG_SPI_S5PC100)
#define SPICLK_SRC "dout_mpll2"
#else
#define SPICLK_SRC "dout_mpll"
#endif

#elif defined(CONFIG_SPICLK_EPLL_FIN)
#define SPICLK_SRC "ext_xtal"

#elif defined(CONFIG_SPICLK_EPLL_27MHZ)
#define SPICLK_SRC "clk_27m"

#elif defined(CONFIG_SPICLK_EPLL_MOUTHPLL)
#define SPICLK_SRC "mout_hpll"
#endif

#endif

#if 0
#define dbg_printk(x...)	printk(x)
#else
#define dbg_printk(x...)	do{}while(0)
#endif

#define DRVST_2MA      0
#define DRVST_4MA      1
#define DRVST_7MA      2
#define DRVST_9MA      3

#define SPICLK1DRVST_MASK  (0x3)
#define SPICLK1DRVST_SHFT  (18)
#define PORTDRVST_MASK     (0x3)
#define PORTDRVST_SHFT     (28)

/* Set drive strength of SPI'idx' to 'ma' */
static void smi_setdrvst(int idx, u16 ma)
{
	u16 prt_st, clk_st;

	prt_st = (ma >> PRT_STRN_SHFT) & PRT_STRN_MASK;
	clk_st = (ma >> CLK_STRN_SHFT) & CLK_STRN_MASK;
	dbg_printk("SPI%d Req port-strength=%dma, clock-strength=%dma\n", idx, prt_st, clk_st);

	switch (prt_st) {
		   /* Map to register field value */
	   case 12:
	   case 9:
		   prt_st = DRVST_9MA;
		   break;
	   case 8:
	   case 7:
		   prt_st = DRVST_7MA;
		   break;
	   case 4:
		   prt_st = DRVST_4MA;
		   break;
	   default:			/* any mis-match defaults to least/safest current */
		   prt_st = DRVST_2MA;
		   break;
	}

	switch (clk_st) {
		   /* Map to register field value */
	   case 12:
	   case 9:
		   clk_st = DRVST_9MA;
		   break;
	   case 8:
	   case 7:
		   clk_st = DRVST_7MA;
		   break;
	   case 4:
		   clk_st = DRVST_4MA;
		   break;
	   default:			/* any mis-match defaults to least/safest current */
		   clk_st = DRVST_2MA;
		   break;
	}

	dbg_printk("Setting Field[%d:%d] to %d\n", PORTDRVST_SHFT+1, PORTDRVST_SHFT, prt_st);
	writel((readl(S3C64XX_SPC_BASE) & 
			~(PORTDRVST_MASK << PORTDRVST_SHFT)) | (prt_st << PORTDRVST_SHFT),
			S3C64XX_SPC_BASE);

	if(idx == 1){
		dbg_printk("Setting Field[%d:%d] to %d\n", SPICLK1DRVST_SHFT+1, SPICLK1DRVST_SHFT, clk_st);
		writel((readl(S3C64XX_SPC_BASE) & 
			~(SPICLK1DRVST_MASK << SPICLK1DRVST_SHFT)) | (clk_st << SPICLK1DRVST_SHFT),
			S3C64XX_SPC_BASE);
	}
}

static int smi_getclcks(struct s3c_spi_mstr_info *smi)
{
	struct clk *cspi, *cp, *cm, *cf;

	cp = NULL;
	cm = NULL;
	cf = NULL;
	cspi = smi->clk;

	if(cspi == NULL){
		cspi = clk_get(&smi->pdev->dev, "spi");
		if(IS_ERR(cspi)){
			printk("Unable to get spi!\n");
			return -EBUSY;
		}
	}
	dbg_printk("%s:%d Got clk=spi\n", __func__, __LINE__);

#if defined(CONFIG_SPICLK_SCLK48M) || defined(CONFIG_SPICLK_EPLL) || defined(CONFIG_SPICLK_SPIEXT)
	cp = clk_get(&smi->pdev->dev, SPICLK);
	if(IS_ERR(cp)){
		printk("Unable to get parent clock(%s)!\n", SPICLK);
		if(smi->clk == NULL){
			clk_disable(cspi);
			clk_put(cspi);
		}
		return -EBUSY;
	}
	dbg_printk("%s:%d Got clk=%s\n", __func__, __LINE__, SPICLK);

#if defined(CONFIG_SPICLK_EPLL) || defined(CONFIG_SPICLK_SPIEXT)
	cm = clk_get(&smi->pdev->dev, SPICLK_SRC);
	if(IS_ERR(cm)){
		printk("Unable to get %s\n", SPICLK_SRC);
		clk_put(cp);
		return -EBUSY;
	}
	dbg_printk("%s:%d Got clk=%s\n", __func__, __LINE__, SPICLK_SRC);
	if(clk_set_parent(cp, cm)){
		printk("failed to set %s as the parent of %s\n", SPICLK_SRC, SPICLK);
		clk_put(cm);
		clk_put(cp);
		return -EBUSY;
	}
	dbg_printk("Set %s as the parent of %s\n", SPICLK_SRC, SPICLK);

#if defined(CONFIG_SPICLK_EPLL_MOUTEPLL) /* MOUTepll through EPLL */
	cf = clk_get(&smi->pdev->dev, "fout_epll");
	if(IS_ERR(cf)){
		printk("Unable to get fout_epll\n");
		clk_put(cm);
		clk_put(cp);
		return -EBUSY;
	}
	dbg_printk("Got fout_epll\n");
	if(clk_set_parent(cm, cf)){
		printk("failed to set FOUTepll as parent of %s\n", SPICLK_SRC);
		clk_put(cf);
		clk_put(cm);
		clk_put(cp);
		return -EBUSY;
	}
	dbg_printk("Set FOUTepll as parent of %s\n", SPICLK_SRC);
	clk_put(cf);
#endif
	clk_put(cm);
#endif

	smi->prnt_clk = cp;
#endif

	smi->clk = cspi;
	return 0;
}

static void smi_putclcks(struct s3c_spi_mstr_info *smi)
{
	if(smi->prnt_clk != NULL)
		clk_put(smi->prnt_clk);

	clk_put(smi->clk);
}

static int smi_enclcks(struct s3c_spi_mstr_info *smi)
{
	if(smi->prnt_clk != NULL)
		clk_enable(smi->prnt_clk);

	return clk_enable(smi->clk);
}

static void smi_disclcks(struct s3c_spi_mstr_info *smi)
{
	if(smi->prnt_clk != NULL)
		clk_disable(smi->prnt_clk);

	clk_disable(smi->clk);
}

static u32 smi_getrate(struct s3c_spi_mstr_info *smi)
{
	if(smi->prnt_clk != NULL)
		return clk_get_rate(smi->prnt_clk);
	else
		return clk_get_rate(smi->clk);
}

static int smi_setrate(struct s3c_spi_mstr_info *smi, u32 r)
{
 /* We don't take charge of the Src Clock, yet */
	return 0;
}

/* SPI (0) */
static struct resource s3c_spi0_resource[] = {
	[0] = {
		.start = S3C_PA_SPI0,
		.end   = S3C_PA_SPI0 + S3C_SZ_SPI0 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SPI0,
		.end   = IRQ_SPI0,
		.flags = IORESOURCE_IRQ,
	}
};

static struct s3c_spi_mstr_info sspi0_mstr_info = {
	.pdev = NULL,
	.clk = NULL,
	.prnt_clk = NULL,
	.num_slaves = 0,
	.set_drvst = smi_setdrvst,
	.spiclck_get = smi_getclcks,
	.spiclck_put = smi_putclcks,
	.spiclck_en = smi_enclcks,
	.spiclck_dis = smi_disclcks,
	.spiclck_setrate = smi_setrate,
	.spiclck_getrate = smi_getrate,
};

static u64 s3c_device_spi0_dmamask = 0xffffffffUL;

struct platform_device s3c_device_spi0 = {
	.name		= "s3c-spi",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(s3c_spi0_resource),
	.resource	= s3c_spi0_resource,
	.dev		= {
		.dma_mask = &s3c_device_spi0_dmamask,
		.coherent_dma_mask = 0xffffffffUL,
		.platform_data = &sspi0_mstr_info,
	}
};
EXPORT_SYMBOL(s3c_device_spi0);

/* SPI (1) */
static struct resource s3c_spi1_resource[] = {
	[0] = {
		.start = S3C_PA_SPI1,
		.end   = S3C_PA_SPI1 + S3C_SZ_SPI1 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SPI1,
		.end   = IRQ_SPI1,
		.flags = IORESOURCE_IRQ,
	}
};

static struct s3c_spi_mstr_info sspi1_mstr_info = {
	.pdev = NULL,
	.clk = NULL,
	.prnt_clk = NULL,
	.num_slaves = 0,
	.set_drvst = smi_setdrvst,
	.spiclck_get = smi_getclcks,
	.spiclck_put = smi_putclcks,
	.spiclck_en = smi_enclcks,
	.spiclck_dis = smi_disclcks,
	.spiclck_setrate = smi_setrate,
	.spiclck_getrate = smi_getrate,
};

static u64 s3c_device_spi1_dmamask = 0xffffffffUL;

struct platform_device s3c_device_spi1 = {
	.name		= "s3c-spi",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(s3c_spi1_resource),
	.resource	= s3c_spi1_resource,
	.dev		= {
		.dma_mask = &s3c_device_spi1_dmamask,
		.coherent_dma_mask = 0xffffffffUL,
		.platform_data = &sspi1_mstr_info,
	}
};
EXPORT_SYMBOL(s3c_device_spi1);

void __init s3c_spi_set_slaves(unsigned id, int n, struct s3c_spi_pdata const *dat)
{
	struct s3c_spi_mstr_info *pinfo;

	if(id == 0)
	   pinfo = (struct s3c_spi_mstr_info *)s3c_device_spi0.dev.platform_data;
	else if(id == 1)
	   pinfo = (struct s3c_spi_mstr_info *)s3c_device_spi1.dev.platform_data;
	else
	   return;

	pinfo->spd = kmalloc(n * sizeof (*dat), GFP_KERNEL);
	if(!pinfo->spd)
	   return;
	memcpy(pinfo->spd, dat, n * sizeof(*dat));

	pinfo->num_slaves = n;
}
