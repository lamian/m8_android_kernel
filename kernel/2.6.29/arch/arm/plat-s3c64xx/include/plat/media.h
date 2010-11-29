/* linux/arch/arm/plat-s5pc1xx/include/plat/media.h
 *
 * Copyright 2009 Samsung Electronics
 *	Jinsung Yang <jsgood.yang@samsung.com>
 *	http://samsungsemi.com/
 *
 * Samsung Media device descriptions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef _S3C_MEDIA_H
#define _S3C_MEDIA_H

#include <linux/types.h>

#define S3C_MDEV_FIMC		0
#define S3C_MDEV_POST		1
#define S3C_MDEV_TV		2
#define S3C_MDEV_MFC		3
#define S3C_MDEV_JPEG		4
#define S3C_MDEV_CMM		5
#define S3C_MDEV_RP		6
#define S3C_MDEV_G3D		7
#define S3C_MDEV_MAX		8

struct s3c_media_device {
	int		id;
	const char 	*name;
	size_t		memsize;
	dma_addr_t	paddr;
};

extern dma_addr_t s3c_get_media_memory(int dev_id);
extern size_t s3c_get_media_memsize(int dev_id);

#endif
