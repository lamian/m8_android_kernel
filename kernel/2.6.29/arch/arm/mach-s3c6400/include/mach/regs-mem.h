/* linux/arch/arm/mach-s3c6400/include/mach/regs-mem.h
 *
 * Copyright (c) 2004 Simtec Electronics <linux@simtec.co.uk>
 *		http://www.simtec.co.uk/products/SWLINUX/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * S3C2410 Memory Control register definitions
*/

#ifndef __ASM_ARM_MEMREGS_H
#define __ASM_ARM_MEMREGS_H

#ifndef S3C64XX_MEMREG
#define S3C64XX_MEMREG(x) (S3C64XX_VA_SROMC + (x))
#endif


/* Bank Idle Cycle Control Registers 0-5 */
#define S3C64XX_SROM_BW		S3C64XX_MEMREG(0x00)

#define S3C64XX_SROM_BC0	S3C64XX_MEMREG(0x04)
#define S3C64XX_SROM_BC1	S3C64XX_MEMREG(0x08)
#define S3C64XX_SROM_BC2	S3C64XX_MEMREG(0x0C)
#define S3C64XX_SROM_BC3	S3C64XX_MEMREG(0x10)
#define S3C64XX_SROM_BC4	S3C64XX_MEMREG(0x14)
#define S3C64XX_SROM_BC5	S3C64XX_MEMREG(0x18)

/* SROM BW */
#define S3C64XX_SROM_BW_DATA_WIDTH0_8BIT	(0 << 0)
#define S3C64XX_SROM_BW_DATA_WIDTH0_16BIT	(1 << 0)
#define S3C64XX_SROM_BW_DATA_WIDTH0_MASK	(1 << 0)

#define S3C64XX_SROM_BW_WAIT_ENABLE0_DISABLE	(0 << 2)
#define S3C64XX_SROM_BW_WAIT_ENABLE0_ENABLE	(1 << 2)
#define S3C64XX_SROM_BW_WAIT_ENABLE0_MASK	(1 << 2)

#define S3C64XX_SROM_BW_BYTE_ENABLE0_DISABLE	(0 << 3)
#define S3C64XX_SROM_BW_BYTE_ENABLE0_ENABLE	(1 << 3)
#define S3C64XX_SROM_BW_BYTE_ENABLE0_MASK	(1 << 3)

#define S3C64XX_SROM_BW_DATA_WIDTH1_8BIT	(0 << 4)
#define S3C64XX_SROM_BW_DATA_WIDTH1_16BIT	(1 << 4)
#define S3C64XX_SROM_BW_DATA_WIDTH1_MASK	(1 << 4)

#define S3C64XX_SROM_BW_WAIT_ENABLE1_DISABLE	(0 << 6)
#define S3C64XX_SROM_BW_WAIT_ENABLE1_ENABLE	(1 << 6)
#define S3C64XX_SROM_BW_WAIT_ENABLE1_MASK	(1 << 6)

#define S3C64XX_SROM_BW_BYTE_ENABLE1_DISABLE	(0 << 7)
#define S3C64XX_SROM_BW_BYTE_ENABLE1_ENABLE	(1 << 7)
#define S3C64XX_SROM_BW_BYTE_ENABLE1_MASK	(1 << 7)

#define S3C64XX_SROM_BW_DATA_WIDTH2_8BIT	(0 << 8)
#define S3C64XX_SROM_BW_DATA_WIDTH2_16BIT	(1 << 8)
#define S3C64XX_SROM_BW_DATA_WIDTH2_MASK	(1 << 8)

/* SROM BCn */
#define S3C64XX_SROM_BCn_TACS(x)		(x << 28)
#define S3C64XX_SROM_BCn_TCOS(x)		(x << 24)
#define S3C64XX_SROM_BCn_TACC(x)		(x << 16)
#define S3C64XX_SROM_BCn_TCOH(x)		(x << 12)
#define S3C64XX_SROM_BCn_TCAH(x)		(x << 8)
#define S3C64XX_SROM_BCn_TACP(x)		(x << 4)
#define S3C64XX_SROM_BCn_PMC_NORMAL		(0 << 0)
#define S3C64XX_SROM_BCn_PMC_4			(1 << 0)

#endif /* __ASM_ARM_MEMREGS_H */
