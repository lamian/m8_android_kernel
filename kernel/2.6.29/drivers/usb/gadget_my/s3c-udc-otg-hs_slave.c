/*
 * drivers/usb/gadget/s3c-udc-otg-hs_slave.c
 * Samsung S3C on-chip full/high speed USB OTG 2.0 device controller slave mode
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
	in slave mode
	ep0(out) & ep_out use INT_RX_FIFO_NOT_EMPTY
	ep0(in) uses INT_NP_TX_FIFO_EMPTY
	ep_in use INT_IN_EP
*/

/*
#if SPIN_LOCK_IRQ_SAVE
		spin_lock_irqsave();
#else
		local_irq_save();
#endif
*/
#define SPIN_LOCK_IRQ_SAVE 		1

//count of reading register error
static	u32		ep_reg_err_cnt = 0;

//status bit for each in_ep : 1 means req, 0 means no req
static	u32		all_ep_req_status = 0;

//confirm to need #ifdef
//in include/linux/usb/cdc.h
#define USB_CDC_REQ_SET_LINE_CODING		0x20
#define USB_CDC_REQ_GET_LINE_CODING		0x21
#define USB_CDC_REQ_SET_CONTROL_LINE_STATE	0x22

void s3c_show_mode(void)
{
	printk("[S3C USB-OTG MODE] : Slave\n");
}

static void s3c_handle_data_phase(struct s3c_udc *dev)
{
	u32 gintmsk, ep_ctrl;
	u8 	is_in;

	gintmsk = readl(S3C_UDC_OTG_GINTMSK);

	DEBUG_EP0("%s: ep direction is %s\n", __FUNCTION__, ep_is_in(&dev->ep[EP0_CON])?"IN":"OUT");
	if(dev->ep0state == WAIT_FOR_SETUP)//USB_CDC_REQ_SET_CONTROL_LINE_STATE)
	{
		DEBUG_EP0("%s: dev->ep0state == WAIT_FOR_SETUP just return \n", __FUNCTION__);
		return;
	}
	
	if(dev->ep0state == USB_CDC_REQ_SET_LINE_CODING)//USB_CDC_REQ_SET_LINE_CODING)
	{
		DEBUG_EP0("%s: dev->ep0state == USB_CDC_REQ_SET_LINE_CODING just return \n", __FUNCTION__);
		writel(gintmsk | INT_RX_FIFO_NOT_EMPTY, S3C_UDC_OTG_GINTMSK);
		return;
	}

	if(dev->ep0state == FAIL_TO_SETUP)
	{
		DEBUG_EP0("%s: dev->ep0state == FAIL_TO_SETUP\n", __FUNCTION__);
		return;
	}
	// enable TX irqu to write zeop length pactet
	if(dev->ep0state == DATA_STATE_NEED_ZLP)
	{
		writel(gintmsk | INT_NP_TX_FIFO_EMPTY, S3C_UDC_OTG_GINTMSK);
		
		DEBUG_EP0("%s: DATA_STATE_NEED_ZLP\n", __FUNCTION__);
		return;
	}	

	//set the state and enable irq regarding direction of ep
//need to change is_in to dev->ep0state because s3c_ep3_kick already had state
	is_in = ep_is_in(&dev->ep[EP0_CON]);
	switch(is_in)
	{
		case 1:	 //EP_IN
			dev->ep0state = DATA_STATE_XMIT;
			
			writel(gintmsk | INT_NP_TX_FIFO_EMPTY, S3C_UDC_OTG_GINTMSK);
			break;
		case 0: //EP_OUT
			dev->ep0state = DATA_STATE_RECV;
			writel(gintmsk | INT_RX_FIFO_NOT_EMPTY, S3C_UDC_OTG_GINTMSK);
			
			ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
			writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
			break;
			
		default:
			DEBUG_EP0("%s: improper dev->ep0state = %d\n", __FUNCTION__, dev->ep0state);
			break;
	}
}

static void s3c_call_done(struct s3c_ep *ep)
{
	struct s3c_request *req;
	
	if (!list_empty(&ep->queue))
		req = list_entry(ep->queue.next, struct s3c_request, queue);
	else {
		DEBUG("%s: NULL REQ on OUT EP-0\n", __FUNCTION__);	
		BUG();	//logic ensures 	-jassi
		return;
	}
	//0 : status of no error
	done(ep, req, 0);
}

static void s3c_fifo_dump(u8 fifo_num, u16 length)
{
	u32 byte, fifo_address, i;
	unsigned count;
	
	count = length / 4; // 4 bytes == 32 bits

	fifo_address = (u32)S3C_UDC_OTG_EP0_FIFO + (fifo_num*4096); //4096 = 0x1000)
	DEBUG("%s: fifo[%d] addr : 0x%x\n", __FUNCTION__, fifo_num, fifo_address);

	for(i=0; i<=count; i++) {
		byte = (u32) readl(fifo_address);
		DEBUG("%s: [%d] => 0x%08x\n", __FUNCTION__, i, 
			le32_to_cpu(get_unaligned((__le32 *)&byte)));		
	}
}

//handle reading error of otg register 
static void s3c_handle_reg_err(struct s3c_udc *dev)
{
	int ep0state;
	u32 ep_ctrl;

	s3c_fifo_dump(0, 8);

	ep0state = dev->ep0state;
	
	if (ep0state == RegReadErr) {
		DEBUG_EP0("%s: DATA_STATE_NEED_ZLP after Err reg\n", __FUNCTION__);
		DEBUG_EP0("		=> Send ZLP [Status IN #0]\n");
		s3c_send_zlp();
		dev->ep0state = WAIT_FOR_SETUP;
		
		ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
		writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);

		s3c_call_done(&dev->ep[EP0_CON]);
		return;		
	}
	else
	{
		DEBUG_EP0("%s: improper ep0state\n", __FUNCTION__);
	}
}

static inline void s3c_send_zlp(void)
{
	u32 ep_ctrl, gintmsk;
/* 	
	Just to send ZLP(Zero length Packet) to HOST in response to SET CONFIGURATION
	1. Program the DIEPTSIZn register with the transfer size and corresponding packet size
	Packet count = 1, Xfersize = 0
*/ 
	DEBUG_EP0("%s\n",__FUNCTION__);

	__raw_writel((1<<DEPTSIZ_PKT_CNT_BIT)| 0<<DEPTSIZ_XFER_SIZE_BIT, S3C_UDC_OTG_DIEPTSIZ0); 
/*
	2. Program the DIEPCTLn register with endpoint characteristics and set the CNAK and 
	   Endpoint enable bits.
*/
	ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL0);
	__raw_writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK|(EP0_CON<<DIEPCTL0_NEXT_EP_BIT), S3C_UDC_OTG_DIEPCTL0); 

	gintmsk = readl(S3C_UDC_OTG_GINTMSK);
	writel(gintmsk&(~(INT_NP_TX_FIFO_EMPTY)), S3C_UDC_OTG_GINTMSK);
	
	ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
	writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
//	udelay(20);
}

static int write_packet(struct s3c_ep *ep, struct s3c_request *req, int max)
{
	u32 *buf;
	u32 in_ctrl, length, count;
	u8	ep_num = ep_index(ep);
	volatile u32 fifo = ep->fifo;
	
	buf = req->req.buf + req->req.actual;
	prefetch(buf);

	length = req->req.length - req->req.actual;
	length = min(length, (u32)max);
	req->req.actual += length;

	DEBUG("%s: Write %d (max %d), fifo=0x%x\n",
		__FUNCTION__, length, max, fifo);

	if(ep_num  == EP0_CON) {
		writel((1<<DEPTSIZ_PKT_CNT_BIT)|(length<<DEPTSIZ_XFER_SIZE_BIT), (u32) S3C_UDC_OTG_DIEPTSIZ0);

		in_ctrl =  readl(S3C_UDC_OTG_DIEPCTL0);
		writel(DEPCTL_EPENA|DEPCTL_CNAK|(EP0_CON<<DIEPCTL0_NEXT_EP_BIT)| in_ctrl, (u32) S3C_UDC_OTG_DIEPCTL0);

		DEBUG_EP0("%s:(DIEPTSIZ0):0x%x, (DIEPCTL0):0x%x, (GNPTXSTS):0x%x\n", __FUNCTION__,
			readl(S3C_UDC_OTG_DIEPTSIZ0),readl(S3C_UDC_OTG_DIEPCTL0),
			readl(S3C_UDC_OTG_GNPTXSTS));

	} else if ((ep_num  == EP2_IN)) {
		writel((1<<DEPTSIZ_PKT_CNT_BIT)|(length<<DEPTSIZ_XFER_SIZE_BIT), S3C_UDC_OTG_DIEPTSIZ2);

		in_ctrl =  readl(S3C_UDC_OTG_DIEPCTL2);
		writel(DEPCTL_EPENA|DEPCTL_CNAK|(EP2_IN<<DIEPCTL0_NEXT_EP_BIT)| in_ctrl, (u32) S3C_UDC_OTG_DIEPCTL2);

		DEBUG_EP2("%s:(DIEPTSIZ2):0x%x, (DIEPCTL2):0x%x, (GNPTXSTS):0x%x\n", __FUNCTION__,
			readl(S3C_UDC_OTG_DIEPTSIZ2),readl(S3C_UDC_OTG_DIEPCTL2),
			readl(S3C_UDC_OTG_GNPTXSTS));

	} else if ((ep_num  == EP3_IN)) {
		writel((1<<DEPTSIZ_PKT_CNT_BIT)|(length<<DEPTSIZ_XFER_SIZE_BIT), S3C_UDC_OTG_DIEPTSIZ3);

		in_ctrl =  readl(S3C_UDC_OTG_DIEPCTL3);
		writel(DEPCTL_EPENA|DEPCTL_CNAK| in_ctrl, (u32) S3C_UDC_OTG_DIEPCTL3);

		DEBUG_EP2("%s:(DIEPTSIZ3):0x%x, (DIEPCTL3):0x%x, (GNPTXSTS):0x%x\n", __FUNCTION__,
			readl(S3C_UDC_OTG_DIEPTSIZ3),readl(S3C_UDC_OTG_DIEPCTL3),
			readl(S3C_UDC_OTG_GNPTXSTS));
	} else {
		printk("%s: --> Error Unused Endpoint!!\n",	__FUNCTION__);
		BUG();
	}

	for (count=0;count<length;count+=4) {
	  	writel(*buf++, fifo);
	}
	return length;
}

