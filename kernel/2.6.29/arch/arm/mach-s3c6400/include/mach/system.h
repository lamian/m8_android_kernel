/* linux/arch/arm/mach-s3c6400/include/mach/system.h
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *      Ben Dooks <ben@simtec.co.uk>
 *      http://armlinux.simtec.co.uk/
 *
 * S3C6400 - system implementation
 */

#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H __FILE__

#include <linux/io.h>
#include <mach/map.h>
#include <plat/regs-watchdog.h>
#include <plat/regs-rtc.h>
#include <plat/regs-adc.h>
#include <plat/regs-clock.h>
#include <mach/gpio.h>

void (*s3c64xx_idle)(void);
void (*s3c64xx_reset_hook)(void);

void s3c64xx_default_idle(void)
{
	/* nothing here yet */
}
	
static void arch_idle(void)
{
	if (s3c64xx_idle != NULL)
		(s3c64xx_idle)();
	else
		s3c64xx_default_idle();
}

void arch_reset(char mode)
{
#if 0
	void __iomem	*wdt_reg;

	wdt_reg = ioremap(S3C64XX_PA_WATCHDOG,S3C64XX_SZ_WATCHDOG);

	/* nothing here yet */

	writel(S3C_WTCNT_CNT, wdt_reg + S3C_WTCNT_OFFSET);	/* Watchdog Count Register*/
	writel(S3C_WTCNT_CON, wdt_reg + S3C_WTCON_OFFSET);	/* Watchdog Controller Register*/
	writel(S3C_WTCNT_DAT, wdt_reg + S3C_WTDAT_OFFSET);	/* Watchdog Data Register*/

	/* wait for reset to assert... */
	mdelay(500);

	printk(KERN_ERR "Watchdog reset failed to assert reset\n");

	/* delay to allow the serial port to show the message */
	mdelay(50);
#else
	gpio_set_value(S3C64XX_GPD(4), 0);

	void __iomem *rtc_reg,  __iomem *adc_reg;
	rtc_reg = ioremap(S3C64XX_PA_RTC, 0x100);
	writel(0, rtc_reg + S3C_RTCCON);

	adc_reg = ioremap(S3C64XX_PA_ADC, 0x1000);
	writel((readl(adc_reg + S3C_ADCCON) | (1<<2)), adc_reg + S3C_ADCCON);

	__raw_writel((1<<25)|(1<<22)|(1<<21)|(1<<0), S3C_HCLK_GATE);

	__raw_writel(0x6410, S3C_SW_RST);

	while(1);

	printk(KERN_ERR "Should never be here!\n");
#endif
}

#endif /* __ASM_ARCH_IRQ_H */
