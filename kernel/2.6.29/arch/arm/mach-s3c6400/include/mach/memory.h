/* linux/arch/arm/mach-s3c6400/include/mach/memory.h
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *      Ben Dooks <ben@simtec.co.uk>
 *      http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

#define PHYS_OFFSET     	UL(0x50000000)
#define CONSISTENT_DMA_SIZE	(SZ_8M + SZ_4M + SZ_2M)

#ifdef CONFIG_SMDK6410_RAMSIZE_256M
#define PHYS_SIZE       (256 * 1024 * 1024)
#else
#define PHYS_SIZE       (128 * 1024 * 1024)
#endif /* CONFIG_SMDK6410_RAMSIZE_256M */

#define __virt_to_bus(x) __virt_to_phys(x)
#define __bus_to_virt(x) __phys_to_virt(x)

#endif
