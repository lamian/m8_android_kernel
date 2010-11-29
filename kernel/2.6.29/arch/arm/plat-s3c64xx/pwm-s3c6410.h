/* arch/arm/plat-s3c64xx/pwm-s3c6410.h
 *
 * Copyright (C) 2003,2004 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * Samsung S3C PWM support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Changelog:
 *  ??-May-2003 BJD   Created file
 *  ??-Jun-2003 BJD   Added more dma functionality to go with arch
 *  10-Nov-2004 BJD   Added sys_device support
*/

#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H __FILE__

#include <linux/sysdev.h>
#include <plat/regs-timer.h>

#define pwmch_t int

/* we have 4 pwm channels */
#define S3C_PWM_CHANNELS        4


struct s3c_pwm_client {
	char                *name;
};

typedef struct s3c_pwm_client s3c_pwm_client_t;


typedef struct s3c_pwm_chan_s s3c6410_pwm_chan_t;

/* s3c_pwm_cbfn_t
 *
 * buffer callback routine type
*/

typedef void (*s3c_pwm_cbfn_t)(void *buf);



/* struct s3c_pwm_chan_s
 *
 * full state information for each DMA channel
*/

struct s3c_pwm_chan_s {
	/* channel state flags and information */
	unsigned char          number;      /* number of this dma channel */
	unsigned char          in_use;      /* channel allocated */
	unsigned char          irq_claimed; /* irq claimed for channel */
	unsigned char          irq_enabled; /* irq enabled for channel */

	/* channel state */

	s3c_pwm_client_t  *client;
	void 	*dev;
	/* channel configuration */
	unsigned int           flags;        /* channel flags */

	/* channel's hardware position and configuration */
	unsigned int           irq;          /* channel irq */

	/* driver handles */
	s3c_pwm_cbfn_t     callback_fn;  /* buffer done callback */

	/* system device */
	struct sys_device	sysdev;
};

/* the currently allocated channel information */
extern s3c6410_pwm_chan_t s3c_chans[];


/* functions --------------------------------------------------------------- */

/* s3c6410_pwm_request
 *
 * request a pwm channel exclusivley
*/

extern int s3c6410_pwm_request(pwmch_t channel, s3c_pwm_client_t *, void *dev);


/* s3c_dma_ctrl
 *
 * change the state of the dma channel
*/

//extern int s3c_dma_ctrl(dmach_t channel, s3c_chan_op_t op);

/* s3c_dma_setflags
 *
 * set the channel's flags to a given state
*/


/* s3c6410_pwm_free
 *
 * free the dma channel (will also abort any outstanding operations)
*/

extern int s3c6410_pwm_free(pwmch_t channel, s3c_pwm_client_t *);


/* s3c_dma_config
 *
 * configure the dma channel
*/




extern int s3c6410_pwm_set_buffdone_fn(pwmch_t, s3c_pwm_cbfn_t rtn);

extern int s3c6410_timer_setup (int channel, int usec, unsigned long g_tcnt, unsigned long g_tcmp);

#endif /* __ASM_ARCH_DMA_H */
