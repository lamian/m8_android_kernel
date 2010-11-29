/* linux/arch/arm/plat-s3c64xx/include/plat/gpio-bank-l.h
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

#ifndef __ASM_PLAT_S3C64XX_GPIO_BANK_L_H
#define __ASM_PLAT_S3C64XX_GPIO_BANK_L_H __FILE__

#define S3C64XX_GPLCON			(S3C64XX_GPL_BASE + 0x00)
#define S3C64XX_GPLCON1			(S3C64XX_GPL_BASE + 0x04)
#define S3C64XX_GPLDAT			(S3C64XX_GPL_BASE + 0x08)
#define S3C64XX_GPLPUD			(S3C64XX_GPL_BASE + 0x0c)

#define S3C64XX_GPL_CONMASK(__gpio)	(0xf << ((__gpio) * 4))
#define S3C64XX_GPL_INPUT(__gpio)	(0x0 << ((__gpio) * 4))
#define S3C64XX_GPL_OUTPUT(__gpio)	(0x1 << ((__gpio) * 4))

#define S3C64XX_GPL0_HOSTIF_ADDR0	(0x2 << 0)
#define S3C64XX_GPL0_KEYPAD_COL0	(0x3 << 0)
#define S3C64XX_GPL0_RESERVED1		(0x4 << 0)
#define S3C64XX_GPL0_RESERVED2		(0x5 << 0)
#define S3C64XX_GPL0_ADDR_CF0		(0x6 << 0)
#define S3C64XX_GPL0_OTG_ULPI_DATA0	(0x7 << 0)

#define S3C64XX_GPL1_HOSTIF_ADDR1	(0x2 << 4)
#define S3C64XX_GPL1_KEYPAD_COL1	(0x3 << 4)
#define S3C64XX_GPL1_RESERVED1		(0x4 << 4)
#define S3C64XX_GPL1_RESERVED2		(0x5 << 4)
#define S3C64XX_GPL1_ADDR_CF1		(0x6 << 4)
#define S3C64XX_GPL1_OTG_ULPI_DATA1	(0x7 << 4)

#define S3C64XX_GPL2_HOSTIF_ADDR2	(0x2 << 8)
#define S3C64XX_GPL2_KEYPAD_COL2	(0x3 << 8)
#define S3C64XX_GPL2_RESERVED1		(0x4 << 8)
#define S3C64XX_GPL2_RESERVED2		(0x5 << 8)
#define S3C64XX_GPL2_ADDR_CF2		(0x6 << 8)
#define S3C64XX_GPL2_OTG_ULPI_DATA2	(0x7 << 8)

#define S3C64XX_GPL3_HOSTIF_ADDR3	(0x2 << 12)
#define S3C64XX_GPL3_KEYPAD_COL3	(0x3 << 12)
#define S3C64XX_GPL3_RESERVED1      (0x4 << 12)
#define S3C64XX_GPL3_RESERVED2		(0x5 << 12)
#define S3C64XX_GPL3_RESERVED3		(0x6 << 12)
#define S3C64XX_GPL3_OTG_ULPI_DATA3	(0x7 << 12)

#define S3C64XX_GPL4_HOSTIF_ADDR4	(0x2 << 16)
#define S3C64XX_GPL4_KEYPAD_COL4	(0x3 << 16)
#define S3C64XX_GPL4_RESERVED1		(0x4 << 16)
#define S3C64XX_GPL4_RESERVED2		(0x5 << 16)
#define S3C64XX_GPL4_RESERVED3		(0x6 << 16)
#define S3C64XX_GPL4_OTG_ULPI_DATA4	(0x7 << 16)

#define S3C64XX_GPL5_HOSTIF_ADDR5	(0x2 << 20)
#define S3C64XX_GPL5_KEYPAD_COL5	(0x3 << 20)
#define S3C64XX_GPL5_RESERVED1		(0x4 << 20)
#define S3C64XX_GPL5_RESERVED2		(0x5 << 20)
#define S3C64XX_GPL5_RESERVED3		(0x6 << 20)
#define S3C64XX_GPL5_OTG_ULPI_DATA5	(0x7 << 20)

#define S3C64XX_GPL6_HOSTIF_ADDR6	(0x2 << 24)
#define S3C64XX_GPL6_KEYPAD_COL6	(0x3 << 24)
#define S3C64XX_GPL6_RESERVED1		(0x4 << 24)
#define S3C64XX_GPL6_RESERVED2		(0x5 << 24)
#define S3C64XX_GPL6_RESERVED3		(0x6 << 24)
#define S3C64XX_GPL6_OTG_ULPI_DATA6	(0x7 << 24)

#define S3C64XX_GPL7_HOSTIF_ADDR7	(0x2 << 28)
#define S3C64XX_GPL7_KEYPAD_COL7	(0x3 << 28)
#define S3C64XX_GPL7_RESERVED1		(0x4 << 28)
#define S3C64XX_GPL7_RESERVED2		(0x5 << 28)
#define S3C64XX_GPL7_RESERVED3		(0x6 << 28)
#define S3C64XX_GPL7_OTG_ULPI_DATA	(0x7 << 28)

#define S3C64XX_GPL8_HOSTIF_ADDR8	(0x2 << 0)
#define S3C64XX_GPL8_EXTINT16		(0x3 << 0)
#define S3C64XX_GPL8_RESERVED1		(0x4 << 0)
#define S3C64XX_GPL8_CE_CF0			(0x5 << 0)
#define S3C64XX_GPL8_RESERVED2		(0x6 << 0)
#define S3C64XX_GPL8_OTG_ULPI_STP	(0x7 << 0)

#define S3C64XX_GPL9_HOSTIF_ADDR9	(0x2 << 4)
#define S3C64XX_GPL9_EXTINT17		(0x3 << 4)
#define S3C64XX_GPL9_RESERVED1		(0x4 << 4)
#define S3C64XX_GPL9_CE_CF1			(0x5 << 4)
#define S3C64XX_GPL9_RESERVED2		(0x6 << 4)
#define S3C64XX_GPL9_OTG_ULPI_CLK	(0x7 << 4)

#define S3C64XX_GPL10_SPI_CLK1		(0x2 << 8)
#define S3C64XX_GPL10_EXTINT18		(0x3 << 8)
#define S3C64XX_GPL10_RESERVED1		(0x4 << 8)
#define S3C64XX_GPL10_IORD_CF		(0x5 << 8)
#define S3C64XX_GPL10_RESERVED2		(0x6 << 8)
#define S3C64XX_GPL10_OTG_ULPI_NXT	(0x7 << 8)

#define S3C64XX_GPL11_SPI_MOSI1		(0x2 << 12)
#define S3C64XX_GPL11_EXTINT19		(0x3 << 12)
#define S3C64XX_GPL11_RESERVED1		(0x4 << 12)
#define S3C64XX_GPL11_IOWR_CF		(0x5 << 12)
#define S3C64XX_GPL11_RESERVED2		(0x6 << 12)
#define S3C64XX_GPL11_OTG_ULPI_DIR	(0x7 << 12)

#define S3C64XX_GPL12_SPI_MISO1		(0x2 << 16)
#define S3C64XX_GPL12_EXTINT20		(0x3 << 16)
#define S3C64XX_GPL12_RESERVED1		(0x4 << 16)
#define S3C64XX_GPL12_IORDY_CF		(0x5 << 16)
#define S3C64XX_GPL12_RESERVED2		(0x6 << 16)
#define S3C64XX_GPL12_RESERVED3		(0x7 << 16)

#define S3C64XX_GPL14_nSS0			(0x2 << 20)
#define S3C64XX_GPL13_EXTINT21		(0x3 << 20)
#define S3C64XX_GPL13_RESERVED1		(0x4 << 20)
#define S3C64XX_GPL13_DATA_CF8		(0x5 << 20)
#define S3C64XX_GPL13_RESERVED2		(0x6 << 20)
#define S3C64XX_GPL13_RESERVED3		(0x7 << 20)

#define S3C64XX_GPL14_nSS1			(0x2 << 24)
#define S3C64XX_GPL14_EXTINT22		(0x3 << 24)
#define S3C64XX_GPL14_RESERVED1		(0x4 << 24)
#define S3C64XX_GPL14_DATA_CF9		(0x5 << 24)
#define S3C64XX_GPL14_RESERVED2		(0x6 << 24)
#define S3C64XX_GPL14_RESERVED3		(0x7 << 24)

#endif	/* __ASM_PLAT_S3C64XX_GPIO_BANK_L_H */