/** Write request to FIFO (max write == maxp size)
 *  Return:  0 = still running, 1 = completed, negative = errno
 */
static int write_fifo(struct s3c_ep *ep, struct s3c_request *req)
{

	u32 max, gintmsk,daintmsk;
	unsigned count;
	int is_last = 0, is_short = 0;	
	
//	struct s3c_ep *ep2 = &ep->dev->ep[2];
//	struct s3c_ep *ep3 = &ep->dev->ep[3];

	u8	ep_num = ep_index(ep);
		
	DEBUG_IN_EP("%s\n",__FUNCTION__);

	gintmsk = readl(S3C_UDC_OTG_GINTMSK);
	daintmsk = readl(S3C_UDC_OTG_DAINTMSK);

	max = le16_to_cpu(ep->desc->wMaxPacketSize);
	count = write_packet(ep, req, max);

	/* last packet is usually short (or a zlp) */
	if (unlikely(count != max))
		is_last = is_short = 1;
	else {
		if (likely(req->req.length != req->req.actual)
		    || req->req.zero)
			is_last = 0;
		else
			is_last = 1;
		/* interrupt/iso maxpacket may not fill the fifo */
		is_short = unlikely(max < ep_maxpacket(ep));
	}

		DEBUG_IN_EP("%s: wrote %s %d bytes%s%s req %p %d/%d\n",
			__FUNCTION__,
      			ep->ep.name, count,
     	 		is_last ? "/L" : "", is_short ? "/S" : "",
      			req, req->req.actual, req->req.length);

	/* requests complete when all IN data is in the FIFO */
	if (is_last) {
		if(!ep_num){
			printk("%s: --> Error EP0 must not come here!\n", __FUNCTION__);
			BUG();
		}
		done(ep, req, 0);
		//if ep->queue is empty mask that ep's irq
		if (list_empty(&ep->queue))
		{
			writel(daintmsk&~(1<<ep_num), S3C_UDC_OTG_DAINTMSK);	
			all_ep_req_status &= ~(1<<ep_num);
		}
		
		//if all ep_in have no request, mask GINTMSK 		
		//if (list_empty(&ep2->queue) && list_empty(&ep3->queue))
		if(all_ep_req_status == 0)
		{
			writel(gintmsk&(~INT_IN_EP), S3C_UDC_OTG_GINTMSK);
		}
		return 1;
	}
	return 0;
}

/** Read to request from FIFO (max read == bytes in fifo)
 *  Return:  0 = still running, 1 = completed, negative = errno
 */
static int read_fifo(struct s3c_ep *ep, struct s3c_request *req)
{
	u32 csr, gintmsk, byte;
	u32 *buf;
	unsigned bufferspace, count, count_bytes, is_short = 0;
	volatile u32 fifo = ep->fifo;

	csr = readl(S3C_UDC_OTG_GRXSTSP);
	count_bytes = (csr & 0x7ff0)>>4;

	gintmsk = readl(S3C_UDC_OTG_GINTMSK);

	if(!count_bytes) {
		DEBUG_OUT_EP("%s: count_bytes %d bytes\n", __FUNCTION__, count_bytes);
	}

	buf = req->req.buf + req->req.actual;
	prefetchw(buf);
	bufferspace = req->req.length - req->req.actual;

	count = count_bytes / 4;
	if(count_bytes%4) count = count + 1;

	req->req.actual += min(count_bytes, bufferspace);

	is_short = (count_bytes < ep->ep.maxpacket);
	DEBUG_OUT_EP("%s: read %s, %d bytes%s req %p %d/%d GRXSTSP:0x%x\n",
		__FUNCTION__,
		ep->ep.name, count_bytes,
		is_short ? "/S" : "", req, req->req.actual, req->req.length, csr);

	while (likely(count-- != 0)) {
		byte = (u32) readl(fifo);

		if (unlikely(bufferspace == 0)) {
			/* this happens when the driver's buffer
		 	* is smaller than what the host sent.
		 	* discard the extra data.
		 	*/
			if (req->req.status != -EOVERFLOW)
				printk("%s overflow %d\n", ep->ep.name, count);
			req->req.status = -EOVERFLOW;
		} else {
			*buf++ = byte;
			bufferspace-=4;
		}
 	 }

	gintmsk = readl(S3C_UDC_OTG_GINTMSK);
	writel(gintmsk | INT_RX_FIFO_NOT_EMPTY, S3C_UDC_OTG_GINTMSK);


	/* completion */
	if (is_short || req->req.actual == req->req.length) {
		return 1;
	}

	/* finished that packet.  the next one may be waiting... */
	return 0;
}


static void done(struct s3c_ep *ep, struct s3c_request *req, int status)
{
	unsigned int stopped = ep->stopped;

	DEBUG("\t%s: %s %p, stopped = %d for req : %p\n", __FUNCTION__, ep->ep.name, ep, stopped, req);
	list_del_init(&req->queue);

	if (likely(req->req.status == -EINPROGRESS))
		req->req.status = status;
	else
		status = req->req.status;

	if (status && status != -ESHUTDOWN)
		DEBUG("complete %s req %p stat %d len %u/%u\n",
			ep->ep.name, &req->req, status,
			req->req.actual, req->req.length);

	/* don't modify queue heads during completion callback */
	ep->stopped = 1;

	spin_unlock(&ep->dev->lock);
	req->req.complete(&ep->ep, &req->req);
	spin_lock(&ep->dev->lock);

	ep->stopped = stopped;

	DEBUG("\t%s: %s %p, stopped = %d for req : %p after complete\n", __FUNCTION__, ep->ep.name, ep, stopped, req);
}

/*
 * 	s3c_ep_nuke - dequeue ALL requests
 */
void s3c_ep_nuke(struct s3c_ep *ep, int status)
{
	struct s3c_request *req;

	DEBUG("%s: %s %p\n", __FUNCTION__, ep->ep.name, ep);

	/* called with irqs blocked */
	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct s3c_request, queue);
		DEBUG("%s: req->req: %p, len %d buf %p\n",__FUNCTION__, req->req, req->req.length, req->req.buf);
		done(ep, req, status);
	}
}

/**
 * s3c_in_epn - handle IN interrupt
 */
static void s3c_in_epn(struct s3c_udc *dev, u32 ep_idx)
{
	struct s3c_ep *ep = &dev->ep[ep_idx];
	struct s3c_request *req;

	if (list_empty(&ep->queue))
		req = 0;
	else
		req = list_entry(ep->queue.next, struct s3c_request, queue);

	if (unlikely(!req)) {
		DEBUG_IN_EP("%s: NULL REQ on IN EP-%d\n", __FUNCTION__, ep_idx);
		return;
	}
	else {
		DEBUG_IN_EP("%s: Write FIFO on IN EP-%d\n", __FUNCTION__, ep_idx);
		write_fifo(ep, req);
	}

}

/* ********************************************************************************************* */
/* Bulk OUT (recv)
 */

static void s3c_out_epn(struct s3c_udc *dev, u32 ep_idx)
{
	struct s3c_ep *ep = &dev->ep[ep_idx];
	struct s3c_request *req;
	u32	ep_ctrl;
	
	if (unlikely(!(ep->desc))) {
		/* Throw packet away.. */
		DEBUG_OUT_EP("%s: No descriptor?!?\n", __FUNCTION__);
		return;
	}

	if (list_empty(&ep->queue))
		req = 0;
	else
		req = list_entry(ep->queue.next,
				struct s3c_request, queue);

	if (unlikely(!req)) {
		DEBUG_OUT_EP("%s: NULL REQ on OUT EP-%d\n", __FUNCTION__, ep_idx);
		return;

	} else {
		read_fifo(ep, req);
		
		ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL1);
		writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL1);
	}
}

static void s3c_udc_stop_activity(struct s3c_udc *dev,
			  struct usb_gadget_driver *driver)
{
	int i;

	/* don't disconnect drivers more than once */
	if (dev->gadget.speed == USB_SPEED_UNKNOWN)
		driver = 0;
	dev->gadget.speed = USB_SPEED_UNKNOWN;

	/* prevent new request submissions, kill any outstanding requests  */
	for (i = 0; i < S3C_MAX_ENDPOINTS; i++) {
		struct s3c_ep *ep = &dev->ep[i];
		ep->stopped = 1;
		s3c_ep_nuke(ep, -ESHUTDOWN);
	}

	/* report disconnect; the driver is already quiesced */
	if (driver) {
		spin_unlock(&dev->lock);
		driver->disconnect(&dev->gadget);
		spin_lock(&dev->lock);
	}

