/*
 * drivers/usb/gadget/s3c-udc-otg-hs_dma.c
 * Samsung S3C on-chip full/high speed USB OTG 2.0 device controller dma mode
 *
 * Copyright (C) 2008 Samsung Electronics, Kyu-Hyeok Jang, Seung-Soo Yang
 * Copyright (C) 2009 Samsung Electronics, Seung-Soo Yang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
	In case of DMA mode, endpoints' next ep setting

default:
	EP0 => list of next ep or EP0 in case of no list
	EP2 => EP3, 	EP3 => EP0
	EP5 => EP6, 	EP6 => EP0
	EP8 => EP9, 	EP9 => EP0

ADB_UMS_ACM || ADB_UMS:
	EP2 => EP0, add EP2 to next ep list of EP0 in reconfig_usbd
	EP5 => EP0, add EP5 to next ep list of EP0 in reconfig_usbd
	EP8 => EP0, add EP8 to next ep list of EP0 in reconfig_usbd

CDC:
	EP2 => EP0, add EP2 to next ep list of EP0 in reconfig_usbd

RNDIS:
	EP3 => EP0, add EP3 to next ep list of EP0 in setdma_tx
*/

#define G_SERIAL_DELAY 			1
#define	SUPPORTING_MAX_EP_NUM	9	//0 ~ 9
#define DEP_OFFSET				0x20

extern void s3c_udc_soft_connect(void);
extern void s3c_udc_soft_disconnect(void);

static int g_reset_available = 1;

struct diepctl_next_ep
{
	u8 ep_num;
	struct list_head list;
};

static struct diepctl_next_ep ep0_nextep_list;

void s3c_show_mode(void)
{
	printk("[S3C USB-OTG MODE] : DMA\n");
}

//set ep_num's DIEPCTLn.nextep 4 bits next_ep for EP_IN
static void s3c_set_next_ep(u8 ep_num, u8 next_ep)
{
	u32 ctrl;

	ctrl = readl(S3C_UDC_OTG_DIEPCTL0+DEP_OFFSET*ep_num);
	//DIEPCTLn.nextep 4bits
	ctrl &= ~(0xf<<DIEPCTL_NEXT_EP_BIT);	
	writel(next_ep<<DIEPCTL_NEXT_EP_BIT |ctrl, S3C_UDC_OTG_DIEPCTL0+DEP_OFFSET*ep_num);
	
	DEBUG_IN_EP("[%s] : set EP[%d].nextep => EP[%d]\n", __FUNCTION__, ep_num, next_ep);
}

static u8 get_ep0_next_ep(void)
{
	u8	ep_num = 0;
	struct diepctl_next_ep *tmp_next_ep;

	if (!list_empty(&ep0_nextep_list.list))
	{		
		tmp_next_ep = list_entry(ep0_nextep_list.list.next, struct diepctl_next_ep, list);
		ep_num = tmp_next_ep->ep_num;
	}
	return ep_num;
}

static void add_ep0_next_ep(u8 ep_num)
{
	u32 ctrl;
	u8 ep0_nextep;

	struct diepctl_next_ep *tmp_next_ep;
	
	ctrl = readl(S3C_UDC_OTG_DIEPCTL0);
	ep0_nextep = 0xf & (ctrl>>DIEPCTL_NEXT_EP_BIT);	

	if (list_empty(&ep0_nextep_list.list))
	{		
		s3c_set_next_ep(EP0_CON, ep_num);
	}

	tmp_next_ep = kmalloc(sizeof *tmp_next_ep, GFP_ATOMIC);
	if (!tmp_next_ep)
	{
		printk("[%s] kmalloc failed \n", __FUNCTION__);
	}
	tmp_next_ep->ep_num = ep_num;
	
	list_add_tail(&(tmp_next_ep->list), &(ep0_nextep_list.list));
	DEBUG_IN_EP("[%s] : list_add_tail EP [%d]\n", __FUNCTION__, ep_num);
}

static void del_ep0_next_ep(u8 ep_num)
{
	struct diepctl_next_ep *tmp_next_ep;

	if (list_empty(&ep0_nextep_list.list))
	{
		DEBUG_IN_EP("[%s] : list_empty\n", __FUNCTION__);
		return;
	}
	
	tmp_next_ep = list_entry(ep0_nextep_list.list.next, struct diepctl_next_ep, list);

	if(tmp_next_ep->ep_num == ep_num)
	{
		list_del(&tmp_next_ep->list);
		kfree(tmp_next_ep);
		DEBUG_IN_EP("[%s] : list_del & kfree EP0->next.ep[%d]\n", __FUNCTION__, ep_num);
	}
	else 
	{				
		DEBUG_IN_EP("[%s] : ep0_nextep_list->ep_num[%d] != ep_num[%d]\n"
			, __FUNCTION__, tmp_next_ep->ep_num, ep_num);
	}
}

static void del_one_ep_list_from_ep0_next_ep(u8 ep_num)
{
	struct diepctl_next_ep *tmp_next_ep;
	struct list_head	*pos, *tmp;

	if (!list_empty(&ep0_nextep_list.list))
	{		
		list_for_each_safe(pos, tmp, &ep0_nextep_list.list) 
		{
			tmp_next_ep = list_entry(pos, struct diepctl_next_ep, list);
			if (tmp_next_ep->ep_num == ep_num)
			{				
				list_del(&tmp_next_ep->list);
				kfree(tmp_next_ep);
			}
		}
	}
}

//Send Zero Length Packet for status stage of control transfer
static inline void s3c_send_zlp(void)
{
	u32 diepctl0;
	
	DEBUG_IN_EP("[%s]\n", __FUNCTION__);

	diepctl0 = readl(S3C_UDC_OTG_DIEPCTL0);

	writel(1<<DEPTSIZ_PKT_CNT_BIT| 0<<DEPTSIZ_XFER_SIZE_BIT, S3C_UDC_OTG_DIEPTSIZ0); 
    writel(diepctl0|DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DIEPCTL0); 
}

static int setdma_rx(struct s3c_ep *ep, struct s3c_request *req)
{
	u32 *buf, ctrl;
	u32 length, pktcnt;
	u32 ep_num = ep_index(ep);

	buf = req->req.buf + req->req.actual;
	prefetchw(buf);

	length = req->req.length - req->req.actual;
	dma_cache_maint(buf, length, DMA_FROM_DEVICE);

	if(length == 0)
		pktcnt = 1;
	else
		pktcnt = (length - 1)/(ep->ep.maxpacket) + 1;

	switch(ep_num)
	{
		case EP0_CON: 
		case EP1_OUT: 
		case EP4_OUT: 			
		case EP7_OUT: 			
			ctrl =  readl(S3C_UDC_OTG_DOEPCTL0+DEP_OFFSET*ep_num);
			
			writel(virt_to_phys(buf), S3C_UDC_OTG_DOEPDMA0+DEP_OFFSET*ep_num);
			writel((pktcnt<<DEPTSIZ_PKT_CNT_BIT)|(length<<DEPTSIZ_XFER_SIZE_BIT), S3C_UDC_OTG_DOEPTSIZ0+DEP_OFFSET*ep_num);
			writel(DEPCTL_EPENA|DEPCTL_CNAK|ctrl, S3C_UDC_OTG_DOEPCTL0+DEP_OFFSET*ep_num);
			
			DEBUG_OUT_EP("[%s] RX DMA start : DOEPDMA[%d] = 0x%x, DOEPTSIZ[%d] = 0x%x, DOEPCTL[%d] = 0x%x,\
				pktcnt = %d, xfersize = %d\n", __FUNCTION__, \
				    ep_num, readl(S3C_UDC_OTG_DOEPDMA0+DEP_OFFSET*ep_num), 
				    ep_num, readl(S3C_UDC_OTG_DOEPTSIZ0+DEP_OFFSET*ep_num), 
				    ep_num, readl(S3C_UDC_OTG_DOEPCTL0+DEP_OFFSET*ep_num), 
					pktcnt, length);
			break;
		default:
			printk("[%s]: Error Unused EP[%d]\n", __FUNCTION__, ep_num);
	}

	return 0;

}

