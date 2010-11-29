/* linux/drivers/spi/spi_s3c64xx.c
 *
 * Copyright (C) 2007 Samsung Electronics
 * Copyright (c) 2006 Ben Dooks
 * Copyright (c) 2006 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * 
 */

#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>

#include <asm/io.h>
#include <asm/dma.h>
#include <asm/delay.h>

#include <plat/regs-clock.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-spi.h>
#include <plat/dma.h>
#include <mach/gpio.h>

#if defined(CONFIG_PM)
#include <plat/pm.h>
#endif

struct s3c6410_spi {
	/* bitbang has to be first */
	struct spi_bitbang		 bitbang;
	struct completion		 done;

	void __iomem			*regs;
	void __iomem			*regs_sys;
	int			 			irq;
	int			 			len;
	int			 			count;

	/* data buffers */
	const void				*tx;
	void					*rx;
	
	struct clk				*clk;
	struct resource			*ioarea;
	struct spi_master		*master;
	struct spi_device		*curdev;
	struct device			*dev;
	struct s3c6410_spi_info *pdata;
	int						tx_done, rx_done;
	int						bits_per_word;
	int						id;
	int 					lsb_first;
	unsigned long			in_clk;
	unsigned int 			flag;
	spinlock_t      		lock;
};

/* functions */
static inline struct s3c6410_spi *to_hw(struct spi_device *sdev);
static void s3c6410_spi_chipsel(struct spi_device *spi, int value);
static int s3c6410_spi_setupxfer(struct spi_device *spi,
				 struct spi_transfer *t);
static int s3c6410_spi_setup(struct spi_device *spi);
static unsigned int hw_tx(struct s3c6410_spi *hw);
static void  hw_rx(struct s3c6410_spi *hw, unsigned int data);
static int s3c6410_spi_txrx(struct spi_device *spi, struct spi_transfer *t);
static irqreturn_t s3c6410_spi_irq(int irq, void *dev);
static int s3c6410_spi_probe(struct platform_device *pdev);
static int s3c6410_spi_remove(struct platform_device *dev);
static int __init s3c6410_spi_init(void);
static void __exit s3c6410_spi_exit(void);





static struct s3c6410_spi *test_hw;


static struct sleep_save s3c_spi_save[12];



/*
 * to_hw  - return the pointer of s3c6410_spi struct 
 * @sdev   : spi_device sturct
 *
 */
static inline struct s3c6410_spi *to_hw(struct spi_device *sdev)
{
	return spi_master_get_devdata(sdev->master);
}

static void s3c6410_spi_set_cs(struct s3c6410_spi *spi, int value)
{
	struct s3c6410_spi *hw = spi;
	int reg;

	switch (value) {
	/* Select */
	case BITBANG_CS_ACTIVE:  
		reg = readl(hw->regs + S3C_SLAVE_SEL);
		reg &= ~(SPI_SLAVE_SIG_INACT);
		writel(reg, hw->regs + S3C_SLAVE_SEL);
		udelay(30);
		break;
	
	/* Non-Select */
	case BITBANG_CS_INACTIVE: 
		reg = readl(hw->regs + S3C_SLAVE_SEL);
		reg |= SPI_SLAVE_SIG_INACT;
		writel(reg, hw->regs + S3C_SLAVE_SEL);
		break;
	}
}

/*
 * s3c6410_spi_chipsel  - select spi slave chip
 * @spi   : spi_device sturct
 * @value : chipselect value
 *
 * bitbang driver member fuction to select spi slave chip 
 */
static void s3c6410_spi_chipsel(struct spi_device *spi, int value)
{
	struct s3c6410_spi *hw = to_hw(spi);
	int reg;

	switch (value) {
	/* Select */
	case BITBANG_CS_ACTIVE:  
		reg = readl(hw->regs + S3C_SLAVE_SEL);
		reg &= ~(SPI_SLAVE_SIG_INACT);
		writel(reg, hw->regs + S3C_SLAVE_SEL);
		udelay(30);
		break;
	
	/* Non-Select */
	case BITBANG_CS_INACTIVE: 
		reg = readl(hw->regs + S3C_SLAVE_SEL);
		reg |= SPI_SLAVE_SIG_INACT;
		writel(reg, hw->regs + S3C_SLAVE_SEL);
		break;
	}
}

