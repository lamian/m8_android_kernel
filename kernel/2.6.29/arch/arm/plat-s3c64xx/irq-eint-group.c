/* arch/arm/plat-s3c64xx/irq-eint-group.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *      Ben Dooks <ben@simtec.co.uk>
 *      http://armlinux.simtec.co.uk/
 *
 * S3C64XX - Interrupt handling for IRQ_EINT(x)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>

#include <asm/hardware/vic.h>

#include <plat/regs-irqtype.h>

#include <mach/map.h>
#include <plat/cpu.h>

#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/gpio-bank-n.h>

#define S3C64XX_EINT_MAX_SOURCES	16

struct s3c_eint_group_t {
	int		sources;
	int		base;
	void __iomem	*cont_reg;
	void __iomem	*mask_reg;
	void __iomem	*pend_reg;
	int		mask_ofs;
	int		pend_ofs;

	/* start offset in control register for each source */
	int		cont_map[S3C64XX_EINT_MAX_SOURCES];
};

static struct s3c_eint_group_t eint_groups[] = {
	[0] = {
		.sources	= 0,
		.base		= 0,
		.cont_reg	= 0x0,
		.mask_reg	= 0x0,
		.pend_reg	= 0x0,
		.mask_ofs	= 0,
		.pend_ofs	= 0,
		.cont_map	= {
			-1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
		},
	},
	[1] = {
		.sources	= IRQ_EINT_GROUP1_NR,
		.base		= IRQ_EINT_GROUP1_BASE,
		.cont_reg	= S3C64XX_VA_GPIO + 0x200,
		.mask_reg	= S3C64XX_VA_GPIO + 0x240,
		.pend_reg	= S3C64XX_VA_GPIO + 0x260,
		.mask_ofs	= 0,
		.pend_ofs	= 0,
		.cont_map	= {
			0, 0, 0, 0, 4, 4, 4, 4,
			8, 8, 8, 8, 12, 12, 12, -1,
		},
	},
	[2] = {
		.sources	= IRQ_EINT_GROUP2_NR,
		.base		= IRQ_EINT_GROUP2_BASE,
		.cont_reg	= S3C64XX_VA_GPIO + 0x200,
		.mask_reg	= S3C64XX_VA_GPIO + 0x240,
		.pend_reg	= S3C64XX_VA_GPIO + 0x260,
		.mask_ofs	= 16,
		.pend_ofs	= 16,
		.cont_map	= {
			16, 16, 16, 16, 20, 20, 20, 20,
			-1, -1, -1, -1, -1, -1, -1, -1,
		},
	},
	[3] = {
		.sources	= IRQ_EINT_GROUP3_NR,
		.base		= IRQ_EINT_GROUP3_BASE,
		.cont_reg	= S3C64XX_VA_GPIO + 0x204,
		.mask_reg	= S3C64XX_VA_GPIO + 0x244,
		.pend_reg	= S3C64XX_VA_GPIO + 0x264,
		.mask_ofs	= 0,
		.pend_ofs	= 0,
		.cont_map	= {
			0, 0, 0, 0, 4, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
		},
	},
	[4] = {
		.sources	= IRQ_EINT_GROUP4_NR,
		.base		= IRQ_EINT_GROUP4_BASE,
		.cont_reg	= S3C64XX_VA_GPIO + 0x204,
		.mask_reg	= S3C64XX_VA_GPIO + 0x244,
		.pend_reg	= S3C64XX_VA_GPIO + 0x264,
		.mask_ofs	= 16,
		.pend_ofs	= 16,
		.cont_map	= {
			16, 16, 16, 16, 20, 20, 20, 20,
			24, 24, 24, 24, 28, 28, -1, -1,
		},
	},
	[5] = {
		.sources	= IRQ_EINT_GROUP5_NR,
		.base		= IRQ_EINT_GROUP5_BASE,
		.cont_reg	= S3C64XX_VA_GPIO + 0x208,
		.mask_reg	= S3C64XX_VA_GPIO + 0x248,
		.pend_reg	= S3C64XX_VA_GPIO + 0x268,
		.mask_ofs	= 0,
		.pend_ofs	= 0,
		.cont_map	= {
			0, 0, 0, 0, 4, 4, 4, -1
			-1, -1, -1, -1, -1, -1, -1, -1,
		},
	},
	[6] = {
		.sources	= IRQ_EINT_GROUP6_NR,
		.base		= IRQ_EINT_GROUP6_BASE,
		.cont_reg	= S3C64XX_VA_GPIO + 0x208,
		.mask_reg	= S3C64XX_VA_GPIO + 0x248,
		.pend_reg	= S3C64XX_VA_GPIO + 0x268,
		.mask_ofs	= 16,
		.pend_ofs	= 16,
		.cont_map	= {
			16, 16, 16, 16, 20, 20, 20, 20,
			24, 24, -1, -1, -1, -1, -1, -1,
		},
	},
	[7] = {
		.sources	= IRQ_EINT_GROUP7_NR,
		.base		= IRQ_EINT_GROUP7_BASE,
		.cont_reg	= S3C64XX_VA_GPIO + 0x20c,
		.mask_reg	= S3C64XX_VA_GPIO + 0x24c,
		.pend_reg	= S3C64XX_VA_GPIO + 0x26c,
		.mask_ofs	= 0,
		.pend_ofs	= 0,
		.cont_map	= {
			0, 0, 0, 0, 4, 4, 4, 4,
			8, 8, 8, 8, 12, 12, 12, 12,
		},
	},
	[8] = {
		.sources	= IRQ_EINT_GROUP8_NR,
		.base		= IRQ_EINT_GROUP8_BASE,
		.cont_reg	= S3C64XX_VA_GPIO + 0x20c,
		.mask_reg	= S3C64XX_VA_GPIO + 0x24c,
		.pend_reg	= S3C64XX_VA_GPIO + 0x26c,
		.mask_ofs	= 16,
		.pend_ofs	= 16,
		.cont_map	= {
			16, 16, 16, 16, 20, 20, 20, 20,
			24, 24, 24, 24, 28, 28, 28, -1,
		},
	},
	[9] = {
		.sources	= IRQ_EINT_GROUP9_NR,
		.base		= IRQ_EINT_GROUP9_BASE,
		.cont_reg	= S3C64XX_VA_GPIO + 0x210,
		.mask_reg	= S3C64XX_VA_GPIO + 0x250,
		.pend_reg	= S3C64XX_VA_GPIO + 0x270,
		.mask_ofs	= 0,
		.pend_ofs	= 0,
		.cont_map	= {
			0, 0, 0, 0, 4, 4, 4, 4,
			4, -1, -1, -1, -1, -1, -1, -1,
		},
	},
};

