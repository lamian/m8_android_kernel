/*
 * linux/drivers/video/s3c_pp_6400.c
 *
 * Revision 3.0
 *
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 *	    S3C PostProcessor driver 
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/errno.h> /* error codes */
#include <asm/div64.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>
#include <mach/map.h>
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/mman.h>
#include <linux/version.h>

#include <plat/regs-pp.h>
#include <plat/regs-lcd.h>
#include <plat/regs-clock.h>
#include <plat/reserved_mem.h>
#include <plat/pm.h>
#include <plat/power-clock-domain.h>

#include "s3c_pp.h"   // ioctl
#include "s3c_pp_common.h" // internal used struct & type

#ifdef CONFIG_S3C64XX_DOMAIN_GATING
#define USE_PP_DOMAIN_GATING
#endif /* CONFIG_S3C64XX_DOMAIN_GATING */

//#define DEBUG

#ifdef DEBUG
#define dprintk printk
#else
#define dprintk(format,args...)
#endif

#define PFX "s3c_pp"

// if you want to modify src/dst buffer size, modify below defined size 
//#define PP_RESERVED_MEM_SIZE 	    8*1024*1024	// 8mb
//#define PP_RESERVED_MEM_ADDR_PHY    POST_RESERVED_MEM_START

#define ALLOC_KMEM		        1

#define PP_MAX_NO_OF_INSTANCES  4

#define PP_VALUE_CHANGED_NONE             0x00
#define PP_VALUE_CHANGED_PARAMS           0x01
#define PP_VALUE_CHANGED_SRC_BUF_ADDR_PHY 0x02
#define PP_VALUE_CHANGED_DST_BUF_ADDR_PHY 0x04

#define PP_INSTANCE_FREE                  0
#define PP_INSTANCE_READY                 1
#define PP_INSTANCE_INUSE_DMA_ONESHOT     2
#define PP_INSTANCE_INUSE_FIFO_FREERUN    3

typedef struct {
	int             running_instance_no;
	int             last_running_instance_no;
	int             fifo_mode_instance_no;
	unsigned int    wincon0_value_before_fifo_mode;
	int             dma_mode_instance_count;
	int             in_use_instance_count;
	int             flag_clk_enable;
	int             num_of_nonblocking_mode;
	unsigned char   instance_state[PP_MAX_NO_OF_INSTANCES]; 
} s3c_pp_instance_info_t;

static s3c_pp_instance_info_t s3c_pp_instance_info;

static struct resource *s3c_pp_mem;

static void __iomem *s3c_pp_base;
static int s3c_pp_irq = NO_IRQ;

static struct clk *pp_clock;

static struct mutex *h_mutex;
static struct mutex *mem_alloc_mutex;

static wait_queue_head_t waitq;

static int flag = 0;

static unsigned int physical_address;

static void clk_pp_enable(void)
{
	__raw_writel((__raw_readl(S3C_HCLK_GATE) | 1 << 5), S3C_HCLK_GATE);
	__raw_writel((__raw_readl(S3C_SCLK_GATE) | 1 << 10 | 1 << 12), S3C_SCLK_GATE);
}

static void clk_pp_disable(void)
{
	__raw_writel((__raw_readl(S3C_HCLK_GATE) & ~(1 << 5)), S3C_HCLK_GATE);
	__raw_writel((__raw_readl(S3C_SCLK_GATE) & ~(1 << 10) & ~(1 << 12)), S3C_SCLK_GATE);
}
// int FIFO_mode_down_sequence = 0;  //. d: sichoi 090112 (FIFO_FREERUN mode relase without using interrupt)

void set_scaler_register(s3c_pp_scaler_info_t * scaler_info, s3c_pp_instance_context_t *pp_instance)
{
	__raw_writel((scaler_info->pre_v_ratio<<7)|(scaler_info->pre_h_ratio<<0), s3c_pp_base + S3C_VPP_PRESCALE_RATIO);
	__raw_writel((scaler_info->pre_dst_height<<12)|(scaler_info->pre_dst_width<<0), s3c_pp_base + S3C_VPP_PRESCALEIMGSIZE);
	__raw_writel(scaler_info->sh_factor, s3c_pp_base + S3C_VPP_PRESCALE_SHFACTOR);
	__raw_writel(scaler_info->dx, s3c_pp_base + S3C_VPP_MAINSCALE_H_RATIO);
	__raw_writel(scaler_info->dy, s3c_pp_base + S3C_VPP_MAINSCALE_V_RATIO);
	__raw_writel((pp_instance->src_height<<12)|(pp_instance->src_width), s3c_pp_base + S3C_VPP_SRCIMGSIZE);
	__raw_writel((pp_instance->dst_height<<12)|(pp_instance->dst_width), s3c_pp_base + S3C_VPP_DSTIMGSIZE);
}


void set_src_addr_register ( s3c_pp_buf_addr_t *buf_addr, s3c_pp_instance_context_t *pp_instance )
{
	switch(pp_instance->src_color_space)
	{
		case PAL1 :
		case PAL2 :
		case PAL4 :
		case PAL8 :
		case RGB8 :
		case ARGB8 :
		case RGB16 :
		case ARGB16 :
		case RGB18 :
		case RGB24 :
		case RGB30 :
		case ARGB24 :
		{
			__raw_writel(buf_addr->in_start_rgb,  s3c_pp_base + S3C_VPP_ADDRSTART_Y);
			__raw_writel(buf_addr->in_offset_rgb, s3c_pp_base + S3C_VPP_OFFSET_Y);
			__raw_writel(buf_addr->in_end_rgb,    s3c_pp_base + S3C_VPP_ADDREND_Y);	
			break;
		}
		case YC420 :
		case YC422 :
		{
			__raw_writel(buf_addr->in_start_y,    s3c_pp_base + S3C_VPP_ADDRSTART_Y);
			__raw_writel(buf_addr->in_offset_y,   s3c_pp_base + S3C_VPP_OFFSET_Y);
			__raw_writel(buf_addr->in_end_y,      s3c_pp_base + S3C_VPP_ADDREND_Y);
		
			__raw_writel(buf_addr->in_start_cb,   s3c_pp_base + S3C_VPP_ADDRSTART_CB);
			__raw_writel(buf_addr->in_offset_cr,  s3c_pp_base + S3C_VPP_OFFSET_CB);
			__raw_writel(buf_addr->in_end_cb,     s3c_pp_base + S3C_VPP_ADDREND_CB);

			__raw_writel(buf_addr->in_start_cr,   s3c_pp_base + S3C_VPP_ADDRSTART_CR);
			__raw_writel(buf_addr->in_offset_cb,  s3c_pp_base + S3C_VPP_OFFSET_CR);
			__raw_writel(buf_addr->in_end_cr,     s3c_pp_base + S3C_VPP_ADDREND_CR);
			break;
		}
		case CRYCBY :
		case CBYCRY :
		case YCRYCB :
		case YCBYCR :
		case YUV444 :
		{
			__raw_writel(buf_addr->in_start_y,    s3c_pp_base + S3C_VPP_ADDRSTART_Y);
			__raw_writel(buf_addr->in_offset_y,   s3c_pp_base + S3C_VPP_OFFSET_Y);
			__raw_writel(buf_addr->in_end_y,      s3c_pp_base + S3C_VPP_ADDREND_Y);
			break;
		}
		default:
		{
			printk(KERN_ERR "not matched pp_instance->src_color_space(%d)\n", pp_instance->src_color_space);
			break;
		}
	}
}

