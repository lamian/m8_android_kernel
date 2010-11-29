/* linux/include/asm-arm/plat-s3c24xx/pm.h
 *
 * Copyright (c) 2004 Simtec Electronics
 *	Written by Ben Dooks, <ben@simtec.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/sysdev.h>

/* s3c2410_pm_init
 *
 * called from board at initialisation time to setup the power
 * management
*/

#ifdef CONFIG_PM

extern __init int s3c6410_pm_init(void);

#else

static inline int s3c6410_pm_init(void)
{
	return 0;
}
#endif

/* configuration for the IRQ mask over sleep */
extern unsigned long s3c_irqwake_intmask;
extern unsigned long s3c_irqwake_eintmask;

/* IRQ masks for IRQs allowed to go to sleep (see irq.c) */
extern unsigned long s3c_irqwake_intallow;
extern unsigned long s3c_irqwake_eintallow;

/* per-cpu sleep functions */

extern void (*pm_cpu_prep)(void);
extern void (*pm_cpu_sleep)(void);

/* Flags for PM Control */

extern unsigned long s3c_pm_flags;

/* from sleep.S */

extern int  s3c6410_cpu_save(unsigned long *saveblk);
extern void s3c6410_cpu_suspend(void);
extern void s3c6410_cpu_resume(void);

extern unsigned long s3c6410_sleep_save_phys;

/* sleep save info */

struct sleep_save {
	void __iomem	*reg;
	unsigned long	val;
};

struct sleep_save_phy {
	unsigned long	reg;
	unsigned long	val;
};

#define SAVE_ITEM(x) \
	{ .reg = (x) }

/* for the power saving unused IP clock list */
#define S3C_SCLK_MASK	~(S3C_CLKCON_SCLK_CAM | \
			S3C_CLKCON_SCLK_IRDA | \
			S3C_CLKCON_SCLK_SECUR |	\
			S3C_CLKCON_SCLK_AUDIO0 | \
			S3C_CLKCON_SCLK_AUDIO1 | \
			S3C_CLKCON_SCLK_POST0 | \
			S3C6410_CLKCON_SCLK_AUDIO2 | \
			S3C_CLKCON_SCLK_POST0_27 | \
			S3C_CLKCON_SCLK_LCD27 | \
			S3C_CLKCON_SCLK_SCALER | \
			S3C_CLKCON_SCLK_SCALER27 | \
			S3C_CLKCON_SCLK_TV27 | \
			S3C_CLKCON_SCLK_DAC27 | \
			S3C_CLKCON_SCLK_SPI0 | \
			S3C_CLKCON_SCLK_SPI1 | \
			S3C_CLKCON_SCLK_SPI0_48 | \
			S3C_CLKCON_SCLK_SPI1_48 | \
			S3C_CLKCON_SCLK_MMC0 | \
			S3C_CLKCON_SCLK_MMC1 | \
			S3C_CLKCON_SCLK_MMC2 | \
			S3C_CLKCON_SCLK_MMC0_48 | \
			S3C_CLKCON_SCLK_MMC1_48 | \
			S3C_CLKCON_SCLK_MMC2_48 | \
			S3C_CLKCON_SCLK_UHOST )

#define S3C_PCLK_MASK	~(S3C_CLKCON_PCLK_UART3 | \
			S3C_CLKCON_PCLK_UART0 | \
			S3C_CLKCON_PCLK_PCM0 | \
			S3C_CLKCON_PCLK_PCM1 | \
			S3C_CLKCON_PCLK_IRDA | \
			S3C_CLKCON_PCLK_TZPC | \
			S3C_CLKCON_PCLK_AC97 | \
			S3C_CLKCON_PCLK_IIS0 | \
			S3C_CLKCON_PCLK_HSITX | \
			S3C_CLKCON_PCLK_HSIRX | \
			S3C_CLKCON_PCLK_SPI1 | \
			S3C_CLKCON_PCLK_CHIPID | \
			S3C_CLKCON_PCLK_SKEY | \
			S3C6410_CLKCON_PCLK_IIS2 )

#define S3C_HCLK_MASK	~(S3C_CLKCON_HCLK_TZIC | \
			S3C_CLKCON_HCLK_TV | \
			S3C_CLKCON_HCLK_SCALER | \
			S3C_CLKCON_HCLK_CAMIF | \
			S3C_CLKCON_HCLK_IHOST | \
			S3C_CLKCON_HCLK_DHOST | \
			S3C_CLKCON_HCLK_MDP | \
			S3C_CLKCON_HCLK_HSMMC2 | \
			S3C_CLKCON_HCLK_UHOST | \
			S3C_CLKCON_HCLK_IROM | \
			S3C_CLKCON_HCLK_SDMA2 | \
			S3C_CLKCON_HCLK_SDMA1 | \
			S3C_CLKCON_HCLK_SECUR | \
			S3C_CLKCON_HCLK_3DSE )

extern void s3c6410_pm_do_save_phy(struct sleep_save_phy *ptr, int count);
extern void s3c6410_pm_do_restore_phy(struct sleep_save_phy *ptr, int count);
extern void s3c6410_pm_do_save(struct sleep_save *ptr, int count);
extern void s3c6410_pm_do_restore(struct sleep_save *ptr, int count);
extern int domain_off_check(unsigned int config);
#ifdef CONFIG_S3C64XX_DOMAIN_GATING
extern void s3c_set_normal_cfg(unsigned int config, unsigned int flag, unsigned int ID);
extern int s3c_wait_blk_pwr_ready(unsigned int config);
#endif /* CONFIG_S3C64XX_DOMAIN_GATING */
#ifdef CONFIG_PM
extern int s3c64xx_irq_suspend(struct sys_device *dev, pm_message_t state);
extern int s3c64xx_irq_resume(struct sys_device *dev);
#else
#define s3c64xx_irq_suspend	NULL
#define s3c64xx_irq_resume	NULL
#endif
