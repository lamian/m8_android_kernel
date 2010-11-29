/* linux/drivers/media/video/samsung/s3c_fimc_core.c
 *
 * Core file for Samsung Camera Interface (FIMC) driver
 *
 * Jinsung Yang, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/memory.h>
#include <plat/clock.h>
#include <plat/media.h>
#include <plat/reserved_mem.h>

#include "s3c_fimc.h"
#ifdef CONFIG_PM
#include <plat/regs-fimc.h>
#endif

static struct s3c_fimc_camera test_pattern = {
	.id 		= S3C_FIMC_TPID,
	.type		= CAM_TYPE_ITU,
	.mode		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_YCBYCR,
	.clockrate	= 0,
	.width		= 640,
	.height		= 480,
	.offset		= {
		.h1 = 0,
		.h2 = 0,
		.v1 = 0,
		.v2 = 0,
	},

	.polarity	= {
		.pclk	= 0,
		.vsync	= 0,
		.href	= 0,
		.hsync	= 0,
	},

	.initialized	= 0,	
};

struct s3c_fimc_config s3c_fimc;

struct s3c_platform_fimc *to_fimc_plat(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	return (struct s3c_platform_fimc *) pdev->dev.platform_data;
}

u8 s3c_fimc_i2c_read(struct i2c_client *client, u8 subaddr)
{
	u8 buf[1];
	struct i2c_msg msg = {client->addr, 0, 1, buf};
	int ret;
	
	buf[0] = subaddr;

	ret = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
	if (ret == -EIO) {
		err("i2c transfer error\n");
		return -EIO;
	}

	msg.flags = I2C_M_RD;
	ret = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;

	return buf[0];
}

int s3c_fimc_i2c_write(struct i2c_client *client, u8 subaddr, u8 val)
{
	u8 buf[2];
	struct i2c_msg msg = {client->addr, 0, 2, buf};

	buf[0] = subaddr;
	buf[1] = val;

	return i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
}

void s3c_fimc_i2c_command(struct s3c_fimc_control *ctrl, u32 cmd, int arg)
{
	struct i2c_client *client = ctrl->in_cam->client;

	printk("ctrl->in_cam->client = 0x%x\n", ctrl->in_cam->client);
	if (client)
		client->driver->command(client, cmd, (void *) arg);
	else
		err("i2c client is not registered\n");
}

void s3c_fimc_register_camera(struct s3c_fimc_camera *cam)
{
	s3c_fimc.camera[cam->id] = cam;

	clk_disable(s3c_fimc.cam_clock);
	clk_set_rate(s3c_fimc.cam_clock, cam->clockrate);
	clk_enable(s3c_fimc.cam_clock);

	s3c_fimc_reset_camera();
}

void s3c_fimc_unregister_camera(struct s3c_fimc_camera *cam)
{
	int i = 0;

	for (i = 0; i < S3C_FIMC_MAX_CTRLS; i++) {
		if (s3c_fimc.ctrl[i].in_cam == cam)
			s3c_fimc.ctrl[i].in_cam = NULL;
	}
	
	s3c_fimc.camera[cam->id] = NULL;
}

void s3c_fimc_set_active_camera(struct s3c_fimc_control *ctrl, int id)
{
	ctrl->in_cam = s3c_fimc.camera[id];

	if (ctrl->in_cam)
		s3c_fimc_select_camera(ctrl);
}

void s3c_fimc_init_camera(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_camera *cam = ctrl->in_cam;

	if (cam && !cam->initialized) {
		s3c_fimc_i2c_command(ctrl, I2C_CAM_INIT, 0);
		s3c_fimc_change_resolution(ctrl, CAM_RES_DEFAULT);



		cam->initialized = 1;
	}
}

static irqreturn_t s3c_fimc_irq(int irq, void *dev_id)
{
	struct s3c_fimc_control *ctrl = (struct s3c_fimc_control *) dev_id;

	s3c_fimc_clear_irq(ctrl);
	s3c_fimc_check_fifo(ctrl);

	if (IS_CAPTURE(ctrl)) {
		dev_dbg(ctrl->dev, "irq is in capture state\n");

		if (s3c_fimc_frame_handler(ctrl) == S3C_FIMC_FRAME_SKIP)
			return IRQ_HANDLED;

		wake_up_interruptible(&ctrl->waitq);
	}

	return IRQ_HANDLED;
}

static
struct s3c_fimc_control *s3c_fimc_register_controller(struct platform_device *pdev)
{
	struct s3c_platform_fimc *pdata;
	struct s3c_fimc_control *ctrl;
	struct resource *res;
	int i = S3C_FIMC_MAX_CTRLS - 1;
	int id = pdev->id;

	pdata = to_fimc_plat(&pdev->dev);

	ctrl = &s3c_fimc.ctrl[id];
	ctrl->id = id;
	ctrl->dev = &pdev->dev;
	ctrl->vd = &s3c_fimc_video_device[id];
	ctrl->rot90 = 0;
	ctrl->vd->minor = id;
	ctrl->out_frame.nr_frames = pdata->nr_frames;
	ctrl->out_frame.skip_frames = 0;
	ctrl->scaler.line_length = pdata->line_length;

	sprintf(ctrl->name, "%s%d", S3C_FIMC_NAME, id);
	strcpy(ctrl->vd->name, ctrl->name);

	ctrl->open_lcdfifo = s3cfb_enable_local;
	ctrl->close_lcdfifo = s3cfb_enable_dma;

	atomic_set(&ctrl->in_use, 0);
	mutex_init(&ctrl->lock);
	init_waitqueue_head(&ctrl->waitq);

	/* get resource for io memory */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		err("failed to get io memory region\n");
		return NULL;
	}

	if (!pdata->shared_io) {
		/* request mem region */
		res = request_mem_region(res->start, res->end - res->start + 1, pdev->name);
		if (!res) {
			err("failed to request io memory region\n");
			return NULL;
		}

		/* ioremap for register block */
		ctrl->regs = ioremap(res->start, res->end - res->start + 1);
	} else {
		while (i >= 0 && ctrl->regs == NULL) {
			ctrl->regs = s3c_fimc.ctrl[i].regs;
			i--;
		}
	}

	if (!ctrl->regs) {
		err("failed to remap io region\n");
		return NULL;
	}

	/* irq */
	ctrl->irq = platform_get_irq(pdev, 0);

	if (request_irq(ctrl->irq, s3c_fimc_irq, IRQF_DISABLED, ctrl->name, ctrl))
		err("request_irq failed\n");

	s3c_fimc_reset(ctrl);
	s3c_fimc_set_active_camera(ctrl, 0);

	return ctrl;
}