	/* re-init driver-visible data structures */
	udc_reinit(dev);
}

static void s3c_udc_initialize(void)
{
	// 2. Soft-reset OTG Core and then unreset again.
	u32 uTemp = writel(CORE_SOFT_RESET, S3C_UDC_OTG_GRSTCTL);

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

	udelay(20);

	// 3. Put the OTG device core in the disconnected state.
	uTemp = readl(S3C_UDC_OTG_DCTL);
	uTemp |= SOFT_DISCONNECT;
	writel(uTemp, S3C_UDC_OTG_DCTL);

	udelay(20);

	// 4. Make the OTG device core exit from the disconnected state.
	uTemp = readl(S3C_UDC_OTG_DCTL);
	uTemp = uTemp & ~SOFT_DISCONNECT;
	writel(uTemp, S3C_UDC_OTG_DCTL);

	// 5. Configure OTG Core to initial settings of device mode.
	writel(1<<18|0x0<<0, S3C_UDC_OTG_DCFG);		// [][1: full speed(30Mhz) 0:high speed]

	mdelay(1);

	// 6. Unmask the core interrupts
	writel(GINTMSK_INIT, S3C_UDC_OTG_GINTMSK);

	// 7. Set NAK bit of EP0, EP1, EP2
	writel(DEPCTL_EPDIS|DEPCTL_SNAK|(0<<0), S3C_UDC_OTG_DOEPCTL0); /* EP0: Control OUT */
	writel(DEPCTL_EPDIS|DEPCTL_SNAK|(0<<0), S3C_UDC_OTG_DIEPCTL0); /* EP0: Control IN */

	writel(DEPCTL_EPDIS|DEPCTL_SNAK|DEPCTL_BULK_TYPE|(0<<0), S3C_UDC_OTG_DOEPCTL1); /* EP1:Data OUT */
	writel(DEPCTL_EPDIS|DEPCTL_SNAK|DEPCTL_BULK_TYPE|(0<<0), S3C_UDC_OTG_DIEPCTL2); /* EP2:Data IN */
	writel(DEPCTL_EPDIS|DEPCTL_SNAK|DEPCTL_INTR_TYPE|(0<<0), S3C_UDC_OTG_DIEPCTL3); /* EP3:IN Interrupt*/

	/*
		8. Unmask EP interrupts on IN EPs : 0, 2, 3
	        	       		      OUT EPs : 0, 1    in case of rndis
	    s3c_ep_queue unmask irq to reduce irq handling
	*/
/*
	#if 0
	writel( (((1<<EP1_OUT)|(1<<EP0_CON))<<16) |
		(1<<EP3_IN)|(1<<EP2_IN)|(1<<EP0_CON),
		S3C_UDC_OTG_DAINTMSK);
	#else
//	writel((1<<EP3_IN)|(1<<EP2_IN)|(1<<EP0_CON),S3C_UDC_OTG_DAINTMSK);
	#endif
*/

	// 9. Unmask device OUT EP common interrupts
	//OUT 1 using Rx_fifo int
	writel(DOEPMSK_INIT, S3C_UDC_OTG_DOEPMSK);

	// 10. Unmask device IN EP common interrupts
	writel(DIEPMSK_INIT, S3C_UDC_OTG_DIEPMSK);

	// 11. Set Rx FIFO Size
	writel(RX_FIFO_SIZE, S3C_UDC_OTG_GRXFSIZ);

	// 12. Set Non Periodic Tx FIFO Size
	writel(NPTX_FIFO_SIZE<<16| NPTX_FIFO_START_ADDR<<0, S3C_UDC_OTG_GNPTXFSIZ);

	// 13. Clear NAK bit of EP0, EP1, EP2
	// For Slave mode
	//for SETUP packet count
	writel(DEPTSIZ_SETUP_PKCNT_3, (u32) S3C_UDC_OTG_DOEPTSIZ0);

	writel(DEPCTL_EPDIS|DEPCTL_CNAK|(0<<0), S3C_UDC_OTG_DOEPCTL0); /* EP0: Control OUT */
	writel(DEPCTL_EPDIS|DEPCTL_CNAK|(0<<0), S3C_UDC_OTG_DOEPCTL1); /* EP1: Bulk OUT */

	writel(DEPCTL_EPDIS|DEPCTL_CNAK|DEPCTL_BULK_TYPE|(0<<0), S3C_UDC_OTG_DIEPCTL0); /* EP0: Control IN */
	writel(DEPCTL_EPDIS|DEPCTL_CNAK|DEPCTL_BULK_TYPE|(0<<0), S3C_UDC_OTG_DIEPCTL2); /* EP2: Bulk IN */
	writel(DEPCTL_EPDIS|DEPCTL_CNAK|DEPCTL_INTR_TYPE|(0<<0), S3C_UDC_OTG_DIEPCTL3); /* EP3: Intr IN */

	// 14. Initialize OTG Link Core.
	writel(GAHBCFG_INIT, S3C_UDC_OTG_GAHBCFG);
	udelay(20);

}

void s3c_udc_set_max_pktsize(struct s3c_udc *dev, enum usb_device_speed speed)
{
	u32 ep_ctrl;

	if (speed == USB_SPEED_HIGH) {
		ep0_fifo_size = 64;
		ep_fifo_size = 512;
		ep_fifo_size2 = 1024;
		dev->gadget.speed = USB_SPEED_HIGH;
	} else {
		ep0_fifo_size = 64;
		ep_fifo_size = 64;
		ep_fifo_size2 = 64;
		dev->gadget.speed = USB_SPEED_FULL;
	}

	dev->ep[EP0_CON].ep.maxpacket = ep0_fifo_size;
	dev->ep[1].ep.maxpacket = ep_fifo_size;
	dev->ep[2].ep.maxpacket = ep_fifo_size;
	dev->ep[3].ep.maxpacket = 16;
	dev->ep[4].ep.maxpacket = ep_fifo_size;
	dev->ep[5].ep.maxpacket = ep_fifo_size2;
	dev->ep[6].ep.maxpacket = ep_fifo_size2;
	dev->ep[7].ep.maxpacket = ep_fifo_size2;
	dev->ep[8].ep.maxpacket = ep_fifo_size2;


	if (speed == USB_SPEED_HIGH) {
		// EP0 - Control IN (64 bytes)
		ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL0);
		writel(ep_ctrl|DEPCTL0_MPS_64, (u32) S3C_UDC_OTG_DIEPCTL0);

		// EP0 - Control OUT (64 bytes)
		ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
		writel(ep_ctrl|DEPCTL0_MPS_64, (u32) S3C_UDC_OTG_DOEPCTL0);
	} else {
		// EP0 - Control IN (8 bytes)
		ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL0);
		writel(ep_ctrl|DEPCTL0_MPS_8, (u32) S3C_UDC_OTG_DIEPCTL0);

		// EP0 - Control OUT (8 bytes)
		ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
		writel(ep_ctrl|DEPCTL0_MPS_8, (u32) S3C_UDC_OTG_DOEPCTL0);
	}

//ss1 : the following is replaced in s3c_ep_enable()
/*
	// EP1 - Bulk Data OUT (512 bytes)
	ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL1);
	writel(ep_ctrl|(ep_fifo_size<<0), (u32) S3C_UDC_OTG_DOEPCTL1);

	// EP2 - Bulk Data IN (512 bytes)
	ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL2);
	writel(ep_ctrl|(ep_fifo_size<<0), (u32) S3C_UDC_OTG_DIEPCTL2);

	// EP3 - INTR Data IN (512 bytes)
	ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL3);
	writel(ep_ctrl|(ep_fifo_size<<0), (u32) S3C_UDC_OTG_DIEPCTL3);
*/
}

static int reset_available = 1;

