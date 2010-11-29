/*
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html

 * (C) Copyright 2006 Marvell International Ltd.
 * All Rights Reserved
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/hwmon-sysfs.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/setup.h>
#include <asm/mach/arch.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
//#include <mach/pxa-regs.h>
//#include <mach/mfp-pxa300.h>
#include <mach/gpio.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
//add by hui
#include <linux/poll.h>
#include <linux/wait.h>
#include "lis302dl_misc.h"
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

#define LIS302DL_IRQ//add by hui


/* Maximum value our axis may get in full res mode for the input device (signed 13 bits) */ 
#define ADXL_FULLRES_MAX_VAL 4096 
          
/* Maximum value our axis may get in fixed res mode for the input device (signed 10 bits) */ 
#define ADXL_FIXEDRES_MAX_VAL 512 

#define MFPR0_GPIO127  0x40E10670
#define MFPR0_GPIO105  0x40E10618

//Three Axis Acceleration Flag
#define		X_CHANNEL		0		
#define		Y_CHANNEL		1
#define		Z_CHANNEL		2

#define	P_0_25G			4160	//4096+0.25*256=4160
#define	N_0_25G			4032	//4096-0.25*256=4032

#define	P_0_75G			4288	//4096+0.75*256=4288
#define	N_0_75G			3904	//4096-0.75*256=3904

//#define LIS302DL_DEBUG

//add by hui
#ifdef LIS302DL_IRQ
static int rtc_has_irq = 1;
#endif
static unsigned long rtc_irq_data;	/* our output to the world	*/

static int suspend = 0;

static DECLARE_WAIT_QUEUE_HEAD(lis302dl_wait);//add by hui

/**************************************************************
声明函数
**************************************************************/
static int lis302dl_open(struct inode *inode, struct file *file);
static int lis302dl_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
static int lis302dl_probe(struct i2c_client *client, const struct i2c_device_id *id);
static unsigned int lis302dl_poll(struct file *file, poll_table * wait);//add by hui
//add by hui
static irqreturn_t lis302dl_irq1_handler(int irq, void *dev_id);
static irqreturn_t lis302dl_irq2_handler(int irq, void *dev_id);
static void lis302dl_work_func1(struct work_struct *work);
static void lis302dl_work_func2(struct work_struct *work);




static struct input_dev  * lis302dl_input_dev = NULL;
static struct i2c_client * lis302dl_client = NULL;

//static short int		Acceleration[3] = {0,0,0};	//Three Axis Acceleration Value
static char		Acceleration[3] = {0,0,0};	//Three Axis Acceleration Value	//add by hui


static const struct i2c_device_id lis302dl_id[] = {
	{ "st-lis302dl", 0 },
	{ }
};

static const struct file_operations lis302dl_fops = {
	.owner = THIS_MODULE,
	.open = lis302dl_open,
	.read = lis302dl_read,
	//.write = lis302dl_write,
	//.release = lis302dl_close,
	//.ioctl = lis302dl_ioctl,
	.poll = lis302dl_poll,
};

static struct miscdevice lis302dl_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "lis302dl",
	.fops = &lis302dl_fops,
};

void LIS302DL_I2C_Write_Data(char Reg,char Reg_Data)
{
	int ret;
	char data[2]={ Reg, Reg_Data };
	
	//printk(KERN_INFO"\n1:%d,2:%x,3:%x\n",data[0],data[1],data[2]);
	
	ret = i2c_master_send(lis302dl_client,data,2);
	if(ret!=2)
	printk(KERN_INFO"write data ret=%d",ret);

}

void LIS302DL_I2C_Read_Data(char Reg,char count,char *data)
{
	int ret;

	ret = i2c_master_send(lis302dl_client, &Reg, 1); 
	if(ret!=1)
	{
		printk(KERN_INFO"read-send-reg-ret=%d ,",ret);
		return ; 
	}
		
	ret = i2c_master_recv(lis302dl_client, data, count);	
	if(ret!=count)
	{
		printk(KERN_INFO"read-recv-ret=%d  \n",ret);
		return ;
	}
	
}


