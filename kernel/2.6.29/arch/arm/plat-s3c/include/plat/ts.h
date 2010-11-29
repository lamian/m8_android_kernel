/* linux/arch/arm/plat-s3c/include/plat/ts.h
 *
 * Copyright (c) 2004 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_TS_H
#define __ASM_ARCH_TS_H __FILE__


enum s3c_adc_type {
	ADC_TYPE_0,
	ADC_TYPE_1,	/* S3C2416, S3C2450 */
	ADC_TYPE_2,	/* S3C64XX, S5PC1XX */
};

struct s3c_ts_mach_info {
	int             	delay;
	int             	presc;
	int             	oversampling_shift;
	int			resol_bit;
	enum s3c_adc_type	s3c_adc_con;
};

struct s3c_ts_info {
	struct input_dev 	*dev;
	long 			xp;
	long 			yp;
#if 1 // Andriod
	long xp_old;
	long yp_old;
#endif
	int 			count;
	int 			shift;
	char 			phys[32];
	int			resol_bit;
	enum s3c_adc_type	s3c_adc_con;
};

extern void __init s3c_ts_set_platdata(struct s3c_ts_mach_info *pd);

#endif /* __ASM_ARCH_TS_H */
