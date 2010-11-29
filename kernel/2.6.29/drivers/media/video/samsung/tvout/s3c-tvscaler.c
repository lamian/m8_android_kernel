
/*
 * linux/drivers/tvenc/s3c-tvscaler.c
 *
 * Revision 1.0  
 *
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 *	    S3C TV Scaler driver 
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
#include <asm/uaccess.h>
#include <linux/miscdevice.h>

#include <linux/version.h>
#include <mach/hardware.h>
#include <mach/map.h>
#include <plat/regs-tvscaler.h>
#include <plat/regs-clock.h>

#include "s3c-tvscaler.h"

#define PFX "s3c_tv_scaler"

#define SINGLE_BUF	1		// Single buffer mode


static struct clk *h_clk;
static struct clk *tvscaler_clock;
static void __iomem *base;
static int s3c_tvscaler_irq = NO_IRQ;
static struct resource *s3c_tvscaler_mem;


//static unsigned char *addr_start_y;
//static unsigned char *addr_start_rgb;

static wait_queue_head_t waitq;

irqreturn_t s3c_tvscaler_isr(int irq, void *dev_id)
{
	u32 mode;
	mode = __raw_readl(base + S3C_MODE);
	mode &= ~(1 << 6);			/* Clear Source in POST Processor */
	__raw_writel(mode, base + S3C_MODE);

//	wake_up_interruptible(&waitq);
	return IRQ_HANDLED;
}

#if 0
static buff_addr_t buf_addr = { NULL };


static u32 post_alloc_pre_buff(scaler_params_t *sp)
{
	u32 size;

#ifdef USE_DEDICATED_MEM
	
	buf_addr.pre_phy_addr = PHYS_OFFSET + (SYSTEM_RAM - RESERVE_POST_MEM);
	buf_addr.pre_virt_addr = ioremap_nocache(buf_addr.pre_phy_addr, PRE_BUFF_SIZE);
	if( !buf_addr.pre_virt_addr ) {
		printk(KERN_ERR "%s: Failed to allocate pre buffer \n",__FUNCTION__);
		return -ENOMEM;
	}

	sp->SrcFrmSt = buf_addr.pre_phy_addr;
#else
	size = sp->SrcWidth * sp->SrcHeight * 2;
	addr_start_y = kmalloc(size, GFP_DMA);
	if(addr_start_y != NULL) return -ENOMEM;
#endif
	return 0;
}

static u32 post_alloc_post_buff(scaler_params_t *sp)
{
	u32 size;
	
#ifdef USE_DEDICATED_MEM
	
	buf_addr.post_phy_addr = PHYS_OFFSET + (SYSTEM_RAM - RESERVE_POST_MEM + PRE_BUFF_SIZE);
	buf_addr.post_virt_addr = ioremap_nocache(buf_addr.post_phy_addr, POST_BUFF_SIZE);
	if( !buf_addr.post_virt_addr ) {
		printk(KERN_ERR "%s: Failed to allocate post buffer \n",__FUNCTION__);
		return -ENOMEM;
	}

	sp->DstFrmSt = buf_addr.post_phy_addr;
#else
	size = sp->DstWidth * sp->DstHeight * 2;
	addr_start_rgb = kmalloc(size, GFP_DMA);
	if(addr_start_rgb != NULL) return -ENOMEM;
#endif
	return 0;
}

static u32 post_free_all_buffer(void)
{
#ifdef USE_DEDICATED_MEM	
	if( buf_addr.pre_virt_addr ) {
		iounmap(buf_addr.pre_virt_addr);
	}
	if( buf_addr.post_virt_addr ) {
		iounmap(buf_addr.post_virt_addr);
	}
#endif	
	return 0;
}
#endif

static void s3c_tvscaler_set_clk_src(scaler_clk_src_t clk_src)
{
	u32 tmp, rate;

	tmp = __raw_readl(base + S3C_MODE);

	h_clk = clk_get(NULL, "hclk");

	rate = clk_get_rate(h_clk);
	
	if(clk_src == HCLK) {
		if(rate > 66000000) {
			tmp &= ~(0x7f<<23);
			tmp |= (1<<24);
			tmp |= (1<<23);
		} else {
			tmp &=~ (0x7f<<23);
		}
		
	} else if(clk_src == PLL_EXT) {
	} else {
		tmp &=~(0x7f<<23);
	}

	tmp = (tmp &~ (0x3<<21)) | (clk_src<<21);

	__raw_writel(tmp, base + S3C_MODE);
}

