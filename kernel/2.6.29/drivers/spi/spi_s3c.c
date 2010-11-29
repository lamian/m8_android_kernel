/*
 * spi_s3c.c - Samsung SOC SPI controller driver.
 * By -- Jaswinder Singh <jassi.brar@samsung.com>
 *
 * Copyright (C) 2009 Samsung Electronics Ltd.
 */

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/spi/spi.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <asm/dma.h>

#include "spi_s3c.h"

//#define DEBUGSPI

#ifdef DEBUGSPI

#define dbg_printk(x...)	printk(x)

static void dump_regs(struct s3cspi_bus *sspi)
{
	u32 val;

	val = readl(sspi->regs + S3C_SPI_CH_CFG);
	printk("CHN-%08X ", val);
	val = readl(sspi->regs + S3C_SPI_CLK_CFG);
	printk("CLK-%08X", val);
	val = readl(sspi->regs + S3C_SPI_MODE_CFG);
	printk("MOD-%08X", val);
	val = readl(sspi->regs + S3C_SPI_SLAVE_SEL);
	printk("SLVSEL-%08X", val);
	val = readl(sspi->regs + S3C_SPI_STATUS);
	if(val & SPI_STUS_TX_DONE)
	   printk("TX_done ");
	if(val & SPI_STUS_TRAILCNT_ZERO)
	   printk("TrailZ ");
	if(val & SPI_STUS_RX_OVERRUN_ERR)
	   printk("RX_Ovrn ");
	if(val & SPI_STUS_RX_UNDERRUN_ERR)
	   printk("Rx_Unrn ");
	if(val & SPI_STUS_TX_OVERRUN_ERR)
	   printk("Tx_Ovrn ");
	if(val & SPI_STUS_TX_UNDERRUN_ERR)
	   printk("Tx_Unrn ");
	if(val & SPI_STUS_RX_FIFORDY)
	   printk("Rx_Rdy ");
	if(val & SPI_STUS_TX_FIFORDY)
	   printk("Tx_Rdy ");
	printk("Rx/TxLvl=%d,%d\n", (val>>13)&0x7f, (val>>6)&0x7f);
}

static void dump_spidevice_info(struct spi_device *spi)
{
	dbg_printk("Modalias = %s\n", spi->modalias);
	dbg_printk("Slave-%d on Bus-%d\n", spi->chip_select, spi->master->bus_num);
	dbg_printk("max_speed_hz = %d\n", spi->max_speed_hz);
	dbg_printk("bits_per_word = %d\n", spi->bits_per_word);
	dbg_printk("irq = %d\n", spi->irq);
	dbg_printk("Clk Phs = %d\n", spi->mode & SPI_CPHA);
	dbg_printk("Clk Pol = %d\n", spi->mode & SPI_CPOL);
	dbg_printk("ActiveCS = %s\n", (spi->mode & (1<<2)) ? "high" : "low" );
	dbg_printk("Our Mode = %s\n", (spi->mode & SPI_SLAVE) ? "Slave" : "Master");
}
#else

#define dbg_printk(x...)		/**/
#define dump_regs(sspi) 		/**/
#define dump_spidevice_info(spi) 	/**/

#endif

static struct s3c2410_dma_client s3c_spi_dma_client = {
	.name = "s3cspi-dma",
};

static inline void enable_spidma(struct s3cspi_bus *sspi, struct spi_transfer *xfer)
{
	u32 val;

	val = readl(sspi->regs + S3C_SPI_MODE_CFG);
	val &= ~(SPI_MODE_TXDMA_ON | SPI_MODE_RXDMA_ON);
	if(xfer->tx_buf != NULL)
	   val |= SPI_MODE_TXDMA_ON;
	if(xfer->rx_buf != NULL)
	   val |= SPI_MODE_RXDMA_ON;
	writel(val, sspi->regs + S3C_SPI_MODE_CFG);
}

static inline void flush_dma(struct s3cspi_bus *sspi, struct spi_transfer *xfer)
{
	if(xfer->tx_buf != NULL)
	   s3c2410_dma_ctrl(sspi->tx_dmach, S3C2410_DMAOP_FLUSH);
	if(xfer->rx_buf != NULL)
	   s3c2410_dma_ctrl(sspi->rx_dmach, S3C2410_DMAOP_FLUSH);
}

static inline void flush_spi(struct s3cspi_bus *sspi)
{
	u32 val;

	val = readl(sspi->regs + S3C_SPI_CH_CFG);
	val |= SPI_CH_SW_RST;
	val &= ~SPI_CH_HS_EN;
	if((sspi->cur_speed > 30000000UL) && !(sspi->cur_mode & SPI_SLAVE)) /* TODO ??? */
	   val |= SPI_CH_HS_EN;
	writel(val, sspi->regs + S3C_SPI_CH_CFG);

	/* Flush TxFIFO*/
	do{
	   val = readl(sspi->regs + S3C_SPI_STATUS);
	   val = (val>>6) & 0x7f;
	}while(val);

	/* Flush RxFIFO*/
	val = readl(sspi->regs + S3C_SPI_STATUS);
	val = (val>>13) & 0x7f;
	while(val){
	   readl(sspi->regs + S3C_SPI_RX_DATA);
	   val = readl(sspi->regs + S3C_SPI_STATUS);
	   val = (val>>13) & 0x7f;
	}

	val = readl(sspi->regs + S3C_SPI_CH_CFG);
	val &= ~SPI_CH_SW_RST;
	writel(val, sspi->regs + S3C_SPI_CH_CFG);
}

static inline void enable_spichan(struct s3cspi_bus *sspi, struct spi_transfer *xfer)
{
	u32 val;

	val = readl(sspi->regs + S3C_SPI_CH_CFG);
	val &= ~(SPI_CH_RXCH_ON | SPI_CH_TXCH_ON);
	if(xfer->tx_buf != NULL){
	   val |= SPI_CH_TXCH_ON;
	}
	if(xfer->rx_buf != NULL){
	   if(!(sspi->cur_mode & SPI_SLAVE)){
	      writel((xfer->len & 0xffff) | SPI_PACKET_CNT_EN, 
			sspi->regs + S3C_SPI_PACKET_CNT); /* XXX TODO Bytes or number of SPI-Words? */
	   }
	   val |= SPI_CH_RXCH_ON;
	}
	writel(val, sspi->regs + S3C_SPI_CH_CFG);
}

