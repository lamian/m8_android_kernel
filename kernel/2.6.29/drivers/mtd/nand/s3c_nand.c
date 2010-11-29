/* linux/drivers/mtd/nand/s3c_nand.c
 *
 * Copyright (c) 2007 Samsung Electronics
 *
 * Samsung S3C NAND driver
 *
 * $Id: s3c_nand.c,v 1.3 2008/11/19 10:07:24 jsgood Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Based on nand driver from Ben Dooks <ben@simtec.co.uk>
 * modified by scsuh. based on au1550nd.c
 *
 * Many functions for hardware ecc are implemented by jsgood.
 */

/* Simple H/W Table for Implementation of S3C nand driver
 * by scsuh
 * ------------------------------------------------------------------
 * |    En/Dis CE           |  required  |                          |
 * |    En/Dis ALE          |      X     | * nand controller does   |
 * |    En/Dis CLE          |      X     | * nand controller does   |
 * |    Wait/Ready          |  required  |                          |
 * |    Write Command       |  required  |                          |
 * |    Write Address       |  required  |                          |
 * |    Write Data          |  required  |                          |
 * |    Read Data           |  required  |                          |
 * |    WP on/off           |  required  | * board specific         |
 * |    AP Specific Init    |  required  |                          |
 * ------------------------------------------------------------------
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/jiffies.h>
#include <linux/sched.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>

#include <plat/regs-nand.h>
#include <plat/nand.h>

enum s3c_cpu_type {
	TYPE_S3C2450,	/* including s3c2416 */
	TYPE_S3C6400,
	TYPE_S3C6410,	/* including s3c6430/31 */
};

struct s3c_nand_info {
	/* mtd info */
	struct nand_hw_control		controller;
	struct s3c_nand_mtd_info	*mtds;
	struct s3c2410_platform_nand	*platform;

	/* device info */
	struct device			*device;
	struct resource			*area;
	struct clk			*clk;
	void __iomem			*regs;
	void __iomem			*sel_reg;
	int				sel_bit;
	int				mtd_count;
	unsigned long			save_sel;
	unsigned long			clk_rate;

	enum s3c_cpu_type		cpu_type;
};
static struct s3c_nand_info s3c_nand;

static struct mtd_info *s3c_mtd = NULL;

/* Nand flash definition values by jsgood */
#define S3C_NAND_TYPE_UNKNOWN	0x0
#define S3C_NAND_TYPE_SLC	0x1
#define S3C_NAND_TYPE_MLC	0x2

/*
 * Cached progamming disabled for now, Not sure if its worth the
 * trouble. The speed gain is not very impressive. (2.3->2.6Mib/s)
 *
 * if want to use cached program, define next
 * by jsgood (modified to keep prevent rule)
 */
#undef	CONFIG_MTD_NAND_S3C_CACHEDPROG

/* Nand flash global values by jsgood */
int cur_ecc_mode = 0;
int nand_type = S3C_NAND_TYPE_UNKNOWN;

#if defined(CONFIG_MTD_NAND_S3C_HWECC)
/* Nand flash oob definition for SLC 512b page size by jsgood */
static struct nand_ecclayout s3c_nand_oob_16 = {
	.eccbytes = 4,
	.eccpos = {1, 2, 3, 4},
	.oobfree = {
		{.offset = 6,
		 .length = 10}}
};

/* Nand flash oob definition for SLC 2k page size by jsgood */
static struct nand_ecclayout s3c_nand_oob_64 = {
	.eccbytes = 16,
	.eccpos = {40, 41, 42, 43, 44, 45, 46, 47,
		   48, 49, 50, 51, 52, 53, 54, 55},
	.oobfree = {
		{.offset = 2,
		 .length = 38}}
};

