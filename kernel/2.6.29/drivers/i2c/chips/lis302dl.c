/*
 *  m9_accsensor.c - ST lis33de accelerometer driver
 *
 */
 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/sysfs.h>
#include <linux/ctype.h>
#include <linux/hwmon-sysfs.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/input.h>

#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
#include "lis302dl.h"

#define GPIO_MEIZU_KEY_VOL_UP 146

#define LIS302DL_DRV_NAME "st-lis302dl"
#define LIS302DL_SIZE 50
#define AXIS_MAX_VALUE 255

#ifdef CONFIG_MACH_MEIZU_M8_3G
  #define INT1_PIN  S5PC1XX_GPH1(0)
  #define INT2_PIN  S5PC1XX_GPH3(5)
  #define GPIO_FUN_INT  S3C_GPIO_SFN(2)
#endif 

#ifdef CONFIG_MACH_MEIZU_M9
  #define INT1_PIN  S5PC11X_GPH1(7)
  #define GPIO_FUN_INT  S3C_GPIO_SFN(15)  //the int func is 1111
#endif 

#if defined(CONFIG_MACH_MEIZU_M8) || defined(CONFIG_MACH_SMDK6410)
  #define INT1_PIN  S3C64XX_GPL(13)
  #define INT2_PIN  S3C64XX_GPM(0)
  #define GPIO_FUN_INT  S3C_GPIO_SFN(3)
#endif

//#define KEY_REPORT	//add by hui
#ifdef KEY_REPORT
extern struct input_dev *extern_input_dev;
#endif
static struct workqueue_struct *lis302dl_wq;
       
//lis33de private data
struct lis302dl_data {
	uint16_t addr;
	unsigned int irq;
	struct work_struct  work1;
	struct i2c_client *client;
	struct input_dev *input_dev;
	unsigned int x;
	unsigned int y;
	unsigned int z;
};

static int lis302_i2c_read_byte(struct i2c_client *client,unsigned char adr)
{
	char buf;
	int ret;
	
	buf = adr;
	ret = i2c_master_send(client, &buf, 1);
	if(ret < 0) {
		dev_err(&client->dev, "%s [%d] failed to transmit instructions to lis302.\n", __FUNCTION__, __LINE__);
		return ret;
	}
	
	ret = i2c_master_recv(client, &buf, 1);
	if (ret<0) {
		dev_err(&client->dev, "%s [%d] failed to receive response from lis302.\n", __FUNCTION__, __LINE__);
		return ret;
	}
	
	return ret = buf;
}

static int lis302_i2c_write_byte(struct i2c_client *client,char adr,char data)
{
	char buf[2];
	int ret;
	
	buf[0] = adr;
	buf[1] = data;
	ret = i2c_master_send(client, buf, 2);
	if(ret<0)
		dev_err(&client->dev, "%s [%d] failed to transmit instructions to lis302.\n", __FUNCTION__, __LINE__);
	
	return ret;
}

/*
 * Generic x,y,z attributes
 */
static ssize_t lis302dl_show_xyz(struct device *dev, struct device_attribute *attr,char *buf)
{
//	signed char x, y, z;
	int ret;
	unsigned long irq_flag;
	struct sensor_device_attribute_2 *sattr = to_sensor_dev_attr_2(attr);
	struct i2c_client *client = to_i2c_client(dev);
	struct lis302dl_data *sensor = i2c_get_clientdata(client);
	
	dev_dbg(dev, "lis302dl_show() called on %s; index:0x%x\n", attr->attr.name,sattr->index);
	
	/* Read the x_outregister */	
	if ((sensor->x == 0) && (sensor->y == 0) && (sensor->z == 0))
	{
		ret = lis302_i2c_read_byte(client, sattr->index);
		if (ret < 0) {
			printk("Read i2c error\n");
			return -EIO;
		}
		sensor->x = ret;
		
		/* Read the y_out register */
		ret = lis302_i2c_read_byte(client, sattr->index+2);
		if (ret < 0) {
			printk("Read i2c error\n");
			return -EIO;
		}
		sensor->y = ret;
		
		/*Read the z_out register*/
		ret = lis302_i2c_read_byte(client, sattr->index+4);
		if (ret < 0) {
			printk("Read i2c error\n");
			return -EIO;
		}    
		sensor->z = ret;
	}
	
//	return sprintf(buf,"(%d, %d, %d)\n", sensor->x, sensor->y, sensor->z);	
	return sprintf(buf,"%c%c%c\n", sensor->x, sensor->y, sensor->z);
}
   