#define S3C_EINT_GROUPS	(sizeof(eint_groups) / sizeof(eint_groups[0]))

static int to_group_number(unsigned int irq)
{
	int grp, found;

	for (grp = 1; grp < S3C_EINT_GROUPS; grp++) {
		if (irq >= eint_groups[grp].base + eint_groups[grp].sources)
			continue;
		else {
			found = 1;
			break;
		}
	}

	if (!found) {
		printk(KERN_ERR "failed to find out the eint group number\n");
		grp = 0;
	}

	return grp;
}

static inline int to_irq_number(int grp, unsigned int irq)
{
	return irq - eint_groups[grp].base;
}

static inline int to_bit_offset(int grp, unsigned int irq)
{
	int offset;

	offset = eint_groups[grp].cont_map[to_irq_number(grp, irq)];

	if (offset == -1) {
		printk(KERN_ERR "invalid bit offset\n");
		offset = 0;
	}

	return offset;
}

static inline void s3c_irq_eint_group_mask(unsigned int irq)
{
	struct s3c_eint_group_t *group;
	int grp;
	u32 mask;

	grp = to_group_number(irq);
	group = &eint_groups[grp];

	mask = __raw_readl(group->mask_reg);
	mask |= (1 << (group->mask_ofs + to_irq_number(grp, irq)));

	__raw_writel(mask, group->mask_reg);
}

