/*
 * spi_s3c.h - Samsung SOC SPI controller driver.
 *
 * Copyright (C) 2009 Samsung Electronics Ltd.
 */

#ifndef _LINUX_SPI_S3C_H
#define _LINUX_SPI_S3C_H

#include <mach/s3c-dma.h>
#include <mach/map.h>
#include <mach/gpio.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/spi.h>

/* AP specific headers */
#if defined(CONFIG_CPU_S3C6410)
#elif defined(CONFIG_CPU_S5P6440)
#elif defined(CONFIG_CPU_S5PC100)
#include <plat/gpio-bank-c.h>
#endif

#define S3C_SPI_CH_CFG		(0x00)      //SPI configuration
#define S3C_SPI_CLK_CFG		(0x04)      //Clock configuration
#define S3C_SPI_MODE_CFG		(0x08)      //SPI FIFO control
#define S3C_SPI_SLAVE_SEL	(0x0C)      //Slave selection
#define S3C_SPI_INT_EN	(0x10)      //SPI interrupt enable
#define S3C_SPI_STATUS	(0x14)      //SPI status
#define S3C_SPI_TX_DATA	(0x18)      //SPI TX data
#define S3C_SPI_RX_DATA	(0x1C)      //SPI RX data
#define S3C_SPI_PACKET_CNT	(0x20)      //count how many data master gets
#define S3C_SPI_PENDING_CLR	(0x24)      //Pending clear
#define S3C_SPI_SWAP_CFG		(0x28)      //SWAP config register
#define S3C_SPI_FB_CLK		(0x2c)      //SWAP FB config register

#define SPI_CH_HS_EN		(1<<6)	/* High Speed Enable */
#define SPI_CH_SW_RST		(1<<5)
#define SPI_CH_SLAVE		(1<<4)
#define SPI_CPOL_L		(1<<3)
#define SPI_CPHA_B		(1<<2)
#define SPI_CH_RXCH_ON		(1<<1)
#define SPI_CH_TXCH_ON		(1<<0)

#define SPI_CLKSEL_PCLK		(0<<9)
#define SPI_CLKSEL_USBCLK	(1<<9)
#define SPI_CLKSEL_ECLK		(2<<9)
#define SPI_CLKSEL_SRCMSK	(3<<9)
#define SPI_ENCLK_ENABLE	(1<<8)

#ifdef CONFIG_SPICLK_PCLK
#define SPI_CLKSEL_SRC	(0 << 9)
#elif defined (CONFIG_SPICLK_SPIEXT)
#define SPI_CLKSEL_SRC	(1 << 9)
#elif defined (CONFIG_SPICLK_SCLK48M)
#define SPI_CLKSEL_SRC	(1 << 9)
#elif defined (CONFIG_SPICLK_EPLL)
#define SPI_CLKSEL_SRC	(2 << 9)
#endif

#define SPI_MODE_CH_TSZ_BYTE		(0<<29)
#define SPI_MODE_CH_TSZ_HALFWORD	(1<<29)
#define SPI_MODE_CH_TSZ_WORD		(2<<29)
#define SPI_MODE_BUS_TSZ_BYTE		(0<<17)
#define SPI_MODE_BUS_TSZ_HALFWORD	(1<<17)
#define SPI_MODE_BUS_TSZ_WORD		(2<<17)
#define SPI_MODE_RXDMA_ON		(1<<2)
#define SPI_MODE_TXDMA_ON		(1<<1)
#define SPI_MODE_4BURST		(1<<0)

#define SPI_SLAVE_AUTO		(1<<1)
#define SPI_SLAVE_SIG_INACT	(1<<0)

#define SPI_INT_TRAILING_EN	(1<<6)
#define SPI_INT_RX_OVERRUN_EN	(1<<5)
#define SPI_INT_RX_UNDERRUN_EN	(1<<4)
#define SPI_INT_TX_OVERRUN_EN	(1<<3)
#define SPI_INT_TX_UNDERRUN_EN	(1<<2)
#define SPI_INT_RX_FIFORDY_EN	(1<<1)
#define SPI_INT_TX_FIFORDY_EN	(1<<0)

#define SPI_STUS_TX_DONE		(1<<21)
#define SPI_STUS_TRAILCNT_ZERO		(1<<20)
#define SPI_STUS_RX_OVERRUN_ERR		(1<<5)
#define SPI_STUS_RX_UNDERRUN_ERR	(1<<4)
#define SPI_STUS_TX_OVERRUN_ERR		(1<<3)
#define SPI_STUS_TX_UNDERRUN_ERR	(1<<2)
#define SPI_STUS_RX_FIFORDY		(1<<1)
#define SPI_STUS_TX_FIFORDY		(1<<0)

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