static ssize_t lis302dl_show_ctrl(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct sensor_device_attribute_2 *sattr = to_sensor_dev_attr_2(attr);
	struct i2c_client *client = to_i2c_client(dev);
	int ret;

	dev_dbg(dev, "lis302dl_show() called on %s; index:0x%x\n", attr->attr.name,sattr->index);

	/* Read the first register */
	ret = lis302_i2c_read_byte(client, sattr->index);
	if (ret < 0) {
		printk("Read i2c error\n");
		return -EIO;
	}
	
	return sprintf(buf, "0x%.2x\n", ret);
}

static ssize_t lis302dl_set_ctrl(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct sensor_device_attribute_2 *sattr = to_sensor_dev_attr_2(attr);
	struct i2c_client *client = to_i2c_client(dev);
	char *endp;
	unsigned int val;
	int ret = 0;

	printk("enter lis302dl_set.\n");

	dev_dbg(dev, "ds1682_store() called on %s\n", attr->attr.name);

	/* Decode input */
	val = simple_strtoul(buf, &endp, 0);
	if (buf == endp) {
		dev_dbg(dev, "input string not a number\n");
		return -EINVAL;
	}

	ret = lis302_i2c_write_byte(client,sattr->index,val);

	return (ret>0) ? count:ret;
}


static void lis302dl_work_func1(struct work_struct *work)
{
	struct lis302dl_data *sensor = container_of(work, struct lis302dl_data, work1);
	int status = 0, ff_src_value;
	int x, y, z; 
	int pin_state;
	
//	int ret=0;

	status = lis302_i2c_read_byte(sensor->client,STATUS_REG);
	ff_src_value = lis302_i2c_read_byte(sensor->client,FF_WU_SRC);	//must read this register
	
	x = lis302_i2c_read_byte(sensor->client,OUT_X);
	y = lis302_i2c_read_byte(sensor->client,OUT_Y);
	z = lis302_i2c_read_byte(sensor->client,OUT_Z);

      //report the event
	input_report_abs(sensor->input_dev, ABS_X, x);
	input_report_abs(sensor->input_dev, ABS_Y, y);
	input_report_abs(sensor->input_dev, ABS_Z, z);
	input_sync(sensor->input_dev);

	//here must be add spin_lock and irq code later in order to prevent other process access
	sensor->x = x;
	sensor->y = y;
	sensor->z = z;

	//here is to add the code of keyboard report input event

	//at first to check the big voice button is pressed or not.

	//then report the event, it must report 1 first and report 0 last ?
#ifdef KEY_REPORT	//add by hui

	pin_state = gpio_get_value(GPIO_MEIZU_KEY_VOL_UP);
	if (pin_state == 0) {
		signed char a = -(signed char)(sensor->x);
		if (a < -10) {
			input_event(extern_input_dev, EV_KEY, KEY_LEFT, 1); //state其实表示的就是press，1表按下，0表示抬起,先上报1，再上报0(按键动作完成，清除按键事件)，Linux规定的, modified by hui
			input_sync(extern_input_dev);
			input_event(extern_input_dev, EV_KEY, KEY_LEFT, 0); 
			input_sync(extern_input_dev);
		}
		else if (a > 10) {
			input_event(extern_input_dev, EV_KEY, KEY_RIGHT, 1); 
			input_sync(extern_input_dev);	
			input_event(extern_input_dev, EV_KEY, KEY_RIGHT, 0); 
			input_sync(extern_input_dev);
		}
	}
#endif

//	printk("enter work_func1\n");
//	printk("status=%02x, ff_src_value=%02x.\n", status, ff_src_value);
//	printk("x = %d, y = %d, z = %d.\n", sensor->x, sensor->y, sensor->z);
//	printk("\n");
	msleep_interruptible(50);
#if 0
	signed char x, y, z;
	x = sensor->x;
	y = sensor->y;
	z = sensor->z;

	if (x < 0) printk("right......................................................................................\n");
	else if (x > 0) printk("left................................................................................................\n");

	printk("acc_x = %d /1000.\n", (int)(x * 18 * 10));
	printk("acc_y = %d /1000.\n", (int)(y * 18 * 10));
	printk("acc_z = %d./1000\n", (int)(z * 18 * 10));
#endif

	if(sensor->irq)
		enable_irq(sensor->irq);
}

