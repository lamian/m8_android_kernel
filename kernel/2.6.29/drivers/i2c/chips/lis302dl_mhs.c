/*
 *  m8_accsensor.c - ST LIS3LV02DL accelerometer driver
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

#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
#include "lis302dl.h"

#define LIS302DL_DRV_NAME "st-lis302dl"
#define LIS302DL_SIZE 0x40

#if defined(CONFIG_MACH_MEIZU_M8_3G) ||	defined(CONFIG_MACH_SMDK6410)	
  #define INT1_PIN  S5PC1XX_GPH1(0)
  #define INT2_PIN  S5PC1XX_GPH3(5)
  #define GPIO_FUN_INT  S3C_GPIO_SFN(2)
#endif 

#if defined(CONFIG_MACH_MEIZU_M8) || defined(CONFIG_MACH_SMDK6410)
  #define INT1_PIN  S3C64XX_GPL(13)
  #define INT2_PIN  S3C64XX_GPM(0)
  #define GPIO_FUN_INT  S3C_GPIO_SFN(3)
#endif

static struct workqueue_struct *lis302dl_wq;

struct lis302dl_data {
	uint16_t addr;
	unsigned int irq1;
	unsigned int irq2;
	struct work_struct  work1;
	struct work_struct  work2;
	uint32_t flags;
	int (*power)(int on);
	struct i2c_client *client;
	unsigned char x;
	unsigned char y;
	unsigned char z;
};

static int lis302_i2c_read_byte(struct i2c_client *client,unsigned char adr)
{
	char buf;
	int ret;
	
	buf = adr;
	ret =i2c_master_send(client, &buf, 1);
	if(ret<0){
		dev_err(&client->dev, "failed to transmit instructions to lis302.\n");
		return ret;
	}
	
	ret = i2c_master_recv(client, &buf, 1);
	if (ret<0){
		dev_err(&client->dev, "failed to receive response from lis302.\n");
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
		dev_err(&client->dev, "failed to transmit instructions to lis302.\n");
	
	return ret;
}

/*
 * Generic x,y,z attributes
 */
static ssize_t lis302dl_show_xyz(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct sensor_device_attribute_2 *sattr = to_sensor_dev_attr_2(attr);
	struct i2c_client *client = to_i2c_client(dev);
	struct lis302dl_data *sensor = client->dev.driver_data;
	int ret;
	char buf_tmp[3];
	
//	dev_dbg(dev, "lis302dl_show() called on %s; index:0x%x\n", attr->attr.name,sattr->index);
		
//	printk("lis302dl_show_xyz() called on %s; index:0x%x\n", attr->attr.name,sattr->index);
//	printk("xyz's address is %02x.\n", sattr->index);

	/* Read the x_outregister */
//	if(sensor->x==0 && sensor->y==0 && sensor->z==0 )
//	{
//		printk("sensor's x, y, z are 0.\n");		
	disable_irq_nosync(sensor->irq1);
		ret = lis302_i2c_read_byte(client, sattr->index);
		if (ret < 0){
			printk("Read i2c error\n");
			return -EIO;
		}
		buf_tmp[0] = ret;
		sensor->x = buf_tmp[0];
		
		/* Read the y_out register */
		ret = lis302_i2c_read_byte(client, sattr->index+2);
		if (ret < 0){
			printk("Read i2c error\n");
			return -EIO;
		}
		buf_tmp[1] = ret;
		sensor->y = buf_tmp[1];
		
		/* Read the z_out register */
		ret = lis302_i2c_read_byte(client, sattr->index+4);
		if (ret < 0){
			printk("Read i2c error\n");
			return -EIO;
		}
		buf_tmp[2] = ret;
		sensor->z = buf_tmp[2];
		enable_irq(sensor->irq1);
//	}
	
//	else
//	{
//		printk("read the sensor structure derectly.\n");

//		buf_tmp[0] = sensor->x;
//		buf_tmp[1] = sensor->y;
//		buf_tmp[2] = sensor->z;

//		printk("x = %d, y = %d, z = %d\n", sensor->x, sensor->y, sensor->z);
//	}
//	return sprintf(buf, "0x%02x::0x%02x::0x%02x\n",buf_tmp[0],buf_tmp[1],buf_tmp[2]);
	return sprintf(buf,"%c%c%c", buf_tmp[0],buf_tmp[1],buf_tmp[2]);
}

