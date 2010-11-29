/*
 * Project Name JPEG DRIVER IN Linux
 * Copyright  2007 Samsung Electronics Co, Ltd. All Rights Reserved. 
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics  ("Confidential Information").   
 * you shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics 
 *
 * This file implements JPEG driver.
 *
 * @name JPEG DRIVER MODULE Module (JPGDriver.c)
 * @author Jiun Yu (jiun.yu@samsung.com)
 * @date 05-07-07
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/signal.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/kmod.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/semaphore.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/clk.h>
#include <linux/version.h>

#include <asm/io.h>
#include <asm/page.h>

#include <mach/irqs.h>
#include <plat/map-base.h>
#include <plat/regs-clock.h>
#include <plat/power-clock-domain.h>
#include <plat/pm.h>

#include "s3c-jpeg.h"
#include "JPGMem.h"
#include "JPGMisc.h"
#include "JPGOpr.h"
#include "LogMsg.h"

#ifdef CONFIG_S3C64XX_DOMAIN_GATING
#define USE_JPEG_DOMAIN_GATING
#endif /* CONFIG_S3C64XX_DOMAIN_GATING */

typedef struct {
    int     size;
    unsigned int virt_addr;
    unsigned int phys_addr;
}s3c_jpeg_t;


static struct clk		*jpeg_hclk;
static struct resource	*jpeg_mem;
static void __iomem		*jpeg_base;
static S3C6400_JPG_CTX	JPGMem;
static int				irq_no;
static int				instanceNo = 0;
volatile int			jpg_irq_reason;

DECLARE_WAIT_QUEUE_HEAD(WaitQueue_JPEG);

static void clk_jpeg_enable(void)
{
	__raw_writel((__raw_readl(S3C_HCLK_GATE) | 1 << 11), S3C_HCLK_GATE);
	__raw_writel((__raw_readl(S3C_SCLK_GATE) | 1 << 1), S3C_SCLK_GATE);
}

static void clk_jpeg_disable(void)
{
	__raw_writel((__raw_readl(S3C_HCLK_GATE) & ~(1 << 11)), S3C_HCLK_GATE);
	__raw_writel((__raw_readl(S3C_SCLK_GATE) & ~(1 << 1)), S3C_SCLK_GATE);
}

irqreturn_t s3c_jpeg_irq(int irq, void *dev_id)
{
	unsigned int	intReason;
	unsigned int	status;

	status = JPGMem.v_pJPG_REG->JPGStatus;
	intReason = JPGMem.v_pJPG_REG->JPGIRQStatus;

	if(intReason) {
		intReason &= ((1<<6)|(1<<4)|(1<<3));

		switch(intReason) {
			case 0x08 : 
				jpg_irq_reason = OK_HD_PARSING; 
				break;
			case 0x00 : 
				jpg_irq_reason = ERR_HD_PARSING; 
				break;
			case 0x40 : 
				jpg_irq_reason = OK_ENC_OR_DEC; 
				break;
			case 0x10 : 
				jpg_irq_reason = ERR_ENC_OR_DEC; 
				break;
			default : 
				jpg_irq_reason = ERR_UNKNOWN;
		}
		wake_up_interruptible(&WaitQueue_JPEG);
	}	
	else {
		jpg_irq_reason = ERR_UNKNOWN;
		wake_up_interruptible(&WaitQueue_JPEG);
	}

	return IRQ_HANDLED;
}

