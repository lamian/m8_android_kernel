/* linux/arch/arm/plat-s3c/include/mach/s3c-dma.h
 *
 */

#ifndef __ARM_MACH_S3C_DMA_H
#define __ARM_MACH_S3C_DMA_H

#include <linux/sysdev.h>
#include <mach/hardware.h>

#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C6410)
#include <mach/dma-pl080.h>
#elif defined(CONFIG_CPU_S5PC100) || defined(CONFIG_CPU_S5P6440) || defined(CONFIG_CPU_S5PC110)
#include <mach/dma-pl330.h>
#endif

/* We use `virtual` dma channels to hide the fact we have only a limited
 * number of DMA channels, and not of all of them (dependant on the device)
 * can be attached to any DMA source. We therefore let the DMA core handle
 * the allocation of hardware channels to clients.
*/

enum dma_ch {
	DMACH_XD0,
	DMACH_XD1,
	DMACH_SDI,
	DMACH_SPI0,
	DMACH_SPI_TX,
	DMACH_SPI_RX,
	DMACH_SPI1,
	DMACH_UART0,
	DMACH_UART1,
	DMACH_UART2,
	DMACH_TIMER,
	DMACH_I2S_IN,
	DMACH_I2S_OUT,
	DMACH_I2S_IN_1,		/* s3c2450 iis_1 rx */
	DMACH_I2S_OUT_1,	/* s3c2450 iis_1 tx */
	DMACH_I2S_V40_IN,
	DMACH_I2S_V40_OUT,
	DMACH_I2S_V50_OUT,
	DMACH_I2S_V50_IN,
	DMACH_PCM_IN,
	DMACH_PCM_OUT,
	DMACH_MIC_IN,
	DMACH_USB_EP1,
	DMACH_USB_EP2,
	DMACH_USB_EP3,
	DMACH_USB_EP4,
	DMACH_UART0_SRC2,	/* s3c2412 second uart sources */
	DMACH_UART1_SRC2,
	DMACH_UART2_SRC2,
	DMACH_UART3,		/* s3c2443 has extra uart */
	DMACH_UART3_SRC2,
	DMACH_I2S1_IN,		/* S3C6400 */
	DMACH_I2S1_OUT,
	DMACH_SPI0_IN,
	DMACH_SPI0_OUT,
	DMACH_SPI1_IN,
	DMACH_SPI1_OUT,
	DMACH_AC97_PCM_OUT,
	DMACH_AC97_PCM_IN,
	DMACH_AC97_MIC_IN,
	DMACH_ONENAND_IN,
	DMACH_3D_M2M,
	DMACH_MAX,		/* the end entry */
};

/* types */

enum s3c_dma_state {
	S3C_DMA_IDLE,
	S3C_DMA_RUNNING,
	S3C_DMA_PAUSED
};


/* enum s3c_dma_loadst
 *
 * This represents the state of the DMA engine, wrt to the loaded / running
 * transfers. Since we don't have any way of knowing exactly the state of
 * the DMA transfers, we need to know the state to make decisions on wether
 * we can
 *
 * S3C_DMA_NONE
 *
 * There are no buffers loaded (the channel should be inactive)
 *
 * S3C_DMA_1LOADED
 *
 * There is one buffer loaded, however it has not been confirmed to be
 * loaded by the DMA engine. This may be because the channel is not
 * yet running, or the DMA driver decided that it was too costly to
 * sit and wait for it to happen.
 *
 * S3C_DMA_1RUNNING
 *
 * The buffer has been confirmed running, and not finisged
 *
 * S3C_DMA_1LOADED_1RUNNING
 *
 * There is a buffer waiting to be loaded by the DMA engine, and one
 * currently running.
*/

enum s3c_dma_loadst {
	S3C_DMALOAD_NONE,
	S3C_DMALOAD_1LOADED,
	S3C_DMALOAD_1RUNNING,
	S3C_DMALOAD_1LOADED_1RUNNING,
};

enum s3c2410_dma_buffresult {
	S3C2410_RES_OK,
	S3C2410_RES_ERR,
	S3C2410_RES_ABORT
};