static void s3c_tvscaler_set_fmt(cspace_t src, cspace_t dst, s3c_scaler_path_t in,
				s3c_scaler_path_t out, u32 *in_pixel_size,
				u32 *out_pixel_size)
{
	u32 tmp;

	tmp = __raw_readl(base + S3C_MODE);
	tmp |= (0x1<<16);
	tmp |= (0x2<<10);

	if(in == POST_DMA) {

		switch(src) {
		case YC420:
			tmp &=~((0x1<<3)|(0x1<<2));
			tmp |= (0x1<<8)|(0x1<<1);
			*in_pixel_size = 1;
			break;
		case CRYCBY:
			tmp &= ~((0x1<<15)|(0x1<<8)|(0x1<<3)|(0x1<<0));
			tmp |= (0x1<<2)|(0x1<<1);
			*in_pixel_size = 2;		
			break;
		case CBYCRY:
			tmp &= ~((0x1<<8)|(0x1<<3)|(0x1<<0));
			tmp |= (0x1<<15)|(0x1<<2)|(0x1<<1);
			*in_pixel_size = 2;	
			break;
		case YCRYCB:
			tmp &= ~((0x1<<15)|(0x1<<8)|(0x1<<3));
			tmp |= (0x1<<2)|(0x1<<1)|(0x1<<0);
			*in_pixel_size = 2;			
			break;
		case YCBYCR:
			tmp &= ~((0x1<<8)|(0x1<<3));
			tmp |= (0x1<<15)|(0x1<<2)|(0x1<<1)|(0x1<<0);	
			*in_pixel_size = 2;			
			break;
		case RGB24:
			tmp &= ~(0x1<<8);
			tmp |=  (0x1<<3)|(0x1<<2)|(0x1<<1);
			*in_pixel_size = 4;
			break;
		case RGB16:
			tmp &= ~((0x1<<8)|(0x1<<1));
			tmp |=  (0x1<<3)|(0x1<<2);
			*in_pixel_size = 2;			
			break;
		default:
			break;
		}

	} 
	else if(in == POST_FIFO) {
	}

	if(out == POST_DMA) {
		switch(dst) {
		case YC420:
			tmp &= ~(0x1<<18);
			tmp |= (0x1<<17);
			*out_pixel_size = 1;			
			break;
		case CRYCBY:
			tmp &= ~((0x1<<20)|(0x1<<19)|(0x1<<18)|(0x1<<17));
			*out_pixel_size = 2;
			break;
		case CBYCRY:
			tmp &= ~((0x1<<19)|(0x1<<18)|(0x1<<17));
			tmp |= (0x1<<20);
			*out_pixel_size = 2;			
			break;
		case YCRYCB:
			tmp &= ~((0x1<<20)|(0x1<<18)|(0x1<<17));
			tmp |= (0x1<<19);
			*out_pixel_size = 2;			
			break;
		case YCBYCR:
			tmp &= ~((0x1<<18)|(0x1<<17));
			tmp |= (0x1<<20)|(0x1<<19);	
			*out_pixel_size = 2;			
			break;
		case RGB24:
			tmp |= (0x1<<18)|(0x1<<4);
			*out_pixel_size = 4;			
			break;
		case RGB16:
			tmp &= ~(0x1<<4);
			tmp |= (0x1<<18);
			*out_pixel_size = 2;			
			break;
		default:
			break;
		}
	}
	else if(out == POST_FIFO) {
		if(dst == RGB24) {
			tmp |= (0x1<<18)|(0x1<<13); 
			
		} else if(dst == YCBYCR) {
			tmp |= (0x1<<13);
			tmp &= ~(0x1<<18)|(0x1<<17);
		} else {
		}
	}

	__raw_writel(tmp, base + S3C_MODE);
}