static ssize_t lis302dl_show_ctrl(struct device *dev, struct device_attribute *attr,char *buf)
{
	struct sensor_device_attribute_2 *sattr = to_sensor_dev_attr_2(attr);
	struct i2c_client *client = to_i2c_client(dev);
	int ret;
	
	printk("enter lis302dl_show.\n");

	dev_dbg(dev, "lis302dl_show() called on %s; index:0x%x\n", attr->attr.name,sattr->index);

	/* Read the first register */
	ret = lis302_i2c_read_byte(client, sattr->index);
	if (ret < 0){
		printk("Read i2c error\n");
		return -EIO;
	}
	
	return sprintf(buf, "0x%.2x\n",ret );
	
}

static ssize_t lis302dl_set_ctrl(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct sensor_device_attribute_2 *sattr = to_sensor_dev_attr_2(attr);
	struct i2c_client *client = to_i2c_client(dev);
	struct lis302dl_data *sensor = client->dev.driver_data;
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

	if(sattr->index == CTRL_REG3 && val>0)
		enable_irq(sensor->irq1);
	
	return (ret>0) ? count:ret;
}

/*
 * Simple register attributes
 */
static SENSOR_DEVICE_ATTR_2(xyz, S_IRUGO | S_IWUSR, lis302dl_show_xyz,
			    NULL, 3, OUTX_H);
static SENSOR_DEVICE_ATTR_2(x, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    NULL, 1, OUTX_H);
static SENSOR_DEVICE_ATTR_2(y, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    NULL, 1, OUTY_H);
static SENSOR_DEVICE_ATTR_2(z, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    NULL, 1, OUTZ_H);
static SENSOR_DEVICE_ATTR_2(ctrl1, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, CTRL_REG1);
static SENSOR_DEVICE_ATTR_2(ctrl2, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, CTRL_REG2);
static SENSOR_DEVICE_ATTR_2(ctrl3, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, CTRL_REG3);
static SENSOR_DEVICE_ATTR_2(status, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    NULL, 1, STATUS_REG);

static SENSOR_DEVICE_ATTR_2(ff_cfg1, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, FF_WU_CFG1);
static SENSOR_DEVICE_ATTR_2(ff_src1, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    NULL, 1, FF_WU_SRC1);
static SENSOR_DEVICE_ATTR_2(ff_ths1, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, FF_WU_THS1);
static SENSOR_DEVICE_ATTR_2(ff_duration1, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
                            lis302dl_set_ctrl, 1, FF_WU_DURATION1);

static SENSOR_DEVICE_ATTR_2(ff_cfg2, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, FF_WU_CFG2);
static SENSOR_DEVICE_ATTR_2(ff_src2, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    NULL, 1, FF_WU_SRC2);
static SENSOR_DEVICE_ATTR_2(ff_ths2, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, FF_WU_THS2);
static SENSOR_DEVICE_ATTR_2(ff_duration2, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
                            lis302dl_set_ctrl, 1, FF_WU_DURATION2);

