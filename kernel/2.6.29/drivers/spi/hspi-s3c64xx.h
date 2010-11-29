/* ------------------------------------------------------------------------- */
/* 									     */
/* spi-s3c6400.h - definitions of s3c6400 specific spi interface	     */
/* 									     */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 2006 Samsung Electronics Co. ltd.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.		     */
/* ------------------------------------------------------------------------- */

#ifndef _S3C6400_SPI_H
#define _S3C6400_SPI_H

#include <asm/dma.h>
#include <plat/dma.h>

#define SPI_CHANNEL 0

#if(SPI_CHANNEL==0)
/* SPI CHANNEL 0 */
#define S3C_SPI_TX_DATA_REG	0x7F00B018  //SPI TX data
#define S3C_SPI_RX_DATA_REG	0x7F00B01C  //SPI RX data
#else
/* SPI CHANNEL 1 */
#define S3C_SPI_TX_DATA_REG	0x7F00C018  //SPI TX data
#define S3C_SPI_RX_DATA_REG	0x7F00C01C  //SPI RX data
#endif

/* DMA channel to be used for the SPI interface. */
#define S3C_SPI_DMA		0	

/* DMA transfer unit (byte). */
#define S3C_DMA_XFER_BYTE   	1
#define S3C_DMA_XFER_WORD	4	

/* DMA configuration setup byte. */
#define S3C_DCON_SPI1		(S3C_DCON_HANDSHAKE | S3C_DCON_SYNC_PCLK)

/* DMA hardware configuration mode (DISRCC register). */
#define S3C_SPI1_DMA_HWCFG	3
#define S3C_SPI_DMA_HWCFG	3

#define	DMA_BUFFER_SIZE		1500

/* spi controller state */
int req_dma_flag = 1;
enum s3c_spi_state {
	STATE_IDLE,
	STATE_XFER_TX,
	STATE_XFER_RX,
	STATE_STOP
};

static struct s3c2410_dma_client s3c6400spi_dma_client = {
	.name		= "s3c6400-spi-dma",
};

struct s3c_spi {
	spinlock_t		lock;
	struct semaphore 	sem;
	int 			nr;
	int			dma;
	u_int			subchannel;/* user fragment index */
	dma_addr_t		dmabuf_addr;

	struct spi_msg		*msg;
	unsigned int		msg_num;
	unsigned int		msg_idx;
	unsigned int		msg_ptr;
	unsigned int		msg_rd_ptr;

	enum s3c_spi_state	state;

	void __iomem		*regs;
	struct clk		*clk;
	struct device		*dev;
	struct resource		*irq;
	struct resource		*ioarea;
	struct spi_dev		spidev;
};


#endif /* _S3C6400_SPI_H */