static int s3c_jpeg_open(struct inode *inode, struct file *file)
{
	S3C6400_JPG_CTX *JPGRegCtx;
	DWORD	ret;

#ifdef USE_JPEG_DOMAIN_GATING
	s3c_set_normal_cfg(S3C64XX_DOMAIN_I, S3C64XX_ACTIVE_MODE, S3C64XX_JPEG);
	if(s3c_wait_blk_pwr_ready(S3C64XX_BLK_I)) {
		printk(KERN_INFO "s3c_wait_blk_pwr_ready(S3C64XX_BLK_I)\n");
		return -1;
	}
#endif /* USE_JPEG_DOMAIN_GATING */

	clk_jpeg_enable();

	JPEG_LOG_MSG(LOG_TRACE, "s3c_jpeg_open", "JPG_open \r\n");

	ret = LockJPGMutex();
	if(!ret){
		JPEG_LOG_MSG(LOG_ERROR, "s3c_jpeg_open", "DD::JPG Mutex Lock Fail\r\n");
		UnlockJPGMutex();
		return FALSE;
	}

	JPGRegCtx = (S3C6400_JPG_CTX *)MemAlloc(sizeof(S3C6400_JPG_CTX));
	memset(JPGRegCtx, 0x00, sizeof(S3C6400_JPG_CTX));

	JPGRegCtx->v_pJPG_REG = JPGMem.v_pJPG_REG;

	if (instanceNo > MAX_INSTANCE_NUM){
		JPEG_LOG_MSG(LOG_ERROR, "s3c_jpeg_open", "DD::Instance Number error-JPEG is running, instance number is %d\n", instanceNo);
		UnlockJPGMutex();
		return FALSE;
	}

	instanceNo++;

	UnlockJPGMutex();

	file->private_data = (S3C6400_JPG_CTX *)JPGRegCtx;

	return 0;
}


static int s3c_jpeg_release(struct inode *inode, struct file *file)
{
	DWORD			ret;
	S3C6400_JPG_CTX	*JPGRegCtx;

	JPEG_LOG_MSG(LOG_TRACE, "s3c_jpeg_release", "JPG_Close\n");

	ret = LockJPGMutex();
	if(!ret){
		JPEG_LOG_MSG(LOG_ERROR, "s3c_jpeg_release", "DD::JPG Mutex Lock Fail\r\n");
		return FALSE;
	}

	JPGRegCtx = (S3C6400_JPG_CTX *)file->private_data;
	if(!JPGRegCtx){
		JPEG_LOG_MSG(LOG_ERROR, "s3c_jpeg_release", "DD::JPG Invalid Input Handle\r\n");
		return FALSE;
	}

	if((--instanceNo) < 0)
		instanceNo = 0;

	kfree(JPGRegCtx);
	UnlockJPGMutex();

	clk_jpeg_disable();
	
#ifdef USE_JPEG_DOMAIN_GATING
	s3c_set_normal_cfg(S3C64XX_DOMAIN_I, S3C64XX_LP_MODE, S3C64XX_JPEG);
#endif /* USE_JPEG_DOMAIN_GATING */

	return 0;
}


static ssize_t s3c_jpeg_write (struct file *file, const char *buf, size_t
		count, loff_t *pos)
{
	return 0;
}

static ssize_t s3c_jpeg_read(struct file *file, char *buf, size_t count, loff_t *pos)
{	
	return 0;
}

