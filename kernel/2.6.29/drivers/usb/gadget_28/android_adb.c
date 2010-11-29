/*
 * Gadget Driver for Android ADB
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/err.h>
#include <linux/interrupt.h>

#include <linux/types.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#if 1
#define xprintk(d, level, fmt, args...) \
	{ if ((d)->gadget) dev_printk(level , &(d)->gadget->dev , \
	    fmt , ## args); else printk(fmt , ## args); }
#ifdef DEBUG
#define DBG(dev, fmt, args...) \
	xprintk(dev , KERN_DEBUG , fmt , ## args)
#else
#define DBG(dev, fmt, args...) do { } while (0)
#endif /* DEBUG */

#ifdef VERBOSE_DEBUG
#define VDBG	DBG
#else
#define VDBG(dev, fmt, args...) do { } while (0)
#endif /* VERBOSE_DEBUG */

#define ERROR(dev, fmt, args...) \
	xprintk(dev , KERN_ERR , fmt , ## args)
#define INFO(dev, fmt, args...) \
	xprintk(dev , KERN_INFO , fmt , ## args)

#else

#define DBG(dev, fmt, args...) \
	printk(fmt, ## args)
#define VDBG	DBG

#define ERROR(dev, fmt, args...) \
	printk(fmt, ## args)
#define INFO(dev, fmt, args...) \
	printk(fmt, ## args)


#endif


MODULE_AUTHOR("Mike Lockwood");
MODULE_DESCRIPTION("Android ADB Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

#define DRIVER_VENDOR_ID	0x18D1
#define DRIVER_PRODUCT_ID	0x0001


/* IDs for string descriptors */
#define STRING_MANUFACTURER             1
#define STRING_PRODUCT                  2
#define STRING_SERIAL                   3
#define STRING_ADB_INTERFACE            4

/* String Table */
static char manufacturer[64];
static char product[64];
static char serial[64];

static struct usb_string strings[] = {
	{ STRING_MANUFACTURER,      manufacturer },
	{ STRING_PRODUCT,           product },
	{ STRING_SERIAL,            serial },
	{ STRING_ADB_INTERFACE,     "ADB Interface" },
	{}
};

static struct usb_gadget_strings stringtab = {
	.language  = 0x0409, /* en-US */
	.strings   = strings,
};

/* ID for our one configuration. */
#define	CONFIG_VALUE                   1

#define BULK_BUFFER_SIZE           4096

/* number of rx and tx requests to allocate */
#define RX_REQ_MAX 4
#define TX_REQ_MAX 4

static struct usb_device_descriptor device_desc = {
	.bLength              = sizeof(device_desc),
	.bDescriptorType      = USB_DT_DEVICE,
	.bcdUSB               = __constant_cpu_to_le16(0x0200),
	.bDeviceClass         = USB_CLASS_PER_INTERFACE,
	.idVendor             = __constant_cpu_to_le16(DRIVER_VENDOR_ID),
	.idProduct            = __constant_cpu_to_le16(DRIVER_PRODUCT_ID),
	.bcdDevice            = __constant_cpu_to_le16(0xffff),
	.iManufacturer        = STRING_MANUFACTURER,
	.iProduct             = STRING_PRODUCT,
	.iSerialNumber        = STRING_SERIAL,
	.bNumConfigurations   = 1,
};

static struct usb_config_descriptor config_desc = {
	.bLength              = sizeof(config_desc),
	.bDescriptorType      = USB_DT_CONFIG,
	.bNumInterfaces       = 1,
	.bConfigurationValue  = CONFIG_VALUE,
	.bmAttributes         = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower            = 0x80, /* 250ma */
};


static struct usb_qualifier_descriptor qualifier_desc = {
	.bLength              = sizeof(struct usb_qualifier_descriptor),
	.bDescriptorType      = USB_DT_DEVICE_QUALIFIER,
	.bcdUSB               = __constant_cpu_to_le16(0x0200),
	.bDeviceClass         =	USB_CLASS_PER_INTERFACE,
	.bNumConfigurations   = 1,
};

#define EP0_BUFSIZE     256
#define MAX_DESC_LEN    256

static const char longname[] = "Android ADB Gadget Driver";
static const char shortname[] = "android_adb";

struct adb_dev {
	spinlock_t lock;
	struct usb_gadget *gadget;
	struct usb_request *req_ep0; /* request for ep0 */
	int config;

	struct usb_ep *ep_in;
	struct usb_ep *ep_out;

	int online;
	int error;

	atomic_t read_excl;
	atomic_t write_excl;
	atomic_t open_excl;

	struct list_head tx_idle;
	struct list_head rx_idle;
	struct list_head rx_done;

	wait_queue_head_t read_wq;
	wait_queue_head_t write_wq;

	/* the request we're currently reading from */
	struct usb_request *read_req;
	unsigned char *read_buf;
	unsigned read_count;
};

static struct usb_interface_descriptor adb_interface_desc = {
	.bLength                = USB_DT_INTERFACE_SIZE,
	.bDescriptorType        = USB_DT_INTERFACE,
	.bInterfaceNumber       = 0,
	.bNumEndpoints          = 2,
	.bInterfaceClass        = 0xFF,
	.bInterfaceSubClass     = 0x42,
	.bInterfaceProtocol     = 1,
	.iInterface             = STRING_ADB_INTERFACE,
};

static struct usb_endpoint_descriptor adb_highspeed_in_desc = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_IN,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize         = __constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor adb_highspeed_out_desc = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_OUT,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize         = __constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor adb_fullspeed_in_desc = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_IN,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor adb_fullspeed_out_desc = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_OUT,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
};


/* temporary variable used between adb_open() and adb_gadget_bind() */
static struct adb_dev *_adb_dev;


static struct usb_request *adb_request_new(struct usb_ep *ep, int buffer_size)
{
	struct usb_request *req = usb_ep_alloc_request(ep, GFP_KERNEL);
	if (!req)
		return NULL;

	/* now allocate buffers for the requests */
	req->buf = kmalloc(buffer_size, GFP_KERNEL);
	if (!req->buf) {
		usb_ep_free_request(ep, req);
		return NULL;
	}

	return req;
}

static void adb_request_free(struct usb_request *req, struct usb_ep *ep)
{
	if (req) {
		kfree(req->buf);
		usb_ep_free_request(ep, req);
	}
}

static inline int _lock(atomic_t *excl)
{
	if (atomic_inc_return(excl) == 1) {
		return 0;
	} else {
		atomic_dec(excl);
		return -1;
	}
}

static inline void _unlock(atomic_t *excl)
{
	atomic_dec(excl);
}

/* add a request to the tail of a list */
void req_put(struct adb_dev *dev, struct list_head *head,
		struct usb_request *req)
{
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);
	list_add_tail(&req->list, head);
	spin_unlock_irqrestore(&dev->lock, flags);
}

/* remove a request from the head of a list */
struct usb_request *req_get(struct adb_dev *dev, struct list_head *head)
{
	unsigned long flags;
	struct usb_request *req;

	spin_lock_irqsave(&dev->lock, flags);
	if (list_empty(head)) {
		req = 0;
	} else {
		req = list_first_entry(head, struct usb_request, list);
		list_del(&req->list);
	}
	spin_unlock_irqrestore(&dev->lock, flags);
	return req;
}

static void ep0_complete(struct usb_ep *ep, struct usb_request *req)
{
}

static void adb_complete_in(struct usb_ep *ep, struct usb_request *req)
{
	struct adb_dev *dev = ep->driver_data;

	if (req->status != 0)
		dev->error = 1;

	req_put(dev, &dev->tx_idle, req);

	wake_up(&dev->write_wq);
}

static void adb_complete_out(struct usb_ep *ep, struct usb_request *req)
{
	struct adb_dev *dev = ep->driver_data;

	if (req->status != 0) {
		dev->error = 1;
		
		req_put(dev, &dev->rx_idle, req);
		DBG(dev, "rx_idle\n");
	} else {
		req_put(dev, &dev->rx_done, req);
		DBG(dev, "rx_done\n");
	}

	wake_up(&dev->read_wq);
}

static int set_configuration(struct adb_dev *dev, int config, int speed)
{
	int result;

	DBG(dev, "set configuration: %d\n", config);
	if (dev->config == config)
		return 0;

	if (config == CONFIG_VALUE) {
		result = usb_ep_enable(dev->ep_in,
			(speed == USB_SPEED_HIGH ?
					&adb_highspeed_in_desc :
					&adb_fullspeed_in_desc));
		if (result)
			return result;
		result = usb_ep_enable(dev->ep_out,
			(speed == USB_SPEED_HIGH ?
					&adb_highspeed_out_desc :
					&adb_fullspeed_out_desc));
		if (result)
			return result;
		dev->online = 1;
	} else {
		dev->online = 0;
		dev->error = 1;
		usb_ep_disable(dev->ep_in);
		usb_ep_disable(dev->ep_out);
	}

	dev->config = config;
	
	/* readers may be blocked waiting for us to go online */
	wake_up(&dev->read_wq);
	return 0;
}

static int __init create_bulk_endpoints(struct usb_gadget *gadget,
				struct adb_dev *dev,
				struct usb_endpoint_descriptor *in_desc,
				struct usb_endpoint_descriptor *out_desc)
{
	struct usb_request *req;
	struct usb_ep *ep;
	int i;

	ep = usb_ep_autoconfig(gadget, in_desc);
	if (!ep) {
		DBG(dev, "usb_ep_autoconfig for ep_in failed\n");
		return -ENODEV;
	}
	DBG(dev, "usb_ep_autoconfig for ep_in got %s\n", ep->name);
	ep->driver_data = dev;
	dev->ep_in = ep;

	ep = usb_ep_autoconfig(gadget, out_desc);
	if (!ep) {
		DBG(dev, "usb_ep_autoconfig for ep_out failed\n");
		return -ENODEV;
	}
	DBG(dev, "usb_ep_autoconfig for adb ep_out got %s\n", ep->name);
	ep->driver_data = dev;
	dev->ep_out = ep;

	/* now allocate requests for our endpoints */
	for (i = 0; i < RX_REQ_MAX; i++) {
		req = adb_request_new(dev->ep_out, BULK_BUFFER_SIZE);
		if (!req)
			goto fail;
		req->complete = adb_complete_out;
		req_put(dev, &dev->rx_idle, req);
	}

	for (i = 0; i < TX_REQ_MAX; i++) {
		req = adb_request_new(dev->ep_in, BULK_BUFFER_SIZE);
		if (!req)
			goto fail;
		req->complete = adb_complete_in;
		req_put(dev, &dev->tx_idle, req);
	}

	return 0;

fail:
	printk(KERN_ERR "adb_bind() could not allocate requests\n");
	return -1;
}

static void adb_gadget_unbind(struct usb_gadget *gadget)
{
	struct adb_dev *dev = get_gadget_data(gadget);
	struct usb_request *req;

	DBG(dev, "adb_gadget_unbind\n");

	set_configuration(dev, 0, USB_SPEED_UNKNOWN);

	gadget->ep0->driver_data = NULL;
	set_gadget_data(gadget, NULL);

	spin_lock_irq(&dev->lock);

	while ((req = req_get(dev, &dev->rx_idle)))
		adb_request_free(req, dev->ep_out);
	while ((req = req_get(dev, &dev->tx_idle)))
		adb_request_free(req, dev->ep_in);

	adb_request_free(dev->req_ep0, gadget->ep0);
	dev->req_ep0 = NULL;

	dev->online = 0;
	dev->error = 1;
	spin_unlock_irq(&dev->lock);
}

static int __init adb_gadget_bind(struct usb_gadget *gadget)
{
	struct adb_dev *dev = _adb_dev;

	DBG(dev, "adb_gadget_bind\n");
	if (!dev)
		return -ESRCH;

	set_gadget_data(gadget, dev);
	dev->gadget = gadget;

	/* auto configure our bulk endpoints */
	usb_ep_autoconfig_reset(gadget);

	DBG(dev, "adb_gadget_bind create_bulk_endpoints\n");
	create_bulk_endpoints(gadget, dev, &adb_fullspeed_in_desc,
			&adb_fullspeed_out_desc);

	/* copy endpoint addresses computed by usb_ep_autoconfig()
	 * to the high speed descriptors
	 */
	adb_highspeed_in_desc.bEndpointAddress =
			adb_fullspeed_in_desc.bEndpointAddress;
	adb_highspeed_out_desc.bEndpointAddress =
			adb_fullspeed_out_desc.bEndpointAddress;

	dev->req_ep0 = adb_request_new(gadget->ep0, EP0_BUFSIZE);
	if (!dev->req_ep0)
		goto err;
	dev->req_ep0->complete = ep0_complete;

	/* set device max packet size */
	device_desc.bMaxPacketSize0 = gadget->ep0->maxpacket;
	qualifier_desc.bMaxPacketSize0 = gadget->ep0->maxpacket;

	usb_gadget_set_selfpowered(gadget);

	return 0;

err:
	adb_gadget_unbind(gadget);
	return -ENOMEM;
}

/* must have enough entries for 3 descriptors and NULL termination */
static const struct usb_descriptor_header *adb_function[3 + 1];

static const struct usb_descriptor_header * *
build_device_function(int high_speed)
{
	const struct usb_descriptor_header **descriptor = adb_function;

	*descriptor++ = (struct usb_descriptor_header *)&adb_interface_desc;

	if (high_speed) {
		*descriptor++ =
			(struct usb_descriptor_header *)&adb_highspeed_in_desc;
		*descriptor++ =
			(struct usb_descriptor_header *)&adb_highspeed_out_desc;
	} else {
		*descriptor++ =
			(struct usb_descriptor_header *)&adb_fullspeed_in_desc;
		*descriptor++ =
			(struct usb_descriptor_header *)&adb_fullspeed_out_desc;
	}

	/* NULL terminate */
	*descriptor = NULL;
	return adb_function;
}

static int adb_gadget_setup(struct usb_gadget *gadget,
		const struct usb_ctrlrequest *ctrl)
{
	struct adb_dev *dev = get_gadget_data(gadget);
	struct usb_request *req = dev->req_ep0;
	const struct usb_descriptor_header **function;

	int	result = -EOPNOTSUPP;
	u8 request_type = (ctrl->bRequestType & USB_TYPE_MASK);
	u8 direction = (ctrl->bRequestType & USB_DIR_IN);
	u8 request = ctrl->bRequest;
	u16 index = le16_to_cpu(ctrl->wIndex);
	u16	value = le16_to_cpu(ctrl->wValue);
	u16	length = le16_to_cpu(ctrl->wLength);

	spin_lock(&dev->lock);

/*	DBG(dev, "SETUP type: %02x, request: %02x, index: %04x, \
			value: %04x, length: %04x\n",
			ctrl->bRequestType, request, index, value, length);
*/
	if (request_type != USB_TYPE_STANDARD)
		goto unsupported;

	switch (request) {
	case USB_REQ_GET_DESCRIPTOR: {
		int descriptorType = value >> 8;
		int descriptorIndex = value & 0xFF;

		DBG(dev, "USB_REQ_GET_DESCRIPTOR: \
			type: %d index: %d\n", descriptorType, descriptorIndex);
		if (direction != USB_DIR_IN) {
			DBG(dev, "wrong direction!\n");
			goto unsupported;
		}

		switch (descriptorType) {
		case USB_DT_DEVICE:
			DBG(dev, "USB_DT_DEVICE\n");
			result = sizeof(device_desc);
			memcpy(req->buf, &device_desc, result);
			break;

		case USB_DT_CONFIG:
		case USB_DT_OTHER_SPEED_CONFIG: {
			struct usb_config_descriptor *config_buf;
			int high_speed;

			if (descriptorType == USB_DT_OTHER_SPEED_CONFIG) {
				high_speed = (gadget->speed != USB_SPEED_HIGH);
				DBG(dev, "USB_DT_OTHER_SPEED_CONFIG\n");
			} else {
				high_speed =
					(gadget->speed == USB_SPEED_HIGH);
				DBG(dev, "USB_DT_CONFIG\n");
			}

			DBG(dev, "high_speed = %d\n", high_speed);
			if (descriptorIndex >= device_desc.bNumConfigurations)
				return -EINVAL;

			function = build_device_function(high_speed);
			result = usb_gadget_config_buf(&config_desc,
					req->buf, MAX_DESC_LEN, function);
			if (result < 0)
				break;
			config_buf = req->buf;
			config_buf->bDescriptorType = descriptorType;
			break;
		}

		case USB_DT_DEVICE_QUALIFIER:
			DBG(dev, "USB_DT_DEVICE_QUALIFIER\n");
			if (!gadget_is_dualspeed(gadget))
				goto unsupported;
			result = sizeof(qualifier_desc);
			memcpy(req->buf, &qualifier_desc, result);
			break;

		case USB_DT_STRING:
			DBG(dev, "USB_DT_STRING\n");
			result = usb_gadget_get_string(&stringtab,
					descriptorIndex, req->buf);
			break;
		}
		break;
	}

	case USB_REQ_GET_INTERFACE:
		DBG(dev, "USB_REQ_GET_INTERFACE\n");
		if (ctrl->bRequestType !=
				(USB_DIR_IN | USB_RECIP_INTERFACE))
			goto unsupported;
		if (dev->config == 0)
			goto unsupported;

		if (index != 0 && index != 1)
			goto unsupported;
		*(u8 *)req->buf = 0;
		result = 1;
		break;

	case USB_REQ_SET_INTERFACE: {
		DBG(dev, "USB_REQ_SET_INTERFACE\n");
		if (ctrl->bRequestType !=
				(USB_DIR_OUT | USB_RECIP_INTERFACE))
			goto unsupported;
		if (dev->config == 0 ||
				(index != 0 && index != 1) ||
				value != 0)
			goto unsupported;
		result = 0;
		break;
	}

	case USB_REQ_GET_CONFIGURATION:
		DBG(dev, "USB_REQ_GET_CONFIGURATION\n");
		if (direction != USB_DIR_IN)
			goto unsupported;
		*(u8 *)req->buf = dev->config;
		result = 1;
		break;

	case USB_REQ_SET_CONFIGURATION:
		DBG(dev, "USB_REQ_SET_CONFIGURATION\n");
		if (ctrl->bRequestType != USB_DIR_OUT)
			goto unsupported;
		if (value != 0 && value != CONFIG_VALUE)
			goto unsupported;

		set_configuration(dev, value, gadget->speed);
		result = 0;
		break;

	default:
unsupported:
		DBG(dev, "Unsupported SETUP type: %02x, request: \
			%02x, index: %04x, value: %04x, length: %04x\n",
			ctrl->bRequestType, request, index, value, length);
	}

	/* send response */
	if (result >= 0) {
		req->length = min(length, (u16)result);

		if (result < length && (result % gadget->ep0->maxpacket) == 0)
			req->zero = 1;
		else
			req->zero = 0;
		result = usb_ep_queue(gadget->ep0, req, GFP_ATOMIC);
		if (result < 0) {
			ERROR(dev, "usb_ep_queue returned %d\n", result);
			req->status = 0;
		}
	}

	spin_unlock(&dev->lock);

	return result;
}

static void adb_gadget_disconnect(struct usb_gadget *gadget)
{
	struct adb_dev *dev = get_gadget_data(gadget);

	DBG(dev, "adb_gadget_disconnect\n");
	dev->online = 0;
	set_configuration(dev, 0, USB_SPEED_UNKNOWN);
}

static struct usb_gadget_driver adb_gadget_driver = {
#ifdef CONFIG_USB_GADGET_DUALSPEED
	.speed           = USB_SPEED_HIGH,
#else
	.speed           = USB_SPEED_FULL,
#endif
	.function        = (char *)longname,
	.bind            = adb_gadget_bind,
	.unbind          = adb_gadget_unbind,
	.setup           = adb_gadget_setup,
	.disconnect      = adb_gadget_disconnect,

	.driver = {
		.name    = (char *) shortname,
		.owner   = THIS_MODULE,
	},
};

static ssize_t adb_read(struct file *fp, char __user *buf,
				size_t count, loff_t *pos)
{
	struct adb_dev *dev = fp->private_data;
	struct usb_request *req;
	int r = count, xfer;
	int ret;

	DBG(dev, "adb_read(%d)\n", count);

	if (_lock(&dev->read_excl))
		return -EBUSY;

	/* we will block until we're online */
	while (!(dev->online || dev->error)) {
		DBG(dev, "adb_read: waiting for online state\n");
		ret = wait_event_interruptible(dev->read_wq,
				(dev->online || dev->error));
		if (ret < 0) {
			_unlock(&dev->read_excl);
			return ret;
		}
	}

	while (count > 0) {
		if (dev->error) {
			r = -EIO;
			break;
		}

		/* if we have idle read requests, get them queued */
		while ((req = req_get(dev, &dev->rx_idle))) {
requeue_req:
			req->length = BULK_BUFFER_SIZE;
			ret = usb_ep_queue(dev->ep_out, req, GFP_ATOMIC);

			if (ret < 0) {
				r = -EIO;
				dev->error = 1;
				req_put(dev, &dev->rx_idle, req);
				goto fail;
			} else {
				DBG(dev, "rx %p queue\n", req);
			}
		}

		/* if we have data pending, give it to userspace */
		if (dev->read_count > 0) {
			if (dev->read_count < count)
				xfer = dev->read_count;
			else
				xfer = count;

			if (copy_to_user(buf, dev->read_buf, xfer)) {
				r = -EFAULT;
				break;
			}
			dev->read_buf += xfer;
			dev->read_count -= xfer;
			buf += xfer;
			count -= xfer;

			/* if we've emptied the buffer, release the request */
			if (dev->read_count == 0) {
				req_put(dev, &dev->rx_idle, dev->read_req);
				dev->read_req = 0;
			}
			continue;
		}

		/* wait for a request to complete */
		req = 0;
		ret = wait_event_interruptible(dev->read_wq,
			((req = req_get(dev, &dev->rx_done)) || dev->error));
		if (req != 0) {
			/* if we got a 0-len one we need to put it back into
			** service.  if we made it the current read req we'd
			** be stuck forever
			*/
			if (req->actual == 0)
				goto requeue_req;

			dev->read_req = req;
			dev->read_count = req->actual;
			dev->read_buf = req->buf;
			DBG(dev, "rx %p %d\n", req, req->actual);
		}

		if (ret < 0) {
			r = ret;
			break;
		}
	}

fail:
	_unlock(&dev->read_excl);
	return r;
}

static ssize_t adb_write(struct file *fp, const char __user *buf,
				 size_t count, loff_t *pos)
{
	struct adb_dev *dev = fp->private_data;
	struct usb_request *req = 0;
	int r = count, xfer;
	int ret;

	DBG(dev, "adb_write(%d)\n", count);

	if (_lock(&dev->write_excl))
		return -EBUSY;

	while (count > 0) {
		if (dev->error) {
			r = -EIO;
			break;
		}

		/* get an idle tx request to use */
		req = 0;
		ret = wait_event_interruptible(dev->write_wq,
			((req = req_get(dev, &dev->tx_idle)) || dev->error));

		if (ret < 0) {
			r = ret;
			break;
		}

		if (req != 0) {
			if (count > BULK_BUFFER_SIZE)
				xfer = BULK_BUFFER_SIZE;
			else
				xfer = count;
			if (copy_from_user(req->buf, buf, xfer)) {
				r = -EFAULT;
				break;
			}

			req->length = xfer;
			ret = usb_ep_queue(dev->ep_in, req, GFP_ATOMIC);
			if (ret < 0) {
				DBG(dev, "adb_write: xfer error %d\n", ret);
				dev->error = 1;
				r = -EIO;
				break;
			}

			buf += xfer;
			count -= xfer;

			/* zero this so we don't try to free it on error exit */
			req = 0;
		}
	}


	if (req)
		req_put(dev, &dev->tx_idle, req);

	_unlock(&dev->write_excl);
	return r;
}

static int adb_open(struct inode *ip, struct file *fp)
{
	DBG(_adb_dev, "adb_open\n");
	if (_lock(&_adb_dev->open_excl))
		return -EBUSY;

	fp->private_data = _adb_dev;

	/* clear the error latch */
	_adb_dev->error = 0;

	return 0;
}

static int adb_release(struct inode *ip, struct file *fp)
{
	DBG(_adb_dev, "adb_release\n");
	_unlock(&_adb_dev->open_excl);
	return 0;
}

/* file operations for ADB device /dev/android_adb */
static struct file_operations adb_fops = {
	.owner = THIS_MODULE,
	.read = adb_read,
	.write = adb_write,
	.open = adb_open,
	.release = adb_release,
};

static struct miscdevice adb_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = shortname,
	.fops = &adb_fops,
};

static int __init adb_init(void)
{
	struct adb_dev *dev;
	int ret;

	printk(KERN_INFO "android adb driver\n");

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	spin_lock_init(&dev->lock);

	init_waitqueue_head(&dev->read_wq);
	init_waitqueue_head(&dev->write_wq);

	atomic_set(&dev->open_excl, 0);
	atomic_set(&dev->read_excl, 0);
	atomic_set(&dev->write_excl, 0);

	INIT_LIST_HEAD(&dev->rx_idle);
	INIT_LIST_HEAD(&dev->rx_done);
	INIT_LIST_HEAD(&dev->tx_idle);

	/* _adb_dev must be set before calling usb_gadget_register_driver */
	_adb_dev = dev;

	ret = misc_register(&adb_device);
	if (ret)
		goto err1;
	ret = usb_gadget_register_driver(&adb_gadget_driver);
	if (ret)
		goto err2;

	strcpy(manufacturer, "Android");
	strcpy(product, "adb driver");
	strcpy(serial, "0123456789ABCDEF");

	return 0;

err2:
	misc_deregister(&adb_device);
err1:
	kfree(dev);
	printk(KERN_ERR "adb gadget driver failed to initialize\n");
	return ret;
}

static void  __exit adb_exit(void)
{
	printk(KERN_INFO "adb_exit\n");

	usb_gadget_unregister_driver(&adb_gadget_driver);
	misc_deregister(&adb_device);
	kfree(_adb_dev);
	_adb_dev = NULL;
}

module_init(adb_init);
module_exit(adb_exit);
