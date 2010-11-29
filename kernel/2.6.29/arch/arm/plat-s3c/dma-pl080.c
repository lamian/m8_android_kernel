/* linux/arch/arm/plat-s3c/dma-pl080.c
 *
 * Copyright (c) 2003-2005,2006 Samsung Electronics
 *
 * S3C6400/S3C6410 DMA core
 *
 * http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifdef CONFIG_S3C_DMA_DEBUG
#define DEBUG
#endif

#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/sysdev.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/delay.h>

#include <asm/system.h>
#include <asm/irq.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/dma.h>

#include <plat/regs-clock.h>
#include <plat/dma.h>
#include <mach/map.h>
#include <mach/dma.h>


/* io map for dma */
static void __iomem 		*dma_base;
static struct kmem_cache 	*dma_kmem;

static int dma_channels;
struct s3c_dma_selection 	dma_sel;
static struct s3c2410_dma_chan *dma_chan_map[DMACH_MAX];

/* dma channel state information */
struct s3c2410_dma_chan 	s3c_dma_chans[S3C_DMA_CHANNELS];
s3c_dma_controller_t 		s3c_dma_cntlrs[S3C_DMA_CONTROLLERS];

#undef pr_debug		
//#define dma_dbg

#ifdef dma_dbg
#define sh_printk(fmt...) 	printk( fmt)
#define pr_debug(fmt...) 	printk( fmt)
#else
#define sh_printk(fmt...)
#define pr_debug(fmt...)
#endif

/* debugging functions */

#define BUF_MAGIC 			(0xcafebabe)

#define dmawarn(fmt...) 		printk(KERN_DEBUG fmt)

#define dma_regaddr(chan, reg) 		((chan)->regs + (reg))
#define dma_wrreg(chan, reg, val) 	writel((val), (chan)->regs + (reg))
#define dma_rdreg(chan, reg) 		readl((chan)->regs + (reg))

#define dbg_showregs(chan) 		do { } while(0)
#define dbg_showchan(chan) 		do { } while(0)

void s3c_dma_dump(int dcon_num, int channel)
{
	unsigned long tmp;
	s3c_dma_controller_t *dma_controller = &s3c_dma_cntlrs[dcon_num];

	tmp = dma_rdreg(dma_controller, S3C_DMAC_INT_STATUS);
	printk("%d dcon_num %d subchnnel INT_STATUS %lx\n", dcon_num, channel, tmp);
	tmp = dma_rdreg(dma_controller, S3C_DMAC_INT_TCSTATUS);
	printk("%d dcon_num %d subchnnel INT_TCSTATUS %lx\n", dcon_num, channel, tmp);

	tmp = dma_rdreg(dma_controller, S3C_DMAC_ENBLD_CHANNELS);
	printk("%d dcon_num %d subchnnel ENBLD_CHANNELS %lx\n", dcon_num, channel, tmp);

	tmp = dma_rdreg(dma_controller, S3C_DMAC_CONFIGURATION);
	printk("%d dcon_num %d subchnnel DMAC_CONFIGUARATION %lx\n", dcon_num, channel, tmp);

	tmp = dma_rdreg(dma_controller, S3C_DMAC_CSRCADDR(channel));
	printk("%d dcon_num %d subchnnel SRCADDRESS %lx\n", dcon_num, channel, tmp);

	tmp = dma_rdreg(dma_controller, S3C_DMAC_CDESTADDR(channel));
	printk("%d dcon_num %d subchnnel DESTADDRESS %lx\n", dcon_num, channel, tmp);

	tmp = dma_rdreg(dma_controller, S3C_DMAC_CLLI(channel));
	printk("%d dcon_num %d subchnnel LLI %lx\n", dcon_num, channel, tmp);

	tmp = dma_rdreg(dma_controller, S3C_DMAC_CCONTROL0(channel));
	printk("%d dcon_num %d subchnnel CCONTROL0 %lx\n", dcon_num, channel, tmp);

	tmp = dma_rdreg(dma_controller, S3C_DMAC_CCONTROL1(channel));
	printk("%d dcon_num %d subchnnel CCONTROL1 %lx\n", dcon_num, channel, tmp);
	tmp = dma_rdreg(dma_controller, S3C_DMAC_CCONFIGURATION(channel));

	printk("%d dcon_num %d subchnnel CH CONFIGUARATION %lx\n", dcon_num, channel, tmp);
}


/* lookup_dma_channel
 *
 * change the dma channel number given into a real dma channel id
*/

static struct s3c2410_dma_chan *lookup_dma_channel(unsigned int channel)
{
	if (channel & DMACH_LOW_LEVEL)
		return &s3c_dma_chans[channel & ~DMACH_LOW_LEVEL];
	else
		return dma_chan_map[channel];
}

/* s3c_dma_stats_timeout
 *
 * Update DMA stats from timeout info
 */
static void s3c_dma_stats_timeout(struct s3c_dma_stats * stats, int val)
{
	if (stats == NULL)
		return;

	if (val > stats->timeout_longest)
		stats->timeout_longest = val;
	if (val < stats->timeout_shortest)
		stats->timeout_shortest = val;

	stats->timeout_avg += val;
}

void s3c_enable_dmac(unsigned int dcon_num)
{
	s3c_dma_controller_t *dma_controller = &s3c_dma_cntlrs[dcon_num];
	dma_wrreg(dma_controller, S3C_DMAC_CONFIGURATION, S3C_DMA_CONTROLLER_ENABLE);
}

void s3c_disable_dmac(unsigned int dcon_num)
{
	unsigned long tmp;
	s3c_dma_controller_t *dma_controller = &s3c_dma_cntlrs[dcon_num];
	tmp = dma_rdreg(dma_controller, S3C_DMAC_CONFIGURATION);
	tmp &= ~S3C_DMA_CONTROLLER_ENABLE;
	dma_wrreg(dma_controller, S3C_DMAC_CONFIGURATION, tmp);
}

void s3c_clear_interrupts (int dcon_num, int channel)
{
	unsigned long tmp;
	s3c_dma_controller_t *dma_controller = &s3c_dma_cntlrs[dcon_num];
	
	tmp = dma_rdreg(dma_controller, S3C_DMAC_INT_TCCLEAR);
	tmp |= (1 << channel);
	dma_wrreg(dma_controller, S3C_DMAC_INT_TCCLEAR, tmp);

	tmp = dma_rdreg(dma_controller, S3C_DMAC_INT_ERRORCLEAR);
	tmp |= (1 << channel);
	dma_wrreg(dma_controller, S3C_DMAC_INT_ERRORCLEAR, tmp);
}

/* s3c_dma_waitforload
 *
 * wait for the DMA engine to load a buffer, and update the state accordingly
 */
