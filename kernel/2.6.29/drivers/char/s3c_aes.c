/*
 * Copyright (C) 2009 Samsung Electronics Co., LTD. All rights reserved.
 *
 * drivers/char/s3c_aes.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *	S3C AES driver for /dev/aes
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>

#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <mach/map.h>
#include <mach/hardware.h>
#include <plat/regs-aes.h>

#include <asm/dma.h>
#include <mach/dma.h>
#include <plat/dma.h>

#include "s3c_aes.h"

static void __iomem *aes_base;
static s3c_aes_dev_t s3c_aes_dev;
static s3c_aes_context_t s3c_aes_context;
static struct clk *s3c_aes_clock;

#define aes_read(offset)		readl(aes_base + (offset))
#define aes_write(offset, value)	writel((value), aes_base + (offset))

static int s3c_aes_buf_alloc(s3c_aes_dev_t *s3c_adt)
{
	s3c_adt->sbufp = (unsigned char *) dma_alloc_coherent(NULL, S3C_AES_BUF_SIZE, &s3c_adt->sdma_addr, GFP_ATOMIC);

	if (!s3c_adt->sbufp) {
		printk(S3C_AES_NAME "## %s: sbuf alloc failed\n ", __FUNCTION__);
		dma_free_coherent(NULL, S3C_AES_BUF_SIZE, s3c_adt->sbufp, s3c_adt->sdma_addr);
		return -ENOMEM;
	}

	s3c_adt->dbufp = (unsigned char *) dma_alloc_coherent(NULL, S3C_AES_BUF_SIZE, &s3c_adt->ddma_addr, GFP_ATOMIC);

	if (!s3c_adt->dbufp) {
		printk(S3C_AES_NAME "## %s: dbuf alloc failed\n ", __FUNCTION__);
		dma_free_coherent(NULL, S3C_AES_BUF_SIZE, s3c_adt->sbufp, s3c_adt->sdma_addr);

		dma_free_coherent(NULL, S3C_AES_BUF_SIZE, s3c_adt->sbufp, s3c_adt->sdma_addr);
		return -ENOMEM;
	}

	memset(s3c_adt->sbufp, 0, S3C_AES_BUF_SIZE);
	memset(s3c_adt->dbufp, 0, S3C_AES_BUF_SIZE);

	//printk(S3C_AES_NAME "## %s: sdma_addr=0x%08x ddma_addr=0x%08x success\n ", __FUNCTION__, s3c_adt->sdma_addr, s3c_adt->ddma_addr);

	return 0;
}

static void s3c_aes_buf_free(s3c_aes_dev_t *s3c_adt)
{
	if (s3c_adt->sbufp)
		dma_free_coherent(NULL, S3C_AES_BUF_SIZE, s3c_adt->sbufp, s3c_adt->sdma_addr);

	if (s3c_adt->dbufp)
		dma_free_coherent(NULL, S3C_AES_BUF_SIZE, s3c_adt->dbufp, s3c_adt->ddma_addr);
}

static void s3c_aes_reset_fifo(void)	/* now, unused */
{
	aes_write(S3C_AES_FRX_CTRL, 0x4);
	aes_write(S3C_AES_FTX_CTRL, 0x4);
}

static void s3c_aes_set_fifo(s3c_aes_dev_t *s3c_adt, s3c_aes_context_t *s3c_act)	/* now, unused */
{
	unsigned int idata_len = s3c_act->idata_len;
	unsigned int last_byte = 0;

	aes_write(S3C_AES_FRX_MLEN, (idata_len >> 2));
	aes_write(S3C_AES_FRX_BLKSZ, ((last_byte << 16) | (16 << 0)));
	aes_write(S3C_AES_FRX_DSTADDR, s3c_adt->sdma_addr);

	aes_write(S3C_AES_FTX_MLEN, 5);
	aes_write(S3C_AES_FTX_BLKSZ, 16);
	aes_write(S3C_AES_FTX_SRCADDR, s3c_adt->ddma_addr);
}

