/* linux/arch/arm/plat-s3c/include/plat/regs-rtc.h
 *
 * Copyright (c) 2003 Simtec Electronics <linux@simtec.co.uk>
 *		      http://www.simtec.co.uk/products/SWLINUX/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * S3C2410 Internal RTC register definition
*/

#ifndef __ASM_ARCH_REGS_RTC_H
#define __ASM_ARCH_REGS_RTC_H __FILE__

#define S3C_RTCREG(x) (x)

#define S3C_INTP	      S3C_RTCREG(0x30)
#define S3C_INTP_ALM	(1<<1)
#define S3C_INTP_TIC	(1<<0)

#define S3C_RTCCON	      S3C_RTCREG(0x40)
#define S3C_RTCCON_RTCEN  (1<<0)
#define S3C_RTCCON_CLKSEL (1<<1)
#define S3C_RTCCON_CNTSEL (1<<2)
#define S3C_RTCCON_CLKRST (1<<3)

#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
#define S3C_MAX_CNT	32768
#define S3C_RTCCON_TICEN	(1<<8)
#define S3C_RTC_TICNT	S3C_RTCREG(0x40)
#else
#define S3C_INTP_ALM	(1<<1)
#define S3C_MAX_CNT	128
#define S3C_RTCCON_TICEN  (1<<7)
#define S3C_RTC_TICNT	S3C_RTCREG(0x44)
#endif

/* Common Reg for samsung AP*/
#define S3C_INTP	S3C_RTCREG(0x30)
#define S3C_INTP_ALM	(1<<1)
#define S3C_INTP_TIC	(1<<0)


#define S3C_TICNT	      S3C_RTCREG(0x44)
#define S3C_TICNT_ENABLE  (1<<7)

#define S3C_RTCALM	S3C_RTCREG(0x50)
#define S3C_RTCALM_ALMEN  (1<<6)
#define S3C_RTCALM_YEAREN (1<<5)
#define S3C_RTCALM_MONEN  (1<<4)
#define S3C_RTCALM_DAYEN  (1<<3)
#define S3C_RTCALM_HOUREN (1<<2)
#define S3C_RTCALM_MINEN  (1<<1)
#define S3C_RTCALM_SECEN  (1<<0)

#define S3C_RTCALM_ALL \
  S3C_RTCALM_ALMEN | S3C_RTCALM_YEAREN | S3C_RTCALM_MONEN |\
  S3C_RTCALM_DAYEN | S3C_RTCALM_HOUREN | S3C_RTCALM_MINEN |\
  S3C_RTCALM_SECEN


#define S3C_ALMSEC	      S3C_RTCREG(0x54)
#define S3C_ALMMIN	      S3C_RTCREG(0x58)
#define S3C_ALMHOUR	      S3C_RTCREG(0x5c)

#define S3C_ALMDATE	      S3C_RTCREG(0x60)
#define S3C_ALMMON	      S3C_RTCREG(0x64)
#define S3C_ALMYEAR	      S3C_RTCREG(0x68)

#define S3C_RTCRST	      S3C_RTCREG(0x6c)

#define S3C_RTCSEC	      S3C_RTCREG(0x70)
#define S3C_RTCMIN	      S3C_RTCREG(0x74)
#define S3C_RTCHOUR	      S3C_RTCREG(0x78)
#define S3C_RTCDATE	      S3C_RTCREG(0x7c)
#define S3C_RTCDAY	      S3C_RTCREG(0x80)
#define S3C_RTCMON	      S3C_RTCREG(0x84)
#define S3C_RTCYEAR	      S3C_RTCREG(0x88)

#endif /* __ASM_ARCH_REGS_RTC_H */