static SENSOR_DEVICE_ATTR_2(click_cfg, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, CLICK_CFG);
static SENSOR_DEVICE_ATTR_2(click_src, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    NULL, 1, CLICK_SRC);
static SENSOR_DEVICE_ATTR_2(click_thsy_x, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, CLICK_THSY_X);
static SENSOR_DEVICE_ATTR_2(click_thsz, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, CLICK_THSZ);
static SENSOR_DEVICE_ATTR_2(click_timelimit, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, CLICK_TIMELIMIT);
static SENSOR_DEVICE_ATTR_2(click_lantency, S_IRUGO | S_IWUSR, lis302dl_show_ctrl,
			    lis302dl_set_ctrl, 1, CLICK_LANTENCY);



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
		&sensor_dev_attr_ff_cfg1.dev_attr.attr,
		&sensor_dev_attr_ff_src1.dev_attr.attr,
		&sensor_dev_attr_ff_ths1.dev_attr.attr,
		&sensor_dev_attr_ff_duration1.dev_attr.attr,
		&sensor_dev_attr_ff_cfg2.dev_attr.attr,
		&sensor_dev_attr_ff_src2.dev_attr.attr,
		&sensor_dev_attr_ff_ths2.dev_attr.attr,
		&sensor_dev_attr_ff_duration2.dev_attr.attr,
		&sensor_dev_attr_click_cfg.dev_attr.attr,
		&sensor_dev_attr_click_src.dev_attr.attr,
		&sensor_dev_attr_click_thsy_x.dev_attr.attr,
		&sensor_dev_attr_click_thsz.dev_attr.attr,
		&sensor_dev_attr_click_timelimit.dev_attr.attr,
		&sensor_dev_attr_click_lantency.dev_attr.attr,
		NULL,
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
	if (ret<0){
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

	return (ret<0) ? ret:count;
}

static void lis302dl_work_func1(struct work_struct *work)
{
	struct lis302dl_data *sensor = container_of(work, struct lis302dl_data, work1);
	int status = 0;
	

//	int ret=0;
	int gpio_state = 0;

//	buf[0] = lis302_i2c_read_byte(sensor->client,HP_FILTER_RESET);
	status = lis302_i2c_read_byte(sensor->client,STATUS_REG);
	
	sensor->x = lis302_i2c_read_byte(sensor->client,OUTX_H);
	sensor->y = lis302_i2c_read_byte(sensor->client,OUTY_H);
	sensor->z = lis302_i2c_read_byte(sensor->client,OUTZ_H);

	printk("enter work_func1\n");
	printk("x = %d, y = %d, z = %d.\n", sensor->x, sensor->y, sensor->z);	

	signed char x, y, z;
	x = sensor->x;
	y = sensor->y;
	z = sensor->z;

	printk("acc_x = %d /1000.\n", (int)(x * 18 * 10));
	printk("acc_y = %d /1000.\n", (int)(y * 18 * 10));
	printk("acc_z = %d./1000\n", (int)(z * 18 * 10));

//	printk("status_reg = 0x%.2x\n", status);
//	printk("outx_h = 0x%.2x\n", sensor->x);
//	printk("outy_h = 0x%.2x\n", sensor->y);
//	printk("outz_h = 0x%.2x\n", sensor->z);
//	printk("\n");

//	if (sensor->x < 128) printk("%d\n", sensor->x);
//	else printk("%d\n", sensor->x - 256);
//	if (sensor->y < 128) printk("%d\n", sensor->y);
//	else printk("%d\n", sensor->y - 256);
//	if (sensor->z < 128) printk("%d\n", sensor->z);
//	else printk("%d\n", sensor->z - 256);

//	printk("\n");
//	gpio_state = gpio_get_value(INT1_PIN);
//	printk("INT1_PIN=%d\n", gpio_state);
	
	printk("\n\n\n\n");
	
//	for(ret=0;ret<sizeof(buf)/sizeof(buf[0]);ret++)
//		pr_debug("buf1[%d] = 0x%02x\n",ret,buf[ret]);

	if(sensor->irq1)
		enable_irq(sensor->irq1);
}

/*
static void lis302dl_work_func2(struct work_struct *work)
{
	struct lis302dl_data *sensor = container_of(work, struct lis302dl_data, work2);
	char buf[10];
	int ret=0;

	buf[0] = lis302_i2c_read_byte(sensor->client,HP_FILTER_RESET);
	buf[1] = lis302_i2c_read_byte(sensor->client,STATUS_REG);
	buf[2] = lis302_i2c_read_byte(sensor->client,OUTX_H);
	buf[3] = lis302_i2c_read_byte(sensor->client,OUTY_H);
	buf[4] = lis302_i2c_read_byte(sensor->client,OUTZ_H);
	buf[5] = lis302_i2c_read_byte(sensor->client,CLICK_SRC);
	buf[7] = lis302_i2c_read_byte(sensor->client,FF_WU_SRC1);
	buf[8] = lis302_i2c_read_byte(sensor->client,FF_WU_THS1);
	buf[9] = lis302_i2c_read_byte(sensor->client,FF_WU_DURATION1);

	for(ret=0;ret<sizeof(buf)/sizeof(buf[0]);ret++)
		pr_debug("buf2[%d] = 0x%02x\n",ret,buf[ret]);

	sensor->x = buf[2];
	sensor->y = buf[3];
	sensor->z = buf[4];

	printk("enter work_func2.\n");
	printk("x=%d, y=%d, z=%d\n", sensor->x, sensor->y, sensor->z);
	
//	if (sensor->x > 0) printk("x event\n");
//	if (sensor->y > 0) printk("y event\n");	

	if(sensor->irq2)
		enable_irq(sensor->irq2);	
}
*/