void set_dest_addr_register ( s3c_pp_buf_addr_t *buf_addr, s3c_pp_instance_context_t *pp_instance )
{
	if(PP_INSTANCE_INUSE_DMA_ONESHOT == s3c_pp_instance_info.instance_state[pp_instance->instance_no] )
	{
		switch(pp_instance->dst_color_space)
		{
			case PAL1 :
			case PAL2 :
			case PAL4 :
			case PAL8 :
			case RGB8 :
			case ARGB8 :
			case RGB16 :
			case ARGB16 :
			case RGB18 :
			case RGB24 :
			case RGB30 :
			case ARGB24 :
			{
				__raw_writel(buf_addr->out_start_rgb,  s3c_pp_base + S3C_VPP_ADDRSTART_RGB);
				__raw_writel(buf_addr->out_offset_rgb, s3c_pp_base + S3C_VPP_OFFSET_RGB);
				__raw_writel(buf_addr->out_end_rgb,    s3c_pp_base + S3C_VPP_ADDREND_RGB);	
				break;
			}
			case YC420 :
			case YC422 :
			{
				__raw_writel(buf_addr->out_start_y,    s3c_pp_base + S3C_VPP_ADDRSTART_RGB);
				__raw_writel(buf_addr->out_offset_y,   s3c_pp_base + S3C_VPP_OFFSET_RGB);
				__raw_writel(buf_addr->out_end_y,      s3c_pp_base + S3C_VPP_ADDREND_RGB);

				__raw_writel(buf_addr->out_start_cb,   s3c_pp_base + S3C_VPP_ADDRSTART_OCB);
				__raw_writel(buf_addr->out_offset_cb,  s3c_pp_base + S3C_VPP_OFFSET_OCB);
				__raw_writel(buf_addr->out_end_cb,     s3c_pp_base + S3C_VPP_ADDREND_OCB);
				
				__raw_writel(buf_addr->out_start_cr,   s3c_pp_base + S3C_VPP_ADDRSTART_OCR);
				__raw_writel(buf_addr->out_offset_cr,  s3c_pp_base + S3C_VPP_OFFSET_OCR);
				__raw_writel(buf_addr->out_end_cr,     s3c_pp_base + S3C_VPP_ADDREND_OCR);
				break;
			}
			case CRYCBY :
			case CBYCRY :
			case YCRYCB :
			case YCBYCR :
			case YUV444 :
			{
				__raw_writel(buf_addr->out_start_y,    s3c_pp_base + S3C_VPP_ADDRSTART_RGB);
				__raw_writel(buf_addr->out_offset_y,   s3c_pp_base + S3C_VPP_OFFSET_RGB);
				__raw_writel(buf_addr->out_end_y,      s3c_pp_base + S3C_VPP_ADDREND_RGB);
				break;
			}
			default:
			{
				printk(KERN_ERR "not matched pp_instance->dst_color_space(%d)\n", pp_instance->dst_color_space);
				break;
			}
		}
	}
}

void set_src_next_addr_register(s3c_pp_buf_addr_t *buf_addr, s3c_pp_instance_context_t *pp_instance)
{
	switch(pp_instance->src_color_space)
	{
		case RGB30 :
		{
			__raw_writel(buf_addr->in_start_rgb,s3c_pp_base + S3C_VPP_NXTADDRSTART_Y);
			__raw_writel(buf_addr->in_end_rgb,  s3c_pp_base + S3C_VPP_NXTADDRSTART_Y);	
			break;
		}
		case YUV444 :
		{
			__raw_writel(buf_addr->in_start_y,  s3c_pp_base + S3C_VPP_NXTADDRSTART_Y);
			__raw_writel(buf_addr->in_end_y,    s3c_pp_base + S3C_VPP_NXTADDREND_Y);
			break;
		}
		default :
		{
			printk(KERN_ERR "not matched pp_instance->src_color_space(%d)\n", pp_instance->src_color_space);
			break;
		}
	}
}

void set_data_format_register(s3c_pp_instance_context_t *pp_instance)
{
	u32 tmp;

	tmp  = __raw_readl(s3c_pp_base + S3C_VPP_MODE);
	tmp |= (0x1<<16);
	tmp |= (0x2<<10);

	// set the source color space
	switch(pp_instance->src_color_space)
	{
		case YC420:
		case YC422:
			tmp &= ~((0x1<<3)|(0x1<<2));
			tmp |=  (0x1<<8)|(0x1<<1);
			break;	
		case CRYCBY:
			tmp &= ~((0x1<<15)|(0x1<<8)|(0x1<<3)|(0x1<<0));
			tmp |=  (0x1<<2)|(0x1<<1);
			break;
		case CBYCRY:
			tmp &= ~((0x1<<8)|(0x1<<3)|(0x1<<0));
			tmp |=  (0x1<<15)|(0x1<<2)|(0x1<<1);
			break;
		case YCRYCB:
			tmp &= ~((0x1<<15)|(0x1<<8)|(0x1<<3));
			tmp |=  (0x1<<2)|(0x1<<1)|(0x1<<0);
			break;
		case YCBYCR:
			tmp &= ~((0x1<<8)|(0x1<<3));
			tmp |=  (0x1<<15)|(0x1<<2)|(0x1<<1)|(0x1<<0);	
			break;
		case RGB24:
			tmp &= ~(0x1<<8);
			tmp |=  (0x1<<3)|(0x1<<2)|(0x1<<1);
			break;
		case RGB16:
			tmp &= ~((0x1<<8)|(0x1<<1));
			tmp |=  (0x1<<3)|(0x1<<2);
			break;
		default:
			break;
	}

	// set the destination color space
	if ( PP_INSTANCE_INUSE_DMA_ONESHOT == s3c_pp_instance_info.instance_state[pp_instance->instance_no] ) 
	{
		switch ( pp_instance->dst_color_space ) 
		{
			case YC420:
			case YC422:
				tmp &= ~(0x1<<18);
				tmp |=  (0x1<<17);
				break;
			case CRYCBY:
				tmp &= ~((0x1<<20)|(0x1<<19)|(0x1<<18)|(0x1<<17));
				break;
			case CBYCRY:
				tmp &= ~((0x1<<19)|(0x1<<18)|(0x1<<17));
				tmp |=  (0x1<<20);
				break;
			case YCRYCB:
				tmp &= ~((0x1<<20)|(0x1<<18)|(0x1<<17));
				tmp |=  (0x1<<19);
				break;
			case YCBYCR:
				tmp &= ~((0x1<<18)|(0x1<<17));
				tmp |=  (0x1<<20)|(0x1<<19);	
				break;
			case RGB24:
				tmp |=  (0x1<<18)|(0x1<<4);
				break;
			case RGB16:
				tmp &= ~(0x1<<4);
				tmp |=  (0x1<<18);
				break;
			default:
				break;
		}
	}
	else if ( PP_INSTANCE_INUSE_FIFO_FREERUN == s3c_pp_instance_info.instance_state[pp_instance->instance_no]) 
	{
		if (pp_instance->dst_color_space == RGB30) 
		{
			tmp |=  (0x1<<18)|(0x1<<13); 
		} 
		else if(pp_instance->dst_color_space == YUV444) 
		{
			tmp |=  (0x1<<13);
			tmp &= ~(0x1<<18)|(0x1<<17);
		} 	
	}

	__raw_writel ( tmp, s3c_pp_base + S3C_VPP_MODE );
}

static void set_clock_src ( s3c_pp_clk_src_t clk_src )
{
	u32 tmp;

	tmp = __raw_readl(s3c_pp_base + S3C_VPP_MODE);

	if ( clk_src == HCLK ) 
	{
		if (clk_get_rate(pp_clock) > 66000000)
		{
			tmp &= ~(0x7f<<23);
			tmp |= (1<<24);
			tmp |= (1<<23);
		} 
		else 
			tmp &=~ (0x7f<<23);
		}
	else if(clk_src == PLL_EXT) 
	{
	} 
	else 
	{
		tmp &=~(0x7f<<23);
	}

	tmp = (tmp &~ (0x3<<21)) | (clk_src<<21);

	__raw_writel(tmp, s3c_pp_base + S3C_VPP_MODE);
}


static void post_int_enable(u32 int_type)
{
	u32 tmp;

	tmp = __raw_readl(s3c_pp_base + S3C_VPP_MODE);

	if(int_type == 0) {		//Edge triggering
		tmp &= ~(S3C_MODE_IRQ_LEVEL);
	} else if(int_type == 1) {	//level triggering
		tmp |= S3C_MODE_IRQ_LEVEL;
	}

	tmp |= S3C_MODE_POST_INT_ENABLE;

	__raw_writel(tmp, s3c_pp_base + S3C_VPP_MODE);
}