/* Nand flash oob definition for MLC 2k page size by jsgood */
static struct nand_ecclayout s3c_nand_oob_mlc_64 = {
	.eccbytes = 32,
	.eccpos = {
		   32, 33, 34, 35, 36, 37, 38, 39,
		   40, 41, 42, 43, 44, 45, 46, 47,
 		   48, 49, 50, 51, 52, 53, 54, 55,
   		   56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = {
		{.offset = 2,
		 .length = 28}}
};
#endif

#if defined(CONFIG_MTD_NAND_S3C_DEBUG)
/*
 * Function to print out oob buffer for debugging
 * Written by jsgood
 */
void print_oob(const char *header, struct mtd_info *mtd)
{
	int i;
	struct nand_chip *chip = mtd->priv;

	printk("%s:\t", header);

	for(i = 0; i < 64; i++)
		printk("%02x ", chip->oob_poi[i]);

	printk("\n");
}
EXPORT_SYMBOL(print_oob);
#endif


/*
 * Hardware specific access to control-lines function
 * Written by jsgood
 */
static void s3c_nand_hwcontrol(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	unsigned int cur;
	void __iomem *regs = s3c_nand.regs;

	if (ctrl & NAND_CTRL_CHANGE) {
		if (ctrl & NAND_NCE) {
			if (dat != NAND_CMD_NONE) {
				cur = readl(regs + S3C_NFCONT);
				cur &= ~S3C_NFCONT_nFCE0;
				writel(cur, regs + S3C_NFCONT);
			}
		} else {
			cur = readl(regs + S3C_NFCONT);
			cur |= S3C_NFCONT_nFCE0;
			writel(cur, regs + S3C_NFCONT);
		}
	}

	if (dat != NAND_CMD_NONE) {
		if (ctrl & NAND_CLE)
			writeb(dat, regs + S3C_NFCMMD);
		else if (ctrl & NAND_ALE)
			writeb(dat, regs + S3C_NFADDR);
	}
}

/*
 * Function for checking device ready pin
 * Written by jsgood
 */
static int s3c_nand_device_ready(struct mtd_info *mtd)
{
	void __iomem *regs = s3c_nand.regs;
/* it's to check the RnB nand signal bit and return to device ready condition in nand_base.c */
	return ((readl(regs + S3C_NFSTAT) & S3C_NFSTAT_READY));
}

/*
 * We don't use a bad block table
 */
static int s3c_nand_scan_bbt(struct mtd_info *mtdinfo)
{
	return 0;
}

#if defined(CONFIG_MTD_NAND_S3C_HWECC)
#if 0
/*
 * S3C Nand flash chip enable function
 * Written by jsgood
 */
static void s3c_nand_ce_on(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;

	chip->cmd_ctrl(mtd, 0x0, NAND_NCE | NAND_CTRL_CHANGE);
	nand_wait_ready(mtd);
}

/*
 * S3C Nand flash chip disable function
 * Written by jsgood
 */
static void s3c_nand_ce_off(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;

	chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_CTRL_CHANGE);
	nand_wait_ready(mtd);
}
#endif

/*
 * Function for checking ECCEncDone in NFSTAT
 * Written by jsgood
 */
static void s3c_nand_wait_enc(void)
{
        void __iomem *regs = s3c_nand.regs;
        unsigned long timeo = jiffies;
 
        timeo += 16;    /* when Hz=200,  jiffies interval 1/200=5mS, waiting for 80mS  80/5 = 16 */
 
        /* Apply this short delay always to ensure that we do wait tWB in
         * any case on any machine. */

        while (time_before(jiffies, timeo)) {
		if (readl(regs + S3C_NFSTAT) & S3C_NFSTAT_ECCENCDONE)
				break;
		cond_resched();
	}
}

/*
 * Function for checking ECCDecDone in NFSTAT
 * Written by jsgood
 */
static void s3c_nand_wait_dec(void)
{
	void __iomem *regs = s3c_nand.regs;
        unsigned long timeo = jiffies;
 
        timeo += 16;    /* when Hz=200,  jiffies interval  1/200=5mS, waiting for 80mS  80/5 = 16 */
 
        /* Apply this short delay always to ensure that we do wait tWB in
         * any case on any machine. */

        while (time_before(jiffies, timeo)) {
		if (readl(regs + S3C_NFSTAT) & S3C_NFSTAT_ECCDECDONE)
			break;
		cond_resched();
	}
}