static irqreturn_t s3c_udc_irq(int irq, void *_dev)
{
	struct s3c_udc *dev = _dev;
	u32 intr_status, dpid;
	u32 usb_status, ep_ctrl, gintmsk;
	unsigned long flags;
	u32 diepint2, diepint3;
	u32 grx_status;
	u32 packet_status, ep_num, fifoCntByte = 0;
	u32 daint, daintmsk, gnptxsts;
	u32 nptxQ_SpcAvail, nptxFifo_SpcAvail;

	spin_lock_irqsave(&dev->lock, flags);

	intr_status = readl(S3C_UDC_OTG_GINTSTS);
	gintmsk = readl(S3C_UDC_OTG_GINTMSK);
	intr_status &= gintmsk;

	DEBUG_ISR("\n**** %s : GINTSTS=0x%x, GINTMSK : 0x%x\n",__FUNCTION__, intr_status, gintmsk);\
	
#if OTG_DBG_ENABLE
//	udelay(20);
#else
//	udelay(100);
#endif

	if (intr_status & INT_RX_FIFO_NOT_EMPTY) {
		// Mask USB OTG 2.0 interrupt source : INT_RX_FIFO_NOT_EMPTY
	//	gintmsk &= ~INT_RX_FIFO_NOT_EMPTY;
	//	writel(gintmsk, S3C_UDC_OTG_GINTMSK);

		grx_status = readl(S3C_UDC_OTG_GRXSTSR);
		packet_status = grx_status & 0x1E0000;
		fifoCntByte = (grx_status & 0x7ff0)>>4;
		ep_num = grx_status & EP_MASK;
		dpid = (grx_status & (0x3<<15))>>15;
		
		DEBUG_ISR("\n%s : fifoCntByte = %d bytes\n", __FUNCTION__, fifoCntByte);
		
		if(dev->ep0state == RegReadErr)
			goto REG_ERR_SETUP;

		switch(packet_status)
		{
			case SETUP_PKT_RECEIVED:
				DEBUG_ISR("    => SETUP_PKT_RECEIVED: %d bytes\n",	fifoCntByte);
				if(fifoCntByte !=8 || ep_num != EP0_CON || dpid != 0)
					DEBUG_ISR("    => improper	~~~~ fifoCntByte !=8 || ep_num != EP0_CON || dpid != 0 ~~~~: fifoCntByte != 0 ep:%d, dpid:%d\n", ep_num, dpid); 					
				else
					s3c_ep0_handle(dev);
				break;
				
			case OUT_PKT_RECEIVED:
				DEBUG_ISR("    => OUT_PKT_RECEIVED: %d bytes\n",	fifoCntByte);
				if (ep_num == EP0_CON) 
				{
					ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);

					if (fifoCntByte == 0) {
						DEBUG_ISR(" 	 => An ZLP received (DOEPCTL0):0x%x\n", ep_ctrl);
						
						grx_status = readl(S3C_UDC_OTG_GRXSTSP);
					}
					else
					{
						DEBUG_ISR("    => A CONTROL OUT data packet received : %d bytes, (DOEPCTL0):0x%x\n",
							fifoCntByte, ep_ctrl);
						if (dev->ep0state != USB_CDC_REQ_SET_LINE_CODING)//USB_CDC_REQ_SET_LINE_CODING
							dev->ep0state = DATA_STATE_RECV;
						s3c_ep0_read(dev);
					}
					
				} 
				else if(ep_num == EP1_OUT) 
				{
					ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL1);
					DEBUG_ISR(" 	 => A Bulk OUT data packet received : %d bytes, (DOEPCTL1):0x%x\n",
						fifoCntByte, ep_ctrl);
					s3c_out_epn(dev, 1);
				} 
				else 
				{
					DEBUG_ISR(" 	 => Unused EP: %d fifoCntByte %d bytes, (GRXSTSR):0x%x\n", ep_num, fifoCntByte, grx_status);
					ep_reg_err_cnt++;
					DEBUG_ISR(" 	===> ep_reg_err_cnt++ : %d\n", ep_reg_err_cnt);

					ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
					writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
					DEBUG_ISR(" 	 =>=>=> DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0, (DOEPCTL0):0x%x\n", ep_ctrl);
					grx_status = readl(S3C_UDC_OTG_GRXSTSP);
				}
				break;
				
			case SETUP_TRANSACTION_COMPLETED:
				ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
				DEBUG_ISR("    => SETUP_STAGE_TRANSACTION_COMPLETED: %d bytes , (DOEPCTL0):0x%x\n", fifoCntByte, ep_ctrl);
				if(fifoCntByte ==0 && ep_num == EP0_CON)
				{
					s3c_handle_data_phase(dev); 				
					grx_status = readl(S3C_UDC_OTG_GRXSTSP);
				}
				else
				{				
					DEBUG_ISR("    => improper	~~~~: fifoCntByte !=0 || ep_num != EP0_CON ep:%d\n", ep_num);
					goto REG_ERR;
				}
				
				writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
				ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
				DEBUG_ISR("    => SETUP_STAGE_TRANSACTION_COMPLETED: %d bytes , (DOEPCTL0):0x%x\n", fifoCntByte, ep_ctrl);
				break;
				
			case OUT_TRANSFER_COMPLELTED:
				DEBUG_ISR("    => OUT_TRANSFER_COMPLELTED: %d bytes\n", fifoCntByte);
				if(fifoCntByte !=0 || ep_num == EP2_IN || ep_num == EP3_IN)
					DEBUG_ISR("    => improper	~~~~: fifoCntByte != 0\n");
				else
				{
					DEBUG_ISR("    => INT_RX_FIFO_NOT_EMPTY enabled\n");
					gintmsk |= INT_RX_FIFO_NOT_EMPTY;
					writel(gintmsk, S3C_UDC_OTG_GINTMSK);

					if (ep_num == EP1_OUT) {
						ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL1);
						DEBUG_ISR(" 	 => An OUT transaction completed %d bytes, (DOEPCTL1):0x%x\n", fifoCntByte, ep_ctrl);
						writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL1);

						s3c_call_done(&dev->ep[1]);
						grx_status = readl(S3C_UDC_OTG_GRXSTSP);
						
					} else if (ep_num == EP0_CON) {
						ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
						DEBUG_ISR(" 	 => An OUT transaction completed %d bytes, (DOEPCTL0):0x%x\n", fifoCntByte, ep_ctrl);

						//control write status stage
						if(dev->ep0state == DATA_STATE_NEED_ZLP)
						{
							writel(gintmsk | INT_NP_TX_FIFO_EMPTY, S3C_UDC_OTG_GINTMSK);							
							DEBUG_EP0("%s: DATA_STATE_NEED_ZLP\n", __FUNCTION__);
							s3c_call_done(&dev->ep[EP0_CON]);
						}	
						else if (dev->ep0state == USB_CDC_REQ_SET_LINE_CODING)
						{
							DEBUG_EP0("%s: dev->ep0state == USB_CDC_REQ_SET_LINE_CODING just go through \n", __FUNCTION__);
							/*
							writel(gintmsk | INT_NP_TX_FIFO_EMPTY, S3C_UDC_OTG_GINTMSK);							
							dev->ep0state = DATA_STATE_NEED_ZLP;
							*/
						}
						writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
						ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
						DEBUG_ISR(" 	 => An OUT transaction completed %d bytes, (DOEPCTL0):0x%x\n", fifoCntByte, ep_ctrl);

						grx_status = readl(S3C_UDC_OTG_GRXSTSP);
					} else {
					
						DEBUG_ISR(" 	 => Unused EP: %d fifoCntByte %d bytes, (GRXSTSR):0x%x\n", ep_num, fifoCntByte, grx_status);
						ep_reg_err_cnt++;
						DEBUG_ISR(" 	===> ep_reg_err_cnt++ : %d\n", ep_reg_err_cnt);
						ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);

						writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
						DEBUG_ISR(" 	 =>=>=> DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0, (DOEPCTL0):0x%x\n", ep_ctrl);
						grx_status = readl(S3C_UDC_OTG_GRXSTSP);
					}
				}
				break;
				
			case GLOBAL_OUT_NAK:				
				DEBUG_ISR("    => GLOBAL_OUT_NAK~~~~: %d bytes\n",	fifoCntByte);
				if(fifoCntByte != 0)
					DEBUG_ISR("    => improper ~~~~ fifoCntByte != 0 \n");
				
				grx_status = readl(S3C_UDC_OTG_GRXSTSP);
				break;
				
			default:
				DEBUG_ISR(" 	 => EP : %d  Unknown Packet status received : %d bytes, packet_status:	0x%x\n",ep_num,  fifoCntByte, packet_status);
				if (ep_num == 8 && fifoCntByte == 18) {
REG_ERR_SETUP:		
					DEBUG_ISR(" 	 => handling of reading register err [ep 8 & packet cnt = 18 bytes]");				
					DEBUG_ISR(" 	 => consider setup packet \n");
					DEBUG_ISR(" 	 => s3c_fifo_dump(0, 8) for reading 3 data from fifo 0\n");
					s3c_handle_reg_err(dev);
					break;
				 }