#if 0
static void post_int_disable(void)
{
	u32 tmp;

	tmp = __raw_readl(s3c_pp_base + S3C_VPP_MODE);

	tmp &=~ S3C_MODE_POST_INT_ENABLE;

	__raw_writel(tmp, s3c_pp_base + S3C_VPP_MODE);
}
#endif

static void start_processing(void)
{
	__raw_writel(0x1<<31, s3c_pp_base + S3C_VPP_POSTENVID);
}

s3c_pp_state_t post_get_processing_state(void)
{
	s3c_pp_state_t	state;
	u32 tmp;

	tmp = __raw_readl(s3c_pp_base + S3C_VPP_POSTENVID);
	if (tmp & S3C_VPP_POSTENVID)
	{
		state = POST_BUSY;
	}
	else
	{
		state = POST_IDLE;
	}

	dprintk("Post processing state = %d\n", state);

	return state;
}

#if 0
static void s3c_dma_mode ( void )
{
	unsigned int temp;

	temp = __raw_readl(s3c_pp_base + S3C_VPP_MODE);  
	temp &=~(0x1<<13);  // dma
	__raw_writel(temp, s3c_pp_base + S3C_VPP_MODE);

#if 1
	/*
	   temp = __raw_readl ( S3C_WINCON0 );
	   temp &= ~(S3C_WINCONx_BPPMODE_F_MASK | (0x3<<9));
	   temp &= ~(S3C_WINCONx_ENLOCAL_POST | S3C_WINCONx_INRGB_MASK);
	   temp &= ~( S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BYTSWP_ENABLE | S3C_WINCONx_BITSWP_ENABLE ); //swap disable
	   temp |= S3C_WINCONx_HAWSWP_ENABLE;
	   temp |= S3C_WINCONx_BPPMODE_F_16BPP_565;  
	   temp |= S3C_WINCONx_ENWIN_F_ENABLE;  
	   __raw_writel ( temp, S3C_WINCON0 );

	   temp = __raw_readl(s3c_pp_base + S3C_VPP_MODE);  
	   temp &=~(0x1<<12);  // 0: progressive mode, 1: interlace mode
	   temp &=~(0x1<<13);  // dma
	   __raw_writel(temp, s3c_pp_base + S3C_VPP_MODE);
	   */
#else
	__raw_writel(0x0<<31, s3c_pp_base + S3C_VPP_POSTENVID);

	// PP shot mode first
	temp  = __raw_readl(s3c_pp_base + S3C_VPP_MODE);  
	temp &= ~(1<<14);    // one shot
	__raw_writel(temp, s3c_pp_base + S3C_VPP_MODE);

	//.[ display controller setting (DMA mode)
	temp  = __raw_readl ( S3C_VIDCON0 );
	temp &= ~( S3C_VIDCON0_ENVID_F_ENABLE | S3C_VIDCON0_ENVID_ENABLE );
	__raw_writel ( temp, S3C_VIDCON0 );

	temp  = __raw_readl ( S3C_WINCON0 );
	temp &= ~S3C_WINCONx_ENWIN_F_ENABLE;
	__raw_writel ( temp, S3C_WINCON0 );

	temp  = __raw_readl ( S3C_WINCON0 );
	temp &= ~(S3C_WINCONx_BPPMODE_F_MASK | (0x3<<9));
	temp &= ~(S3C_WINCONx_ENLOCAL_POST | S3C_WINCONx_INRGB_MASK);
	temp &= ~( S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BYTSWP_ENABLE | S3C_WINCONx_BITSWP_ENABLE ); //swap disable
	temp |= S3C_WINCONx_HAWSWP_ENABLE;
	temp |= S3C_WINCONx_BPPMODE_F_16BPP_565;  
	temp |= S3C_WINCONx_ENWIN_F_ENABLE;  
	__raw_writel ( temp, S3C_WINCON0 );

	temp  = __raw_readl ( S3C_VIDCON0 );
	temp |= ( S3C_VIDCON0_ENVID_F_ENABLE | S3C_VIDCON0_ENVID_ENABLE );
	__raw_writel ( temp, S3C_VIDCON0 );
	//.] display controller setting (DMA mode)

	i=0;
	while ( (temp = __raw_readl ( s3c_pp_base + S3C_VPP_POSTENVID )) )
	{
		i++; 
		__raw_writel(0x0<<31, s3c_pp_base + S3C_VPP_POSTENVID);
	}

	temp = __raw_readl(s3c_pp_base + S3C_VPP_MODE);  

	temp &= ~(0x1<<12);   // 0: progressive mode, 1: interlace mode
	temp &= ~(0x1<<31);

	temp &= ~(0x1<<13);  // dma

	__raw_writel(temp, s3c_pp_base + S3C_VPP_MODE);
#endif
}
#endif

static void pp_dma_mode_set_and_start ( void )
{
	unsigned int temp;

	__raw_writel(0x0<<31, s3c_pp_base + S3C_VPP_POSTENVID);

	temp = __raw_readl(s3c_pp_base + S3C_VPP_MODE);  

	temp &= ~(0x1<<31);     // must be 0
	temp &= ~(0x1<<13);     // dma
	temp &= ~(0x1<<14);     // per frame mode

	__raw_writel(temp, s3c_pp_base + S3C_VPP_MODE);

	__raw_writel(0x1<<31, s3c_pp_base + S3C_VPP_POSTENVID); // PP start
}

static void pp_fifo_mode_set_and_start ( s3c_pp_instance_context_t *current_instance )
{
	unsigned int temp;

#if 1
	// line count check
	while ( __raw_readl ( S3C_VIDCON1 ) & 0x07FF0000 )
	{
	}

	//.[ i: sichoi 090112 (FIFO_FREERUN mode relase without using interrupt)
	temp  = __raw_readl ( S3C_WINCON0 );
	temp &= ~S3C_WINCONx_ENWIN_F_ENABLE;
	__raw_writel ( temp, S3C_WINCON0 );
	//.] sichoi 090112 (FIFO_FREERUN mode relase without using interrupt)

	__raw_writel(0x0<<31, s3c_pp_base + S3C_VPP_POSTENVID);

	temp = __raw_readl(s3c_pp_base + S3C_VPP_MODE);  

	temp &= ~(0x1<<31);
	if ( PROGRESSIVE_MODE == current_instance->scan_mode )
	{
		temp &= ~(0x1<<12);     // 0: progressive mode, 1: interlace mode
	}
	else
	{
		temp |= (0x1<<12);      // 0: progressive mode, 1: interlace mode
	}

	temp |= (0x1<<13);          // fifo
	temp |= (0x1<<14);          // free run

	__raw_writel(temp, s3c_pp_base + S3C_VPP_MODE);

	__raw_writel(0x1<<31, s3c_pp_base + S3C_VPP_POSTENVID); // PP start


	temp = __raw_readl ( S3C_WINCON0 ); //. i: sichoi 090112 (FIFO_FREERUN mode relase without using interrupt)

	//.[ d: sichoi 090112 (FIFO_FREERUN mode relase without using interrupt)
	/*
	   temp &= ~S3C_WINCONx_ENWIN_F_ENABLE;
	   __raw_writel ( temp, S3C_WINCON0 );
	   */
	//.] sichoi 090112 (FIFO_FREERUN mode relase without using interrupt)

	temp = current_instance->dst_width * current_instance->dst_height;
	__raw_writel ( temp, S3C_VIDOSD0C );

	temp  = __raw_readl ( S3C_WINCON0 );
	temp &= ~(S3C_WINCONx_BPPMODE_F_MASK | (0x3<<9));
	temp &= ~(S3C_WINCONx_ENLOCAL_POST | S3C_WINCONx_INRGB_MASK);
	temp &= ~( S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BYTSWP_ENABLE | S3C_WINCONx_BITSWP_ENABLE ); //swap disable
	temp |= S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BURSTLEN_4WORD;
	temp |= S3C_WINCONx_ENLOCAL_POST | S3C_WINCONx_ENWIN_F_ENABLE;

	__raw_writel ( temp, S3C_WINCON0 );

#else
	//.[ display controller setting (FIFO mode)
	// line count check
	while ( __raw_readl ( S3C_VIDCON1 ) & 0x07FF0000 )
	{
	}

	__raw_writel(0x0<<31, s3c_pp_base + S3C_VPP_POSTENVID);

	temp = __raw_readl(s3c_pp_base + S3C_VPP_MODE);  

	temp &= ~(0x1<<12);   // 0: progressive mode, 1: interlace mode
	temp &= ~(0x1<<31);

	temp |=  (0x1<<13);      // fifo
	temp |=  (1<<14);        // free run

	__raw_writel(temp, s3c_pp_base + S3C_VPP_MODE);

	__raw_writel(0x1<<31, s3c_pp_base + S3C_VPP_POSTENVID); // PP start

	temp  = __raw_readl ( S3C_VIDCON0 );
	temp &= ~( S3C_VIDCON0_ENVID_F_ENABLE | S3C_VIDCON0_ENVID_ENABLE );
	__raw_writel ( temp, S3C_VIDCON0 );

	temp  = __raw_readl ( S3C_WINCON0 );
	temp &= ~S3C_WINCONx_ENWIN_F_ENABLE;
	__raw_writel ( temp, S3C_WINCON0 );

	temp  = __raw_readl ( S3C_WINCON0 );
	temp &= ~(S3C_WINCONx_BPPMODE_F_MASK | (0x3<<9));
	temp &= ~(S3C_WINCONx_ENLOCAL_POST | S3C_WINCONx_INRGB_MASK);
	temp &= ~( S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BYTSWP_ENABLE | S3C_WINCONx_BITSWP_ENABLE ); //swap disable
	temp |= S3C_WINCONx_BPPMODE_F_24BPP_888 | S3C_WINCONx_BURSTLEN_4WORD;
	temp |= S3C_WINCONx_ENLOCAL_POST | S3C_WINCONx_ENWIN_F_ENABLE;

	__raw_writel ( temp, S3C_WINCON0 );

	temp = current_instance->dst_width * current_instance->dst_height;
	__raw_writel ( temp, S3C_VIDOSD0C );

	temp  = __raw_readl ( S3C_VIDCON0 );
	temp |= S3C_VIDCON0_ENVID_F_ENABLE | S3C_VIDCON0_ENVID_ENABLE;
	__raw_writel ( temp, S3C_VIDCON0 );
	//.] display controller setting (FIFO mode)
#endif
}