#if defined(CONFIG_CPU_S3C6410)
#define CH0_TX_MAXBYTES		   (64)
#define CH0_RX_MAXBYTES		   (64)
#define CH0_PER_UNIT               (1)
#elif defined(CONFIG_CPU_S5P6440)
#define CH0_TX_MAXBYTES		   (256)
#define CH0_RX_MAXBYTES		   (256)
#define CH0_PER_UNIT               (4)
#elif defined(CONFIG_CPU_S5PC100)
#define CH0_TX_MAXBYTES		   (64)
#define CH0_RX_MAXBYTES		   (64)
#define CH0_PER_UNIT               (1)
#endif

#define CH1_TX_MAXBYTES	           (64)
#define CH1_RX_MAXBYTES		   (64)
#define CH1_PER_UNIT               (4)
#define SPI_CH0_TXFIFO_MAXLEN	(CH0_TX_MAXBYTES / CH0_PER_UNIT - 1)
#define SPI_CH0_RXFIFO_MAXLEN	(CH0_RX_MAXBYTES / CH0_PER_UNIT - 1)
#define SPI_CH0_TXFLEN_OFF		(5)
#define SPI_CH0_RXFLEN_OFF		(11)
#define SPI_CH1_TXFIFO_MAXLEN	(CH1_TX_MAXBYTES / CH1_PER_UNIT - 1)
#define SPI_CH1_RXFIFO_MAXLEN	(CH1_RX_MAXBYTES / CH1_PER_UNIT - 1)
#define SPI_CH1_TXFLEN_OFF		(5)
#define SPI_CH1_RXFLEN_OFF		(11)
#define SPI_MAX_TRAILCNT		(0x3ff)
#define SPI_TRAILCNT_OFF		(19)

#define SPI_CH0_TXFIFO_LEN		SPI_CH0_TXFIFO_MAXLEN
#define SPI_CH0_RXFIFO_LEN		SPI_CH0_RXFIFO_MAXLEN
#define SPI_CH1_TXFIFO_LEN		SPI_CH1_TXFIFO_MAXLEN
#define SPI_CH1_RXFIFO_LEN		SPI_CH1_RXFIFO_MAXLEN
#define SPI_TRAILCNT			SPI_MAX_TRAILCNT

#if defined(CONFIG_CPU_S3C6410)

#define S3C_SPI_PA_SPI0 S3C64XX_PA_SPI0
#define S3C_SPI_PA_SPI1 S3C64XX_PA_SPI1

#elif defined(CONFIG_CPU_S5P6440)

#define S3C_SPI_PA_SPI0 S5P64XX_PA_SPI0
#define S3C_SPI_PA_SPI1 S5P64XX_PA_SPI1

#elif defined(CONFIG_CPU_S5PC100)

#define S3C_SPI_PA_SPI0 S5PC1XX_PA_SPI0
#define S3C_SPI_PA_SPI1 S5PC1XX_PA_SPI1
//#define S3C_SPI_PA_SPI2 S5PC1XX_PA_SPI2

#endif

#define DMACH_SPIIN_0        DMACH_SPI0_IN
#define DMACH_SPIOUT_0       DMACH_SPI0_OUT
#define DMACH_SPIIN_1        DMACH_SPI1_IN
#define DMACH_SPIOUT_1       DMACH_SPI1_OUT

#define GPMISO_0             0
#define GPCLK_0              1
#define GPMOSI_0             2
#define GPCS_0               3
#define GPMISO_1             4
#define GPCLK_1              5
#define GPMOSI_1             6
#define GPCS_1               7

#if defined(CONFIG_CPU_S3C6410)

#define GPNAME               S3C64XX_GPC
#define GPIO_MISO_0          S3C64XX_GPC0_SPI_MISO0
#define GPIO_CLK_0           S3C64XX_GPC1_SPI_CLK0
#define GPIO_MOSI_0          S3C64XX_GPC2_SPI_MOSI0
#define GPIO_CS_0            S3C64XX_GPC3_SPI_nCS0
#define GPIO_MISO_1          S3C64XX_GPC4_SPI_MISO1
#define GPIO_CLK_1           S3C64XX_GPC5_SPI_CLK1
#define GPIO_MOSI_1          S3C64XX_GPC6_SPI_MOSI1
#define GPIO_CS_1            S3C64XX_GPC7_SPI_nCS1