REG_ERR:					 
				if (!list_empty(&(dev->ep[EP0_CON].queue)))
				{
					 DEBUG_ISR("	  => consider setup packet !list_empty(&(dev->ep[EP0_CON].queue)\n");
					 dev->ep0state = RegReadErr;
					 s3c_handle_reg_err(dev);
				}
				grx_status = readl(S3C_UDC_OTG_GRXSTSP);
				break;
		}//switch
		
		// Un/Mask USB OTG 2.0 interrupt sources
		gintmsk = readl(S3C_UDC_OTG_GINTMSK);
		gintmsk |= INT_RX_FIFO_NOT_EMPTY;
		writel(gintmsk, S3C_UDC_OTG_GINTMSK);
		
		ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
		writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
		goto	OK_OUT;
	}// INT_RX_FIFO_NOT_EMPTY

	if (!intr_status) {
		goto	OK_OUT;
	}

	if (intr_status & INT_ENUMDONE) {
		DEBUG_SETUP("####################################\n");
		DEBUG_SETUP("    %s: Speed Detection interrupt\n",
				__FUNCTION__);
		writel(INT_ENUMDONE, S3C_UDC_OTG_GINTSTS);

		usb_status = (readl(S3C_UDC_OTG_DSTS) & 0x6);

		if (usb_status & (USB_FULL_30_60MHZ | USB_FULL_48MHZ)) {
			DEBUG_SETUP("    %s: Full Speed Detection\n",__FUNCTION__);
			s3c_udc_set_max_pktsize(dev, USB_SPEED_FULL);

		} else {
			DEBUG_SETUP("    %s: High Speed Detection : 0x%x\n", __FUNCTION__, usb_status);
			s3c_udc_set_max_pktsize(dev, USB_SPEED_HIGH);
		}
		goto	OK_OUT;
	}

	if (intr_status & INT_EARLY_SUSPEND) {
		DEBUG_SETUP("####################################\n");
		printk("    %s:Early suspend interrupt\n", __FUNCTION__);
		writel(INT_EARLY_SUSPEND, S3C_UDC_OTG_GINTSTS);
		goto	OK_OUT;
	}

	if (intr_status & INT_SUSPEND) {
		usb_status = readl(S3C_UDC_OTG_DSTS);
		DEBUG_SETUP("####################################\n");
		printk("    %s:Suspend interrupt :(DSTS):0x%x\n", __FUNCTION__, usb_status);
		writel(INT_SUSPEND, S3C_UDC_OTG_GINTSTS);

		if (dev->gadget.speed != USB_SPEED_UNKNOWN
		    && dev->driver
		    && dev->driver->suspend) {
			dev->driver->suspend(&dev->gadget);
		}
		goto	OK_OUT;
	}

	if (intr_status & INT_RESUME) {
		DEBUG_SETUP("####################################\n");
		printk("    %s: Resume interrupt\n", __FUNCTION__);
		writel(INT_RESUME, S3C_UDC_OTG_GINTSTS);

		if (dev->gadget.speed != USB_SPEED_UNKNOWN
		    && dev->driver
		    && dev->driver->resume) {
			dev->driver->resume(&dev->gadget);
		}
		goto	OK_OUT;
	}

	if (intr_status & INT_RESET) {
		usb_status = readl(S3C_UDC_OTG_GOTGCTL);
		DEBUG_SETUP("####################################\n");
		printk("    %s: Reset interrupt - (GOTGCTL):0x%x\n", __FUNCTION__, usb_status);
		writel(INT_RESET, S3C_UDC_OTG_GINTSTS);

		if((usb_status & 0xc0000) == (0x3 << 18)) {
			if(reset_available) {
				DEBUG_SETUP("     ===> OTG core got reset (%d)!! \n", reset_available);
				s3c_udc_initialize();
				dev->ep0state = WAIT_FOR_SETUP;
				reset_available = 0;
			}
		} else {
			reset_available = 1;
			DEBUG_SETUP("      RESET handling skipped : reset_available : %d\n", reset_available);
		}
		goto	OK_OUT;
	}

	if (intr_status & (INT_NP_TX_FIFO_EMPTY)) {

		daint = readl(S3C_UDC_OTG_DAINT);		
		daintmsk = readl(S3C_UDC_OTG_DAINTMSK);
		daint &= daintmsk;
		
		DEBUG_ISR("    INT_NP_TX_FIFO_EMPTY daintmsk=0x%x, daint=0x%x\n",daintmsk ,daint);

		gnptxsts = readl(S3C_UDC_OTG_GNPTXSTS);
		
		nptxQ_SpcAvail = (gnptxsts & (0xff<<16))>>16;
		nptxFifo_SpcAvail = gnptxsts & 0xffff;
		
		DEBUG_ISR("    GNPTXSTS nptxQ_SpcAvail = %d, nptxFifo_SpcAvail = %d\n",nptxQ_SpcAvail ,nptxFifo_SpcAvail);
		if (nptxQ_SpcAvail == 0 || nptxFifo_SpcAvail == 0)
		{
			DEBUG_ISR("    improper ~~~~ nptxQ_SpcAvail == 0 || nptxFifo_SpcAvail == 0 \n");
			goto FAIL_OUT;
		}

		//DATA_STATE_NEED_ZLP is for sending zlp without gadger driver's req queue
		//zlp gadget driver requested will be send by s3c_ep0_write in case of ep0state is DATA_STATE_XMIT
		if(dev->ep0state == DATA_STATE_NEED_ZLP ||
			dev->ep0state == USB_CDC_REQ_SET_LINE_CODING) {
			s3c_send_zlp();
			dev->ep0state = WAIT_FOR_SETUP;
			
			ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
			writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
			
		} else if(dev->ep0state == DATA_STATE_XMIT ||
				  dev->ep0state == USB_CDC_REQ_SET_CONTROL_LINE_STATE) {
			ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);					
			s3c_ep0_write(dev);//it set ep0state wait_for_setup
		} else {
			DEBUG_ISR("		=> Unwanted IN_TKN in ep0state[%d]\n", dev->ep0state);
		}
		goto	OK_OUT;
	}

	if (intr_status & (INT_IN_EP)) {

		daint = readl(S3C_UDC_OTG_DAINT);		
		daintmsk = readl(S3C_UDC_OTG_DAINTMSK);
		daint &= daintmsk;
		
		DEBUG_ISR("    INT_IN_EP daintmsk=0x%x, daint=0x%x\n",daintmsk ,daint);

		gnptxsts = readl(S3C_UDC_OTG_GNPTXSTS);

		nptxQ_SpcAvail = (gnptxsts & (0xff<<16))>>16;
		nptxFifo_SpcAvail = gnptxsts & 0xffff;
		
		DEBUG_ISR("    GNPTXSTS nptxQ_SpcAvail = %d, nptxFifo_SpcAvail = %d\n",nptxQ_SpcAvail ,nptxFifo_SpcAvail);

		if (nptxQ_SpcAvail == 0 || nptxFifo_SpcAvail == 0)
		{
			DEBUG_ISR("nptxQ_SpcAvail == 0 || nptxFifo_SpcAvail == 0 \n");
			goto FAIL_OUT;
		}

		if(daint & (0x1<<EP2_IN)) {	//EP2_IN_INT
		
			DEBUG_ISR("      => INT_IN_EP 2\n");
			diepint2 = readl(S3C_UDC_OTG_DIEPINT2);
			if(diepint2 & (0x1<<4)) 
			{	//IN_TKN_RECEIVED
				DEBUG_ISR("      => IN_TKN_RECEIVED...EP2 (DIEPINT2):0x%x\n", diepint2);
				s3c_in_epn(dev, EP2_IN);
			}
			if(diepint2 & (0x1<<3)) {	//TIMEOUT_CONDITION
				writel((0x1<<8), S3C_UDC_OTG_DCTL);				
			}
			writel(diepint2, S3C_UDC_OTG_DIEPINT2);
		}

		if(daint & (0x1<<EP3_IN)) {	//EP3_IN_INT
		
			DEBUG_ISR("      => INT_IN_EP 3\n");
			diepint3 = readl(S3C_UDC_OTG_DIEPINT3);
			if(diepint3 & (0x1<<4)) 
			{	//IN_TKN_RECEIVED
				DEBUG_ISR("      => IN_TKN_RECEIVED...EP3(DIEPINT3):0x%x\n", diepint3);
				s3c_in_epn(dev, EP3_IN);
			}			
			if(diepint3 & (0x1<<3)) {	//TIMEOUT_CONDITION
				writel((0x1<<8), S3C_UDC_OTG_DCTL);				
			}
			writel(diepint3, S3C_UDC_OTG_DIEPINT3);
		}
		goto	OK_OUT;
	}
	

	
OK_OUT:
	
#if SPIN_LOCK_IRQ_SAVE
	spin_unlock_irqrestore(&dev->lock, flags);
#else
	local_irq_restore(flags);
#endif

	return IRQ_HANDLED;

FAIL_OUT:
	
#if SPIN_LOCK_IRQ_SAVE
	spin_unlock_irqrestore(&dev->lock, flags);
#else
	local_irq_restore(flags);
#endif

	return IRQ_NONE;
}

static int s3c_ep_enable(struct usb_ep *_ep,
			     const struct usb_endpoint_descriptor *desc)
{
	struct s3c_ep *ep;
	struct s3c_udc *dev;
	unsigned long flags;
	u32 daintmsk;

	DEBUG("%s: %p\n", __FUNCTION__, _ep);

	ep = container_of(_ep, struct s3c_ep, ep);
	if (!_ep || !desc || ep->desc || _ep->name == ep0name
	    || desc->bDescriptorType != USB_DT_ENDPOINT
	    || ep->bEndpointAddress != desc->bEndpointAddress
	    || ep_maxpacket(ep) < le16_to_cpu(desc->wMaxPacketSize)) {
		DEBUG("%s: bad ep or descriptor\n", __FUNCTION__);
		return -EINVAL;
	}

	/* xfer types must match, except that interrupt ~= bulk */
	if (ep->bmAttributes != desc->bmAttributes
	    && ep->bmAttributes != USB_ENDPOINT_XFER_BULK
	    && desc->bmAttributes != USB_ENDPOINT_XFER_INT) {
		DEBUG("%s: %s type mismatch\n", __FUNCTION__, _ep->name);
		return -EINVAL;
	}

	/* hardware _could_ do smaller, but driver doesn't */
	if ((desc->bmAttributes == USB_ENDPOINT_XFER_BULK
	     && le16_to_cpu(desc->wMaxPacketSize) != ep_maxpacket(ep))
	    || !desc->wMaxPacketSize) {
		DEBUG("%s: bad %s maxpacket\n", __FUNCTION__, _ep->name);
		return -ERANGE;
	}

	dev = ep->dev;
	if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN) {
		DEBUG("%s: bogus device state\n", __FUNCTION__);
		return -ESHUTDOWN;
	}

	
#if SPIN_LOCK_IRQ_SAVE
	spin_lock_irqsave(&dev->lock, flags);
#else
	local_irq_save(flags);
#endif

	ep->stopped = 0;
	ep->desc = desc;
	ep->ep.maxpacket = le16_to_cpu(desc->wMaxPacketSize);

	/* Reset halt state */
	s3c_set_halt(_ep, 0);