static irqreturn_t lis302dl_irq1_handler(int irq, void *dev_id)
{
	struct lis302dl_data  *sensor = (struct lis302dl_data  *)dev_id;
	
	dev_dbg(&sensor->client->dev,"%s\n",__func__);
	printk("lis302dl_irq1_handler\n");
	
	disable_irq_nosync(irq);
	queue_work(lis302dl_wq, &sensor->work1);
	
	return IRQ_HANDLED;
}

void power_on(struct i2c_client *client)
{
	unsigned int irq_num;
	int ret;
	
	lis302_i2c_write_byte(client, CTRL_REG1, CTRL1_PD | CTRL1_Xen | CTRL1_Yen | CTRL1_Zen);  //100Hz, 2g, xen, yen, zen
      lis302_i2c_write_byte(client, CTRL_REG2, 0x00);   
//	lis302_i2c_write_byte(client, CTRL_REG3, 0x00);  //disable ff_wu_intterrupt
	lis302_i2c_write_byte(client, CTRL_REG3, 0x81);  //active low, int1->ff_wu_cfg
	
	lis302_i2c_write_byte(client, FF_WU_THS, 0x14);  //350mg
	lis302_i2c_write_byte(client,FF_WU_DURATION,0x00);  //Duration value
//	lis302_i2c_write_byte(client, FF_WU_CFG, 0x2a);  //or, all high, if i set this, the interrupt is not work ok
	lis302_i2c_write_byte(client, FF_WU_CFG, 0x6a);  //or, latch, all high

	if(gpio_is_valid(INT1_PIN))		/*init1*/
	{
		irq_num = gpio_to_irq(INT1_PIN);
		ret =request_irq(irq_num, lis302dl_irq1_handler, IRQF_TRIGGER_FALLING, client->name, client->dev.driver_data);
		if(ret < 0) {
			printk("request_irq failed\n");
			pr_err("request_irq failed\n");
		}
	}

}

void power_off(struct i2c_client *client)
{
	struct lis302dl_data *lis302dl = i2c_get_clientdata(client);
	
	free_irq(lis302dl->irq, lis302dl);
	lis302_i2c_write_byte(client,CTRL_REG1,0x00);
}   

/*
 * Simple register attributes
 */
static SENSOR_DEVICE_ATTR_2(xyz, S_IRUGO | S_IWUSR, lis302dl_show_xyz,
			    NULL, 3, OUT_X);
static SENSOR_DEVICE_ATTR_2(x, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    NULL, 1, OUT_X);
static SENSOR_DEVICE_ATTR_2(y, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    NULL, 1, OUT_Y);
static SENSOR_DEVICE_ATTR_2(z, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    NULL, 1, OUT_Z);
static SENSOR_DEVICE_ATTR_2(ctrl1, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, CTRL_REG1);
static SENSOR_DEVICE_ATTR_2(ctrl2, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, CTRL_REG2);
static SENSOR_DEVICE_ATTR_2(ctrl3, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, CTRL_REG3);
static SENSOR_DEVICE_ATTR_2(status, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    NULL, 1, STATUS_REG);
static SENSOR_DEVICE_ATTR_2(ff_cfg, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, FF_WU_CFG);
static SENSOR_DEVICE_ATTR_2(ff_src, S_IRUGO, lis302dl_show_ctrl,
			    NULL, 1, FF_WU_SRC);
static SENSOR_DEVICE_ATTR_2(ff_ths, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, FF_WU_THS);
static SENSOR_DEVICE_ATTR_2(ff_duration, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
                            lis302dl_set_ctrl, 1, FF_WU_DURATION);


