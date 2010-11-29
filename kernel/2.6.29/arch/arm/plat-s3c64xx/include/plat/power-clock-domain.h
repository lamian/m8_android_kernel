/* Copyright 2008 Samsung Electronics
 *     
 *
 * S3C64XX NORMAL CONFIG definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __POWER_DOMAIN_H
#define __POWER_DOMAIN_H __FILE__
/*NORMAL_CFG*/
/* 0 -> 8 RESERVED */
#define S3C64XX_DOMAIN_V	(1 << 9)
#define S3C64XX_DOMAIN_G	(1 << 10)
#define S3C64XX_DOMAIN_I	(1 << 12)
#define S3C64XX_DOMAIN_P	(1 << 13)
#define S3C64XX_DOMAIN_F	(1 << 14)
#define S3C64XX_DOMAIN_S	(1 << 15)
#define S3C64XX_DOMAIN_ETM	(1 << 16)
#define S3C64XX_DOMAIN_IROM	(1 << 30)

#define S3C64XX_LP_MODE		0
#define S3C64XX_ACTIVE_MODE	1

#define S3C64XX_BLK_TOP		(1 << 0)
#define S3C64XX_BLK_V		(1 << 1)
#define S3C64XX_BLK_I		(1 << 2)
#define S3C64XX_BLK_P		(1 << 3)
#define S3C64XX_BLK_F		(1 << 4)
#define S3C64XX_BLK_S		(1 << 5)
#define S3C64XX_BLK_ETM		(1 << 6)
#define S3C64XX_BLK_G		(1 << 7)

#define S3C64XX_3D		(0)
#define S3C64XX_MFC		(1)
#define S3C64XX_JPEG		(2)
#define S3C64XX_CAMERA		(3)
#define S3C64XX_2D		(4)
#define S3C64XX_TVENC		(5)
#define S3C64XX_SCALER		(6)
#define S3C64XX_ROT		(7)
#define S3C64XX_POST		(8)
#define S3C64XX_LCD		(9)
#define S3C64XX_SDMA0		(10)
#define S3C64XX_SDMA1		(11)
#define S3C64XX_SECURITY	(12)
#define S3C64XX_ETM       	(13)
#define S3C64XX_IROM	        (14)

#define S3C64XX_DOMAIN_V_MASK	(1 << 1)
#define S3C64XX_DOMAIN_G_MASK	(1 << 0)
#define S3C64XX_DOMAIN_I_MASK	(0x3 << 2)
#define S3C64XX_DOMAIN_P_MASK	(0x7 << 4)
#define S3C64XX_DOMAIN_F_MASK	(0x7 << 7)
#define S3C64XX_DOMAIN_S_MASK   (0x7 << 10)
#define S3C64XX_DOMAIN_ETM_MASK (1 << 13)
#define S3C64XX_DOMAIN_IROM_MASK (1 << 14)	


#endif  