static int lis302dl_remove(struct i2c_client *client)
{
	//int irq;
	int ret;	

	//irq = IRQ_GPIO(127);
	//printk(KERN_INFO"irq:%d",irq);
	//free_irq(irq,lis302dl_client);

	misc_deregister(&lis302dl_device);

	if ((ret = i2c_detach_client(lis302dl_client)))
		return ret;

	kfree(i2c_get_clientdata(lis302dl_client));

//add by hui
	struct lis302dl_data *sensor = i2c_get_clientdata(client);
	kfree(sensor);
	
	return 0;
	
}

#ifdef CONFIG_PM

static int lis302dl_suspend(struct i2c_client *client, pm_message_t mesg)
{
	suspend = 1;
	
	return 0;
}

static int lis302dl_resume(struct i2c_client *client)
{
	suspend = 0;
	
	return 0;
}

#else

#define lis302dl_suspend		NULL
#define lis302dl_resume			NULL

#endif   /* CONFIG_PM */

//lis302dl
static struct i2c_driver lis302dl_driver = 
{
	.driver = 
	{
		.name	= "lis302dl_misc",
	},
	.probe		= lis302dl_probe,
	.remove		= lis302dl_remove,
	.suspend	= lis302dl_suspend,
	.resume		= lis302dl_resume,
	.id_table	= lis302dl_id,
	
};

static int read_xyz()
{

	char buf[6] = {0x00};	
	unsigned	char	DataX_High, DataX_Low;
	unsigned	char	DataY_High, DataY_Low;		//High Byte and Low Byte of Data Register for Y
	unsigned	char	DataZ_High, DataZ_Low;		//High Byte and Low Byte of Data Register for Z

	LIS302DL_I2C_Read_Data(OUTX_H,1,buf);
	Acceleration[X_CHANNEL] = buf[0];
	LIS302DL_I2C_Read_Data(OUTY_H,1,buf);
	Acceleration[Y_CHANNEL] = buf[0];
	LIS302DL_I2C_Read_Data(OUTZ_H,1,buf);
	Acceleration[Z_CHANNEL] = buf[0];

	return 0;
}