static void s3c_tvscaler_set_path(s3c_scaler_path_t in, s3c_scaler_path_t out)
{
	u32 tmp;

	tmp = __raw_readl(base + S3C_MODE);	
	
	tmp &=~(0x1<<12);	// 0: progressive mode, 1: interlace mode

	if(in == POST_FIFO) {
		tmp |= (0x1<<31);
	} else if(in == POST_DMA) {
		tmp &=~(0x1<<31);
	}

	if(out == POST_FIFO) {
		tmp |= (0x1<<13);
	} else if(out == POST_DMA) {
		tmp &=~(0x1<<13);
	}

	__raw_writel(tmp, base + S3C_MODE);
}

static void s3c_tvscaler_set_addr(scaler_params_t *sp, u32 in_pixel_size, u32 out_pixel_size)
{
	u32 offset_y, offset_cb, offset_cr;
	u32 src_start_y, src_start_cb, src_start_cr;
	u32 src_end_y, src_end_cb, src_end_cr;
	u32 start_pos_y, end_pos_y;
	u32 start_pos_cb, end_pos_cb;
	u32 start_pos_cr, end_pos_cr;
	u32 start_pos_rgb, end_pos_rgb;
	u32 dst_start_rgb, dst_end_rgb;
	u32 src_frm_start_addr;
	
	u32 offset_rgb, out_offset_cb, out_offset_cr;
	u32 out_start_pos_cb, out_start_pos_cr;
	u32 out_end_pos_cb, out_end_pos_cr;
	u32 out_src_start_cb, out_src_start_cr;
	u32 out_src_end_cb, out_src_end_cr;

	if(sp->InPath == POST_DMA) {
		offset_y = (sp->SrcFullWidth - sp->SrcWidth) * in_pixel_size;
		start_pos_y = (sp->SrcFullWidth*sp->SrcStartY+sp->SrcStartX)*in_pixel_size;
		end_pos_y = sp->SrcWidth*sp->SrcHeight*in_pixel_size + offset_y*(sp->SrcHeight-1);
		src_frm_start_addr = sp->SrcFrmSt;
		src_start_y = sp->SrcFrmSt + start_pos_y;
		src_end_y = src_start_y + end_pos_y;

		__raw_writel(src_start_y, base + S3C_ADDRSTART_Y);
		__raw_writel(offset_y, base + S3C_OFFSET_Y);
		__raw_writel(src_end_y, base + S3C_ADDREND_Y);	

		if(sp->SrcCSpace == YC420) {
			offset_cb = offset_cr = ((sp->SrcFullWidth - sp->SrcWidth) / 2) * in_pixel_size;
			start_pos_cb = sp->SrcFullWidth * sp->SrcFullHeight * 1 \
					+ (sp->SrcFullWidth * sp->SrcStartY / 2 + sp->SrcStartX) /2 * 1;
					
			end_pos_cb = sp->SrcWidth/2*sp->SrcHeight/2*in_pixel_size \
					+ (sp->SrcHeight/2 -1)*offset_cb;
			start_pos_cr = sp->SrcFullWidth * sp->SrcFullHeight *1 \
					+ sp->SrcFullWidth*sp->SrcFullHeight/4 *1 \
					+ (sp->SrcFullWidth*sp->SrcStartY/2 + sp->SrcStartX)/2*1;
			end_pos_cr = sp->SrcWidth/2*sp->SrcHeight/2*in_pixel_size \
					+ (sp->SrcHeight/2-1)*offset_cr;

			src_start_cb = sp->SrcFrmSt + start_pos_cb;
			src_end_cb = src_start_cb + end_pos_cb;

			src_start_cr = sp->SrcFrmSt + start_pos_cr;
			src_end_cr = src_start_cr + end_pos_cr;

			__raw_writel(src_start_cb, base + S3C_ADDRSTART_CB);
			__raw_writel(offset_cr, base + S3C_OFFSET_CB);
			__raw_writel(src_end_cb, base + S3C_ADDREND_CB);
			__raw_writel(src_start_cr, base + S3C_ADDRSTART_CR);
			__raw_writel(offset_cb, base + S3C_OFFSET_CR);
			__raw_writel(src_end_cr, base + S3C_ADDREND_CR);		
		}
	}
	if(sp->OutPath == POST_DMA) {
		offset_rgb = (sp->DstFullWidth - sp->DstWidth)*out_pixel_size;
		start_pos_rgb = (sp->DstFullWidth*sp->DstStartY + sp->DstStartX)*out_pixel_size;
		end_pos_rgb = sp->DstWidth*sp->DstHeight*out_pixel_size + offset_rgb*(sp->DstHeight - 1);
		dst_start_rgb = sp->DstFrmSt + start_pos_rgb;
		dst_end_rgb = dst_start_rgb + end_pos_rgb;

		__raw_writel(dst_start_rgb, base + S3C_ADDRSTART_RGB);
		__raw_writel(offset_rgb, base + S3C_OFFSET_RGB);
		__raw_writel(dst_end_rgb, base + S3C_ADDREND_RGB);

		if(sp->DstCSpace == YC420) {
			out_offset_cb = out_offset_cr = ((sp->DstFullWidth - sp->DstWidth)/2)*out_pixel_size;
			out_start_pos_cb = sp->DstFullWidth*sp->DstFullHeight*1 \
					+ (sp->DstFullWidth*sp->DstStartY/2 + sp->DstStartX)/2*1;
			out_end_pos_cb = sp->DstWidth/2*sp->DstHeight/2*out_pixel_size \
					+ (sp->DstHeight/2 -1)*out_offset_cr;

			out_start_pos_cr = sp->DstFullWidth*sp->DstFullHeight*1 \
					+ (sp->DstFullWidth*sp->DstFullHeight/4)*1 \
					+ (sp->DstFullWidth*sp->DstStartY/2 +sp->DstStartX)/2*1;
			out_end_pos_cr = sp->DstWidth/2*sp->DstHeight/2*out_pixel_size \
					+ (sp->DstHeight/2 -1)*out_offset_cb;

			out_src_start_cb = sp->DstFrmSt + out_start_pos_cb;
			out_src_end_cb = out_src_start_cb + out_end_pos_cb;
			out_src_start_cr = sp->DstFrmSt + out_start_pos_cr;
			out_src_end_cr = out_src_start_cr + out_end_pos_cr;

			__raw_writel(out_src_start_cb, base + S3C_ADDRSTART_OCB);
			__raw_writel(out_offset_cb, base + S3C_OFFSET_OCB);
			__raw_writel(out_src_end_cb, base + S3C_ADDREND_OCB);
			__raw_writel(out_src_start_cr, base + S3C_ADDRSTART_OCR);
			__raw_writel(out_offset_cr, base + S3C_OFFSET_OCR);
			__raw_writel(out_src_end_cr, base + S3C_ADDREND_OCR);
			
		}
	}

	
}

