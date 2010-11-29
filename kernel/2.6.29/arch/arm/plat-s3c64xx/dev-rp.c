/* linux/arch/arm/plat-s3c64xx/dev-rp.c
 *
 * S3C64XX series device definition for Renderer pipeline
 *
 * Jonghun Han, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>

#include <mach/map.h>

#include <plat/devs.h>
#include <plat/cpu.h>

static struct resource s3c_rp_resource[] = {
	[0] = {
		.start = S3C6400_PA_VPP,
		.end   = S3C6400_PA_VPP + S3C_SZ_VPP - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = S3C6400_PA_ROTATOR,
		.end   = S3C6400_PA_ROTATOR + S3C_SZ_ROTATOR - 1,
		.flags = IORESOURCE_MEM,
	},	
	[2] = {
		.start = S3C6400_PA_TVSCALER,
		.end   = S3C6400_PA_TVSCALER + S3C_SZ_TVSCALER - 1,
		.flags = IORESOURCE_MEM,
	},	
	[3] = {
		.start = IRQ_POST0,
		.end   = IRQ_POST0,
		.flags = IORESOURCE_IRQ,
	},
	[4] = {
		.start = IRQ_ROTATOR,
		.end   = IRQ_ROTATOR,
		.flags = IORESOURCE_IRQ,
	},	
	[5] = {
		.start = IRQ_SCALER,
		.end   = IRQ_SCALER,
		.flags = IORESOURCE_IRQ,
	},	
};

struct platform_device s3c_device_rp = {
	.name		  = "s3c-rp",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_rp_resource),
	.resource	  = s3c_rp_resource,
};