enum s3c2410_dmasrc {
	S3C2410_DMASRC_HW,		/* source is memory */
	S3C2410_DMASRC_MEM,		/* source is hardware */
	S3C_DMA_MEM2MEM,      		/* source is memory - READ/WRITE */
	S3C_DMA_MEM2MEM_SET,      	/* source is memory - READ/WRITE for MEMSET*/
	S3C_DMA_MEM2MEM_P,      	/* source is hardware - READ/WRITE */
	S3C_DMA_PER2PER      		/* source is hardware - READ/WRITE */
};

/* enum s3c_chan_op
 *
 * operation codes passed to the DMA code by the user, and also used
 * to inform the current channel owner of any changes to the system state
*/

enum s3c_chan_op {
	S3C2410_DMAOP_START,
	S3C2410_DMAOP_STOP,
	S3C2410_DMAOP_PAUSE,
	S3C2410_DMAOP_RESUME,
	S3C2410_DMAOP_FLUSH,
	S3C2410_DMAOP_TIMEOUT,		/* internal signal to handler */
	S3C2410_DMAOP_STARTED,		/* indicate channel started */
	S3C2410_DMAOP_ABORT,		/* abnormal stop */
};

/* dma buffer */

struct s3c2410_dma_client {
	char                *name;
};

/* s3c2410_dma_buf_s
 *
 * internally used buffer structure to describe a queued or running
 * buffer.
*/

struct s3c_dma_buf;
struct s3c_dma_buf {
	struct s3c_dma_buf	*next;
	int			 magic;		/* magic */
	int			 size;		/* buffer size in bytes */
	dma_addr_t		 data;		/* start of DMA data */
	dma_addr_t		 ptr;		/* where the DMA got to [1] */
	void			*id;		/* client's id */
	dma_addr_t		mcptr;		/* physical pointer to a set of micro codes */
	unsigned long 		*mcptr_cpu;	/* virtual pointer to a set of micro codes */
};

/* [1] is this updated for both recv/send modes? */

struct s3c2410_dma_chan;

/* s3c2410_dma_cbfn_t
 *
 * buffer callback routine type
*/

typedef void (*s3c2410_dma_cbfn_t)(struct s3c2410_dma_chan *,
				   void *buf, int size,
				   enum s3c2410_dma_buffresult result);

typedef int  (*s3c2410_dma_opfn_t)(struct s3c2410_dma_chan *,
				   enum s3c_chan_op );

struct s3c_dma_stats {
	unsigned long		loads;
	unsigned long		timeout_longest;
	unsigned long		timeout_shortest;
	unsigned long		timeout_avg;
	unsigned long		timeout_failed;
};

struct s3c2410_dma_map;

/* struct s3c2410_dma_chan
 *
 * full state information for each DMA channel
*/

/*========================== S3C DMA ===========================================*/
typedef struct s3c_dma_controller s3c_dma_controller_t;
struct s3c_dma_controller {
	/* channel state flags and information */
	unsigned char          number;      /* number of this dma channel */
	unsigned char          in_use;      /* channel allocated and how many channel are used */
	unsigned char          irq_claimed; /* irq claimed for channel */
	unsigned char          irq_enabled; /* irq enabled for channel */
	unsigned char          xfer_unit;   /* size of an transfer */

	/* channel state */

	enum s3c_dma_state    state;
	enum s3c_dma_loadst   load_state;
	struct s3c2410_dma_client  *client;

	/* channel configuration */
	unsigned long          dev_addr;
	unsigned long          load_timeout;
	unsigned int           flags;        /* channel flags */

	/* channel's hardware position and configuration */
	void __iomem           *regs;        /* channels registers */
	void __iomem           *addr_reg;    /* data address register */
	unsigned int           irq;          /* channel irq */
	unsigned long          dcon;         /* default value of DCON */

};

struct s3c2410_dma_chan {
	/* channel state flags and information */
	unsigned char		 number;      /* number of this dma channel */
	unsigned char		 in_use;      /* channel allocated */
	unsigned char		 irq_claimed; /* irq claimed for channel */
	unsigned char		 irq_enabled; /* irq enabled for channel */
	unsigned char		 xfer_unit;   /* size of an transfer */