static int setdma_tx(struct s3c_ep *ep, struct s3c_request *req)
{
	u32 *buf, ctrl;
	u32 length, pktcnt;
	u32 ep_num = ep_index(ep); 

	buf = req->req.buf + req->req.actual;
	prefetch(buf);
	length = req->req.length - req->req.actual;
	
	if(ep_num == EP0_CON) 
		length = min(length, (u32)ep_maxpacket(ep));
	req->req.actual += length;
	dma_cache_maint(buf, length, DMA_TO_DEVICE);

	if(length == 0)
		pktcnt = 1;
	else
		pktcnt = (length - 1)/(ep->ep.maxpacket) + 1;

	DEBUG_IN_EP("==[%s]: EP [%d]\n", __FUNCTION__, ep_num);		

	switch(ep_num)
	{
		case EP0_CON:
			s3c_set_next_ep(EP0_CON, get_ep0_next_ep());
		case EP2_IN:
		case EP3_IN:
		case EP5_IN:
		case EP6_IN:			
		case EP8_IN:
		case EP9_IN:			
			ctrl = readl(S3C_UDC_OTG_DIEPCTL0+DEP_OFFSET*ep_num);
			
			writel((pktcnt<<DEPTSIZ_PKT_CNT_BIT)|(length<<DEPTSIZ_XFER_SIZE_BIT), (u32) S3C_UDC_OTG_DIEPTSIZ0+DEP_OFFSET*ep_num);
			writel(virt_to_phys(buf), S3C_UDC_OTG_DIEPDMA0+DEP_OFFSET*ep_num);		
			writel(DEPCTL_EPENA|DEPCTL_CNAK|ctrl , (u32) S3C_UDC_OTG_DIEPCTL0+DEP_OFFSET*ep_num);
			
			DEBUG_IN_EP("[%s] :TX DMA start : DIEPDMA[%d] = 0x%x, DIEPTSIZ[%d] = 0x%x, DIEPCTL[%d] = 0x%x,"
					 "pktcnt = %d, xfersize = %d\n", __FUNCTION__, 
					ep_num, readl(S3C_UDC_OTG_DIEPDMA0+DEP_OFFSET*ep_num),
					ep_num, readl(S3C_UDC_OTG_DIEPTSIZ0+DEP_OFFSET*ep_num),
					ep_num, readl(S3C_UDC_OTG_DIEPCTL0+DEP_OFFSET*ep_num),
					pktcnt, length);
			switch(ep_num)
			{
				case EP2_IN:
				case EP5_IN:
				case EP8_IN:
					add_ep0_next_ep(ep_num);
					break;
				case EP3_IN:
					if (ep->dev->config_gadget_driver == ETHER_RNDIS)
						add_ep0_next_ep(ep_num);
					break;
			}
			break;
		default:
			printk("[%s]: Error Unused EP[%d]\n", __FUNCTION__, ep_num);
	}
	return length;
}

static void complete_rx(struct s3c_udc *dev, u32 ep_num)
{
	struct s3c_ep *ep = &dev->ep[ep_num];
	struct s3c_request *req = NULL;
	u32 csr = 0, count_bytes=0, xfer_length, is_short = 0;
	DEBUG_OUT_EP("%s EP [%d]\n",__FUNCTION__, ep_num);

	if (list_empty(&ep->queue)) {
		DEBUG_OUT_EP("[%s] NULL REQ on OUT EP-%d\n", __FUNCTION__, ep_num);
		return;
	}
	req = list_entry(ep->queue.next,	struct s3c_request, queue);

	switch(ep_num)
	{
		case EP0_CON: 
			csr = readl(S3C_UDC_OTG_DOEPTSIZ0);
			count_bytes = (csr & 0x7f);
			break;
		case EP1_OUT: 
		case EP4_OUT: 			
		case EP7_OUT: 			
			csr = readl(S3C_UDC_OTG_DOEPTSIZ0+DEP_OFFSET*ep_num);
			count_bytes = (csr & 0x7fff);
			break;
		default:
			printk("[%s]: Error Unused EP[%d]\n", __FUNCTION__, ep_num);
	}

	dma_cache_maint(req->req.buf, req->req.length, DMA_FROM_DEVICE);
	xfer_length = req->req.length-count_bytes;
	req->req.actual += min(xfer_length, req->req.length-req->req.actual);
	is_short = (xfer_length < ep->ep.maxpacket);

	DEBUG_OUT_EP("[%s] RX DMA done : %d/%d bytes received%s, DOEPTSIZ = 0x%x, %d bytes remained\n",
			__FUNCTION__, req->req.actual, req->req.length,
			is_short ? "/S" : "", csr, count_bytes);
	DEBUG_OUT_EP("%s : req->length = %d / req->actual = %d / xfer_length = %d / ep.maxpacket = %d / is_short = %d \n",
			__FUNCTION__, req->req.length,req->req.actual,
			xfer_length,ep->ep.maxpacket, is_short);

	if (is_short || req->req.actual == xfer_length) 
	{
		if(ep_num == EP0_CON && dev->ep0state == DATA_STATE_RECV)
		{
			dev->ep0state = WAIT_FOR_SETUP;
			s3c_send_zlp();	
		}
		else
		{
			done(ep, req, 0);

			if(!list_empty(&ep->queue)) 
			{
				req = list_entry(ep->queue.next, struct s3c_request, queue);
				DEBUG_OUT_EP("[%s] Next Rx request start...\n", __FUNCTION__);
				setdma_rx(ep, req);
			}
		}
	}
}

static void complete_tx(struct s3c_udc *dev, u32 ep_num)
{
	struct s3c_ep *ep = &dev->ep[ep_num];
	struct s3c_request *req;
	u32 count_bytes = 0;

	if (list_empty(&ep->queue)) 
	{
		DEBUG_IN_EP("[%s] : NULL REQ on IN EP-%d\n", __FUNCTION__, ep_num);
		return;

	}

	req = list_entry(ep->queue.next, struct s3c_request, queue);

	if(ep_num == EP0_CON && dev->ep0state == DATA_STATE_XMIT)
	{
		u32 last = write_fifo_ep0(ep, req);
		if(last)
		{
			dev->ep0state = WAIT_FOR_SETUP;
		}
		return;
	}

	switch(ep_num)
	{
		case EP0_CON:
			count_bytes = (readl(S3C_UDC_OTG_DIEPTSIZ0)) & 0x7f;
			req->req.actual = req->req.length-count_bytes;
			DEBUG_IN_EP("[%s] : TX DMA done : %d/%d bytes sent, DIEPTSIZ0 = 0x%x\n",
					__FUNCTION__, req->req.actual, req->req.length,
					readl(S3C_UDC_OTG_DIEPTSIZ0));
			break;
		case EP2_IN:
		case EP3_IN:
		case EP5_IN:
		case EP6_IN:						
		case EP8_IN:
		case EP9_IN:						
			count_bytes = (readl(S3C_UDC_OTG_DIEPTSIZ0+DEP_OFFSET*ep_num)) & 0x7fff;
			req->req.actual = req->req.length-count_bytes;
			DEBUG_IN_EP("[%s] : TX DMA done : %d/%d bytes sent, DIEPTSIZ[%d] = 0x%x\n",
					__FUNCTION__, req->req.actual, req->req.length,
					ep_num, readl(S3C_UDC_OTG_DIEPTSIZ0+DEP_OFFSET*ep_num));
			break;
		default:
			printk("[%s]: Error Unused EP[%d]\n", __FUNCTION__, ep_num);
	}

	if (req->req.actual == req->req.length) 
	{
		done(ep, req, 0);
		DEBUG_IN_EP("[%s] : After done EP [%d]\n", __FUNCTION__, ep_num);

		if(!list_empty(&ep->queue)) 
		{
			req = list_entry(ep->queue.next, struct s3c_request, queue);
			DEBUG_IN_EP("[%s] : Next Tx request start...\n", __FUNCTION__);
			setdma_tx(ep, req);
		}
	}
}

/*
 *	done - retire a request; caller blocked irqs
 */
static void done(struct s3c_ep *ep, struct s3c_request *req, int status)
{
	u32 stopped = ep->stopped;
	u8 	ep_num = ep_index(ep);

	pr_debug("[%s] %s %p, req = %p, stopped = %d\n",__FUNCTION__, ep->ep.name, ep, &req->req, stopped);
	list_del_init(&req->queue);

	if (likely(req->req.status == -EINPROGRESS))
		req->req.status = status;
	else
		status = req->req.status;

	if (status && status != -ESHUTDOWN)
	{
		pr_debug("complete %s req %p stat %d len %u/%u\n",
			ep->ep.name, &req->req, status,
			req->req.actual, req->req.length);
	}

	/* don't modify queue heads during completion callback */
	ep->stopped = 1;

	spin_unlock(&ep->dev->lock);
	req->req.complete(&ep->ep, &req->req);
	spin_lock(&ep->dev->lock);

	ep->stopped = stopped;

	if(ep_num != get_ep0_next_ep())
		return;

	del_ep0_next_ep(ep_num);
	s3c_set_next_ep(EP0_CON, get_ep0_next_ep());
}

/*
 * 	nuke - dequeue ALL requests
 */
void nuke(struct s3c_ep *ep, int status)
{
	struct s3c_request *req;

	pr_debug("[%s] %s %p\n", __FUNCTION__, ep->ep.name, ep);

	/* called with irqs blocked */
	while (!list_empty(&ep->queue)) 
	{
		req = list_entry(ep->queue.next, struct s3c_request, queue);
		done(ep, req, status);
	}
}