static int s3c_fimc_unregister_controller(struct platform_device *pdev)
{
	struct s3c_fimc_control *ctrl;
	struct s3c_platform_fimc *pdata;
	int id = pdev->id;

	ctrl = &s3c_fimc.ctrl[id];

	s3c_fimc_free_output_memory(&ctrl->out_frame);

	pdata = to_fimc_plat(ctrl->dev);

	if (!pdata->shared_io)
		iounmap(ctrl->regs);

	memset(ctrl, 0, sizeof(*ctrl));
	
	return 0;
}

static int s3c_fimc_mmap(struct file* filp, struct vm_area_struct *vma)
{
	struct s3c_fimc_control *ctrl = filp->private_data;
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;

	u32 size = vma->vm_end - vma->vm_start;
	u32 pfn, total_size = frame->buf_size;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	vma->vm_flags |= VM_RESERVED;

	/* page frame number of the address for a source frame to be stored at. */
	pfn = __phys_to_pfn(frame->addr[vma->vm_pgoff].phys_y);

	
	if (size > total_size) {
		err("the size of mapping is too big\n");
		return -EINVAL;
	}

	if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED)) {
		err("writable mapping must be shared\n");
		return -EINVAL;
	}

	if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot)) {
		err("mmap fail\n");
		return -EINVAL;
	}

	return 0;
}

static u32 s3c_fimc_poll(struct file *filp, poll_table *wait)
{
	struct s3c_fimc_control *ctrl = filp->private_data;
	u32 mask = 0;

	poll_wait(filp, &ctrl->waitq, wait);

	if (IS_IRQ_HANDLING(ctrl))
		mask = POLLIN | POLLRDNORM;

	FSET_STOP(ctrl);

	return mask;
}

static
ssize_t s3c_fimc_read(struct file *filp, char *buf, size_t count, loff_t *pos)
{
	struct s3c_fimc_control *ctrl = filp->private_data;
	size_t end;

	if (IS_CAPTURE(ctrl)) {
		if (wait_event_interruptible(ctrl->waitq, IS_IRQ_HANDLING(ctrl)))
				return -ERESTARTSYS;

		FSET_STOP(ctrl);
	}

	end = min_t(size_t, ctrl->out_frame.buf_size, count);

	if (copy_to_user(buf, s3c_fimc_get_current_frame(ctrl), end))
		return -EFAULT;

	return end;
}

static
ssize_t s3c_fimc_write(struct file *filp, const char *b, size_t c, loff_t *offset)
{
	return 0;
}