static irqreturn_t stylus_action(int irq, void *dev_id)
{

	//int ret;
	int INT_SOURCE;
	char buf[6] = {0x00};
	
	unsigned	char	DataX_High, DataX_Low;
	unsigned	char	DataY_High, DataY_Low;		//High Byte and Low Byte of Data Register for Y
	unsigned	char	DataZ_High, DataZ_Low;		//High Byte and Low Byte of Data Register for Z
	
	//printk(KERN_INFO"ADXL-stulus_action,irq=%d\n",irq);

	disable_irq(irq);
	
//	LIS302DL_I2C_Read_Data(0x30,1,&INT_SOURCE);//marked by hui
	//printk(KERN_INFO"reg:0x30 (INT_SOURCE)=%x\n",INT_SOURCE);
	
//	LIS302DL_I2C_Read_Data(0x32,6,buf);
	
	/*

	if(INT_SOURCE&0x08)//横竖屏翻转功能
	{
		DataX_Low = buf[0];
		DataX_High = buf[1]&0x1F;
		DataY_Low = buf[2];
		DataY_High = buf[3]&0x1F;
		DataZ_Low = buf[4];
		DataZ_High = buf[5]&0x1F;
	
		Acceleration[X_CHANNEL] = DataX_High;
		Acceleration[X_CHANNEL] = (Acceleration[X_CHANNEL]<<8) | DataX_Low;
		Acceleration[Y_CHANNEL] = DataY_High;
		Acceleration[Y_CHANNEL] = (Acceleration[Y_CHANNEL]<<8) | DataY_Low;
	
		Acceleration[Z_CHANNEL] = DataZ_High;
		Acceleration[Z_CHANNEL] = (Acceleration[Z_CHANNEL]<<8) | DataZ_Low;	
					
		printk(KERN_INFO"roll(%d,%d,%d)",Acceleration[X_CHANNEL],Acceleration[Y_CHANNEL],Acceleration[Z_CHANNEL]);
		
		//Translate the twos complement to true binary code	(256+4096)->1g, (4096-256)->(-1)g										  
		for(iTemp=X_CHANNEL;iTemp<=Z_CHANNEL;iTemp++)
		{
			if(Acceleration[iTemp] < 4096)
			{
				Acceleration[iTemp] = Acceleration[iTemp] + 4096;	
			}
			else if(Acceleration[iTemp] >= 4096)
			{
				Acceleration[iTemp] = Acceleration[iTemp] - 4096;	
			}
		}
		if(Acceleration[X_CHANNEL] > P_0_75G)
		{
			printk(KERN_INFO"Right\n");			  //Right
			input_report_abs(lis302dl_input_dev, ABS_RX, 100);
		}
		else if(Acceleration[X_CHANNEL] < N_0_75G)
		{
			printk(KERN_INFO"Left\n");				//Left
			input_report_abs(lis302dl_input_dev, ABS_RX, -100);
		}
		else if(Acceleration[Y_CHANNEL] > P_0_75G)
		{
			printk(KERN_INFO"Down\n");				//Down
			input_report_abs(lis302dl_input_dev, ABS_RY, 100);
		}
		else if(Acceleration[Y_CHANNEL] < N_0_75G)
		{
			printk(KERN_INFO"Up\n");				//Up
			input_report_abs(lis302dl_input_dev, ABS_RY, -100);
		}
		else if(Acceleration[Z_CHANNEL] > P_0_75G)
		{
			printk(KERN_INFO"Top\n");				//Top
			input_report_abs(lis302dl_input_dev, ABS_RZ, -100);
		}
		else if(Acceleration[Z_CHANNEL] < N_0_75G)
		{
			printk(KERN_INFO"Bottom\n");	    	//Bottom
			input_report_abs(lis302dl_input_dev, ABS_RZ, 100);
		}
	}	
	else if(INT_SOURCE&0x10) //晃动功能
	{
		DataX_Low = buf[0];
		DataX_High = buf[1]&0x1F;
	
		Acceleration[X_CHANNEL] = DataX_High;
		Acceleration[X_CHANNEL] = (Acceleration[X_CHANNEL]<<8) | DataX_Low;
		
		DataX_Low = buf[0];
		DataX_High = buf[1]&0x1F;
		DataY_Low = buf[2];
		DataY_High = buf[3]&0x1F;
		DataZ_Low = buf[4];
		DataZ_High = buf[5]&0x1F;
	
		Acceleration[X_CHANNEL] = DataX_High;
		Acceleration[X_CHANNEL] = (Acceleration[X_CHANNEL]<<8) | DataX_Low;
		Acceleration[Y_CHANNEL] = DataY_High;
		Acceleration[Y_CHANNEL] = (Acceleration[Y_CHANNEL]<<8) | DataY_Low;
	
		Acceleration[Z_CHANNEL] = DataZ_High;
		Acceleration[Z_CHANNEL] = (Acceleration[Z_CHANNEL]<<8) | DataZ_Low;	
					
		printk(KERN_INFO"huangdong(%d,%d,%d)\n",Acceleration[X_CHANNEL],Acceleration[Y_CHANNEL],Acceleration[Z_CHANNEL]);
		
		if(Acceleration[X_CHANNEL] < 4096)
		{
			Acceleration[X_CHANNEL] = Acceleration[X_CHANNEL] + 4096;	
		}
		else
		{
			Acceleration[X_CHANNEL] = Acceleration[X_CHANNEL] - 4096;	
		}
		
		if(Acceleration[X_CHANNEL] < N_0_25G)
		{
				printk(KERN_INFO"N_0_25G\n");//左侧
				input_report_abs(lis302dl_input_dev, ABS_X, -100);
		}
		else if(Acceleration[X_CHANNEL] > P_0_25G)
		{
			printk(KERN_INFO"P_0_25G\n");//右侧
			input_report_abs(lis302dl_input_dev, ABS_X, 100);
		}
	}
	else //自由落体功能
	{
		printk(KERN_INFO"free-fall\n");
	}
	
	*/
	
	//printk(KERN_INFO"(%d,%d,%d,%d,%d,%d)\n",read_data[0],read_data[1],
	//		read_data[2],read_data[3],read_data[4],read_data[5]);



	LIS302DL_I2C_Read_Data(OUTX_H,1,buf);
	Acceleration[X_CHANNEL] = buf[0];
	LIS302DL_I2C_Read_Data(OUTY_H,1,buf);
	Acceleration[Y_CHANNEL] = buf[0];
	LIS302DL_I2C_Read_Data(OUTZ_H,1,buf);
	Acceleration[Z_CHANNEL] = buf[0];

	
	//input_report_abs(lis302dl_input_dev, ABS_X, Acceleration[X_CHANNEL]);
 	//input_report_abs(lis302dl_input_dev, ABS_Y, Acceleration[Y_CHANNEL]);
 	//input_report_abs(lis302dl_input_dev, ABS_Z, Acceleration[Z_CHANNEL]);
	
 	//input_sync(lis302dl_input_dev);
	
	enable_irq(irq);

	return IRQ_HANDLED;
}