/*
 * Function for checking ECC Busy
 * Written by jsgood
 */
static void s3c_nand_wait_ecc_busy(void)
{
	void __iomem *regs = s3c_nand.regs;
        unsigned long timeo = jiffies;
 
        timeo += 16;    /* when Hz=200,  jiffies interval  1/200=5mS, waiting for 80mS  80/5 = 16 */
 
        /* Apply this short delay always to ensure that we do wait tWB in
         * any case on any machine. */

        while (time_before(jiffies, timeo)) {
		if (!(readl(regs + S3C_NFMECCERR0) & S3C_NFECCERR0_ECCBUSY))
			break;
		cond_resched();
	}
}

/*
 * This function is called before encoding ecc codes to ready ecc engine.
 * Written by jsgood
 */
static void s3c_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	u_long nfcont;
	u_long nfconf;
	void __iomem *regs = s3c_nand.regs;

	cur_ecc_mode = mode;

	nfconf = readl(regs + S3C_NFCONF);

	if (s3c_nand.cpu_type == TYPE_S3C6400) {
		if (nand_type == S3C_NAND_TYPE_SLC)
			nfconf &= ~S3C_NFCONF_ECC_MLC;	/* SLC */
		else
			nfconf |= S3C_NFCONF_ECC_MLC;	/* MLC */
	} else {
		nfconf &= ~(0x3 << 23);

		if (nand_type == S3C_NAND_TYPE_SLC)
			nfconf |= S3C_NFCONF_ECC_1BIT;
		else
			nfconf |= S3C_NFCONF_ECC_4BIT;
	}

	writel(nfconf, regs + S3C_NFCONF);

	/* Init main ECC & unlock */
	nfcont = readl(regs + S3C_NFCONT);
	nfcont |= S3C_NFCONT_INITMECC;
	nfcont &= ~S3C_NFCONT_MECCLOCK;

	if (nand_type == S3C_NAND_TYPE_MLC) {
		if (mode == NAND_ECC_WRITE)
			nfcont |= S3C_NFCONT_ECC_ENC;
		else if (mode == NAND_ECC_READ)
			nfcont &= ~S3C_NFCONT_ECC_ENC;
	}

	writel(nfcont, regs + S3C_NFCONT);
}

/*
 * This function is called immediately after encoding ecc codes.
 * This function returns encoded ecc codes.
 * Written by jsgood
 */
static int s3c_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code)
{
	u_long nfcont, nfmecc0, nfmecc1;
	void __iomem *regs = s3c_nand.regs;

	/* Lock */
	nfcont = readl(regs + S3C_NFCONT);
	nfcont |= S3C_NFCONT_MECCLOCK;
	writel(nfcont, regs + S3C_NFCONT);

	if (nand_type == S3C_NAND_TYPE_SLC) {
		nfmecc0 = readl(regs + S3C_NFMECC0);

		ecc_code[0] = nfmecc0 & 0xff;
		ecc_code[1] = (nfmecc0 >> 8) & 0xff;
		ecc_code[2] = (nfmecc0 >> 16) & 0xff;
		ecc_code[3] = (nfmecc0 >> 24) & 0xff;
	} else {
		if (cur_ecc_mode == NAND_ECC_READ)
			s3c_nand_wait_dec();
		else {
			s3c_nand_wait_enc();
			
			nfmecc0 = readl(regs + S3C_NFMECC0);
			nfmecc1 = readl(regs + S3C_NFMECC1);

			ecc_code[0] = nfmecc0 & 0xff;
			ecc_code[1] = (nfmecc0 >> 8) & 0xff;
			ecc_code[2] = (nfmecc0 >> 16) & 0xff;
			ecc_code[3] = (nfmecc0 >> 24) & 0xff;			
			ecc_code[4] = nfmecc1 & 0xff;
			ecc_code[5] = (nfmecc1 >> 8) & 0xff;
			ecc_code[6] = (nfmecc1 >> 16) & 0xff;
			ecc_code[7] = (nfmecc1 >> 24) & 0xff;
		}
	}
	
	return 0;
}