static inline void enable_spiintr(struct s3cspi_bus *sspi, struct spi_transfer *xfer)
{
	u32 val = 0;

	if(xfer->tx_buf != NULL){
	   val |= SPI_INT_TX_OVERRUN_EN;
	   if(!(sspi->cur_mode & SPI_SLAVE))
	      val |= SPI_INT_TX_UNDERRUN_EN;
	}
	if(xfer->rx_buf != NULL){
	   val |= (SPI_INT_RX_UNDERRUN_EN | SPI_INT_RX_OVERRUN_EN | SPI_INT_TRAILING_EN);
	}
	writel(val, sspi->regs + S3C_SPI_INT_EN);
}

static inline void enable_spienqueue(struct s3cspi_bus *sspi, struct spi_transfer *xfer)
{
	if(xfer->rx_buf != NULL){
	   sspi->rx_done = BUSY;
	   s3c2410_dma_config(sspi->rx_dmach, sspi->cur_bpw/8, 0);
	   s3c2410_dma_enqueue(sspi->rx_dmach, (void *)sspi, xfer->rx_dma, xfer->len);
	}
	if(xfer->tx_buf != NULL){
	   sspi->tx_done = BUSY;
	   s3c2410_dma_config(sspi->tx_dmach, sspi->cur_bpw/8, 0);
	   s3c2410_dma_enqueue(sspi->tx_dmach, (void *)sspi, xfer->tx_dma, xfer->len);
	}
}

static inline void enable_cs(struct s3cspi_bus *sspi, struct spi_device *spi)
{
	u32 val;
	struct s3c_spi_pdata *spd;

	if(sspi->tgl_spi != NULL){ /* If last device toggled after mssg */
	   if(sspi->tgl_spi != spi){ /* if last mssg on diff device */
	      /* Deselect the last toggled device */
	      spd = &sspi->spi_mstinfo->spd[sspi->tgl_spi->chip_select];
#if 0
	      if(sspi->tgl_spi->mode & SPI_CS_HIGH){
	         spd->cs_set(spd->cs_pin, CS_LOW);
	         spd->cs_level = CS_LOW;
	      }else{
	         spd->cs_set(spd->cs_pin, CS_HIGH);
	         spd->cs_level = CS_HIGH;
	      }
#endif
	   }
	   sspi->tgl_spi = NULL;
	}

	spd = &sspi->spi_mstinfo->spd[spi->chip_select];
	val = readl(sspi->regs + S3C_SPI_SLAVE_SEL);

	if(sspi->cur_mode & SPI_SLAVE){
	   val |= SPI_SLAVE_AUTO; /* Auto Mode */
	   val |= SPI_SLAVE_SIG_INACT;
	}else{ /* Master Mode */
	   val &= ~SPI_SLAVE_AUTO; /* Manual Mode */
	   val &= ~SPI_SLAVE_SIG_INACT; /* Activate CS */
#if 0
	   if(spi->mode & SPI_CS_HIGH){
	      spd->cs_set(spd->cs_pin, CS_HIGH);
	      spd->cs_level = CS_HIGH;
	   }else{
	      spd->cs_set(spd->cs_pin, CS_LOW);
	      spd->cs_level = CS_LOW;
	   }
#endif
	}

	writel(val, sspi->regs + S3C_SPI_SLAVE_SEL);
}

static inline void disable_cs(struct s3cspi_bus *sspi, struct spi_device *spi)
{
	u32 val;
	struct s3c_spi_pdata *spd = &sspi->spi_mstinfo->spd[spi->chip_select];

	if(sspi->tgl_spi == spi)
	   sspi->tgl_spi = NULL;

	val = readl(sspi->regs + S3C_SPI_SLAVE_SEL);

	if(sspi->cur_mode & SPI_SLAVE){
	   val |= SPI_SLAVE_AUTO; /* Auto Mode */
	}else{ /* Master Mode */
	   val &= ~SPI_SLAVE_AUTO; /* Manual Mode */
	   val |= SPI_SLAVE_SIG_INACT; /* DeActivate CS */
#if 0
	   if(spi->mode & SPI_CS_HIGH){
	      spd->cs_set(spd->cs_pin, CS_LOW);
	      spd->cs_level = CS_LOW;
	   }else{
	      spd->cs_set(spd->cs_pin, CS_HIGH);
	      spd->cs_level = CS_HIGH;
	   }
#endif
	}

	writel(val, sspi->regs + S3C_SPI_SLAVE_SEL);
}

static inline void set_polarity(struct s3cspi_bus *sspi)
{
	u32 val;

	val = readl(sspi->regs + S3C_SPI_CH_CFG);
	val &= ~(SPI_CH_SLAVE | SPI_CPOL_L | SPI_CPHA_B);
	if(sspi->cur_mode & SPI_SLAVE)
	   val |= SPI_CH_SLAVE;
	if(sspi->cur_mode & SPI_CPOL)
	   val |= SPI_CPOL_L;
	if(sspi->cur_mode & SPI_CPHA)
	   val |= SPI_CPHA_B;
	writel(val, sspi->regs + S3C_SPI_CH_CFG);
}

static inline void set_clock(struct s3cspi_bus *sspi)
{
	u32 val;
	struct s3c_spi_mstr_info *smi = sspi->spi_mstinfo;

	val = readl(sspi->regs + S3C_SPI_CLK_CFG);
	val &= ~(SPI_CLKSEL_SRCMSK | SPI_ENCLK_ENABLE | 0xff);
	val |= SPI_CLKSEL_SRC;
	if(!(sspi->cur_mode & SPI_SLAVE)){
	   val |= ((smi->spiclck_getrate(smi) / sspi->cur_speed / 2 - 1) << 0);	// PCLK and PSR
	   val |= SPI_ENCLK_ENABLE;
	}
	writel(val, sspi->regs + S3C_SPI_CLK_CFG);
}

