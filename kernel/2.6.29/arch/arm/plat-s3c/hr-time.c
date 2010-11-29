/*
 * linux/arch/arm/plat-s3c/hr-time.c
 *
 * SEC Timers
 *
 * Copyright (c) 2006 Samsung Electronics
 *
 *
 * SEC (and compatible) Power Manager (Suspend-To-RAM) support
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/io.h>

#include <asm/system.h>
#include <mach/hardware.h>
#include <asm/leds.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <asm/mach/time.h>
#include <asm/mach-types.h>
#include <mach/map.h>
#include <plat/regs-timer.h>
#include <mach/regs-irq.h>
#include <mach/tick.h>

#include <plat/clock.h>
#include <plat/cpu.h>


static void sec_timer_setup(void);

static inline void sec_tick_set_autoreset(void)
{
	unsigned long tcon;

	tcon  = __raw_readl(S3C_TCON);
	tcon |= (1<<22);
	__raw_writel(tcon, S3C_TCON);
}

static inline void sec_tick_remove_autoreset(void)
{
	unsigned long tcon;	

	tcon  = __raw_readl(S3C_TCON);
	tcon &= ~(1<<22);
	__raw_writel(tcon, S3C_TCON);
}

static void sec_tick_timer_start(unsigned long load_val,
					int autoreset)
{
	unsigned long tcon;
	unsigned long tcnt;
	unsigned long tcfg1;
	unsigned long tcfg0;
	unsigned long tcstat;

	tcon  = __raw_readl(S3C_TCON);
	tcfg1 = __raw_readl(S3C_TCFG1);
	tcfg0 = __raw_readl(S3C_TCFG0);

	tcstat = __raw_readl(S3C_TINT_CSTAT);
	tcstat |=  0x10;
	__raw_writel(tcstat, S3C_TINT_CSTAT);

	tcnt = load_val;
	tcnt--;
	__raw_writel(tcnt, S3C_TCNTB(4));

	tcfg1 &= ~S3C_TCFG1_MUX4_MASK;
	tcfg1 |= S3C_TCFG1_MUX4_DIV1;

	tcfg0 &= ~S3C_TCFG_PRESCALER1_MASK;
	tcfg0 |= (0) << S3C_TCFG_PRESCALER1_SHIFT;
		
	__raw_writel(tcfg1, S3C_TCFG1);
	__raw_writel(tcfg0, S3C_TCFG0);

	tcon &= ~(7<<20);
	
	tcon |= S3C_TCON_T4MANUALUPD;

	if (autoreset) 
		tcon |= S3C_TCON_T4RELOAD;

	__raw_writel(tcon, S3C_TCON);

	
	/* start the timer running */
	tcon |= S3C_TCON_T4START;
	tcon &= ~S3C_TCON_T4MANUALUPD;
	__raw_writel(tcon, S3C_TCON);

}

static inline void sec_tick_timer_stop(void)
{
	unsigned long tcon;

	tcon  = __raw_readl(S3C_TCON);
	tcon &= ~(1<<20);
	__raw_writel(tcon, S3C_TCON);
}

static void sec_sched_timer_start(unsigned long load_val,
					int autoreset)
{
	unsigned long tcon;
	unsigned long tcnt;
	unsigned long tcfg1;
	unsigned long tcfg0;
	unsigned long tcstat;

	tcstat = __raw_readl(S3C_TINT_CSTAT);
	tcstat |=  0x04;
	__raw_writel(tcstat, S3C_TINT_CSTAT);

	tcon  = __raw_readl(S3C_TCON);
	tcfg1 = __raw_readl(S3C_TCFG1);
	tcfg0 = __raw_readl(S3C_TCFG0);
		
	tcnt = load_val;
	tcnt--;
	__raw_writel(tcnt, S3C_TCNTB(2));
	__raw_writel(tcnt, S3C_TCMPB(2));	
	
	tcon &= ~(0x0b<<12);
	
	if (autoreset) 
		tcon |= S3C_TCON_T2RELOAD;

	tcon |= S3C_TCON_T2MANUALUPD;

	__raw_writel(tcon, S3C_TCON);
	

	
	/* start the timer running */
	tcon |= S3C_TCON_T2START;
	tcon &= ~S3C_TCON_T2MANUALUPD;
	__raw_writel(tcon, S3C_TCON);

}

/*
 * ---------------------------------------------------------------------------
 * PWM timer 4 ... count down to zero, interrupt, reload
 * ---------------------------------------------------------------------------
 */
static int sec_tick_set_next_event(unsigned long cycles,
				   struct clock_event_device *evt)
{
	sec_tick_timer_start(cycles, 0);
	return 0;
}
 
static void sec_tick_set_mode(enum clock_event_mode mode,
			      struct clock_event_device *evt)
{
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		sec_tick_set_autoreset();
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		sec_tick_timer_stop();
		sec_tick_remove_autoreset();
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		break;
	case CLOCK_EVT_MODE_RESUME:
		sec_timer_setup();
		break;
	}
}

static struct clock_event_device clockevent_tick_timer = {
	.name		= "pwm_timer4",
	.features       = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.shift		= 32,
	.set_next_event	= sec_tick_set_next_event,
	.set_mode	= sec_tick_set_mode,
};

