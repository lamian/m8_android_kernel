/* linux/arch/arm/mach-s3c6400/include/mach/map.h
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	http://armlinux.simtec.co.uk/
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * S3C64XX - Memory map definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_MAP_H
#define __ASM_ARCH_MAP_H __FILE__

#include <plat/map-base.h>

/* HSMMC units */
#define S3C64XX_PA_HSMMC(x)	(0x7C200000 + ((x) * 0x100000))
#define S3C64XX_PA_HSMMC0	S3C64XX_PA_HSMMC(0)
#define S3C64XX_PA_HSMMC1	S3C64XX_PA_HSMMC(1)
#define S3C64XX_PA_HSMMC2	S3C64XX_PA_HSMMC(2)
#define S3C_SZ_HSMMC	   	SZ_1M

#define S3C_PA_UART		(0x7F005000)
#define S3C_PA_UART0		(S3C_PA_UART + 0x00)
#define S3C_PA_UART1		(S3C_PA_UART + 0x400)
#define S3C_PA_UART2		(S3C_PA_UART + 0x800)
#define S3C_PA_UART3		(S3C_PA_UART + 0xC00)
#define S3C_UART_OFFSET		(0x400)

/* See notes on UART VA mapping in debug-macro.S */
#define S3C_VA_UARTx(x)		(S3C_VA_UART + (S3C_PA_UART & 0xfffff) + ((x) * S3C_UART_OFFSET))

#define S3C_VA_UART0		S3C_VA_UARTx(0)
#define S3C_VA_UART1		S3C_VA_UARTx(1)
#define S3C_VA_UART2		S3C_VA_UARTx(2)
#define S3C_VA_UART3		S3C_VA_UARTx(3)
#define S3C_SZ_UART		SZ_256

#define S3C64XX_PA_SYSCON	(0x7E00F000)
#define S3C64XX_PA_TIMER	(0x7F006000)
#define S3C64XX_PA_IIC0		(0x7F004000)
#define S3C64XX_PA_IIC1		(0x7F00F000)

#define S3C64XX_PA_GPIO		(0x7F008000)
#define S3C64XX_VA_GPIO		S3C_ADDR(0x00500000)
#define S3C64XX_SZ_GPIO		SZ_4K

#define S3C64XX_PA_SDRAM	(0x50000000)
#define S3C64XX_PA_VIC0		(0x71200000)
#define S3C64XX_PA_VIC1		(0x71300000)

#define S3C64XX_VA_SROMC	S3C_VA_SROMC
#define S3C64XX_PA_SROMC	(0x70000000)
#define S3C64XX_SZ_SROMC	SZ_1M

#define S3C64XX_VA_LCD	   	S3C_VA_LCD
#define S3C64XX_PA_LCD	   	(0x77100000)
#define S3C64XX_SZ_LCD		SZ_1M

#define S3C64XX_PA_G2D	   	(0x76100000)
#define S3C64XX_SZ_G2D		SZ_1M

#define S3C64XX_PA_G3D	   	(0x72000000)
#define S3C64XX_SZ_G3D		SZ_16M

#define S3C64XX_PA_FIMC		(0x78000000)
#define S3C64XX_SZ_FIMC		SZ_1M

#define S3C64XX_PA_ADC		(0x7E00B000)
#define S3C64XX_PA_SMC9115	(0x18000000)
#define S3C64XX_SZ_SMC9115	SZ_512M

#define S3C64XX_PA_RTC		(0x7E005000)
#define S3C64XX_PA_AC97		(0x7F001000)
#define S3C64XX_PA_IIS_V32	(0x7F002000)
#define S3C64XX_PA_IIS_V40 	(0x7F00D000)
#define S3C_SZ_IIS		SZ_8K

/* DMA controller */
#define S3C64XX_PA_DMA		(0x75000000)

/* place VICs close together */
#define S3C_VA_VIC0		(S3C_VA_IRQ + 0x00)
#define S3C_VA_VIC1		(S3C_VA_IRQ + 0x10000)

/* Host I/F Indirect & Direct */
#define S3C64XX_VA_HOSTIFA	S3C_ADDR(0x00B00000)
#define S3C64XX_PA_HOSTIFA	(0x74000000)
#define S3C64XX_SZ_HOSTIFA	SZ_1M

#define S3C64XX_VA_HOSTIFB	S3C_ADDR(0x00C00000)
#define S3C64XX_PA_HOSTIFB	(0x74100000)
#define S3C64XX_SZ_HOSTIFB	SZ_1M

/* TV-ENCODER */
#define S3C6400_PA_TVENC	(0x76200000)
#define S5PC100_PA_TVENC	(0xF0000000)
#define S3C_SZ_TVENC		SZ_1M