static int s3c_dma_waitforload(struct s3c2410_dma_chan *chan, int line)
{
	int timeout = chan->load_timeout;
	int took;

	pr_debug("%s channel number : %d\n", __FUNCTION__, chan->number);

	if (chan->load_state != S3C_DMALOAD_1LOADED) {
		printk(KERN_ERR
		       "dma%d: s3c_dma_waitforload() called in loadstate %d from line %d\n",
		       chan->number, chan->load_state, line);
		return 0;
	}

	if (chan->stats != NULL)
		chan->stats->loads++;

	while (--timeout > 0) {
		if ((dma_rdreg(chan->dma_con, S3C_DMAC_ENBLD_CHANNELS)) & (0x1 << chan->number)) {
			took = chan->load_timeout - timeout;
			s3c_dma_stats_timeout(chan->stats, took);

			switch (chan->load_state) {
			case S3C_DMALOAD_1LOADED:
				chan->load_state = S3C_DMALOAD_1RUNNING;
				break;

			default:
				printk(KERN_ERR
				       "dma%d: unknown load_state in s3c_dma_waitforload() %d\n",
				       chan->number, chan->load_state);
			}

			return 1;
		}
	}

	if (chan->stats != NULL) {
		chan->stats->timeout_failed++;
	}

	return 0;
}


/* s3c_dma_loadbuffer
 *
 * load a buffer, and update the channel state
 */
static inline int s3c_dma_loadbuffer(struct s3c2410_dma_chan *chan,
		       struct s3c_dma_buf *buf)
{
	unsigned long reload;

	pr_debug("s3c_chan_loadbuffer: loading buffer %p (0x%08lx,0x%06x)\n",
		 buf, (unsigned long) buf->data, buf->size);

	if (buf == NULL) {
		dmawarn("buffer is NULL\n");
		return -EINVAL;
	}

	/* check the state of the channel before we do anything */

	if (chan->load_state == S3C_DMALOAD_1LOADED) {
		dmawarn("load_state is S3C2410_DMALOAD_1LOADED\n");
		reload = (buf->next == NULL) ? S3C2410_DCON_NORELOAD : 0;
	}

	if (chan->load_state == S3C_DMALOAD_1LOADED_1RUNNING) {
		dmawarn("state is S3C2410_DMALOAD_1LOADED_1RUNNING\n");
		reload = S3C2410_DCON_AUTORELOAD;
	}

	writel(buf->data, chan->addr_reg);

	pr_debug("%s: DMA control0 - %08x\n", __FUNCTION__, chan->dcon);
	pr_debug("%s: DMA control1 - %08x\n", __FUNCTION__, (buf->size / chan->xfer_unit));
	
	dma_wrreg(chan, S3C_DMAC_CxCONTROL0, chan->dcon);
	dma_wrreg(chan, S3C_DMAC_CxCONTROL1, (buf->size / chan->xfer_unit));
	
	chan->next = buf->next;

	/* update the state of the channel */

	switch (chan->load_state) {
	case S3C_DMALOAD_NONE:
		chan->load_state = S3C_DMALOAD_1LOADED;
		break;

	case S3C_DMALOAD_1RUNNING:
		chan->load_state = S3C_DMALOAD_1LOADED_1RUNNING;
		break;

	default:
		dmawarn("dmaload: unknown state %d in loadbuffer\n", chan->load_state);
		break;
	}

	return 0;
}


/* s3c_dma_call_op
 *
 * small routine to call the o routine with the given op if it has been
 * registered
 */
static void s3c_dma_call_op(struct s3c2410_dma_chan * chan, enum s3c_chan_op op)
{
	if (chan->op_fn != NULL) {
		(chan->op_fn) (chan, op);
	}
}

/* s3c_dma_buffdone
 *
 * small wrapper to check if callback routine needs to be called, and
 * if so, call it
 */
static inline void s3c_dma_buffdone(struct s3c2410_dma_chan * chan,
				struct s3c_dma_buf * buf,
				enum s3c2410_dma_buffresult result)
{
	pr_debug("callback_fn will be called=%p, buf=%p, id=%p, size=%d, result=%d\n",
		 chan->callback_fn, buf, buf->id, buf->size, result);

	if (chan->callback_fn != NULL) {
		(chan->callback_fn) (chan, buf->id, buf->size, result);
	}
}

/* s3c_dma_start
 *
 * start a dma channel going
 */
static int s3c_dma_start(struct s3c2410_dma_chan *chan)
{
	unsigned long flags;

	pr_debug("s3c_start_dma: channel number=%d, index=%d\n", chan->number, chan->index);

	local_irq_save(flags);

	if (chan->state == S3C_DMA_RUNNING) {
		pr_debug("s3c_start_dma: already running (%d)\n", chan->state);
		local_irq_restore(flags);
		return 0;
	}

	chan->state = S3C_DMA_RUNNING;

	/* check wether there is anything to load, and if not, see
	 * if we can find anything to load
	 */

	if (chan->load_state == S3C_DMALOAD_NONE) {
		if (chan->next == NULL) {
			printk(KERN_ERR "dma%d: dcon_num has nothing loaded\n", chan->number);
			chan->state = S3C_DMA_IDLE;
			local_irq_restore(flags);
			return -EINVAL;
		}

		s3c_dma_loadbuffer(chan, chan->next);
	}

	dbg_showchan(chan);

	/* enable the channel */

	if (!chan->irq_enabled) {
		enable_irq(chan->irq);
		chan->irq_enabled = 1;
	}

	/* Get the DMA channel  started ...*/
	dma_wrreg(chan, S3C_DMAC_CxCONFIGURATION, chan->config_flags);

	pr_debug("%s:wrote %08lx to S3C_DMAC_CxCONFIGURATION.\n",__FUNCTION__, chan->config_flags);

	/* Start the DMA operation on Peripheral */
	s3c_dma_call_op(chan, S3C2410_DMAOP_START);

	dbg_showchan(chan);

	local_irq_restore(flags);
	return 0;
}


/* s3c2410_dma_enqueue
 *
 * queue an given buffer for dma transfer.
 *
 * id         the device driver's id information for this buffer
 * data       the physical address of the buffer data
 * size       the size of the buffer in bytes
 *
 * If the channel is not running, then the flag S3C2410_DMAF_AUTOSTART
 * is checked, and if set, the channel is started. If this flag isn't set,
 * then an error will be returned.
 *
 * It is possible to queue more than one DMA buffer onto a channel at
 * once, and the code will deal with the re-loading of the next buffer
 * when necessary.
 */