static int s3c_jpeg_ioctl(struct inode *inode, struct file *file, unsigned
		int cmd, unsigned long arg)
{
	S3C6400_JPG_CTX		*JPGRegCtx;
	s3c_jpeg_t			*s3c_jpeg_buf;

	JPG_DEC_PROC_PARAM	* DecParam;
	JPG_ENC_PROC_PARAM	* EncParam;
	BOOL				result = TRUE;
	DWORD				ret;
	
	ret = LockJPGMutex();
	if(!ret){
		JPEG_LOG_MSG(LOG_ERROR, "s3c_jpeg_ioctl", "DD::JPG Mutex Lock Fail\r\n");
		return FALSE;
	}

	JPGRegCtx = (S3C6400_JPG_CTX *)file->private_data;
	if(!JPGRegCtx){
		JPEG_LOG_MSG(LOG_ERROR, "s3c_jpeg_ioctl", "DD::JPG Invalid Input Handle\r\n");
		return FALSE;
	}
	switch (cmd) 
	{
		case IOCTL_JPG_DECODE:

			JPEG_LOG_MSG(LOG_TRACE, "s3c_jpeg_ioctl", "IOCTL_JPEG_DECODE\n");

			DecParam = (JPG_DEC_PROC_PARAM *)arg;

			//JPEG_Copy_From_User(&DecParam, (JPG_DEC_PROC_PARAM *)arg, sizeof(JPG_DEC_PROC_PARAM));
//			JPGRegCtx->v_pJPGData_Buff = JPGMem.v_pJPGData_Buff;
//			JPGRegCtx->p_pJPGData_Buff = JPGMem.p_pJPGData_Buff;
//			JPGRegCtx->v_pYUVData_Buff = JPGMem.v_pYUVData_Buff;
//			JPGRegCtx->p_pYUVData_Buff = JPGMem.p_pYUVData_Buff;

			result = decodeJPG(JPGRegCtx, DecParam);

			JPEG_LOG_MSG(LOG_TRACE, "s3c_jpeg_ioctl", "width : %d hegiht : %d size : %d\n", 
					DecParam->width, DecParam->height, DecParam->dataSize);

			//JPEG_Copy_To_User((void *)arg, (void *)&DecParam, sizeof(JPG_DEC_PROC_PARAM));
			
			break;

		case IOCTL_JPG_ENCODE:
		
			JPEG_LOG_MSG(LOG_TRACE, "s3c_jpeg_ioctl", "IOCTL_JPEG_ENCODE\n");

			EncParam = (JPG_ENC_PROC_PARAM *)arg;

			//JPEG_Copy_From_User(&EncParam, (JPG_ENC_PROC_PARAM *)arg, sizeof(JPG_ENC_PROC_PARAM));

			JPEG_LOG_MSG(LOG_TRACE, "s3c_jpeg_ioctl", "width : %d hegiht : %d\n", 
					EncParam->width, EncParam->height);

			/*
			if(EncParam.encType == JPG_MAIN)
			{
				JPGRegCtx->v_pJPGData_Buff = JPGMem.v_pJPGData_Buff;
				JPGRegCtx->p_pJPGData_Buff = JPGMem.p_pJPGData_Buff;
				JPGRegCtx->v_pYUVData_Buff = JPGMem.v_pYUVData_Buff;
				JPGRegCtx->p_pYUVData_Buff = JPGMem.p_pYUVData_Buff;
			}
			else {
				JPGRegCtx->v_pJPGData_Buff = JPGMem.frmUserThumbBuf;
				JPGRegCtx->p_pJPGData_Buff = JPGMem.p_frmUserThumbBuf;
				JPGRegCtx->v_pYUVData_Buff = JPGMem.strUserThumbBuf;
				JPGRegCtx->p_pYUVData_Buff = JPGMem.p_strUserThumbBuf;
			}
			*/
			result = encodeJPG(JPGRegCtx, EncParam);

			JPEG_LOG_MSG(LOG_TRACE, "s3c_jpeg_ioctl", "encoded file size : %d\n", EncParam->fileSize);

			//JPEG_Copy_To_User((void *)arg, (void *)&EncParam,  sizeof(JPG_ENC_PROC_PARAM));

			break;

		case IOCTL_JPG_SET_STRBUF:
			
			s3c_jpeg_buf = (s3c_jpeg_t *)arg;

			//JPEG_Copy_From_User(&s3c_jpeg_buf, (s3c_jpeg_t *)arg, sizeof(s3c_jpeg_t));
			//JPGMem.p_pJPGData_Buff = (unsigned int)s3c_jpeg_buf->phys_addr; 
			//JPGMem.v_pJPGData_Buff = (unsigned char *)s3c_jpeg_buf->virt_addr;
			
			JPGRegCtx->p_pJPGData_Buff = (unsigned int)   s3c_jpeg_buf->phys_addr; 
			JPGRegCtx->v_pJPGData_Buff = (unsigned char *)s3c_jpeg_buf->virt_addr;

			JPEG_LOG_MSG(LOG_TRACE, "s3c_jpeg_ioctl", "IOCTL_JPG_GET_STRBUF\n");
			
			break;	

		case IOCTL_JPG_SET_FRMBUF:
			
			s3c_jpeg_buf = (s3c_jpeg_t *)arg;

			//JPEG_Copy_From_User(s3c_jpeg_buf, (s3c_jpeg_t *)arg, sizeof(s3c_jpeg_t));
			//JPGMem.p_pYUVData_Buff = (unsigned int) s3c_jpeg_buf->phys_addr; 
			//JPGMem.v_pYUVData_Buff = (unsigned char *)s3c_jpeg_buf->virt_addr;

			JPGRegCtx->p_pYUVData_Buff = (unsigned int)   s3c_jpeg_buf->phys_addr; 
			JPGRegCtx->v_pYUVData_Buff = (unsigned char *)s3c_jpeg_buf->virt_addr;

			JPEG_LOG_MSG(LOG_TRACE, "s3c_jpeg_ioctl", "IOCTL_JPG_GET_FRMBUF\n");
			break;


		case IOCTL_JPG_SET_THUMB_STRBUF:
			
			s3c_jpeg_buf = (s3c_jpeg_t *)arg;
			
			//JPEG_Copy_From_User(s3c_jpeg_buf, (s3c_jpeg_t *)arg, sizeof(s3c_jpeg_t));
			//JPGMem.p_strUserThumbBuf = (unsigned int)s3c_jpeg_buf->phys_addr; 
			//JPGMem.strUserThumbBuf   = (unsigned char *)s3c_jpeg_buf->virt_addr;

			JPGRegCtx->p_strUserThumbBuf = (unsigned int)   s3c_jpeg_buf->phys_addr; 
			JPGRegCtx->strUserThumbBuf   = (unsigned char *)s3c_jpeg_buf->virt_addr;

			JPEG_LOG_MSG(LOG_TRACE, "s3c_jpeg_ioctl", "IOCTL_JPG_GET_STRBUF\n");
			break;

		case IOCTL_JPG_SET_THUMB_FRMBUF:
			
			s3c_jpeg_buf = (s3c_jpeg_t *)arg;
			
			//JPEG_Copy_From_User(s3c_jpeg_buf, (s3c_jpeg_t *)arg, sizeof(s3c_jpeg_t));
			//JPGMem.p_frmUserThumbBuf = (unsigned int)s3c_jpeg_buf->phys_addr; 
			//JPGMem.frmUserThumbBuf   = (unsigned char *)s3c_jpeg_buf->virt_addr;

			JPGRegCtx->p_frmUserThumbBuf = (unsigned int)   s3c_jpeg_buf->phys_addr; 
			JPGRegCtx->frmUserThumbBuf   = (unsigned char *)s3c_jpeg_buf->virt_addr;

			JPEG_LOG_MSG(LOG_TRACE, "s3c_jpeg_ioctl", "IOCTL_JPG_GET_STRBUF\n");

			break;

		default : 
			JPEG_LOG_MSG(LOG_ERROR, "s3c_jpeg_ioctl", "DD::JPG Invalid ioctl : 0x%X\r\n", cmd);
	}

	UnlockJPGMutex();
	return result;
}