#if 0
static void s3c_tvscaler_set_fifo_in(s3c_scaler_path_t in_path)
{
	u32 tmp;

	tmp = __raw_readl(base + S3C_MODE);

	if(in_path == POST_FIFO) tmp |= (0x1<<31);
	else tmp &=~(0x1<<31);
	
	__raw_writel(tmp, base + S3C_MODE);
	
}
#endif

void s3c_tvscaler_set_interlace(u32 on_off)
{
	u32 tmp;

	tmp = __raw_readl(base + S3C_MODE);

	if(on_off == 1) tmp |=(1<<12);
	else tmp &=~(1<<12);

	__raw_writel(tmp, base + S3C_MODE);
}
EXPORT_SYMBOL(s3c_tvscaler_set_interlace);

static void s3c_tvscaler_set_size(scaler_params_t *sp)
{
	u32 pre_h_ratio, pre_v_ratio, h_shift, v_shift, sh_factor;
	u32 pre_dst_width, pre_dst_height, dx, dy;

	if (sp->SrcWidth >= (sp->DstWidth<<6)) {
		printk("Out of PreScalar range !!!\n");
		return;
	}
	if(sp->SrcWidth >= (sp->DstWidth<<5)) {
		pre_h_ratio = 32;
		h_shift = 5;		
	} else if(sp->SrcWidth >= (sp->DstWidth<<4)) {
		pre_h_ratio = 16;
		h_shift = 4;		
	} else if(sp->SrcWidth >= (sp->DstWidth<<3)) {
		pre_h_ratio = 8;
		h_shift = 3;		
	} else if(sp->SrcWidth >= (sp->DstWidth<<2)) {
		pre_h_ratio = 4;
		h_shift = 2;		
	} else if(sp->SrcWidth >= (sp->DstWidth<<1)) {
		pre_h_ratio = 2;
		h_shift = 1;		
	} else {
		pre_h_ratio = 1;
		h_shift = 0;		
	}

	pre_dst_width = sp->SrcWidth / pre_h_ratio;
	dx = (sp->SrcWidth<<8) / (sp->DstWidth<<h_shift);


	if (sp->SrcHeight >= (sp->DstHeight<<6)) {
		printk("Out of PreScalar range !!!\n");
		return;
	}
	if(sp->SrcHeight>= (sp->DstHeight<<5)) {
		pre_v_ratio = 32;
		v_shift = 5;		
	} else if(sp->SrcHeight >= (sp->DstHeight<<4)) {
		pre_v_ratio = 16;
		v_shift = 4;		
	} else if(sp->SrcHeight >= (sp->DstHeight<<3)) {
		pre_v_ratio = 8;
		v_shift = 3;		
	} else if(sp->SrcHeight >= (sp->DstHeight<<2)) {
		pre_v_ratio = 4;
		v_shift = 2;		
	} else if(sp->SrcHeight >= (sp->DstHeight<<1)) {
		pre_v_ratio = 2;
		v_shift = 1;		
	} else {
		pre_v_ratio = 1;
		v_shift = 0;		
	}	

	pre_dst_height = sp->SrcHeight / pre_v_ratio;
	dy = (sp->SrcHeight<<8) / (sp->DstHeight<<v_shift);
	sh_factor = 10 - (h_shift + v_shift);

	__raw_writel((pre_v_ratio<<7)|(pre_h_ratio<<0), base + S3C_PRESCALE_RATIO);
	__raw_writel((pre_dst_height<<12)|(pre_dst_width<<0), base + S3C_PRESCALEIMGSIZE);
	__raw_writel(sh_factor, base + S3C_PRESCALE_SHFACTOR);
	__raw_writel(dx, base + S3C_MAINSCALE_H_RATIO);
	__raw_writel(dy, base + S3C_MAINSCALE_V_RATIO);
	__raw_writel((sp->SrcHeight<<12)|(sp->SrcWidth), base + S3C_SRCIMGSIZE);
	__raw_writel((sp->DstHeight<<12)|(sp->DstWidth), base + S3C_DSTIMGSIZE);

}


