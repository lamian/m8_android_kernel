#ifndef __ASM_S3C_HSMMC_H
#define __ASM_S3C_HSMMC_H

#ifndef __ASSEMBLY__

#include <linux/mmc/card.h>
#include <linux/mmc/host.h>

#define NUM_OF_HSMMC_CLKSOURCES 	1

#define	SPEED_NORMAL		0
#define	SPEED_HIGH		1

struct s3c_hsmmc_fd_cfg {
	ulong ctrl2;
	ulong ctrl3[2];		/* 0: low speed, 1: high speed */
	ulong ctrl4;
};

struct s3c_hsmmc_clk_cfg {
	char *name;
	u32 src;
};

struct s3c_hsmmc_cfg {
	u32 hwport;		/* hardware port number */
	u32 enabled;		/* if port is used, set 1 */
	u32 host_caps;		/* host capabilities */
	u32 bus_width;		/* bus width */

	void *base;		/* base address of host */
	
	u8 highspeed;		/* ENHIGHSPD bit configuration */
		
	/* feedback delay control configuration (0: mmc, 1: sd) */
	struct s3c_hsmmc_fd_cfg fd_ctrl[2];

	/* clock source control */	
	struct s3c_hsmmc_clk_cfg clocks[NUM_OF_HSMMC_CLKSOURCES];
};

extern void hsmmc_set_gpio(uint channel, uint width);

#endif /* __ASSEMBLY__ */

#endif /* __ASM_S3C_HSMMC_H */