static inline void set_dmachan(struct s3cspi_bus *sspi)
{
	u32 val;

	val = readl(sspi->regs + S3C_SPI_MODE_CFG);
	val &= ~((0x3<<17) | (0x3<<29));
	if(sspi->cur_bpw == 8){
	   val |= SPI_MODE_CH_TSZ_BYTE;
	   val |= SPI_MODE_BUS_TSZ_BYTE;
	}else if(sspi->cur_bpw == 16){
	   val |= SPI_MODE_CH_TSZ_HALFWORD;
	   val |= SPI_MODE_BUS_TSZ_HALFWORD;
	}else if(sspi->cur_bpw == 32){
	   val |= SPI_MODE_CH_TSZ_WORD;
	   val |= SPI_MODE_BUS_TSZ_WORD;
	}else{
	   printk("Invalid Bits/Word!\n");
	}
	val &= ~(SPI_MODE_4BURST | SPI_MODE_TXDMA_ON | SPI_MODE_RXDMA_ON);
	writel(val, sspi->regs + S3C_SPI_MODE_CFG);
}

static void s3c_spi_config(struct s3cspi_bus *sspi)
{
	/* Set Polarity and Phase */
	set_polarity(sspi);

	/* Set Channel & DMA Mode */
	set_dmachan(sspi);
}

static void s3c_spi_hwinit(struct s3cspi_bus *sspi, int channel)
{
	unsigned int val;
	struct s3c_spi_mstr_info *smi = sspi->spi_mstinfo;

	writel(SPI_SLAVE_SIG_INACT, sspi->regs + S3C_SPI_SLAVE_SEL);

	/* Disable Interrupts */
	writel(0, sspi->regs + S3C_SPI_INT_EN);

	if(smi->set_drvst)
	   smi->set_drvst(smi->pdev->id, PORT_STRENGTH(4) | CLK_STRENGTH(2));

	writel(SPI_CLKSEL_SRC, sspi->regs + S3C_SPI_CLK_CFG);
	writel(0, sspi->regs + S3C_SPI_MODE_CFG);
	writel(SPI_SLAVE_SIG_INACT, sspi->regs + S3C_SPI_SLAVE_SEL);
	writel(0, sspi->regs + S3C_SPI_PACKET_CNT);
	writel(readl(sspi->regs + S3C_SPI_PENDING_CLR), sspi->regs + S3C_SPI_PENDING_CLR);
	writel(SPI_FBCLK_0NS, sspi->regs + S3C_SPI_FB_CLK);

	flush_spi(sspi);

	writel(0, sspi->regs + S3C_SPI_SWAP_CFG);
	writel(SPI_FBCLK_9NS, sspi->regs + S3C_SPI_FB_CLK);

	val = readl(sspi->regs + S3C_SPI_MODE_CFG);
	val &= ~(SPI_MAX_TRAILCNT << SPI_TRAILCNT_OFF);
	if(channel == 0)
	   SET_MODECFG(val, 0);
	else 
	   SET_MODECFG(val, 1);
	val |= (SPI_TRAILCNT << SPI_TRAILCNT_OFF);
	writel(val, sspi->regs + S3C_SPI_MODE_CFG);
}

static irqreturn_t s3c_spi_interrupt(int irq, void *dev_id)
{
	u32 val;
	struct s3cspi_bus *sspi = (struct s3cspi_bus *)dev_id;

	dump_regs(sspi);
	val = readl(sspi->regs + S3C_SPI_PENDING_CLR);
	dbg_printk("PENDING=%x\n", val);
	writel(val, sspi->regs + S3C_SPI_PENDING_CLR);

	/* We get interrupted only for bad news */
	if(sspi->tx_done != PASS){
	   sspi->tx_done = FAIL;
	}
	if(sspi->rx_done != PASS){
	   sspi->rx_done = FAIL;
	}

	/* No need to spinlock, as called in IRQ_DISABLED mode */
	atomic_set(&sspi->state, atomic_read(&sspi->state) | IRQERR);

	complete(&sspi->xfer_completion);

	return IRQ_HANDLED;
}

void s3c_spi_dma_rxcb(struct s3c2410_dma_chan *chan, void *buf_id, int size, enum s3c2410_dma_buffresult res)
{
	struct s3cspi_bus *sspi = (struct s3cspi_bus *)buf_id;

	if(res == S3C2410_RES_OK){
	   sspi->rx_done = PASS;
	   dbg_printk("DmaRx-%d ", size);
	}else{
	   sspi->rx_done = FAIL;
	   dbg_printk("DmaAbrtRx-%d ", size);
	}

	if(sspi->tx_done != BUSY && !(atomic_read(&sspi->state) & IRQERR)) /* If other done and all OK */
	   complete(&sspi->xfer_completion);
}

void s3c_spi_dma_txcb(struct s3c2410_dma_chan *chan, void *buf_id, int size, enum s3c2410_dma_buffresult res)
{
	struct s3cspi_bus *sspi = (struct s3cspi_bus *)buf_id;

	if(res == S3C2410_RES_OK){
	   sspi->tx_done = PASS;
	   dbg_printk("DmaTx-%d ", size);
	}else{
	   sspi->tx_done = FAIL;
	   dbg_printk("DmaAbrtTx-%d ", size);
	}

	if(sspi->rx_done != BUSY && !(atomic_read(&sspi->state) & IRQERR)) /* If other done and all OK */
	   complete(&sspi->xfer_completion);
}

static int wait_for_txshiftout(struct s3cspi_bus *sspi, unsigned long t)
{
	unsigned long timeout;

	timeout = jiffies + t;
	while((__raw_readl(sspi->regs + S3C_SPI_STATUS) >> 6) & 0x7f){
	   if(time_after(jiffies, timeout))
	      return -1;
	   cpu_relax();
	}
	return 0;
}