static irqreturn_t s3c_pp_isr (int irq, void *dev_id, struct pt_regs *regs)
{
	u32 temp;

	temp = __raw_readl(s3c_pp_base + S3C_VPP_MODE);
	temp &= ~(S3C_MODE_POST_PENDING | S3C_MODE_POST_INT_ENABLE); // int disable & pending bit clear
	__raw_writel(temp, s3c_pp_base + S3C_VPP_MODE);

	//.[ d: sichoi 090112 (FIFO_FREERUNmode relase without using interrupt)
#if 0
	switch ( FIFO_mode_down_sequence )
	{
		case 1: 
			temp &= ~S3C_MODE_AUTOLOAD_ENABLE; // set Per Frame mode
			__raw_writel(temp, s3c_pp_base + S3C_VPP_MODE);
			break;
		case 2:
#if 1
			temp = s3c_pp_instance_info.wincon0_value_before_fifo_mode & ~S3C_WINCONx_ENWIN_F_ENABLE;
			__raw_writel ( temp, S3C_WINCON0 );
#else
			temp  = __raw_readl ( S3C_WINCON0 );
			temp &= ~S3C_WINCONx_ENWIN_F_ENABLE;
			temp &= ~(S3C_WINCONx_BPPMODE_F_MASK | (0x3<<9));
			temp &= ~(S3C_WINCONx_ENLOCAL_POST | S3C_WINCONx_INRGB_MASK);
			temp &= ~( S3C_WINCONx_HAWSWP_ENABLE | S3C_WINCONx_BYTSWP_ENABLE | S3C_WINCONx_BITSWP_ENABLE ); //swap disable
			temp |= S3C_WINCONx_HAWSWP_ENABLE;
			temp |= S3C_WINCONx_BPPMODE_F_16BPP_565;  
			__raw_writel ( temp, S3C_WINCON0 );
#endif

			break;
		default:
			break;
	}
#endif
	//.] sichoi 090112 (FIFO_FREERUN mode relase without using interrupt)

	s3c_pp_instance_info.running_instance_no = -1;

	wake_up_interruptible(&waitq);

	return IRQ_HANDLED;
}


int s3c_pp_open(struct inode *inode, struct file *file) 
{ 	
	s3c_pp_instance_context_t *current_instance;
	int i;

	mutex_lock(h_mutex);

	// fifo mode로 되어 있으면 다른 instance Open 불가
	if ( s3c_pp_instance_info.fifo_mode_instance_no != -1 )
	{
		printk(KERN_ERR "PP instance allocation is fail: Fifo Mode was already opened.\n");
		mutex_unlock(h_mutex);
		return -1;
	}

	// check instance pool 
	if (PP_MAX_NO_OF_INSTANCES <= s3c_pp_instance_info.in_use_instance_count)
	{
		printk(KERN_ERR "PP instance allocation is fail: No more instance.\n");
		mutex_unlock(h_mutex);
		return -1;
	}

	// allocating the post processor instance
	current_instance = (s3c_pp_instance_context_t *) kmalloc(sizeof(s3c_pp_instance_context_t), GFP_DMA|GFP_ATOMIC );
	if (current_instance == NULL) {
		printk(KERN_ERR "PP instance allocation is fail: Kmalloc failed.\n");
		mutex_unlock(h_mutex);
		return -1;
	}

	// Find free instance
	for (i=0; i < PP_MAX_NO_OF_INSTANCES; i++)
	{
		if (PP_INSTANCE_FREE == s3c_pp_instance_info.instance_state[i])
		{
			s3c_pp_instance_info.instance_state[i] = PP_INSTANCE_READY;
			s3c_pp_instance_info.in_use_instance_count++;
			break;
		}
	}
	if (PP_MAX_NO_OF_INSTANCES == i)
	{
		kfree (current_instance);
		printk(KERN_ERR "PP instance allocation is fail: No more instance.\n");
		mutex_unlock(h_mutex);
		return -1;
	}

	memset (current_instance, 0, sizeof(s3c_pp_instance_context_t));
	current_instance->instance_no = i;

	// check first time
	if (1 == s3c_pp_instance_info.in_use_instance_count)
	{
		// PP HW Initalize or clock enable; for Power Saving
	}
	if(file->f_flags & O_NONBLOCK)
		s3c_pp_instance_info.num_of_nonblocking_mode++;

	dprintk ( KERN_DEBUG "%s PP instance allocation is success. (%d)\n", __FUNCTION__, i );

	file->private_data = (s3c_pp_instance_context_t *) current_instance;

	mutex_unlock(h_mutex);

	return 0;
}


int s3c_pp_read ( struct file *file, char *buf, size_t count, loff_t *f_pos )
{
	return 0;
}

int s3c_pp_write ( struct file *file, const char *buf, size_t count, loff_t *f_pos )
{
	return 0;
}