static struct file_operations jpeg_fops = {
owner:		THIS_MODULE,
			open:		s3c_jpeg_open,
			release:	s3c_jpeg_release,
			ioctl:		s3c_jpeg_ioctl,
			read:		s3c_jpeg_read,
			write:		s3c_jpeg_write,
};


static struct miscdevice s3c_jpeg_miscdev = {
minor:		254, 		
			name:		"s3c-jpg",
			fops:		&jpeg_fops
};


static int s3c_jpeg_probe(struct platform_device *pdev)
{
	struct resource *res;
	static int		size;
	static int		ret;
	HANDLE 			h_Mutex;
	unsigned int	jpg_clk;
	
#ifdef USE_JPEG_DOMAIN_GATING
	s3c_set_normal_cfg(S3C64XX_DOMAIN_I, S3C64XX_ACTIVE_MODE, S3C64XX_JPEG);
	if(s3c_wait_blk_pwr_ready(S3C64XX_BLK_I)) {
		printk(KERN_INFO "s3c_wait_blk_pwr_ready(S3C64XX_BLK_I)\n");
		return -1;
	}
#endif /* USE_JPEG_DOMAIN_GATING */

	// JPEG clock enable 
	jpeg_hclk	= clk_get(NULL, "hclk_jpeg");
	if (IS_ERR(jpeg_hclk)) {
		printk(KERN_ERR "failed to get jpeg hclk source\n");
		return -ENOENT;
	}
	clk_jpeg_enable();

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		printk(KERN_INFO "failed to get memory region resouce\n");
		return -ENOENT;
	}

	size = (res->end-res->start)+1;
	jpeg_mem = request_mem_region(res->start, size, pdev->name);
	if (jpeg_mem == NULL) {
		printk(KERN_INFO "failed to get memory region\n");
		return -ENOENT;
	}

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		printk(KERN_INFO "failed to get irq resource\n");
		return -ENOENT;
	}

	irq_no = res->start;
	ret = request_irq(res->start, s3c_jpeg_irq, 0, pdev->name, pdev);
	if (ret != 0) {
		printk(KERN_INFO "failed to install irq (%d)\n", ret);
		return ret;
	}

	jpeg_base = ioremap(res->start, size);
	if (jpeg_base == 0) {
		printk(KERN_INFO "failed to ioremap() region\n");
		return -EINVAL;
	}

	// JPEG clock was set as 66 MHz
	jpg_clk = readl(S3C_CLK_DIV0);
	jpg_clk = (jpg_clk & ~(0xF << 24)) | (3 << 24);
	__raw_writel(jpg_clk, S3C_CLK_DIV0);

	JPEG_LOG_MSG(LOG_TRACE, "s3c_jpeg_probe", "JPG_Init\n");

	// Mutex initialization
	h_Mutex = CreateJPGmutex();
	if (h_Mutex == NULL) 
	{
		JPEG_LOG_MSG(LOG_ERROR, "s3c_jpeg_probe", "DD::JPG Mutex Initialize error\r\n");
		return FALSE;
	}

	ret = LockJPGMutex();
	if (!ret){
		JPEG_LOG_MSG(LOG_ERROR, "s3c_jpeg_probe", "DD::JPG Mutex Lock Fail\n");
		return FALSE;
	}

	// Memory initialization
	if( !JPGMemMapping(&JPGMem) ){
		JPEG_LOG_MSG(LOG_ERROR, "s3c_jpeg_probe", "DD::JPEG-HOST-MEMORY Initialize error\r\n");
		UnlockJPGMutex();
		return FALSE;
	}