static int wait_for_xfer(struct s3cspi_bus *sspi, struct spi_transfer *xfer)
{
	int status;
	u32 val;

	val = msecs_to_jiffies(xfer->len / (sspi->min_speed / 8 / 1000)); /* time to xfer data at min. speed */
	if(sspi->cur_mode & SPI_SLAVE)
	   val += msecs_to_jiffies(5000); /* 5secs to switch on the Master */
	else
	   val += msecs_to_jiffies(10); /* just some more */
	status = wait_for_completion_interruptible_timeout(&sspi->xfer_completion, val);

	if(status == 0)
	   status = -ETIMEDOUT;
	else if(status == -ERESTARTSYS)
	   status = -EINTR;
	else if((sspi->tx_done != PASS) || (sspi->rx_done != PASS)) /* Some Xfer failed */
	   status = -EIO;
	else
	   status = 0;	/* All OK */

	/*
	 * DmaTx returns after simply writing data in the FIFO,
	 * w/o waiting for real transmission on the bus to finish.
	 * DmaRx returns only after Dma read data from FIFO which
	 * needs bus transmission to finish, so we don't worry if 
	 * Xfer involved Rx alone or with Tx.
	 */
	if(!status && (xfer->rx_buf == NULL)){
	   val = msecs_to_jiffies(xfer->len / (sspi->min_speed / 8 / 1000)); /* Be lenient */
	   val += msecs_to_jiffies(5000); /* 5secs to switch on the Master */
	   status = wait_for_txshiftout(sspi, val);
	   if(status == -1){ /* Time-out */
	      status = -ETIMEDOUT;
	   }else{ /* Still last byte on the bus */
	      status = 0;
	      udelay(1000000 / sspi->cur_speed * 8 + 5); /* time to xfer 1 byte at sspi->cur_speed plus 5 usecs extra */
	   }
	}

//	printk("%s returns %d\n", __FUNCTION__, status);
	return status;
}

/*  First, allocate from our preallocated DMA buffer.
 *  If preallocated DMA buffers are not big enough,
 *  then allocate new DMA Coherent buffers.
 */
static int s3c_spi_map_xfer(struct s3cspi_bus *sspi, struct spi_transfer *xfer)
{
	struct s3c_spi_mstr_info *smi = sspi->spi_mstinfo;
	struct device *dev = &smi->pdev->dev;

	sspi->rx_tmp = NULL;
	sspi->tx_tmp = NULL;

	if(xfer->len <= S3C_SPI_DMABUF_LEN){
	   if(xfer->rx_buf != NULL){
	      xfer->rx_dma = sspi->rx_dma_phys;
	      sspi->rx_tmp = (void *)sspi->rx_dma_cpu;
	   }
	   if(xfer->tx_buf != NULL){
	      xfer->tx_dma = sspi->tx_dma_phys;
	      sspi->tx_tmp = (void *)sspi->tx_dma_cpu;
	   }
	}else{
	   dbg_printk("If you plan to use this Xfer size often, increase S3C_SPI_DMABUF_LEN\n");
	   if(xfer->rx_buf != NULL){
	      sspi->rx_tmp = dma_alloc_coherent(dev, S3C_SPI_DMABUF_LEN, 
							&xfer->rx_dma, GFP_KERNEL | GFP_DMA);
		if(sspi->rx_tmp == NULL)
		   return -ENOMEM;
	   }
	   if(xfer->tx_buf != NULL){
	      sspi->tx_tmp = dma_alloc_coherent(dev, S3C_SPI_DMABUF_LEN, &xfer->tx_dma, GFP_KERNEL | GFP_DMA);
		if(sspi->tx_tmp == NULL){
		   if(xfer->rx_buf != NULL)
		      dma_free_coherent(dev, S3C_SPI_DMABUF_LEN, sspi->rx_tmp, xfer->rx_dma);
		   return -ENOMEM;
		}
	   }
	}

	if(xfer->tx_buf != NULL) {
#if 0
	   int i;
	   printk("tx_buf dump: ");
	   for (i=0; i<xfer->len; i++) {
	   	printk("%02X ", *((unsigned char*)xfer->tx_buf+i));
	   }
	   printk("\n");
#endif
	   memcpy(sspi->tx_tmp, xfer->tx_buf, xfer->len);
	}

	return 0;
}

static void s3c_spi_unmap_xfer(struct s3cspi_bus *sspi, struct spi_transfer *xfer)
{
	struct s3c_spi_mstr_info *smi = sspi->spi_mstinfo;
	struct device *dev = &smi->pdev->dev;

	if((xfer->rx_buf != NULL) && (sspi->rx_tmp != NULL))
	   memcpy(xfer->rx_buf, sspi->rx_tmp, xfer->len);

	if(xfer->len > S3C_SPI_DMABUF_LEN){
	   if(xfer->rx_buf != NULL)
	      dma_free_coherent(dev, S3C_SPI_DMABUF_LEN, sspi->rx_tmp, xfer->rx_dma);
	   if(xfer->tx_buf != NULL)
	      dma_free_coherent(dev, S3C_SPI_DMABUF_LEN, sspi->tx_tmp, xfer->tx_dma);
	}else{
	   sspi->rx_tmp = NULL;
	   sspi->tx_tmp = NULL;
	}

	/* Restore to t/rx_dma pointers */
	if(xfer->rx_buf != NULL)
	   xfer->rx_dma = 0;
	if(xfer->tx_buf != NULL)
	   xfer->tx_dma = 0;
}