/*
 * This function determines whether read data is good or not.
 * If SLC, must write ecc codes to controller before reading status bit.
 * If MLC, status bit is already set, so only reading is needed.
 * If status bit is good, return 0.
 * If correctable errors occured, do that.
 * If uncorrectable errors occured, return -1.
 * Written by jsgood
 */
static int s3c_nand_correct_data(struct mtd_info *mtd, u_char *dat, u_char *read_ecc, u_char *calc_ecc)
{
	int ret = -1;
	u_long nfestat0, nfestat1, nfmeccdata0, nfmeccdata1, nfmlcbitpt;
	u_char err_type;
	void __iomem *regs = s3c_nand.regs;

	if (!dat) {
		printk("No page data\n");
		return ret;
	}

	if (nand_type == S3C_NAND_TYPE_SLC) {
		/* SLC: Write ECC data to compare */
		nfmeccdata0 = (read_ecc[1] << 16) | read_ecc[0];
		nfmeccdata1 = (read_ecc[3] << 16) | read_ecc[2];
		writel(nfmeccdata0, regs + S3C_NFMECCDATA0);
		writel(nfmeccdata1, regs + S3C_NFMECCDATA1);

		/* Read ECC status */
		nfestat0 = readl(regs + S3C_NFMECCERR0);
		err_type = nfestat0 & 0x3;

		switch (err_type) {
		case 0: /* No error */
			ret = 0;
			break;

		case 1: /* 1 bit error (Correctable)
			   (nfestat0 >> 7) & 0x7ff	:error byte number
			   (nfestat0 >> 4) & 0x7	:error bit number */
			printk("s3c-nand: 1 bit error detected at byte %ld, correcting from "
					"0x%02x ", (nfestat0 >> 7) & 0x7ff, dat[(nfestat0 >> 7) & 0x7ff]);
			dat[(nfestat0 >> 7) & 0x7ff] ^= (1 << ((nfestat0 >> 4) & 0x7));
			printk("to 0x%02x...OK\n", dat[(nfestat0 >> 7) & 0x7ff]);
			ret = 1;
			break;

		case 2: /* Multiple error */
		case 3: /* ECC area error */
			printk("s3c-nand: ECC uncorrectable error detected\n");
			ret = -1;
			break;
		}
	} else {
		/* MLC: */
		s3c_nand_wait_ecc_busy();
		
		nfestat0 = readl(regs + S3C_NFMECCERR0);
		nfestat1 = readl(regs + S3C_NFMECCERR1);
		nfmlcbitpt = readl(regs + S3C_NFMLCBITPT);

		err_type = (nfestat0 >> 26) & 0x7;

		/* No error, If free page (all 0xff) */
		if ((nfestat0 >> 29) & 0x1) {
			err_type = 0;
		} else {
			/* No error, If all 0xff from 17th byte in oob (in case of JFFS2 format) */
			if (dat) {
				if (dat[17] == 0xff && dat[26] == 0xff && dat[35] == 0xff && dat[44] == 0xff && dat[54] == 0xff)
					err_type = 0;
			}
		}

		switch (err_type) {
		case 5: /* Uncorrectable */
			printk("s3c-nand: ECC uncorrectable error detected\n");
			ret = -1;
			break;

		case 4: /* 4 bit error (Correctable) */
			dat[(nfestat1 >> 16) & 0x3ff] ^= ((nfmlcbitpt >> 24) & 0xff);

		case 3: /* 3 bit error (Correctable) */
			dat[nfestat1 & 0x3ff] ^= ((nfmlcbitpt >> 16) & 0xff);

		case 2: /* 2 bit error (Correctable) */
			dat[(nfestat0 >> 16) & 0x3ff] ^= ((nfmlcbitpt >> 8) & 0xff);

		case 1: /* 1 bit error (Correctable) */
			printk("s3c-nand: %d bit(s) error detected, corrected successfully\n", err_type);
			dat[nfestat0 & 0x3ff] ^= (nfmlcbitpt & 0xff);
			ret = err_type;
			break;

		case 0: /* No error */
			ret = 0;
			break;
		}
	}

	return ret;
}