	/* channel state */

	enum s3c_dma_state	 state;
	enum s3c_dma_loadst	 load_state;
	struct s3c2410_dma_client *client;

	/* channel configuration */
	enum s3c2410_dmasrc	 source;
	unsigned long		 dev_addr;
	unsigned long		 load_timeout;
	unsigned int		 flags;		/* channel flags */

	struct s3c_dma_map	*map;		/* channel hw maps */

	/* channel's hardware position and configuration */
	void __iomem		*regs;		/* channels registers */
	void __iomem		*addr_reg;	/* data address register */
	unsigned int		 irq;		/* channel irq */
	unsigned long		 dcon;		/* default value of DCON */

	/* driver handles */
	s3c2410_dma_cbfn_t	 callback_fn;	/* buffer done callback */
	s3c2410_dma_opfn_t	 op_fn;		/* channel op callback */

	/* stats gathering */
	struct s3c_dma_stats *stats;
	struct s3c_dma_stats  stats_store;

	/* buffer list and information */
	struct s3c_dma_buf	*curr;		/* current dma buffer */
	struct s3c_dma_buf	*next;		/* next buffer to load */
	struct s3c_dma_buf	*end;		/* end of queue */

	/* system device */
	struct sys_device	dev;

	unsigned int            index;        	/* channel index */
	unsigned int            config_flags;        /* channel flags */
	unsigned int            control_flags;        /* channel flags */
	s3c_dma_controller_t	*dma_con;
};

/* the currently allocated channel information */
extern struct s3c2410_dma_chan s3c2410_chans[];

/* note, we don't really use dma_device_t at the moment */
typedef unsigned long dma_device_t;

struct s3c_sg_list {
	unsigned long	uSrcAddr;
	unsigned long	uDstAddr;
	unsigned long	uNextLLI;
	unsigned long	uCxControl0;
	unsigned long	uCxControl1;
};

/* functions --------------------------------------------------------------- */

/* s3c2410_dma_request
 *
 * request a dma channel exclusivley
*/

extern int s3c2410_dma_request(dmach_t channel,
			       struct s3c2410_dma_client *, void *dev);


/* s3c2410_dma_ctrl
 *
 * change the state of the dma channel
*/

extern int s3c2410_dma_ctrl(dmach_t channel, enum s3c_chan_op op);


/* s3c2410_dma_setflags
 *
 * set the channel's flags to a given state
*/

extern int s3c2410_dma_setflags(dmach_t channel,
				unsigned int flags);


/* s3c2410_dma_free
 *
 * free the dma channel (will also abort any outstanding operations)
*/

extern int s3c2410_dma_free(dmach_t channel, struct s3c2410_dma_client *);


/* s3c2410_dma_enqueue
 *
 * place the given buffer onto the queue of operations for the channel.
 * The buffer must be allocated from dma coherent memory, or the Dcache/WB
 * drained before the buffer is given to the DMA system.
*/

extern int s3c2410_dma_enqueue(dmach_t channel, void *id,
			       dma_addr_t data, int size);

extern int s3c2410_dma_enqueue_sg(dmach_t channel, void *id,
			       dma_addr_t data, int size, struct s3c_sg_list *sg_list);

/* s3c2410_dma_config
 *
 * configure the dma channel
*/

extern int s3c2410_dma_config(dmach_t channel, int xferunit, int dcon);

/* s3c2410_dma_devconfig
 *
 * configure the device we're talking to
*/

extern int s3c2410_dma_devconfig(int channel, enum s3c2410_dmasrc source,
				 int hwcfg, unsigned long devaddr);

/* s3c2410_dma_getposition
 *
 * get the position that the dma transfer is currently at
*/

extern int s3c2410_dma_getposition(dmach_t channel,
				   dma_addr_t *src, dma_addr_t *dest);


extern int s3c2410_dma_set_opfn(dmach_t, s3c2410_dma_opfn_t rtn);
extern int s3c2410_dma_set_buffdone_fn(dmach_t, s3c2410_dma_cbfn_t rtn);




#endif //__ARM_MACH_S3C_DMA_H
