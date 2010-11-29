/*
 * Gadget Driver for Android, with ADB and UMS support
 *
 * Copyright (C) 2009 Samsung Electronics, Seung-Soo Yang
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

#include <linux/usb/android.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>

#include "f_mass_storage.h"
#include "f_adb.h"

#include "gadget_chips.h"

/*
 * Kbuild is not very cooperative with respect to linking separately
 * compiled library objects into one module.  So for now we won't use
 * separate compilation ... ensuring init/exit sections work to shrink
 * the runtime footprint, and giving us at least some parts of what
 * a "gcc --combine ... part1.c part2.c part3.c ... " build would.
 */
#include "usbstring.c"
#include "config.c"
#include "epautoconf.c"
#include "composite.c"

MODULE_AUTHOR("SeungSoo Yange");
MODULE_DESCRIPTION("Android Composite USB Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static const char longname[] = "Gadget Android";

#define DRIVER_DESC		"Android Composite USB"
#define DRIVER_VERSION	__DATE__

/* Default vendor and product IDs, overridden by platform data */
#define VENDOR_ID		0x18D1
#define PRODUCT_ID		0xa004
#define ADB_PRODUCT_ID	0xb004

/* 
 * Originally, adbd will enable adb function at booting 
 * if the value of persist.service.adb.enable is 1
 * ADB_ENABLE_AT_BOOT intends to enable adb function 
 * in case of no enabling by adbd or improper RFS
 */
#define ADB_ENABLE_AT_BOOT	0


struct android_dev {
	struct usb_gadget *gadget;
	struct usb_composite_dev *cdev;

	int product_id;
	int adb_product_id;
	int version;

	int adb_enabled;
	int nluns;
};

static atomic_t adb_enable_excl;
static struct android_dev *_android_dev;

/* string IDs are assigned dynamically */

#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

/* String Table */
static struct usb_string strings_dev[] = {
	/* These dummy values should be overridden by platform data */
	[STRING_MANUFACTURER_IDX].s = "Samsung",
	[STRING_PRODUCT_IDX].s = "S3C6410 Android",
	[STRING_SERIAL_IDX].s = "S3C6410 Android",
	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

static struct usb_device_descriptor device_desc = {
	.bLength              = sizeof(device_desc),
	.bDescriptorType      = USB_DT_DEVICE,
	.bcdUSB               = __constant_cpu_to_le16(0x0200),
	.bDeviceClass		  = USB_CLASS_MASS_STORAGE,
	.bDeviceSubClass	  = 0x06,//US_SC_SCSI,
	.bDeviceProtocol	  = 0x50,//US_PR_BULK,
	.idVendor             = __constant_cpu_to_le16(VENDOR_ID),
	.idProduct            = __constant_cpu_to_le16(PRODUCT_ID),
	.bcdDevice            = __constant_cpu_to_le16(0xffff),
	.bNumConfigurations   = 1,
};

static struct usb_otg_descriptor otg_descriptor = {
	.bLength =		sizeof otg_descriptor,
	.bDescriptorType =	USB_DT_OTG,

	/* REVISIT SRP-only hardware is possible, although
	 * it would not be called "OTG" ...
	 */
	.bmAttributes =		USB_OTG_SRP | USB_OTG_HNP,
};

static const struct usb_descriptor_header *otg_desc[] = {
	(struct usb_descriptor_header *) &otg_descriptor,
	NULL,
};

void android_usb_set_connected(int connected)
{
	if (_android_dev && _android_dev->cdev && _android_dev->cdev->gadget) {
		if (connected)
			usb_gadget_connect(_android_dev->cdev->gadget);
		else
			usb_gadget_disconnect(_android_dev->cdev->gadget);
	}
}

#if ADB_ENABLE_AT_BOOT
static void enable_adb(struct android_dev *dev, int enable);
#endif

static int __init android_bind_config(struct usb_configuration *c)
{
	struct android_dev *dev = _android_dev;
	int ret;

	ret = mass_storage_function_add(dev->cdev, c, dev->nluns);
	if (ret) {
		printk("[%s] Fail to mass_storage_function_add()\n", __func__);
		return ret;
	}
	ret = adb_function_add(dev->cdev, c);
	if (ret) {
		printk("[%s] Fail to adb_function_add()\n", __func__);
		return ret;
	}

#if ADB_ENABLE_AT_BOOT
	printk("[%s] Enabling adb function at booting\n", __func__);
	enable_adb(dev, 1);
#endif

		return ret;
}

#define	ANDROID_DEBUG_CONFIG_STRING "UMS + ADB (Debugging mode)"
#define	ANDROID_NO_DEBUG_CONFIG_STRING "UMS Only (Not debugging mode)"

static struct usb_configuration android_config  = {
	.label		= ANDROID_NO_DEBUG_CONFIG_STRING,
	.bind		= android_bind_config,
	.bConfigurationValue = 1,
	.bmAttributes	= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower	= 0x30, /*  x2 = 160ma */
};

static int __init android_bind(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;
	struct usb_gadget	*gadget = cdev->gadget;
	int			gcnum;
	int			id;
	int			ret;
	char usb_serial_number[13] = {0,};

//	printk(KERN_INFO "android_bind\n");

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */
	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_MANUFACTURER_IDX].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_PRODUCT_IDX].id = id;
	device_desc.iProduct = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_SERIAL_IDX].id = id;

	if( (usb_serial_number[0] + 
		 usb_serial_number[1] + 
		 usb_serial_number[2]) != 0 )
	strcpy((char *)(strings_dev[STRING_SERIAL_IDX].s), usb_serial_number);
	printk("[ADB_UMS_ACM] string_dev = %s \n",strings_dev[STRING_SERIAL_IDX].s);

	device_desc.iSerialNumber = id;

	gcnum = usb_gadget_controller_number(gadget);
	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(0x0200 + gcnum);
	else {
		/* gadget zero is so simple (for now, no altsettings) that
		 * it SHOULD NOT have problems with bulk-capable hardware.
		 * so just warn about unrcognized controllers -- don't panic.
		 *
		 * things like configuration and altsetting numbering
		 * can need hardware-specific attention though.
		 */
		pr_warning("%s: controller '%s' not recognized\n",
			longname, gadget->name);
		device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
	}

	if (gadget_is_otg(cdev->gadget)) 
		android_config.descriptors = otg_desc;
	
