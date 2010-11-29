/* linux/arch/arm/mach-s3c6410/mach-smdk6410.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/i2c.h>
#ifdef CONFIG_I2C_GPIO
#include <linux/i2c-gpio.h>
#endif
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/spi/spi.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

#include <mach/hardware.h>
#include <mach/map.h>
#include <mach/regs-mem.h>
#include <mach/gpio-core.h>

#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>
#include <plat/iic.h>

#include <plat/regs-rtc.h>
#include <plat/regs-clock.h>
#include <plat/regs-gpio.h>

#include <plat/nand.h>
#include <plat/partition.h>
#include <plat/s3c6410.h>
#include <plat/clock.h>
#include <plat/regs-clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/ts.h>
#include <plat/adc.h>
#include <plat/reserved_mem.h>
#include <plat/pm.h>
#include <plat/pll.h>
#include <plat/spi.h>

#include <linux/android_pmem.h>
#include <linux/proc_fs.h>

#ifdef CONFIG_USB_SUPPORT
#include <plat/regs-otg.h>
#include <linux/usb/ch9.h>
#include <mach/gpio.h>/*add by hui 2009-08-24*/
#include <plat/gpio-cfg.h>/*add by hui 2009-08-24*/
#include <plat/regs-gpio.h>/*add by hui 2009-08-24*/
#include <plat/regs-spi.h>/*add by lamian 22:29 2010/7/3*/
#include <linux/spi/libertas_spi.h>
#include <linux/switch.h>
#include "../../../drivers/staging/android/timed_gpio.h"

/* S3C_USB_CLKSRC 0: EPLL 1: CLK_48M */
#define S3C_USB_CLKSRC	1

#ifdef USB_HOST_PORT2_EN
#define OTGH_PHY_CLK_VALUE      (0x60)  /* Serial Interface, otg_phy input clk 48Mhz Oscillator */
#else
#define OTGH_PHY_CLK_VALUE      (0x20)  /* UTMI Interface, otg_phy input clk 48Mhz Oscillator */
#endif
#endif

#define UCON S3C_UCON_DEFAULT | S3C_UCON_UCLK
#define ULCON S3C_LCON_CS8 | S3C_LCON_PNONE | S3C_LCON_STOPB
#define UFCON S3C_UFCON_RXTRIG8 | S3C_UFCON_FIFOMODE

#ifndef CONFIG_HIGH_RES_TIMERS
extern struct sys_timer s3c_timer;
#else
extern struct sys_timer sec_timer;
#endif /* CONFIG_HIGH_RES_TIMERS */

extern void s3c6410_cpu_resume(void);
extern void s3c64xx_reserve_bootmem(void);

static struct libertas_spi_platform_data libertas_data __initdata = {
	.use_dummy_writes = 0,
	.setup = NULL,
	.teardown = NULL,
//	.gpio_cs = S3C64XX_GPC(3),
};

static struct spi_board_info s3c_spi_devs[] __initdata = {
	{
		.modalias		= "libertas_spi", /* Test Interface */
		.mode			= SPI_MODE_0,	/* CPOL=0, CPHA=0 */
		.max_speed_hz	= 10000000,
		/* Connected to SPI-0 as 1st Slave */
		.bus_num		= 0,
		.irq			= IRQ_EINT(19),
		.chip_select	= 0,
		.platform_data	= &libertas_data,
	},
};


static void cs_config(int pin, int mode, int pull)
{
//	printk("Initialize GPC3\n");
	s3c_gpio_cfgpin(pin, mode);
	s3c_gpio_setpull(pin, S3C_GPIO_PULL_UP);
	gpio_set_value(pin, pull);
}

static void cs_set(int pin, int lvl)
{
	gpio_set_value(pin, lvl);
//	printk("Set GPC3=%d\n", lvl);
}

static void cs_suspend(int pin, pm_message_t pm)
{
	return;
}

static void cs_resume(int pin)
{
	return;
}

static struct s3c_spi_pdata libertas_cfg __initdata = {
	.cs_level			= CS_FLOAT,
	.cs_pin				= S3C64XX_GPC(3),
	.cs_mode			= S3C64XX_GPC3_SPI_nCS0,
	.cs_config			= cs_config,
	.cs_set				= cs_set,
	.cs_suspend			= cs_suspend,
	.cs_resume			= cs_resume,
};

static struct s3c_uartcfg smdk6410_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon			= S3C_UCON_DEFAULT,
		.ulcon		= S3C_ULCON_DEFAULT,
		.ufcon		= S3C_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon			= S3C_UCON_DEFAULT,
		.ulcon		= S3C_ULCON_DEFAULT,
		.ufcon		= S3C_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon			= S3C_UCON_DEFAULT,
		.ulcon		= S3C_ULCON_DEFAULT,
		.ufcon		= S3C_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon			= S3C_UCON_DEFAULT,
		.ulcon		= S3C_ULCON_DEFAULT,
		.ufcon		= S3C_UFCON_DEFAULT,
	},
};

struct map_desc smdk6410_iodesc[] = {};

struct platform_device sec_device_backlight = {
	.name	= "smdk-backlight",
	.id		= -1,
};

struct platform_device sec_device_battery = {
	.name	= "smdk6410-battery",
	.id		= -1,
};

static struct s3c6410_pmem_setting pmem_setting = {
 	.pmem_start = RESERVED_PMEM_START,
	.pmem_size = RESERVED_PMEM,
	.pmem_gpu1_start = /*GPU1_RESERVED_PMEM_START*/G3D_RESERVED_START,
	.pmem_gpu1_size = /*RESERVED_PMEM_GPU1*/RESERVED_G3D,
	.pmem_render_start = RENDER_RESERVED_PMEM_START,
	.pmem_render_size = RESERVED_PMEM_RENDER,
	.pmem_stream_start = STREAM_RESERVED_PMEM_START,
	.pmem_stream_size = RESERVED_PMEM_STREAM,
	.pmem_preview_start = PREVIEW_RESERVED_PMEM_START,
	.pmem_preview_size = RESERVED_PMEM_PREVIEW,
	.pmem_picture_start = PICTURE_RESERVED_PMEM_START,
	.pmem_picture_size = RESERVED_PMEM_PICTURE,
	.pmem_jpeg_start = JPEG_RESERVED_PMEM_START,
	.pmem_jpeg_size = RESERVED_PMEM_JPEG,
        .pmem_skia_start = SKIA_RESERVED_PMEM_START,
        .pmem_skia_size = RESERVED_PMEM_SKIA,
};