/*
 * s3c6410_spi_setupxfer  - setup spi control
 * @spi : spi_device sturct
 * @t   : spi_transfer struct
 *
 * bitbang driver member fuction to setup spi control
 * it set bit_per_word and serial clock rate.
 */
static int s3c6410_spi_setupxfer(struct spi_device *spi,
				 struct spi_transfer *t)
{
	struct s3c6410_spi *hw = to_hw(spi);
	unsigned int bpw, mode, spi_chcfg;
	int scr, ret = 0;
	unsigned int bit_rate, reg = 0;
	unsigned long pclk;

	/*	bpw = t ? t->bits_per_word : spi->bits_per_word; */
	bpw = spi->bits_per_word;
	bit_rate  = t ? t->speed_hz : spi->max_speed_hz;

	/* set bits per word */
	if (bpw > 32 || bpw < 8) {
		dev_err(&spi->dev, "spi:  invalid bits-per-word (%d)\n", bpw);
		return -EINVAL;
	}

	/* Set SPI_SCALER :
	   bit rate = PCLK / ( 2 * (1 + SPI_SCALER)) 
	   SPI_SCALER = (PCLK / bit rate / 2) -1	
	   SPI_SCALER = 0 (default)	*/

	writel(readl(hw->regs + S3C_CLK_CFG) & (~SPI_ENCLK_ENABLE), hw->regs + S3C_CLK_CFG );	

	pclk =  hw->in_clk;
	scr = (pclk / bit_rate / 2) - 1;

	if(scr < 0) 	scr = 0;
	if(scr > 0xff) 	scr = 0xff;

	bit_rate = (unsigned int) (pclk / ( 2 * (scr + 1)));
	printk("spi:  setting serial clock rate to %d (bit rate = %d hz, pclk = %d)\n", scr, bit_rate, (int)pclk);
	reg = readl(hw->regs + S3C_CLK_CFG);
	writel(reg | scr | SPI_ENCLK_ENABLE, hw->regs + S3C_CLK_CFG );	

	if	   (bpw > 16)	mode = SPI_MODE_CH_TSZ_WORD | SPI_MODE_BUS_TSZ_WORD;
	else if(bpw > 8) 	mode = SPI_MODE_CH_TSZ_HALFWORD | SPI_MODE_BUS_TSZ_HALFWORD;
	else 				mode = SPI_MODE_CH_TSZ_BYTE | SPI_MODE_BUS_TSZ_BYTE;

	mode |= (SPI_MODE_RXDMA_OFF | SPI_MODE_TXDMA_OFF);
	writel(mode, hw->regs + S3C_MODE_CFG);
	mode = readl( hw->regs + S3C_MODE_CFG);

	spi_chcfg = readl(hw->regs + S3C_CH_CFG);
	spi_chcfg |= SPI_CH_TXCH_ON | SPI_CH_RXCH_ON;
	writel(spi_chcfg , hw->regs + S3C_CH_CFG);

	spin_lock(&hw->bitbang.lock);
	if (!hw->bitbang.busy) {
		hw->bitbang.chipselect(spi, BITBANG_CS_INACTIVE);
		/* need to ndelay for 0.5 clocktick */
	}
	spin_unlock(&hw->bitbang.lock);

	return ret;
}

/*
 * s3c6410_spi_setup  - setup spi attribute
 * @spi : spi_device sturct
 *
 * bitbang driver member fuction to setup spi attribute
 * it set bit_per_word and spi mode.
 */
