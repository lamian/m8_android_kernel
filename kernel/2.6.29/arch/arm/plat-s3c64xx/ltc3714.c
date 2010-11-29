#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/stat.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <plat/regs-gpio.h>
#include <plat/gpio-cfg.h>

#include <plat/s3c64xx-dvfs.h>

#ifdef CONFIG_DVFS_LOGMESSAGE
#include <linux/cpufreq.h>
#include <linux/kernel.h>
extern void dvfs_debug_printk(unsigned int type, const char *prefix, const char *fmt, ...);
#define dprintk(msg...) dvfs_debug_printk(CPUFREQ_DEBUG_DRIVER, "dvfs", msg)
#else

#define dprintk(msg...) do { } while(0)

#endif /* CONFIG_CPU_FREQ_DEBUG */

/* ltc3714 voltage table */
static const unsigned int voltage_table[32] = {
	1750, 1700, 1650, 1600, 1550, 1500, 1450, 1400,
	1350, 1300, 1250, 1200, 1150, 1100, 1050, 1000,
	975, 950, 925, 900, 875, 850, 825, 800,
	775, 750, 725, 700, 675, 650, 625, 600,
};

/* LTC3714 Setting Routine */
static int ltc3714_gpio_setting(void)
{
	gpio_direction_output(S3C64XX_GPN(11), 0);
	gpio_direction_output(S3C64XX_GPN(12), 0);
	gpio_direction_output(S3C64XX_GPN(13), 0);
	gpio_direction_output(S3C64XX_GPN(14), 0);
	gpio_direction_output(S3C64XX_GPN(15), 0);
	gpio_direction_output(S3C64XX_GPL(8), 0);
	gpio_direction_output(S3C64XX_GPL(9), 0);
	gpio_direction_output(S3C64XX_GPL(10), 0);

	s3c_gpio_setpull(S3C64XX_GPN(11), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C64XX_GPN(12), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C64XX_GPN(13), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C64XX_GPN(14), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C64XX_GPN(15), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C64XX_GPL(8), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C64XX_GPL(9), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C64XX_GPL(10), S3C_GPIO_PULL_NONE);

	return 0;
}

int set_pmic(unsigned int pwr, unsigned int voltage)
{
	int position = 0;
	int first = 0;

	int last = 31;

	if(voltage > voltage_table[0] || voltage < voltage_table[last]) {
		dprintk("[ERROR]: voltage value over limits!!!");
		return -EINVAL;
	}

	while(first <= last) {
		position = (first + last) / 2;
		if(voltage_table[position] == voltage) {

			position &=0x1f;

			gpio_set_value(S3C64XX_GPN(11),(position >> 0)&0x1);
			udelay(5);
			gpio_set_value(S3C64XX_GPN(12),(position >> 1)&0x1);
			gpio_set_value(S3C64XX_GPN(13),(position >> 2)&0x1);
			gpio_set_value(S3C64XX_GPN(14),(position >> 3)&0x1);
			gpio_set_value(S3C64XX_GPN(15),(position >> 4)&0x1);
			
			if(pwr == VCC_ARM) {
				gpio_set_value(S3C64XX_GPL(8), 1);
				udelay(10);
				gpio_set_value(S3C64XX_GPL(8), 0);
				dprintk("============ VDD_ARM = %d\n",voltage);
				return 0;
			}
			else if(pwr == VCC_INT) {
				gpio_set_value(S3C64XX_GPL(10), 1);
				udelay(10);
				gpio_set_value(S3C64XX_GPL(10), 0);
				dprintk("============ VDD_INT = %d\n",voltage);
				return 0;
			}
			else {
				dprintk("[error]: set_power, check mode [pwr] value\n");
				return -EINVAL;
			}

		}
		else if (voltage > voltage_table[position])
			last = position - 1;
		else
			first = position + 1;
	}
	dprintk("[error]: Can't find adquate voltage table list value\n");
	return -EINVAL;
}

void ltc3714_init(unsigned int arm_voltage, unsigned int int_voltage)
{
	ltc3714_gpio_setting();

	/* set maximum voltage */
	set_pmic(VCC_ARM, arm_voltage);
	set_pmic(VCC_INT, int_voltage);

	gpio_set_value(S3C64XX_GPL(9), 1);
}

EXPORT_SYMBOL(ltc3714_init);
