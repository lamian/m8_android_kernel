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
#include <linux/i2c.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include <plat/regs-gpio.h>
#include <plat/gpio-cfg.h>

struct ltc3555_data {
	uint16_t addr;
	char data1;
	char data2;
	unsigned int usb_detect_irq;
	unsigned int bat_low_irq;
	uint32_t flags;
	struct i2c_client *client;
	unsigned int core_voltage;
	unsigned char int_voltage;
};

static int ltc3555_write(struct i2c_client *client,char data1,char data2)
{
	char buf[2];
	int ret;
	
	buf[0] = data1;
	buf[1] = data2;
	ret = i2c_master_send(client, buf, 2);
	if(ret<0)
		dev_err(&client->dev, "ltc3555 operation failed.\n");
	
	return ret;
}

#ifdef CONFIG_PM

static int ltc3555_suspend(struct i2c_client *client, pm_message_t mesg)
{
	if (__raw_readl(S3C64XX_GPNDAT) & (1<<13)) {
		ltc3555_write(client,0xff,0x61);
	} else {
		ltc3555_write(client,0xff,0xe2);
	}
	return 0;
}

static int ltc3555_resume(struct i2c_client *client)
{
	if(ltc3555_write(client,0xff,0x7d)<0)
		printk("fail to resume ltc3555\n");

	return 0;
}

#else

#define ltc3555_suspend			NULL
#define ltc3555_resume			NULL

#endif   /* CONFIG_PM */

static int __devinit ltc3555_probe(struct i2c_client *client,const struct i2c_device_id *id)
{	
	int ret = 0;
	int pin_num;
	unsigned int irq_num;
	struct ltc3555_data *ltc3555;

	if (!i2c_check_functionality(client->adapter,I2C_FUNC_I2C)) {
		printk("i2c bus does not support the ltc3555\n");
		ret = -ENODEV;
		return ret;
	}
	
	ret = ltc3555_write(client,0xff,0x7d);
	if(ret != 2) {
		printk("fail to initialize ltc3555\n");
		return ret;
	} else {
		ret = 0;
	}
	
	ltc3555 = kzalloc(sizeof(*ltc3555), GFP_KERNEL);
	if (ltc3555 == NULL) {
		ret = -ENOMEM;
		return ret;
	}
	ltc3555->client = client;
	i2c_set_clientdata(client, ltc3555);
	
	return ret;

}

static int __devexit ltc3555_remove(struct i2c_client *client)
{
	struct ltc3555_data *ltc3555 = i2c_get_clientdata(client);;
	kfree(ltc3555);
	return 0;
}

static void ltc3555_shutdown(struct i2c_client *client)
{
	ltc3555_write(client, 0xff, 0x03);
	return 0;
}

static const struct i2c_device_id ltc3555_id[] = {
	{ "ltc3555", 0 },
	{ }
};

static struct i2c_driver ltc3555_driver = {
	.driver = {
		.name	= "ltc3555",
		.owner	= THIS_MODULE,
	},
	.probe	= ltc3555_probe,
	.remove	= ltc3555_remove,
	.suspend = ltc3555_suspend,
	.resume = ltc3555_resume,
	.shutdown = ltc3555_shutdown,
	.id_table = ltc3555_id,
};

static int __init ltc3555_init(void)
{
	return i2c_add_driver(&ltc3555_driver);
}

static void __exit ltc3555_exit(void)
{
	i2c_del_driver(&ltc3555_driver);
}

module_init(ltc3555_init);
module_exit(ltc3555_exit);

MODULE_DESCRIPTION("LTC3555 PMIC driver");
MODULE_AUTHOR("lamian");
MODULE_LICENSE("GPL");