#elif defined(CONFIG_CPU_S5P6440)

#define GPNAME               S5P64XX_GPC
#define GPIO_MISO_0          S5P64XX_GPC0_SPI_MISO0
#define GPIO_CLK_0           S5P64XX_GPC1_SPI_CLK0
#define GPIO_MOSI_0          S5P64XX_GPC2_SPI_MOSI0
#define GPIO_CS_0            S5P64XX_GPC3_SPI_nCS0
#define GPIO_MISO_1          S5P64XX_GPC4_SPI_MISO1
#define GPIO_CLK_1           S5P64XX_GPC5_SPI_CLK1
#define GPIO_MOSI_1          S5P64XX_GPC6_SPI_MOSI1
#define GPIO_CS_1            S5P64XX_GPC7_SPI_nCS1

#elif defined(CONFIG_CPU_S5PC100)

//#define DMACH_SPIIN_2        DMACH_SPI2_IN
//#define DMACH_SPIOUT_2       DMACH_SPI2_OUT
#define GPNAME               S5PC1XX_GPB
#define GPIO_MISO_0          S5PC1XX_GPB0_SPI_MISO0
#define GPIO_CLK_0           S5PC1XX_GPB1_SPI_CLK0
#define GPIO_MOSI_0          S5PC1XX_GPB2_SPI_MOSI0
#define GPIO_CS_0            S5PC1XX_GPB3_SPI_CS0
#define GPIO_MISO_1          S5PC1XX_GPB4_SPI_MISO1
#define GPIO_CLK_1           S5PC1XX_GPB5_SPI_CLK1
#define GPIO_MOSI_1          S5PC1XX_GPB6_SPI_MOSI1
#define GPIO_CS_1            S5PC1XX_GPB7_SPI_CS1
//#define GPIO_MISO_2          S5PC1XX_GPG3_2SPI_MISO2
//#define GPIO_CLK_2           S5PC1XX_GPG3_0SPI_CLK2
//#define GPIO_MOSI_2          S5PC1XX_GPG3_3SPI_MOSI2
//#define GPIO_CS_2            S5PC1XX_GPG3_1SPI_CS2
//#define GPMISO_2             4
//#define GPCLK_2              5
//#define GPMOSI_2             6
//#define GPCS_2               7

#endif

/* Going by the AP, SPI controller, in Master mode, can control only 1 Slave.
 * Inorder to control more Slaves, we disable ChipSelect(CS) pin for Master altogether.
 * ChipSelects are defined as GPIO pins and set/config functions are provided by plat-specific code.
 * So, here we set CS only if SPI controller is in SLAVE mode.
 */