int s3c2410_dma_enqueue(unsigned int channel, void *id,
			dma_addr_t data, int size)
{
	struct s3c2410_dma_chan *chan = lookup_dma_channel(channel);
	struct s3c_dma_buf *buf;
	unsigned long flags;

	pr_debug("%s: id=%p, data=%08x, size=%d\n", __FUNCTION__, id, (unsigned int) data, size);

	buf = kmem_cache_alloc(dma_kmem, GFP_ATOMIC);
	if (buf == NULL) {
		printk(KERN_ERR "dma<%d> no memory for buffer\n", channel);
		return -ENOMEM;
	}

	pr_debug("%s: new buffer %p\n", __FUNCTION__, buf);

	//dbg_showchan(chan);

	buf->next = NULL;
	buf->data = buf->ptr = data;
	buf->size = size;
	buf->id = id;
	buf->magic = BUF_MAGIC;

	local_irq_save(flags);

	if (chan->curr == NULL) {
		/* we've got nothing loaded... */
		pr_debug("%s: buffer %p queued onto empty channel\n", __FUNCTION__, buf);

		chan->curr = buf;
		chan->end = buf;
		chan->next = NULL;
	} else {
		pr_debug("dma%d: %s: buffer %p queued onto non-empty channel\n",
			 chan->number, __FUNCTION__, buf);

		if (chan->end == NULL)   /* In case of flushing */
			pr_debug("dma%d: %s: %p not empty, and chan->end==NULL?\n",
				 chan->number, __FUNCTION__, chan);
		else {
			chan->end->next = buf;
			chan->end = buf;
		}
	}

	/* if necessary, update the next buffer field */
	if (chan->next == NULL)
		chan->next = buf;

	/* check to see if we can load a buffer */
	if (chan->state == S3C_DMA_RUNNING) {
		if (chan->load_state == S3C_DMALOAD_1LOADED && 1) {
			if (s3c_dma_waitforload(chan, __LINE__) == 0) {
				printk(KERN_ERR "dma%d: loadbuffer:"
				       "timeout loading buffer\n", chan->number);
				dbg_showchan(chan);
				local_irq_restore(flags);
				return -EINVAL;
			}
		}
	} else if (chan->state == S3C_DMA_IDLE) {
		if (chan->flags & S3C2410_DMAF_AUTOSTART) {
			s3c2410_dma_ctrl(channel, S3C2410_DMAOP_START);
		} else {
			pr_debug("loading onto stopped channel\n");
		}
	}

	local_irq_restore(flags);
	return 0;
}
EXPORT_SYMBOL(s3c2410_dma_enqueue);

/* s3c2410_dma_enqueue_sg
 *
 * queue an given buffer for dma transfer.
 *
 * id         the device driver's id information for this buffer
 * data       the physical address of the buffer data
 * size       the size of the buffer in bytes
 * sg_reg     the scatter&gather list
 *
 * If the channel is not running, then the flag S3C2410_DMAF_AUTOSTART
 * is checked, and if set, the channel is started. If this flag isn't set,
 * then an error will be returned.
 *
 * It is possible to queue more than one DMA buffer onto a channel at
 * once, and the code will deal with the re-loading of the next buffer
 * when necessary.
 */
int s3c2410_dma_enqueue_sg(unsigned int channel, void *id,
			dma_addr_t data, int size, struct s3c_sg_list *sg_list)
{
	struct s3c2410_dma_chan *chan = lookup_dma_channel(channel);
	struct s3c_dma_buf *buf;
	unsigned long flags;

	pr_debug("%s: id=%p, data=%08x, size=%d\n", __FUNCTION__, id, (unsigned int) data, size);

	buf = kmem_cache_alloc(dma_kmem, GFP_ATOMIC);
	if (buf == NULL) {
		printk(KERN_ERR "dma<%d> no memory for buffer\n", channel);
		return -ENOMEM;
	}

	pr_debug("%s: new buffer %p\n", __FUNCTION__, buf);

	//dbg_showchan(chan);

	buf->next = NULL;
	buf->data = buf->ptr = data;
	buf->size = size;
	buf->id = id;
	buf->magic = BUF_MAGIC;

	local_irq_save(flags);

	if (chan->curr == NULL) {
		/* we've got nothing loaded... */
		pr_debug("%s: buffer %p queued onto empty channel\n", __FUNCTION__, buf);

		chan->curr = buf;
		chan->end = buf;
		chan->next = NULL;
	} else {
		pr_debug("dma%d: %s: buffer %p queued onto non-empty channel\n",
			 chan->number, __FUNCTION__, buf);

		if (chan->end == NULL)   /* In case of flushing */
			pr_debug("dma%d: %s: %p not empty, and chan->end==NULL?\n",
				 chan->number, __FUNCTION__, chan);
		else {
			chan->end->next = buf;
			chan->end = buf;
		}
	}

	/* if necessary, update the next buffer field */
	if (chan->next == NULL)
		chan->next = buf;

	/* check to see if we can load a buffer */
	if (chan->state == S3C_DMA_RUNNING) {
		if (chan->load_state == S3C_DMALOAD_1LOADED && 1) {
			if (s3c_dma_waitforload(chan, __LINE__) == 0) {
				printk(KERN_ERR "dma%d: loadbuffer:"
				       "timeout loading buffer\n", chan->number);
				dbg_showchan(chan);
				local_irq_restore(flags);
				return -EINVAL;
			}
		}
	} else if (chan->state == S3C_DMA_IDLE) {
		dma_wrreg(chan, S3C_DMAC_CxLLI, virt_to_phys(&sg_list));
		
		if (chan->flags & S3C2410_DMAF_AUTOSTART) {
			s3c2410_dma_ctrl(channel, S3C2410_DMAOP_START);
		} else {
			pr_debug("loading onto stopped channel\n");
		}
	}

	local_irq_restore(flags);
	return 0;
}
EXPORT_SYMBOL(s3c2410_dma_enqueue_sg);


static inline void s3c_dma_freebuf(struct s3c_dma_buf * buf)
{
	int magicok = (buf->magic == BUF_MAGIC);

	buf->magic = -1;

	if (magicok) {
		kmem_cache_free(dma_kmem, buf);
	} else {
		printk("s3c_dma_freebuf: buff %p with bad magic\n", buf);
	}
}

/* s3c_dma_lastxfer
 *
 * called when the system is out of buffers, to ensure that the channel
 * is prepared for shutdown.
 */