static void handle_msg(struct s3cspi_bus *sspi, struct spi_message *msg)
{
	u8 bpw;
	u32 speed, val;
	int status = 0;
	struct spi_transfer *xfer;
	struct spi_device *spi = msg->spi;

	/* Change pull-up only when switching between Master-Slave modes */
	if((sspi->cur_mode & SPI_SLAVE) ^ (spi->mode & SPI_SLAVE)) {
	   sspi->cur_mode &= ~SPI_SLAVE;
	   sspi->cur_mode |= (spi->mode & SPI_SLAVE);
	   S3C_SETGPIOPULL(sspi);
	}

	/* Write to regs only if necessary */
	if((sspi->cur_speed != spi->max_speed_hz)
		||(sspi->cur_mode != spi->mode)
		||(sspi->cur_bpw != spi->bits_per_word)) {
	   sspi->cur_bpw = spi->bits_per_word;
	   sspi->cur_speed = spi->max_speed_hz;
	   sspi->cur_mode = spi->mode;
	   s3c_spi_config(sspi);
	}

	list_for_each_entry (xfer, &msg->transfers, transfer_list) {

		if(!msg->is_dma_mapped && s3c_spi_map_xfer(sspi, xfer)){
		   printk("Xfer: Unable to allocate DMA buffer!\n");
		   status = -ENOMEM;
		   goto out;
		}

		INIT_COMPLETION(sspi->xfer_completion);

		/* Only BPW and Speed may change across transfers */
		bpw = xfer->bits_per_word ? : spi->bits_per_word;
		speed = xfer->speed_hz ? : spi->max_speed_hz;

		if(sspi->cur_bpw != bpw || sspi->cur_speed != speed){
			sspi->cur_bpw = bpw;
			sspi->cur_speed = speed;
			s3c_spi_config(sspi);
		}

		/* Pending only which is to be done */
		sspi->rx_done = PASS;
		sspi->tx_done = PASS;

		/* Configure Clock */
		set_clock(sspi);

		/* Enable Interrupts */
		enable_spiintr(sspi, xfer);

		if(!(sspi->cur_mode & SPI_SLAVE))
		   flush_spi(sspi);

		/* Enqueue data on DMA */
		enable_spienqueue(sspi, xfer);

		/* Enable DMA */
		enable_spidma(sspi, xfer);

		/* Enable TX/RX */
		enable_spichan(sspi, xfer);

		// dump_regs(sspi);

		/* Slave Select */
		enable_cs(sspi, spi);

		status = wait_for_xfer(sspi, xfer);

		/**************
		 * Block Here *
		 **************/

		if(status == -ETIMEDOUT){
		   printk("Xfer: Timeout!\n");
		   // dump_regs(sspi);
		   /* DMA Disable*/
		   val = readl(sspi->regs + S3C_SPI_MODE_CFG);
		   val &= ~(SPI_MODE_TXDMA_ON | SPI_MODE_RXDMA_ON);
		   writel(val, sspi->regs + S3C_SPI_MODE_CFG);
		   flush_dma(sspi, xfer);
		   flush_spi(sspi);
		   if(!msg->is_dma_mapped)
		      s3c_spi_unmap_xfer(sspi, xfer);
		   goto out;
		}
		if(status == -EINTR){
		   printk("Xfer: Interrupted!\n");
		   // dump_regs(sspi);
		   /* DMA Disable*/
		   val = readl(sspi->regs + S3C_SPI_MODE_CFG);
		   val &= ~(SPI_MODE_TXDMA_ON | SPI_MODE_RXDMA_ON);
		   writel(val, sspi->regs + S3C_SPI_MODE_CFG);
		   flush_dma(sspi, xfer);
		   flush_spi(sspi);
		   if(!msg->is_dma_mapped)
		      s3c_spi_unmap_xfer(sspi, xfer);
		   goto out;
		}
		if(status == -EIO){ /* Some Xfer failed */
		   printk("Xfer: Failed!\n");
		   // dump_regs(sspi);
		   /* DMA Disable*/
		   val = readl(sspi->regs + S3C_SPI_MODE_CFG);
		   val &= ~(SPI_MODE_TXDMA_ON | SPI_MODE_RXDMA_ON);
		   writel(val, sspi->regs + S3C_SPI_MODE_CFG);
		   flush_dma(sspi, xfer);
		   flush_spi(sspi);
		   if(!msg->is_dma_mapped)
		      s3c_spi_unmap_xfer(sspi, xfer);
		   goto out;
		}

		if(xfer->delay_usecs)
		   udelay(xfer->delay_usecs);

		if(!(sspi->cur_mode & SPI_SLAVE) && xfer->cs_change){
		   if(list_is_last(&xfer->transfer_list, &msg->transfers))  /* Hint that the next mssg is gonna be for the same device */
		      sspi->spi_mstinfo->spd[spi->chip_select].cs_level = CS_TOGGLE;
		   else
		      disable_cs(sspi, spi);
		}

		msg->actual_length += xfer->len;

		if(!msg->is_dma_mapped)
		   s3c_spi_unmap_xfer(sspi, xfer);
	}

out:
	/* Slave Deselect in Master mode only if _not_ hinted for next mssg to the same device */
	if(!(sspi->cur_mode & SPI_SLAVE)){
	   if((sspi->spi_mstinfo->spd[spi->chip_select].cs_level != CS_TOGGLE) || status){ /* De-select the device in case of some error */
	      disable_cs(sspi, spi);
	   }else{
	      val = readl(sspi->regs + S3C_SPI_SLAVE_SEL);
	      val &= ~SPI_SLAVE_AUTO; /* Manual Mode */
	      val |= SPI_SLAVE_SIG_INACT; /* DeActivate CS */
	      writel(val, sspi->regs + S3C_SPI_SLAVE_SEL);
	      sspi->tgl_spi = spi;
	   }
	}

	/* Disable Interrupts */
	writel(0, sspi->regs + S3C_SPI_INT_EN);

	/* Tx/Rx Disable */
	val = readl(sspi->regs + S3C_SPI_CH_CFG);
	val &= ~(SPI_CH_RXCH_ON | SPI_CH_TXCH_ON);
	writel(val, sspi->regs + S3C_SPI_CH_CFG);

	/* DMA Disable*/
	val = readl(sspi->regs + S3C_SPI_MODE_CFG);
	val &= ~(SPI_MODE_TXDMA_ON | SPI_MODE_RXDMA_ON);
	writel(val, sspi->regs + S3C_SPI_MODE_CFG);

	msg->status = status;
	if(msg->complete)
	   msg->complete(msg->context);

}

static void s3c_spi_work(struct work_struct *work)
{
	struct s3cspi_bus *sspi = container_of(work, struct s3cspi_bus, work);
	unsigned long flags;

	spin_lock_irqsave(&sspi->lock, flags);
	while(!list_empty(&sspi->queue) && !(atomic_read(&sspi->state) & SUSPND)){
		struct spi_message *msg;

		msg = container_of(sspi->queue.next, struct spi_message, queue);
		list_del_init(&msg->queue);
		atomic_set(&sspi->state, atomic_read(&sspi->state) & ~ERRS); /* Clear every ERROR flag */
		atomic_set(&sspi->state, atomic_read(&sspi->state) | XFERBUSY); /* Set Xfer busy flag */
		spin_unlock_irqrestore(&sspi->lock, flags);

		handle_msg(sspi, msg);

		spin_lock_irqsave(&sspi->lock, flags);
		atomic_set(&sspi->state, atomic_read(&sspi->state) & ~XFERBUSY);
	}
	spin_unlock_irqrestore(&sspi->lock, flags);
}