/* TV-SCALER*/
#define S3C6400_PA_TVSCALER	(0x76300000)
#define S3C_SZ_TVSCALER		SZ_1M

/* Rotator */
#define S3C6400_PA_ROTATOR      (0x77200000)
#define S3C_SZ_ROTATOR          SZ_1M

/* JPEG */
#define S3C6400_PA_JPEG         (0x78800000)
#define S3C_SZ_JPEG             SZ_4M

/* VPP */
#define S3C6400_PA_VPP          (0x77000000)
#define S5PC100_PA_VPP          (0xF0100000)
#define S3C_SZ_VPP              SZ_1M

/* MFC */
#define S3C6400_PA_MFC		(0x7E002000)
#define S5PC100_PA_MFC		(0xF1000000)
#define S3C_SZ_MFC		SZ_4K
    
/* NAND flash controller */
#define S3C64XX_PA_NAND	   	(0x70200000)
#define S3C64XX_SZ_NAND	   	SZ_1M

/* OneNAND */
#define S3C64XX_PA_ONENAND	(0x70100000)
#define S3C64XX_SZ_ONENAND	   	SZ_1M
#define S3C_SZ_ONENAND		SZ_1M

/* USB Host */
#define S3C64XX_PA_USBHOST	(0x74300000)
#define S3C64XX_SZ_USBHOST	SZ_1M

/* USB OTG */
#define S3C64XX_VA_OTG		S3C_ADDR(0x03900000)
#define S3C64XX_PA_OTG		(0x7C000000)
#define S3C64XX_SZ_OTG		SZ_1M

/* USB OTG SFR */
#define S3C64XX_VA_OTGSFR	S3C_ADDR(0x03a00000)
#define S3C64XX_PA_OTGSFR	(0x7C100000)
#define S3C64XX_SZ_OTGSFR	SZ_1M

#define S3C64XX_PA_KEYPAD	(0x7E00A000)
#define S3C64XX_SZ_KEYPAD	SZ_4K

/* SPI */
#define S3C64XX_PA_SPI		(0x7F00B000)
#define S3C64XX_PA_SPI0		(0x7F00B000)
#define S3C64XX_PA_SPI1		(0x7F00C000)
#define S3C64XX_SZ_SPI		SZ_8K
#define S3C64XX_SZ_SPI0		SZ_4K
#define S3C64XX_SZ_SPI1		SZ_4K

/* Watchdog */
#define S3C64XX_PA_WATCHDOG	(0x7E004000)
#define S3C64XX_SZ_WATCHDOG 	SZ_4K

/* AES */
#define S3C64XX_PA_AES		(0x7D000000)
#define S3C64XX_SZ_AES		SZ_16M

/* compatibiltiy defines. */
#define S3C_PA_TIMER		S3C64XX_PA_TIMER
#define S3C_PA_HSMMC0		S3C64XX_PA_HSMMC0
#define S3C_PA_HSMMC1		S3C64XX_PA_HSMMC1
#define S3C_PA_HSMMC2		S3C64XX_PA_HSMMC2
#define S3C_PA_IIC		S3C64XX_PA_IIC0
#define S3C_PA_IIC1		S3C64XX_PA_IIC1

#define S3C_PA_RTC		S3C64XX_PA_RTC

#define S3C_PA_SPI		S3C64XX_PA_SPI
#define S3C_PA_SPI0		S3C64XX_PA_SPI0
#define S3C_PA_SPI1		S3C64XX_PA_SPI1
#define S3C_SZ_SPI		S3C64XX_SZ_SPI
#define S3C_SZ_SPI0		S3C64XX_SZ_SPI0
#define S3C_SZ_SPI1		S3C64XX_SZ_SPI1

#define S3C_PA_ADC		S3C64XX_PA_ADC
#define S3C_PA_DMA		S3C64XX_PA_DMA

#define S3C_VA_OTG		S3C64XX_VA_OTG
#define S3C_PA_OTG		S3C64XX_PA_OTG
#define S3C_SZ_OTG		S3C64XX_SZ_OTG

#define S3C_VA_OTGSFR		S3C64XX_VA_OTGSFR
#define S3C_PA_OTGSFR		S3C64XX_PA_OTGSFR
#define S3C_SZ_OTGSFR		S3C64XX_SZ_OTGSFR

#define S3C_PA_KEYPAD		S3C64XX_PA_KEYPAD
#define S3C_SZ_KEYPAD		S3C64XX_SZ_KEYPAD

#endif /* __ASM_ARCH_6400_MAP_H */
