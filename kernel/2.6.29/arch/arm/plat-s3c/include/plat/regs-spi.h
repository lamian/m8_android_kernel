/* linux/include/asm-arm/arch-s3c2410/regs-spi.h
 *
 * Copyright (c) 2004 Fetron GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * S3C2410 SPI register definition
*/

#ifndef __ASM_ARCH_REGS_SPI_H
#define __ASM_ARCH_REGS_SPI_H

/*
 * SPI(High speed) Registers
 */

#define S3C_SPI_REG_CH0(x)	((x) + S3C_VA_SPI0)
#define S3C_SPI_REG_CH1(x)	((x) + S3C_VA_SPI1)

#define S3C_CH_CFG		(0x00)      //SPI configuration
#define S3C_CLK_CFG		(0x04)      //Clock configuration
#define S3C_MODE_CFG		(0x08)      //SPI FIFO control
#define S3C_SLAVE_SEL		(0x0C)      //Slave selection
#define S3C_SPI_INT_EN		(0x10)      //SPI interrupt enable
#define S3C_SPI_STATUS		(0x14)      //SPI status
#define S3C_SPI_TX_DATA		(0x18)      //SPI TX data
#define S3C_SPI_RX_DATA		(0x1C)      //SPI RX data
#define S3C_PACKET_CNT		(0x20)      //count how many data master gets
#define S3C_PENDING_CLR		(0x24)      //Pending clear
#define S3C_SWAP_CFG		(0x28)      //SWAP config register
#define S3C_FB_CLK		(0x2C)      //SWAP FB config register


#define SPI_CH_HIGH_SPEED_EN	(1<<6)
#define SPI_CH_SW_RST		(1<<5)
#define SPI_CH_MASTER		(0<<4)
#define SPI_CH_SLAVE		(1<<4)
#define SPI_CH_RISING		(0<<3)
#define SPI_CH_FALLING		(1<<3)
#define SPI_CH_FORMAT_A		(0<<2)
#define SPI_CH_FORMAT_B		(1<<2)
#define SPI_CH_RXCH_OFF		(0<<1)
#define SPI_CH_RXCH_ON		(1<<1)
#define SPI_CH_TXCH_OFF		(0<<0)
#define SPI_CH_TXCH_ON		(1<<0)

#define SPI_CLKSEL_PCLK		(0<<9)
#define SPI_CLKSEL_SCLK_48	(1<<9)
#define SPI_CLKSEL_SCLK		(2<<9)
#define SPI_ENCLK_DISABLE	(0<<8)
#define SPI_ENCLK_ENABLE	(1<<8)

#define SPI_MODE_CH_TSZ_BYTE	(0<<29)
#define SPI_MODE_CH_TSZ_HALFWORD	(1<<29)
#define SPI_MODE_CH_TSZ_WORD	(2<<29)
#define SPI_MODE_BUS_TSZ_BYTE	(0<<18)
#define SPI_MODE_BUS_TSZ_HALFWORD	(1<<17)
#define SPI_MODE_BUS_TSZ_WORD	(2<<17)
#define SPI_MODE_SWAP_ENABLE	(1<<3)
#define SPI_MODE_SWAP_DISABLE	(0<<3)
#define SPI_MODE_RXDMA_OFF	(0<<2)
#define SPI_MODE_RXDMA_ON	(1<<2)
#define SPI_MODE_TXDMA_OFF	(0<<1)
#define SPI_MODE_TXDMA_ON	(1<<1)
#define SPI_MODE_SINGLE		(0<<0)
#define SPI_MODE_4BURST		(1<<0)

#define SPI_SLAVE_MAN		(0<<1)
#define SPI_SLAVE_AUTO		(1<<1)
#define SPI_SLAVE_SIG_ACT	(0<<0)
#define SPI_SLAVE_SIG_INACT	(1<<0)

#define SPI_INT_TRAILING_DIS	(0<<6)
#define SPI_INT_TRAILING_EN	(1<<6)
#define SPI_INT_RX_OVERRUN_DIS	(0<<5)
#define SPI_INT_RX_OVERRUN_EN	(1<<5)
#define SPI_INT_RX_UNDERRUN_DIS	(0<<4)
#define SPI_INT_RX_UNDERRUN_EN	(1<<4)
#define SPI_INT_TX_OVERRUN_DIS	(0<<3)
#define SPI_INT_TX_OVERRUN_EN	(1<<3)
#define SPI_INT_TX_UNDERRUN_DIS	(0<<2)
#define SPI_INT_TX_UNDERRUN_EN	(1<<2)
#define SPI_INT_RX_FIFORDY_DIS	(0<<1)
#define SPI_INT_RX_FIFORDY_EN	(1<<1)
#define SPI_INT_TX_FIFORDY_DIS	(0<<0)
#define SPI_INT_TX_FIFORDY_EN	(1<<0)

#define SPI_STUS_TX_DONE	(1<<21)
#define SPI_STUS_TRAILCNT_ZERO	(1<<20)
#define SPI_STUS_RX_OVERRUN_ERR	(1<<5)
#define SPI_STUS_RX_UNDERRUN_ERR	(1<<4)
#define SPI_STUS_TX_OVERRUN_ERR	(1<<3)
#define SPI_STUS_TX_UNDERRUN_ERR	(1<<2)
#define SPI_STUS_RX_FIFORDY	(1<<1)
#define SPI_STUS_TX_FIFORDY	(1<<0)

#define SPI_PACKET_CNT_DIS	(0<<16)
#define SPI_PACKET_CNT_EN	(1<<16)

#define SPI_PND_TX_UNDERRUN_CLR	(1<<4)
#define SPI_PND_TX_OVERRUN_CLR	(1<<3)
#define SPI_PND_RX_UNDERRUN_CLR	(1<<2)
#define SPI_PND_RX_OVERRUN_CLR	(1<<1)
#define SPI_PND_TRAILING_CLR	(1<<0)

#define SPI_SWAP_RX_HALF_WORD	(1<<7)
#define SPI_SWAP_RX_BYTE	(1<<6)
#define SPI_SWAP_RX_BIT		(1<<5)
#define SPI_SWAP_RX_EN		(1<<4)
#define SPI_SWAP_TX_HALF_WORD	(1<<3)
#define SPI_SWAP_TX_BYTE	(1<<2)
#define SPI_SWAP_TX_BIT		(1<<1)
#define SPI_SWAP_TX_EN		(1<<0)

#define SPI_FBCLK_0NS		(0<<0)
#define SPI_FBCLK_3NS		(1<<0)
#define SPI_FBCLK_6NS		(2<<0)
#define SPI_FBCLK_9NS		(3<<0)


#endif /* __ASM_ARCH_REGS_SPI_H */

