/* linux/arch/arm/mach-s3c6410/pm.c
 *
 * Copyright (c) 2006 Samsung Electronics
 *
 *
 * S3C6410 (and compatible) Power Manager (Suspend-To-RAM) support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/sysdev.h>
#include <linux/io.h>

#include <mach/hardware.h>

#include <asm/mach-types.h>

#include <plat/regs-gpio.h>
#include <plat/regs-clock.h>
//#include <asm/plat-s3c/regs-rtc.h>
#include <plat/cpu.h>
#include <plat/pm.h>

#ifdef CONFIG_S3C2410_PM_DEBUG
extern void pm_dbg(const char *fmt, ...);
#define DBG(fmt...) pm_dbg(fmt)
#else
#define DBG(fmt...) printk(KERN_DEBUG fmt)
#endif

//static void s3c6410_cpu_suspend(void)
void s3c6410_cpu_suspend(void)
{
	unsigned long tmp;

	/* issue the standby signal into the pm unit. Note, we
	 * issue a write-buffer drain just in case */

	tmp = 0;

	asm("b 1f\n\t"
	    ".align 5\n\t"
	    "1:\n\t"
	    "mcr p15, 0, %0, c7, c10, 5\n\t"
	    "mcr p15, 0, %0, c7, c10, 4\n\t"
	    "mcr p15, 0, %0, c7, c0, 4" :: "r" (tmp));

	/* we should never get past here */

	panic("sleep resumed to originator?");
}

static void s3c6410_pm_prepare(void)
{

}

static int s3c6410_pm_add(struct sys_device *sysdev)
{
	pm_cpu_prep = s3c6410_pm_prepare;
	pm_cpu_sleep = s3c6410_cpu_suspend;

	return 0;
}

static struct sleep_save s3c6410_sleep[] = {

};

static int s3c6410_pm_resume(struct sys_device *dev)
{
	s3c6410_pm_do_restore(s3c6410_sleep, ARRAY_SIZE(s3c6410_sleep));
	return 0;
}

static struct sysdev_driver s3c6410_pm_driver = {
	.add		= s3c6410_pm_add,
	.resume		= s3c6410_pm_resume,
};

static __init int s3c6410_pm_drvinit(void)
{
	return sysdev_driver_register(&s3c6410_sysclass, &s3c6410_pm_driver);
}

arch_initcall(s3c6410_pm_drvinit);