irqreturn_t sec_tick_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &clockevent_tick_timer;

	evt->event_handler(evt);

	return IRQ_HANDLED;
}

static struct irqaction sec_tick_timer_irq = {
	.name		= "pwm_timer4",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= sec_tick_timer_interrupt,
};

static void __init  sec_init_dynamic_tick_timer(unsigned long rate)
{
	sec_tick_timer_start((rate / HZ) - 1, 1);

	clockevent_tick_timer.mult = div_sc(rate, NSEC_PER_SEC,
					    clockevent_tick_timer.shift);
	clockevent_tick_timer.max_delta_ns =
		clockevent_delta2ns(-1, &clockevent_tick_timer);
	clockevent_tick_timer.min_delta_ns =
		clockevent_delta2ns(1, &clockevent_tick_timer);

	clockevent_tick_timer.cpumask = cpumask_of(0);
	clockevents_register_device(&clockevent_tick_timer);
}


/*
 * ---------------------------------------------------------------------------
 * PWM timer 2 ... free running 32-bit clock source and scheduler clock
 * ---------------------------------------------------------------------------
 */
 
static unsigned long sec_mpu_timer2_overflows;

irqreturn_t sec_mpu_timer2_interrupt(int irq, void *dev_id)
{
	sec_mpu_timer2_overflows++;

	return IRQ_HANDLED;
}

struct irqaction sec_timer2_irq = {
	.name		= "pwm_timer2",
	.flags		= IRQF_DISABLED ,
	.handler	= sec_mpu_timer2_interrupt,
};


static cycle_t sec_sched_timer_read(void)
{

	return (cycle_t)~__raw_readl(S3C_TIMERREG(0x2c));
}

struct clocksource clocksource_sec= {
	.name		= "clock_source_timer2",
	.rating		= 300,
	.read		= sec_sched_timer_read,
	.mask		= CLOCKSOURCE_MASK(32),
	.shift		= 20,
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS ,
};

/*
 *  Rounds down to nearest nsec.
 */
unsigned long long sec_ticks_to_nsecs(unsigned long ticks)
{
        return cyc2ns(&clocksource_sec, ticks);
}

/*
 * Returns current time from boot in nsecs. It's OK for this to wrap
 * around for now, as it's just a relative time stamp.
 */
unsigned long long sched_clock(void)
{
	static int first = 1;
	static cycle_t saved_ticks;
	static int saved_ticks_valid;
	static unsigned long long base;
	static unsigned long long last_result;

	unsigned long irq_flags;
	static cycle_t last_ticks;
	cycle_t ticks;
	static unsigned long long result;

	local_irq_save(irq_flags);

	last_ticks = saved_ticks;
	saved_ticks = ticks = sec_sched_timer_read();

	if(!saved_ticks_valid)
	{
		saved_ticks_valid = 1;
		last_ticks = ticks;
		base -= cyc2ns(&clocksource_sec, ticks);
	}
	if(ticks < last_ticks)
	{
		if (first)
			first = 0;
		else
		{
			base += cyc2ns(&clocksource_sec, clocksource_sec.mask);
			base += cyc2ns(&clocksource_sec, 1);
		}
	}

	last_result = result = cyc2ns(&clocksource_sec, ticks) + base;

	local_irq_restore(irq_flags);

	return result;
}

static void __init sec_init_clocksource(unsigned long rate)
{
	static char err[] __initdata = KERN_ERR
			"%s: can't register clocksource!\n";

	clocksource_sec.mult
		= clocksource_khz2mult(rate/1000, clocksource_sec.shift);

	sec_sched_timer_start(~0, 1);

	if (clocksource_register(&clocksource_sec))
		printk(err, clocksource_sec.name);
}

/*
 * ---------------------------------------------------------------------------
 *  Tick Timer initialization
 * ---------------------------------------------------------------------------
 */
static void sec_dynamic_timer_setup(void)
{
	struct clk	*ck_ref = clk_get(NULL, "timers");
	unsigned long	rate;

	if (IS_ERR(ck_ref))
		panic("failed to get clock for system timer");

	rate = clk_get_rate(ck_ref);
	clk_put(ck_ref);

	sec_init_dynamic_tick_timer(rate);
	sec_init_clocksource(rate);

}


static void sec_timer_setup(void)
{
	struct clk	*ck_ref = clk_get(NULL, "timers");
	unsigned long	rate;

	
	if (IS_ERR(ck_ref))
		panic("failed to get clock for system timer");

	rate = clk_get_rate(ck_ref);
	clk_put(ck_ref);

	sec_tick_timer_start((rate / HZ) - 1, 1);
	sec_sched_timer_start(~0, 1);

}

	
static void __init sec_dynamic_timer_init(void)
{
	sec_dynamic_timer_setup();
	setup_irq(IRQ_TIMER2, &sec_timer2_irq);	
	setup_irq(IRQ_TIMER4, &sec_tick_timer_irq);
}


struct sys_timer sec_timer = {
	.init		= sec_dynamic_timer_init,
};