static int s3c6410_spi_setup(struct spi_device *spi)
{
	struct s3c6410_spi *hw = to_hw(spi);
	int ret;

	if (!spi->bits_per_word)
		spi->bits_per_word = 16;

	/* Set Swap Config Reg */
	if(spi->mode & SPI_LSB_FIRST) 
		writel(SPI_SWAP_RX_BIT |
		       SPI_SWAP_RX_EN  |
		       SPI_SWAP_TX_BIT |
		       SPI_SWAP_TX_EN, hw->regs + S3C_SWAP_CFG);
	else 
		writel(0, hw->regs + S3C_SWAP_CFG);		

	ret = s3c6410_spi_setupxfer(spi, NULL);
	if (ret < 0) {
		dev_err(&spi->dev, "setupxfer returned %d\n", ret);
		return ret;
	}

	printk("spi:  mode %d, %u bpw, %d hz\n",
		spi->mode, spi->bits_per_word,spi->max_speed_hz);

	return ret;
}

/*
 * hw_tx - load data pointed by a hw->tx pointer to tx fifo.
 * @hw   : s3c6410_spi struct
 *
 * it is a sub funtion to load data pointed by a hw->tx pointer to tx fifo
 *
 */
static unsigned int hw_tx(struct s3c6410_spi *hw)
{
	unsigned int tmp;

	// if hw->tx is NULL, return 0
	if(!hw->tx) return 0;
	
	if(hw->bits_per_word > 16) //32 bit
		tmp=*((unsigned int*)hw->tx+hw->tx_done);

	else if(hw->bits_per_word > 8)	//16 bit
		tmp=(unsigned int) *((unsigned short*)hw->tx + hw->tx_done);

	else					//8 bit
		tmp = (unsigned int) *((unsigned char*)hw->tx + hw->tx_done);
	
	return tmp;
}

/*
 * hw_rx - store data to memory pointed to by a hw->rx pointer
 * @hw   : s3c6410_spi struct
 * @data : data
 *
 * it is a sub funtion to store data to memory pointed to by a hw->rx pointer.
 *
*/
static void  hw_rx(struct s3c6410_spi *hw, unsigned int data)
{
	// if hw->rx is NULL, return
	if(!hw->rx) return;

	if(hw->bits_per_word>16)
		*((unsigned int*)hw->rx+hw->rx_done) = data;

	else if(hw->bits_per_word>8)	// 16 bit
		*((unsigned short*)hw->rx+hw->rx_done)= (unsigned short)data;	

	else 					// 8 bit
		*((unsigned char*)hw->rx+hw->rx_done)=(unsigned char) data;
}


unsigned int swap_bit(unsigned int data)
{
	unsigned char data1 = (unsigned char)data;
	unsigned char ret = 0;
	unsigned int val, i;

	for(i=0;i<8;i++)
	{
		ret+=(data1&1);
		data1>>=1;
		if(i<7)
		ret<<=1;
	}

	val = (unsigned int) ret;
	printk("SPI:swap_bit 0x%x -> 0x%x\n",data, val);
	return val;
}

/*
 * s3c6410_spi_txrx  - handle for spi_message
 * @spi : spi_device sturct
 * @t   : spi_transfer struct
 *
 * bitbang driver member fuction to handle for spi_message
 * it send first message.
 */
static int s3c6410_spi_txrx(struct spi_device *spi, struct spi_transfer *t)
{
	struct s3c6410_spi *hw = to_hw(spi);
	unsigned int spi_modecfg, spi_inten;

	hw->tx = t->tx_buf;
	hw->rx = t->rx_buf;
	hw->len = t->len;
	hw->count = 0;
	hw->tx_done = 0;
	hw->rx_done = 0;
	hw->flag = 0;	

	init_completion(&hw->done);

	spi_modecfg = readl(hw->regs + S3C_MODE_CFG);
	spi_modecfg |= (hw->len << 5) | (hw->len << 11);
	writel(spi_modecfg, hw->regs + S3C_MODE_CFG);

	/* enable SPI Interrupt */
	if(hw->tx)
		spi_inten = SPI_INT_TX_FIFORDY_EN|SPI_INT_TX_UNDERRUN_EN|SPI_INT_TX_OVERRUN_EN;

	if(hw->rx)
		spi_inten = SPI_INT_RX_FIFORDY_EN|SPI_INT_RX_UNDERRUN_EN|SPI_INT_RX_OVERRUN_EN|SPI_INT_TRAILING_EN;

	writel(spi_inten , hw->regs + S3C_SPI_INT_EN); 

	wait_for_completion(&hw->done);

	return hw->rx_done;
}