static void set_disconnect_state(struct s3c_udc *dev)
{
	u8 i;
	
	dev->gadget.speed = USB_SPEED_UNKNOWN;
	
	/* prevent new request submissions, kill any outstanding requests  */
	for (i = 0; i < S3C_MAX_ENDPOINTS; i++) 
	{
		struct s3c_ep *ep = &dev->ep[i];
		ep->stopped = 1;
		nuke(ep, -ESHUTDOWN);
	}

	/* report disconnect; the driver is already quiesced */
	DEBUG_SETUP("disconnect, gadget %s\n", dev->driver->driver.name);
	if (dev->driver && dev->driver->disconnect) {  // modify hyunsoo
		if(dev->lock_enable)
			spin_unlock(&dev->lock);		/*added infor by lvcha*/
		else
			spin_lock(&dev->lock);
		
		dev->driver->disconnect(&dev->gadget);

		if(dev->lock_enable)
			spin_lock(&dev->lock);
		else
			spin_unlock(&dev->lock);	
	}

	/* re-init driver-visible data structures */
	udc_ep_list_reinit(dev);			
}

static void stop_activity(struct s3c_udc *dev, struct usb_gadget_driver *driver)
{
	/* don't disconnect drivers more than once */
	if (dev->gadget.speed == USB_SPEED_UNKNOWN)
		driver = 0;
	set_disconnect_state(dev);
}

static void reconfig_usbd(struct s3c_udc *dev)
{
	// 2. Soft-reset OTG Core and then unreset again.
	writel(CORE_SOFT_RESET, S3C_UDC_OTG_GRSTCTL);

	writel(	0<<15		// PHY Low Power Clock sel
		|1<<14		// Non-Periodic TxFIFO Rewind Enable
		|0x5<<10	// Turnaround time
		|0<<9|0<<8	// [0:HNP disable, 1:HNP enable][ 0:SRP disable, 1:SRP enable] H1= 1,1
		|0<<7		// Ulpi DDR sel
		|0<<6		// 0: high speed utmi+, 1: full speed serial
		|0<<4		// 0: utmi+, 1:ulpi
		|1<<3		// phy i/f  0:8bit, 1:16bit
		|0x7<<0,	// HS/FS Timeout*
		S3C_UDC_OTG_GUSBCFG);

	//only re-connect if not configured
	if(dev->udc_state != USB_STATE_CONFIGURED)
	{
		// 3. Put the OTG device core in the disconnected state.
		s3c_udc_soft_disconnect();
		udelay(20);

		// 4. Make the OTG device core exit from the disconnected state.
		s3c_udc_soft_connect();
	}
	
	// 5. Configure OTG Core to initial settings of device mode.
	writel(1<<18|0x0<<0, S3C_UDC_OTG_DCFG);		// [][1: full speed(30Mhz) 0:high speed]

	mdelay(1);
//	udelay(500);

	// 6. Unmask the core interrupts
	writel(GINTMSK_INIT, S3C_UDC_OTG_GINTMSK);

	// 7. Set NAK bit of EP
	writel(DEPCTL_EPDIS|DEPCTL_SNAK|(0<<0), S3C_UDC_OTG_DOEPCTL0); /* EP0: Control OUT */
	writel(DEPCTL_EPDIS|DEPCTL_SNAK|(0<<0), S3C_UDC_OTG_DIEPCTL0); /* EP0: Control IN */

	writel(DEPCTL_EPDIS|DEPCTL_SNAK|DEPCTL_BULK_TYPE|(0<<0), S3C_UDC_OTG_DOEPCTL1); /* EP1:Data OUT */
	writel(DEPCTL_EPDIS|DEPCTL_SNAK|DEPCTL_BULK_TYPE|EP3_IN<<DIEPCTL_NEXT_EP_BIT |(0<<0), S3C_UDC_OTG_DIEPCTL2); /* EP2:Data IN */
	writel(DEPCTL_EPDIS|DEPCTL_SNAK|DEPCTL_BULK_TYPE|EP0_CON<<DIEPCTL_NEXT_EP_BIT |(0<<0), S3C_UDC_OTG_DIEPCTL3); /* EP3:IN Interrupt*/
	udelay(10);

	writel(DEPCTL_EPDIS|DEPCTL_SNAK|DEPCTL_BULK_TYPE|(0<<0), S3C_UDC_OTG_DOEPCTL4); /* EP4:Data OUT */
	writel(DEPCTL_EPDIS|DEPCTL_SNAK|DEPCTL_BULK_TYPE|EP6_IN<<DIEPCTL_NEXT_EP_BIT |(0<<0), S3C_UDC_OTG_DIEPCTL5); /* EP5:Data IN */
	writel(DEPCTL_EPDIS|DEPCTL_SNAK|DEPCTL_BULK_TYPE|EP0_CON<<DIEPCTL_NEXT_EP_BIT |(0<<0), S3C_UDC_OTG_DIEPCTL6); /* EP6:IN Interrupt*/

	writel(DEPCTL_EPDIS|DEPCTL_SNAK|DEPCTL_BULK_TYPE|(0<<0), S3C_UDC_OTG_DOEPCTL7); /* EP7:Data OUT */
	writel(DEPCTL_EPDIS|DEPCTL_SNAK|DEPCTL_BULK_TYPE|EP9_IN<<DIEPCTL_NEXT_EP_BIT |(0<<0), S3C_UDC_OTG_DIEPCTL8); /* EP8:Data IN */
	writel(DEPCTL_EPDIS|DEPCTL_SNAK|DEPCTL_BULK_TYPE|EP0_CON<<DIEPCTL_NEXT_EP_BIT |(0<<0), S3C_UDC_OTG_DIEPCTL9); /* EP9:IN Interrupt*/

	// 8. Unmask EP interrupts on IN EPs : 0, 2, 3
	//        		      OUT EPs : 0, 1
	writel( (((1<<EP0_CON))<<16) |(1<<EP0_CON), S3C_UDC_OTG_DAINTMSK);

	// 9. Unmask device OUT EP common interrupts
	writel(DOEPMSK_INIT, S3C_UDC_OTG_DOEPMSK);

	// 10. Unmask device IN EP common interrupts
	writel(DIEPMSK_INIT, S3C_UDC_OTG_DIEPMSK);

	// 11. Set Rx FIFO Size
	writel(RX_FIFO_SIZE, S3C_UDC_OTG_GRXFSIZ);

	// 12. Set Non Periodic Tx FIFO Size
	writel(NPTX_FIFO_SIZE<<16| NPTX_FIFO_START_ADDR<<0, S3C_UDC_OTG_GNPTXFSIZ);

	// 13. Clear NAK bit of EP0, EP1, EP2
	// For Slave mode
	writel(DEPCTL_EPDIS|DEPCTL_CNAK|(0<<0), S3C_UDC_OTG_DOEPCTL0); /* EP0: Control OUT */

	// 14. Initialize OTG Link Core.
	writel(GAHBCFG_INIT, S3C_UDC_OTG_GAHBCFG);

	INIT_LIST_HEAD(&ep0_nextep_list.list);

	//change default next ep setting regarding gadget driver configured
	if(dev->udc_state == USB_STATE_CONFIGURED)
	{		
		if(dev->config_gadget_driver == ETHER_CDC)
		{			
			s3c_set_next_ep(EP2_IN, EP0_CON);
		}
		if(dev->config_gadget_driver == ANDROID_ADB ||
		    dev->config_gadget_driver == ANDROID_ADB_UMS ||
			dev->config_gadget_driver == ANDROID_ADB_UMS_ACM)
		{			
			printk("[%s] Android USB Composite \n", __FUNCTION__);
			s3c_set_next_ep(EP2_IN, EP0_CON);
			s3c_set_next_ep(EP5_IN, EP0_CON);
			s3c_set_next_ep(EP8_IN, EP0_CON);
		}
	}	
}