int s3c_pp_mmap ( struct file *file, struct vm_area_struct *vma )
{
	u32 page_frame_no, size, phys_addr;
	void *virt_addr;

	mutex_lock ( mem_alloc_mutex );

	size = vma->vm_end - vma->vm_start;

	switch (flag) 
	{ 
		case ALLOC_KMEM :
			virt_addr = kmalloc ( size, GFP_DMA|GFP_ATOMIC ); // old setting is GFP_KERNEL

			if ( virt_addr == NULL) 
			{
				printk ( KERN_ERR "%s kmalloc() failed !\n", __FUNCTION__ );
				mutex_unlock(mem_alloc_mutex);
				return -EINVAL;
			}

			dprintk (KERN_DEBUG "MMAP_KMALLOC : virt addr = 0x%p, size = %d\n", virt_addr, (unsigned int) size);

			phys_addr = virt_to_phys(virt_addr);
			physical_address = (unsigned int) phys_addr;

			page_frame_no = __phys_to_pfn(phys_addr);

			vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

			if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED)) 
			{
				printk ( KERN_ERR "%s: Writable G3D_SFR_SIZE mapping must be shared !\n", __FUNCTION__ );
				mutex_unlock(mem_alloc_mutex);
				return -EINVAL;
			}

			if (remap_pfn_range(vma, vma->vm_start, page_frame_no, size, vma->vm_page_prot)) 
			{
				printk ( KERN_ERR "%s: remap_pfn_range() failed !\n", __FUNCTION__ );
				mutex_unlock(mem_alloc_mutex);
				return -EINVAL;
			}
			break;

		default :
/*
			page_frame_no = __phys_to_pfn(PP_RESERVED_MEM_ADDR_PHY);

			max_size = PP_RESERVED_MEM_SIZE + PAGE_SIZE - (PP_RESERVED_MEM_SIZE % PAGE_SIZE);

			if ( max_size < size ) 
			{
				mutex_unlock(mem_alloc_mutex);
				return -EINVAL;
			}

			vma->vm_flags |= VM_RESERVED;

			if ( remap_pfn_range(vma, vma->vm_start, page_frame_no, size, vma->vm_page_prot) ) 
			{
				printk(KERN_ERR "%s: mmap_error\n", __FUNCTION__);
				mutex_unlock(mem_alloc_mutex);
				return -EAGAIN;
			}
			break;
*/
			return -EINVAL;
	} // switch (flag)

	mutex_unlock(mem_alloc_mutex);

	return 0;
}

int s3c_pp_release ( struct inode *inode, struct file *file ) 
{
	s3c_pp_instance_context_t *current_instance;

	mutex_lock(h_mutex);

	dprintk ( "%s: Enterance\n", __FUNCTION__ );

	current_instance = (s3c_pp_instance_context_t *) file->private_data;
	if (NULL == current_instance) 
	{
		mutex_unlock(h_mutex);
		return -1;
	}

	if ( PP_INSTANCE_INUSE_DMA_ONESHOT == s3c_pp_instance_info.instance_state[current_instance->instance_no] )
	{
		s3c_pp_instance_info.dma_mode_instance_count--;
	}
	else if ( PP_INSTANCE_INUSE_FIFO_FREERUN == s3c_pp_instance_info.instance_state[current_instance->instance_no] )
	{

		s3c_pp_instance_info.fifo_mode_instance_no = -1;

		// change Display Controller WIN0 mode to DMA Mode
		// change Post Processor mode to DMA Mode & turn off Free Run Mode

		//.[ i: sichoi 090112 (FIFO_FREERUN mode relase without using interrupt)
#if 1
		do 
		{
			unsigned int temp;

			temp = __raw_readl ( S3C_VIDCON0 ) & ~S3C_VIDCON0_ENVID_ENABLE;
			__raw_writel ( temp, S3C_VIDCON0 );

			while ( __raw_readl ( S3C_VIDCON1 ) & 0x07FF0000 )
			{
			}

			temp = __raw_readl(s3c_pp_base + S3C_VPP_MODE) & ~S3C_MODE_AUTOLOAD_ENABLE; // set Per Frame mode
			__raw_writel(temp, s3c_pp_base + S3C_VPP_MODE);

			temp = s3c_pp_instance_info.wincon0_value_before_fifo_mode & ~S3C_WINCONx_ENWIN_F_ENABLE;
			__raw_writel ( temp, S3C_WINCON0 );

			temp = __raw_readl ( S3C_VIDCON0 ) | S3C_VIDCON0_ENVID_ENABLE | S3C_VIDCON0_ENVID_F_ENABLE;
			__raw_writel ( temp, S3C_VIDCON0 );
		} while ( 0 );
#else
		//.[ i: sichoi 081027 (Safety FIFO Mode Down)
		FIFO_mode_down_sequence = 1;

		while ( FIFO_mode_down_sequence < 4 )
		{ 
			post_int_enable(1);
			interruptible_sleep_on_timeout(&waitq, 20);

			FIFO_mode_down_sequence++;
		}
		//.] sichoi 081027

		FIFO_mode_down_sequence = 0;
#endif
		//.] sichoi 090112 (FIFO_FREERUN mode relase without using interrupt)
	}

	
	if ( current_instance->instance_no == s3c_pp_instance_info.last_running_instance_no ) 
	{ 
		s3c_pp_instance_info.last_running_instance_no = -1;
	}

	s3c_pp_instance_info.instance_state[current_instance->instance_no] = PP_INSTANCE_FREE;
	if (   0 < s3c_pp_instance_info.in_use_instance_count
	    && s3c_pp_instance_info.in_use_instance_count <= PP_MAX_NO_OF_INSTANCES )
	{
		s3c_pp_instance_info.in_use_instance_count--;
		if ( 0 == s3c_pp_instance_info.in_use_instance_count )
		{
			s3c_pp_instance_info.last_running_instance_no = -1;

			// TO DO: PP H/W Deinitialize (optional)
		}
	}

	if(file->f_flags & O_NONBLOCK)
		s3c_pp_instance_info.num_of_nonblocking_mode--;

#ifdef USE_PP_DOMAIN_GATING	
	if(   s3c_pp_instance_info.flag_clk_enable == 1
	   && s3c_pp_instance_info.num_of_nonblocking_mode <= 0)
	{
		clk_pp_disable();
		s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_LP_MODE, S3C64XX_POST);
		s3c_pp_instance_info.flag_clk_enable = 0;
	}
#endif /* USE_PP_DOMAIN_GATING */

	dprintk ( "%s: handle=%d, count=%d\n", __FUNCTION__, current_instance->instance_no, s3c_pp_instance_info.in_use_instance_count );

	kfree(current_instance);

	mutex_unlock(h_mutex);

	return 0;
}