static inline void s3c_dma_lastxfer(struct s3c2410_dma_chan *chan)
{
	pr_debug("DMA CH %d: s3c_dma_lastxfer: load_state %d\n", chan->number, chan->load_state);
	
	switch (chan->load_state) {
	case S3C_DMALOAD_NONE:
		pr_debug("DMA CH %d: s3c_dma_lastxfer: load_state : S3C2_DMALOAD_NONE%d\n", chan->number);
		break;

	case S3C_DMALOAD_1LOADED:
		if (s3c_dma_waitforload(chan, __LINE__) == 0) {
			/* flag error? */
			printk(KERN_ERR "dma%d: timeout waiting for load\n", chan->number);
			return;
		}
		break;

	default:
		pr_debug("dma%d: lastxfer: unhandled load_state %d with no next",
			 chan->number, chan->load_state);
		return;

	}

}


#define dmadbg2(x...)

static irqreturn_t s3c_dma_irq(int irq, void *devpw)
{
	unsigned int channel = 0, dcon_num, i;
	unsigned long tmp;
	s3c_dma_controller_t *dma_controller = (s3c_dma_controller_t *) devpw;
	
	struct s3c2410_dma_chan *chan=NULL;
	struct s3c_dma_buf *buf;

	dcon_num = dma_controller->number;
	tmp = dma_rdreg(dma_controller, S3C_DMAC_INT_TCSTATUS);
	pr_debug("# s3c_dma_irq: TC status : 0x%x\n", tmp);

	if (tmp == 0)
		return IRQ_HANDLED;

	for (i = 0; i < S3C_CHANNELS_PER_DMA; i++) {
		if (tmp & 0x01) {

			pr_debug("# DMA Controller %d: requestor %d\n", dcon_num, i);
			
			channel = i;
			chan = &s3c_dma_chans[channel + dcon_num * S3C_CHANNELS_PER_DMA];
			pr_debug("# DMA channel number : %d, index : %d\n", chan->number, chan->index);
			
			buf = chan->curr;
			
			dbg_showchan(chan);

			/* modify the channel state */
			switch (chan->load_state) {
			case S3C_DMALOAD_1RUNNING:
				/* TODO - if we are running only one buffer, we probably
				 * want to reload here, and then worry about the buffer
				 * callback */

				chan->load_state = S3C_DMALOAD_NONE;
				break;

			case S3C_DMALOAD_1LOADED:
				/* iirc, we should go back to NONE loaded here, we
				 * had a buffer, and it was never verified as being
				 * loaded.
				 */

				chan->load_state = S3C_DMALOAD_NONE;
				break;

			case S3C_DMALOAD_1LOADED_1RUNNING:
				/* we'll worry about checking to see if another buffer is
				 * ready after we've called back the owner. This should
				 * ensure we do not wait around too long for the DMA
				 * engine to start the next transfer
				 */

				chan->load_state = S3C_DMALOAD_1LOADED;
				break;

			case S3C_DMALOAD_NONE:
				printk(KERN_ERR "dma%d: IRQ with no loaded buffer?\n",
				       chan->number);
				break;

			default:
				printk(KERN_ERR "dma%d: IRQ in invalid load_state %d\n",
				       chan->number, chan->load_state);
				break;
			}

			if (buf != NULL) {
				/* update the chain to make sure that if we load any more
				 * buffers when we call the callback function, things should
				 * work properly */

				chan->curr = buf->next;
				buf->next = NULL;

				if (buf->magic != BUF_MAGIC) {
					printk(KERN_ERR "dma%d: %s: buf %p incorrect magic\n",
					       chan->number, __FUNCTION__, buf);
					goto next_channel;
				}

				s3c_dma_buffdone(chan, buf, S3C2410_RES_OK);

				/* free resouces */
				s3c_dma_freebuf(buf);
			} else {
			
			}

			if (chan->next != NULL) {
				unsigned long flags;

				switch (chan->load_state) {
				case S3C_DMALOAD_1RUNNING:
					/* don't need to do anything for this state */
					break;

				case S3C_DMALOAD_NONE:
					/* can load buffer immediately */
					break;

				case S3C_DMALOAD_1LOADED:
					if (s3c_dma_waitforload(chan, __LINE__) == 0) {
						/* flag error? */
						printk(KERN_ERR "dma%d: timeout waiting for load\n",
						       chan->number);
						goto next_channel;
					}

					break;

				case S3C_DMALOAD_1LOADED_1RUNNING:
					goto next_channel;

				default:
					printk(KERN_ERR "dma%d: unknown load_state in irq, %d\n",
					       chan->number, chan->load_state);
					goto next_channel;
				}

				local_irq_save(flags);
				s3c_dma_loadbuffer(chan, chan->next);
				
				//shaju added for dbg
				dma_wrreg(chan, S3C_DMAC_CxCONFIGURATION, chan->config_flags);
				
				local_irq_restore(flags);
				
			} else {
				s3c_dma_lastxfer(chan);
				
				/* see if we can stop this channel.. */
				if (chan->load_state == S3C_DMALOAD_NONE) {
					pr_debug("# DMA CH %d(index:%d): end of transfer, stopping channel (%ld)\n",
						 chan->number, chan->index, jiffies);
					s3c2410_dma_ctrl(chan->index | DMACH_LOW_LEVEL, S3C2410_DMAOP_STOP);
				}
			}
			s3c_clear_interrupts(chan->dma_con->number, chan->number);

		}

next_channel:
		tmp >>= 1;
	}

	return IRQ_HANDLED;
}

static struct s3c2410_dma_chan *s3c_dma_map_channel(int channel);

/* s3c_request_dma
 *
 * get control of an dma channel
*/

int s3c2410_dma_request(unsigned int channel,
			struct s3c2410_dma_client *client,
			void *dev)
{
	struct s3c2410_dma_chan *chan;
	unsigned long flags;
	int err;

	pr_debug("DMA CH %d: s3c2410_request_dma: client=%s, dev=%p\n",
		 channel, client->name, dev);

	local_irq_save(flags);

	chan = s3c_dma_map_channel(channel);
	if (chan == NULL) {
		local_irq_restore(flags);
		return -EBUSY;
	}

	dbg_showchan(chan);

	chan->client = client;
	chan->in_use = 1;

	chan->dma_con->in_use++;

	if (!chan->irq_claimed) {
		pr_debug("DMA CH %d: %s : requesting irq %d\n",
			 channel, __FUNCTION__, chan->irq);

		chan->irq_claimed = 1;
		local_irq_restore(flags);

		err = request_irq(chan->irq, s3c_dma_irq, IRQF_SHARED | IRQF_DISABLED,
				  client->name, (void *) chan->dma_con);

		local_irq_save(flags);

		if (err) {
			chan->in_use = 0;
			chan->irq_claimed = 0;
			chan->dma_con->in_use--;
			local_irq_restore(flags);

			printk(KERN_ERR "%s: cannot get IRQ %d for DMA %d\n",
			       client->name, chan->irq, chan->number);
			return err;
		}

		chan->irq_enabled = 1;
		
		/* enable the main dma.. this can be disabled
		 * when main channel use count is 0 */
		s3c_enable_dmac(chan->dma_con->number);
	}
	
	s3c_clear_interrupts(chan->dma_con->number, chan->number);
	local_irq_restore(flags);

	/* need to setup */

	pr_debug("%s: channel initialised, %p, number:%d, index:%d\n", __FUNCTION__, chan, chan->number, chan->index);

	return 0;
}
EXPORT_SYMBOL(s3c2410_dma_request);

