/* arch/arm/plat-s3c/include/plat/regs-watchdog.h
 *
 * Copyright (c) 2003 Simtec Electronics <linux@simtec.co.uk>
 *		      http://www.simtec.co.uk/products/SWLINUX/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * S3C Watchdog timer control
*/


#ifndef __ASM_ARCH_REGS_WATCHDOG_H
#define __ASM_ARCH_REGS_WATCHDOG_H

#define S3C_WDOGREG(x) ((x) + S3C_VA_WATCHDOG)

#define S3C_WTCON	   S3C_WDOGREG(0x00)
#define S3C_WTDAT	   S3C_WDOGREG(0x04)
#define S3C_WTCNT	   S3C_WDOGREG(0x08)

#define S3C_WTCON_OFFSET	   (0x00)
#define S3C_WTDAT_OFFSET	   (0x04)
#define S3C_WTCNT_OFFSET	   (0x08)

#define S3C_WTCNT_CNT	   (0x1)
#define S3C_WTCNT_CON	   (0x7639)
#define S3C_WTCNT_DAT	   (0xFFCF)

/* the watchdog can either generate a reset pulse, or an
 * interrupt.
 */

#define S3C_WTCON_RSTEN   (0x01)
#define S3C_WTCON_INTEN   (1<<2)
#define S3C_WTCON_ENABLE  (1<<5)

#define S3C_WTCON_DIV16   (0<<3)
#define S3C_WTCON_DIV32   (1<<3)
#define S3C_WTCON_DIV64   (2<<3)
#define S3C_WTCON_DIV128  (3<<3)

#define S3C_WTCON_PRESCALE(x) ((x) << 8)
#define S3C_WTCON_PRESCALE_MASK (0xff00)

#endif /* __ASM_ARCH_REGS_WATCHDOG_H */