static int s3c_pp_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	s3c_pp_instance_context_t *current_instance;
	s3c_pp_params_t *parg;

	unsigned int temp = 0;

	mutex_lock(h_mutex);

	current_instance	= (s3c_pp_instance_context_t *) file->private_data;
	parg	            = (s3c_pp_params_t *) arg;

	switch ( cmd )
	{
		case S3C_PP_SET_PARAMS:
			{
				s3c_pp_out_path_t temp_out_path; 
				unsigned int temp_src_width, temp_src_height, temp_dst_width, temp_dst_height;
				s3c_color_space_t temp_src_color_space, temp_dst_color_space;

				temp_out_path = parg->out_path;

				if (   (-1 != s3c_pp_instance_info.fifo_mode_instance_no )
				    || (   (s3c_pp_instance_info.dma_mode_instance_count)
				        && (FIFO_FREERUN == temp_out_path)) )
				{
					printk ( KERN_ERR "\n%s: S3C_PP_SET_PARAMS can't be executed.\n", __FUNCTION__ );
					mutex_unlock(h_mutex);
					return -EINVAL; 
				}

				temp_src_width =  parg->src_width;
				temp_src_height = parg->src_height;
				temp_dst_width  = parg->dst_width;
				temp_dst_height = parg->dst_height;

				// S3C6410 support that the source image is up to 4096 x 4096
				//     and the destination image is up to 2048 x 2048.
				if (    (4096 < temp_src_width) || (4096 < temp_src_height) 
				     || (2048 < temp_dst_width) || (2048 < temp_dst_height) )
				{
					printk(KERN_ERR "\n%s: Size is too big to be supported.\n", __FUNCTION__);
					printk(KERN_ERR "%s: temp_src_width(%d), temp_src_height(%d)\n", __FUNCTION__, temp_src_width, temp_src_height);
					printk(KERN_ERR "%s: temp_dst_width(%d), temp_dst_height(%d)\n", __FUNCTION__, temp_dst_width, temp_dst_height);

					mutex_unlock(h_mutex);
					return -EINVAL;
				}

				temp_src_color_space = parg->src_color_space;
				temp_dst_color_space = parg->dst_color_space;

				/*
				if(   ( (temp_src_color_space == YC420 || temp_src_color_space == YC422) && (temp_src_width & 0x07) )
				   || ( (temp_src_color_space == RGB16)                                  && (temp_src_width & 0x01) )
				   || (   (temp_out_path == DMA_ONESHOT)
				       && (   ((temp_dst_color_space == YC420 || temp_dst_color_space == YC422) && (temp_dst_width & 0x07))
				           || ((temp_dst_color_space == RGB16)                                  && (temp_dst_width & 0x01)))) )
				{
					printk(KERN_ERR "\n%s: YUV420 image width must be a multiple of 8.\n", __FUNCTION__);
					printk(KERN_ERR "%s: RGB16  image width must be a multiple of 2.\n", __FUNCTION__);
					printk(KERN_ERR "%s: temp_src_width(%d), temp_dst_width(%d)\n", __FUNCTION__, temp_src_width, temp_dst_width);
					mutex_unlock(h_mutex);
					return -EINVAL;
				}
				*/

				if(   ( (temp_src_width & 0x07) && (temp_src_color_space == YC420 || temp_src_color_space == YC422))
				   || ( (temp_src_width & 0x01) && (temp_src_color_space == RGB16))
				   || (   (   ((temp_dst_width & 0x07) && (temp_dst_color_space == YC420 || temp_dst_color_space == YC422))
				           || ((temp_dst_width & 0x01) && (temp_dst_color_space == RGB16)                                 ))
					   && (temp_out_path == DMA_ONESHOT)))
				{
					printk(KERN_ERR "\n%s: YUV420 image width must be a multiple of 8.\n", __FUNCTION__);
					printk(KERN_ERR "%s: RGB16  image width must be a multiple of 2.\n", __FUNCTION__);
					printk(KERN_ERR "%s: temp_src_width(%d), temp_dst_width(%d)\n", __FUNCTION__, temp_src_width, temp_dst_width);
					mutex_unlock(h_mutex);
					return -EINVAL;
				}


				current_instance->src_full_width  = parg->src_full_width;
				current_instance->src_full_height = parg->src_full_height;
				current_instance->src_start_x     = parg->src_start_x;
				current_instance->src_start_y     = parg->src_start_y;
				current_instance->src_width       = temp_src_width;
				current_instance->src_height      = temp_src_height;
				current_instance->src_color_space = temp_src_color_space;

				current_instance->dst_full_width  = parg->dst_full_width;
				current_instance->dst_full_height = parg->dst_full_height;
				current_instance->dst_start_x     = parg->dst_start_x;
				current_instance->dst_start_y     = parg->dst_start_y;
				current_instance->dst_width       = temp_dst_width;
				current_instance->dst_height      = temp_dst_height;
				current_instance->dst_color_space = temp_dst_color_space;

				current_instance->out_path        = temp_out_path;

				if (    (current_instance->src_full_width  < (current_instance->src_width  + current_instance->src_start_x))
				     || (current_instance->src_full_height < (current_instance->src_height + current_instance->src_start_y))
				     || (current_instance->dst_full_width  < (current_instance->dst_width  + current_instance->dst_start_x))
				     || (current_instance->dst_full_height < (current_instance->dst_height + current_instance->dst_start_y)) )
				{
					printk(KERN_ERR "\n%s: Source and destination area must be in full source and destination area 	\
										in case of zoom in/out and PIP operation.\n", __FUNCTION__);
					mutex_unlock(h_mutex);
					return -EINVAL;
				}

				// source width and height calibration
				if (parameters_calibration(current_instance) < 0)
				{
					printk(KERN_ERR "\n%s: destination width or height is very small.\n", __FUNCTION__);
					mutex_unlock(h_mutex);
					return -EINVAL;
				}

				
				if ( DMA_ONESHOT == current_instance->out_path )
				{
					s3c_pp_instance_info.instance_state[current_instance->instance_no] = PP_INSTANCE_INUSE_DMA_ONESHOT;
					s3c_pp_instance_info.dma_mode_instance_count++;               
				}
				else
				{
					current_instance->scan_mode = parg->scan_mode;

					current_instance->dst_color_space = RGB30;

					s3c_pp_instance_info.instance_state[current_instance->instance_no] = PP_INSTANCE_INUSE_FIFO_FREERUN;
					s3c_pp_instance_info.fifo_mode_instance_no = current_instance->instance_no;
					s3c_pp_instance_info.wincon0_value_before_fifo_mode = __raw_readl ( S3C_WINCON0 );

					//.[ REDUCE_VCLK_SYOP_TIME
					if (current_instance->dst_height < current_instance->src_height)
					{
						int i;

						for(i = 2; (i < 8) && ((current_instance->dst_height * i) <= current_instance->src_height); i++ )
						{
						}

						current_instance->src_full_width  *= i;
						current_instance->src_full_height /= i;
						current_instance->src_height      /= i;
					}
					//.] REDUCE_VCLK_SYOP_TIME
				}

				current_instance->value_changed |= PP_VALUE_CHANGED_PARAMS;
			}
			break;

		case S3C_PP_START:
			dprintk ( "%s: S3C_PP_START last_instance=%d, curr_instance=%d\n", __FUNCTION__, 
					s3c_pp_instance_info.last_running_instance_no, current_instance->instance_no );

			if ( PP_INSTANCE_READY == s3c_pp_instance_info.instance_state[current_instance->instance_no] )
			{
				printk ( KERN_ERR "%s: S3C_PP_START must be executed after running S3C_PP_SET_PARAMS.\n", __FUNCTION__ );
				mutex_unlock(h_mutex);
				return -EINVAL;
			}

#ifdef USE_PP_DOMAIN_GATING
			if(s3c_pp_instance_info.flag_clk_enable == 0)
			{
				s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_ACTIVE_MODE, S3C64XX_POST);
				if(s3c_wait_blk_pwr_ready(S3C64XX_BLK_F)) {
					return -1;
				}
				clk_pp_enable();
				s3c_pp_instance_info.flag_clk_enable = 1;
			}
#endif /* USE_PP_DOMAIN_GATING */
			if ( current_instance->instance_no != s3c_pp_instance_info.last_running_instance_no )
			{
				__raw_writel(0x0<<31, s3c_pp_base + S3C_VPP_POSTENVID);

				temp = S3C_MODE2_ADDR_CHANGE_DISABLE | S3C_MODE2_CHANGE_AT_FRAME_END | S3C_MODE2_SOFTWARE_TRIGGER;
				__raw_writel(temp, s3c_pp_base + S3C_VPP_MODE_2);

				set_clock_src(HCLK);

				// setting the src/dst color space
				set_data_format(current_instance);

				// setting the src/dst size 
				set_scaler(current_instance);

				// setting the src/dst buffer address
				set_src_addr (current_instance);
				set_dest_addr(current_instance);

				current_instance->value_changed = PP_VALUE_CHANGED_NONE;

				s3c_pp_instance_info.last_running_instance_no = current_instance->instance_no;
				s3c_pp_instance_info.running_instance_no      = current_instance->instance_no;

				if ( PP_INSTANCE_INUSE_DMA_ONESHOT == s3c_pp_instance_info.instance_state[current_instance->instance_no] )
				{ // DMA OneShot Mode
					dprintk ( "%s: DMA_ONESHOT mode\n", __FUNCTION__ );

					post_int_enable(1);
					pp_dma_mode_set_and_start();


					if ( !(file->f_flags & O_NONBLOCK) )
					{
						if (interruptible_sleep_on_timeout(&waitq, 200) == 0) 
						{
							printk(KERN_ERR "\n%s: Waiting for interrupt is timeout\n", __FUNCTION__);
						}
					}
				}
				else
				{ // FIFO freerun Mode
					dprintk ( "%s: FIFO_freerun mode\n", __FUNCTION__ );
					s3c_pp_instance_info.fifo_mode_instance_no = current_instance->instance_no;

					post_int_enable(1);
					pp_fifo_mode_set_and_start(current_instance); 
				}
			}
			else
			{
				if ( current_instance->value_changed != PP_VALUE_CHANGED_NONE )
				{
					__raw_writel(0x0<<31, s3c_pp_base + S3C_VPP_POSTENVID);

					if ( current_instance->value_changed & PP_VALUE_CHANGED_PARAMS )
					{
						set_data_format(current_instance);
						set_scaler(current_instance);
					}

					if ( current_instance->value_changed & PP_VALUE_CHANGED_SRC_BUF_ADDR_PHY )
					{
						set_src_addr(current_instance);
					}

					if ( current_instance->value_changed & PP_VALUE_CHANGED_DST_BUF_ADDR_PHY )
					{
						set_dest_addr(current_instance);
					}

					current_instance->value_changed = PP_VALUE_CHANGED_NONE;
				}

				s3c_pp_instance_info.running_instance_no = current_instance->instance_no;

				post_int_enable(1);
				start_processing();

				if ( !(file->f_flags & O_NONBLOCK) )
				{
					if (interruptible_sleep_on_timeout(&waitq, 200) == 0) 
					{
						printk(KERN_ERR "\n%s: Waiting for interrupt is timeout\n", __FUNCTION__);
					}
				}
			}