/* s3c_dma_free
 *
 * release the given channel back to the system, will stop and flush
 * any outstanding transfers, and ensure the channel is ready for the
 * next claimant.
 *
 * Note, although a warning is currently printed if the freeing client
 * info is not the same as the registrant's client info, the free is still
 * allowed to go through.
 */
int s3c2410_dma_free(dmach_t channel, struct s3c2410_dma_client *client)
{
	unsigned long flags;
	struct s3c2410_dma_chan *chan = lookup_dma_channel(channel);
	pr_debug("%s: DMA channel %d will be stopped\n", __FUNCTION__, chan->number);

	if (chan == NULL)
		return -EINVAL;

	local_irq_save(flags);

	if (chan->client != client) {
		printk(KERN_WARNING
		       "DMA CH %d: possible free from different client (channel %p, passed %p)\n",
		       channel, chan->client, client);
	}

	/* sort out stopping and freeing the channel */

	if (chan->state != S3C_DMA_IDLE) {
		pr_debug("%s: need to stop dma channel %p\n", __FUNCTION__, chan);

		/* possibly flush the channel */
		s3c2410_dma_ctrl(channel, S3C2410_DMAOP_STOP);
	}

	chan->client = NULL;
	chan->in_use = 0;
	
	chan->dma_con->in_use--;

	if (chan->irq_claimed)
		free_irq(chan->irq, (void *)chan->dma_con);

	chan->irq_claimed = 0;

	if (!(channel & DMACH_LOW_LEVEL))
		dma_chan_map[channel] = NULL;

	local_irq_restore(flags);

	return 0;
}
EXPORT_SYMBOL(s3c2410_dma_free);

/*actively polling for the A bit can block the cpu*/
void s3c_dma_flush_fifo(struct s3c2410_dma_chan *chan)
{
	unsigned long tmp;
	
	tmp = dma_rdreg(chan, S3C_DMAC_CxCONFIGURATION);
	tmp |= S3C_DMACONFIG_HALT;
	dma_wrreg(chan, S3C_DMAC_CxCONFIGURATION, tmp);

	tmp = dma_rdreg(chan, S3C_DMAC_CxCONFIGURATION);
	
	/*this while loop can be very dangerous..may be put the process to sleep rather than waiting till fifo is drained */
	while (tmp & S3C_DMACONFIG_ACTIVE) {
		tmp = dma_rdreg(chan, S3C_DMAC_CxCONFIGURATION);
	}
}

static int s3c_dma_abort(struct s3c2410_dma_chan *chan) 
{
	unsigned long tmp;
	unsigned long flags;

	pr_debug("%s: DMA Channel No : %d\n", __FUNCTION__, chan->number);

	dbg_showchan(chan);

	local_irq_save(flags);

	s3c_dma_call_op(chan, S3C2410_DMAOP_ABORT);

	tmp = dma_rdreg(chan, S3C_DMAC_CxCONFIGURATION);
	
	tmp &= ~S3C_DMACONFIG_CHANNEL_ENABLE;
	dma_wrreg(chan, S3C_DMAC_CxCONFIGURATION, tmp);
	
	pr_debug("%s: S3C_DMAC_CxCONFIGURATION : %08x\n", __FUNCTION__, tmp);

	chan->state = S3C_DMA_IDLE;
	chan->load_state = S3C_DMALOAD_NONE;

	local_irq_restore(flags);

	return 0;
}

static int s3c_dma_dostop(struct s3c2410_dma_chan *chan) 
{
	unsigned long tmp;
	unsigned long flags;

	pr_debug("%s: DMA Channel No : %d\n", __FUNCTION__, chan->number);

	dbg_showchan(chan);

	local_irq_save(flags);

	s3c_dma_flush_fifo(chan);
	
	s3c_dma_call_op(chan, S3C2410_DMAOP_STOP);

	tmp = dma_rdreg(chan, S3C_DMAC_CxCONFIGURATION);
	
	tmp &= ~S3C_DMACONFIG_CHANNEL_ENABLE;
	dma_wrreg(chan, S3C_DMAC_CxCONFIGURATION, tmp);
	
	pr_debug("%s: S3C_DMAC_CxCONFIGURATION : %08x\n", __FUNCTION__, tmp);

	chan->state = S3C_DMA_IDLE;
	chan->load_state = S3C_DMALOAD_NONE;

	local_irq_restore(flags);

	return 0;
}

static void s3c_dma_showchan(struct s3c2410_dma_chan * chan)
{

}

/* s3c_dma_flush
 *
 * stop the channel, and remove all current and pending transfers
 */

void s3c_waitforstop(struct s3c2410_dma_chan *chan)
{
	
}

static int s3c_dma_flush(struct s3c2410_dma_chan *chan)
{
	struct s3c_dma_buf *buf, *next;
	unsigned long flags;

	pr_debug("%s:\n", __FUNCTION__);

	local_irq_save(flags);

	s3c_dma_showchan(chan);

	if (chan->state != S3C_DMA_IDLE) {
		pr_debug("%s: stopping channel...\n", __FUNCTION__);
		s3c2410_dma_ctrl(chan->number, S3C2410_DMAOP_STOP);
	}

	buf = chan->curr;
	if (buf == NULL)
		buf = chan->next;

	chan->curr = chan->next = chan->end = NULL;
	chan->load_state = S3C_DMALOAD_NONE;

	if (buf != NULL) {
		for (; buf != NULL; buf = next) {
			next = buf->next;

			pr_debug("%s: free buffer %p, next %p\n", __FUNCTION__, buf, buf->next);

			s3c_dma_buffdone(chan, buf, S3C2410_RES_ABORT);
			s3c_dma_freebuf(buf);
		}
	}
	//s3c_dma_waitforstop(chan);
	
	s3c_dma_showchan(chan);
	local_irq_restore(flags);

	return 0;
}