void set_max_pktsize(struct s3c_udc *dev, enum usb_device_speed speed)
{
	u32 ep_ctrl;

	if (speed == USB_SPEED_HIGH) 
	{
		ep0_fifo_size = 64;
		ep_fifo_size = 512;
		ep_fifo_size2 = 1024;
		dev->gadget.speed = USB_SPEED_HIGH;
	} 
	else 
	{
		ep0_fifo_size = 64;
		ep_fifo_size = 64;
		ep_fifo_size2 = 64;
		dev->gadget.speed = USB_SPEED_FULL;
	}

	dev->ep[0].ep.maxpacket = ep0_fifo_size;
	dev->ep[1].ep.maxpacket = ep_fifo_size;
	dev->ep[2].ep.maxpacket = ep_fifo_size;
	//dev->ep[3].ep.maxpacket = ep_fifo_size;
	dev->ep[3].ep.maxpacket = 16;
	dev->ep[4].ep.maxpacket = ep_fifo_size;
	dev->ep[5].ep.maxpacket = ep_fifo_size;
	dev->ep[6].ep.maxpacket = 16;
	dev->ep[7].ep.maxpacket = ep_fifo_size;
	dev->ep[8].ep.maxpacket = ep_fifo_size;
	dev->ep[9].ep.maxpacket = 16;


	if (speed == USB_SPEED_HIGH) 
	{
		// EP0 - Control IN (64 bytes)
		ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL0);
		writel(ep_ctrl|(0<<0), (u32) S3C_UDC_OTG_DIEPCTL0);

		// EP0 - Control OUT (64 bytes)
		ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
		writel(ep_ctrl|(0<<0), (u32) S3C_UDC_OTG_DOEPCTL0);
	} 
	else 
	{
		// EP0 - Control IN (8 bytes)
		ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL0);
		writel(ep_ctrl|(3<<0), (u32) S3C_UDC_OTG_DIEPCTL0);

		// EP0 - Control OUT (8 bytes)
		ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
		writel(ep_ctrl|(3<<0), (u32) S3C_UDC_OTG_DOEPCTL0);
	}


	// EP1 - Bulk Data OUT (512 bytes)
	ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL1);
	writel(ep_ctrl|(ep_fifo_size<<0), (u32) S3C_UDC_OTG_DOEPCTL1);

	// EP2 - Bulk Data IN (512 bytes)
	ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL2);
	writel(ep_ctrl|(ep_fifo_size<<0), (u32) S3C_UDC_OTG_DIEPCTL2);

	// EP3 - INTR Data IN (512 bytes)
	ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL3);
	writel(ep_ctrl|(16<<0), (u32) S3C_UDC_OTG_DIEPCTL3);	

	// EP4 - Bulk Data OUT (512 bytes)
	ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL4);
	writel(ep_ctrl|(ep_fifo_size<<0), (u32) S3C_UDC_OTG_DOEPCTL4);

	// EP5 - Bulk Data IN (512 bytes)
	ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL5);
	writel(ep_ctrl|(ep_fifo_size<<0), (u32) S3C_UDC_OTG_DIEPCTL5);

	// EP6 - INTR Data IN (512 bytes)
	ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL6);
	writel(ep_ctrl|(16<<0), (u32) S3C_UDC_OTG_DIEPCTL6);

	// EP7 - Bulk Data OUT (512 bytes)
	ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL7);
	writel(ep_ctrl|(ep_fifo_size<<0), (u32) S3C_UDC_OTG_DOEPCTL7);

	// EP8 - Bulk Data IN (512 bytes)
	ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL8);
	writel(ep_ctrl|(ep_fifo_size<<0), (u32) S3C_UDC_OTG_DIEPCTL8);

	// EP9 - INTR Data IN (512 bytes)
	ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL9);
	writel(ep_ctrl|(16<<0), (u32) S3C_UDC_OTG_DIEPCTL9);

}

static void handle_disconnect_intr(struct s3c_udc *dev)
{
	writel(INT_DISCONN, S3C_UDC_OTG_GINTSTS);
	
	if (dev->gadget.speed != USB_SPEED_UNKNOWN) 
	{
		set_disconnect_state(dev);
	}
}

static void handle_enum_done_intr(struct s3c_udc *dev)
{
	u32	usb_status = (readl(S3C_UDC_OTG_DSTS) & 0x6);
	writel(INT_ENUMDONE, S3C_UDC_OTG_GINTSTS);
	
	if (usb_status & (USB_FULL_30_60MHZ | USB_FULL_48MHZ)) 
	{
		DEBUG_SETUP("	 %s: Full Speed Detection\n",__FUNCTION__);
		set_max_pktsize(dev, USB_SPEED_FULL);
	
	} 
	else 
	{
		DEBUG_SETUP("	 %s: High Speed Detection,  DSTS: 0x%x\n", __FUNCTION__, usb_status);
		set_max_pktsize(dev, USB_SPEED_HIGH);
	}
}


static void handle_reset_intr(struct s3c_udc *dev)
{	
	u32	usb_status = readl(S3C_UDC_OTG_GOTGCTL);
	writel(INT_RESET, S3C_UDC_OTG_GINTSTS);
	DEBUG_SETUP("[%s] : Reset interrupt - (GOTGCTL):0x%x\n", __FUNCTION__, usb_status);

	if((usb_status & 0xc0000) == (0x3 << 18)) 
	{
		if(g_reset_available) 
		{
			DEBUG_SETUP("     ===> OTG core got reset (%d)!! \n", g_reset_available);
			
			reconfig_usbd(dev);

			dev->ep0state = WAIT_FOR_SETUP;
			g_reset_available = 0;

			writel((1 << 19)|sizeof(struct usb_ctrlrequest), S3C_UDC_OTG_DOEPTSIZ0);
			writel(virt_to_phys(&ctrl), S3C_UDC_OTG_DOEPDMA0);
			writel(DEPCTL_EPENA |DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
		}
		
		// for reconnecting by ss1 specific to android_adb		
		if (dev->config_gadget_driver == ANDROID_ADB ||
			dev->config_gadget_driver == ANDROID_ADB_UMS ||
			dev->config_gadget_driver == ANDROID_ADB_UMS_ACM )
		{
			if (dev->gadget.speed != USB_SPEED_UNKNOWN) 
			{
				set_disconnect_state(dev);
			}
		}
	} 
	else 
	{
		g_reset_available = 1;
		DEBUG_SETUP("      RESET handling skipped : g_reset_available : %d\n", g_reset_available);
	}
	
	dev->udc_state = USB_STATE_DEFAULT;
}

static void handle_resume_intr(struct s3c_udc *dev)
{
	writel(INT_RESUME, S3C_UDC_OTG_GINTSTS);
	//	s3c_udc_resume_clock_gating();
	
	dev->udc_state = dev->udc_resume_state;

	if (dev->gadget.speed != USB_SPEED_UNKNOWN
		&& dev->driver && dev->driver->resume) 
	{
		spin_unlock(&dev->lock);
		dev->driver->resume(&dev->gadget);
		spin_lock(&dev->lock);
	}
}

static void handle_suspend_intr(struct s3c_udc *dev)
{	
	u32 usb_status;
	
	writel(INT_SUSPEND, S3C_UDC_OTG_GINTSTS);
	
	//confirm device is attached or not
	if(dev->udc_state == USB_STATE_NOTATTACHED ||
		dev->udc_state == USB_STATE_POWERED ||
		dev->udc_state == USB_STATE_SUSPENDED )
	{
		DEBUG_PM("[%s]: not proper state to go into the suspend mode\n", __FUNCTION__);
		return;
	}
	
	usb_status = readl(S3C_UDC_OTG_DSTS);
	
	if ( !(usb_status & (1<<SUSPEND_STS)) )
	{
		DEBUG_PM("[%s]: not suspend !~\n", __FUNCTION__);
		return;
	}	

	if (dev->gadget.speed != USB_SPEED_UNKNOWN)
	{
		DEBUG_PM("[%s]: go to the suspend mode (DSTS):0x%x\n", __FUNCTION__, usb_status);\
			
		dev->udc_resume_state = dev->udc_state;
		dev->udc_state = USB_STATE_SUSPENDED;
		
		//s3c_udc_suspend_clock_gating();
		if (dev->driver && dev->driver->suspend) 
		{
			spin_unlock(&dev->lock);
			dev->driver->suspend(&dev->gadget);
			spin_lock(&dev->lock);			
		}
#ifdef CONFIG_PM		
		s3c_udc_pm_unlock(dev);
#endif
		if (dev->config_gadget_driver == ANDROID_ADB ||
		    dev->config_gadget_driver == ANDROID_ADB_UMS ||
			dev->config_gadget_driver == ANDROID_ADB_UMS_ACM)
		{
			set_disconnect_state(dev);
		}
	}
}

static int handle_ep_in_intr(struct s3c_udc *dev)
{
	u32 ep_int, ep_int_status, ep_ctrl;
	u32 nptxQ_SpcAvail, nptxFifo_SpcAvail, gnptxsts;
	u8	ep_num = SUPPORTING_MAX_EP_NUM + 1, i;

	gnptxsts = readl(S3C_UDC_OTG_GNPTXSTS);
	
	nptxQ_SpcAvail = (gnptxsts & (0xff<<16))>>16;
	nptxFifo_SpcAvail = gnptxsts & 0xffff;
	
	DEBUG_IN_EP("	 GNPTXSTS nptxQ_SpcAvail = %d, nptxFifo_SpcAvail = %d\n",nptxQ_SpcAvail ,nptxFifo_SpcAvail);
	
	if (nptxQ_SpcAvail == 0 || nptxFifo_SpcAvail == 0)
	{
		printk("[%s] : nptxQ_SpcAvail == 0 || nptxFifo_SpcAvail == 0 \n", __FUNCTION__);
		reconfig_usbd(dev);
		return 0;
	}
	
	/*
		The below delaying is specific to g_serial for handling 'ls' command in 
		directory which has many files
	*/
	if(dev->config_gadget_driver == SERIAL || dev->config_gadget_driver == CDC2 )
	{			
	#if G_SERIAL_DELAY 
	#if OTG_DBG_ENABLE
		#ifdef DEBUG_S3C_UDC_IN_EP
					DEBUG_IN_EP("[%s] : EP In for g_serial or g_cdc\n", __FUNCTION__);
		#else
					printk("[%s] : EP In for g_serial or g_cdc\n", __FUNCTION__);
		#endif			
	#else
					udelay(800);
	#endif //OTG_DBG_ENABLE
	#endif //G_SERIAL_DELAY
	}
		
	ep_int = readl(S3C_UDC_OTG_DAINT);
	DEBUG_IN_EP("\tDAINT : 0x%x \n", ep_int);

	for(i=0; i<=SUPPORTING_MAX_EP_NUM; i++)
	{
		if (ep_int & (1<<i)) 
		{
			ep_num = i;
			break;
		}
	}
	

	switch(ep_num)
	{
		case EP0_CON:
		case EP2_IN:
		case EP3_IN:
		case EP5_IN:
		case EP6_IN:			
			ep_int_status = readl(S3C_UDC_OTG_DIEPINT0+DEP_OFFSET*ep_num);
			ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL0+DEP_OFFSET*ep_num);
			DEBUG_IN_EP("\tEP[%d]-IN : DIEPINT[%d] = 0x%x, DIEPCTL[%d] = 0x%x \n", 
				ep_num, ep_num, ep_int_status, ep_num,  ep_ctrl);
			writel(ep_int_status, S3C_UDC_OTG_DIEPINT0+DEP_OFFSET*ep_num);		// Interrupt Clear
			
			if (ep_int_status & TRANSFER_DONE) 
			{
				DEBUG_IN_EP("\tEP[%d]-IN transaction completed - (TX DMA done)\n", ep_num);
			
				complete_tx(dev, ep_num);
			
				if(ep_num == EP0_CON && dev->ep0state == WAIT_FOR_SETUP)
				{
					ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
					
					writel((1 << 19)|sizeof(struct usb_ctrlrequest), S3C_UDC_OTG_DOEPTSIZ0);
					writel(virt_to_phys(&ctrl), S3C_UDC_OTG_DOEPDMA0);
					writel(ep_ctrl|DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
				}
			}
			break;
		default:
			printk("[%s]: Error Unused EP[%d]\n", __FUNCTION__, ep_num);
			if (ep_num > SUPPORTING_MAX_EP_NUM) 
			{
				printk("[%s]: No matching EP DAINT : 0x%x \n", __FUNCTION__, ep_int);
				return -1;
			}
	} //switch
	return 0;
}

static void handle_ep_out_intr(struct s3c_udc * dev)
{
	u32 ep_int, ep_int_status, ep_ctrl;
	u8	ep_num = SUPPORTING_MAX_EP_NUM + 1, i;

	ep_int = readl(S3C_UDC_OTG_DAINT);
	DEBUG_OUT_EP("\tDAINT : 0x%x \n", ep_int);
	
	for(i=0; i<=SUPPORTING_MAX_EP_NUM; i++)
		if (ep_int & ((1<<i)<<16)) 
		{
			ep_num = i;
			break;
		}
	
	if (ep_num > SUPPORTING_MAX_EP_NUM) 
	{
		printk("[%s]: No matching EP DAINT : 0x%x \n", __FUNCTION__, ep_int);
		return;
	}

	switch(ep_num)
	{
		case EP0_CON: 
			ep_int_status = readl(S3C_UDC_OTG_DOEPINT0);
			ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
			DEBUG_EP0("\tEP0-OUT : DOEPINT0 = 0x%x, DOEPCTL0 = 0x%x\n", ep_int_status, ep_ctrl);
			
			if (ep_int_status & CTRL_OUT_EP_SETUP_PHASE_DONE) 
			{
				DEBUG_EP0("\tSETUP packet(transaction) arrived\n");
				s3c_handle_ep0(dev);
				writel(CTRL_OUT_EP_SETUP_PHASE_DONE, S3C_UDC_OTG_DOEPINT0); // Interrupt Clear
			}
			else if (ep_int_status & TRANSFER_DONE) 
			{
				if (dev->ep0state == WAIT_FOR_SETUP) 
				{
					complete_rx(dev, EP0_CON);
					writel((1 << 19)|sizeof(struct usb_ctrlrequest), S3C_UDC_OTG_DOEPTSIZ0);
					writel(virt_to_phys(&ctrl), S3C_UDC_OTG_DOEPDMA0);
					writel(ep_ctrl|DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0); // ep0 OUT enable, clear nak
					dev->ep0state = WAIT_FOR_SETUP;
				} 
				else 
				{
					complete_rx(dev, EP0_CON);
				}
				writel(TRANSFER_DONE, S3C_UDC_OTG_DOEPINT0);	// Interrupt Clear
			}
			else
			{
				writel(ep_int_status, S3C_UDC_OTG_DOEPINT0);	// Interrupt Clear
			}
			break;
		case EP1_OUT: 
		case EP4_OUT:			
			ep_int_status = readl(S3C_UDC_OTG_DOEPINT0+DEP_OFFSET*ep_num);
			ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0+DEP_OFFSET*ep_num);
			
			DEBUG_OUT_EP("\tEP[%d]-OUT : DOEPINT[%d] = 0x%x, DOEPCTL[%d] = 0x%x\n", 
				ep_num, ep_num, ep_int_status, ep_num, ep_ctrl);
			
			if (ep_int_status & TRANSFER_DONE) 
			{
				DEBUG_OUT_EP("\tBULK OUT packet(transaction) arrived - (RX DMA done)\n");
				complete_rx(dev, ep_num);
			}
			writel(ep_int_status, S3C_UDC_OTG_DOEPINT0+DEP_OFFSET*ep_num);		// ep1 Interrupt Clear
			break;
		default:
			printk("[%s]: Error Unused EP[%d]\n", __FUNCTION__, ep_num);
	}
}

