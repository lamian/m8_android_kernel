/* linux/arch/arm/plat-s3c64xx/include/plat/gpio-bank-k.h
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 * 	Ben Dooks <ben@simtec.co.uk>
 * 	http://armlinux.simtec.co.uk/
 *
 * GPIO Bank J register and configuration definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_PLAT_S3C64XX_GPIO_BANK_K_H
#define __ASM_PLAT_S3C64XX_GPIO_BANK_K_H __FILE__

#define S3C64XX_GPKCON			(S3C64XX_GPK_BASE + 0x00)
#define S3C64XX_GPKCON1			(S3C64XX_GPK_BASE + 0x04)
#define S3C64XX_GPKDAT			(S3C64XX_GPK_BASE + 0x08)
#define S3C64XX_GPKPUD			(S3C64XX_GPK_BASE + 0x0c)

#define S3C64XX_GPK_CONMASK(__gpio)	(0x3 << ((__gpio) * 2))
#define S3C64XX_GPK_INPUT(__gpio)	(0x0 << ((__gpio) * 2))
#define S3C64XX_GPK_OUTPUT(__gpio)	(0x1 << ((__gpio) * 2))


#endif	/* __ASM_PLAT_S3C64XX_GPIO_BANK_K_H */