/*
 * s3c6410_spi_interrupt  - spi fifo interrupt handler
 * @irq    : interrupt number
 * @dev    : device instance
 *
 * spi fifo interrupt handler
 */
static irqreturn_t s3c6410_spi_irq(int irq, void *dev)
{
	struct s3c6410_spi *hw = dev;
	unsigned int pend = readl(hw->regs + S3C_SPI_STATUS);
	unsigned int w_tmp;
	unsigned int r_tmp;
	int err = 0;

	spin_lock(&hw->lock);

	if(pend & SPI_STUS_RX_OVERRUN_ERR)
	{
		printk("spi%d: RX FIFO overrun error\n", hw->id);
		writel(SPI_PND_RX_OVERRUN_CLR, hw->regs + S3C_PENDING_CLR);
		err = 1;
	}
	if(pend & SPI_STUS_RX_UNDERRUN_ERR)
	{
		printk("spi%d: RX FIFO underrun error\n", hw->id);
		writel(SPI_PND_RX_UNDERRUN_CLR, hw->regs + S3C_PENDING_CLR);
		err = 1;
	}
	if(pend & SPI_STUS_TX_OVERRUN_ERR)
	{
		printk("spi%d: TX FIFO overrun error\n", hw->id);
		writel(SPI_PND_TX_OVERRUN_CLR, hw->regs + S3C_PENDING_CLR);
		err = 1;
	}
	if(pend & SPI_STUS_TX_UNDERRUN_ERR)
	{
		printk("spi%d: TX FIFO underrun error\n", hw->id);
		writel(SPI_PND_TX_UNDERRUN_CLR, hw->regs + S3C_PENDING_CLR);
		err = 1;
	}

	if(err)
	{
		complete(&hw->done);
		return IRQ_HANDLED;
	}
	
	if((pend & SPI_STUS_TX_FIFORDY) && (hw->tx_done < hw->len))
	{
		w_tmp = hw_tx(hw);
        writel(w_tmp, hw->regs + S3C_SPI_TX_DATA);
		udelay(30);
		
		hw->flag = 1;
	    hw->tx_done++;
	}
	
	if(pend & (SPI_STUS_RX_FIFOLVL)) 
	{
		r_tmp = readl(hw->regs + S3C_SPI_RX_DATA);
        hw_rx(hw, r_tmp);
		hw->flag = 0;
		hw->rx_done++;
	}

	if(hw->rx_done == hw->len && hw->tx_done == hw->len)
	{
		writel(SPI_INT_ALL_DISABLE, hw->regs + S3C_SPI_INT_EN);
		udelay(30);
		complete(&hw->done);
	}
	
	spin_unlock(&hw->lock);

	return IRQ_HANDLED;
}

/*
 * s3c6410_spi_probe  - probe spi driver
 * @pdev	: platform device struct
 *
 * platform driver member function for probing
 */