static irqreturn_t s3c_udc_irq(int irq, void *_dev)
{
	struct s3c_udc *dev = _dev;
	u32 intr_status, gintmsk;
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);

	dev->lock_enable = 1;	//added by lvcha
	
	intr_status = readl(S3C_UDC_OTG_GINTSTS);
	gintmsk = readl(S3C_UDC_OTG_GINTMSK);

	DEBUG_ISR("\n**** %s : GINTSTS=0x%x(on state %s), GINTMSK : 0x%x, DAINT : 0x%x, DAINTMSK : 0x%x\n",
			__FUNCTION__, intr_status, state_names[dev->ep0state], gintmsk, 
			readl(S3C_UDC_OTG_DAINT), readl(S3C_UDC_OTG_DAINTMSK));

	if (intr_status & INT_IN_EP) 
	{
		DEBUG_IN_EP("[%s] : EP In interrupt \n", __FUNCTION__);
		if(handle_ep_in_intr(dev) == -1)
			goto FAIL_OUT;
		goto	OK_OUT;
	}

	if(intr_status & INT_OUT_EP)
	{
		DEBUG_OUT_EP("[%s] : EP Out interrupt \n", __FUNCTION__);
		handle_ep_out_intr(dev);
		goto	OK_OUT;
	}
	
	if (intr_status & INT_RESET) {
		handle_reset_intr(dev);			
		goto	OK_OUT;
	}

	if (intr_status & INT_ENUMDONE) 
	{
		DEBUG_SETUP("[%s] : Enumeration Done interrupt\n",	__FUNCTION__);
		handle_enum_done_intr(dev);
		goto	OK_OUT;
	}

	//Disconnect interrupt has not yet happen even though enabled
	if (intr_status & INT_DISCONN) 
	{
		printk("[%s] :Disconnect interrupt\n", __FUNCTION__);
		handle_disconnect_intr(dev);
		goto	OK_OUT;
	}
/*
	if (intr_status & INT_EARLY_SUSPEND) 
	{
		DEBUG_PM("[%s] Early suspend interrupt\n", __FUNCTION__);
		writel(INT_EARLY_SUSPEND, S3C_UDC_OTG_GINTSTS);
		goto	OK_OUT;
	}
*/
	if (intr_status & INT_SUSPEND) 
	{
		DEBUG_PM("[%s] Suspend interrupt\n", __FUNCTION__);
    
		if (intr_status & INT_EARLY_SUSPEND) 
	  {
		    DEBUG_PM("[%s] Early suspend status \n", __FUNCTION__);
		    writel(INT_EARLY_SUSPEND, S3C_UDC_OTG_GINTSTS);
				g_reset_available = 1;
		    goto	OK_OUT;
	  }

		handle_suspend_intr(dev);
		goto	OK_OUT;
	}
	
	if (intr_status & INT_RESUME) 
	{
		DEBUG_PM("[%s] Resume interrupt\n", __FUNCTION__);
		handle_resume_intr(dev);
		goto	OK_OUT;
	}
	
	if (intr_status) 
		printk("no handler for S3C_UDC_OTG_GINTSTS [%d]\n", intr_status);
	else
		printk("no S3C_UDC_OTG_GINTSTS( == 0)\n");

	goto	FAIL_OUT;

	
OK_OUT:
	spin_unlock_irqrestore(&dev->lock, flags);
	dev->lock_enable = 0; //added by lvcha
	return IRQ_HANDLED;

FAIL_OUT:
	spin_unlock_irqrestore(&dev->lock, flags);
	dev->lock_enable = 0; //added by lvcha
	return IRQ_NONE;
}

