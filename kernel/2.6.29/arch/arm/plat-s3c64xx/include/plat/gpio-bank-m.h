/* linux/arch/arm/plat-s3c64xx/include/plat/gpio-bank-m.h
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *  Ben Dooks <ben@simtec.co.uk>
 *  http://armlinux.simtec.co.uk/
 *
 * GPIO Bank M register and configuration definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __ASM_PLAT_S3C64XX_GPIO_BANK_M_H
#define __ASM_PLAT_S3C64XX_GPIO_BANK_M_H __FILE__

#define S3C64XX_GPMCON          (S3C64XX_GPM_BASE + 0x00)
#define S3C64XX_GPMDAT          (S3C64XX_GPM_BASE + 0x04)
#define S3C64XX_GPMPUD          (S3C64XX_GPM_BASE + 0x08)

#define S3C64XX_GPM_CONMASK(__gpio) (0x3 << ((__gpio) * 2))
#define S3C64XX_GPM_INPUT(__gpio)   (0x0 << ((__gpio) * 2))
#define S3C64XX_GPM_OUTPUT(__gpio)  (0x1 << ((__gpio) * 2))

#define S3C64XX_GPM0_HOSTIF_CS      (0x02 << 0)
#define S3C64XX_GPM0_EINT23      (0x03 << 0)
#define S3C64XX_GPM0_RESERVED1      (0x04 << 0)
#define S3C64XX_GPM0_DATA_CF10      (0x05 << 0)
#define S3C64XX_GPM0_CE_CF0      (0x06 << 0)
#define S3C64XX_GPM0_RESERVED2      (0x07 << 0)

#define S3C64XX_GPM1_HOSTIF_CS_M      (0x02 << 0)
#define S3C64XX_GPM1_EINT24      (0x03 << 0)
#define S3C64XX_GPM1_RESERVED1      (0x04 << 0)
#define S3C64XX_GPM1_DATA_CF11      (0x05 << 0)
#define S3C64XX_GPM1_CE_CF1      (0x06 << 0)
#define S3C64XX_GPM1_RESERVED2      (0x07 << 0)

#define S3C64XX_GPM2_HOSTIF_IF_CS_S      (0x02 << 0)
#define S3C64XX_GPM2_EINT25      (0x03 << 0)
#define S3C64XX_GPM2_HOSTIF_MDP_VSYNC      (0x04 << 0)
#define S3C64XX_GPM2_DATA_CF12      (0x05 << 0)
#define S3C64XX_GPM2_IORD_CF      (0x06 << 0)
#define S3C64XX_GPM2_RESERVED2      (0x07 << 0)

#define S3C64XX_GPM3_HOSTIF_WE      (0x02 << 0)
#define S3C64XX_GPM3_EINT26      (0x03 << 0)
#define S3C64XX_GPM3_RESERVED1      (0x04 << 0)
#define S3C64XX_GPM3_DATA_CF13      (0x05 << 0)
#define S3C64XX_GPM3_IOWR_CF      (0x06 << 0)
#define S3C64XX_GPM3_RESERVED2      (0x07 << 0)

#define S3C64XX_GPM4_HOSTIF_OE      (0x02 << 0)
#define S3C64XX_GPM4_EINT27      (0x03 << 0)
#define S3C64XX_GPM4_RESERVED1      (0x04 << 0)
#define S3C64XX_GPM4_DATA_CF14      (0x05 << 0)
#define S3C64XX_GPM4_IORDY_CF      (0x06 << 0)
#define S3C64XX_GPM4_RESERVED2      (0x07 << 0)

#define S3C64XX_GPM5_HOSTIF_INTR      (0x02 << 0)
#define S3C64XX_GPM5_CF_DATA_DIR      (0x03 << 0)
#define S3C64XX_GPM5_RESERVED1      (0x04 << 0)
#define S3C64XX_GPM5_DATA_CF15      (0x05 << 0)
#define S3C64XX_GPM5_RESERVED2      (0x06 << 0)
#define S3C64XX_GPM5_RESERVED3      (0x07 << 0)

#endif	/* __ASM_PLAT_S3C64XX_GPIO_BANK_M_H */