int s3c_dma_started(struct s3c2410_dma_chan *chan)
{
	unsigned long flags;

	local_irq_save(flags);

	dbg_showchan(chan);

	/* if we've only loaded one buffer onto the channel, then chec
	 * to see if we have another, and if so, try and load it so when
	 * the first buffer is finished, the new one will be loaded onto
	 * the channel */

	if (chan->next != NULL) {
		if (chan->load_state == S3C_DMALOAD_1LOADED) {

			if (s3c_dma_waitforload(chan, __LINE__) == 0) {
				pr_debug("%s: buff not yet loaded, no more todo\n",
					 __FUNCTION__);
			} else {
				chan->load_state = S3C_DMALOAD_1RUNNING;
				s3c_dma_loadbuffer(chan, chan->next);
			}

		} else if (chan->load_state == S3C_DMALOAD_1RUNNING) {
			s3c_dma_loadbuffer(chan, chan->next);
		}
	}

	local_irq_restore(flags);

	return 0;

}

int s3c2410_dma_ctrl(dmach_t channel, enum s3c_chan_op op)
{
	struct s3c2410_dma_chan *chan = lookup_dma_channel(channel);

	if (chan == NULL)
		return -EINVAL;

	switch (op) {
	case S3C2410_DMAOP_START:
		return s3c_dma_start(chan);

	case S3C2410_DMAOP_STOP:
		return s3c_dma_dostop(chan);

	case S3C2410_DMAOP_PAUSE:
	case S3C2410_DMAOP_RESUME:
		return -ENOENT;

	case S3C2410_DMAOP_FLUSH:
		return s3c_dma_flush(chan);

	case S3C2410_DMAOP_STARTED:
		return s3c_dma_started(chan);

	case S3C2410_DMAOP_TIMEOUT:
		return 0;

	case S3C2410_DMAOP_ABORT:
		return s3c_dma_abort(chan);
	}

	return -ENOENT;      /* unknown, don't bother */
}
EXPORT_SYMBOL(s3c2410_dma_ctrl);


/* s3c_dma_config
 *
 * xfersize:     size of unit in bytes (1,2,4)
 * dcon:         base value of the DCONx register
 */
int s3c2410_dma_config(dmach_t channel,
		       int xferunit,
		       int dcon)
{
	struct s3c2410_dma_chan *chan = lookup_dma_channel(channel);

	pr_debug("%s: chan=%d, xfer_unit=%d, dcon=%08x\n",
		 __FUNCTION__, channel, xferunit, dcon);

	if (chan == NULL)
		return -EINVAL;

	pr_debug("%s: Initial dcon is %08x\n", __FUNCTION__, dcon);

	dcon |= chan->dcon & dma_sel.dcon_mask;

	pr_debug("%s: New dcon is %08x\n", __FUNCTION__, dcon);
	
	switch (xferunit) {
	case 1:
		dcon |= S3C_DMACONTROL_SRC_WIDTH_BYTE;
		dcon |= S3C_DMACONTROL_DEST_WIDTH_BYTE;
		break;

	case 2:
		dcon |= S3C_DMACONTROL_SRC_WIDTH_HWORD;
		dcon |= S3C_DMACONTROL_DEST_WIDTH_HWORD;
		break;

	case 4:
		dcon |= S3C_DMACONTROL_SRC_WIDTH_WORD;
		dcon |= S3C_DMACONTROL_DEST_WIDTH_WORD;
		break;

	default:
		pr_debug("%s: Bad transfer size %d\n", __FUNCTION__, xferunit);
		return -EINVAL;
	}

	pr_debug("%s: DMA Channel control :  %08x\n", __FUNCTION__, dcon);

	dcon |= S3C_DMACONTROL_TC_INT_ENABLE;
	dcon |= chan->control_flags;
	pr_debug("%s: dcon now %08x\n", __FUNCTION__, dcon);

	/* For DMCCxControl 0 */
	chan->dcon = dcon;

	/* For DMACCxControl 1 : xferunit means transfer width.*/
	chan->xfer_unit = xferunit;

	return 0;
}
EXPORT_SYMBOL(s3c2410_dma_config);


int s3c2410_dma_setflags(dmach_t channel, unsigned int flags)
{
	struct s3c2410_dma_chan *chan = lookup_dma_channel(channel);

	if (chan == NULL)
		return -EINVAL;

	pr_debug("%s: chan=%p, flags=%08x\n", __FUNCTION__, chan, flags);

	chan->flags = flags;

	return 0;
}
EXPORT_SYMBOL(s3c2410_dma_setflags);


/* do we need to protect the settings of the fields from
 * irq?
 */

int s3c2410_dma_set_opfn(dmach_t channel, s3c2410_dma_opfn_t rtn)
{
	struct s3c2410_dma_chan *chan = lookup_dma_channel(channel);

	if (chan == NULL)
		return -EINVAL;

	pr_debug("%s: chan=%p, op rtn=%p\n", __FUNCTION__, chan, rtn);

	chan->op_fn = rtn;

	return 0;
}
EXPORT_SYMBOL(s3c2410_dma_set_opfn);


int s3c2410_dma_set_buffdone_fn(dmach_t channel, s3c2410_dma_cbfn_t rtn)
{
	struct s3c2410_dma_chan *chan = lookup_dma_channel(channel);

	if (chan == NULL)
		return -EINVAL;

	pr_debug("%s: chan=%p, callback rtn=%p\n", __FUNCTION__, chan, rtn);

	chan->callback_fn = rtn;

	return 0;
}
EXPORT_SYMBOL(s3c2410_dma_set_buffdone_fn);


/* s3c2410_dma_devconfig
 *
 * configure the dma source/destination hardware type and address
 *
 * flowctrl: direction of dma flow
 *
 * src_per dst_per: dma channel number of src and dst periphreal,
 *
 * devaddr:   physical address of the source
 */