#if 0
static int s3c_nand_write_oob_1bit(struct mtd_info *mtd, struct nand_chip *chip,
			      int page)
{
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	int status = 0;
	int eccbytes = chip->ecc.bytes;
	int secc_start = mtd->oobsize - eccbytes;
	int i;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, mtd->writesize, page);

	/* spare area */
	chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
	chip->write_buf(mtd, chip->oob_poi, secc_start);
	chip->ecc.calculate(mtd, 0, &ecc_calc[chip->ecc.total]);

	for (i = 0; i < eccbytes; i++)
		chip->oob_poi[secc_start + i] = ecc_calc[chip->ecc.total + i];

	chip->write_buf(mtd, chip->oob_poi + secc_start, eccbytes);

	/* Send command to program the OOB data */
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	status = chip->waitfunc(mtd, chip);

	return status & NAND_STATUS_FAIL ? -EIO : 0;
}

static int s3c_nand_read_oob_1bit(struct mtd_info *mtd, struct nand_chip *chip,
			     int page, int sndcmd)
{
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	int eccbytes = chip->ecc.bytes;
	int secc_start = mtd->oobsize - eccbytes;
	
	if (sndcmd) {
		chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);
		sndcmd = 0;
	}

	chip->ecc.hwctl(mtd, NAND_ECC_READ);
	chip->read_buf(mtd, chip->oob_poi, secc_start);
	chip->ecc.calculate(mtd, 0, &ecc_calc[chip->ecc.total]);
	chip->read_buf(mtd, chip->oob_poi + secc_start, eccbytes);

	/* jffs2 special case */
	if (!(chip->oob_poi[2] == 0x85 && chip->oob_poi[3] == 0x19))
		chip->ecc.correct(mtd, chip->oob_poi, chip->oob_poi + secc_start, 0);
	
	return sndcmd;
}

static void s3c_nand_write_page_1bit(struct mtd_info *mtd, struct nand_chip *chip,
				  const uint8_t *buf)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	int secc_start = mtd->oobsize - eccbytes;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	const uint8_t *p = buf;
	
	uint32_t *eccpos = chip->ecc.layout->eccpos;

	/* main area */
	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
		chip->write_buf(mtd, p, eccsize);
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);
	}

	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ecc_calc[i];

	/* spare area */
	chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
	chip->write_buf(mtd, chip->oob_poi, secc_start);
	chip->ecc.calculate(mtd, p, &ecc_calc[chip->ecc.total]);

	for (i = 0; i < eccbytes; i++)
		chip->oob_poi[secc_start + i] = ecc_calc[chip->ecc.total + i];

	chip->write_buf(mtd, chip->oob_poi + secc_start, eccbytes);
}

static int s3c_nand_read_page_1bit(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf)
{
	int i, stat, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	int secc_start = mtd->oobsize - eccbytes;
	int col = 0;
	uint8_t *p = buf;	
	uint32_t *mecc_pos = chip->ecc.layout->eccpos;
	uint8_t *ecc_calc = chip->buffers->ecccalc;

	col = mtd->writesize;
	chip->cmdfunc(mtd, NAND_CMD_RNDOUT, col, -1);

	/* spare area */
	chip->ecc.hwctl(mtd, NAND_ECC_READ);
	chip->read_buf(mtd, chip->oob_poi, secc_start);
	chip->ecc.calculate(mtd, p, &ecc_calc[chip->ecc.total]);
	chip->read_buf(mtd, chip->oob_poi + secc_start, eccbytes);

	/* jffs2 special case */
	if (!(chip->oob_poi[2] == 0x85 && chip->oob_poi[3] == 0x19))
		chip->ecc.correct(mtd, chip->oob_poi, chip->oob_poi + secc_start, 0);

	col = 0;

	/* main area */
	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, col, -1);
		chip->ecc.hwctl(mtd, NAND_ECC_READ);
		chip->read_buf(mtd, p, eccsize);
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);

		stat = chip->ecc.correct(mtd, p, chip->oob_poi + mecc_pos[0] + ((chip->ecc.steps - eccsteps) * eccbytes), 0);
		if (stat == -1)
			mtd->ecc_stats.failed++;

		col = eccsize * (chip->ecc.steps + 1 - eccsteps);
	}
	
	return 0;
}
#endif