static int s3c_ep_enable(struct usb_ep *_ep,
			     const struct usb_endpoint_descriptor *desc)
{
	struct s3c_ep *ep;
	struct s3c_udc *dev;
	unsigned long flags;
	
	u32 ep_ctrl, daintmsk;
	u8  ep_num;
	
	ep = container_of(_ep, struct s3c_ep, ep);

	pr_debug("[%s] EP-%d\n", __FUNCTION__, ep_index(ep));

	if (!_ep || !desc || ep->desc || _ep->name == ep0name
	    || desc->bDescriptorType != USB_DT_ENDPOINT
	    || ep->bEndpointAddress != desc->bEndpointAddress
	    || ep_maxpacket(ep) < le16_to_cpu(desc->wMaxPacketSize)) 
	{
		printk("[%s] bad ep or descriptor\n", __FUNCTION__);
		return -EINVAL;
	}

	/* xfer types must match, except that interrupt ~= bulk */
	if (ep->bmAttributes != desc->bmAttributes
	    && ep->bmAttributes != USB_ENDPOINT_XFER_BULK
	    && desc->bmAttributes != USB_ENDPOINT_XFER_INT) 
	{
		pr_debug("[%s] %s type mismatch\n", __FUNCTION__, _ep->name);
		return -EINVAL;
	}

	/* hardware _could_ do smaller, but driver doesn't */
	if ((desc->bmAttributes == USB_ENDPOINT_XFER_BULK
	     && le16_to_cpu(desc->wMaxPacketSize) != ep_maxpacket(ep))
	    || !desc->wMaxPacketSize) 
	{
		pr_debug("[%s] bad %s maxpacket\n", __FUNCTION__, _ep->name);
		return -ERANGE;
	}

	dev = ep->dev;
	if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN) 
	{
		pr_debug("[%s] bogus device state\n", __FUNCTION__);
		return -ESHUTDOWN;
	}

	spin_lock_irqsave(&ep->dev->lock, flags);

	ep->stopped = 0;
	ep->desc = desc;
	ep->ep.maxpacket = le16_to_cpu(desc->wMaxPacketSize);

	/* Reset halt state */
	s3c_set_halt(_ep, 0);

	daintmsk = readl(S3C_UDC_OTG_DAINTMSK);

	ep_num = ep_index(ep);
	if(ep_num <= SUPPORTING_MAX_EP_NUM)
	{
		if(ep_is_in(ep))
		{
			/* EP: IN */
			ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL0+DEP_OFFSET*ep_num);
			writel(DEPCTL_USBACTEP|ep_ctrl, S3C_UDC_OTG_DIEPCTL0+DEP_OFFSET*ep_num); 
			writel(daintmsk|(1<<ep_num), S3C_UDC_OTG_DAINTMSK);
		}
		else
		{
			/* EP: OUT */
			ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0+DEP_OFFSET*ep_num);
			writel(DEPCTL_USBACTEP|ep_ctrl, S3C_UDC_OTG_DOEPCTL0+DEP_OFFSET*ep_num);
			writel(daintmsk|(1<<(ep_num+16)), S3C_UDC_OTG_DAINTMSK);
		}
	}
	else
	{
		printk("[%s] not supported ep %d\n",__FUNCTION__, ep_num); 		
	}
	spin_unlock_irqrestore(&ep->dev->lock, flags);

	pr_debug("[%s] enabled %s, stopped = %d, maxpacket = %d\n",
		__FUNCTION__, _ep->name, ep->stopped, ep->ep.maxpacket);
	return 0;
}

/** Disable EP
 */
static int s3c_ep_disable(struct usb_ep *_ep)
{
	struct s3c_ep *ep = NULL;
	unsigned long flags;
	u8 ep_num;
	
	u32 ep_ctrl, daintmsk;

	ep = container_of(_ep, struct s3c_ep, ep);
	
	pr_debug("[%s] EP-%d\n", __FUNCTION__, ep_index(ep));

	if (!_ep || !ep->desc) 
	{
		pr_debug("[%s] %s not enabled\n", __FUNCTION__,
		      _ep ? ep->ep.name : NULL);
		return -EINVAL;
	}
	
	ep_num = ep_index(ep);

	spin_lock_irqsave(&ep->dev->lock, flags);

	/* Nuke all pending requests */
	nuke(ep, -ESHUTDOWN);

	ep->desc = 0;
	ep->stopped = 1;
	
	daintmsk = readl(S3C_UDC_OTG_DAINTMSK);

	del_one_ep_list_from_ep0_next_ep(ep_num);
	
	if(ep_num <= SUPPORTING_MAX_EP_NUM)
	{
		if(ep_is_in(ep))
		{
			/* EP: IN */
			ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL0+DEP_OFFSET*ep_num);
			writel(ep_ctrl&~(DEPCTL_USBACTEP), S3C_UDC_OTG_DIEPCTL0+DEP_OFFSET*ep_num); 
			writel(daintmsk&(~(1<<ep_num)), S3C_UDC_OTG_DAINTMSK);
		}
		else
		{
			/* EP: OUT */
			ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0+DEP_OFFSET*ep_num);
			writel(ep_ctrl&~(DEPCTL_USBACTEP), S3C_UDC_OTG_DOEPCTL0+DEP_OFFSET*ep_num); 
			writel(daintmsk&(~(1<<(ep_num+16))), S3C_UDC_OTG_DAINTMSK);
		}
	}
	else
	{
		printk("[%s] not supported ep %d\n",__FUNCTION__, ep_num);		
	}
	spin_unlock_irqrestore(&ep->dev->lock, flags);

	pr_debug("[%s] disabled %s\n", __FUNCTION__, _ep->name);
	
	return 0;
}

static struct usb_request *s3c_alloc_request(struct usb_ep *ep,
						 gfp_t gfp_flags)
{
	struct s3c_request *req;

	pr_debug("[%s] %s %p\n", __FUNCTION__, ep->name, ep);

	req = kmalloc(sizeof *req, gfp_flags);
	if (!req)
		return 0;

	memset(req, 0, sizeof *req);
	INIT_LIST_HEAD(&req->queue);

	return &req->req;
}

static void s3c_free_request(struct usb_ep *ep, struct usb_request *_req)
{
	struct s3c_request *req;

	pr_debug("[%s] %p\n", __FUNCTION__, ep);

	req = container_of(_req, struct s3c_request, req);
	WARN_ON(!list_empty(&req->queue));
	kfree(req);
}

/*
 * Queue one request
 *  Kickstart transfer if needed
 */
static int s3c_queue(struct usb_ep *_ep, struct usb_request *_req,
			 gfp_t gfp_flags)
{
	struct s3c_request *req;
	struct s3c_ep *ep;
	struct s3c_udc *dev;
	unsigned long flags;
	u32 ep_num;

	req = container_of(_req, struct s3c_request, req);
	if (unlikely(!_req || !_req->complete || !_req->buf || !list_empty(&req->queue)))
	{
		pr_debug("[%s] bad params\n", __FUNCTION__);
		return -EINVAL;
	}

	ep = container_of(_ep, struct s3c_ep, ep);	
	
	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name))) 
	{
		printk("[%s] : bad ep\n", __FUNCTION__);
		return -EINVAL;
	}

	ep_num = (u32)ep_index(ep);
	dev = ep->dev;
	if (unlikely(!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN)) 
	{
		pr_debug("[%s] bogus device state %p\n", __FUNCTION__, dev->driver);
		return -ESHUTDOWN;
	}

	pr_debug("\n%s: %s queue req %p, len %d buf %p\n",__FUNCTION__, _ep->name, _req, _req->length, _req->buf);

	spin_lock_irqsave(&dev->lock, flags);

	_req->status = -EINPROGRESS;
	_req->actual = 0;

	/* kickstart this i/o queue? */
	pr_debug("[%s] Add to ep=%d, Q empty=%d, stopped=%d\n",__FUNCTION__, ep_num, list_empty(&ep->queue), ep->stopped);

	if (list_empty(&ep->queue) && !ep->stopped) 
	{
		u32 csr;

		if (ep_num == 0) 
		{
			/* EP0 */
			list_add_tail(&req->queue, &ep->queue);
			s3c_ep0_kick(dev, ep);
			req = 0;

		} 
		//EP-IN 
		else if(ep_is_in(ep))// 
		{
			csr = readl((u32) S3C_UDC_OTG_GINTSTS);
			DEBUG_IN_EP("[%s] : ep_is_in, S3C_UDC_OTG_GINTSTS=0x%x\n",
				__FUNCTION__, csr);

			setdma_tx(ep, req);
		} 
		//EP-OUT 
		else 
		{
			csr = readl((u32) S3C_UDC_OTG_GINTSTS);
			DEBUG_OUT_EP("[%s] ep_is_out, S3C_UDC_OTG_GINTSTS=0x%x\n",
				__FUNCTION__, csr);

			setdma_rx(ep, req);
		}
	}

	/* pio or dma irq handler advances the queue. */
	if (likely(req != 0))
	{
		list_add_tail(&req->queue, &ep->queue);
	}

	spin_unlock_irqrestore(&dev->lock, flags);

	return 0;
}