int s3c2410_dma_devconfig(int channel,
			  enum s3c2410_dmasrc source,
			  int hwcfg,
			  unsigned long devaddr)
{
	unsigned long tmp;

	struct s3c2410_dma_chan *chan = lookup_dma_channel(channel);

	if (chan == NULL)
		return -EINVAL;

	pr_debug("%s: source=%d, hwcfg=%08x, devaddr=%08lx\n",
		 __FUNCTION__, (int)source, hwcfg, devaddr);

	chan->source = source;
	chan->dev_addr = devaddr;

	switch (source) {
	case S3C2410_DMASRC_MEM:
		/* source is Memory : Mem-to-Peri ( Write into FIFO) */
		tmp = S3C_DMACONFIG_TCMASK | S3C_DMACONFIG_FLOWCTRL_MEM2PER | (chan->map->hw_addr.to) <<
			S3C_DEST_SHIFT | S3C_DMACONFIG_CHANNEL_ENABLE;
		
		chan->config_flags = tmp;

		/* TODO : Now, Scatter&Gather DMA NOT supported */
		dma_wrreg(chan, S3C_DMAC_CxLLI, 0);
		
		/* devaddr : Periperal address (destination) */
		dma_wrreg(chan, S3C_DMAC_CxDESTADDR, devaddr);

		/* source address : memory(buffer) address */
		chan->addr_reg = dma_regaddr(chan, S3C_DMAC_CxSRCADDR);
		
		chan->control_flags = S3C_DMACONTROL_SRC_INC | S3C_DMACONTROL_DEST_AXI_PERI ;
		//chan->control_flags = hwcfg;
		return 0;

	case S3C2410_DMASRC_HW:
		/* source is peripheral : Peri-to-Mem ( Read from FIFO) */
		tmp = S3C_DMACONFIG_TCMASK | S3C_DMACONFIG_FLOWCTRL_PER2MEM | (chan->map->hw_addr.from) <<
			S3C_SRC_SHIFT | S3C_DMACONFIG_CHANNEL_ENABLE;
		
		chan->config_flags = tmp;

		/* TODO : Now, Scatter&Gather DMA NOT supported */
		dma_wrreg(chan, S3C_DMAC_CxLLI, 0);
		
		/* devaddr : Periperal address (source) */
		dma_wrreg(chan, S3C_DMAC_CxSRCADDR, devaddr);

		/* destination address : memory(buffer) address */
		chan->addr_reg = dma_regaddr(chan, S3C_DMAC_CxDESTADDR);
		
		chan->control_flags = S3C_DMACONTROL_DEST_INC | S3C_DMACONTROL_SRC_AXI_PERI;
		//chan->control_flags = hwcfg;
		
		return 0;

	case S3C_DMA_MEM2MEM:
		/* this is temporary for G3D */
		tmp = S3C_DMACONFIG_TCMASK | S3C_DMACONFIG_FLOWCTRL_MEM2MEM | S3C_DMACONFIG_CHANNEL_ENABLE;
		
		chan->config_flags = tmp;

		/* TODO : Now, Scatter&Gather DMA NOT YET supported */
		dma_wrreg(chan, S3C_DMAC_CxLLI, 0);
		
		/* devaddr : memory/onenand address (source) */
		dma_wrreg(chan, S3C_DMAC_CxSRCADDR, devaddr);

		/* destination address : memory(buffer) address */
		chan->addr_reg = dma_regaddr(chan, S3C_DMAC_CxDESTADDR);
	
		chan->control_flags |= (S3C_DMACONTROL_SRC_INC | S3C_DMACONTROL_DEST_AXI_PERI
				| S3C_DMACONTROL_SBSIZE_4 | S3C_DMACONTROL_DBSIZE_4);
		//chan->control_flags = hwcfg;
		
		return 0;

	case S3C_DMA_MEM2MEM_P:
		/* source is memory : Memory-to-Mem ( Read/Write) */
		tmp = S3C_DMACONFIG_TCMASK | S3C_DMACONFIG_FLOWCTRL_MEM2MEM | S3C_DMACONFIG_CHANNEL_ENABLE;

		//if(chan->map->hw_addr.from == S3C_DMA0_ONENAND_RX) {
			//tmp |= S3C_DMACONFIG_ONENANDMODESRC;
		//}
		
		chan->config_flags = tmp;

		/* TODO : Now, Scatter&Gather DMA NOT YET supported */
		dma_wrreg(chan, S3C_DMAC_CxLLI, 0);
		
		/* devaddr : memory/onenand address (source) */
		dma_wrreg(chan, S3C_DMAC_CxSRCADDR, devaddr);

		/* destination address : memory(buffer) address */
		chan->addr_reg = dma_regaddr(chan, S3C_DMAC_CxDESTADDR);
		
		chan->control_flags |= (S3C_DMACONTROL_SRC_INC | S3C_DMACONTROL_DEST_INC 
				| S3C_DMACONTROL_SBSIZE_4 | S3C_DMACONTROL_DBSIZE_4);
		//chan->control_flags = hwcfg;
		
		return 0;

	case S3C_DMA_PER2PER:
		printk("Peripheral-to-Peripheral DMA NOT YET implemented !! \n");
		return -EINVAL;

	default:
		printk(KERN_ERR "DMA CH :%d - invalid source type ()\n", channel);
		printk("Unsupported DMA configuration from the device driver using DMA driver \n");
		return -EINVAL;
	}
	
}

EXPORT_SYMBOL(s3c2410_dma_devconfig);


/* 
 * s3c_dma_getposition
 * returns the current transfer points for the dma source and destination
 */
int s3c2410_dma_getposition(dmach_t channel, dma_addr_t *src, dma_addr_t *dst)
{
 	struct s3c2410_dma_chan *chan = lookup_dma_channel(channel);

	if (chan == NULL)
		return -EINVAL;

	if (src != NULL)
 		*src = dma_rdreg(chan, S3C_DMAC_CxSRCADDR);
	
 	if (dst != NULL)
 		*dst = dma_rdreg(chan, S3C_DMAC_CxDESTADDR);

 	return 0;
}

EXPORT_SYMBOL(s3c2410_dma_getposition);


/* system device class */
#ifdef CONFIG_PM
static int s3c_dma_suspend (struct sys_device *dev, pm_message_t state)
{
	return 0;
}

static int s3c_dma_resume (struct sys_device *dev)
{
	return 0;
}
#else
#define s3c_dma_suspend NULL
#define s3c_dma_resume  NULL
#endif				/* CONFIG_PM */

struct sysdev_class dma_sysclass = {
	.name = "s3c64xx-dma",
	.suspend = s3c_dma_suspend,
	.resume = s3c_dma_resume,
};

/* kmem cache implementation */

static void s3c_dma_cache_ctor(void *p)
{
	memset(p, 0, sizeof(struct s3c_dma_buf));
}


void dma_test (int dcon_num, int channel)
{
	int tmp;
	
	s3c_dma_controller_t *dma_controller = &s3c_dma_cntlrs[dcon_num];

	dma_wrreg(dma_controller, S3C_DMAC_CONFIGURATION, S3C_DMA_CONTROLLER_ENABLE);
	tmp = dma_rdreg(dma_controller, S3C_DMAC_CONFIGURATION);
	printk("reg val %d\n", tmp);
	dma_wrreg(dma_controller, S3C_DMAC_CCONFIGURATION(channel), 0x01);
	tmp = dma_rdreg(dma_controller, S3C_DMAC_CCONFIGURATION(channel));

	printk("reg conf %x\n", tmp);
	dma_wrreg(dma_controller, S3C_DMAC_CCONTROL0(channel), 0x8ff02064);

	tmp = dma_rdreg(dma_controller, S3C_DMAC_CCONTROL0(channel));
	printk("reg ctrl %x\n", tmp);
}


/* initialisation code */

