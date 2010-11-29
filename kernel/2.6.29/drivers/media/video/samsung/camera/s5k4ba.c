/*
 *  Copyright (C) 2004 Samsung Electronics
 *             SW.LEE <hitchcar@samsung.com>
 *            - based on Russell King : pcf8583.c
 * 	      - added  smdk24a0, smdk2440
 *            - added  poseidon (s3c24a0+wavecom)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Driver for FIMC2.x Camera Decoder
 *
 */

//#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <plat/power-clock-domain.h>
#include <plat/pm.h>

//#define CAMIF_DEBUG

#include "s3c_camif.h"
#include "s5k4ba.h"

static struct i2c_driver s5k4ba_driver;

static void s5k4ba_sensor_gpio_init(void);
static void s5k4ba_sensor_enable(void);
static void s5k4ba_sensor_disable(void);

static int s5k4ba_sensor_init(void);
static void s5k4ba_sensor_exit(void);

static int s5k4ba_sensor_change_size(struct i2c_client *client, int size);

#ifdef CONFIG_FLASH_AAT1271A
extern int aat1271a_flash_init(void);
extern void aat1271a_flash_exit(void);
extern void aat1271a_falsh_camera_control(int ctrl);
extern void aat1271a_falsh_movie_control(int ctrl);
#endif


/* 
 * MCLK: 24MHz, PCLK: 54MHz
 * 
 * In case of PCLK 54MHz
 *
 * Preview Mode (800 * 600) 
 * 
 * Capture Mode  (800 * 600) 
 * 
 * Camcorder Mode
 */
static camif_cis_t s5k4ba_data = {
	itu_fmt:       CAMIF_ITU601,
	order422:      CAMIF_CBYCRY,
	camclk:        44000000,	// for 20 fps: 44MHz, for 12 fps(more stable): 26MHz
	source_x:      800,
	source_y:      600,
	win_hor_ofst:  0,
	win_ver_ofst:  0,
	win_hor_ofst2: 0,
	win_ver_ofst2: 0,
	polarity_pclk: 0,
	polarity_vsync:1,
	polarity_href: 0,
	reset_type:CAMIF_EX_RESET_AL,
	reset_udelay: 5000,
};

#define S5K4BA_ID 0x5a

static unsigned short s5k4ba_normal_i2c[] = { (S5K4BA_ID >> 1), I2C_CLIENT_END };
static unsigned short s5k4ba_ignore[] = { I2C_CLIENT_END };
static unsigned short s5k4ba_probe[] = { I2C_CLIENT_END };

static struct i2c_client_address_data s5k4ba_addr_data = {
	.normal_i2c = s5k4ba_normal_i2c,
	.ignore     = s5k4ba_ignore,
	.probe      = s5k4ba_probe,
};

unsigned char s5k4ba_sensor_read(struct i2c_client *client, unsigned short subaddr)
{
	int ret;
	unsigned char buf[1];
	struct i2c_msg msg = { client->addr, 0, 1, buf };
	buf[0] = subaddr;

	ret = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
	if (ret == -EIO) {
		printk(" I2C write Error \n");
		return -EIO;
	}

	msg.flags = I2C_M_RD;
	ret = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;

	return buf[0];
}

static int s5k4ba_sensor_write(struct i2c_client *client,
	     unsigned char subaddr, unsigned char val)
{
	unsigned char buf[2];
	struct i2c_msg msg = { client->addr, 0, 2, buf };

	buf[0] = subaddr;
	buf[1] = val;

	return i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
}

static void s5k4ba_sensor_gpio_init(void)
{
}

static void s5k4ba_sensor_enable(void)
{
	s5k4ba_sensor_gpio_init();

	clk_enable(cam_clock);
	clk_enable(cam_hclk);
}

static void s5k4ba_sensor_disable(void)
{
	/* MCLK Disable */
	clk_disable(cam_clock);
	clk_disable(cam_hclk);
}

static void sensor_init(struct i2c_client *sam_client)
{
	unsigned int i;
	
	unsigned int num_of_regs = S5K4BA_INIT_REGS;

	for (i = 0; i < num_of_regs; i++) {
		s5k4ba_sensor_write(sam_client,
			     s5k4ba_reg[i].subaddr, s5k4ba_reg[i].value);
	}
}

static int s5k4ba_attach(struct i2c_adapter *adap, int addr, int kind)
{
	struct i2c_client *c;

	c = kmalloc(sizeof(*c), GFP_KERNEL);
	if (!c)
		return -ENOMEM;

	memset(c, 0, sizeof(struct i2c_client));	

	strcpy(c->name, "S5K4BA");
	c->addr = addr;
	c->adapter = adap;
	c->driver = &s5k4ba_driver;
	s5k4ba_data.sensor = c;

	printk("s5k4ba attached successfully\n");

	return i2c_attach_client(c);
}

static int s5k4ba_sensor_attach_adapter(struct i2c_adapter *adap)
{
	return i2c_probe(adap, &s5k4ba_addr_data, s5k4ba_attach);
}