static int s3c6410_spi_probe(struct platform_device *pdev)
{
	struct s3c6410_spi *hw;
	struct spi_master *master;
	struct spi_board_info *bi;
	struct resource *res;
	int err = 0;
	int i;
	unsigned int spi_chcfg;

	master = spi_alloc_master(&pdev->dev, sizeof(struct s3c6410_spi));
	if (master == NULL) {
		dev_err(&pdev->dev, "No memory for spi_master\n");
		err = -ENOMEM;
		goto err_nomem;
	}

	hw = spi_master_get_devdata(master);
	memset(hw, 0, sizeof(struct s3c6410_spi));

	hw->master = spi_master_get(master);
	hw->master->num_chipselect = 2;
	hw->pdata = pdev->dev.platform_data;
	hw->dev = &pdev->dev;
	hw->id = pdev->id;
	hw->bits_per_word = 16;

	if (hw->pdata == NULL) {
		dev_err(&pdev->dev, "No platform data supplied\n");
		err = -ENOENT;
		goto err_no_pdata;
	}

	platform_set_drvdata(pdev, hw);

	/* setup the state for the bitbang driver */
	hw->bitbang.master         = hw->master;
	hw->bitbang.setup_transfer = s3c6410_spi_setupxfer;
	hw->bitbang.chipselect     = s3c6410_spi_chipsel;
	hw->bitbang.txrx_bufs      = s3c6410_spi_txrx;
	hw->bitbang.master->setup  = s3c6410_spi_setup;
	
	init_completion(&hw->done);

	/* find and map our resources */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");
		err = -ENOENT;
		goto err_no_iores;
	}

	hw->ioarea = request_mem_region(res->start, (res->end - res->start)+1,
					pdev->name);

	if (hw->ioarea == NULL) {
		dev_err(&pdev->dev, "Cannot reserve region\n");
		err = -ENXIO;
		goto err_no_iores;
	}

	hw->regs = ioremap(res->start, (res->end - res->start)+1);
	hw->regs += 0x1000 * hw->id;


	if (hw->regs == NULL) {
		dev_err(&pdev->dev, "Cannot map IO\n");
		err = -ENXIO;
		goto err_no_iomap;
	}

	hw->irq = platform_get_irq(pdev, 0);
	if (hw->irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		err = -ENOENT;
		goto err_no_irq;
	}

	err = request_irq(hw->irq, s3c6410_spi_irq, IRQF_DISABLED, pdev->name, hw);
	if (err) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		goto err_no_irq;
	}

	hw->clk = clk_get(&pdev->dev, "spi");
	if (IS_ERR(hw->clk)) {
		dev_err(&pdev->dev, "No clock for device\n");
		err = PTR_ERR(hw->clk);
		goto err_no_clk;
	}

	printk("S3C6410 SPI Driver at 0x%x, irq %d\n", (unsigned int)hw->regs, hw->irq);

	/* for the moment, permanently enable the clock */
#ifdef CONFIG_SPICLK_PCLK
	clk_enable(hw->clk);
	hw->in_clk = clk_get_rate(hw->clk);
#elif defined (CONFIG_SPICLK_EPLL)
	writel((readl(S3C_PCLK_GATE)|S3C_CLKCON_PCLK_SPI0|S3C_CLKCON_PCLK_SPI1),S3C_PCLK_GATE);
	writel((readl(S3C_SCLK_GATE)|S3C_CLKCON_SCLK_SPI0|S3C_CLKCON_SCLK_SPI1),S3C_SCLK_GATE);

	writel(readl(S3C_CLK_SRC)|S3C_CLKSRC_MPLL_CLKSEL, S3C_CLK_SRC);

	/* Set SPi Clock to DOUTMPLL*/
	if(hw->id == 0)		/* SPI_CHANNEL = 0 */
		writel((readl(S3C_CLK_SRC)&~(0x3<<14))|(1<<14), S3C_CLK_SRC);
	else  				/* SPI_CHANNEL = 1 */
		writel((readl(S3C_CLK_SRC)&~(0x3<<16))|(1<<16), S3C_CLK_SRC);

	/* CLK_DIV2 setting */
	/* SPI Input Clock(88.87Mhz) = 266.66Mhz / (2 + 1)*/
	writel(((readl(S3C_CLK_DIV2) & ~(0xff << 0)) | 2) , S3C_CLK_DIV2);

	hw->in_clk = 266660000;

#elif defined (CONFIG_SPICLK_USBCLK)
	writel((readl(S3C_PCLK_GATE)| S3C_CLKCON_PCLK_SPI0|S3C_CLKCON_PCLK_SPI1),S3C_PCLK_GATE);
	writel((readl(S3C_SCLK_GATE)|S3C_CLKCON_SCLK_SPI0_48|S3C_CLKCON_SCLK_SPI1_48),S3C_SCLK_GATE);
	hw->in_clk = 48000000;