static void s3c_tvscaler_set_auto_load(scaler_params_t *sp)
{
	u32 tmp;

	tmp = __raw_readl(base + S3C_MODE);

	if(sp->Mode == FREE_RUN) {
		tmp |= (1<<14);
	} else if(sp->Mode == ONE_SHOT) {
		tmp &=~(1<<14);
	}

	__raw_writel(tmp, base + S3C_MODE);
	
}

void s3c_tvscaler_set_base_addr(void __iomem * base_addr)
{
	base = base_addr;
}
EXPORT_SYMBOL(s3c_tvscaler_set_base_addr);

void s3c_tvscaler_free_base_addr(void)
{
	base = NULL;
}
EXPORT_SYMBOL(s3c_tvscaler_free_base_addr);

void s3c_tvscaler_int_enable(u32 int_type)
{
	u32 tmp;

	tmp = __raw_readl(base + S3C_MODE);

	if(int_type == 0) {		//Edge triggering
		tmp &= ~(S3C_MODE_IRQ_LEVEL);
	} else if(int_type == 1) {	//level triggering
		tmp |= S3C_MODE_IRQ_LEVEL;
	}

	tmp |= S3C_MODE_POST_INT_ENABLE;

	__raw_writel(tmp, base + S3C_MODE);
}
EXPORT_SYMBOL(s3c_tvscaler_int_enable);

void s3c_tvscaler_int_disable(void)
{
	u32 tmp;

	tmp = __raw_readl(base + S3C_MODE);

	tmp &=~ (S3C_MODE_POST_INT_ENABLE);

	__raw_writel(tmp, base + S3C_MODE);

}
EXPORT_SYMBOL(s3c_tvscaler_int_disable);


void s3c_tvscaler_start(void)
{
	__raw_writel(S3C_POSTENVID_ENABLE, base + S3C_POSTENVID);

}
EXPORT_SYMBOL(s3c_tvscaler_start);