static void s3c_aes_start_fifo(unsigned int frx_ctrl_value, unsigned int ftx_ctrl_value)
{
	aes_write(S3C_AES_FRX_CTRL, frx_ctrl_value);
	aes_write(S3C_AES_FTX_CTRL, ftx_ctrl_value);
}

static int s3c_aes_fifotx_done(void)
{
	return ((aes_read(S3C_AES_FTX_CTRL) >> 25) & 0x1);
}

static int s3c_aes_fifotx_full(void)
{
	return ((aes_read(S3C_AES_FTX_CTRL) >> 27) & 0x1);
}

static int s3c_aes_fiforx_size(void)
{
	return ((aes_read(S3C_AES_FRX_CTRL) >> 16) & 0xff);
}

static int s3c_aes_putdata_fifo(unsigned char *idata, int length)	/* now, unused */
{
	int i;
	int total_size;
	int available_size;
	unsigned int *ptr;

	ptr = (unsigned int *) idata;
	total_size = length >> 2;

	while (total_size > 0) {
		available_size = s3c_aes_fiforx_size();

		if (total_size <= available_size) {
			available_size = total_size;
			total_size = 0;
		} else
			total_size -= available_size;

		for (i = 0; i < available_size; i++)
			aes_write(S3C_AES_FRX_WRBUF, *ptr++);
	}

	return 0;
}

/* length < 128 Bytes */
static int s3c_aes_putdata_fifo_later(unsigned char *idata, int length)
{
	int i;
	int total_size;
	unsigned int *ptr;

	ptr = (unsigned int *) idata;
	total_size = length >> 2;

	for (i = 0; i < total_size; i++)
		aes_write(S3C_AES_FRX_WRBUF, *ptr++);

	return 0;
}

static int s3c_aes_putdata_fifo_fix(unsigned char *idata)
{
	unsigned int *ptr;

	ptr = (unsigned int *) idata;

	s3c_aes_copyfifo_8w((unsigned int)(ptr), aes_base + S3C_AES_FRX_WRBUF, 4);

	return 0;
}

static int s3c_aes_getdata_fifo(unsigned char *odata, int length)
{
	int i;
	unsigned int *ptr;

	ptr = (unsigned int *) odata;

	if (length > MAX_FIFO_SIZE_BYTE)
		return -1;

	for (i = 0; i < (length >> 2); i++)
		*ptr++ = aes_read(S3C_AES_FTX_RDBUF);
	
	return 0;
}

static int s3c_aes_getdata_fifo_fix(unsigned char *odata)
{
	unsigned int *ptr;

	ptr = (unsigned int *) odata;

	s3c_aes_getfifo_8w(aes_base + S3C_AES_FTX_RDBUF, (unsigned int)(ptr), 4);

	return 0;
}

static ssize_t s3c_aes_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	int ret;
	unsigned int data_size;
	s3c_aes_dev_t *s3c_adt = &s3c_aes_dev;

	//if (s3c_adt->dbuf_full == 0)
		//return 0;

	data_size = (unsigned int)count;

	ret = copy_to_user(buf, s3c_adt->dbufp, data_size);
	//s3c_adt->dbuf_full = 0;

	return data_size;
}

static ssize_t s3c_aes_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	int ret;
	unsigned int data_size;
	s3c_aes_dev_t *s3c_adt = &s3c_aes_dev;

	//if (s3c_adt->sbuf_empty == 0)
		//return 0;

	data_size = (unsigned int)count;

	ret = copy_from_user(s3c_adt->sbufp, buf, data_size);
	//s3c_adt->sbuf_empty = 0;

	return data_size;
}

