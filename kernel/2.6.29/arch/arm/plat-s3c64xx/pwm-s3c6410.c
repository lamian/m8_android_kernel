/* arch/arm/plat-s3c64xx/pwm-s3c6410.c
 *
 * (c) 2003-2005 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * S3C64XX PWM core
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Changelog:
 * This file is based on the Sangwook Lee/Samsung patches, re-written due
 * to various ommisions from the code (such as flexible pwm configuration)
 * for use with the BAST system board.
 *
 *
 */
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/mutex.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-cfg.h>

#include <plat/regs-timer.h>
#include <mach/regs-irq.h>
#include "pwm-s3c6410.h"

#define PRESCALER	4

s3c6410_pwm_chan_t s3c_chans[S3C_PWM_CHANNELS];

static inline void
s3c6410_pwm_buffdone(s3c6410_pwm_chan_t *chan, void *dev)
{

	if (chan->callback_fn != NULL) {
		(chan->callback_fn)( dev);
	}
}


static int s3c6410_pwm_start (int channel)
{
	unsigned long tcon;
	tcon = __raw_readl(S3C_TCON);
	switch(channel)
	{
	case 0:
		tcon |= S3C_TCON_T0START;
		tcon &= ~S3C_TCON_T0MANUALUPD;
	break;
	case 1:
		tcon |= S3C_TCON_T1START;
		tcon &= ~S3C_TCON_T1MANUALUPD;
	break;
	case 2:
		tcon |= S3C_TCON_T2START;
		tcon &= ~S3C_TCON_T2MANUALUPD;
	break;
	case 3:
		tcon |= S3C_TCON_T3START;
		tcon &= ~S3C_TCON_T3MANUALUPD;
	break;

	}
	__raw_writel(tcon, S3C_TCON);

	return 0;
}


int s3c6410_timer_setup (int channel, int usec, unsigned long g_tcnt, unsigned long g_tcmp)
{
	unsigned long tcon;
	unsigned long tcnt;
	unsigned long tcmp;
	unsigned long tcfg1;
	unsigned long tcfg0;
	unsigned long pclk;
	struct clk *clk;

	//printk("\nPWM channel %d set g_tcnt = %ld, g_tcmp = %ld \n", channel, g_tcnt, g_tcmp);

	tcnt = 0xffffffff;  /* default value for tcnt */

	/* read the current timer configuration bits */
	tcon = __raw_readl(S3C_TCON);
	tcfg1 = __raw_readl(S3C_TCFG1);
	tcfg0 = __raw_readl(S3C_TCFG0);

	clk = clk_get(NULL, "timers");
	if (IS_ERR(clk))
		panic("failed to get clock for pwm timer");

	clk_enable(clk);

	pclk = clk_get_rate(clk);

	/* configure clock tick */
	switch(channel)
	{
		case 0:
			/* set gpio as PWM TIMER0 to signal output*/
			s3c_gpio_cfgpin(S3C64XX_GPF(14), S3C64XX_GPF14_PWM_TOUT0);
			s3c_gpio_setpull(S3C64XX_GPF(14), S3C_GPIO_PULL_NONE);
			tcfg1 &= ~S3C_TCFG1_MUX0_MASK;
			tcfg1 |= S3C_TCFG1_MUX0_DIV2;

			tcfg0 &= ~S3C_TCFG_PRESCALER0_MASK;
			tcfg0 |= (PRESCALER) << S3C_TCFG_PRESCALER0_SHIFT;
			tcon &= ~(7<<0);
			tcon |= S3C_TCON_T0RELOAD;
			break;

		case 1:
			/* set gpio as PWM TIMER1 to signal output*/
			s3c_gpio_cfgpin(S3C64XX_GPF(15), S3C64XX_GPF15_PWM_TOUT1);
			s3c_gpio_setpull(S3C64XX_GPF(15), S3C_GPIO_PULL_NONE);
			tcfg1 &= ~S3C_TCFG1_MUX1_MASK;
			tcfg1 |= S3C_TCFG1_MUX1_DIV2;

			tcfg0 &= ~S3C_TCFG_PRESCALER0_MASK;
			tcfg0 |= (PRESCALER) << S3C_TCFG_PRESCALER0_SHIFT;

			tcon &= ~(7<<8);
			tcon |= S3C_TCON_T1RELOAD;
			break;
		case 2:
			tcfg1 &= ~S3C_TCFG1_MUX2_MASK;
			tcfg1 |= S3C_TCFG1_MUX2_DIV2;

			tcfg0 &= ~S3C_TCFG_PRESCALER1_MASK;
			tcfg0 |= (PRESCALER) << S3C_TCFG_PRESCALER1_SHIFT;

			tcon &= ~(7<<12);
			tcon |= S3C_TCON_T2RELOAD;
			break;
		case 3:
			tcfg1 &= ~S3C_TCFG1_MUX3_MASK;
			tcfg1 |= S3C_TCFG1_MUX3_DIV2;

			tcfg0 &= ~S3C_TCFG_PRESCALER1_MASK;
			tcfg0 |= (PRESCALER) << S3C_TCFG_PRESCALER1_SHIFT;
			tcon &= ~(7<<16);
			tcon |= S3C_TCON_T3RELOAD;
			break;
	}

	__raw_writel(tcfg1, S3C_TCFG1);
	__raw_writel(tcfg0, S3C_TCFG0);


	__raw_writel(tcon, S3C_TCON);
	tcnt = 160;
	__raw_writel(tcnt, S3C_TCNTB(channel));
	tcmp = 110;
	__raw_writel(tcmp, S3C_TCMPB(channel));

	switch(channel)
	{
		case 0:
			tcon |= S3C_TCON_T0MANUALUPD;
			break;
		case 1:
			tcon |= S3C_TCON_T1MANUALUPD;
			break;
		case 2:
			tcon |= S3C_TCON_T2MANUALUPD;
			break;
		case 3:
			tcon |= S3C_TCON_T3MANUALUPD;
			break;
	}
	__raw_writel(tcon, S3C_TCON);

	tcnt = g_tcnt;
	__raw_writel(tcnt, S3C_TCNTB(channel));

	tcmp = g_tcmp;
	__raw_writel(tcmp, S3C_TCMPB(channel));

	/* start the timer running */
	s3c6410_pwm_start ( channel);

	return 0;
}