/* dequeue JUST ONE request */
static int s3c_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct s3c_ep *ep;
	struct s3c_request *req;
	unsigned long flags;

	pr_debug("[%s] %p\n", __FUNCTION__, _ep);

	ep = container_of(_ep, struct s3c_ep, ep);
	if (!_ep || ep->ep.name == ep0name)
	{
		pr_debug("[%s] !_ep || !ep\n", __FUNCTION__);
		return -EINVAL;
	}

	spin_lock_irqsave(&ep->dev->lock, flags);

	/* make sure it's actually queued on this endpoint */
	list_for_each_entry(req, &ep->queue, queue) 
	{
		if (&req->req == _req)
			break;
	}
	
	if (&req->req != _req) 
	{
		spin_unlock_irqrestore(&ep->dev->lock, flags);
		return -EINVAL;
	}

	done(ep, req, -ECONNRESET);

	spin_unlock_irqrestore(&ep->dev->lock, flags);
	return 0;
}

/** Halt specific EP
 *  Return 0 if success
 */
static int s3c_set_halt(struct usb_ep *_ep, int value)
{
	return 0;
}

/** Return bytes in EP FIFO
 */
static int s3c_fifo_status(struct usb_ep *_ep)
{
	int count = 0;
	struct s3c_ep *ep;

	ep = container_of(_ep, struct s3c_ep, ep);
	if (!_ep) 
	{
		pr_debug("[%s] bad ep\n", __FUNCTION__);
		return -ENODEV;
	}

	pr_debug("[%s] %d\n", __FUNCTION__, ep_index(ep));

	/* LPD can't report unclaimed bytes from IN fifos */
	if (ep_is_in(ep))
		return -EOPNOTSUPP;

	return count;
}

/** Flush EP FIFO
 */
static void s3c_fifo_flush(struct usb_ep *_ep)
{
	struct s3c_ep *ep;

	ep = container_of(_ep, struct s3c_ep, ep);
	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name))) 
	{
		pr_debug("[%s] bad ep\n", __FUNCTION__);
		return;
	}

	pr_debug("[%s] EP [%d] \n", __FUNCTION__, ep_index(ep));
}

/****************************************************************/
/* End Point 0 related functions                                */
/****************************************************************/

/* return:  0 = still running, 1 = completed, negative = errno */
static int write_fifo_ep0(struct s3c_ep *ep, struct s3c_request *req)
{
	u32 max;
	unsigned count;
	int is_last;

	max = ep_maxpacket(ep);

	DEBUG_EP0("[%s] max = %d\n", __FUNCTION__, max);

	count = setdma_tx(ep, req);

	/* last packet is usually short (or a zlp) */
	if (likely(count != max))
		is_last = 1;
	else 
	{
		if (likely(req->req.length != req->req.actual) || req->req.zero)
			is_last = 0;
		else
			is_last = 1;
	}

	DEBUG_EP0("[%s] wrote %s %d bytes%s %d left %p\n", __FUNCTION__,
		  ep->ep.name, count,
		  is_last ? "/L" : "", req->req.length - req->req.actual, req);

	/* requests complete when all IN data is in the FIFO */
	if (is_last) 
	{
		ep->dev->ep0state = WAIT_FOR_SETUP;
		return 1;
	}

	return 0;
}

static __inline__ int s3c_fifo_read(struct s3c_ep *ep, u32 *cp, int max)
{
	u32 bytes;

	bytes = sizeof(struct usb_ctrlrequest);
	dma_cache_maint(&ctrl, bytes, DMA_FROM_DEVICE);
	DEBUG_EP0("[%s] bytes=%d, ep_index=%d \n", __FUNCTION__, bytes, ep_index(ep));

	return bytes;
}

/**
 * udc_set_address - set the USB address for this device
 * @address:
 *
 * Called from control endpoint function
 * after it decodes a set address setup packet.
 */
static void udc_set_address(struct s3c_udc *dev, unsigned char address)
{
	u32 ctrl = readl(S3C_UDC_OTG_DCFG);
	writel(address << 4 | ctrl, S3C_UDC_OTG_DCFG);

	s3c_send_zlp();

	DEBUG_EP0("[%s] USB OTG 2.0 Device address=%d, DCFG=0x%x\n",
		__FUNCTION__, address, readl(S3C_UDC_OTG_DCFG));

	dev->usb_address = address;
	dev->udc_state = USB_STATE_ADDRESS;
}

static void s3c_ep0_read(struct s3c_udc *dev)
{
	struct s3c_request *req;
	struct s3c_ep *ep = &dev->ep[0];
	int ret;

	if (!list_empty(&ep->queue))
	{
		req = list_entry(ep->queue.next, struct s3c_request, queue);
	}
	else 
	{
		pr_debug("[%s] ---> BUG\n", __FUNCTION__);
		BUG();	//logic ensures		-jassi
		return;
	}

	DEBUG_EP0("[%s] req.length = 0x%x, req.actual = 0x%x\n",
		__FUNCTION__, req->req.length, req->req.actual);

	if(req->req.length == 0) 
	{
		dev->ep0state = WAIT_FOR_SETUP;
		done(ep, req, 0);
		return;
	}

	ret = setdma_rx(ep, req);

	if (ret) 
	{
		dev->ep0state = WAIT_FOR_SETUP;
		done(ep, req, 0);
		return;
	}

}

/*
 * DATA_STATE_XMIT
 */
static int s3c_ep0_write(struct s3c_udc *dev)
{
	struct s3c_request *req;
	struct s3c_ep *ep = &dev->ep[0];
	int ret, need_zlp = 0;

	DEBUG_EP0("[%s] ep0 write\n", __FUNCTION__);

	if (list_empty(&ep->queue))
	{
		req = 0;
	}
	else
	{
		req = list_entry(ep->queue.next, struct s3c_request, queue);
	}

	if (!req) 
	{
		DEBUG_EP0("[%s] NULL REQ\n", __FUNCTION__);
		return 0;
	}

	DEBUG_EP0("[%s] req.length = 0x%x, req.actual = 0x%x\n",
		__FUNCTION__, req->req.length, req->req.actual);

	if (req->req.length - req->req.actual == ep0_fifo_size) 
	{
		/* Next write will end with the packet size, */
		/* so we need Zero-length-packet */
		need_zlp = 1;
	}

	ret = write_fifo_ep0(ep, req);

	if ((ret == 1) && !need_zlp) 
	{
		/* Last packet */
		DEBUG_EP0("[%s] finished, waiting for status\n", __FUNCTION__);
		dev->ep0state = WAIT_FOR_SETUP;
	} 
	else 
	{
		DEBUG_EP0("[%s] not finished\n", __FUNCTION__);
		dev->ep0state = DATA_STATE_XMIT;
	}

	if (need_zlp) {
		DEBUG_EP0("[%s] Need ZLP!\n", __FUNCTION__);
		dev->ep0state = DATA_STATE_NEED_ZLP;
	}
	return 1;
}

static	u16	g_status;

/*
 * WAIT_FOR_SETUP (OUT_PKT_RDY)
 */