#if USBCV_CH9_REMOTE_WAKE_UP_TEST 
	if (gadget->ops->wakeup)
		android_config.bmAttributes |= USB_CONFIG_ATT_WAKEUP;
#endif

	/* register our configuration */
	ret = usb_add_config(cdev, &android_config);
	if (ret) {
		printk(KERN_ERR "usb_add_config failed\n");
	        return ret;
	}

	usb_gadget_set_selfpowered(gadget);
	dev->cdev = cdev;

	INFO(cdev, "%s, version: " DRIVER_VERSION "\n", DRIVER_DESC);

	return 0;
}

/*
	Google gadget doesn't support module type building
static int __exit android_unbind(struct usb_composite_dev *cdev)
{
	gserial_cleanup();
	return 0;
}
*/

static struct usb_composite_driver android_usb_driver = {
	.name		= "android_usb",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.bind		= android_bind,
//	.unbind 	= __exit_p(android_unbind),
};

static void enable_adb(struct android_dev *dev, int enable)
{
	if (enable != dev->adb_enabled) {
		dev->adb_enabled = enable;
		adb_function_enable(enable);

		/* set product ID to the appropriate value */
		if (enable)
		{
			device_desc.idProduct =
				__constant_cpu_to_le16(dev->adb_product_id);
			device_desc.bDeviceClass	  = USB_CLASS_PER_INTERFACE;
			device_desc.bDeviceSubClass	  = 0x00;
			device_desc.bDeviceProtocol	  = 0x00;
			android_config.label = ANDROID_DEBUG_CONFIG_STRING;
		}
		else
		{
			device_desc.idProduct =
				__constant_cpu_to_le16(dev->product_id);
			device_desc.bDeviceClass	  = USB_CLASS_MASS_STORAGE;
			device_desc.bDeviceSubClass	  = 0x06;//US_SC_SCSI;
			device_desc.bDeviceProtocol	  = 0x50;//US_PR_BULK;
			android_config.label = ANDROID_NO_DEBUG_CONFIG_STRING;
		}

		/* Host get the following dev->cdev->desc as Device Description */
		if (dev->cdev)
		{
			dev->cdev->desc.idProduct = device_desc.idProduct;
			dev->cdev->desc.bDeviceClass = device_desc.bDeviceClass;
			dev->cdev->desc.bDeviceSubClass = device_desc.bDeviceSubClass;
			dev->cdev->desc.bDeviceProtocol = device_desc.bDeviceProtocol;
		}

#if 0
		/* force reenumeration */
		if (dev->cdev && dev->cdev->gadget &&
				//the following means that usb device already enumerated by host
				dev->cdev->gadget->speed != USB_SPEED_UNKNOWN) {
#else
/*
	for reenumeration in case of booting-up connected with host
	because if connected, host don't enumerate 
*/
		if (dev->cdev && dev->cdev->gadget ) {
#endif			
			usb_gadget_disconnect(dev->cdev->gadget);
			msleep(5);
			usb_gadget_connect(dev->cdev->gadget);
		}
	}
}

static int adb_enable_open(struct inode *ip, struct file *fp)
{
	if (atomic_inc_return(&adb_enable_excl) != 1) {
		atomic_dec(&adb_enable_excl);
		return -EBUSY;
	}
	enable_adb(_android_dev, 1);

	return 0;
}

static int adb_enable_release(struct inode *ip, struct file *fp)
{
	enable_adb(_android_dev, 0);
	atomic_dec(&adb_enable_excl);
	return 0;
}

static const struct file_operations adb_enable_fops = {
	.owner =   THIS_MODULE,
	.open =    adb_enable_open,
	.release = adb_enable_release,
};

static struct miscdevice adb_enable_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "android_adb_enable",
	.fops = &adb_enable_fops,
};