EXPORT_SYMBOL(s3c6410_timer_setup);

static irqreturn_t s3c6410_pwm_irq(int irq, void *devpw)
{
	s3c6410_pwm_chan_t *chan = (s3c6410_pwm_chan_t *)devpw;
	void *dev=chan->dev;

	/* modify the channel state */
	s3c6410_pwm_buffdone(chan, dev);

	return IRQ_HANDLED;
}


int s3c6410_pwm_request(pwmch_t  channel, s3c_pwm_client_t *client, void *dev)
{
	s3c6410_pwm_chan_t *chan = &s3c_chans[channel];
	unsigned long flags;
	int err;

	pr_debug("pwm%d: s3c_request_pwm: client=%s, dev=%p\n",
		 channel, client->name, dev);


	local_irq_save(flags);


	if (chan->in_use) {
		if (client != chan->client) {
			printk(KERN_ERR "pwm%d: already in use\n", channel);
			local_irq_restore(flags);
			return -EBUSY;
		} else {
			printk(KERN_ERR "pwm%d: client already has channel\n", channel);
		}
	}

	chan->client = client;
	chan->in_use = 1;
	chan->dev = dev;

	if (!chan->irq_claimed) {
		pr_debug("pwm%d: %s : requesting irq %d\n",
			 channel, __FUNCTION__, chan->irq);

		err = request_irq(chan->irq, s3c6410_pwm_irq, IRQF_DISABLED,
				  client->name, (void *)chan);

		if (err) {
			chan->in_use = 0;
			local_irq_restore(flags);

			printk(KERN_ERR "%s: cannot get IRQ %d for PWM %d\n",
			       client->name, chan->irq, chan->number);
			return err;
		}

		chan->irq_claimed = 1;
		chan->irq_enabled = 1;
	}

	local_irq_restore(flags);

	/* need to setup */

	pr_debug("%s: channel initialised, %p\n", __FUNCTION__, chan);

	return 0;
}

int s3c6410_pwm_free (pwmch_t channel, s3c_pwm_client_t *client)
{
	s3c6410_pwm_chan_t *chan = &s3c_chans[channel];
	unsigned long flags;


	local_irq_save(flags);

	if (chan->client != client) {
		printk(KERN_WARNING "pwm%d: possible free from different client (channel %p, passed %p)\n",
		       channel, chan->client, client);
	}

	/* sort out stopping and freeing the channel */


	chan->client = NULL;
	chan->in_use = 0;

	if (chan->irq_claimed)
		free_irq(chan->irq, (void *)chan);
	chan->irq_claimed = 0;

	local_irq_restore(flags);

	return 0;
}


int s3c6410_pwm_set_buffdone_fn(pwmch_t channel, s3c_pwm_cbfn_t rtn)
{
	s3c6410_pwm_chan_t *chan = &s3c_chans[channel];


	pr_debug("%s: chan=%d, callback rtn=%p\n", __FUNCTION__, channel, rtn);

	chan->callback_fn = rtn;

	return 0;
}


#define s3c6410_pwm_suspend NULL
#define s3c6410_pwm_resume  NULL

struct sysdev_class pwm_sysclass = {
	.name		= "s3c-pwm",
	.suspend	= s3c6410_pwm_suspend,
	.resume		= s3c6410_pwm_resume,
};


/* initialisation code */

static int __init s3c6410_init_pwm(void)
{
	s3c6410_pwm_chan_t *cp;
	int channel;
	int ret;

	printk("S3C PWM Driver, (c) 2006-2007 Samsung Electronics\n");

	ret = sysdev_class_register(&pwm_sysclass);
	if (ret != 0) {
		printk(KERN_ERR "pwm sysclass registration failed\n");
		return -ENODEV;
	}

	for (channel = 0; channel < S3C_PWM_CHANNELS; channel++) {
		cp = &s3c_chans[channel];

		memset(cp, 0, sizeof(s3c6410_pwm_chan_t));

		cp->number = channel;
		/* pwm channel irqs are in order.. */
		cp->irq    = channel + IRQ_TIMER0;

		/* register system device */

		ret = sysdev_register(&cp->sysdev);

		pr_debug("PWM channel %d , irq %d\n",
		       cp->number,  cp->irq);
	}

	return ret;
}
__initcall(s3c6410_init_pwm);
