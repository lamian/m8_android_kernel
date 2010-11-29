/*
 *  lis3lv02d.h - ST LIS3LV02DL accelerometer driver
 * 
 * The actual chip is STMicroelectronics LIS3LV02DL or LIS3LV02DQ that seems to
 * be connected via SPI. There exists also several similar chips (such as LIS302DL or
 * LIS3L02DQ) but not in the HP laptops and they have slightly different registers.
 * They can also be connected via I2C.
 */
//

#ifndef _LIS33DE_H
#define _LIS33DE_H

//#define LIS3LV02DL_ID	0x3A /* Also the LIS3LV02DQ */
//#define LIS302DL_ID		0x3B /* Also the LIS202DL! */

enum lis33de_reg {
	CTRL_REG1		= 0x20,
	CTRL_REG2		= 0x21,
	CTRL_REG3		= 0x22,
	STATUS_REG		= 0x27,
	OUT_X			= 0x29,
	OUT_Y			= 0x2B,
	OUT_Z			= 0x2D,
	FF_WU_CFG		= 0x30,
	FF_WU_SRC		= 0x31,
	FF_WU_THS		= 0x32,
	FF_WU_DURATION = 0x33,
};

enum lis33de_ctrl1 {
	CTRL1_Xen	= 0x01,
	CTRL1_Yen	= 0x02,
	CTRL1_Zen	= 0x04,
	CTRL1_STM	= 0x08,
	CTRL1_STP	= 0x10,
	CTRL1_FS	= 0x20,
	CTRL1_PD	= 0x40,
	CTRL1_DR	= 0x80,
};
enum lis33de_ctrl2 {
	CTRL2_SIM	= 0x40,
	CTRL2_BOOT	= 0x80, 
};


enum lis33de_ctrl3 {
	CTRL3_ICFG0 	= 0x01,
	CTRL3_ICFG1	= 0x02,
	CTRL3_ICFG2	= 0x04,
	CTRL3_IHL		= 0x80,
};

/*
enum lis33de_status_reg {
	STATUS_XDA	= 0x01,
	STATUS_YDA	= 0x02,
	STATUS_ZDA	= 0x04,
	STATUS_XYZDA	= 0x08,
	STATUS_XOR	= 0x10,
	STATUS_YOR	= 0x20,
	STATUS_ZOR	= 0x40,
	STATUS_XYZOR	= 0x80,
};

enum lis33de_ff_wu_cfg {
	FF_WU_CFG_XLIE	= 0x01,
	FF_WU_CFG_XHIE	= 0x02,
	FF_WU_CFG_YLIE	= 0x04,
	FF_WU_CFG_YHIE	= 0x08,
	FF_WU_CFG_ZLIE	= 0x10,
	FF_WU_CFG_ZHIE	= 0x20,
	FF_WU_CFG_LIR	= 0x40,
	FF_WU_CFG_AOI	= 0x80,
};

enum lis33de_ff_wu_src {
	FF_WU_SRC_XL	= 0x01,
	FF_WU_SRC_XH	= 0x02,
	FF_WU_SRC_YL	= 0x04,
	FF_WU_SRC_YH	= 0x08,
	FF_WU_SRC_ZL	= 0x10,
	FF_WU_SRC_ZH	= 0x20,
	FF_WU_SRC_IA	= 0x40,
};
*/

#endif