#ifdef USE_PP_DOMAIN_GATING
			if(   s3c_pp_instance_info.fifo_mode_instance_no == -1
			   && s3c_pp_instance_info.num_of_nonblocking_mode <= 0)
			{
				clk_pp_disable();
				s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_LP_MODE, S3C64XX_POST);
				s3c_pp_instance_info.flag_clk_enable = 0;
			}
#endif /* USE_PP_DOMAIN_GATING */
			break;

		case S3C_PP_GET_SRC_BUF_SIZE:

			if ( PP_INSTANCE_READY == s3c_pp_instance_info.instance_state[current_instance->instance_no] )
			{
				dprintk ( "%s: S3C_PP_GET_SRC_BUF_SIZE must be executed after running S3C_PP_SET_PARAMS.\n", __FUNCTION__ );
				mutex_unlock(h_mutex);
				return -EINVAL;
			}

			temp = cal_data_size ( current_instance->src_color_space, current_instance->src_full_width, current_instance->src_full_height );

			mutex_unlock(h_mutex);
			return temp;

		case S3C_PP_SET_SRC_BUF_ADDR_PHY:

			current_instance->src_buf_addr_phy_rgb_y = parg->src_buf_addr_phy_rgb_y;
			current_instance->src_buf_addr_phy_cb    = parg->src_buf_addr_phy_cb;
			current_instance->src_buf_addr_phy_cr    = parg->src_buf_addr_phy_cr;

			current_instance->value_changed |= PP_VALUE_CHANGED_SRC_BUF_ADDR_PHY;
			break;
			
		case S3C_PP_SET_SRC_BUF_NEXT_ADDR_PHY:

			if ( current_instance->instance_no != s3c_pp_instance_info.fifo_mode_instance_no )
			{ // if FIFO Mode is not Active
				dprintk (KERN_DEBUG "%s: S3C_PP_SET_SRC_BUF_NEXT_ADDR_PHY can't be executed.\n", __FUNCTION__ );
				mutex_unlock(h_mutex);
				return -EINVAL;
			}            

			current_instance->src_next_buf_addr_phy_rgb_y = parg->src_next_buf_addr_phy_rgb_y;
			current_instance->src_next_buf_addr_phy_cb    = parg->src_next_buf_addr_phy_cb;
			current_instance->src_next_buf_addr_phy_cr    = parg->src_next_buf_addr_phy_cr;

#ifdef USE_PP_DOMAIN_GATING	
			if(s3c_pp_instance_info.flag_clk_enable == 0)
			{
				s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_ACTIVE_MODE, S3C64XX_POST);
				if(s3c_wait_blk_pwr_ready(S3C64XX_BLK_F)) {
					return -1;
				}
				clk_pp_enable();
				s3c_pp_instance_info.flag_clk_enable = 1;
			}
#endif /* USE_PP_DOMAIN_GATING */
			
			temp = __raw_readl(s3c_pp_base + S3C_VPP_MODE_2);
			temp |= (0x1<<4);
			__raw_writel(temp, s3c_pp_base + S3C_VPP_MODE_2);

			set_src_next_buf_addr(current_instance);

			temp = __raw_readl(s3c_pp_base + S3C_VPP_MODE_2);
			temp &= ~(0x1<<4);
			__raw_writel(temp, s3c_pp_base + S3C_VPP_MODE_2);

#ifdef USE_PP_DOMAIN_GATING
			if(   s3c_pp_instance_info.fifo_mode_instance_no == -1
			   && s3c_pp_instance_info.num_of_nonblocking_mode <= 0)
			{
				clk_pp_disable();
				s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_LP_MODE, S3C64XX_POST);
				s3c_pp_instance_info.flag_clk_enable = 0;
			}
#endif /* USE_PP_DOMAIN_GATING */
			break;

		case S3C_PP_GET_DST_BUF_SIZE:

			if ( PP_INSTANCE_READY == s3c_pp_instance_info.instance_state[current_instance->instance_no] )
			{
				dprintk ( "%s: S3C_PP_GET_DST_BUF_SIZE must be executed after running S3C_PP_SET_PARAMS.\n", __FUNCTION__ );
				mutex_unlock(h_mutex);
				return -EINVAL;
			}

			temp = cal_data_size ( current_instance->dst_color_space, current_instance->dst_full_width, current_instance->dst_full_height );

			mutex_unlock(h_mutex);
			return temp;

		case S3C_PP_SET_DST_BUF_ADDR_PHY:

			current_instance->dst_buf_addr_phy_rgb_y = parg->dst_buf_addr_phy_rgb_y;
			current_instance->dst_buf_addr_phy_cb    = parg->dst_buf_addr_phy_cb;
			current_instance->dst_buf_addr_phy_cr    = parg->dst_buf_addr_phy_cr;

			current_instance->value_changed |= PP_VALUE_CHANGED_DST_BUF_ADDR_PHY;
			break;
			
		case S3C_PP_ALLOC_KMEM:
			{
				s3c_pp_mem_alloc_t param;

				if (copy_from_user(&param, (s3c_pp_mem_alloc_t *)arg, sizeof(s3c_pp_mem_alloc_t)))
				{
					mutex_unlock(h_mutex);
					return -EFAULT;
				}

				flag = ALLOC_KMEM;

				param.vir_addr = do_mmap(file, 0, param.size, PROT_READ|PROT_WRITE, MAP_SHARED, 0);
				dprintk (KERN_DEBUG "param.vir_addr = %08x\n", param.vir_addr);

				flag = 0;

				if(param.vir_addr == -EINVAL) {
					printk(KERN_ERR "%s: PP_MEM_ALLOC FAILED\n", __FUNCTION__);
					mutex_unlock(h_mutex);
					return -EFAULT;
				}
				param.phy_addr = physical_address;

				dprintk (KERN_DEBUG "KERNEL MALLOC : param.phy_addr = 0x%X \t size = %d \t param.vir_addr = 0x%X\n", param.phy_addr, param.size, param.vir_addr);

				if (copy_to_user((s3c_pp_mem_alloc_t *)arg, &param, sizeof(s3c_pp_mem_alloc_t)))
				{
					mutex_unlock(h_mutex);
					return -EFAULT;
				}
			}
			break;

		case S3C_PP_FREE_KMEM:
			{
				s3c_pp_mem_alloc_t param;
				struct mm_struct *mm = current->mm;
				void *virt_addr;

				if ( copy_from_user(&param, (s3c_pp_mem_alloc_t *)arg, sizeof(s3c_pp_mem_alloc_t)) )
				{
					mutex_unlock(h_mutex);
					return -EFAULT;
				}

				dprintk (KERN_DEBUG "KERNEL FREE : param.phy_addr = 0x%X \t size = %d \t param.vir_addr = 0x%X\n", param.phy_addr, param.size, param.vir_addr);

				if ( do_munmap(mm, param.vir_addr, param.size ) < 0 ) 
				{
					dprintk("do_munmap() failed !!\n");
					mutex_unlock(h_mutex);
					return -EINVAL;
				}
				virt_addr = phys_to_virt(param.phy_addr);
				dprintk ( "KERNEL : virt_addr = 0x%X\n", (unsigned int) virt_addr );

				kfree(virt_addr);
				param.size = 0;

				dprintk(KERN_DEBUG "do_munmap() succeed !!\n");
			}
			break;