static irqreturn_t lis302dl_irq1_handler(int irq, void *dev_id)
{
	struct lis302dl_data  *sensor = (struct lis302dl_data  *)dev_id;
	
	dev_dbg(&sensor->client->dev,"%s\n",__func__);
	
	disable_irq_nosync(irq);
	queue_work(lis302dl_wq, &sensor->work1);
	
	return IRQ_HANDLED;
}

/*
static irqreturn_t lis302dl_irq2_handler(int irq, void *dev_id)
{
	struct lis302dl_data  *sensor = (struct lis302dl_data  *)dev_id;
	
	dev_dbg(&sensor->client->dev,"%s\n",__func__);
	
	disable_irq_nosync(irq);
	queue_work(lis302dl_wq, &sensor->work2);

	return IRQ_HANDLED;
}
*/

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
	unsigned int irq_num;
	struct lis302dl_data *lis302dl;
	
	if (!i2c_check_functionality(client->adapter,I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c bus does not support the ST lis302dl\n");
		ret = -ENODEV;
		return ret;
	}

	ret = lis302_i2c_read_byte(client, WHO_AM_I);
	if(ret<0){
		pr_err("Operated I2C Fail\n");
		return ret;
	}
	else if((ret != LIS3LV02DL_ID) && (ret != LIS302DL_ID)){
		pr_err("Accelerometer chip not LIS3LV02D{L,Q}\n");
		return -ERANGE;
	}		

//	lis302_i2c_write_byte(client, CTRL_REG1, 0x67);   //8g
	lis302_i2c_write_byte(client, CTRL_REG1, 0x47);
//	lis302_i2c_write_byte(client, CTRL_REG2, 0x17);  //usr FDS
//	lis302_i2c_write_byte(client, CTRL_REG2, 0x10);  //don't use FDS  //HP_FF_WU2, HP_FF_WU1 bypass
       lis302_i2c_write_byte(client, CTRL_REG2, 0x07);  //don't use FDS, but enable HP_FF_WU1
	lis302_i2c_write_byte(client, CTRL_REG3, 0x81);  //active low, int1->ff_wu_cfg
//	lis302_i2c_write_byte(client, FF_WU_THS1, 0x05);  //350mg
	lis302_i2c_write_byte(client, FF_WU_THS1, 0x14);  //350mg
	lis302_i2c_write_byte(client,FF_WU_DURATION1,0x00);  //Duration value

//	lis302_i2c_read_byte(client,HP_FILTER_RESET);

	lis302_i2c_write_byte(client, FF_WU_CFG2, 0x00);
	lis302_i2c_write_byte(client, CLICK_CFG, 0x00);
	lis302_i2c_write_byte(client, FF_WU_CFG1, 0x2a);  //or, all high

	lis302dl = kzalloc(sizeof(*lis302dl), GFP_KERNEL);
	if (lis302dl == NULL) {
		ret = -ENOMEM;
		return ret;
	}
	
	INIT_WORK(&lis302dl->work1, lis302dl_work_func1);
//	INIT_WORK(&lis302dl->work2, lis302dl_work_func2);
	
	lis302dl->client = client;
	i2c_set_clientdata(client, lis302dl);
	
//	pin_num = S3C64XX_GPL(13);
	pin_num =INT1_PIN;
	if(gpio_is_valid(pin_num))		/*init1*/
	{
		irq_num = gpio_to_irq(pin_num);
		lis302dl->irq1 = irq_num;
		ret = gpio_request(pin_num, "GPL");
//		s3c_gpio_cfgpin(pin_num, S3C_GPIO_SFN(3));
		s3c_gpio_cfgpin(pin_num,  GPIO_FUN_INT);
		s3c_gpio_setpull(pin_num, S3C_GPIO_PULL_NONE);
		ret =request_irq(irq_num, lis302dl_irq1_handler, IRQF_TRIGGER_FALLING, client->name, lis302dl);
		if(ret<0)
			pr_err("request_irq failed\n");
	}

	
//	pin_num = S3C64XX_GPM(0);
//        pin_num =INT2_PIN;
//	if(gpio_is_valid(pin_num))		/*init2*/
//	{		
//		irq_num = gpio_to_irq(pin_num);
//		lis302dl->irq2 = irq_num;
//		ret = gpio_request(pin_num, "GPM");
//		s3c_gpio_cfgpin(pin_num, S3C_GPIO_SFN(3));
//		s3c_gpio_cfgpin(pin_num,  GPIO_FUN_INT);
//		s3c_gpio_setpull(pin_num, S3C_GPIO_PULL_NONE);
//		ret =request_irq(irq_num, lis302dl_irq2_handler, IRQF_TRIGGER_LOW, client->name, lis302dl);
//		if(ret<0)
//			pr_err("request_irq failed\n");
//	}

	
	ret = sysfs_create_group(&client->dev.kobj, &lis302dl_group);
	if (ret) return ret;
	
	ret = sysfs_create_bin_file(&client->dev.kobj, &lis302dl_sensor_attr);
	if (ret)
		sysfs_remove_group(&client->dev.kobj, &lis302dl_group);

	return ret;

}