static void s3c_spi_cleanup(struct spi_device *spi)
{
	dbg_printk("%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

static int s3c_spi_transfer(struct spi_device *spi, struct spi_message *msg)
{
	struct spi_master *master = spi->master;
	struct s3cspi_bus *sspi = spi_master_get_devdata(master);
	unsigned long flags;

	spin_lock_irqsave(&sspi->lock, flags);

	if(atomic_read(&sspi->state) & SUSPND){
	   spin_unlock_irqrestore(&sspi->lock, flags);
	   return -ESHUTDOWN;
	}

	msg->actual_length = 0;
	msg->status = -EINPROGRESS;
	list_add_tail(&msg->queue, &sspi->queue);
	queue_work(sspi->workqueue, &sspi->work);

	spin_unlock_irqrestore(&sspi->lock, flags);

	return 0;
}

/* the spi->mode bits understood by this driver: */
#define MODEBITS	(SPI_CPOL | SPI_CPHA | SPI_SLAVE | SPI_CS_HIGH)
/*
 * Here we only check the validity of requested configuration and 
 * save the configuration in a local data-structure.
 * The controller is actually configured only just before
 * we get a message to transfer _and_ if no other message is pending(already configured).
 */
static int s3c_spi_setup(struct spi_device *spi)
{
	unsigned long flags;
	unsigned int psr;
	struct spi_message *msg;
	struct s3cspi_bus *sspi = spi_master_get_devdata(spi->master);
	struct s3c_spi_mstr_info *smi = sspi->spi_mstinfo;
	struct s3c_spi_pdata *spd = &sspi->spi_mstinfo->spd[spi->chip_select];

	spin_lock_irqsave(&sspi->lock, flags);
	
	list_for_each_entry(msg, &sspi->queue, queue){
	   if(msg->spi == spi){ /* Is some mssg is already queued for this device */
	      dev_err(&spi->dev, "setup: attempt while mssg in queue!\n");
	      spin_unlock_irqrestore(&sspi->lock, flags);
	      return -EBUSY;
	   }
	}

	if(atomic_read(&sspi->state) & SUSPND){
		spin_unlock_irqrestore(&sspi->lock, flags);
		dev_err(&spi->dev, "setup: SPI-%d not active!\n", spi->master->bus_num);
		return -ESHUTDOWN;
	}

	spin_unlock_irqrestore(&sspi->lock, flags);

	if (spi->chip_select > spi->master->num_chipselect) {
		dev_err(&spi->dev, "setup: invalid chipselect %u (%u defined)\n",
				spi->chip_select, spi->master->num_chipselect);
		return -EINVAL;
	}

	spi->bits_per_word = spi->bits_per_word ? : 16;

	if((spi->bits_per_word != 8) && 
			(spi->bits_per_word != 16) && 
			(spi->bits_per_word != 32)){
		dev_err(&spi->dev, "setup: %dbits/wrd not supported!\n", spi->bits_per_word);
		return -EINVAL;
	}

	/* XXX Should we return -EINVAL or tolerate it XXX */
	if(spi->max_speed_hz < sspi->min_speed)
		spi->max_speed_hz = sspi->min_speed;
	if(spi->max_speed_hz > sspi->max_speed)
		spi->max_speed_hz = sspi->max_speed;

	/* Round-off max_speed_hz */
	psr = smi->spiclck_getrate(smi) / spi->max_speed_hz / 2 - 1;
	psr &= 0xff;
	if(spi->max_speed_hz < smi->spiclck_getrate(smi) / 2 / (psr + 1))
	   psr = (psr+1) & 0xff;

	spi->max_speed_hz = smi->spiclck_getrate(smi) / 2 / (psr + 1);

	if (spi->max_speed_hz > sspi->max_speed
			|| spi->max_speed_hz < sspi->min_speed){
		dev_err(&spi->dev, "setup: req speed(%u) out of range[%u-%u]\n", 
				spi->max_speed_hz, sspi->min_speed, sspi->max_speed);
		return -EINVAL;
	}

	if (spi->mode & ~MODEBITS) {
		dev_err(&spi->dev, "setup: unsupported mode bits %x\n",	spi->mode & ~MODEBITS);
		return -EINVAL;
	}

	if(!(spi->mode & SPI_SLAVE)){
#if 0
	   if(spd->cs_level == CS_FLOAT)
	      spd->cs_config(spd->cs_pin, spd->cs_mode, (spi->mode & SPI_CS_HIGH) ? CS_LOW : CS_HIGH);
#endif
	   disable_cs(sspi, spi);
	}

	return 0;
}

static int __init s3c_spi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct s3cspi_bus *sspi;
	struct s3c_spi_mstr_info *smi;
	int ret = -ENODEV;

	dbg_printk("%s:%s:%d ID=%d\n", __FILE__, __func__, __LINE__, pdev->id);
	master = spi_alloc_master(&pdev->dev, sizeof(struct s3cspi_bus)); /* Allocate contiguous SPI controller */
	if (master == NULL)
		return ret;

	sspi = spi_master_get_devdata(master);
	sspi->master = master;
	platform_set_drvdata(pdev, master);

	sspi->spi_mstinfo = (struct s3c_spi_mstr_info *)pdev->dev.platform_data;
	smi = sspi->spi_mstinfo;
	smi->pdev = pdev;

	INIT_WORK(&sspi->work, s3c_spi_work);
	spin_lock_init(&sspi->lock);
	INIT_LIST_HEAD(&sspi->queue);
	init_completion(&sspi->xfer_completion);

	ret = smi->spiclck_get(smi);
	if(ret){
		dev_err(&pdev->dev, "cannot acquire clock \n");
		ret = -EBUSY;
		goto lb1;
	}
	ret = smi->spiclck_en(smi);
	if(ret){
		dev_err(&pdev->dev, "cannot enable clock \n");
		ret = -EBUSY;
		goto lb2;
	}

	sspi->max_speed = smi->spiclck_getrate(smi) / 2 / (0x0 + 1);
	sspi->min_speed = smi->spiclck_getrate(smi) / 2 / (0xff + 1);

	sspi->cur_bpw = 16;

	/* Get and Map Resources */
	sspi->iores = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (sspi->iores == NULL) {
		dev_err(&pdev->dev, "cannot find IO resource\n");
		ret = -ENOENT;
		goto lb3;
	}

	sspi->ioarea = request_mem_region(sspi->iores->start, sspi->iores->end - sspi->iores->start + 1, pdev->name);
	if (sspi->ioarea == NULL) {
		dev_err(&pdev->dev, "cannot request IO\n");
		ret = -ENXIO;
		goto lb4;
	}

	sspi->regs = ioremap(sspi->iores->start, sspi->iores->end - sspi->iores->start + 1);
	if (sspi->regs == NULL) {
		dev_err(&pdev->dev, "cannot map IO\n");
		ret = -ENXIO;
		goto lb5;
	}

	sspi->tx_dma_cpu = dma_alloc_coherent(&pdev->dev, S3C_SPI_DMABUF_LEN, &sspi->tx_dma_phys, GFP_KERNEL | GFP_DMA);
	if(sspi->tx_dma_cpu == NULL){
		dev_err(&pdev->dev, "Unable to allocate TX DMA buffers\n");
		ret = -ENOMEM;
		goto lb6;
	}

	sspi->rx_dma_cpu = dma_alloc_coherent(&pdev->dev, S3C_SPI_DMABUF_LEN, &sspi->rx_dma_phys, GFP_KERNEL | GFP_DMA);
	if(sspi->rx_dma_cpu == NULL){
		dev_err(&pdev->dev, "Unable to allocate RX DMA buffers\n");
		ret = -ENOMEM;
		goto lb7;
	}

	sspi->irqres = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if(sspi->irqres == NULL){
		dev_err(&pdev->dev, "cannot find IRQ\n");
		ret = -ENOENT;
		goto lb8;
	}

	ret = request_irq(sspi->irqres->start, s3c_spi_interrupt, IRQF_DISABLED,
			pdev->name, sspi);
	if(ret){
		dev_err(&pdev->dev, "cannot acquire IRQ\n");
		ret = -EBUSY;
		goto lb9;
	}

	sspi->workqueue = create_singlethread_workqueue(master->dev.parent->bus_id);
	if(!sspi->workqueue){
		dev_err(&pdev->dev, "cannot create workqueue\n");
		ret = -EBUSY;
		goto lb10;
	}

	/* Configure GPIOs */
	if(pdev->id == 0)
		SETUP_SPI(sspi, 0);
	else if(pdev->id == 1)
		SETUP_SPI(sspi, 1);
	S3C_SETGPIOPULL(sspi);

	if(s3c2410_dma_request(sspi->rx_dmach, &s3c_spi_dma_client, NULL)){
		dev_err(&pdev->dev, "cannot get RxDMA\n");
		ret = -EBUSY;
		goto lb11;
	}
	s3c2410_dma_set_buffdone_fn(sspi->rx_dmach, s3c_spi_dma_rxcb);
	s3c2410_dma_devconfig(sspi->rx_dmach, S3C2410_DMASRC_HW, 0, sspi->sfr_phyaddr + S3C_SPI_RX_DATA);
	s3c2410_dma_config(sspi->rx_dmach, sspi->cur_bpw/8, 0);
	s3c2410_dma_setflags(sspi->rx_dmach, S3C2410_DMAF_AUTOSTART);

	if(s3c2410_dma_request(sspi->tx_dmach, &s3c_spi_dma_client, NULL)){
		dev_err(&pdev->dev, "cannot get TxDMA\n");
		ret = -EBUSY;
		goto lb12;
	}
	s3c2410_dma_set_buffdone_fn(sspi->tx_dmach, s3c_spi_dma_txcb);
	s3c2410_dma_devconfig(sspi->tx_dmach, S3C2410_DMASRC_MEM, 0, sspi->sfr_phyaddr + S3C_SPI_TX_DATA);
	s3c2410_dma_config(sspi->tx_dmach, sspi->cur_bpw/8, 0);
	s3c2410_dma_setflags(sspi->tx_dmach, S3C2410_DMAF_AUTOSTART);

	/* Setup Deufult Mode */
	s3c_spi_hwinit(sspi, pdev->id);

	master->bus_num = pdev->id;
	master->setup = s3c_spi_setup;
	master->transfer = s3c_spi_transfer;
	master->cleanup = s3c_spi_cleanup;
	master->num_chipselect = sspi->spi_mstinfo->num_slaves;

	if(spi_register_master(master)){
		dev_err(&pdev->dev, "cannot register SPI master\n");
		ret = -EBUSY;
		goto lb13;
	}

	printk("Samsung SoC SPI Driver loaded for Bus SPI-%d with %d Slaves attached\n", pdev->id, master->num_chipselect);
	printk("\tMax,Min-Speed [%d, %d]Hz\n", sspi->max_speed, sspi->min_speed);
	printk("\tIrq=%d\tIOmem=[0x%x-0x%x]\tDMA=[Rx-%d, Tx-%d]\n",
			sspi->irqres->start,
			sspi->iores->end, sspi->iores->start,
			sspi->rx_dmach, sspi->tx_dmach);

	return 0;

lb13:
	s3c2410_dma_free(sspi->tx_dmach, &s3c_spi_dma_client);
lb12:
	s3c2410_dma_free(sspi->rx_dmach, &s3c_spi_dma_client);
lb11:
	destroy_workqueue(sspi->workqueue);
lb10:
	free_irq(sspi->irqres->start, sspi);
lb9:
lb8:
	dma_free_coherent(&pdev->dev, S3C_SPI_DMABUF_LEN, sspi->rx_dma_cpu, sspi->rx_dma_phys);
lb7:
	dma_free_coherent(&pdev->dev, S3C_SPI_DMABUF_LEN, sspi->tx_dma_cpu, sspi->tx_dma_phys);
lb6:
	iounmap((void *) sspi->regs);
lb5:
	release_mem_region(sspi->iores->start, sspi->iores->end - sspi->iores->start + 1);
lb4:
lb3:
	smi->spiclck_dis(smi);
lb2:
	smi->spiclck_put(smi);
lb1:
	platform_set_drvdata(pdev, NULL);
	spi_master_put(master);

	return ret;
}

static int __exit s3c_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master = spi_master_get(platform_get_drvdata(pdev));
	struct s3cspi_bus *sspi = spi_master_get_devdata(master);
	struct s3c_spi_mstr_info *smi = sspi->spi_mstinfo;

	s3c2410_dma_free(sspi->tx_dmach, &s3c_spi_dma_client);
	s3c2410_dma_free(sspi->rx_dmach, &s3c_spi_dma_client);
	spi_unregister_master(master);
	destroy_workqueue(sspi->workqueue);
	free_irq(sspi->irqres->start, sspi);
	dma_free_coherent(&pdev->dev, S3C_SPI_DMABUF_LEN, sspi->rx_dma_cpu, sspi->rx_dma_phys);
	dma_free_coherent(&pdev->dev, S3C_SPI_DMABUF_LEN, sspi->tx_dma_cpu, sspi->tx_dma_phys);
	iounmap((void *) sspi->regs);
	release_mem_region(sspi->iores->start, sspi->iores->end - sspi->iores->start + 1);
	smi->spiclck_dis(smi);
	smi->spiclck_put(smi);
	platform_set_drvdata(pdev, NULL);
	spi_master_put(master);

	return 0;
}

