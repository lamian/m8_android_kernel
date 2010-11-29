/* linux/arch/arm/mach-s3c6400/include/mach/regs-irq.h
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	http://armlinux.simtec.co.uk/
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * S3C64XX - IRQ register definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_REGS_IRQ_H
#define __ASM_ARCH_REGS_IRQ_H __FILE__

#include <asm/hardware/vic.h>
#include <mach/map.h>
/* interrupt controller */
#define S3C_VIC0REG(x)          ((x) + S3C_VA_VIC0)
#define S3C_VIC1REG(x)          ((x) + S3C_VA_VIC1)

#define S3C64XX_VIC0IRQSTATUS       S3C_VIC0REG(0x000)
#define S3C64XX_VIC1IRQSTATUS       S3C_VIC1REG(0x000)

#define S3C64XX_VIC0FIQSTATUS       S3C_VIC0REG(0x004)
#define S3C64XX_VIC1FIQSTATUS       S3C_VIC1REG(0x004)

#define S3C64XX_VIC0RAWINTR         S3C_VIC0REG(0x008)
#define S3C64XX_VIC1RAWINTR         S3C_VIC1REG(0x008)

#define S3C64XX_VIC0INTSELECT       S3C_VIC0REG(0x00C)
#define S3C64XX_VIC1INTSELECT       S3C_VIC1REG(0x00C)

#define S3C64XX_VIC0INTENABLE       S3C_VIC0REG(0x010)
#define S3C64XX_VIC1INTENABLE       S3C_VIC1REG(0x010)

#define S3C64XX_VIC0INTENCLEAR      S3C_VIC0REG(0x014)
#define S3C64XX_VIC1INTENCLEAR      S3C_VIC1REG(0x014)

#define S3C64XX_VIC0SOFTINT         S3C_VIC0REG(0x018)
#define S3C64XX_VIC1SOFTINT         S3C_VIC1REG(0x018)

#define S3C64XX_VIC0SOFTINTCLEAR    S3C_VIC0REG(0x01C)
#define S3C64XX_VIC1SOFTINTCLEAR    S3C_VIC1REG(0x01C)

#define S3C64XX_VIC0PROTECTION      S3C_VIC0REG(0x020)
#define S3C64XX_VIC1PROTECTION      S3C_VIC1REG(0x020)

#define S3C64XX_VIC0SWPRIORITYMASK  S3C_VIC0REG(0x024)
#define S3C64XX_VIC1SWPRIORITYMASK  S3C_VIC1REG(0x024)

#define S3C64XX_VIC0PRIORITYDAISY   S3C_VIC0REG(0x028)
#define S3C64XX_VIC1PRIORITYDAISY   S3C_VIC1REG(0x028)

#define S3C64XX_VIC0VECTADDR0       S3C_VIC0REG(0x100)
#define S3C64XX_VIC1VECTADDR0       S3C_VIC1REG(0x100)

#define S3C64XX_VIC0VECTADDR1       S3C_VIC0REG(0x104)
#define S3C64XX_VIC1VECTADDR1       S3C_VIC1REG(0x104)

#define S3C64XX_VIC0VECTADDR2       S3C_VIC0REG(0x108)
#define S3C64XX_VIC1VECTADDR2       S3C_VIC1REG(0x108)

#define S3C64XX_VIC0ADDRESS         S3C_VIC0REG(0xF00)
#define S3C64XX_VIC1ADDRESS         S3C_VIC1REG(0xF00)

#endif /* __ASM_ARCH_6400_REGS_IRQ_H */