void lis302dl_misc_initialize()
{

	char data;

	LIS302DL_I2C_Write_Data( CTRL_REG1, 0x47);
    LIS302DL_I2C_Write_Data( CTRL_REG2, 0x07);  //don't use FDS, but enable HP_FF_WU1
	LIS302DL_I2C_Write_Data( CTRL_REG3, 0x81);  //active low, int1->ff_wu_cfg
	LIS302DL_I2C_Write_Data( FF_WU_THS1, 0x08);  //350mg//感应灵敏度
	LIS302DL_I2C_Write_Data(FF_WU_DURATION1,0x00);  //Duration value
	LIS302DL_I2C_Write_Data( FF_WU_CFG2, 0x00);
	LIS302DL_I2C_Write_Data( CLICK_CFG, 0x00);
	LIS302DL_I2C_Write_Data( FF_WU_CFG1, 0x2a);  //or, all high
	
//	udelay(100);
//	LIS302DL_I2C_Read_Data(0x30,1,&data);
//	printk(KERN_INFO"0x30:%x\n",data);

}

typedef struct  {
//		short x, /**< holds x-axis acceleration data sign extended. Range -512 to 511. */
		char x, /**< holds x-axis acceleration data sign extended. Range -512 to 511. */	//modified by hui
			  y, /**< holds y-axis acceleration data sign extended. Range -512 to 511. */
			  z, /**< holds z-axis acceleration data sign extended. Range -512 to 511. */
			  zero;//add by hui
} accel;

/*	open command for LIS302DL device file	*/
static int lis302dl_open(struct inode *inode, struct file *file)
{
//	printk(KERN_INFO"lis302dl_open:test\n");
#ifdef LIS302DL_DEBUG
		printk(KERN_INFO "%s\n",__FUNCTION__); 
#endif

	if( lis302dl_client == NULL)
	{
		printk(KERN_INFO "I2C driver not install\n"); 
		return -1;
	}


#ifdef LIS302DL_DEBUG
	printk(KERN_INFO "Analog has been opened\n");
#endif
	return 0;
}


/*	read command for lis302dl device file	*/
static int lis302dl_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{	
#ifndef LIS302DL_IRQ
	return -EIO;
#else
	DECLARE_WAITQUEUE(wait, current);
	unsigned long data;
	ssize_t retval;

	if (rtc_has_irq == 0)
		return -EIO;

	/*
	 * Historically this function used to assume that sizeof(unsigned long)
	 * is the same in userspace and kernelspace.  This lead to problems
	 * for configurations with multiple ABIs such a the MIPS o32 and 64
	 * ABIs supported on the same kernel.  So now we support read of both
	 * 4 and 8 bytes and assume that's the sizeof(unsigned long) in the
	 * userspace ABI.
	 */
	if (count >=  sizeof(unsigned long))
		return -EINVAL;

	add_wait_queue(&lis302dl_wait, &wait);

	do {
		/* First make it right. Then make it fast. Putting this whole
		 * block within the parentheses of a while would be too
		 * confusing. And no, xchg() is not the answer. */

		__set_current_state(TASK_INTERRUPTIBLE);

//		spin_lock_irq(&rtc_lock);
		data = rtc_irq_data;
		rtc_irq_data = 0;
//		spin_unlock_irq(&rtc_lock);

		if (data != 0)
			break;

		if (file->f_flags & O_NONBLOCK) {
			retval = -EAGAIN;
			goto out;
		}
		if (signal_pending(current)) {
			retval = -ERESTARTSYS;
			goto out;
		}
		schedule();
	} while (1);

	if (count == sizeof(unsigned int)) {
		retval = put_user(data,
				  (unsigned int __user *)buf) ?: sizeof(int);
	} else {
		retval = put_user(data,
				  (unsigned long __user *)buf) ?: sizeof(long);
	}
	if (!retval)
		retval = count;
 out:
	__set_current_state(TASK_RUNNING);
	remove_wait_queue(&lis302dl_wait, &wait);

	return retval;
#endif

}