static int __devexit lis302dl_remove(struct i2c_client *client)
{
//	gpio_free(INT1_PIN);
	struct lis302dl_data *sensor = i2c_get_clientdata(client);
	
	//free_irq and free_gpio
	kfree(sensor);

	return 0;
}

#ifdef CONFIG_PM

static int lis302dl_suspend(struct i2c_client *client, pm_message_t mesg)
{
	lis302_i2c_write_byte(client,CTRL_REG1,0x67);	//8g scale, x,yz en; power down
	lis302_i2c_write_byte(client,CTRL_REG2,0x1f);
	lis302_i2c_write_byte(client,CTRL_REG3,0x9f);	//int2->FF_WU_1 or FF_WU_2; int1->Click Interrupt; active low; push-pull
	lis302_i2c_write_byte(client,CLICK_CFG,0x00);   //disable click interrupt
	lis302_i2c_write_byte(client,FF_WU_CFG1,0x2a);  //ZH YH and XH event
	lis302_i2c_write_byte(client,FF_WU_CFG2,0x00);  //disable FF_WU_CFG2
	lis302_i2c_write_byte(client,FF_WU_THS1,0x5);   //Threshold value
	lis302_i2c_write_byte(client,FF_WU_DURATION1,0x00);  //Duration value
	return 0;
}
 // 
static int lis302dl_resume(struct i2c_client *client)
{

	lis302_i2c_write_byte(client,CTRL_REG1,0x00);	
	lis302_i2c_write_byte(client,CTRL_REG2,0x00);
	lis302_i2c_write_byte(client,CTRL_REG3,0x00);	
	lis302_i2c_write_byte(client,CLICK_CFG,0x00);  
	lis302_i2c_write_byte(client,FF_WU_CFG1,0x00);  
	lis302_i2c_write_byte(client,FF_WU_CFG2,0x00);  
	lis302_i2c_write_byte(client,FF_WU_THS1,0x00);  
	lis302_i2c_write_byte(client,FF_WU_DURATION1,0x00);	
	return 0;
}

#else

#define lis302dl_suspend		NULL
#define lis302dl_resume			NULL

#endif /* CONFIG_PM */

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
	.suspend = lis302dl_suspend,
	.resume	= lis302dl_resume,
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