/* 
 * Hardware specific page read function for MLC.
 * Written by jsgood
 */
static int s3c_nand_read_page_4bit(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf)
{
	int i, stat, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	int col = 0;
	uint8_t *p = buf;	
	uint32_t *mecc_pos = chip->ecc.layout->eccpos;

	/* Step1: read whole oob */
	col = mtd->writesize;
	chip->cmdfunc(mtd, NAND_CMD_RNDOUT, col, -1);
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	col = 0;
	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, col, -1);
		chip->ecc.hwctl(mtd, NAND_ECC_READ);
		chip->read_buf(mtd, p, eccsize);
		chip->write_buf(mtd, chip->oob_poi + mecc_pos[0] + ((chip->ecc.steps - eccsteps) * eccbytes), eccbytes);
		chip->ecc.calculate(mtd, 0, 0);
		stat = chip->ecc.correct(mtd, p, 0, 0);

		if (stat == -1)
			mtd->ecc_stats.failed++;

		col = eccsize * (chip->ecc.steps + 1 - eccsteps);
	}

	return 0;
}

/* 
 * Hardware specific page write function for MLC.
 * Written by jsgood
 */
static void s3c_nand_write_page_4bit(struct mtd_info *mtd, struct nand_chip *chip,
				  const uint8_t *buf)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	const uint8_t *p = buf;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	uint32_t *mecc_pos = chip->ecc.layout->eccpos;

	/* Step1: write main data and encode mecc */
	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
		chip->write_buf(mtd, p, eccsize);
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);
	}

	/* Step2: save encoded mecc */
	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[mecc_pos[i]] = ecc_calc[i];

	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);
}
#endif

/* s3c_nand_probe
 *
 * called by device layer when it finds a device matching
 * one our driver can handled. This code checks to see if
 * it can allocate all necessary resources then calls the
 * nand layer to look for devices
 */