//by ss1 the follwoing setting is specific with RNDIS of s3c6410
	daintmsk = readl(S3C_UDC_OTG_DAINTMSK);

	switch(ep_index(ep))
	{
		//spec. say: mps, USB active endpoing, start data toggle, type,

		case EP1_OUT :			
			/* EP1: Bulk OUT */
			writel(DEPCTL_MPS_BULK_512|DEPCTL_USBACTEP|DEPCTL_BULK_TYPE
				, S3C_UDC_OTG_DOEPCTL1); 
			break;
			
		case EP2_IN	 :			
			/* EP2: Bulk IN */
			writel(DEPCTL_MPS_BULK_512|DEPCTL_USBACTEP|DEPCTL_BULK_TYPE
				, S3C_UDC_OTG_DIEPCTL2); 
			break;
			
		case EP3_IN	 :			
			/* EP3: INTR IN */
			writel(DEPCTL_MPS_INT_MPS_16|DEPCTL_USBACTEP|DEPCTL_INTR_TYPE
				, S3C_UDC_OTG_DIEPCTL3); 
			break;
			
		default :
			printk("%s: not supported ep %d\n",__FUNCTION__, ep_index(ep));			
	}

#if SPIN_LOCK_IRQ_SAVE
	spin_unlock_irqrestore(&dev->lock, flags);
#else
	local_irq_restore(flags);
#endif

	DEBUG("%s: enabled %s, stopped = %d, maxpacket = %d\n",
		__FUNCTION__, _ep->name, ep->stopped, ep->ep.maxpacket);
	return 0;
}

/** Disable EP
 */
static int s3c_ep_disable(struct usb_ep *_ep)
{
	struct s3c_ep *ep;
	unsigned long flags;
	u32 daintmsk;
	u32 ep_ctrl;

	DEBUG("%s: %p\n", __FUNCTION__, _ep);

	ep = container_of(_ep, struct s3c_ep, ep);
	if (!_ep || !ep->desc) {
		DEBUG("%s: %s not enabled\n", __FUNCTION__,
		      _ep ? ep->ep.name : NULL);
		return -EINVAL;
	}

#if SPIN_LOCK_IRQ_SAVE
	spin_lock_irqsave(&ep->dev->lock, flags);
#else
	local_irq_save(flags);
#endif

	/* Nuke all pending requests */
	s3c_ep_nuke(ep, -ESHUTDOWN);

	ep->desc = 0;
	ep->stopped = 1;
	
	daintmsk = readl(S3C_UDC_OTG_DAINTMSK);

	switch(ep_index(ep))
	{
	//mps, USB active endpoing, start data toggle, type, TxFifo for int
		case EP1_OUT :			
			/* EP1: Bulk OUT */			
			ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL1);
			writel(ep_ctrl&~(DEPCTL_USBACTEP), S3C_UDC_OTG_DOEPCTL1); 
			writel(daintmsk&(~(1<<(ep_index(ep)+16))), S3C_UDC_OTG_DAINTMSK);
			break;
			
		case EP2_IN	 :			
			/* EP2: Bulk IN */
			ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL2);
			writel(ep_ctrl&~(DEPCTL_USBACTEP), S3C_UDC_OTG_DIEPCTL2); 
			writel(daintmsk&(~(1<<ep_index(ep))), S3C_UDC_OTG_DAINTMSK);
			break;
			
		case EP3_IN	 :			
			/* EP3: INTR IN */
			ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL3);
			writel(ep_ctrl&~(DEPCTL_USBACTEP), S3C_UDC_OTG_DIEPCTL3);
			writel(daintmsk&(~(1<<ep_index(ep))), S3C_UDC_OTG_DAINTMSK);
			break;
			
		default :
			DEBUG("%s: not supported ep %d\n",__FUNCTION__);			
	}

#if SPIN_LOCK_IRQ_SAVE
	spin_unlock_irqrestore(&ep->dev->lock, flags);
#else
	local_irq_restore(flags);
#endif

	DEBUG("%s: disabled %s\n", __FUNCTION__, _ep->name);
	return 0;
}

static struct usb_request *s3c_ep_alloc_request(struct usb_ep *ep,
						 gfp_t gfp_flags)
{
	struct s3c_request *req;

	DEBUG("%s: %s %p\n", __FUNCTION__, ep->name, ep);

	req = kmalloc(sizeof(struct s3c_request), gfp_flags);
	if (!req) {
		DEBUG("%s: kmalloc failed!!!\n", __FUNCTION__);
		return 0;
	}
	memset(req, 0, sizeof(struct s3c_request));
	INIT_LIST_HEAD(&req->queue);

	return &req->req;
}

static void s3c_ep_free_request(struct usb_ep *ep, struct usb_request *_req)
{
	struct s3c_request *req;

	DEBUG("%s: %p\n", __FUNCTION__, ep);

	req = container_of(_req, struct s3c_request, req);
	WARN_ON(!list_empty(&req->queue));
	kfree(req);
}

/** Queue one request
 *  Kickstart transfer if needed
 */
static int s3c_ep_queue(struct usb_ep *_ep, struct usb_request *_req,
			 gfp_t gfp_flags)
{
	struct s3c_request *req;
	struct s3c_ep *ep;
	struct s3c_udc *dev;
	unsigned long flags;
	u32 gintmsk, daintmsk;
	u32 csr;
	u8 is_in, ep_num;

	req = container_of(_req, struct s3c_request, req);

	if (unlikely(!_req || !_req->complete || !_req->buf
			|| !list_empty(&req->queue)))
	{
		DEBUG("%s: bad params\n", __FUNCTION__);
		return -EINVAL;
	}

	ep = container_of(_ep, struct s3c_ep, ep);
	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name))) {
		DEBUG("%s: bad ep\n", __FUNCTION__);
		return -EINVAL;
	}

	dev = ep->dev;
	if (unlikely(!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN)) {
		DEBUG("%s: bogus device state %p\n", __FUNCTION__, dev->driver);
		return -ESHUTDOWN;
	}

#if SPIN_LOCK_IRQ_SAVE
	spin_lock_irqsave(&dev->lock, flags);
#else
	local_irq_save (flags);
#endif

	DEBUG_KEVIN("\n%s: %s queue req %p, len %d buf %p\n",
		__FUNCTION__, _ep->name, _req, _req->length, _req->buf);

	_req->status = -EINPROGRESS;
	_req->actual = 0;

	/* kickstart this i/o queue? */
	DEBUG("%s: Add to ep=%d, Q empty=%d, stopped=%d\n",
		__FUNCTION__, ep_index(ep), list_empty(&ep->queue), ep->stopped);
	
	is_in = ep_is_in(ep);
	ep_num = ep_index(ep);
	
	gintmsk = readl(S3C_UDC_OTG_GINTMSK);
	daintmsk = readl(S3C_UDC_OTG_DAINTMSK);

	if (list_empty(&ep->queue) && likely(!ep->stopped)) {
			
		if (ep_num == 0) {
			list_add_tail(&req->queue, &ep->queue);
			s3c_ep0_kick(dev, ep);
			req = 0;
		} 
		else if (is_in) {			
			all_ep_req_status |= (1 << ep_num);
			
			csr = readl((u32) S3C_UDC_OTG_GINTSTS);
			DEBUG("%s: ep_is_in, S3C_UDC_OTG_GINTSTS=0x%x\n",
				__FUNCTION__, csr);
			
			list_add_tail(&req->queue, &ep->queue);
			req = 0;
#if 0		//for rndis specific logic that works
			if (ep_num == EP2_IN) {
				writel(gintmsk | INT_IN_EP, S3C_UDC_OTG_GINTMSK);
				writel(daintmsk|(1<<ep_num), S3C_UDC_OTG_DAINTMSK);				
			} else if (ep_num == EP3_IN) {		
				writel(gintmsk | INT_IN_EP, S3C_UDC_OTG_GINTMSK);
				writel(daintmsk|(1<<ep_num), S3C_UDC_OTG_DAINTMSK);
			} else 
				DEBUG_ISR(">>>>>>>>>  not proper EP num %d\n", ep_index(ep));	
#else
			writel(gintmsk | INT_IN_EP, S3C_UDC_OTG_GINTMSK);
			writel(daintmsk|(1<<ep_num), S3C_UDC_OTG_DAINTMSK); 			
#endif
		} // for IN
		//for OUT
		else {
			list_add_tail(&req->queue, &ep->queue);
			req = 0;
		}// for OUT
	}// for empty
	else
	{
		if (ep_num != 0 && is_in) {
			
			all_ep_req_status |= (1 << ep_num);

			csr = readl((u32) S3C_UDC_OTG_GINTSTS);
			DEBUG("%s: ep_is_in, S3C_UDC_OTG_GINTSTS=0x%x\n",
				__FUNCTION__, csr);
			
			list_add_tail(&req->queue, &ep->queue);
			req = 0;

			if (ep_num == EP2_IN) {

				writel(gintmsk | INT_IN_EP, S3C_UDC_OTG_GINTMSK);
				writel(daintmsk|(1<<ep_num), S3C_UDC_OTG_DAINTMSK);
			} else if (ep_index(ep) == EP3_IN) {
		
				writel(gintmsk | INT_IN_EP, S3C_UDC_OTG_GINTMSK);
				writel(daintmsk|(1<<ep_num), S3C_UDC_OTG_DAINTMSK);

			} else 
				DEBUG_ISR(">>>>>>>>>  not proper EP num %d\n", ep_index(ep));			
		} // for IN
	}

	/* pio or dma irq handler advances the queue. */
	if (likely(req != 0))
		list_add_tail(&req->queue, &ep->queue);

#if SPIN_LOCK_IRQ_SAVE
	spin_unlock_irqrestore(&dev->lock, flags);
#else
	local_irq_restore(flags);
#endif

	return 0;
}

/* dequeue JUST ONE request */
static int s3c_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct s3c_ep *ep;
	struct s3c_request *req;
	unsigned long flags;

	DEBUG("%s: %p\n", __FUNCTION__, _ep);

	ep = container_of(_ep, struct s3c_ep, ep);
	if (!_ep || ep->ep.name == ep0name)
		return -EINVAL;