static void s3c_aes_setup(int mode, s3c_aes_context_t *s3c_act)
{
	aes_write(S3C_AES_RX_CTRL, 0x0);
	aes_write(S3C_AES_FRX_CTRL, 0x4);
	aes_write(S3C_AES_FTX_CTRL, 0x4);

	aes_write(S3C_AES_RX_KEY1, s3c_act->userkey[0]);
	aes_write(S3C_AES_RX_KEY2, s3c_act->userkey[1]);
	aes_write(S3C_AES_RX_KEY3, s3c_act->userkey[2]);
	aes_write(S3C_AES_RX_KEY4, s3c_act->userkey[3]);

	if ((mode == MODE_AES_CBC_ENCRYPT) || (mode == MODE_AES_CBC_DECRYPT)) {
		aes_write(S3C_AES_RX_IV1, s3c_act->param[0]);
		aes_write(S3C_AES_RX_IV2, s3c_act->param[1]);
		aes_write(S3C_AES_RX_IV3, s3c_act->param[2]);
		aes_write(S3C_AES_RX_IV4, s3c_act->param[3]);
	} else if ((mode == MODE_AES_CTR_ENCRYPT) || (mode == MODE_AES_CTR_DECRYPT)) {
		aes_write(S3C_AES_RX_CTR1, s3c_act->param[0]);
		aes_write(S3C_AES_RX_CTR2, s3c_act->param[1]);
		aes_write(S3C_AES_RX_CTR3, s3c_act->param[2]);
		aes_write(S3C_AES_RX_CTR4, s3c_act->param[3]);
	}

	aes_write(S3C_AES_RX_CTRL, mode);

	aes_write(S3C_AES_DNI_CFG, 0xc0000000);
}

static int s3c_aes_fifo_do(s3c_aes_dev_t *s3c_adt, s3c_aes_context_t *s3c_act)
{
	int ret, i;
	int fifo_size;
	int mod_len;

	fifo_size = MAX_FIFO_SIZE_WORD * 4;	/* 128 Bytes */
	
	if (s3c_act->idata_len & 15) {
		printk(S3C_AES_NAME "## %s: idata_len error \n ", __FUNCTION__);
		return -1;
	}

	aes_write(S3C_AES_FRX_MLEN, s3c_act->idata_len>>2);
	aes_write(S3C_AES_FRX_BLKSZ, 4);
	aes_write(S3C_AES_FRX_DSTADDR, S3C64XX_PA_AES + S3C_AES_RX_DIN1);

	aes_write(S3C_AES_FTX_MLEN, s3c_act->idata_len>>2);
	aes_write(S3C_AES_FTX_BLKSZ, 4);
	aes_write(S3C_AES_FTX_SRCADDR, S3C64XX_PA_AES + S3C_AES_TX_DOUT1);

	s3c_aes_start_fifo(0x39, 0x31);
		
	for (i = 0; i < s3c_act->idata_len >> 7; i++) {
		ret = s3c_aes_putdata_fifo_fix(s3c_adt->sbufp + (i << 7));
		if (ret != 0) {
			printk(S3C_AES_NAME "## %s: s3c_aes_putdata_fifo_fix error \n ", __FUNCTION__);
			return -1;
		}
	
		while (!s3c_aes_fifotx_full());
	
		ret = s3c_aes_getdata_fifo_fix(s3c_adt->dbufp + (i << 7));
		if (ret != 0) {
			printk(S3C_AES_NAME "## %s: s3c_aes_getdata_fifo_fix error \n ", __FUNCTION__);
			return -1;
		}
	}

	mod_len = s3c_act->idata_len & (fifo_size - 1);

	if (mod_len != 0) {
		ret = s3c_aes_putdata_fifo_later(s3c_adt->sbufp + (i << 7), mod_len);
		if (ret != 0) {
			printk(S3C_AES_NAME "## %s: s3c_aes_putdata_fifo_later error \n ", __FUNCTION__);
			return -1;
		}

		while (!s3c_aes_fifotx_done());

		ret = s3c_aes_getdata_fifo(s3c_adt->dbufp + (i << 7), mod_len);
		if (ret != 0) {
			printk(S3C_AES_NAME "## %s: s3c_aes_getdata_fifo error \n ", __FUNCTION__);
			return -1;		
		}
	}

	s3c_act->odata_len = s3c_act->idata_len;
		
	return 0;
}

