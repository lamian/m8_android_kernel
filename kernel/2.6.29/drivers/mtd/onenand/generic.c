/*
 *  linux/drivers/mtd/onenand/generic.c
 *
 *  Copyright (c) 2005 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Overview:
 *   This is a device driver for the OneNAND flash for generic boards.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>
#include <asm/mach/flash.h>
#include <plat/nand.h>
#include <plat/regs-clock.h>
#include <plat/map-base.h>

#define DRIVER_NAME	"onenand"


#ifdef CONFIG_MTD_PARTITIONS
static const char *part_probes[] = { "cmdlinepart", NULL,  };
#endif

struct onenand_info {
	struct mtd_info		mtd;
	struct mtd_partition	*parts;
	struct onenand_chip	onenand;
};

#ifdef CONFIG_PM		/* Evan Tan, 2009-04-08 */
#include <plat/pm.h>

static struct sleep_save ond_save[] = {
	SAVE_ITEM((void __iomem *)0x1C0),
	SAVE_ITEM((void __iomem *)0x0D0),
	SAVE_ITEM((void __iomem *)0x0E0),
	SAVE_ITEM((void __iomem *)0x0F0),
	SAVE_ITEM((void __iomem *)0x160),
	SAVE_ITEM((void __iomem *)0x050),
	SAVE_ITEM((void __iomem *)0x040),
	SAVE_ITEM((void __iomem *)0x1A0),
	SAVE_ITEM((void __iomem *)0x000),
	SAVE_ITEM((void __iomem *)0x010),
};

static int generic_onenand_resume(struct device *dev)
{
	struct onenand_info *info = dev_get_drvdata(dev);
	unsigned long value;
	extern int onenand_resume_left(struct mtd_info *mtd);
	
	value = __raw_readl(S3C_CLK_DIV0);
	value = (value & ~(3 << 16)) | (1 << 16);
	__raw_writel(value, S3C_CLK_DIV0);

	__raw_writel(0x01, info->onenand.base + 0x300);
	__raw_writel(0x02, info->onenand.base + 0x020);
	s3c6410_pm_do_restore(ond_save, ARRAY_SIZE(ond_save));
	onenand_resume_left(&info->mtd);
	
	return 0;
}

static int generic_onenand_suspend(struct device *dev, pm_message_t state)
{
	s3c6410_pm_do_save(ond_save, ARRAY_SIZE(ond_save));

	return 0;
}
#endif
static int __devinit generic_onenand_probe(struct device *dev)
{
	struct onenand_info *info;
	struct platform_device *pdev = to_platform_device(dev);
	struct s3c_nand_mtd_info *pdata = pdev->dev.platform_data;
	struct resource *res = pdev->resource;
	unsigned long size = res->end - res->start + 1;
	int err;

	info = kzalloc(sizeof(struct onenand_info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	if (!request_mem_region(res->start, size, dev->driver->name)) {
		err = -EBUSY;
		goto out_free_info;
	}

	info->onenand.base = ioremap(res->start, size);
	if (!info->onenand.base) {
		err = -ENOMEM;
		goto out_release_mem_region;
	}

	//info->onenand.mmcontrol = pdata->mmcontrol;
	info->onenand.irq = platform_get_irq(pdev, 0);

	info->mtd.name = pdev->dev.bus_id;
	info->mtd.priv = &info->onenand;
	info->mtd.owner = THIS_MODULE;

	if (onenand_scan(&info->mtd, 1)) {
		err = -ENXIO;
		goto out_iounmap;
	}

#ifdef CONFIG_MTD_PARTITIONS
	err = parse_mtd_partitions(&info->mtd, part_probes, &info->parts, 0);
	if (err > 0)
		{
		add_mtd_partitions(&info->mtd, info->parts, err);
		}
	else if (err <= 0 && pdata->partition)
		{
		add_mtd_partitions(&info->mtd, pdata->partition, pdata->mtd_part_nr);
		}
	else
#endif
		err = add_mtd_device(&info->mtd);

	dev_set_drvdata(&pdev->dev, info);
#ifdef CONFIG_PM		/* Evan Tan, 2009-04-08 */
{
	int i;
	for (i=0; i<sizeof(ond_save)/sizeof(struct sleep_save); i++)
	{
		ond_save[i].reg = (void __iomem *)((unsigned long)ond_save[i].reg+(unsigned long)info->onenand.base);
	}
}
#endif

	return 0;

out_iounmap:
	iounmap(info->onenand.base);
out_release_mem_region:
	release_mem_region(res->start, size);
out_free_info:
	kfree(info);

	return err;
}

static int __devexit generic_onenand_remove(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct onenand_info *info = dev_get_drvdata(&pdev->dev);
	struct resource *res = pdev->resource;
	unsigned long size = res->end - res->start + 1;

	dev_set_drvdata(&pdev->dev, NULL);

	if (info) {
		if (info->parts)
			del_mtd_partitions(&info->mtd);
		else
			del_mtd_device(&info->mtd);

		onenand_release(&info->mtd);
		release_mem_region(res->start, size);
		iounmap(info->onenand.base);
		kfree(info);
	}

	return 0;
}

static struct device_driver generic_onenand_driver = {
	.name		= DRIVER_NAME,
	.bus		= &platform_bus_type,
	.probe		= generic_onenand_probe,
	.remove		= __devexit_p(generic_onenand_remove),
#ifdef CONFIG_PM		/* Evan Tan, 2009-04-08 */
	.suspend		= generic_onenand_suspend,
	.resume		= generic_onenand_resume,
#endif
};

MODULE_ALIAS(DRIVER_NAME);

static int __init generic_onenand_init(void)
{
	return driver_register(&generic_onenand_driver);
}

static void __exit generic_onenand_exit(void)
{
	driver_unregister(&generic_onenand_driver);
}

module_init(generic_onenand_init);
module_exit(generic_onenand_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kyungmin Park <kyungmin.park@samsung.com>");
MODULE_DESCRIPTION("Glue layer for OneNAND flash on generic boards");