static void s3c_irq_eint_group_unmask(unsigned int irq)
{
	struct s3c_eint_group_t *group;
	int grp;
	u32 mask;

	grp = to_group_number(irq);
	group = &eint_groups[grp];

	mask = __raw_readl(group->mask_reg);
	mask &= ~(1 << (group->mask_ofs + to_irq_number(grp, irq)));

	__raw_writel(mask, group->mask_reg);
}

static inline void s3c_irq_eint_group_ack(unsigned int irq)
{
	struct s3c_eint_group_t *group;
	int grp;
	u32 pend;

	grp = to_group_number(irq);
	group = &eint_groups[grp];

	pend = (1 << (group->pend_ofs + to_irq_number(grp, irq)));

	__raw_writel(pend, group->pend_reg);
}

static void s3c_irq_eint_group_maskack(unsigned int irq)
{
	/* compiler should in-line these */
	s3c_irq_eint_group_mask(irq);
	s3c_irq_eint_group_ack(irq);
}

static int s3c_irq_eint_group_set_type(unsigned int irq, unsigned int type)
{
	struct s3c_eint_group_t *group;
	int grp, shift;
	u32 ctrl, mask, newvalue = 0;

	grp = to_group_number(irq);
	group = &eint_groups[grp];

	switch (type) {
	case IRQ_TYPE_NONE:
		printk(KERN_WARNING "No edge setting!\n");
		break;

	case IRQ_TYPE_EDGE_RISING:
		newvalue = S3C2410_EXTINT_RISEEDGE;
		break;

	case IRQ_TYPE_EDGE_FALLING:
		newvalue = S3C2410_EXTINT_FALLEDGE;
		break;

	case IRQ_TYPE_EDGE_BOTH:
		newvalue = S3C2410_EXTINT_BOTHEDGE;
		break;

	case IRQ_TYPE_LEVEL_LOW:
		newvalue = S3C2410_EXTINT_LOWLEV;
		break;

	case IRQ_TYPE_LEVEL_HIGH:
		newvalue = S3C2410_EXTINT_HILEV;
		break;

	default:
		printk(KERN_ERR "No such irq type %d", type);
		return -1;
	}

	shift = to_bit_offset(grp, irq);
	mask = 0x7 << shift;

	ctrl = __raw_readl(group->cont_reg);
	ctrl &= ~mask;
	ctrl |= newvalue << shift;
	__raw_writel(ctrl, group->cont_reg);

	return 0;
}

static struct irq_chip s3c_irq_eint_group = {
	.name		= "s3c-eint-group",
	.mask		= s3c_irq_eint_group_mask,
	.unmask		= s3c_irq_eint_group_unmask,
	.mask_ack	= s3c_irq_eint_group_maskack,
	.ack		= s3c_irq_eint_group_ack,
	.set_type	= s3c_irq_eint_group_set_type,
};

/*
 * s3c_irq_demux_eint_group
*/
static inline void s3c_irq_demux_eint_group(unsigned int irq, struct irq_desc *desc)
{
	struct s3c_eint_group_t *group;
	u32 status, mask, newirq;
	int grp, src;

	for (grp = 1; grp < S3C_EINT_GROUPS; grp++) {
		group = &eint_groups[grp];
		status = __raw_readl(group->pend_reg);
		mask = __raw_readl(group->mask_reg);

		status &= ~mask;
		status >>= group->pend_ofs;
		status &= 0xffff;

		if (!status)
			continue;

		for (src = 0; src < S3C64XX_EINT_MAX_SOURCES; src++) {
			if (status & 1) {
				newirq = group->base + src;
				generic_handle_irq(newirq);
			}

			status >>= 1;
		}
	}
}

int __init s3c64xx_init_irq_eint_group(void)
{
	int irq;

	for (irq = IRQ_EINT_GROUP_BASE; irq < NR_IRQS; irq++) {
		set_irq_chip(irq, &s3c_irq_eint_group);
		set_irq_handler(irq, handle_level_irq);
		set_irq_flags(irq, IRQF_VALID);
	}

	set_irq_chained_handler(IRQ_EINT_GROUPS, s3c_irq_demux_eint_group);

	return 0;
}

arch_initcall(s3c64xx_init_irq_eint_group);