int __init s3c_dma_init(unsigned int channels, unsigned int irq,
			    unsigned int stride)
{
	struct s3c2410_dma_chan *cp;
	s3c_dma_controller_t *dconp;
	int channel, controller;
	int ret;

	printk("S3C DMA-pl080 Controller Driver, (c) 2006-2007 Samsung Electronics\n");

	dma_channels = channels;
	printk("Total %d DMA channels will be initialized.\n", channels);

	ret = sysdev_class_register(&dma_sysclass);
	if (ret != 0) {
		printk(KERN_ERR "dma sysclass registration failed.\n");
		goto err;
	}

	dma_kmem = kmem_cache_create("dma_desc", sizeof(struct s3c_dma_buf), 0,
				     SLAB_HWCACHE_ALIGN, (void *)s3c_dma_cache_ctor);

	if (dma_kmem == NULL) {
		printk(KERN_ERR "DMA failed to make kmem cache\n");
		ret = -ENOMEM;
		goto err;
	}

	for (controller = 0; controller < S3C_DMA_CONTROLLERS; controller++) {
		dconp = &s3c_dma_cntlrs[controller];

		memset(dconp, 0, sizeof(s3c_dma_controller_t));

		if(controller < 2) {
			dma_base = ioremap((S3C_PA_DMA + (controller * 0x100000)), 0x200);
			if (dma_base == NULL) {
				printk(KERN_ERR "DMA failed to ioremap register block\n");
				return -ENOMEM;
			}

			/* dma controller's irqs are in order.. */
			dconp->irq = controller + IRQ_DMA0;
		}
		else {
			dma_base = ioremap(((S3C_PA_DMA + 0x8B00000) + ((controller%2) * 0x100000)), 0x200);
			if (dma_base == NULL) {
				printk(KERN_ERR "SDMA failed to ioremap register block\n");
				return -ENOMEM;
			}

			/* dma controller's irqs are in order.. */
			dconp->irq = (controller%2) + IRQ_SDMA0;
		}
		
		dconp->number = controller;
		dconp->regs = dma_base;
		sh_printk("DMA controller : %d irq %d regs_base %x\n", dconp->number, dconp->irq,
			  dconp->regs);
	}

	for (channel = 0; channel < channels; channel++) {
		controller = channel / S3C_CHANNELS_PER_DMA;
		cp = &s3c_dma_chans[channel];

		memset(cp, 0, sizeof(struct s3c2410_dma_chan));

		cp->dma_con = &s3c_dma_cntlrs[controller];
		
		/* dma channel irqs are in order.. */
		cp->index = channel;
		cp->number = channel%S3C_CHANNELS_PER_DMA;
		
		cp->irq = s3c_dma_cntlrs[controller].irq;
		
		cp->regs = s3c_dma_cntlrs[controller].regs + ((channel%S3C_CHANNELS_PER_DMA)*stride) + 0x100;

		/* point current stats somewhere */
		cp->stats = &cp->stats_store;
		cp->stats_store.timeout_shortest = LONG_MAX;

		/* basic channel configuration */
		cp->load_timeout = 1 << 18;

		/* register system device */
		cp->dev.cls = &dma_sysclass;
		cp->dev.id = channel;
		//ret = sysdev_register(&cp->dev);

		sh_printk("DMA channel %d at %p, irq %d\n", cp->number, cp->regs, cp->irq);
	}

	return 0;

err:
	kmem_cache_destroy(dma_kmem);
	iounmap(dma_base);
	dma_base = NULL;
	return ret;
}



static inline int is_channel_valid(unsigned int channel)
{
	return (channel & DMA_CH_VALID);
}

static struct s3c_dma_order *dma_order;


/* s3c_dma_map_channel()
 *
 * turn the virtual channel number into a real, and un-used hardware
 * channel.
 *
 * first, try the dma ordering given to us by either the relevant
 * dma code, or the board. Then just find the first usable free
 * channel
*/

struct s3c2410_dma_chan *s3c_dma_map_channel(int channel)
{
	struct s3c_dma_order_ch *ord = NULL;
	struct s3c_dma_map *ch_map;
	struct s3c2410_dma_chan *dmach;
	int ch;

	if (dma_sel.map == NULL || channel > dma_sel.map_size)
		return NULL;

	ch_map = dma_sel.map + channel;

	/* first, try the board mapping */

	if (dma_order) {
		ord = &dma_order->channels[channel];

		for (ch = 0; ch < dma_channels; ch++) {
			if (!is_channel_valid(ord->list[ch]))
				continue;

			if (s3c_dma_chans[ord->list[ch]].in_use == 0) {
				ch = ord->list[ch] & ~DMA_CH_VALID;
				goto found;
			}
		}

		if (ord->flags & DMA_CH_NEVER)
			return NULL;
	}

	/* second, search the channel map for first free */

	for (ch = 0; ch < dma_channels; ch++) {
		if (!is_channel_valid(ch_map->channels[ch]))
			continue;

		if (s3c_dma_chans[ch].in_use == 0) {
			pr_debug("mapped channel %d to %d\n", channel, ch);
			break;
		}
	}

	if (ch >= dma_channels)
		return NULL;

	/* update our channel mapping */

 found:
	dmach = &s3c_dma_chans[ch];
	dma_chan_map[channel] = dmach;

	/* select the channel */
	(dma_sel.select)(dmach, ch_map);

	return dmach;
}

static int s3c_dma_check_entry(struct s3c_dma_map *map, int ch)
{
	unsigned long tmp = __raw_readl(S3C_SDMA_SEL);
	
	tmp |= map->sdma_sel;
	__raw_writel(tmp, S3C_SDMA_SEL);
	
	return 0;
}

int __init s3c_dma_init_map(struct s3c_dma_selection *sel)
{
	struct s3c_dma_map *nmap;
	size_t map_sz = sizeof(*nmap) * sel->map_size;
	int ptr;

	nmap = kmalloc(map_sz, GFP_KERNEL);
	if (nmap == NULL)
		return -ENOMEM;

	memcpy(nmap, sel->map, map_sz);
	memcpy(&dma_sel, sel, sizeof(*sel));

	dma_sel.map = nmap;

	for (ptr = 0; ptr < sel->map_size; ptr++)
		s3c_dma_check_entry(nmap+ptr, ptr);

	return 0;
}

int __init s3c_dma_order_set(struct s3c_dma_order *ord)
{
	struct s3c_dma_order *nord = dma_order;

	if (nord == NULL)
		nord = kmalloc(sizeof(struct s3c_dma_order), GFP_KERNEL);

	if (nord == NULL) {
		printk(KERN_ERR "no memory to store dma channel order\n");
		return -ENOMEM;
	}

	dma_order = nord;
	memcpy(nord, ord, sizeof(struct s3c_dma_order));
	return 0;
}