static int s3c_fimc_open(struct inode *inode, struct file *filp)
{
	struct s3c_fimc_control *ctrl;
	int id, ret;

	id = MINOR(inode->i_rdev);
	ctrl = &s3c_fimc.ctrl[id];

	mutex_lock(&ctrl->lock);

	if (atomic_read(&ctrl->in_use)) {
		ret = -EBUSY;
		goto resource_busy;
	} else {
		atomic_inc(&ctrl->in_use);
		s3c_fimc_reset(ctrl);
		filp->private_data = ctrl;
	}

	mutex_unlock(&ctrl->lock);

	return 0;

resource_busy:
	mutex_unlock(&ctrl->lock);
	return ret;
}

static int s3c_fimc_release(struct inode *inode, struct file *filp)
{
	struct s3c_fimc_control *ctrl;
	int id;

	id = MINOR(inode->i_rdev);
	ctrl = &s3c_fimc.ctrl[id];

	mutex_lock(&ctrl->lock);

	atomic_dec(&ctrl->in_use);
	filp->private_data = NULL;

	mutex_unlock(&ctrl->lock);

	return 0;
}

static const struct file_operations s3c_fimc_fops = {
	.owner = THIS_MODULE,
	.open = s3c_fimc_open,
	.release = s3c_fimc_release,
	.ioctl = video_ioctl2,
	.read = s3c_fimc_read,
	.write = s3c_fimc_write,
	.mmap = s3c_fimc_mmap,
	.poll = s3c_fimc_poll,
};

static void s3c_fimc_vdev_release(struct video_device *vdev)
{
	kfree(vdev);
}

struct video_device s3c_fimc_video_device[S3C_FIMC_MAX_CTRLS] = {
	[0] = {
		.vfl_type = VID_TYPE_OVERLAY | VID_TYPE_CAPTURE | VID_TYPE_CLIPPING | VID_TYPE_SCALES,
		.fops = &s3c_fimc_fops,
		.ioctl_ops = &s3c_fimc_v4l2_ops,
		.release  = s3c_fimc_vdev_release,
	},
	[1] = {
		.vfl_type = VID_TYPE_OVERLAY | VID_TYPE_CAPTURE | VID_TYPE_CLIPPING | VID_TYPE_SCALES,
		.fops = &s3c_fimc_fops,
		.ioctl_ops = &s3c_fimc_v4l2_ops,
		.release  = s3c_fimc_vdev_release,
	},
	[2] = {
		.vfl_type = VID_TYPE_OVERLAY | VID_TYPE_CAPTURE | VID_TYPE_CLIPPING | VID_TYPE_SCALES,
		.fops = &s3c_fimc_fops,
		.ioctl_ops = &s3c_fimc_v4l2_ops,
		.release  = s3c_fimc_vdev_release,
	},
};

static int s3c_fimc_init_global(struct platform_device *pdev)
{
	/* camera clock */
	s3c_fimc.cam_clock = clk_get(&pdev->dev, "sclk_cam");
	if (IS_ERR(s3c_fimc.cam_clock)) {
		err("failed to get camera clock source\n");
		return -EINVAL;
	}

#ifdef CAMERA_RESERVED_MEM_START
	s3c_fimc.dma_start = CAMERA_RESERVED_MEM_START;
	s3c_fimc.dma_total = RESERVED_MEM_CAMERA;
#else
	#error Please define CAMERA_RESERVED_MEM_START and setup CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC.
#endif
	s3c_fimc.dma_current = s3c_fimc.dma_start;

	/* test pattern */
	s3c_fimc.camera[test_pattern.id] = &test_pattern;

	return 0;
}

static int s3c_fimc_probe(struct platform_device *pdev)
{
	struct s3c_platform_fimc *pdata;
	struct s3c_fimc_control *ctrl;
	struct clk *srclk;
	int ret;

	ctrl = s3c_fimc_register_controller(pdev);
	if (!ctrl) {
		err("cannot register fimc controller\n");
		goto err_fimc;
	}

	pdata = to_fimc_plat(&pdev->dev);
	if (pdata->cfg_gpio)
		pdata->cfg_gpio(pdev);

	/* fimc source clock */
	srclk = clk_get(&pdev->dev, pdata->srclk_name);
	if (IS_ERR(srclk)) {
		err("failed to get source clock of fimc\n");
		goto err_clk_io;
	}

	/* fimc clock */
	ctrl->clock = clk_get(&pdev->dev, pdata->clk_name);
	if (IS_ERR(ctrl->clock)) {
		err("failed to get fimc clock source\n");
		goto err_clk_io;
	}

	/* set parent clock */
	if (ctrl->clock->set_parent)
		ctrl->clock->set_parent(ctrl->clock, srclk);

	/* set clockrate for FIMC interface block */
	if (ctrl->clock->set_rate)
		ctrl->clock->set_rate(ctrl->clock, pdata->clockrate);

	clk_enable(ctrl->clock);

	/* things to initialize once */
	if (ctrl->id == 0) {
		ret = s3c_fimc_init_global(pdev);
		if (ret)
			goto err_global;
	}

	ret = video_register_device(ctrl->vd, VFL_TYPE_GRABBER, ctrl->id);
	if (ret) {
		err("cannot register video driver\n");
		goto err_video;
	}

	info("controller %d registered successfully\n", ctrl->id);

	return 0;

err_video:
	clk_put(s3c_fimc.cam_clock);

err_global:
	clk_disable(ctrl->clock);
	clk_put(ctrl->clock);

err_clk_io:
	s3c_fimc_unregister_controller(pdev);

err_fimc:
	return -EINVAL;
	
}