#define SETUP_SPI(sspi, n)	do{                                                             \
				   sspi->sfr_phyaddr = S3C_SPI_PA_SPI##n;                        \
				   sspi->rx_dmach = DMACH_SPIIN_##n;                            \
				   sspi->tx_dmach = DMACH_SPIOUT_##n;                           \
				   s3c_gpio_cfgpin(GPNAME(GPMISO_##n), GPIO_MISO_##n);          \
				   s3c_gpio_cfgpin(GPNAME(GPCLK_##n), GPIO_CLK_##n);            \
				   s3c_gpio_cfgpin(GPNAME(GPMOSI_##n), GPIO_MOSI_##n);          \
                   s3c_gpio_cfgpin(GPNAME(GPCS_##n), GPIO_CS_##n);              \
				}while(0)

#define SET_GPIOPULL(sspi, n)	do{                                                                     \
				   if(sspi->cur_mode & SPI_SLAVE){				        \
					   s3c_gpio_setpull(GPNAME(GPMISO_##n), S3C_GPIO_PULL_DOWN);    \
					   s3c_gpio_setpull(GPNAME(GPCLK_##n), S3C_GPIO_PULL_NONE);     \
					   s3c_gpio_setpull(GPNAME(GPMOSI_##n), S3C_GPIO_PULL_DOWN);    \
					   s3c_gpio_setpull(GPNAME(GPCS_##n), S3C_GPIO_PULL_NONE);      \
				   }else{								\
					   s3c_gpio_setpull(GPNAME(GPMISO_##n), S3C_GPIO_PULL_UP);    \
					   s3c_gpio_setpull(GPNAME(GPCLK_##n), S3C_GPIO_PULL_UP);     \
					   s3c_gpio_setpull(GPNAME(GPMOSI_##n), S3C_GPIO_PULL_UP);    \
					   s3c_gpio_setpull(GPNAME(GPCS_##n), S3C_GPIO_PULL_UP);      \
				   }									\
				}while(0)

#define S3C_SETGPIOPULL(sspi)   do{                                          \
				   if(sspi->spi_mstinfo->pdev->id == 0)      \
					SET_GPIOPULL(sspi, 0);               \
				   else if(sspi->spi_mstinfo->pdev->id == 1) \
					SET_GPIOPULL(sspi, 1);               \
				}while(0)

#define UNSET_GPIOPULL(sspi, n)	do{                                                                     \
				   if(sspi->cur_mode & SPI_SLAVE){				        \
					   s3c_gpio_setpull(GPNAME(GPMISO_##n), S3C_GPIO_PULL_UP);    \
					   s3c_gpio_setpull(GPNAME(GPCLK_##n), S3C_GPIO_PULL_UP);     \
					   s3c_gpio_setpull(GPNAME(GPMOSI_##n), S3C_GPIO_PULL_UP);    \
					   s3c_gpio_setpull(GPNAME(GPCS_##n), S3C_GPIO_PULL_UP);      \
				   }else{								\
					   s3c_gpio_setpull(GPNAME(GPMISO_##n), S3C_GPIO_PULL_UP);    \
					   s3c_gpio_setpull(GPNAME(GPCLK_##n), S3C_GPIO_PULL_UP);     \
					   s3c_gpio_setpull(GPNAME(GPMOSI_##n), S3C_GPIO_PULL_UP);    \
					   s3c_gpio_setpull(GPNAME(GPCS_##n), S3C_GPIO_PULL_UP);      \
				   }									\
				}while(0)

#define S3C_UNSETGPIOPULL(sspi)   do{                                          \
				   if(sspi->spi_mstinfo->pdev->id == 0)      \
					UNSET_GPIOPULL(sspi, 0);               \
				   else if(sspi->spi_mstinfo->pdev->id == 1) \
					UNSET_GPIOPULL(sspi, 1);               \
				}while(0)

#define SET_MODECFG(v, n)   do{                                                                      \
                               v &= ~((SPI_CH##n##_TXFIFO_MAXLEN << SPI_CH##n##_TXFLEN_OFF)          \
                                         | (SPI_CH##n##_RXFIFO_MAXLEN << SPI_CH##n##_RXFLEN_OFF));   \
                               v |= (SPI_CH##n##_TXFIFO_LEN << SPI_CH##n##_TXFLEN_OFF)               \
                                         | (SPI_CH##n##_RXFIFO_LEN << SPI_CH##n##_RXFLEN_OFF);       \
                            }while(0)

#define S3C_SPI_DMABUF_LEN	(16*1024)

#define SUSPND     (1<<0)
#define XFERBUSY   (1<<1)
#define IRQERR     (1<<2)
#define TOUTERR    (1<<3)
#define INTERR     (1<<4)
#define IOERR      (1<<5)
#define ERRS       (IRQERR | TOUTERR | INTERR | IOERR)

enum xfer_state {
	PASS,
	FAIL,
	BUSY,
};

/* Structure for each SPI controller of Samsung SOC */
struct s3cspi_bus {
	struct spi_master        *master;
	struct workqueue_struct	 *workqueue;
	struct s3c_spi_mstr_info *spi_mstinfo;
	struct spi_device        *tgl_spi; /* Pointer to last spi device, if toggled by spi_transfer.cs_change flag */
	struct work_struct       work;
	struct list_head         queue;
	spinlock_t               lock;	/* protect 'queue' */
	atomic_t                 state;
	enum dma_ch              rx_dmach;
	enum dma_ch              tx_dmach;
	u32                      sfr_phyaddr;
	struct resource		 *irqres;
	struct resource		 *ioarea;
	struct resource 	 *iores;
	void __iomem             *regs;
	void __iomem             *tx_dma_cpu;
	void __iomem             *rx_dma_cpu;
	void __iomem             *rx_tmp;
	void __iomem             *tx_tmp;
	dma_addr_t               tx_dma_phys;
	dma_addr_t               rx_dma_phys;
	struct completion        xfer_completion;
	enum xfer_state          tx_done;
	enum xfer_state          rx_done;
	u32                      max_speed;
	u32                      min_speed;
	/* Current parameters of the controller due to request from active transfer */
	u8 	                 cur_mode, cur_bpw, active_chip;
	u32                      cur_speed;
};

#endif 	//_LINUX_SPI_S3C_H