static int s5k4ba_sensor_detach(struct i2c_client *client)
{
	i2c_detach_client(client);
	return 0;
}

static int s5k4ba_sensor_mode_set(struct i2c_client *client, int type)
{
	int i;
	return 0;
	
	if (type & SENSOR_PREVIEW)
	{
		s5k4ba_sensor_write(client, s5k4ba_reg_qsvga[i].subaddr, s5k4ba_reg_qsvga[i].value);
	}
	else if (type & SENSOR_CAPTURE)
	{
		s5k4ba_sensor_write(client, s5k4ba_reg_svga[i].subaddr, s5k4ba_reg_svga[i].value);
	}
	else if (type & SENSOR_CAMCORDER )
	{
		s5k4ba_sensor_write(client, s5k4ba_reg_svga[i].subaddr, s5k4ba_reg_svga[i].value);
	}

	return 0;
}

static int s5k4ba_sensor_change_size(struct i2c_client *client, int size)
{
	unsigned int i = 0;
	unsigned int num_of_regs  = 0;

	switch (size) {
		case SENSOR_QSVGA:

			num_of_regs = S5K4BA_QSVGA_REGS;
			
			for (i = 0; i < num_of_regs; i++)
				s5k4ba_sensor_write(client, s5k4ba_reg_qsvga[i].subaddr, s5k4ba_reg_qsvga[i].value);
			break;

		case SENSOR_SVGA:

			num_of_regs = S5K4BA_SVGA_REGS;

 			for (i = 0; i < num_of_regs; i++)
				s5k4ba_sensor_write(client, s5k4ba_reg_svga[i].subaddr, s5k4ba_reg_svga[i].value);
			break;
		default:
			printk("Unknown Size! (Only QSVGA & SVGA)\n");
			break;
	}

	return 0;
}

static int s5k4ba_sensor_af_control(struct i2c_client *client, int type)
{
	return 0;
}

static int s5k4ba_sensor_change_effect(struct i2c_client *client, int type)
{
	return 0;
}

static int s5k4ba_sensor_change_br(struct i2c_client *client, int type)
{
	return 0;
}

static int s5k4ba_sensor_change_wb(struct i2c_client *client, int type)
{
	return 0;	// temp
    printk("[ *** Page 0, 4XA Sensor White Balance Mode ***]\n");

#if defined(CONFIG_VIDEO_SAMSUNG_S5K4BA)
       s5k4ba_sensor_write(client, 0xFC, 0x0);
       s5k4ba_sensor_write(client, 0x30, type);
#endif

       switch(type){
           case 0:
           default:
                printk(" -> AWB auto mode ]\n");
                break;
           case 1:
                printk(" -> Indoor 3100 mode ]\n");
                break;
           case 2:
                printk(" -> Outdoor 5100 mode ]\n");
                break;
           case 3:
                printk(" -> Indoor 2000 mode ]\n");
                break;
           case 4:
                printk(" -> AE/AWB halt ]\n");
                break;
           case 5:
                printk(" -> Cloudy(6000) mode ]\n");
                break;
           case 6:
                printk(" -> Sunny(8000) mode ]\n");
                break;
       }

       return 0;
}


static int s5k4ba_sensor_change_contrast(struct i2c_client *client, int type)
{
	return 0;
}

static int s5k4ba_sensor_change_saturation(struct i2c_client *client, int type)
{
	return 0;
}

static int s5k4ba_sensor_change_sharpness(struct i2c_client *client, int type)
{
	return 0;
}

static int s5k4ba_sensor_change_iso(struct i2c_client *client, int type)
{
	return 0;
}

static int s5k4ba_sensor_change_photometry(struct i2c_client *client, int type)
{
	return 0;
}

static int s5k4ba_sensor_change_scene_mode(struct i2c_client *client, int type)
{
	return 0;
}

static int s5k4ba_sensor_exif_read(void)
{
	return 0;
}