static int s3c_fimc_remove(struct platform_device *pdev)
{
	s3c_fimc_unregister_controller(pdev);

	return 0;
}

//static struct sleep_save fimc_save[];
static unsigned int save_reg[11];

int s3c_fimc_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct s3c_fimc_control *ctrl;
    int id = pdev->id;

	ctrl = &s3c_fimc.ctrl[id];

	save_reg[0] = __raw_readl(ctrl->regs + S3C_CIGCTRL);
	save_reg[1] = __raw_readl(ctrl->regs + S3C_CICOCTRL);
	save_reg[2] = __raw_readl(ctrl->regs + S3C_CICOSCPRERATIO);
	save_reg[3] = __raw_readl(ctrl->regs + S3C_CICOSCPREDST);
	save_reg[4] = __raw_readl(ctrl->regs + S3C_CICOSCCTRL);
	save_reg[5] = __raw_readl(ctrl->regs + S3C_CIPRCTRL);
	save_reg[6] = __raw_readl(ctrl->regs + S3C_CIPRSCPRERATIO);
	save_reg[7] = __raw_readl(ctrl->regs + S3C_CIPRSCPREDST);
	save_reg[8] = __raw_readl(ctrl->regs + S3C_CIPRSCCTRL);
	save_reg[9] = __raw_readl(ctrl->regs + S3C_MSCOCTRL);
	save_reg[10] = __raw_readl(ctrl->regs + S3C_MSPRCTRL);
	if(ctrl->in_cam){
		ctrl->in_cam->initialized = 0;
		}

	clk_disable(ctrl->clock);

	return 0;
}

int s3c_fimc_resume(struct platform_device *pdev)
{
	struct s3c_fimc_control *ctrl;
	struct s3c_platform_fimc *pdata;
	int id = pdev->id;

	ctrl = &s3c_fimc.ctrl[id];

	clk_enable(ctrl->clock);

	__raw_writel(save_reg[0], ctrl->regs + S3C_CIGCTRL);
	__raw_writel(save_reg[1], ctrl->regs + S3C_CICOCTRL);
	__raw_writel(save_reg[2], ctrl->regs + S3C_CICOSCPRERATIO);
	__raw_writel(save_reg[3], ctrl->regs + S3C_CICOSCPREDST);
	__raw_writel(save_reg[4], ctrl->regs + S3C_CICOSCCTRL);
	__raw_writel(save_reg[5], ctrl->regs + S3C_CIPRCTRL);
	__raw_writel(save_reg[6], ctrl->regs + S3C_CIPRSCPRERATIO);
	__raw_writel(save_reg[7], ctrl->regs + S3C_CIPRSCPREDST);
	__raw_writel(save_reg[8], ctrl->regs + S3C_CIPRSCCTRL);
	__raw_writel(save_reg[9], ctrl->regs + S3C_MSCOCTRL);
	__raw_writel(save_reg[10], ctrl->regs + S3C_MSPRCTRL);

	pdata = to_fimc_plat(&pdev->dev);
	if (pdata->cfg_gpio)
		pdata->cfg_gpio(pdev);

	return 0;
}

static struct platform_driver s3c_fimc_driver = {
	.probe		= s3c_fimc_probe,
	.remove		= s3c_fimc_remove,
	.suspend	= s3c_fimc_suspend,
	.resume		= s3c_fimc_resume,
	.driver		= {
		.name	= "s3c-fimc",
		.owner	= THIS_MODULE,
	},
};

static int s3c_fimc_register(void)
{
	platform_driver_register(&s3c_fimc_driver);

	return 0;
}

static void s3c_fimc_unregister(void)
{
	platform_driver_unregister(&s3c_fimc_driver);
}

module_init(s3c_fimc_register);
module_exit(s3c_fimc_unregister);
	
MODULE_AUTHOR("Jinsung, Yang <jsgood.yang@samsung.com>");
MODULE_DESCRIPTION("Samsung Camera Interface (FIMC) driver");
MODULE_LICENSE("GPL");