#if SPIN_LOCK_IRQ_SAVE
	spin_lock_irqsave(&ep->dev->lock, flags);
#else
	local_irq_save (flags);
#endif

	/* make sure it's actually queued on this endpoint */
	list_for_each_entry(req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	if (&req->req != _req) {
		local_irq_restore(flags);
		return -EINVAL;
	}
	done(ep, req, -ECONNRESET);

#if SPIN_LOCK_IRQ_SAVE
	spin_unlock_irqrestore(&ep->dev->lock, flags);
#else
	local_irq_restore(flags);
#endif
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
	if (!_ep) {
		DEBUG("%s: bad ep\n", __FUNCTION__);
		return -ENODEV;
	}

	DEBUG("%s: %d\n", __FUNCTION__, ep_index(ep));

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
	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name))) {
		DEBUG("%s: bad ep\n", __FUNCTION__);
		return;
	}

	DEBUG("%s: %d\n", __FUNCTION__, ep_index(ep));
}

/****************************************************************/
/* End Point 0 related functions                                */
/****************************************************************/

/* return:  0 = still running, 1 = completed, negative = errno */
static int s3c_ep0_write_fifo(struct s3c_ep *ep, struct s3c_request *req)
{
	u32 max;
	unsigned count;
	int is_last;
	u32 gintmsk, daintmsk;

	gintmsk = readl(S3C_UDC_OTG_GINTMSK);
	daintmsk = readl(S3C_UDC_OTG_DAINTMSK);

	max = ep_maxpacket(ep);

	DEBUG_EP0("%s: max = %d\n", __FUNCTION__, max);

	count = write_packet(ep, req, max);

	/* last packet is usually short (or a zlp) */
	if (likely(count != max))
		is_last = 1;
	else {
		if (likely(req->req.length != req->req.actual) || req->req.zero)
			is_last = 0;
		else
			is_last = 1;
	}

	DEBUG_EP0("%s: wrote %s %d bytes%s %d left %p\n", __FUNCTION__,
		  ep->ep.name, count,
		  is_last ? "/L" : "", req->req.length - req->req.actual, req);

	/* requests complete when all IN data is in the FIFO */
	if (is_last) {
		done(ep, req, 0);
		writel(gintmsk&(~(INT_NP_TX_FIFO_EMPTY)), S3C_UDC_OTG_GINTMSK);
		return 1;
	}

	writel(gintmsk |INT_NP_TX_FIFO_EMPTY, S3C_UDC_OTG_GINTMSK);
	return 0;

}

static __inline__ int s3c_fifo_read(struct s3c_ep *ep, u32 *cp, int max)
{
	int bytes;

	int count;
	u32 grx_status = readl(S3C_UDC_OTG_GRXSTSP);
	bytes = (grx_status & 0x7ff0)>>4;

	DEBUG_EP0("%s: GRXSTSP=0x%x, bytes=%d, ep_index=%d, fifo=0x%x\n",
			__FUNCTION__, grx_status, bytes, ep_index(ep), ep->fifo);

	// 32 bits interface
	count = bytes / 4;

	while (count--) {
		*cp++ = (u32) readl(S3C_UDC_OTG_EP0_FIFO);
	}
	return bytes;
}

static int read_fifo_ep0(struct s3c_ep *ep, struct s3c_request *req)
{
	u32 csr;
	u32 *buf;
	unsigned bufferspace, count, is_short, bytes;
	volatile u32 fifo = ep->fifo;

	DEBUG_EP0("%s\n", __FUNCTION__);

	csr = readl(S3C_UDC_OTG_GRXSTSP);
	bytes = (csr & 0x7ff0)>>4;

	buf = req->req.buf + req->req.actual;
	prefetchw(buf);
	bufferspace = req->req.length - req->req.actual;

	/* read all bytes from this packet */
	if (likely((csr & EP_MASK) == EP0_CON)) {
		count = bytes / 4;
		req->req.actual += min(bytes, bufferspace);

	} else	{		// zlp
		count = 0;
		bytes = 0;
	}

	is_short = (bytes < ep->ep.maxpacket);
	DEBUG_KEVIN("%s: read %s %02x, %d bytes%s req %p %d/%d\n",
		  __FUNCTION__,
		  ep->ep.name, csr, bytes,
		  is_short ? "/S" : "", req, req->req.actual, req->req.length);

	while (likely(count-- != 0)) {
		u32 byte = (u32) readl(fifo);

		if (unlikely(bufferspace == 0)) {
			/* this happens when the driver's buffer
			 * is smaller than what the host sent.
			 * discard the extra data.
			 */
			if (req->req.status != -EOVERFLOW)
				DEBUG_EP0("%s overflow %d\n", ep->ep.name,
					  count);
			req->req.status = -EOVERFLOW;
		} else {
			*buf++ = byte;
			bufferspace = bufferspace - 4;
		}
	}

	/* completion */
	if (is_short || req->req.actual == req->req.length) {
		return 1;
	}
	DEBUG_EP0("%s is not last\n", __FUNCTION__);

	return 0;
}

/**
 * s3c_udc_set_address - set the USB address for this device
 * @address:
 *
 * Called from control endpoint function
 * after it decodes a set address setup packet.
 */
static void s3c_udc_set_address(struct s3c_udc *dev, unsigned char address)
{
	u32 tmp = readl(S3C_UDC_OTG_DCFG);
	writel(address << 4 | tmp, S3C_UDC_OTG_DCFG);

// send ZLP : the following logic was replaced in s3c_ep0_setup
//	tmp = readl(S3C_UDC_OTG_DIEPCTL0);
//	writel(DEPCTL_EPENA|DEPCTL_CNAK|tmp, S3C_UDC_OTG_DIEPCTL0); /* EP0: Control IN */

	DEBUG_EP0("%s: USB OTG 2.0 Device address=%d, DCFG=0x%x\n",
		__FUNCTION__, address, readl(S3C_UDC_OTG_DCFG));

	dev->usb_address = address;
}

static void s3c_ep0_read(struct s3c_udc *dev)
{
	struct s3c_request *req;
	struct s3c_ep *ep = &dev->ep[EP0_CON];
	int ret, is_last = 0;
	u32 gintmsk;
	u32 ep_ctrl;
	
	DEBUG_EP0("%s: ep0 read\n", __FUNCTION__);
	
	if (!list_empty(&ep->queue))
		req = list_entry(ep->queue.next, struct s3c_request, queue);
	else {
		DEBUG("%s: NULL REQ on OUT EP-0\n", __FUNCTION__);	
		return;
	}

	DEBUG_EP0("%s: req.length = 0x%x, req.actual = 0x%x\n",
		__FUNCTION__, req->req.length, req->req.actual);

	if(req->req.length == 0) {
		
		DEBUG_EP0(" improper req->req.length == 0n");
		dev->ep0state = WAIT_FOR_SETUP;
		is_last = 1;
		done(ep, req, 0);
		return;
	}

	ret = read_fifo_ep0(ep, req);
	//ret 0: not last, 1: last packet
	gintmsk = readl(S3C_UDC_OTG_GINTMSK);
	writel(gintmsk | INT_RX_FIFO_NOT_EMPTY, S3C_UDC_OTG_GINTMSK);

	if (ret) {
		DEBUG_KEVIN("%s : ret = %d : last packet~~~\n", __FUNCTION__, ret);
		if (dev->ep0state == USB_CDC_REQ_SET_LINE_CODING)
		{
			DEBUG_EP0("%s: dev->ep0state == USB_CDC_REQ_SET_LINE_CODING\n", __FUNCTION__);
			done(ep, req, 0);
			writel(gintmsk | INT_NP_TX_FIFO_EMPTY, S3C_UDC_OTG_GINTMSK);
			return;
		}
		
		dev->ep0state = DATA_STATE_NEED_ZLP;
	//	s3c_send_zlp(); irq will call s3c_send_zlp()
		if (req->req.length == 34)
			dev->ep0state = RegReadErr;	

		is_last = 1;
		return;
	}
	else
	{
		DEBUG_EP0("%s : ret = %d : not last packet~~~\n", __FUNCTION__, ret);				
		
		ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
		writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
	}
	return;

}

/*
 * DATA_STATE_XMIT
 */
static int s3c_ep0_write(struct s3c_udc *dev)
{
	struct s3c_request *req;
	struct s3c_ep *ep = &dev->ep[EP0_CON];
	int ret, need_zlp = 0;
	u32 ep_ctrl, gintmsk;

	DEBUG_EP0("%s: ep0 write\n", __FUNCTION__);

	if (list_empty(&ep->queue))
		req = 0;
	else
		req = list_entry(ep->queue.next, struct s3c_request, queue);

	if (!req) {
		DEBUG_EP0("%s: NULL REQ\n", __FUNCTION__);
		return 0;
	}

	DEBUG_EP0("%s: req.length = 0x%x, req.actual = 0x%x\n",
		__FUNCTION__, req->req.length, req->req.actual);

	if (req->req.length == 0) {
		DEBUG_EP0("%s: req->req.length == 0\n", __FUNCTION__);

		s3c_send_zlp();
	   	done(ep, req, 0);		
		dev->ep0state = WAIT_FOR_SETUP;				

		gintmsk = readl(S3C_UDC_OTG_GINTMSK);
		writel(gintmsk&(~(INT_NP_TX_FIFO_EMPTY)), S3C_UDC_OTG_GINTMSK);

		return 1;
	}

	if (req->req.length - req->req.actual == ep0_fifo_size) {
		/* Next write will end with the packet size, */
		/* so we need Zero-length-packet */
		need_zlp = 1;
	}

	ret = s3c_ep0_write_fifo(ep, req);

	if ((ret == 1) && !need_zlp) {
		/* Last packet */
		DEBUG_KEVIN("%s: finished, waiting for status\n", __FUNCTION__);
		dev->ep0state = WAIT_FOR_SETUP;

		//writel((readl(S3C_UDC_OTG_GINTMSK) & ~INT_NP_TX_FIFO_EMPTY), S3C_UDC_OTG_GINTMSK);
	} else {
		DEBUG_EP0("%s: not finished\n", __FUNCTION__);
	}

	if (need_zlp) {
		DEBUG_EP0("%s: Need ZLP!\n", __FUNCTION__);
		dev->ep0state = DATA_STATE_NEED_ZLP;
	}

	ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
	writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);

	return 1;
}