static void s3c_ep0_setup(struct s3c_udc *dev)
{
	struct s3c_ep *ep = &dev->ep[0];
	int i, bytes, is_in;
	u32 ep_ctrl, pktcnt;

	/* Nuke all previous transfers */
	nuke(ep, -EPROTO);

	/* read control req from fifo (8 bytes) */
	bytes = s3c_fifo_read(ep, (u32 *)&ctrl, 8);

	DEBUG_SETUP("Read CTRL REQ %d bytes\n", bytes);
	DEBUG_SETUP("  CTRL.bRequestType = 0x%x (is_in %d)\n", ctrl.bRequestType,
		    ctrl.bRequestType & USB_DIR_IN);
	DEBUG_SETUP("  CTRL.bRequest = 0x%x\n", ctrl.bRequest);
	DEBUG_SETUP("  CTRL.wLength = 0x%x\n", ctrl.wLength);
	DEBUG_SETUP("  CTRL.wValue = 0x%x (%d)\n", ctrl.wValue, ctrl.wValue >> 8);
	DEBUG_SETUP("  CTRL.wIndex = 0x%x\n", ctrl.wIndex);

	/* Set direction of EP0 */
	if (likely(ctrl.bRequestType & USB_DIR_IN)) 
	{
		ep->bEndpointAddress |= USB_DIR_IN;
		is_in = 1;
	} 
	else 
	{
		ep->bEndpointAddress &= ~USB_DIR_IN;
		is_in = 0;
	}
	
	/*
		the following is specific to class type request
		should make some logic for req->length == 0 , status w/o data stage
	*/
	if ( (ctrl.bRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS ) 
	{ // CLASS TYPE
		 DEBUG_SETUP("CLASS Type request. ep->bEndpointAddress : 0x%02x\n", ep->bEndpointAddress);
		 
		switch(ctrl.bRequest)
		{
/* 
	g_serial specific 
    in include/linux/usb/cdc.h
*/
#define USB_CDC_REQ_SET_LINE_CODING			0x20
#define USB_CDC_REQ_GET_LINE_CODING			0x21
#define USB_CDC_REQ_SET_CONTROL_LINE_STATE	0x22
			case USB_CDC_REQ_SET_LINE_CODING :
				DEBUG_SETUP("USB_CDC_REQ_SET_LINE_CODING\n");
				//read more 7 bytes data
				goto gadget_setup;
			case USB_CDC_REQ_GET_LINE_CODING :
				DEBUG_SETUP("USB_CDC_REQ_GET_LINE_CODING\n");
				DEBUG_SETUP("modify USB_DIR_IN\n");
				ep->bEndpointAddress |= USB_DIR_IN;
				goto gadget_setup;
			case USB_CDC_REQ_SET_CONTROL_LINE_STATE :				
				DEBUG_SETUP("USB_CDC_REQ_SET_CONTROL_LINE_STATE\n");
				DEBUG_SETUP("modify USB_DIR_IN\n");
				ep->bEndpointAddress |= USB_DIR_IN;
				goto gadget_setup;
		//for g_cdc
#define USB_CDC_SET_ETHERNET_PACKET_FILTER	0x43
			case USB_CDC_SET_ETHERNET_PACKET_FILTER :				
				DEBUG_SETUP("USB_CDC_SET_ETHERNET_PACKET_FILTER\n");
				DEBUG_SETUP("modify USB_DIR_IN\n");
				ep->bEndpointAddress |= USB_DIR_IN;
				goto gadget_setup;
				
/* f_mass_storage Bulk-only class specific requests */
/*
#define USB_BULK_RESET_REQUEST			0xff
#define USB_BULK_GET_MAX_LUN_REQUEST	0xfe
		case USB_BULK_RESET_REQUEST :				
			DEBUG_SETUP("USB_BULK_RESET_REQUEST\n");
			DEBUG_SETUP("modify USB_DIR_IN\n");
				ep->bEndpointAddress |= USB_DIR_IN;
				goto gadget_setup;
		case USB_BULK_GET_MAX_LUN_REQUEST :				
			DEBUG_SETUP("USB_BULK_GET_MAX_LUN_REQUEST\n");
			DEBUG_SETUP("modify USB_DIR_IN\n");
			ep->bEndpointAddress |= USB_DIR_IN;
			goto gadget_setup;				
*/			
		}
	}

	/* Handle some SETUP packets ourselves */
	switch (ctrl.bRequest) {
		case USB_REQ_CLEAR_FEATURE:	//0x01
			if ((ctrl.bRequestType & USB_TYPE_MASK)
					!= (USB_TYPE_STANDARD))
			{
				DEBUG_SETUP("[%s] bRequestType != (USB_DIR_IN | USB_TYPE_STANDARD) : delegated !!!  \n",__FUNCTION__);
				break;
			}
			
			if ((ctrl.bRequestType & USB_RECIP_MASK)
					!= USB_RECIP_ENDPOINT)
			{
				DEBUG_SETUP("[%s] bRequestType != USB_RECIP_ENDPOINT : delegated !!!  \n",__FUNCTION__);
				break;
			}
			DEBUG_SETUP("CLEAR FEATURE request.\n");
			//need to implement real work for request instead of just sending zlp
			s3c_send_zlp();
			return; 
			
		case USB_REQ_SET_ADDRESS:
			if (ctrl.bRequestType != (USB_TYPE_STANDARD | USB_RECIP_DEVICE))
				break;
			DEBUG_SETUP("============================================\n");
			DEBUG_SETUP("[%s] *** USB_REQ_SET_ADDRESS (%d)\n",
					__FUNCTION__, ctrl.wValue);
			DEBUG_SETUP("============================================\n");
			udc_set_address(dev, ctrl.wValue);
			return;

		case USB_REQ_SET_CONFIGURATION :
			DEBUG_SETUP("============================================\n");
			DEBUG_SETUP("[%s] USB_REQ_SET_CONFIGURATION (%d)\n",
					__FUNCTION__, ctrl.wValue);
			DEBUG_SETUP("============================================\n");
config_change:
			//gadget will queue zlp
			DEBUG_SETUP("modify USB_DIR_IN\n");
			ep->bEndpointAddress |= USB_DIR_IN;
			g_reset_available = 1;
		
			/* g_ether's CDC config => 1, RNDIS => 2 */
			if (strcmp(dev->driver->driver.name, "g_ether") == 0 )
			{
				switch(ctrl.wValue )
				{
					case 1:	//CDC
						dev->config_gadget_driver = ETHER_CDC;
						s3c_set_next_ep(EP2_IN, EP0_CON);
						break;
					case 2: //RNDIS
						dev->config_gadget_driver = ETHER_RNDIS;
						break;
					default:
						printk("[%s]not proper ctrl.wValue[%d]\n", __FUNCTION__, ctrl.wValue );
				}
			}
			dev->udc_state = USB_STATE_CONFIGURED;

		#ifdef CONFIG_PM			
			s3c_udc_pm_lock(dev);
		#endif
			
			break;

		case USB_REQ_GET_DESCRIPTOR:
			DEBUG_SETUP("[%s] *** USB_REQ_GET_DESCRIPTOR  \n",__FUNCTION__);
			break;
			
		case USB_REQ_SET_INTERFACE:
			//confirm USB_RECIP_INTERFACE?
			if ((ctrl.bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
				break;
			DEBUG_SETUP("[%s] *** USB_REQ_SET_INTERFACE (%d)\n",
					__FUNCTION__, ctrl.wValue);
	
			goto config_change;
			
		case USB_REQ_GET_CONFIGURATION:
			DEBUG_SETUP("[%s] *** USB_REQ_GET_CONFIGURATION  \n",__FUNCTION__);
			break;

		case USB_REQ_GET_STATUS:
			DEBUG_SETUP("[%s] *** USB_REQ_GET_STATUS  \n",__FUNCTION__);

			if ((ctrl.bRequestType & (USB_DIR_IN | USB_TYPE_MASK))
					!= (USB_DIR_IN | USB_TYPE_STANDARD))
			{
				DEBUG_SETUP("[%s] *** USB_REQ_GET_STATUS : delegated !!!  \n",__FUNCTION__);
					break;
			}
		
		    //need to implement real work for request instead of just sending 0 value
			g_status = __constant_cpu_to_le16(0);

			dma_cache_maint(&g_status, 2, DMA_TO_DEVICE);
			pktcnt = 1;
			
			writel(virt_to_phys(&g_status), S3C_UDC_OTG_DIEPDMA0);
			writel((pktcnt<<DEPTSIZ_PKT_CNT_BIT)|(2<<DEPTSIZ_XFER_SIZE_BIT), (u32) S3C_UDC_OTG_DIEPTSIZ0);
						
			ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL0);

			writel(DEPCTL_EPENA|DEPCTL_CNAK|ep_ctrl, (u32) S3C_UDC_OTG_DIEPCTL0);
	
			dev->ep0state = WAIT_FOR_SETUP;
			return;

		default:
			DEBUG_SETUP("[%s] *** Default of ctrl.bRequest=0x%x happened.\n",
					__FUNCTION__, ctrl.bRequest);
			break;
	}// switch

gadget_setup:
	if (likely(dev->driver)) 
	{
		/* device-2-host (IN) or no data setup command,
		 * process immediately */
		spin_unlock(&dev->lock);
		DEBUG_SETUP("[%s] usb_ctrlrequest will be passed to gadget's setup()\n", __FUNCTION__);
		i = dev->driver->setup(&dev->gadget, &ctrl);
		spin_lock(&dev->lock);

		if (i < 0) 
		{
			/* setup processing failed, force stall */
			DEBUG_SETUP("[%s] gadget setup FAILED (stalling), setup returned %d\n",
				__FUNCTION__, i);
			/* ep->stopped = 1; */
			dev->ep0state = WAIT_FOR_SETUP;
		}
	}
}

/*
 * handle ep0 interrupt
 */
static void s3c_handle_ep0(struct s3c_udc *dev)
{
	if (dev->ep0state == WAIT_FOR_SETUP) 
	{
		DEBUG_EP0("[%s] WAIT_FOR_SETUP\n", __FUNCTION__);
		s3c_ep0_setup(dev);
	} 
	else 
	{
		DEBUG_EP0("[%s] strange state!!(state = %s)\n",
			__FUNCTION__, state_names[dev->ep0state]);
	}
}

static void s3c_ep0_kick(struct s3c_udc *dev, struct s3c_ep *ep)
{
	DEBUG_EP0("[%s] ep_is_in = %d\n", __FUNCTION__, ep_is_in(ep));
	if (ep_is_in(ep)) 
	{
		dev->ep0state = DATA_STATE_XMIT;
		s3c_ep0_write(dev);
	} 
	else 
	{
		dev->ep0state = DATA_STATE_RECV;
		s3c_ep0_read(dev);
	}
}