void s3c_tvscaler_stop_freerun(void)
{
	u32 tmp;

	tmp = __raw_readl(base + S3C_MODE);

	tmp &=~(1<<14);

	__raw_writel(tmp, base + S3C_MODE);
}
EXPORT_SYMBOL(s3c_tvscaler_stop_freerun);


void s3c_tvscaler_config(scaler_params_t *sp)
{
	u32 tmp = 0; 
	u32 in_pixel_size = 0; 
	u32 out_pixel_size = 0;
	u32 loop = 0;

	tmp = __raw_readl(base + S3C_POSTENVID);
	tmp &= ~S3C_POSTENVID_ENABLE;
	__raw_writel(tmp, base + S3C_POSTENVID);
#ifdef SINGLE_BUF
	tmp = S3C_MODE2_ADDR_CHANGE_DISABLE |S3C_MODE2_CHANGE_AT_FRAME_END |S3C_MODE2_SOFTWARE_TRIGGER;
#else
	tmp = S3C_MODE2_ADDR_CHANGE_ENABLE |S3C_MODE2_CHANGE_AT_FRAME_END |S3C_MODE2_SOFTWARE_TRIGGER;
#endif
	__raw_writel(tmp, base + S3C_MODE2);

// peter mod.	start	
	sp->DstStartX = sp->DstStartY = 0;	
	sp->DstWidth = sp->DstFullWidth;
	sp->DstHeight = sp->DstFullHeight;	
// peter mod. end		

	// 2009.04.01 Modified by hyunkyung
	//sp->DstFrmSt = ( POST_BUFF_BASE_ADDR + PRE_BUFF_SIZE );
	sp->DstFrmSt = ( POST_BUFF_BASE_ADDR);
	//printk("\n---peter s3c_tvscaler_config : SrcFrmSt = 0x%08x\n", sp->SrcFrmSt);
	//printk("---peter s3c_tvscaler_config : DstFrmSt = 0x%08x\n", sp->DstFrmSt);

	s3c_tvscaler_set_clk_src(HCLK);

	s3c_tvscaler_set_path(sp->InPath, sp->OutPath);

	s3c_tvscaler_set_fmt(sp->SrcCSpace, sp->DstCSpace, sp->InPath, 
						sp->OutPath, &in_pixel_size, &out_pixel_size);

	s3c_tvscaler_set_size(sp);

	s3c_tvscaler_set_addr(sp, in_pixel_size, out_pixel_size);

	s3c_tvscaler_set_auto_load(sp);

}
EXPORT_SYMBOL(s3c_tvscaler_config);

void s3c_tvscaler_set_param(scaler_params_t *sp)
{
#if 0	
	param.SrcFullWidth	= sp->SrcFullWidth;
	param.SrcFullHeight	= sp->SrcFullHeight;
	param.SrcStartX		= sp->SrcStartX;	
	param.SrcStartY		= sp->SrcStartY;
	param.SrcWidth		= sp->SrcWidth;
	param.SrcHeight		= sp->SrcHeight;
	param.SrcFrmSt		= sp->SrcFrmSt;
	param.SrcCSpace		= sp->SrcCSpace;
	param.DstFullWidth	= sp->DstFullWidth;
	param.DstFullHeight	= sp->DstFullHeight;
	param.DstStartX		= sp->DstStartX;
	param.DstStartY		= sp->DstStartY;
	param.DstWidth		= sp->DstWidth;
	param.DstHeight		= sp->DstHeight;
	param.DstFrmSt		= sp->DstFrmSt;
	param.DstCSpace		= sp->DstCSpace;
	param.SrcFrmBufNum	= sp->SrcFrmBufNum;
	param.DstFrmSt		= sp->DstFrmSt;
	param.Mode		= sp->Mode;	
	param.InPath		= sp->InPath;
	param.OutPath		= sp->OutPath;
#endif	
}
EXPORT_SYMBOL(s3c_tvscaler_set_param);

void s3c_tvscaler_init(void)
{

	int tmp;

	// Use DOUTmpll source clock as a scaler clock
	tmp = __raw_readl(S3C_CLK_SRC);

	tmp &=~(0x3<<28);
	tmp |= (0x1<<28);
	__raw_writel(tmp, S3C_CLK_SRC);
 
       	printk(" %s \n", __FUNCTION__);

}
EXPORT_SYMBOL(s3c_tvscaler_init);