#if defined(CONFIG_PM)
static int s3c_spi_suspend(struct platform_device *pdev, pm_message_t state)
{
	int i;
	unsigned long flags;
	struct s3c_spi_pdata *spd;
	struct spi_master *master = spi_master_get(platform_get_drvdata(pdev));
	struct s3cspi_bus *sspi = spi_master_get_devdata(master);
	struct s3c_spi_mstr_info *smi = sspi->spi_mstinfo;

	/* Release DMA Channels */
	s3c2410_dma_free(sspi->tx_dmach, &s3c_spi_dma_client);
	s3c2410_dma_free(sspi->rx_dmach, &s3c_spi_dma_client);

	spin_lock_irqsave(&sspi->lock, flags);
	atomic_set(&sspi->state, atomic_read(&sspi->state) | SUSPND);
	spin_unlock_irqrestore(&sspi->lock, flags);

	while(atomic_read(&sspi->state) & XFERBUSY)
	   msleep(10);

	/* Decrease the drive strength to the least possible */
	if(smi->set_drvst)
	   smi->set_drvst(smi->pdev->id, PORT_STRENGTH(0) | CLK_STRENGTH(0));

	/* Disable the clock */
	smi->spiclck_dis(smi);
	sspi->cur_speed = 0; /* Output Clock is stopped */

	/* Set GPIOs in least power consuming state */
	S3C_UNSETGPIOPULL(sspi);

	for(i=0; i<sspi->spi_mstinfo->num_slaves; i++){
	   spd = &sspi->spi_mstinfo->spd[i];
#if 0
	   if(spd->cs_suspend)
	      spd->cs_suspend(spd->cs_pin, state);
#endif
	}

	return 0;
}