static int s3c_aes_ecb_encrypt(s3c_aes_dev_t *s3c_adt, s3c_aes_context_t *s3c_act)
{
	if (s3c_act->userkey_len != 16) {
		printk("## %s: userkey_len != 16\n ", __FUNCTION__);
		return -1;
	}

	if (s3c_act->param_len != 0) {
		printk("## %s: param_len != 0\n ", __FUNCTION__);
		return -1;
	}

	s3c_aes_setup(MODE_AES_ECB_ENCRYPT, s3c_act);

	s3c_aes_fifo_do(s3c_adt, s3c_act);

	return 0;
} 

static int s3c_aes_ecb_decrypt(s3c_aes_dev_t *s3c_adt, s3c_aes_context_t *s3c_act)
{
	if (s3c_act->userkey_len != 16) {
		printk("## %s: userkey_len != 16\n ", __FUNCTION__);
		return -1;
	}

	if (s3c_act->param_len != 0) {
		printk("## %s: param_len != 0\n ", __FUNCTION__);
		return -1;
	}

	s3c_aes_setup(MODE_AES_ECB_DECRYPT, s3c_act);

	s3c_aes_fifo_do(s3c_adt, s3c_act);

	return 0;
} 

static int s3c_aes_cbc_encrypt(s3c_aes_dev_t *s3c_adt, s3c_aes_context_t *s3c_act)
{
	if (s3c_act->userkey_len != 16) {
		printk("## %s: userkey_len != 16\n ", __FUNCTION__);
		return -1;
	}

	if (s3c_act->param_len != 16) {
		printk("## %s: param_len != 16\n ", __FUNCTION__);
		return -1;
	}

	s3c_aes_setup(MODE_AES_CBC_ENCRYPT, s3c_act);

	s3c_aes_fifo_do(s3c_adt, s3c_act);

	return 0;
} 

static int s3c_aes_cbc_decrypt(s3c_aes_dev_t *s3c_adt, s3c_aes_context_t *s3c_act)
{
	if (s3c_act->userkey_len != 16) {
		printk("## %s: userkey_len != 16\n ", __FUNCTION__);
		return -1;
	}

	if (s3c_act->param_len != 16) {
		printk("## %s: param_len != 16\n ", __FUNCTION__);
		return -1;
	}

	s3c_aes_setup(MODE_AES_CBC_DECRYPT, s3c_act);

	s3c_aes_fifo_do(s3c_adt, s3c_act);

	return 0;
} 

static int s3c_aes_ctr_encrypt(s3c_aes_dev_t *s3c_adt, s3c_aes_context_t *s3c_act)
{
	if (s3c_act->userkey_len != 16) {
		printk("## %s: userkey_len != 16\n ", __FUNCTION__);
		return -1;
	}

	if (s3c_act->param_len != 16) {
		printk("## %s: param_len != 16\n ", __FUNCTION__);
		return -1;
	}

	s3c_aes_setup(MODE_AES_CTR_ENCRYPT, s3c_act);

	s3c_aes_fifo_do(s3c_adt, s3c_act);

	return 0;
} 

static int s3c_aes_ctr_decrypt(s3c_aes_dev_t *s3c_adt, s3c_aes_context_t *s3c_act)
{
	if (s3c_act->userkey_len != 16) {
		printk("## %s: userkey_len != 16\n ", __FUNCTION__);
		return -1;
	}

	if (s3c_act->param_len != 16) {
		printk("## %s: param_len != 16\n ", __FUNCTION__);
		return -1;
	}

	s3c_aes_setup(MODE_AES_CTR_DECRYPT, s3c_act);

	s3c_aes_fifo_do(s3c_adt, s3c_act);

	return 0;
} 

static int s3c_aes_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	//unsigned long flags;
	s3c_aes_dev_t *s3c_adt = &s3c_aes_dev;

	//local_save_flags(flags);
	//local_irq_disable();

	switch (cmd) {
	case S3C_AES_ECB_ENCRYPT:
		s3c_aes_ecb_encrypt(s3c_adt, (s3c_aes_context_t *)arg);
		break;

	case S3C_AES_ECB_DECRYPT:
		s3c_aes_ecb_decrypt(s3c_adt, (s3c_aes_context_t *)arg);
		break;

	case S3C_AES_CBC_ENCRYPT:
		s3c_aes_cbc_encrypt(s3c_adt, (s3c_aes_context_t *)arg);
		break;

	case S3C_AES_CBC_DECRYPT:
		s3c_aes_cbc_decrypt(s3c_adt, (s3c_aes_context_t *)arg);
		break;

	case S3C_AES_CTR_ENCRYPT:
		s3c_aes_ctr_encrypt(s3c_adt, (s3c_aes_context_t *)arg);
		break;

	case S3C_AES_CTR_DECRYPT:
		s3c_aes_ctr_decrypt(s3c_adt, (s3c_aes_context_t *)arg);
		break;

	default:
		return -EINVAL;
	}
	
	//local_irq_restore(flags);

	return 0;
}