static int s3c_tvscaler_probe(struct platform_device *pdev)
{
   
	struct resource *res;

        int ret;

	// 2008.03.27 added for tv-out debugging  by hyunkyung
	//printk("[hk] entered the s3c_tvenc_probe function\n");

	/* find the IRQs */
	s3c_tvscaler_irq = platform_get_irq(pdev, 0);
	if(s3c_tvscaler_irq <= 0) {
		printk(KERN_ERR PFX "failed to get irq resouce\n");
              return -ENOENT;
	}

        /* get the memory region for the tv scaler driver */
       res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
       if(res == NULL) {
               printk(KERN_ERR PFX "failed to get memory region resouce\n");
               return -ENOENT;
       }

	s3c_tvscaler_mem = request_mem_region(res->start, res->end-res->start+1, pdev->name);
	if(s3c_tvscaler_mem == NULL) {
		printk(KERN_ERR PFX "failed to reserve memory region\n");
              return -ENOENT;
	}

	base = ioremap(s3c_tvscaler_mem->start, s3c_tvscaler_mem->end - res->start + 1);
	if(s3c_tvscaler_mem == NULL) {
		printk(KERN_ERR PFX "failed ioremap\n");
                return -ENOENT;
	}

	tvscaler_clock = clk_get(&pdev->dev, "tv_encoder");
        if(tvscaler_clock == NULL) {
                printk(KERN_ERR PFX "failed to find tvscaler clock source\n");
                return -ENOENT;
        }

        clk_enable(tvscaler_clock);

	h_clk = clk_get(&pdev->dev, "hclk");
        if(h_clk == NULL) {
                printk(KERN_ERR PFX "failed to find h_clk clock source\n");
                return -ENOENT;
        }

	init_waitqueue_head(&waitq);

	ret = request_irq(s3c_tvscaler_irq, s3c_tvscaler_isr, IRQF_DISABLED,
			"TV_SCALER", NULL);
	if (ret) {
		printk("request_irq(TV_SCALER) failed.\n");
		return ret;
	}
	
	printk(" Success\n");

       return 0;
}

static int s3c_tvscaler_remove(struct platform_device *dev)
{
	printk(KERN_INFO "s3c_tvscaler_remove called !\n");
       clk_disable(tvscaler_clock);
	free_irq(s3c_tvscaler_irq, NULL);
	if (s3c_tvscaler_mem != NULL) {
		pr_debug("s3-tvscaler: releasing s3c_tvscaler_mem\n");
		iounmap(base);
		release_resource(s3c_tvscaler_mem);
		kfree(s3c_tvscaler_mem);
	}

	return 0;
}

static int s3c_tvscaler_suspend(struct platform_device *dev, pm_message_t state)
{
       clk_disable(tvscaler_clock);
	return 0;
}

static int s3c_tvscaler_resume(struct platform_device *pdev)
{
       clk_enable(tvscaler_clock);
	return 0;
}

static struct platform_driver s3c_tvscaler_driver = {
       .probe          = s3c_tvscaler_probe,
       .remove         = s3c_tvscaler_remove,
       .suspend        = s3c_tvscaler_suspend,
       .resume         = s3c_tvscaler_resume,
       .driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c-tvscaler",
	},
};

static char banner[] __initdata = KERN_INFO "S3C6410 TV scaler Driver, (c) 2008 Samsung Electronics\n";


int __init  s3c_tvscaler_pre_init(void)
{

	printk(banner);
	
 	if(platform_driver_register(&s3c_tvscaler_driver) != 0)
  	{
   		printk("platform device register Failed \n");
   		return -1;
  	}

	// 2008.03.24 comment out by hyunkyung	
	//printk(" S3C6410 TV scaler Driver module init OK. %x\n", DRAM_END_ADDR);

       return 0;
}

void  s3c_tvscaler_exit(void)
{
       platform_driver_unregister(&s3c_tvscaler_driver);
 	printk("S3C: tvscaler module exit\n");
}

module_init(s3c_tvscaler_pre_init);
module_exit(s3c_tvscaler_exit);


MODULE_AUTHOR("Peter, Oh");
MODULE_DESCRIPTION("S3C TV Controller Device Driver");
MODULE_LICENSE("GPL");