static int lis302dl_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	int ret = 0;
	
	char data;
	
	int irq;

	struct input_dev *input_dev;
	
	void __iomem *p;
	
	char read_data[6];

//add by hui
	int pin_num;
	unsigned int irq_num;
	struct lis302dl_data *lis302dl;
	
	printk(KERN_INFO"lis302dl_probe\n");
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

	lis302dl_client = client;

	LIS302DL_I2C_Read_Data(0x0F,1,&data);
	printk(KERN_INFO"devid-data:%x\n",data);

	lis302dl_misc_initialize();

//add by hui
	lis302dl = kzalloc(sizeof(*lis302dl), GFP_KERNEL);
	if (lis302dl == NULL) {
		ret = -ENOMEM;
		return ret;
	}

	INIT_WORK(&lis302dl->work1, lis302dl_work_func1);
	INIT_WORK(&lis302dl->work2, lis302dl_work_func2);

	lis302dl->client = client;
	i2c_set_clientdata(client, lis302dl);

	 pin_num =INT1_PIN;
	if(gpio_is_valid(pin_num))		/*init1*/
	{
		irq_num = gpio_to_irq(pin_num);
		lis302dl->irq1 = irq_num;
		ret = gpio_request(pin_num, "GPL");
//		s3c_gpio_cfgpin(pin_num, S3C_GPIO_SFN(3));
		s3c_gpio_cfgpin(pin_num,  GPIO_FUN_INT);
		s3c_gpio_setpull(pin_num, S3C_GPIO_PULL_NONE);
		ret =request_irq(irq_num, lis302dl_irq1_handler, IRQF_TRIGGER_LOW, client->name, lis302dl);
		if(ret<0)
			pr_err("request_irq failed\n");			
	}
	
//	pin_num = S3C64XX_GPM(0);
        pin_num =INT2_PIN;
	if(gpio_is_valid(pin_num))		/*init2*/
	{		
		irq_num = gpio_to_irq(pin_num);
		lis302dl->irq2 = irq_num;
		ret = gpio_request(pin_num, "GPM");
//		s3c_gpio_cfgpin(pin_num, S3C_GPIO_SFN(3));
		s3c_gpio_cfgpin(pin_num,  GPIO_FUN_INT);
		s3c_gpio_setpull(pin_num, S3C_GPIO_PULL_NONE);
		ret =request_irq(irq_num, lis302dl_irq2_handler, IRQF_TRIGGER_LOW, client->name, lis302dl);
		if(ret<0)
			pr_err("request_irq failed\n");
	}

//add end	

#if 0
	irq = gpio_to_irq(127);
	gpio_direction_input(127);
	
	ret = request_irq(irq, stylus_action, IRQF_TRIGGER_RISING/*|IRQF_TRIGGER_FALLING*/, "lis302dl_misc", lis302dl_client);
	
	if (ret)
	{	
		printk(KERN_ALERT"request_irq IRQ_GPIO_127 fail %d\n",ret);
		goto exit_kfree;
	}
#endif	


	strlcpy(lis302dl_client->name, "lis302dl_misc", I2C_NAME_SIZE);
	
	ret = misc_register(&lis302dl_device);
	if (ret) {
		printk(KERN_ERR "st lis302dl_misc accelerometer device register failed\n");
		//goto err_free_irq;
		goto exit_kfree;
	}
	printk(KERN_INFO "st lis302dl_misc accelerometer create ok\n");
	
//	LIS302DL_I2C_Read_Data(0x32,1,&data);	//marked by hui
	
	return 0;