static int s5k4ba_sensor_command(struct i2c_client *client, unsigned int cmd, void *arg)
{
	struct v4l2_control *ctrl;
	int ret=0;

	switch (cmd) {
	case SENSOR_INIT:
		sensor_init(client);
		printk(KERN_INFO "External Camera initialized\n");
		break;

	case USER_ADD:
		break;

	case USER_EXIT:
		s5k4ba_sensor_exit();
		break;

	case SENSOR_EFFECT:
		ctrl = (struct v4l2_control *)arg;
		s5k4ba_sensor_change_effect(client, ctrl->value);
		break;

	case SENSOR_BRIGHTNESS:
		ctrl = (struct v4l2_control *)arg;
		s5k4ba_sensor_change_br(client, ctrl->value);
		break;

	case SENSOR_WB:
		ctrl = (struct v4l2_control *)arg;
		s5k4ba_sensor_change_wb(client, ctrl->value);
		break;

	case SENSOR_SCENE_MODE:
		ctrl = (struct v4l2_control *)arg;
		s5k4ba_sensor_change_scene_mode(client, ctrl->value);
		break;

	case SENSOR_PHOTOMETRY:
		ctrl = (struct v4l2_control *)arg;
		s5k4ba_sensor_change_photometry(client, ctrl->value);
		break;

	case SENSOR_ISO:
		ctrl = (struct v4l2_control *)arg;
		s5k4ba_sensor_change_iso(client, ctrl->value);
		break;

	case SENSOR_CONTRAST:
		ctrl = (struct v4l2_control *)arg;
		s5k4ba_sensor_change_contrast(client, ctrl->value);
		break;

	case SENSOR_SATURATION:
		ctrl = (struct v4l2_control *)arg;
		s5k4ba_sensor_change_saturation(client, ctrl->value);
		break;

	case SENSOR_SHARPNESS:
		ctrl = (struct v4l2_control *)arg;
		s5k4ba_sensor_change_sharpness(client, ctrl->value);
		break;

	case SENSOR_AF:
		ctrl = (struct v4l2_control *)arg;
		ret = s5k4ba_sensor_af_control(client, ctrl->value);
		break;

	case SENSOR_MODE_SET:
		ctrl = (struct v4l2_control *)arg;
		s5k4ba_sensor_mode_set(client, ctrl->value);
		break;

	case SENSOR_XGA:
		s5k4ba_sensor_change_size(client, SENSOR_XGA);	
		break;
	
	case SENSOR_QXGA:
		s5k4ba_sensor_change_size(client, SENSOR_QXGA);	
		break;

	case SENSOR_QSVGA:
		s5k4ba_sensor_change_size(client, SENSOR_QSVGA);
		break;

	case SENSOR_VGA:
		s5k4ba_sensor_change_size(client, SENSOR_VGA);
		break;

	case SENSOR_SVGA:
		s5k4ba_sensor_change_size(client, SENSOR_SVGA);
		break;

	case SENSOR_SXGA:
		s5k4ba_sensor_change_size(client, SENSOR_SXGA);
		break;

	case SENSOR_UXGA:
		s5k4ba_sensor_change_size(client, SENSOR_UXGA);
		break;

	case SENSOR_USER_WRITE:
		break;

	case SENSOR_USER_READ:
		break;
			
	case SENSOR_FLASH_CAMERA:
		ctrl = (struct v4l2_control *)arg;
#ifdef CONFIG_FLASH_AAT1271A
		aat1271a_falsh_camera_control(ctrl->value);	
#endif			
		break;

	case SENSOR_FLASH_MOVIE:
		ctrl = (struct v4l2_control *)arg;
#ifdef CONFIG_FLASH_AAT1271A
		aat1271a_falsh_movie_control(ctrl->value);	
#endif
			break;

	case SENSOR_EXIF_DATA:
		break;

	default:
		printk("4xa_sensor.c : Unexpect Sensor Command \n");
		break;
	}

	return ret;
}

static struct i2c_driver s5k4ba_driver = {
	.driver = {
		.name = "s5k4xa",
	},
	.id = I2C_DRIVERID_S5K_4XA,
	.attach_adapter = s5k4ba_sensor_attach_adapter,
	.detach_client = s5k4ba_sensor_detach,
	.command = s5k4ba_sensor_command
};

static int s5k4ba_sensor_init(void)
{
	int ret;

	s5k4ba_sensor_enable();
	
	s3c_camif_open_sensor(&s5k4ba_data);

	if (s5k4ba_data.sensor == NULL)
		if ((ret = i2c_add_driver(&s5k4ba_driver)))
			return ret;

	if (s5k4ba_data.sensor == NULL) {
		i2c_del_driver(&s5k4ba_driver);	
		return -ENODEV;
	}

	s3c_camif_register_sensor(&s5k4ba_data);

	return 0;
}

static void s5k4ba_sensor_exit(void)
{
	s5k4ba_sensor_disable();
	
	if (s5k4ba_data.sensor != NULL)
		s3c_camif_unregister_sensor(&s5k4ba_data);
}

static struct v4l2_input s5k4ba_input = {
	.index		= 0,
	.name		= "Camera Input (S5K4BA)",
	.type		= V4L2_INPUT_TYPE_CAMERA,
	.audioset	= 1,
	.tuner		= 0,
	.std		= V4L2_STD_PAL_BG | V4L2_STD_NTSC_M,
	.status		= 0,
};

static struct v4l2_input_handler s5k4ba_input_handler = {
	s5k4ba_sensor_init,
	s5k4ba_sensor_exit
};


static int s5k4ba_sensor_add(void)
{
	return s3c_camif_add_sensor(&s5k4ba_input, &s5k4ba_input_handler);
}


static void s5k4ba_sensor_remove(void)
{
	if (s5k4ba_data.sensor != NULL)
		i2c_del_driver(&s5k4ba_driver);

	s3c_camif_remove_sensor(&s5k4ba_input, &s5k4ba_input_handler);
}

module_init(s5k4ba_sensor_add)
module_exit(s5k4ba_sensor_remove)

MODULE_AUTHOR("Jinsung, Yang <jsgood.yang@samsung.com>");
MODULE_DESCRIPTION("I2C Client Driver for FIMC V4L2 Driver");
MODULE_LICENSE("GPL");