#endif
	
	printk("SPI: Source Clock = %ldMhz\n", hw->in_clk);

	/* initialize the gpio */
	if (hw->id == 0) {
		s3c_gpio_cfgpin(S3C64XX_GPC(0), S3C64XX_GPC0_SPI_MISO0);
		s3c_gpio_cfgpin(S3C64XX_GPC(1), S3C64XX_GPC1_SPI_CLK0);
		s3c_gpio_cfgpin(S3C64XX_GPC(2), S3C64XX_GPC2_SPI_MOSI0);
		s3c_gpio_cfgpin(S3C64XX_GPC(3), S3C64XX_GPC3_SPI_nCS0);

		s3c_gpio_setpull(S3C64XX_GPC(0), S3C_GPIO_PULL_UP);
		s3c_gpio_setpull(S3C64XX_GPC(1), S3C_GPIO_PULL_UP);
		s3c_gpio_setpull(S3C64XX_GPC(2), S3C_GPIO_PULL_UP);
		s3c_gpio_setpull(S3C64XX_GPC(3), S3C_GPIO_PULL_UP);
	} else {
		s3c_gpio_cfgpin(S3C64XX_GPC(4), S3C64XX_GPC4_SPI_MISO1);
		s3c_gpio_cfgpin(S3C64XX_GPC(5), S3C64XX_GPC5_SPI_CLK1);
		s3c_gpio_cfgpin(S3C64XX_GPC(6), S3C64XX_GPC6_SPI_MOSI1);
		s3c_gpio_cfgpin(S3C64XX_GPC(7), S3C64XX_GPC7_SPI_nCS1);

		s3c_gpio_setpull(S3C64XX_GPC(4), S3C_GPIO_PULL_UP);
		s3c_gpio_setpull(S3C64XX_GPC(5), S3C_GPIO_PULL_UP);
		s3c_gpio_setpull(S3C64XX_GPC(6), S3C_GPIO_PULL_UP);
		s3c_gpio_setpull(S3C64XX_GPC(7), S3C_GPIO_PULL_UP);
	}

	/* SW Reset */	
	writel(readl(hw->regs + S3C_CH_CFG) | SPI_CH_SW_RST, hw->regs + S3C_CH_CFG);
	udelay(100);
	writel(readl(hw->regs + S3C_CH_CFG) & (~SPI_CH_SW_RST), hw->regs + S3C_CH_CFG);
	udelay(100);

	/* disable  SPI Interrupt */
	writel(SPI_INT_ALL_DISABLE, hw->regs + S3C_SPI_INT_EN);

	/* Set transfer type (CPOL & CPHA set) */
	spi_chcfg = readl(hw->regs + S3C_CH_CFG);
	spi_chcfg &= ~SPI_CH_HSPD_EN;
	spi_chcfg |= SPI_CH_FORMAT_B | SPI_CH_RISING | SPI_CH_MASTER; 
	writel(spi_chcfg, hw->regs + S3C_CH_CFG);

	/* Set NSSOUT to start high after Reset */
	s3c6410_spi_set_cs(hw, BITBANG_CS_INACTIVE);

	/* register our spi controller */
	err = spi_bitbang_start(&hw->bitbang);
	if (err) {
		dev_err(&pdev->dev, "Failed to register SPI master\n");
		goto err_register;
	}

	/* register all the devices associated */
	bi = hw->pdata->board_info;
	for (i = 0; i < hw->pdata->board_size; i++, bi++) {
		dev_info(hw->dev, "registering %s\n", bi->modalias);

		bi->controller_data = hw;
		spi_new_device(master, bi);
	}
	
	/* for suspend & resume */
	test_hw = hw;

	return 0;

 err_register:
	clk_disable(hw->clk);
	clk_put(hw->clk);

 err_no_clk:
	free_irq(hw->irq, hw);

 err_no_irq:
	iounmap(hw->regs);

 err_no_iomap:
	release_resource(hw->ioarea);
	kfree(hw->ioarea);

 err_no_iores:
 err_no_pdata:
	spi_master_put(hw->master);;

 err_nomem:
	return err;
}

/*
 * s3c6410_spi_remove  - remove spi driver
 * @pdev	: platform device struct
 *
 * platform driver member function to remove spi driver
 */