//err_free_irq: 
//	free_irq(irq, lis302dl_client); 
exit_kfree:
	kfree(lis302dl_client);
	
	return ret;
}

static unsigned int lis302dl_poll(struct file *file, poll_table * wait)
{
	unsigned long l;

	if (rtc_has_irq == 0)
		return 0;

	poll_wait(file, &lis302dl_wait, wait);//add by hui

	l = rtc_irq_data;

	if (l != 0){
		//printk("return POLLIN | POLLRDNORM;\n");
		return POLLIN | POLLRDNORM;
		}
	return 0;

}
/**
 *	support irq read data.	//add by hui
 */
 
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
static void lis302dl_work_func1(struct work_struct *work)
{
	struct lis302dl_data *sensor = container_of(work, struct lis302dl_data, work1);

#if 1	//marked by hui 10-03-19
//	struct sensor_device_attribute_2 *sattr = to_sensor_dev_attr_2(attr);
//	struct i2c_client *client = to_i2c_client(dev);
//	struct lis302dl_data *sensor = client->dev.driver_data;
	int ret;
	char buf_tmp[3];
	long ltmp0,ltmp2;

	if (suspend)
		goto error;

	ret = lis302_i2c_read_byte(sensor->client, OUTX_H);
	if (ret < 0){
		printk("Read i2c error\n");
		goto error;
	}
	buf_tmp[0] = ret;
	ltmp0 = sensor->x = buf_tmp[0];
	ltmp2 |= (ltmp0 << 0);

	/* Read the y_out register */
	ret = lis302_i2c_read_byte(sensor->client, OUTY_H);
	if (ret < 0){
		printk("Read i2c error\n");
		goto error;
	}
	buf_tmp[1] = ret;
	ltmp0 = sensor->y = buf_tmp[1];
	ltmp2 |= (ltmp0 << 8);

	/* Read the z_out register */
	ret = lis302_i2c_read_byte(sensor->client, OUTZ_H);
	if (ret < 0){
		printk("Read i2c error\n");
		goto error;
	}
	buf_tmp[2] = ret;
	ltmp0 = sensor->z = buf_tmp[2];
	ltmp2 |= (ltmp0 << 16);

	rtc_irq_data = ltmp2;
error:
	wake_up_interruptible(&lis302dl_wait);
	//printk( "%s  X:%03d Y:%03d Z:%03d\n",__FUNCTION__,sensor->x,sensor->y,sensor->z);
	//printk( "%s %08x X:%02x Y:%02x Z:%02x\n",__FUNCTION__,rtc_irq_data,sensor->x,sensor->y,sensor->z);
#endif

	if(sensor->irq1)
		enable_irq(sensor->irq1);
}

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

//printk( "%s  %02x::%02x  X:%03d Y:%03d Z:%03d %02x::%02x::%02x::%02x::%02x\n",__FUNCTION__,buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9]);


	if(sensor->irq2)
		enable_irq(sensor->irq2);	
}

static irqreturn_t lis302dl_irq1_handler(int irq, void *dev_id)
{
	struct lis302dl_data  *sensor = (struct lis302dl_data  *)dev_id;
	
	dev_dbg(&sensor->client->dev,"%s\n",__func__);
	
	disable_irq_nosync(irq);
	queue_work(lis302dl_wq, &sensor->work1);
	
	return IRQ_HANDLED;
}

static irqreturn_t lis302dl_irq2_handler(int irq, void *dev_id)
{
	struct lis302dl_data  *sensor = (struct lis302dl_data  *)dev_id;
	
	dev_dbg(&sensor->client->dev,"%s\n",__func__);
	
	disable_irq_nosync(irq);
	queue_work(lis302dl_wq, &sensor->work2);

	return IRQ_HANDLED;
}

/*hui add end*/

static int __init lis302dl_init(void)
{
	printk(KERN_INFO"lis302dl_init\n");
	
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

MODULE_AUTHOR("JinningYang<yangjinning@bbktel.com.cn>");
MODULE_DESCRIPTION("gravity sensor,i2c interface");
MODULE_LICENSE("GPL");

module_init(lis302dl_init);
module_exit(lis302dl_exit);