static int __init android_probe(struct platform_device *pdev)
{
	struct android_usb_platform_data *pdata = pdev->dev.platform_data;
	struct android_dev *dev = _android_dev;

	if (pdata) {
		if (pdata->vendor_id)
			device_desc.idVendor =
				__constant_cpu_to_le16(pdata->vendor_id);
		if (pdata->product_id) {
			dev->product_id = pdata->product_id;
			device_desc.idProduct =
				__constant_cpu_to_le16(pdata->product_id);
		}
		if (pdata->adb_product_id)
			dev->adb_product_id = pdata->adb_product_id;
		if (pdata->version)
			dev->version = pdata->version;

		if (pdata->product_name)
			strings_dev[STRING_PRODUCT_IDX].s = pdata->product_name;
		if (pdata->manufacturer_name)
			strings_dev[STRING_MANUFACTURER_IDX].s =
					pdata->manufacturer_name;
		if (pdata->serial_number)
			strings_dev[STRING_SERIAL_IDX].s = pdata->serial_number;
		dev->nluns = pdata->nluns;
	}

	return 0;
}

static struct platform_driver android_platform_driver = {
	.driver = { .name = "S3C6410 Android USB", },
	.probe = android_probe,
};

static int __init init(void)
{
	struct android_dev *dev;
	int ret;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	/* set default values, which should be overridden by platform data */
	dev->product_id = PRODUCT_ID;
	dev->adb_product_id = ADB_PRODUCT_ID;
	_android_dev = dev;

	ret = platform_driver_register(&android_platform_driver);
	if (ret)
		return ret;
	ret = misc_register(&adb_enable_device);
	if (ret) {
		platform_driver_unregister(&android_platform_driver);
		return ret;
	}
	ret = usb_composite_register(&android_usb_driver);
	if (ret) {
		misc_deregister(&adb_enable_device);
		platform_driver_unregister(&android_platform_driver);
	}
	return ret;
}
module_init(init);

static void __exit cleanup(void)
{
	usb_composite_unregister(&android_usb_driver);
	misc_deregister(&adb_enable_device);
	platform_driver_unregister(&android_platform_driver);
	kfree(_android_dev);
	_android_dev = NULL;
}
module_exit(cleanup);