/*
		case S3C_PP_GET_RESERVED_MEM_SIZE:
			mutex_unlock(h_mutex);
			return PP_RESERVED_MEM_SIZE;

		case S3C_PP_GET_RESERVED_MEM_ADDR_PHY:
			mutex_unlock(h_mutex);
			return PP_RESERVED_MEM_ADDR_PHY;
*/
		default:
			mutex_unlock(h_mutex);
			return -EINVAL;
	}

	mutex_unlock(h_mutex);

	return 0;
}

static unsigned int s3c_pp_poll(struct file *file, poll_table *wait)
{
	unsigned int	mask = 0;
	s3c_pp_instance_context_t *current_instance;

	mutex_lock(h_mutex);

	current_instance = (s3c_pp_instance_context_t *) file->private_data;

	poll_wait(file, &waitq, wait);

	if ( -1 == s3c_pp_instance_info.fifo_mode_instance_no ) 
	{
		if ( -1 == s3c_pp_instance_info.running_instance_no )
		{
			dprintk ( "nw(%d) ", current_instance->instance_no );

			mask = POLLOUT|POLLWRNORM;
		}
		else
		{
			dprintk ( "w(%d) ", current_instance->instance_no );
		}
	}
	else
	{
		dprintk ( "%s : error\n", __FUNCTION__ );
		mask = POLLERR;
	}

	mutex_unlock(h_mutex);

	return mask;
}


static struct file_operations s3c_pp_fops = {
	.owner 	    = THIS_MODULE,
	.ioctl 	    = s3c_pp_ioctl,
	.open 	    = s3c_pp_open,
	.read 	    = s3c_pp_read,
	.write 	    = s3c_pp_write,
	.mmap	    = s3c_pp_mmap,
	.poll	    = s3c_pp_poll,
	.release 	= s3c_pp_release
};


static struct miscdevice s3c_pp_dev = {
	.minor		= PP_MINOR,
	.name		= "s3c-pp",
	.fops		= &s3c_pp_fops,
};

static int s3c_pp_remove(struct platform_device *dev)
{
#ifdef USE_PP_DOMAIN_GATING
	clk_pp_disable();
	s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_LP_MODE, S3C64XX_POST);
#endif /* USE_PP_DOMAIN_GATING */

	free_irq(s3c_pp_irq, NULL);
	if (s3c_pp_mem != NULL) {
		pr_debug("s3c_post: releasing s3c_post_mem\n");
		iounmap(s3c_pp_base);
		release_resource(s3c_pp_mem);
		kfree(s3c_pp_mem);
	}
	misc_deregister(&s3c_pp_dev);
	return 0;
}



int s3c_pp_probe(struct platform_device *pdev)
{
	struct resource *res;

	int ret;

	int i;
#ifdef USE_PP_DOMAIN_GATING
	s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_ACTIVE_MODE, S3C64XX_POST);
	if(s3c_wait_blk_pwr_ready(S3C64XX_BLK_F)) {
	return -1;
	}
#endif /* USE_PP_DOMAIN_GATING */
	/* find the IRQs */
	s3c_pp_irq = platform_get_irq(pdev, 0);

	if(s3c_pp_irq <= 0) {
		printk(KERN_ERR PFX "failed to get irq resouce\n");
		return -ENOENT;
	}

	/* get the memory region for the post processor driver */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(res == NULL) {
		printk(KERN_ERR PFX "failed to get memory region resouce\n");
		return -ENOENT;
	}

	s3c_pp_mem = request_mem_region(res->start, res->end-res->start+1, pdev->name);
	if(s3c_pp_mem == NULL) {
		printk(KERN_ERR PFX "failed to reserve memory region\n");
		return -ENOENT;
	}

	s3c_pp_base = ioremap(s3c_pp_mem->start, s3c_pp_mem->end - res->start + 1);
	if(s3c_pp_base == NULL) {
		printk(KERN_ERR PFX "failed ioremap\n");
		return -ENOENT;
	}

	pp_clock = clk_get(&pdev->dev, "post0");

	if (IS_ERR(pp_clock)) {
		printk(KERN_ERR PFX "failed to find post clock source\n");
		return -ENOENT;
	}

#ifndef USE_PP_DOMAIN_GATING
	clk_pp_enable();
#endif /* USE_PP_DOMAIN_GATING */

	init_waitqueue_head(&waitq);

	ret = misc_register(&s3c_pp_dev);
	if (ret) {
		printk (KERN_ERR "cannot register miscdev on minor=%d (%d)\n",
				PP_MINOR, ret);
		return ret;
	}

	ret = request_irq(s3c_pp_irq, (irq_handler_t) s3c_pp_isr, IRQF_DISABLED,
			pdev->name, NULL);
	if (ret) {
		printk(KERN_ERR "request_irq(PP) failed.\n");
		return ret;
	}

	h_mutex = (struct mutex *) kmalloc(sizeof(struct mutex), GFP_DMA|GFP_ATOMIC );
	if (h_mutex == NULL)
		return -1;

	mutex_init(h_mutex);

	mem_alloc_mutex = (struct mutex *) kmalloc(sizeof(struct mutex), GFP_DMA|GFP_ATOMIC );
	if (mem_alloc_mutex == NULL)
		return -1;

	mutex_init(mem_alloc_mutex);

	// initialzie instance infomation 
	s3c_pp_instance_info.running_instance_no      = -1;
	s3c_pp_instance_info.last_running_instance_no = -1;
	s3c_pp_instance_info.in_use_instance_count    = 0;
	s3c_pp_instance_info.dma_mode_instance_count  = 0;
	s3c_pp_instance_info.fifo_mode_instance_no    = -1;
	s3c_pp_instance_info.flag_clk_enable          = 0;
	s3c_pp_instance_info.num_of_nonblocking_mode  = 0;

	for ( i=0; i < PP_MAX_NO_OF_INSTANCES; i++ )
		s3c_pp_instance_info.instance_state[i] = PP_INSTANCE_FREE;

#ifdef USE_PP_DOMAIN_GATING
	clk_pp_disable();
	s3c_set_normal_cfg(S3C64XX_DOMAIN_F, S3C64XX_LP_MODE, S3C64XX_POST);
#endif /* USE_PP_DOMAIN_GATING */
	/* check to see if everything is setup correctly */	
	return 0;
}

static int s3c_pp_suspend(struct platform_device *dev, pm_message_t state)
{
	return 0;
}

static int s3c_pp_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver s3c_pp_driver = {
	.probe          = s3c_pp_probe,
	.remove         = s3c_pp_remove,
	.suspend        = s3c_pp_suspend,
	.resume         = s3c_pp_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c-vpp",
	},
};

static char version[] __initdata = "3.12";
static char banner[] __initdata = KERN_INFO "S3C PostProcessor Driver v%s, (c) 2009 Samsung Electronics\n";

int __init  s3c_pp_init(void)
{

	printk(banner, version);
	if(platform_driver_register(&s3c_pp_driver)!=0)
	{
		printk(KERN_ERR "platform device register Failed \n");
		return -1;
	}

	return 0;
}

void  s3c_pp_exit(void)
{
	platform_driver_unregister(&s3c_pp_driver);
	mutex_destroy(h_mutex);
	mutex_destroy(mem_alloc_mutex);
	printk("S3C PostProcessor module exit\n");
}

module_init(s3c_pp_init);
module_exit(s3c_pp_exit);


MODULE_AUTHOR("jiun.yu@samsung.com");
MODULE_DESCRIPTION("S3C PostProcessor Device Driver");
MODULE_LICENSE("GPL");


