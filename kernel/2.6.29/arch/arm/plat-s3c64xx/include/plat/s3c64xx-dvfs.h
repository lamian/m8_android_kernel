/* /arch/arm/plat-s3c64xx/include/plat/s3c64xx-dvfs.h
 *
 * Copyright (c) 2009 Samsung Electronics
  *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __PLAT_S3C64XX_DVFS_H
#define __PLAT_S3C64XX_DVFS_H __FILE__

#define MAXIMUM_FREQ 800000
//#undef USE_DVS
#define USE_DVS
#define USE_DVFS_AL1_LEVEL
//#undef USE_DVFS_AL1_LEVEL
#define KHZ_T		1000

#define MPU_CLK		"clk_cpu"
#define INDX_ERROR  65535

#if defined(CONFIG_MACH_SMDK6410)
#define VCC_ARM		0
#define VCC_INT		1
extern void ltc3714_init(unsigned int, unsigned int);
#endif
extern unsigned int s3c64xx_cpufreq_index;
extern unsigned int S3C64XX_FREQ_TAB;
extern unsigned int S3C64XX_MAXFREQLEVEL;
extern unsigned int s3c64xx_target_frq(unsigned int pred_freq, int flag);
extern void set_dvfs_level(int flag);
extern void set_dvfs_perf_level(void);
#endif /* __PLAT_S3C64XX_DVFS_H */