static const struct attribute_group lis302dl_group = {
	.attrs = (struct attribute *[]) {
		&sensor_dev_attr_xyz.dev_attr.attr,
		&sensor_dev_attr_x.dev_attr.attr,
		&sensor_dev_attr_y.dev_attr.attr,
		&sensor_dev_attr_z.dev_attr.attr,
		&sensor_dev_attr_ctrl1.dev_attr.attr,
		&sensor_dev_attr_ctrl2.dev_attr.attr,
		&sensor_dev_attr_ctrl3.dev_attr.attr,
		&sensor_dev_attr_status.dev_attr.attr,
		&sensor_dev_attr_ff_cfg.dev_attr.attr,
		&sensor_dev_attr_ff_src.dev_attr.attr,
		&sensor_dev_attr_ff_ths.dev_attr.attr,
		&sensor_dev_attr_ff_duration.dev_attr.attr,
		NULL
	},
};

/*
 * User data attribute
 */
static ssize_t lis302dl_sensor_read(struct kobject *kobj, struct bin_attribute *attr,
				  char *buf, loff_t off, size_t count)
{
	struct i2c_client *client = kobj_to_i2c_client(kobj);
	int ret;
	char buf_tmp;
	
	printk("enter lis302_sensor_read.\n");

	dev_dbg(&client->dev, "ds1682_eeprom_read(p=%p, off=%lli, c=%zi)\n",
		buf, off, count);

	if (off >= LIS302DL_SIZE)
		return 0;

	if (off + count > LIS302DL_SIZE)
		count = LIS302DL_SIZE - off;

	buf_tmp = off;
	ret =i2c_master_send(client, &buf_tmp, 1);
	if(ret<0){
		dev_err(&client->dev, "failed to transmit instructions to lis302.\n");
		return ret;
	}
	
	ret = i2c_master_recv(client, buf, count);
	if (ret<0) {
		dev_err(&client->dev, "failed to receive response from lis302.\n");
		return ret;
	}

	return count;
}

static ssize_t lis302dl_sensor_write(struct kobject *kobj, struct bin_attribute *attr,
				   char *buf, loff_t off, size_t count)
{
	struct i2c_client *client = kobj_to_i2c_client(kobj);
	int ret;
	char *buf_tmp;
	
	printk("enter lis302dl_sensor_write.\n");	

	dev_dbg(&client->dev, "ds1682_eeprom_write(p=%p, off=%lli, c=%zi)\n",
		buf, off, count);

	if (off >= LIS302DL_SIZE)
		return -ENOSPC;

	if (off + count > LIS302DL_SIZE)
		count = LIS302DL_SIZE - off;

	/* Write out to the device */
	buf_tmp = kzalloc(count+1, GFP_TEMPORARY);
	if(buf_tmp == NULL)
		return -ENOMEM;

	buf_tmp[0] = off;
	memcpy(&buf_tmp[1],buf,count);
	
	ret =i2c_master_send(client, buf_tmp, count+1);
	if(ret<0)
		dev_err(&client->dev, "failed to transmit instructions to lis302.\n");
	
	kfree(buf_tmp);

	return (ret < 0) ? ret:count;
}

static struct bin_attribute lis302dl_sensor_attr = {
	.attr = {
		.name = "meizu_sensor",
		.mode = S_IRUGO | S_IWUSR,
	},
	.size = LIS302DL_SIZE,
	.read = lis302dl_sensor_read,
	.write = lis302dl_sensor_write,
};