/*	else {
		if (!JPGBuffMapping(&JPGMem)){
			JPEG_LOG_MSG(LOG_ERROR, "s3c_jpeg_probe", "DD::JPEG-DATA-MEMORY Initialize error : %d\n");
			UnlockJPGMutex();
			return FALSE;	
		}
	}*/

	instanceNo = 0;

	UnlockJPGMutex();

	ret = misc_register(&s3c_jpeg_miscdev);

	clk_jpeg_disable();
	
#ifdef USE_JPEG_DOMAIN_GATING
	s3c_set_normal_cfg(S3C64XX_DOMAIN_I, S3C64XX_LP_MODE, S3C64XX_JPEG);
#endif /* USE_JPEG_DOMAIN_GATING */
	return 0;
}

static int s3c_jpeg_remove(struct platform_device *dev)
{
	if (jpeg_mem != NULL) {
		release_resource(jpeg_mem);
		kfree(jpeg_mem);
		jpeg_mem = NULL;
	}

	free_irq(irq_no, dev);
	misc_deregister(&s3c_jpeg_miscdev);
	return 0;
}


static struct platform_driver s3c_jpeg_driver = {
	.probe		= s3c_jpeg_probe,
	.remove		= s3c_jpeg_remove,
	.shutdown	= NULL,
	.suspend	= NULL,
	.resume		= NULL,
	.driver		= {
		.owner		= THIS_MODULE,
		.name		= "s3c-jpeg",
	},
};

static char banner[] __initdata = KERN_INFO "S3C JPEG Driver, (c) 2007 Samsung Electronics\n";

static int __init s3c_jpeg_init(void)
{
	printk(banner);
	return platform_driver_register(&s3c_jpeg_driver);
}

static void __exit s3c_jpeg_exit(void)
{
	DWORD	ret;

	JPEG_LOG_MSG(LOG_TRACE, "s3c_jpeg_exit", "JPG_Deinit\n");

	ret = LockJPGMutex();
	if(!ret){
		JPEG_LOG_MSG(LOG_ERROR, "s3c_jpeg_exit", "DD::JPG Mutex Lock Fail\r\n");
	}

	JPGMemFree(&JPGMem);
	UnlockJPGMutex();

	DeleteJPGMutex();	

	platform_driver_unregister(&s3c_jpeg_driver);	
	printk("S3C JPEG driver module exit\n");
}

module_init(s3c_jpeg_init);
module_exit(s3c_jpeg_exit);

MODULE_AUTHOR("Jiun, Yu");
MODULE_LICENSE("GPL");