static int s3c6410_spi_remove(struct platform_device *dev)
{
	struct s3c6410_spi *hw = platform_get_drvdata(dev);

	platform_set_drvdata(dev, NULL);

	spi_unregister_master(hw->master);

	clk_disable(hw->clk);
	clk_put(hw->clk);

	free_irq(hw->irq, hw);
	iounmap(hw->regs);

	release_resource(hw->ioarea);
	kfree(hw->ioarea);

	spi_master_put(hw->master);
	return 0;
}




#ifdef CONFIG_PM
/*
 * s3c6410_spi_suspend  - resume spi driver
 * @pdev	: platform device struct
 * @msg		: pm_message_t
 *
 * platform driver member function to suspend spi driver
 */
static int s3c6410_spi_suspend(struct platform_device *pdev, pm_message_t msg)
{
	struct s3c6410_spi *hw = platform_get_drvdata(pdev);
	printk("test_hw->regs = %x , %s\n",(unsigned int)(test_hw->regs),__func__);

	s3c_spi_save[0].reg = test_hw->regs + S3C_CH_CFG;
	s3c_spi_save[1].reg = test_hw->regs + S3C_CLK_CFG;
	s3c_spi_save[2].reg = test_hw->regs + S3C_MODE_CFG;
	s3c_spi_save[3].reg = test_hw->regs + S3C_SLAVE_SEL;
	s3c_spi_save[4].reg = test_hw->regs + S3C_SPI_INT_EN;
	s3c_spi_save[5].reg = test_hw->regs + S3C_SPI_STATUS;
	s3c_spi_save[6].reg = test_hw->regs + S3C_SPI_TX_DATA;
	s3c_spi_save[7].reg = test_hw->regs + S3C_SPI_RX_DATA;
	s3c_spi_save[8].reg = test_hw->regs + S3C_PACKET_CNT;
	s3c_spi_save[9].reg = test_hw->regs + S3C_PENDING_CLR;
	s3c_spi_save[10].reg = test_hw->regs + S3C_SWAP_CFG;
	s3c_spi_save[11].reg = test_hw->regs + S3C_FB_CLK;
	
	s3c6410_pm_do_save(s3c_spi_save, ARRAY_SIZE(s3c_spi_save));
	clk_disable(hw->clk);
	return 0;
}

/*
 * s3c6410_spi_resume  - resume spi driver
 * @pdev	: platform device struct
 *
 * platform driver member function to resume spi driver
 */
static int s3c6410_spi_resume(struct platform_device *pdev)
{
	struct s3c6410_spi *hw = platform_get_drvdata(pdev);

	printk("%s\n",__func__);

	/* for the moment, permanently enable the clock */

	clk_enable(hw->clk);

    s3c6410_pm_do_restore(s3c_spi_save, ARRAY_SIZE(s3c_spi_save));

	/* sw reset */
	
	writel(readl(hw->regs + S3C_CH_CFG) | SPI_CH_SW_RST, hw->regs + S3C_CH_CFG);
	udelay(100);
	writel(readl(hw->regs + S3C_CH_CFG) & (~SPI_CH_SW_RST), hw->regs + S3C_CH_CFG);
	udelay(100);

	return 0;
}

#else
#define s3c6410_spi_suspend NULL
#define s3c6410_spi_resume  NULL
#endif

static struct platform_driver s3c6410_spidrv = {
	.probe		= s3c6410_spi_probe,
	.remove		= s3c6410_spi_remove,
	.suspend	= s3c6410_spi_suspend,
	.resume		= s3c6410_spi_resume,
	.driver		= {
		.name	= "s3c-spi",
		.owner	= THIS_MODULE,
	},
};

/*
 * s3c6410_spi_init - module init function for spi driver
 */
static int __init s3c6410_spi_init(void)
{
	return platform_driver_register(&s3c6410_spidrv);
}

/*
 * s3c6410_spi_exit - module exit function for spi driver
 */
static void __exit s3c6410_spi_exit(void)
{
	platform_driver_unregister(&s3c6410_spidrv);
}

module_init(s3c6410_spi_init);
module_exit(s3c6410_spi_exit);

MODULE_DESCRIPTION("Samsung S3C6000 spi driver");
MODULE_AUTHOR("Donghoon Yu <hoony.yu@samsung.com>");
MODULE_LICENSE("GPL");