static int __devinit lis302dl_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	int ret;
	int pin_num;
	struct lis302dl_data *lis302dl;
	struct input_dev *idev;
	
	if (!i2c_check_functionality(client->adapter,I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c bus does not support the ST lis302dl\n");
		ret = -ENODEV;
		return ret;
	}

	lis302dl = kzalloc(sizeof(*lis302dl), GFP_KERNEL);
	if (lis302dl == NULL) {
		ret = -ENOMEM;
		return ret;
	}
	lis302dl->client = client;

	printk("client name is %s\n", client->name);
	//	printk("dev name is %s\n", client->dev.name);
	//here is to add the input dev struct

	idev = input_allocate_device();
	if (idev == NULL)
		return -ENOMEM;
	
	idev->name       = LIS302DL_DRV_NAME;
	idev->phys       = LIS302DL_DRV_NAME "/input0";
	idev->id.bustype = BUS_HOST;
	idev->id.vendor  = 0;
	idev->dev.parent = &client->dev;
//	idev->open       = lis3lv02d_joystick_open;
//	idev->close      = lis3lv02d_joystick_close;

	set_bit(EV_ABS, idev->evbit);
	set_bit(ABS_X, idev->absbit);
	set_bit(ABS_Y, idev->absbit);
	set_bit(ABS_Z, idev->absbit);
	input_set_abs_params(idev, ABS_X, 0, AXIS_MAX_VALUE, 0, 0);
	input_set_abs_params(idev, ABS_Y, 0, AXIS_MAX_VALUE, 0, 0);
	input_set_abs_params(idev, ABS_Z, 0, AXIS_MAX_VALUE, 0, 0);
	lis302dl->input_dev = idev;

	i2c_set_clientdata(client, lis302dl);	
	
//end add input device

	INIT_WORK(&lis302dl->work1, lis302dl_work_func1);
	
	pin_num =INT1_PIN;
	if(gpio_is_valid(pin_num))		/*init1*/
	{
		lis302dl->irq = gpio_to_irq(pin_num);
		ret = gpio_request(pin_num, "GPL");
		s3c_gpio_cfgpin(pin_num,  GPIO_FUN_INT);
		s3c_gpio_setpull(pin_num, S3C_GPIO_PULL_DOWN);
	}

	power_on(client);

	ret = sysfs_create_group(&client->dev.kobj, &lis302dl_group);
	if (ret) return ret;
	
	ret = sysfs_create_bin_file(&client->dev.kobj, &lis302dl_sensor_attr);
	if (ret)
		sysfs_remove_group(&client->dev.kobj, &lis302dl_group);

	ret = input_register_device(lis302dl->input_dev);
	if (ret) {
		printk("register failed!\n");
		input_free_device(lis302dl->input_dev);
		return ret;
	}

	printk("enter the lis302dl_probe !\n");

	return ret;

}

static int __devexit lis302dl_remove(struct i2c_client *client)
{
	struct lis302dl_data *sensor = i2c_get_clientdata(client);
	sysfs_remove_group(&client->dev.kobj, &lis302dl_group);
	sysfs_remove_bin_file(&client->dev.kobj, &lis302dl_sensor_attr);
	free_irq(sensor->irq, sensor);
	gpio_free(INT1_PIN);
	input_free_device(sensor->input_dev);
	kfree(sensor);

	printk("enter the lis302dl_remove !\n");
	return 0;
}

#ifdef CONFIG_PM

static int lis302dl_suspend(struct i2c_client *client, pm_message_t mesg)
{
	power_off(client);
	
	return 0;
}

static int lis302dl_resume(struct i2c_client *client)
{
	power_on(client);
	
	return 0;
}

#else

#define lis302dl_suspend		NULL
#define lis302dl_resume			NULL

#endif   /* CONFIG_PM */

static const struct i2c_device_id lis302dl_id[] = {
	{ "st-lis302dl", 0 },
	{ }
};
static struct i2c_driver lis302dl_driver = {
	.driver = {
		.name	= LIS302DL_DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.probe	= lis302dl_probe,
	.remove	= lis302dl_remove,
//	.suspend = lis302dl_suspend,
//	.resume	= lis302dl_resume,
	.id_table = lis302dl_id,
};

static int __init lis302dl_init(void)
{
	lis302dl_wq = create_singlethread_workqueue("lis302_wq");
	if(!lis302dl_wq)
		return -ENOMEM;
	return i2c_add_driver(&lis302dl_driver);
}

static void __exit lis302dl_exit(void)
{
	i2c_del_driver(&lis302dl_driver);
	if (lis302dl_wq)
		destroy_workqueue(lis302dl_wq);	
}

module_init(lis302dl_init);
module_exit(lis302dl_exit);

MODULE_DESCRIPTION("ST LIS302DL driver");
MODULE_AUTHOR("Meizu");
MODULE_LICENSE("GPL");