static int s3c_nand_probe(struct platform_device *pdev, enum s3c_cpu_type cpu_type)
{	
	struct s3c_nand_mtd_info *plat_info = pdev->dev.platform_data;
	struct mtd_partition *partition_info = (struct mtd_partition *)plat_info->partition;
	struct nand_chip *nand;
	struct resource *res;
	int err = 0;
	int ret = 0;
	int i, j, size;

#if defined(CONFIG_MTD_NAND_S3C_HWECC)
	struct nand_flash_dev *type = NULL;
	u_char tmp;
#endif

	/* get the clock source and enable it */

	s3c_nand.clk = clk_get(&pdev->dev, "nand");
	if (IS_ERR(s3c_nand.clk)) {
		dev_err(&pdev->dev, "failed to get clock");
		err = -ENOENT;
		goto exit_error;
	}

	clk_enable(s3c_nand.clk);

	/* allocate and map the resource */

	/* currently we assume we have the one resource */
	res  = pdev->resource;
	size = res->end - res->start + 1;

	s3c_nand.area = request_mem_region(res->start, size, pdev->name);

	if (s3c_nand.area == NULL) {
		dev_err(&pdev->dev, "cannot reserve register region\n");
		err = -ENOENT;
		goto exit_error;
	}

	s3c_nand.cpu_type   = cpu_type;
	s3c_nand.device     = &pdev->dev;
	s3c_nand.regs       = ioremap(res->start, size);

	if (s3c_nand.regs == NULL) {
		dev_err(&pdev->dev, "cannot reserve register region\n");
		err = -EIO;
		goto exit_error;
	}

	/* allocate memory for MTD device structure and private data */
	s3c_mtd = kmalloc(sizeof(struct mtd_info) + sizeof(struct nand_chip), GFP_KERNEL);

	if (!s3c_mtd) {
		printk("Unable to allocate NAND MTD dev structure.\n");
		return -ENOMEM;
	}

	/* Get pointer to private data */
	nand = (struct nand_chip *) (&s3c_mtd[1]);

	/* Initialize structures */
	memset((char *) s3c_mtd, 0, sizeof(struct mtd_info));
	memset((char *) nand, 0, sizeof(struct nand_chip));

	/* Link the private data with the MTD structure */
	s3c_mtd->priv = nand;

	for (i = 0; i < plat_info->chip_nr; i++) {
		nand->IO_ADDR_R		= (char *)(s3c_nand.regs + S3C_NFDATA);
		nand->IO_ADDR_W		= (char *)(s3c_nand.regs + S3C_NFDATA);
		nand->cmd_ctrl		= s3c_nand_hwcontrol;
		nand->dev_ready		= s3c_nand_device_ready;		
		nand->scan_bbt		= s3c_nand_scan_bbt;
		nand->options		= 0;

#if defined(CONFIG_MTD_NAND_S3C_CACHEDPROG)
		nand->options		|= NAND_CACHEPRG;
#endif

#if defined(CONFIG_MTD_NAND_S3C_HWECC)
		nand->ecc.mode		= NAND_ECC_HW;
		nand->ecc.hwctl		= s3c_nand_enable_hwecc;
		nand->ecc.calculate	= s3c_nand_calculate_ecc;
		nand->ecc.correct	= s3c_nand_correct_data;
		
		s3c_nand_hwcontrol(0, NAND_CMD_READID, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		s3c_nand_hwcontrol(0, 0x00, NAND_CTRL_CHANGE | NAND_NCE | NAND_ALE);
		s3c_nand_hwcontrol(0, 0x00, NAND_NCE | NAND_ALE);
		s3c_nand_hwcontrol(0, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
		s3c_nand_device_ready(0);

		tmp = readb(nand->IO_ADDR_R); /* Maf. ID */
		tmp = readb(nand->IO_ADDR_R); /* Device ID */

		for (j = 0; nand_flash_ids[j].name != NULL; j++) {
			if (tmp == nand_flash_ids[j].id) {
				type = &nand_flash_ids[j];
				break;
			}
		}

		if (!type) {
			printk("Unknown NAND Device.\n");
			goto exit_error;
		}
		
		nand->cellinfo = readb(nand->IO_ADDR_R);	/* the 3rd byte */
		tmp = readb(nand->IO_ADDR_R);			/* the 4th byte */

		if (!type->pagesize) {
			if (((nand->cellinfo >> 2) & 0x3) == 0) {
				nand_type = S3C_NAND_TYPE_SLC;				
				nand->ecc.size = 512;
				nand->ecc.bytes	= 4;

				if ((1024 << (tmp & 0x3)) > 512) {
#if 0 //choi
					nand->ecc.read_page = s3c_nand_read_page_1bit;
					nand->ecc.write_page = s3c_nand_write_page_1bit;
					nand->ecc.read_oob = s3c_nand_read_oob_1bit;
					nand->ecc.write_oob = s3c_nand_write_oob_1bit;
					nand->ecc.layout = &s3c_nand_oob_64;
#endif
					nand->ecc.size = 2048;
					nand->ecc.bytes = 4;
					nand->ecc.layout = &s3c_nand_oob_64;
				} else {
				  //choi
					nand->ecc.size = 512;
					nand->ecc.bytes = 3;
					nand->ecc.layout = &s3c_nand_oob_16;
				}
			} else {
				nand_type = S3C_NAND_TYPE_MLC;
				nand->options |= NAND_NO_SUBPAGE_WRITE;	/* NOP = 1 if MLC */
#if 1 //choi
				nand->ecc.read_page = s3c_nand_read_page_4bit;
				nand->ecc.write_page = s3c_nand_write_page_4bit;
#endif
//				nand->ecc.read_page = s3c_nand_read_page;
//				nand->ecc.write_page = s3c_nand_write_page;
				nand->ecc.size = 512;
				nand->ecc.bytes = 8;	/* really 7 bytes */
				nand->ecc.layout = &s3c_nand_oob_mlc_64;
			}
		} else {
			nand_type = S3C_NAND_TYPE_SLC;
			nand->ecc.size = 512;
			nand->cellinfo = 0;
			nand->ecc.bytes = 4;
			nand->ecc.layout = &s3c_nand_oob_16;
		}

		printk("S3C NAND Driver is using hardware ECC.\n");
#else
		nand->ecc.mode = NAND_ECC_SOFT;
		printk("S3C NAND Driver is using software ECC.\n");
#endif
		if (nand_scan(s3c_mtd, 1)) {
			ret = -ENXIO;
			goto exit_error;
		}

		/* Register the partitions */
		add_mtd_partitions(s3c_mtd, partition_info, plat_info->mtd_part_nr);
	}

	pr_debug("initialized ok\n");
	return 0;

exit_error:
	kfree(s3c_mtd);

	return ret;
}

static int s3c2450_nand_probe(struct platform_device *dev)
{
	return s3c_nand_probe(dev, TYPE_S3C2450);
}

static int s3c6400_nand_probe(struct platform_device *dev)
{
	return s3c_nand_probe(dev, TYPE_S3C6400);
}

static int s3c6410_nand_probe(struct platform_device *dev)
{
	return s3c_nand_probe(dev, TYPE_S3C6410);
}

/* PM Support */
#if defined(CONFIG_PM)
#include <plat/pm.h>

static struct sleep_save nand_save[17]; 

static int s3c_nand_suspend(struct platform_device *dev, pm_message_t pm)
{
	int i;

	void __iomem *regs = s3c_nand.regs;

	for(i = 0; i < 17; i++)
	{
		nand_save[i].reg = regs + S3C2410_NFREG(i*4);
	}

	s3c6410_pm_do_save(nand_save, ARRAY_SIZE(nand_save));

	return 0;
}

static int s3c_nand_resume(struct platform_device *dev)
{
	s3c6410_pm_do_restore(nand_save, ARRAY_SIZE(nand_save));
	
	return 0;
}

#else
#define s3c_nand_suspend NULL
#define s3c_nand_resume NULL
#endif

/* device management functions */
static int s3c_nand_remove(struct platform_device *dev)
{
	platform_set_drvdata(dev, NULL);

	return 0;
}

static struct platform_driver s3c2450_nand_driver = {
	.probe		= s3c2450_nand_probe,
	.remove		= s3c_nand_remove,
	.suspend	= s3c_nand_suspend,
	.resume		= s3c_nand_resume,
	.driver		= {
		.name	= "s3c2450-nand",
		.owner	= THIS_MODULE,
	},
};

static struct platform_driver s3c6400_nand_driver = {
	.probe		= s3c6400_nand_probe,
	.remove		= s3c_nand_remove,
	.suspend	= s3c_nand_suspend,
	.resume		= s3c_nand_resume,
	.driver		= {
		.name	= "s3c6400-nand",
		.owner	= THIS_MODULE,
	},
};

static struct platform_driver s3c6410_nand_driver = {
	.probe		= s3c6410_nand_probe,
	.remove		= s3c_nand_remove,
	.suspend	= s3c_nand_suspend,
	.resume		= s3c_nand_resume,
	.driver		= {
		.name	= "s3c6410-nand",
		.owner	= THIS_MODULE,
	},
};

static int __init s3c_nand_init(void)
{
	printk("S3C NAND Driver, (c) 2008 Samsung Electronics\n");

	platform_driver_register(&s3c2450_nand_driver);
	platform_driver_register(&s3c6400_nand_driver);
	return platform_driver_register(&s3c6410_nand_driver);
}

static void __exit s3c_nand_exit(void)
{
	platform_driver_unregister(&s3c2450_nand_driver);
	platform_driver_unregister(&s3c6400_nand_driver);
	platform_driver_unregister(&s3c6410_nand_driver);
}

module_init(s3c_nand_init);
module_exit(s3c_nand_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jinsung Yang <jsgood.yang@samsung.com>");
MODULE_DESCRIPTION("S3C MTD NAND driver");