static int s3c_aes_open(struct inode *inode, struct file *filp)
{

	return 0;
}

static int s3c_aes_release(struct inode *inode, struct file *filp)
{

	return 0;
}

struct file_operations s3c_aes_fops = {
	.owner		= THIS_MODULE,
	.llseek		= NULL,
	.read		= s3c_aes_read,
	.write		= s3c_aes_write,
	.ioctl		= s3c_aes_ioctl,
	.open		= s3c_aes_open,
	.release	= s3c_aes_release,
};

static int __init s3c_aes_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *res;
	s3c_aes_dev_t *s3c_adt = &s3c_aes_dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to get platform resource \n");
		return -ENOENT;
	}

	aes_base = ioremap(res->start, res->end - res->start + 1);
	if (aes_base == NULL) {
		dev_err(&pdev->dev, "failed to remap register block \n");
		ret = -ENOMEM;
	}

	ret = register_chrdev(S3C_AES_MAJOR, S3C_AES_NAME, &s3c_aes_fops);
	if (ret < 0) {
		iounmap(aes_base);
		aes_base = NULL;
		dev_err(&pdev->dev, "failed to register chrdev \n");
		return ret;
	}

	s3c_aes_clock = clk_get(&pdev->dev, "secur");
	if (IS_ERR(s3c_aes_clock)) {
		iounmap(aes_base);
		aes_base = NULL;
		dev_err(&pdev->dev, "failed to find secur clock source \n");
		return -1;
	}

	clk_enable(s3c_aes_clock);

	s3c_aes_buf_alloc(s3c_adt);
	
	printk(S3C_AES_NAME " initialized \n");

	return 0;
}

static int s3c_aes_remove(struct platform_device *dev)
{
	s3c_aes_dev_t *s3c_adt = &s3c_aes_dev;

	s3c_aes_buf_free(s3c_adt);

	if (s3c_aes_clock) {
		clk_disable(s3c_aes_clock);
		clk_put(s3c_aes_clock);
		s3c_aes_clock = NULL;
	}

	unregister_chrdev(S3C_AES_MAJOR, S3C_AES_NAME);

	if (aes_base) {
		iounmap(aes_base);
		aes_base = NULL;
	}

	printk(S3C_AES_NAME " removed \n");

	return 0;
}

#ifdef CONFIG_PM
static int s3c_aes_suspend(struct platform_device *dev, pm_message_t state)
{
	clk_disable(s3c_aes_clock);

	return 0;
}

static int s3c_aes_resume(struct platform_device *dev)
{
	clk_enable(s3c_aes_clock);

	return 0;
}
#else
#define s3c_aes_suspend	NULL
#define s3c_aes_resume	NULL
#endif /* CONFIG_PM */

static struct platform_driver s3c_aes_driver = {
	.probe		= s3c_aes_probe,
	.remove		= s3c_aes_remove,
	.suspend	= s3c_aes_suspend,
	.resume		= s3c_aes_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c-aes",
	},
};

static int __init s3c_aes_init(void)
{
	int ret;

	ret = platform_driver_register(&s3c_aes_driver);

	if (!ret)
		printk(KERN_INFO "S3C AES Drvier \n");

	return ret;
}

static void __exit s3c_aes_exit(void)
{
	platform_driver_unregister(&s3c_aes_driver);
}

module_init(s3c_aes_init);
module_exit(s3c_aes_exit);

MODULE_AUTHOR("Samsung");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Device Driver for S3C AES");