#if defined(CONFIG_SND_S3C_I2S_V32)//add by hui
static  int __init i2s_pin_init(void)
{
	/* Configure the I2S1 pins in correct mode */
	s3c_gpio_cfgpin(S3C64XX_GPE(0),S3C64XX_GPE0_I2S1_CLK);
	s3c_gpio_cfgpin(S3C64XX_GPE(1),S3C64XX_GPE1_I2S1_CDCLK);
	s3c_gpio_cfgpin(S3C64XX_GPE(2),S3C64XX_GPE2_I2S1_LRCLK);

	s3c_gpio_cfgpin(S3C64XX_GPE(3),S3C64XX_GPE3_I2S1_DI);
	s3c_gpio_cfgpin(S3C64XX_GPE(4),S3C64XX_GPE4_I2S1_DO);

	/* pull-up-enable, pull-down-disable*/
	s3c_gpio_setpull(S3C64XX_GPE(0), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(S3C64XX_GPE(1), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(S3C64XX_GPE(2), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(S3C64XX_GPE(3), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(S3C64XX_GPE(4), S3C_GPIO_PULL_UP);
}
#endif

static struct gpio_switch_platform_data m8_switch_data = {
	.name = "h2w",
	.gpio = S3C64XX_GPN(12),
};

static struct platform_device m8_switch_device = {
	.name	= "switch-gpio",
	.dev	= {
		.platform_data = &m8_switch_data,
	},
};

static struct timed_gpio vibrator = {
	.name = "vibrator",
	.gpio = S3C64XX_GPD(1),
	.max_timeout = 10000,
	.active_low = 0,
};

static struct timed_gpio_platform_data m8_vibrator_data = {
	.num_gpios = 1,
	.gpios = &vibrator,
};

static struct platform_device m8_vibrator = {
	.name = TIMED_GPIO_NAME,
	.dev	= {
		.platform_data = &m8_vibrator_data,
	},
};

static struct platform_device *smdk6410_devices[] __initdata = {
	&s3c_device_hsmmc1,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_spi0,
//	&s3c_device_spi1,
//	&s3c_device_ts,//marked by hui
//	&s3c_device_smc911x,
	&s3c_device_lcd,
//	&s3c_device_nand,	//marked by hui
	&s3c_device_onenand,	/*add by hui 2009-08-21*/
	&s3c_device_keypad,
//	&s3c_device_usb,
	&meizu_m8_buttons,		/*modified by meizu*/
	&s3c_device_usbgadget,
#ifdef CONFIG_S3C_ADC
	&s3c_device_adc,
#endif
#ifdef CONFIG_RTC_DRV_S3C
	&s3c_device_rtc,
#endif
#ifdef CONFIG_VIDEO_G2D
	&s3c_device_g2d,
#endif
#ifdef CONFIG_VIDEO_FIMC
	&s3c_device_fimc0,
	&s3c_device_fimc1,
#endif
#if CONFIG_VIDEO_CAM 
	&s3c_device_camif,
#endif
#ifdef CONFIG_VIDEO_MFC
	&s3c_device_mfc,
#endif
#ifdef CONFIG_VIDEO_G3D
	&s3c_device_g3d,
#endif
#ifdef CONFIG_VIDEO_ROTATOR
	&s3c_device_rotator,
#endif
#ifdef CONFIG_VIDEO_JPEG
	&s3c_device_jpeg,
#endif
	&s3c_device_vpp,
#ifdef CONFIG_S3C6410_TVOUT
	&s3c_device_tvenc,
	&s3c_device_tvscaler,
#endif
	&sec_device_backlight,
	&sec_device_battery,
	&m8_switch_device,
	&m8_vibrator,
};

static struct i2c_board_info i2c_devs0_se[] __initdata = {
	{ I2C_BOARD_INFO("24c08", 0x50), },
	{ I2C_BOARD_INFO("ltc3555",0x09)},	/*M8 LTC3555 PMIC*/
/*	{ I2C_BOARD_INFO("WM8580", 0x1b), },	*/
	{ I2C_BOARD_INFO("st-lis302dl", 0x1d)}, 	/*支持lis302dl_misc sensor*/
	{ I2C_BOARD_INFO("wm8993", 0x1a), },
};

static struct i2c_board_info i2c_devs0_fe[] __initdata = {
	{ I2C_BOARD_INFO("24c08", 0x50), },
	{ I2C_BOARD_INFO("ltc3555",0x09)},	/*M8 LTC3555 PMIC*/
/*	{ I2C_BOARD_INFO("WM8580", 0x1b), },	*/
	{ I2C_BOARD_INFO("st-lis302dl", 0x1d)}, 	/*支持lis302dl_misc sensor*/
	{ I2C_BOARD_INFO("wm8753", 0x1a), },
};

static struct i2c_board_info i2c_devs1[] __initdata = {
	{ I2C_BOARD_INFO("synaptics-rmi-ts", 0x20), },	/*meizu m8 touch pannel controller*/
	//{ I2C_BOARD_INFO("24c128", 0x57), },	/* Samsung S524AD0XD1 */
	//{ I2C_BOARD_INFO("WM8580", 0x1b), },
};

static struct s3c_ts_mach_info s3c_ts_platform __initdata = {
	.delay 			= 10000,
	.presc 			= 49,
	.oversampling_shift	= 2,
	.resol_bit 		= 12,
	.s3c_adc_con		= ADC_TYPE_2,
};

static struct s3c_adc_mach_info s3c_adc_platform __initdata= {
	/* Support 12-bit resolution */
	.delay	= 	0xff,
	.presc 	= 	5,
	.resolution = 	12,
};

// Power up WIFI Module
void m8_gsm_power(int on) {
// only power on is supported, for now.
// PWBB: GPD4, RESET_BB:GPM4
	s3c_gpio_cfgpin(S3C64XX_GPD(4),S3C_GPIO_OUTPUT);
	s3c_gpio_cfgpin(S3C64XX_GPM(4),S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S3C64XX_GPD(4), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C64XX_GPM(4), S3C_GPIO_PULL_NONE);

	gpio_set_value(S3C64XX_GPD(4), 1);
	msleep(1500);
	gpio_set_value(S3C64XX_GPM(4), 1);
	msleep(500);
	gpio_set_value(S3C64XX_GPM(4), 0);
	msleep(500);
	gpio_set_value(S3C64XX_GPM(4), 1);	
}

int wifi_status = 0;
void m8_wifi_power(int on) {
	int err;

	int is_m8se = m8_checkse();

	wifi_status = 0;
	
	s3c_gpio_cfgpin(S3C64XX_GPD(0),S3C_GPIO_OUTPUT);
	s3c_gpio_cfgpin(S3C64XX_GPN(6),S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S3C64XX_GPD(0), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C64XX_GPN(6), S3C_GPIO_PULL_NONE);
	gpio_set_value(S3C64XX_GPD(0), 0);
	gpio_set_value(S3C64XX_GPN(6), 0);

	if (is_m8se) {
		s3c_gpio_setpull(S3C64XX_GPL(7), S3C_GPIO_OUTPUT);
		s3c_gpio_setpull(S3C64XX_GPL(7), S3C_GPIO_PULL_NONE);
		gpio_set_value(S3C64XX_GPL(7), 0);
	}

	if (on) {
		wifi_status = 1;
		msleep(10);
		gpio_set_value(S3C64XX_GPD(0), 1);
		gpio_set_value(S3C64XX_GPN(6), 1);
		if (is_m8se) {
			gpio_set_value(S3C64XX_GPL(7), 1);
		}
	}
	
}
EXPORT_SYMBOL(m8_wifi_power);

/*
*拉高GPK14,给usb上电
*/
static void __init smdk6410_usb_power(int value){

	unsigned int gpio = S3C64XX_GPK(14);
/*	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_DOWN);
	s3c_gpio_setpin(gpio, value&0x01);
	s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
	s3c_gpio_setpin(gpio, value&0x01);
	*/

//writel(0x20, S3C_USBOTG_PHYCLK);	/*commented by hui 2009-08-24*/
	writel(0x00, S3C_USBOTG_PHYCLK);		/*00:USB使用外部晶振。modified by hui 2009-08-24*/


#if 1	//打开usb GPK14 GPIO,给usb上电
	gpio_direction_output(gpio, value & 0x01);
#else
	int err;

	if (gpio_is_valid(S3C64XX_GPK(14))) {
		err = gpio_request(S3C64XX_GPK(14), "GPK");

		if (err) {
			printk(KERN_ERR "failed to request GPK for "
				"USB power control\n");
		}
		gpio_direction_output(S3C64XX_GPK(14), value);
		mdelay(100);
		gpio_set_value(S3C64XX_GPK(14), value);
		/* GPIO G : Chip detect + LED */
		//s3c_gpio_cfgpin(S3C64XX_GPK(14), S3C_GPIO_SFN(2));
		//s3c_gpio_setpull(S3C64XX_GPK(14), S3C_GPIO_PULL_UP);
		gpio_free(S3C64XX_GPK(14));
	printk("\nhui test: USB power on...[%s %d %s]:\n ",__FILE__,__LINE__,__FUNCTION__);
	}
#if 0
	while(1)//test only
		{
		}
#endif

#endif

#if 1
	unsigned long flags;
	unsigned long dat;
	unsigned long con;
	void __iomem *base = S3C64XX_GPK_BASE + 0x04;
	unsigned long 	offset=14;

#if 1	
	//base = S3C64XX_GPK_BASE + 0x04;
	offset=14;
	value=1;
	local_irq_save(flags);

	dat = __raw_readl(base + 0x04);
	dat &= ~(1 << offset);
	if (value)
		dat |= 1 << offset;
	__raw_writel(dat, base + 0x04);

	con = __raw_readl(base + 0x00);
	con &= ~(0x0f000000);
	con |= 0x01000000;

	__raw_writel(con, base + 0x00);
	__raw_writel(dat, base + 0x04);


	local_irq_restore(flags);
#endif	
	//测试GPK(14)是否已经打开
	unsigned long conval = __raw_readl(base + 0x00);
	unsigned long datval = __raw_readl(base + 0x04);
	//val >>= 14;
	//val &= 1;
	printk("\nhui test: S3C64XX_GPIO con==0x%x,dat==0x%x\n[%s %d %s]:\n\n ",conval,datval,__FILE__,__LINE__,__FUNCTION__);
#endif	

}

static void __init smdk6410_map_io(void)
{
	s3c_device_nand.name = "s3c6410-nand";

	s3c64xx_init_io(smdk6410_iodesc, ARRAY_SIZE(smdk6410_iodesc));
	s3c64xx_gpiolib_init();
	s3c_init_clocks(XTAL_FREQ);
	s3c_init_uarts(smdk6410_uartcfgs, ARRAY_SIZE(smdk6410_uartcfgs));
}

static void __init smdk6410_smc911x_set(void)
{
	unsigned int tmp;

	tmp = __raw_readl(S3C64XX_SROM_BW);
	tmp &= ~(S3C64XX_SROM_BW_WAIT_ENABLE1_MASK | S3C64XX_SROM_BW_WAIT_ENABLE1_MASK |
		S3C64XX_SROM_BW_DATA_WIDTH1_MASK);
	tmp |= S3C64XX_SROM_BW_BYTE_ENABLE1_ENABLE | S3C64XX_SROM_BW_WAIT_ENABLE1_ENABLE |
		S3C64XX_SROM_BW_DATA_WIDTH1_16BIT;

	__raw_writel(tmp, S3C64XX_SROM_BW);

	__raw_writel(S3C64XX_SROM_BCn_TACS(0) | S3C64XX_SROM_BCn_TCOS(4) |
			S3C64XX_SROM_BCn_TACC(13) | S3C64XX_SROM_BCn_TCOH(1) |
			S3C64XX_SROM_BCn_TCAH(4) | S3C64XX_SROM_BCn_TACP(6) |
			S3C64XX_SROM_BCn_PMC_NORMAL, S3C64XX_SROM_BC1);
}

static void __init smdk6410_fixup (struct machine_desc *desc, struct tag *tags,
	      char **cmdline, struct meminfo *mi)
{
	/*
	 * Bank start addresses are not present in the information
	 * passed in from the boot loader.  We could potentially
	 * detect them, but instead we hard-code them.
	 */
	mi->nr_banks = 1;
	mi->bank[0].start = PHYS_OFFSET;
	mi->bank[0].size = PHYS_UNRESERVED_SIZE;
	mi->bank[0].node = 0;
}

static void smdk6410_set_qos(void)
{
	u32 reg;

	/* AXI sfr */
	reg = (u32) ioremap((unsigned long) 0x7e003000, SZ_4K);

	/* QoS override: FIMD min. latency */
	writel(0x2, S3C_VA_SYS + 0x128);

	/* AXI QoS */
	writel(0x7, reg + 0x460);	/* (8 - MFC ch.) */
	writel(0x7ff7, reg + 0x464);

	/* Bus cacheable */
	writel(0x8ff, S3C_VA_SYS + 0x838);
}

static void __init smdk6410_machine_init(void)
{
	s3c_device_nand.dev.platform_data = &s3c_nand_mtd_part_info;
	s3c_device_onenand.dev.platform_data = &s3c_nand_mtd_part_info;

	smdk6410_smc911x_set();

	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);

//	s3c_ts_set_platdata(&s3c_ts_platform);	/*commented by hui 2009-08-27*/
	s3c_adc_set_platdata(&s3c_adc_platform);

	if (m8_checkse()) {
		i2c_register_board_info(0, i2c_devs0_se, ARRAY_SIZE(i2c_devs0_se));
	} else {
		i2c_register_board_info(0, i2c_devs0_fe, ARRAY_SIZE(i2c_devs0_fe));
	}
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));

	s3c_spi_set_slaves(0, 1, &libertas_cfg);
	spi_register_board_info(s3c_spi_devs, ARRAY_SIZE(s3c_spi_devs));

	platform_add_devices(smdk6410_devices, ARRAY_SIZE(smdk6410_devices));

	if (m8_checkse()) {
		platform_device_register(&s3c_device_hsmmc0);
	}

	s3c6410_add_mem_devices (&pmem_setting);

	s3c6410_pm_init();

	smdk6410_set_qos();

	/*
	*拉高GPK14,给usb上电
	*拉高GPD4,给baseband 上电
	*/
	smdk6410_usb_power(1);
	//m8_gsm_power(1);
	
#if defined(CONFIG_SND_S3C_I2S_V32)
	i2s_pin_init();
#endif

}

MACHINE_START(SMDK6410, "SMDK6410")
	/* Maintainer: Ben Dooks <ben@fluff.org> */
	.phys_io	= S3C_PA_UART & 0xfff00000,
	.io_pg_offst	= (((u32)S3C_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C64XX_PA_SDRAM + 0x100100,
	.fixup		= smdk6410_fixup,
	.init_irq	= s3c6410_init_irq,
	.map_io		= smdk6410_map_io,
	.init_machine	= smdk6410_machine_init,
#ifndef CONFIG_HIGH_RES_TIMERS
	.timer		= &s3c64xx_timer,
#else
	.timer		= &sec_timer,
#endif /* CONFIG_HIGH_RES_TIMERS */

MACHINE_END

#ifdef CONFIG_USB_SUPPORT
/* Initializes OTG Phy. */
void otg_phy_init(void) {

	writel(readl(S3C_OTHERS)|S3C_OTHERS_USB_SIG_MASK, S3C_OTHERS);
	writel(0x0, S3C_USBOTG_PHYPWR);		/* Power up */
        writel(OTGH_PHY_CLK_VALUE, S3C_USBOTG_PHYCLK);
	writel(0x1, S3C_USBOTG_RSTCON);

	udelay(50);
	writel(0x0, S3C_USBOTG_RSTCON);
	udelay(50);
}
EXPORT_SYMBOL(otg_phy_init);

/* USB Control request data struct must be located here for DMA transfer */
struct usb_ctrlrequest usb_ctrl __attribute__((aligned(8)));
EXPORT_SYMBOL(usb_ctrl);

/* OTG PHY Power Off */
void otg_phy_off(void) {
	writel(readl(S3C_USBOTG_PHYPWR)|(0x1F<<1), S3C_USBOTG_PHYPWR);
	writel(readl(S3C_OTHERS)&~S3C_OTHERS_USB_SIG_MASK, S3C_OTHERS);
}
EXPORT_SYMBOL(otg_phy_off);

void usb_host_clk_en(void) {
	struct clk *otg_clk;

        switch (S3C_USB_CLKSRC) {
	case 0: /* epll clk */
		writel((readl(S3C_CLK_SRC)& ~S3C6400_CLKSRC_UHOST_MASK)
			|S3C_CLKSRC_EPLL_CLKSEL|S3C_CLKSRC_UHOST_EPLL,
			S3C_CLK_SRC);

		/* USB host colock divider ratio is 2 */
		writel((readl(S3C_CLK_DIV1)& ~S3C6400_CLKDIV1_UHOST_MASK)
			|(1<<20), S3C_CLK_DIV1);
		break;
	case 1: /* oscillator 48M clk */
		otg_clk = clk_get(NULL, "otg");
		clk_enable(otg_clk);
		writel(readl(S3C_CLK_SRC)& ~S3C6400_CLKSRC_UHOST_MASK, S3C_CLK_SRC);
		otg_phy_init();
		/* USB host colock divider ratio is 1 */
		writel(readl(S3C_CLK_DIV1)& ~S3C6400_CLKDIV1_UHOST_MASK, S3C_CLK_DIV1);
		break;
	default:
		printk(KERN_INFO "Unknown USB Host Clock Source\n");
		BUG();
		break;
	}

	writel(readl(S3C_HCLK_GATE)|S3C_CLKCON_HCLK_UHOST|S3C_CLKCON_HCLK_SECUR,
		S3C_HCLK_GATE);
	writel(readl(S3C_SCLK_GATE)|S3C_CLKCON_SCLK_UHOST, S3C_SCLK_GATE);

}

EXPORT_SYMBOL(usb_host_clk_en);
#endif

#if defined(CONFIG_RTC_DRV_S3C)
/* RTC common Function for samsung APs*/
unsigned int s3c_rtc_set_bit_byte(void __iomem *base, uint offset, uint val)
{
	writeb(val, base + offset);

	return 0;
}

unsigned int s3c_rtc_read_alarm_status(void __iomem *base)
{
	return 1;
}

void s3c_rtc_set_pie(void __iomem *base, uint to)
{
	unsigned int tmp;

	tmp = readw(base + S3C_RTCCON) & ~S3C_RTCCON_TICEN;

        if (to)
                tmp |= S3C_RTCCON_TICEN;

        writew(tmp, base + S3C_RTCCON);
}

void s3c_rtc_set_freq_regs(void __iomem *base, uint freq, uint s3c_freq)
{
	unsigned int tmp;

        tmp = readw(base + S3C_RTCCON) & (S3C_RTCCON_TICEN | S3C_RTCCON_RTCEN );
        writew(tmp, base + S3C_RTCCON);
        s3c_freq = freq;
        tmp = (32768 / freq)-1;
        writel(tmp, base + S3C_TICNT);
}

void s3c_rtc_enable_set(struct platform_device *pdev,void __iomem *base, int en)
{
	unsigned int tmp;

	if (!en) {
		tmp = readw(base + S3C_RTCCON);
		writew(tmp & ~ (S3C_RTCCON_RTCEN | S3C_RTCCON_TICEN), base + S3C_RTCCON);
	} else {
		/* re-enable the device, and check it is ok */
		if ((readw(base+S3C_RTCCON) & S3C_RTCCON_RTCEN) == 0){
			dev_info(&pdev->dev, "rtc disabled, re-enabling\n");

			tmp = readw(base + S3C_RTCCON);
			writew(tmp|S3C_RTCCON_RTCEN, base+S3C_RTCCON);
		}

		if ((readw(base + S3C_RTCCON) & S3C_RTCCON_CNTSEL)){
			dev_info(&pdev->dev, "removing RTCCON_CNTSEL\n");

			tmp = readw(base + S3C_RTCCON);
			writew(tmp& ~S3C_RTCCON_CNTSEL, base+S3C_RTCCON);
		}

		if ((readw(base + S3C_RTCCON) & S3C_RTCCON_CLKRST)){
			dev_info(&pdev->dev, "removing RTCCON_CLKRST\n");

			tmp = readw(base + S3C_RTCCON);
			writew(tmp & ~S3C_RTCCON_CLKRST, base+S3C_RTCCON);
		}
	}
}
#endif

#if defined(CONFIG_KEYPAD_S3C) || defined (CONFIG_KEYPAD_S3C_MODULE)
void s3c_setup_keypad_cfg_gpio(int rows, int columns)
{
	unsigned int gpio;
	unsigned int end;

	end = S3C64XX_GPK(8 + rows);

	/* Set all the necessary GPK pins to special-function 0 */
	for (gpio = S3C64XX_GPK(8); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}

	end = S3C64XX_GPL(0 + columns);

	/* Set all the necessary GPL pins to special-function 0 */
	for (gpio = S3C64XX_GPL(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}
}

EXPORT_SYMBOL(s3c_setup_keypad_cfg_gpio);
#endif


void s3c_config_sleep_gpio(void)
{
	if (m8_checkse()) {
		__raw_writel((1<<0*4)|(1<<1*4)|(1<<2*4)|(1<<3*4)|(1<<4*4)|(1<<5*4)|(1<<6*4)|(1<<7*4), S3C64XX_GPKCON);
		__raw_writel((1<<(8-8)*4)|(1<<(9-8)*4)|(1<<(10-8)*4)|(1<<(11-8)*4)|(1<<(12-8)*4)|(1<<(13-8)*4)|(1<<(14-8)*4)|(1<<(15-8)*4), S3C64XX_GPKCON1);
		__raw_writel((0x1<<0*2)|(0x1<<1*2)|(0x1<<2*2)|(0x1<<3*2)|(0x1<<4*2)|(0x1<<5*2)|(0x1<<6*2)|(0x1<<7*2)|(0x1<<8*2)|(0x1<<9*2)|(0x1<<10*2)|(0x1<<11*2)|(0x1<<12*2)|(0x2<<13*2)|(0x1<<14*2)|(0x2<<15*2), S3C64XX_GPKPUD);
		__raw_writel((0<<14)|(1<<15), S3C64XX_GPKDAT);
		__raw_writel((__raw_readl(S3C64XX_GPMCON) & ~((0xF<<0*4)|(0xF<<1*4)|(0xF<<4*4)|(0xF<<5*4))) |((0<<0*4)|(0<<1*4)|(0<<4*4)|(0<<5*4)), S3C64XX_GPMCON);
		__raw_writel((__raw_readl(S3C64XX_GPMPUD) & ~((0x3<<0*2)|(0x3<<1*2)|(0x3<<4*2)|(0x3<<5*2)))|((0x1<<0*2)|(0x2<<1*2)|(0x2<<4*2)|(0x1<<5*2)), S3C64XX_GPMPUD);
		__raw_writel((__raw_readl(S3C64XX_GPMDAT) & ~((1<<0)|(1<<1)|(1<<4))) |((0<<0)|(1<<4)), S3C64XX_GPMDAT);
		__raw_writel(0x00, S3C64XX_GPAPUDSLP);
		__raw_writel((0x1<<0*2)|(0x2<<1*2)|(0x2<<2*2)|(0x1<<3*2)|(0x0<<4*2)|(0x0<<5*2)|(0x0<<6*2)|(0x0<<7*2), S3C64XX_GPACONSLP);
		__raw_writel((2<<2*2)|(2<<3*2), S3C64XX_GPBPUDSLP);
		__raw_writel((0x2<<0*2)|(0x1<<1*2)|(0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x2<<6*2), S3C64XX_GPBCONSLP);
		__raw_writel((0<<(4*2))|(0<<(6*2)), S3C64XX_GPCPUDSLP);
		__raw_writel((1<<(4*2))|(1<<(6*2)), S3C64XX_GPCCONSLP);
		__raw_writel((0x1<<0*2)|(0x1<<1*2)|(0x1<<2*2)|(0x1<<3*2)|(0x1<<4*2), S3C64XX_GPEPUDSLP);
		__raw_writel((0x1<<0*2)|(0x1<<1*2)|(0x1<<2*2)|(0x1<<3*2)|(0x1<<4*2), S3C64XX_GPECONSLP);
		__raw_writel((0<<0*2)|(2<<3*2), S3C64XX_GPFPUDSLP);
		__raw_writel((1<<0*2)|(2<<3*2), S3C64XX_GPFCONSLP);
		__raw_writel((0x0<<0*2)|(0x2<<1*2)|(0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x0<<6*2), S3C64XX_GPGPUDSLP);
		__raw_writel((0x2<<0*2)|(0x2<<1*2)|(0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x2<<6*2), S3C64XX_GPGCONSLP);
		__raw_writel(0xAAAAA, S3C64XX_GPHPUDSLP);
		__raw_writel((0x2<<0*2)|(0x2<<1*2)|(0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x2<<6*2)|(0x2<<7*2)|(0x2<<8*2)|(0x2<<9*2), S3C64XX_GPHCONSLP);
		__raw_writel(0x55555555, S3C64XX_GPIPUDSLP);
		__raw_writel(0xAAAAAAAA, S3C64XX_GPICONSLP);
		__raw_writel(0x55555555, S3C64XX_GPJPUDSLP);
		__raw_writel(0xAAAAAAAA, S3C64XX_GPJCONSLP);
		__raw_writel(0xAAAAAAAA, S3C64XX_GPOCON);
		__raw_writel(0x2AAAAAAA, S3C64XX_GPPCON);
		__raw_writel((0x2<<0*2)|(0x2<<1*2)|(0x2<<7*2)|(0x2<<8*2), S3C64XX_GPQCON);
		__raw_writel((0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x2<<6*2), S3C64XX_GPQCONSLP);
		__raw_writel((0x0<<2*2)|(0x0<<3*2)|(0x0<<4*2)|(0x0<<5*2)|(0x0<<6*2), S3C64XX_GPQPUDSLP);
		__raw_writel((0x0<<0)|(0x1<<12)|(1<<14)|(0x0<<20)|(0x0<<22)|(0x0<<4)| (0x0<<6)|(0x0<<16)|(0x0<<18), S3C64XX_MEM0CONSLP0);
		__raw_writel((0x1<<0)|(0x3<<2)|(0x3<<4)|(0x3<<6)|(0x3<<8)|(0x1<<10)|(0x1<<12)|(0x0<<14)|(0x0<<16)|(0x0<<18)|(0x1<<20)|(0x0<<22)|(0x0<<24), S3C64XX_MEM0CONSLP1);
		__raw_writel((0x5<<0)|(0x5<<4)|(0x5<<8)|(0x1<<12)|(0x1<<14)|(0x1<<16)|(0x1<<18)|(0x0<<20)|(0x0<<22)|(0x0<<24)|(0x0<<26)|(0x0<<28), S3C64XX_MEM1CONSLP);
/*
		if (wifi_status)
		{
			__raw_writel((0x0<<0*2)|(0x1<<1*2)|(0x1<<2*2)|(0x2<<3*2)|(0x1<<4*2), S3C64XX_GPDPUDSLP);
			__raw_writel((0x1<<0*2)|(0x0<<1*2)|(0x0<<2*2)|(0x1<<3*2)|(0x2<<4*2), S3C64XX_GPDCONSLP);
			__raw_writel((0x2<<0*2)|(0x2<<1*2)|(0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x1<<6*2)|(0x0<<7*2)|(0x2<<8*2)|(0x2<<9*2)|(0x2<<10*2)|(2<<11*2)|(1<<12*2)|(0x2<<13*2)|(0x2<<14*2)|(0x2<<15*2), S3C64XX_GPNCON);
			__raw_writel((1<<7*2)|(1<<8*2)|(1<<10*2)|(1<<11*2)|(1<<14*2), S3C64XX_GPNPUD);
			__raw_writel((1<<6), S3C64XX_GPNDAT);
			__raw_writel((0<<0*4)|(0<<1*4)|(0<<2*4)|(0<<3*4)|(1<<4*4)|(0<<5*4)|(0<<6*4)|(1<<7*4), S3C64XX_GPLCON);
			__raw_writel((3<<(8-8)*4)|(3<<(9-8)*4)|(0<<(10-8)*4)|(3<<(11-8)*4)|(0<<(12-8)*4)|(0<<(13-8)*4)|(1<<(14-8)*4), S3C64XX_GPLCON1);
			__raw_writel((0x1<<0*2)|(0x1<<1*2)|(0x1<<2*2)|(0x2<<3*2)|(0x1<<4*2)|(0x1<<5*2)|(0x1<<6*2)|(0x0<<7*2)|(0x1<<8*2)|(0x1<<9*2)|(0x1<<10*2)|(0x2<<11*2)|(0x1<<12*2)|(0x1<<13*2)|(0x1<<14*2), S3C64XX_GPLPUD);
			__raw_writel((0<<4)|(1<<7)|(0<<14), S3C64XX_GPLDAT);
		}
		else
		{
*/
			__raw_writel((0x0<<0*2)|(0x1<<1*2)|(0x1<<2*2)|(0x2<<3*2)|(0x1<<4*2), S3C64XX_GPDPUDSLP);
			__raw_writel((0x0<<0*2)|(0x0<<1*2)|(0x0<<2*2)|(0x1<<3*2)|(0x2<<4*2), S3C64XX_GPDCONSLP);
			__raw_writel((0x2<<0*2)|(0x2<<1*2)|(0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x1<<6*2)|(0x0<<7*2)|(0x2<<8*2)|(0x2<<9*2)|(0x2<<10*2)|(2<<11*2)|(1<<12*2)|(0x2<<13*2)|(0x2<<14*2)|(0x2<<15*2), S3C64XX_GPNCON);
			__raw_writel((1<<7*2)|(1<<8*2)|(1<<10*2)|(1<<11*2)|(1<<14*2), S3C64XX_GPNPUD);
			__raw_writel((0<<6), S3C64XX_GPNDAT);
			__raw_writel((0<<0*4)|(0<<1*4)|(0<<2*4)|(0<<3*4)|(1<<4*4)|(0<<5*4)|(0<<6*4)|(1<<7*4), S3C64XX_GPLCON);
			__raw_writel((3<<(8-8)*4)|(3<<(9-8)*4)|(0<<(10-8)*4)|(3<<(11-8)*4)|(0<<(12-8)*4)|(0<<(13-8)*4)|(1<<(14-8)*4), S3C64XX_GPLCON1);
			__raw_writel((0x1<<0*2)|(0x1<<1*2)|(0x1<<2*2)|(0x2<<3*2)|(0x1<<4*2)|(0x1<<5*2)|(0x1<<6*2)|(0x2<<7*2)|(0x1<<8*2)|(0x1<<9*2)|(0x1<<10*2)|(0x2<<11*2)|(0x1<<12*2)|(0x1<<13*2)|(0x1<<14*2), S3C64XX_GPLPUD);
			__raw_writel((0<<4)|(0<<7)|(0<<14), S3C64XX_GPLDAT);
//		}
	} else {
		__raw_writel((1<<0*4)|(1<<1*4)|(1<<2*4)|(1<<3*4)|(1<<4*4)|(1<<5*4)|(1<<6*4)|(1<<7*4), S3C64XX_GPKCON);
		__raw_writel((1<<(8-8)*4)|(1<<(9-8)*4)|(1<<(10-8)*4)|(1<<(11-8)*4)|(1<<(12-8)*4)|(1<<(13-8)*4)|(1<<(14-8)*4)|(1<<(15-8)*4), S3C64XX_GPKCON1);
		__raw_writel((0x2<<0*2)|(0x1<<1*2)|(0x1<<2*2)|(0x1<<3*2)|(0x1<<4*2)|(0x1<<5*2)|(0x1<<6*2)|(0x1<<7*2)|(0x1<<8*2)|(0x1<<9*2)|(0x1<<10*2)|(0x1<<11*2)|(0x1<<12*2)|(0x2<<13*2)|(0x1<<14*2)|(0x0<<15*2), S3C64XX_GPKPUD);
		__raw_writel((1<<0)|(0<<1)|(0<<13)|(0<<14)|(0<<15), S3C64XX_GPKDAT);
		__raw_writel((0<<0*4)|(0<<1*4)|(0<<2*4)|(0<<3*4)|(1<<4*4)|(0<<5*4)|(0<<6*4)|(1<<7*4), S3C64XX_GPLCON);
		__raw_writel((3<<(8-8)*4)|(3<<(9-8)*4)|(0<<(10-8)*4)|(1<<(11-8)*4)|(0<<(12-8)*4)|(0<<(13-8)*4)|(1<<(14-8)*4), S3C64XX_GPLCON1);
		__raw_writel((0x1<<0*2)|(0x1<<1*2)|(0x1<<2*2)|(0x2<<3*2)|(0x0<<4*2)|(0x1<<5*2)|(0x1<<6*2)|(0x1<<7*2)|(0x1<<8*2)|(0x1<<9*2)|(0x1<<10*2)|(0x0<<11*2)|(0x1<<12*2)|(0x1<<13*2)|(0x1<<14*2), S3C64XX_GPLPUD);
		__raw_writel((1<<4)|(0<<7)|(0<<14), S3C64XX_GPLDAT);
		__raw_writel((0<<0*4)|(1<<1*4)|(1<<2*4)|(0<<3*4)|(0<<4*4)|(0<<5*4), S3C64XX_GPMCON);
		__raw_writel((0x1<<0*2)|(0x2<<1*2)|(0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x1<<5*2), S3C64XX_GPMPUD);
		__raw_writel((0<<1)|(0<<2)|(1<<3)|(1<<4), S3C64XX_GPMDAT);
		__raw_writel(0x00, S3C64XX_GPAPUDSLP);
		__raw_writel((0x1<<0*2)|(0x2<<1*2)|(0x2<<2*2)|(0x1<<3*2)|(0x0<<4*2)|(0x0<<5*2)|(0x0<<6*2)|(0x0<<7*2), S3C64XX_GPACONSLP);
		__raw_writel((2<<2*2)|(2<<3*2), S3C64XX_GPBPUDSLP);
		__raw_writel((0x2<<0*2)|(0x1<<1*2)|(0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x2<<6*2), S3C64XX_GPBCONSLP);
		__raw_writel((0<<(4*2))|(0<<(6*2)), S3C64XX_GPCPUDSLP);
		__raw_writel((1<<(4*2))|(1<<(6*2)), S3C64XX_GPCCONSLP);
		__raw_writel((0x1<<0*2)|(0x1<<1*2)|(0x1<<2*2)|(0x1<<3*2)|(0x1<<4*2), S3C64XX_GPEPUDSLP);
		__raw_writel((0x1<<0*2)|(0x1<<1*2)|(0x1<<2*2)|(0x1<<3*2)|(0x1<<4*2), S3C64XX_GPECONSLP);
		__raw_writel((0<<0*2)|(2<<3*2), S3C64XX_GPFPUDSLP);
		__raw_writel((1<<0*2)|(2<<3*2), S3C64XX_GPFCONSLP);
		__raw_writel((0x0<<0*2)|(0x2<<1*2)|(0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x0<<6*2), S3C64XX_GPGPUDSLP);
		__raw_writel((0x2<<0*2)|(0x2<<1*2)|(0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x2<<6*2), S3C64XX_GPGCONSLP);
		__raw_writel(0xAAAAA, S3C64XX_GPHPUDSLP);
		__raw_writel((0x2<<0*2)|(0x2<<1*2)|(0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x2<<6*2)|(0x2<<7*2)|(0x2<<8*2)|(0x2<<9*2), S3C64XX_GPHCONSLP);
		__raw_writel(0x55555555, S3C64XX_GPIPUDSLP);
		__raw_writel(0xAAAAAAAA, S3C64XX_GPICONSLP);
		__raw_writel(0x55555555, S3C64XX_GPJPUDSLP);
		__raw_writel(0xAAAAAAAA, S3C64XX_GPJCONSLP);
		__raw_writel(0xAAAAAAAA, S3C64XX_GPOCON);
		__raw_writel(0x2AAAAAAA, S3C64XX_GPPCON);
		__raw_writel((0x2<<0*2)|(0x2<<1*2)|(0x2<<7*2)|(0x2<<8*2), S3C64XX_GPQCON);
		__raw_writel((0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x2<<6*2), S3C64XX_GPQCONSLP);
		__raw_writel((0x0<<2*2)|(0x0<<3*2)|(0x0<<4*2)|(0x0<<5*2)|(0x0<<6*2), S3C64XX_GPQPUDSLP);
		__raw_writel((0x0<<0)|(0x1<<12)|(1<<14)|(0x0<<20)|(0x0<<22)|(0x0<<4)| (0x0<<6)|(0x0<<16)|(0x0<<18), S3C64XX_MEM0CONSLP0);
		__raw_writel((0x1<<0)|(0x3<<2)|(0x3<<4)|(0x3<<6)|(0x3<<8)|(0x1<<10)|(0x1<<12)|(0x0<<14)|(0x0<<16)|(0x0<<18)|(0x1<<20)|(0x0<<22)|(0x0<<24), S3C64XX_MEM0CONSLP1);
		__raw_writel((0x5<<0)|(0x5<<4)|(0x5<<8)|(0x1<<12)|(0x1<<14)|(0x1<<16)|(0x1<<18)|(0x0<<20)|(0x0<<22)|(0x0<<24)|(0x0<<26)|(0x0<<28), S3C64XX_MEM1CONSLP);

		if (wifi_status)
		{
			__raw_writel((0x2<<0*2)|(0x1<<1*2)|(0x1<<2*2)|(0x2<<3*2)|(0x1<<4*2), S3C64XX_GPDPUDSLP);
			__raw_writel((0x1<<0*2)|(0x0<<1*2)|(0x0<<2*2)|(0x1<<3*2)|(0x2<<4*2), S3C64XX_GPDCONSLP);
			__raw_writel((0x2<<0*2)|(0x2<<1*2)|(0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x1<<6*2)|(0x2<<7*2)|(0x2<<8*2)|(0x2<<9*2)|(0x2<<10*2)|(2<<11*2)|(1<<12*2)|(0x2<<13*2)|(0x2<<14*2)|(0x2<<15*2), S3C64XX_GPNCON);
			__raw_writel((1<<7*2)|(1<<8*2)|(1<<10*2)|(1<<11*2)|(1<<14*2), S3C64XX_GPNPUD);
			__raw_writel((1<<6), S3C64XX_GPNDAT);
		}
		else
		{
			__raw_writel((0x0<<0*2)|(0x1<<1*2)|(0x1<<2*2)|(0x2<<3*2)|(0x1<<4*2), S3C64XX_GPDPUDSLP);
			__raw_writel((0x0<<0*2)|(0x0<<1*2)|(0x0<<2*2)|(0x1<<3*2)|(0x2<<4*2), S3C64XX_GPDCONSLP);
			__raw_writel((0x2<<0*2)|(0x2<<1*2)|(0x2<<2*2)|(0x2<<3*2)|(0x2<<4*2)|(0x2<<5*2)|(0x1<<6*2)|(0x2<<7*2)|(0x2<<8*2)|(0x2<<9*2)|(0x2<<10*2)|(2<<11*2)|(1<<12*2)|(0x2<<13*2)|(0x2<<14*2)|(0x2<<15*2), S3C64XX_GPNCON);
			__raw_writel((1<<7*2)|(1<<8*2)|(1<<10*2)|(1<<11*2)|(1<<14*2), S3C64XX_GPNPUD);
			__raw_writel((0<<6), S3C64XX_GPNDAT);
		}
	}
}
EXPORT_SYMBOL(s3c_config_sleep_gpio);

void s3c_config_wakeup_gpio(void)
{
}
EXPORT_SYMBOL(s3c_config_wakeup_gpio);

void s3c_config_wakeup_source(void)
{
	set_irq_type(gpio_to_irq(S3C64XX_GPN(9)), IRQ_TYPE_EDGE_BOTH);
	set_irq_type(gpio_to_irq(S3C64XX_GPN(4)), IRQ_TYPE_EDGE_BOTH);
	set_irq_type(gpio_to_irq(S3C64XX_GPN(13)), IRQ_TYPE_EDGE_BOTH);
	s3c_gpio_setpull(S3C64XX_GPN(13), S3C_GPIO_PULL_NONE);
	set_irq_type(gpio_to_irq(S3C64XX_GPL(9)), IRQ_TYPE_EDGE_BOTH);
	s3c_gpio_setpull(S3C64XX_GPL(9), S3C_GPIO_PULL_NONE);

	udelay(50);

	__raw_writel((1<<4)|(1<<9)|(1<<13)|(1<<17), S3C64XX_EINT0PEND);
	__raw_writel(0x0fffffff&~((1<<4)|(1<<9)|(1<<13)|(1<<17)), S3C64XX_EINT0MASK);	
	__raw_writel(0x0fffffff&~((1<<4)|(1<<9)|(1<<13)|(1<<17)), S3C_EINT_MASK);

	/* Alarm Wakeup Enable */
	__raw_writel((__raw_readl(S3C_PWR_CFG) & ~(0x1 << 10)), S3C_PWR_CFG);
	__raw_writel(0xfff, S3C_WAKEUP_STAT);
}
EXPORT_SYMBOL(s3c_config_wakeup_source);

// Return true for M8-SE, otherwise for M8-FE 
int m8_checkse(void)
{
	static bool detected = false;
	static bool m8_se = false;
	int gpq2, gpq3;
	
	if (!detected) {
		s3c_gpio_cfgpin(S3C64XX_GPQ(2),S3C_GPIO_INPUT);
		s3c_gpio_setpull(S3C64XX_GPQ(2), S3C_GPIO_PULL_DOWN);
		gpq2 = gpio_get_value(S3C64XX_GPQ(2));

		s3c_gpio_cfgpin(S3C64XX_GPQ(3),S3C_GPIO_INPUT);
		s3c_gpio_setpull(S3C64XX_GPQ(3), S3C_GPIO_PULL_DOWN);
		gpq3 = gpio_get_value(S3C64XX_GPQ(3));
		
		if ((gpq2 == 0) && (gpq3 == 0)) {
			m8_se = false;
			printk("M8 Version Detect: M8 FE\n");
			proc_mkdir("m8_fe", 0);
		} else {
			m8_se = true;
			printk("M8 Version Detect: M8 SE\n");
			proc_mkdir("m8_se", 0);
		}

		detected = true;
	}
	
	return m8_se;
}
EXPORT_SYMBOL(m8_checkse);