static int s3c_spi_resume(struct platform_device *pdev)
{
	int val;
	unsigned long flags;
	struct s3c_spi_pdata *spd;
	struct spi_master *master = spi_master_get(platform_get_drvdata(pdev));
	struct s3cspi_bus *sspi = spi_master_get_devdata(master);
	struct s3c_spi_mstr_info *smi = sspi->spi_mstinfo;

	for(val=0; val<sspi->spi_mstinfo->num_slaves; val++){
	   spd = &sspi->spi_mstinfo->spd[val];
#if 0
	   if(spd->cs_resume)
	      spd->cs_resume(spd->cs_pin);
#endif
	}

	S3C_SETGPIOPULL(sspi);

	/* Enable the clock */
	smi->spiclck_en(smi);

	s3c_spi_hwinit(sspi, pdev->id);

	spin_lock_irqsave(&sspi->lock, flags);
	atomic_set(&sspi->state, atomic_read(&sspi->state) & ~SUSPND);
	spin_unlock_irqrestore(&sspi->lock, flags);

	/* Aquire DMA Channels */
	val = s3c2410_dma_request(sspi->rx_dmach, &s3c_spi_dma_client, NULL);
	if(val){
	   printk("%s:%d Oooops!!\n", __func__, __LINE__);
	   return val;
	}
	s3c2410_dma_set_buffdone_fn(sspi->rx_dmach, s3c_spi_dma_rxcb);
	s3c2410_dma_devconfig(sspi->rx_dmach, S3C2410_DMASRC_HW, 0, sspi->sfr_phyaddr + S3C_SPI_RX_DATA);
	s3c2410_dma_config(sspi->rx_dmach, sspi->cur_bpw/8, 0);
	s3c2410_dma_setflags(sspi->rx_dmach, S3C2410_DMAF_AUTOSTART);

	val = s3c2410_dma_request(sspi->tx_dmach, &s3c_spi_dma_client, NULL);
	if(val){
	   printk("%s:%d Oooops!!\n", __func__, __LINE__);
	   s3c2410_dma_free(sspi->rx_dmach, &s3c_spi_dma_client);
	   return val;
	}
	s3c2410_dma_set_buffdone_fn(sspi->tx_dmach, s3c_spi_dma_txcb);
	s3c2410_dma_devconfig(sspi->tx_dmach, S3C2410_DMASRC_MEM, 0, sspi->sfr_phyaddr + S3C_SPI_TX_DATA);
	s3c2410_dma_config(sspi->tx_dmach, sspi->cur_bpw/8, 0);
	s3c2410_dma_setflags(sspi->tx_dmach, S3C2410_DMAF_AUTOSTART);

	return 0;
}
#else
#define s3c_spi_suspend	NULL
#define s3c_spi_resume	NULL
#endif /* CONFIG_PM */

static struct platform_driver s3c_spi_driver = {
	.driver = {
		.name	= "s3c-spi",
		.owner = THIS_MODULE,
		.bus    = &platform_bus_type,
	},
	.suspend = s3c_spi_suspend,
	.resume = s3c_spi_resume,
};

static int __init s3c_spi_init(void)
{
	dbg_printk("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return platform_driver_probe(&s3c_spi_driver, s3c_spi_probe);
}
module_init(s3c_spi_init);

static void __exit s3c_spi_exit(void)
{
	platform_driver_unregister(&s3c_spi_driver);
}
module_exit(s3c_spi_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jaswinder Singh Brar <jassi.brar@samsung.com>");
MODULE_DESCRIPTION("Samsung SOC SPI Controller");
MODULE_ALIAS("platform:s3c-spi");