/*
 * WAIT_FOR_SETUP (OUT_PKT_RDY)
 */
static void s3c_ep0_setup(struct s3c_udc *dev)
{
	struct s3c_ep *ep = &dev->ep[EP0_CON];
	int i, bytes, is_in;
	u32 gintmsk;

	/* Nuke all previous transfers */
	s3c_ep_nuke(ep, -EPROTO);

	/* read control req from fifo (8 bytes) */
	bytes = s3c_fifo_read(ep, (u32 *)&ctrl, 8);

	gintmsk = readl(S3C_UDC_OTG_GINTMSK);
	writel(gintmsk | INT_RX_FIFO_NOT_EMPTY, S3C_UDC_OTG_GINTMSK);

	DEBUG_SETUP("Read CTRL REQ %d bytes\n", bytes);
	DEBUG_SETUP("  CTRL.bRequestType = 0x%x (is_in %d)\n", ctrl.bRequestType,
		    ctrl.bRequestType & USB_DIR_IN);
	DEBUG_SETUP("  CTRL.bRequest = 0x%x\n", ctrl.bRequest);
	DEBUG_SETUP("  CTRL.wLength = 0x%x\n", ctrl.wLength);
	DEBUG_SETUP("  CTRL.wValue = 0x%x (%d)\n", ctrl.wValue, ctrl.wValue >> 8);
	DEBUG_SETUP("  CTRL.wIndex = 0x%x\n", ctrl.wIndex);

	/* Set direction of EP0 */
	if (likely(ctrl.bRequestType & USB_DIR_IN)) {
		ep->bEndpointAddress |= USB_DIR_IN;
		is_in = 1;
	} else {
		ep->bEndpointAddress &= ~USB_DIR_IN;
		is_in = 0;
	}

#if 1

	if ( (ctrl.bRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS ) { // CLASS TYPE
		printk("CLASS Type request. ep->bEndpointAddress : 0x%02x\n", ep->bEndpointAddress);
		switch(ctrl.bRequest)
		{
			case USB_CDC_REQ_SET_LINE_CODING :
				printk("USB_CDC_REQ_SET_LINE_CODING\n");
				dev->ep0state = USB_CDC_REQ_SET_LINE_CODING;
				//read more data
				break;
			case USB_CDC_REQ_GET_LINE_CODING :
				printk("USB_CDC_REQ_GET_LINE_CODING\n");
				printk("modify USB_DIR_IN\n");
				ep->bEndpointAddress |= USB_DIR_IN;
				break;
			case USB_CDC_REQ_SET_CONTROL_LINE_STATE :				
				printk("USB_CDC_REQ_SET_CONTROL_LINE_STATE\n");
				printk("modify USB_DIR_IN\n");
				ep->bEndpointAddress |= USB_DIR_IN;
				dev->ep0state = USB_CDC_REQ_SET_CONTROL_LINE_STATE;
				break;
		}
	}
#endif

	/* Handle some SETUP packets ourselves */
	switch (ctrl.bRequest) {

#if 1
		case USB_REQ_CLEAR_FEATURE:	//0x01
			DEBUG_SETUP("%s: *** USB_REQ_CLEAR_FEATURE \n", __FUNCTION__);
/*
			if ((ctrl.bRequestType & (USB_DIR_IN | USB_TYPE_MASK))
					!= (USB_DIR_IN | USB_TYPE_STANDARD))
			{
				DEBUG_SETUP("%s: *** USB_REQ_GET_STATUS : delegated !!!  \n",__FUNCTION__);
					break;
			}
*/
			//USB_TYPE_VENDOR = 0x02
			if ((ctrl.bRequestType & (USB_DIR_IN | USB_TYPE_MASK))
					!= (USB_DIR_IN | USB_TYPE_VENDOR))
			{
					printk("CLEAR FEATURE request.\n");
					printk("modify USB_DIR_IN\n");
					ep->bEndpointAddress |= USB_DIR_IN;
			}
			break;
#endif

		case USB_REQ_SET_ADDRESS:
			if (ctrl.bRequestType
				!= (USB_TYPE_STANDARD | USB_RECIP_DEVICE))
				break;

			DEBUG_SETUP("%s: *** USB_REQ_SET_ADDRESS (%d)\n",
					__FUNCTION__, ctrl.wValue);
			s3c_udc_set_address(dev, ctrl.wValue);
			dev->ep0state = DATA_STATE_NEED_ZLP;
			
			return;

		case USB_REQ_SET_CONFIGURATION :
			DEBUG_SETUP("============================================\n");
			DEBUG_SETUP("%s: USB_REQ_SET_CONFIGURATION (%d)\n",
					__FUNCTION__, ctrl.wValue);
config_change:
			DEBUG_SETUP("============================================\n");

			reset_available = 1;
			dev->ep0state = DATA_STATE_NEED_ZLP;
			break;

		case USB_REQ_GET_DESCRIPTOR:
			DEBUG_SETUP("%s: *** USB_REQ_GET_DESCRIPTOR  \n",__FUNCTION__);
			break;

		case USB_REQ_SET_INTERFACE:
			DEBUG_SETUP("%s: *** USB_REQ_SET_INTERFACE (%d)\n",
					__FUNCTION__, ctrl.wValue);
			goto config_change;
			break;

		case USB_REQ_GET_CONFIGURATION:
			DEBUG_SETUP("%s: *** USB_REQ_GET_CONFIGURATION  \n",__FUNCTION__);
			break;

		case USB_REQ_GET_STATUS:
			DEBUG_SETUP("%s: *** USB_REQ_GET_STATUS  \n",__FUNCTION__);
			break;

		default:
//			DEBUG_SETUP("%s: *** Default of ctrl.bRequest=0x%x happened.\n",__FUNCTION__, ctrl.bRequest);
			break;
	}

//delegate:

	if (likely(dev->driver)) {
		/* device-2-host (IN) or no data setup command,
		 * process immediately */
		spin_unlock(&dev->lock);
		i = dev->driver->setup(&dev->gadget, &ctrl);
		spin_lock(&dev->lock);

		if (i < 0) {
			/* setup processing failed, force stall */
			DEBUG_SETUP("%s: gadget setup FAILED (stalling), setup returned %d\n",
				__FUNCTION__, i);
			/* ep->stopped = 1; */
			dev->ep0state = FAIL_TO_SETUP;
		}
	}
}

/*
 * handle ep0 interrupt
 */
static void s3c_ep0_handle(struct s3c_udc *dev)
{
	if (dev->ep0state == WAIT_FOR_SETUP) {
		DEBUG_EP0("%s: WAIT_FOR_SETUP\n", __FUNCTION__);
		s3c_ep0_setup(dev);
	} else {
		DEBUG_EP0("%s: strange state!!(state = %s)\n",
			__FUNCTION__, state_names[dev->ep0state]);
		
		readl(S3C_UDC_OTG_GRXSTSP);
	}
}

static void s3c_ep0_kick(struct s3c_udc *dev, struct s3c_ep *ep)
{
	u32 gintmsk, ep_ctrl;
	u8 is_in = ep_is_in(ep);

	gintmsk = readl(S3C_UDC_OTG_GINTMSK);

	DEBUG_EP0("%s: ep_is_in = %d\n", __FUNCTION__, ep_is_in(ep));
	
	if(dev->ep0state == USB_CDC_REQ_SET_CONTROL_LINE_STATE)
	{	
			writel(gintmsk | INT_NP_TX_FIFO_EMPTY, S3C_UDC_OTG_GINTMSK);
			return;
	}
	
	if(dev->ep0state == USB_CDC_REQ_SET_LINE_CODING)
	{	
		writel(gintmsk | INT_RX_FIFO_NOT_EMPTY, S3C_UDC_OTG_GINTMSK);
		ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
		writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
		return;
	}
	
		
	if(dev->ep0state == DATA_STATE_NEED_ZLP)
	{
		DEBUG_EP0("%s: DATA_STATE_NEED_ZLP\n", __FUNCTION__);
		//zlp can be sent after setup transaction complete irq so enabel rxFifonotEmpty
		writel(gintmsk | INT_RX_FIFO_NOT_EMPTY, S3C_UDC_OTG_GINTMSK);
		ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
		writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
		return;
	}
	if (is_in) {
		dev->ep0state = DATA_STATE_XMIT;
	} else {
		dev->ep0state = DATA_STATE_RECV;
	}
	
	writel(gintmsk | INT_RX_FIFO_NOT_EMPTY, S3C_UDC_OTG_GINTMSK);
	ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
	writel(ep_ctrl | DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
}

