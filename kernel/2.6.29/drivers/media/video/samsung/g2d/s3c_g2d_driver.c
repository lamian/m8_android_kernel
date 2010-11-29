/*
 * Copyright  2008 Samsung Electronics Co, Ltd. All Rights Reserved. 
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics  ("Confidential Information").   
 * you shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics 
 *
 * This file implements s3c-g2d driver.
 *
 * @name G2D DRIVER MODULE Module (s3c_g2d_driver.c)
 * @date 2008-12-05
 */
#include <linux/init.h>

#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <asm/uaccess.h>
#include <linux/errno.h> /* error codes */
#include <asm/div64.h>
#include <linux/tty.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>

#include <linux/version.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/signal.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/kmod.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/semaphore.h>

#include <asm/io.h>
#include <asm/page.h>
#include <asm/irq.h>
#include <linux/mm.h>
#include <linux/moduleparam.h>

#include <mach/hardware.h>
#include <mach/map.h>

#include <plat/power-clock-domain.h>
#include <plat/pm.h>

#include "FIMGSE2D.h"
#include "regs_s3c_g2d.h"
#include "s3c_g2d_driver.h"

#ifdef CONFIG_S3C64XX_DOMAIN_GATING
/* 
 * 2009/09/30 Kang hui sung
 * Disabling using G2D domain gating temporally.
 */
/* #define USE_G2D_DOMAIN_GATING */
#endif

//#define USE_G2D_MMAP

static struct clk *s3c_g2d_clock;
static int s3c_g2d_irq_num = NO_IRQ;
static struct resource *s3c_g2d_mem;
static void __iomem *s3c_g2d_base;
static wait_queue_head_t waitq_g2d;

#ifdef USE_G2D_DOMAIN_GATING
static int          g_flag_clk_enable = 0;
#endif /* USE_G2D_DOMAIN_GATING */
static int          g_num_of_g2d_object;
static int          g_num_of_nonblock_object = 0;

#ifdef USE_G2D_DOMAIN_GATING
#define USE_G2D_TIMER_FOR_CLK

#ifdef USE_G2D_TIMER_FOR_CLK
static struct timer_list  g_g2d_domain_timer;
static int g2d_pwr_off_flag = 0;
static int g_flag_timer = 0;
DEFINE_SPINLOCK(g2d_domain_lock);
#else
static struct mutex g_g2d_clk_mutex;
#endif /* USE_G2D_TIMER_FOR_CLK */
#endif /* USE_G2D_DOMAIN_GATING */
static struct mutex *h_rot_mutex;

static u16 s3c_g2d_poll_flag = 0;

static FIMGSE2D fimg;

int shift_x,shift_y;

void s3c_g2d_check_fifo(int empty_fifo)
{
//	while((__raw_readl(s3c_g2d_base + S3C_G2D_FIFO_STAT_REG)&0x3f) > (FIFO_NUM - empty_fifo) );
	while((((__raw_readl(s3c_g2d_base + S3C_G2D_FIFO_STAT_REG)&0x7e) >> 1)) > (FIFO_NUM - empty_fifo) );
}
#if 0
static int s3c_g2d_init_regs(s3c_g2d_params *params)
{
	u32 	bpp_mode_dst;
	u32 	bpp_mode_src;
	u32 alpha_reg = 0;
	u32	tmp_reg;
	s3c_g2d_check_fifo(32);
	bpp_mode_src = __raw_readl(s3c_g2d_base + S3C_G2D_SRC_COLOR_MODE);
	bpp_mode_src &= 0xfffffff8;
	bpp_mode_dst = __raw_readl(s3c_g2d_base + S3C_G2D_DST_COLOR_MODE);
	bpp_mode_dst &= 0xfffffff8;
	
	bpp_mode_dst |=
				(params->bpp_dst == G2D_RGB16)   ? S3C_G2D_COLOR_RGB_565 :
				(params->bpp_dst == G2D_RGBA16) ? S3C_G2D_COLOR_RGBA_5551 :
				(params->bpp_dst == G2D_ARGB16) ? S3C_G2D_COLOR_ARGB_1555 :	
				(params->bpp_dst == G2D_RGBA32) ? S3C_G2D_COLOR_RGBA_8888 :	
				(params->bpp_dst == G2D_ARGB32) ? S3C_G2D_COLOR_ARGB_8888 :
				(params->bpp_dst == G2D_XRGB32) ? S3C_G2D_COLOR_XRGB_8888 :	
				(params->bpp_dst == G2D_RGBX32) ? S3C_G2D_COLOR_RGBX_8888 : S3C_G2D_COLOR_RGB_565;

	bpp_mode_src |= 
				(params->bpp_src == G2D_RGB16)   ? S3C_G2D_COLOR_RGB_565 :
				(params->bpp_src == G2D_RGBA16) ? S3C_G2D_COLOR_RGBA_5551 :
				(params->bpp_src == G2D_ARGB16) ? S3C_G2D_COLOR_ARGB_1555 :	
				(params->bpp_src == G2D_RGBA32) ? S3C_G2D_COLOR_RGBA_8888 :	
				(params->bpp_src == G2D_ARGB32) ? S3C_G2D_COLOR_ARGB_8888 :
				(params->bpp_src == G2D_XRGB32) ? S3C_G2D_COLOR_XRGB_8888 :	
				(params->bpp_src == G2D_RGBX32) ? S3C_G2D_COLOR_RGBX_8888 : S3C_G2D_COLOR_RGB_565;
				
	/*set register for soruce image ===============================*/
	__raw_writel(params->src_base_addr,  s3c_g2d_base + S3C_G2D_SRC_BASE_ADDR);
	__raw_writel(params->src_full_width, s3c_g2d_base + S3C_G2D_HORI_RES_REG);
	__raw_writel(params->src_full_height, s3c_g2d_base + S3C_G2D_VERT_RES_REG);
	__raw_writel(((params->src_full_height<<16)|(params->src_full_width)), s3c_g2d_base+S3C_G2D_SRC_RES_REG);
	__raw_writel(bpp_mode_src, s3c_g2d_base + S3C_G2D_SRC_COLOR_MODE);

	if(params->bpp_src == G2D_RGBA32)
		__raw_writel(1, s3c_g2d_base + 0x350);
	else
		__raw_writel(0, s3c_g2d_base + 0x350);
	
	/*set register for destination image =============================*/
	__raw_writel(params->dst_base_addr,  s3c_g2d_base + S3C_G2D_DST_BASE_ADDR);
	__raw_writel(params->dst_full_width, s3c_g2d_base + S3C_G2D_SC_HORI_REG);
	__raw_writel(params->dst_full_height, s3c_g2d_base + S3C_G2D_SC_VERT_REG);
	__raw_writel(((params->dst_full_height<<16)|(params->dst_full_width)), s3c_g2d_base+S3C_G2D_SC_HORI_REG);
	__raw_writel(bpp_mode_dst, s3c_g2d_base + S3C_G2D_DST_COLOR_MODE);

	/*set register for clipping window===============================*/
	__raw_writel(params->cw_x1, s3c_g2d_base + S3C_G2D_CW_LT_X_REG);
	__raw_writel(params->cw_y1, s3c_g2d_base + S3C_G2D_CW_LT_Y_REG);
	__raw_writel(params->cw_x2, s3c_g2d_base + S3C_G2D_CW_RB_X_REG);
	__raw_writel(params->cw_y2, s3c_g2d_base + S3C_G2D_CW_RB_Y_REG);

	/*set register for color=======================================*/
	__raw_writel(params->color_val[G2D_WHITE], s3c_g2d_base + S3C_G2D_FG_COLOR_REG); // set color to both font and foreground color
	__raw_writel(params->color_val[G2D_BLACK], s3c_g2d_base + S3C_G2D_BG_COLOR_REG);
	__raw_writel(params->color_val[G2D_BLUE], s3c_g2d_base + S3C_G2D_BS_COLOR_REG); // Set blue color to blue screen color

	/*set register for ROP & Alpha==================================*/
	alpha_reg = __raw_readl(s3c_g2d_base + S3C_G2D_ROP_REG);
	alpha_reg = alpha_reg & 0xffffc000;
	if(params->alpha_mode == TRUE)
	{
		//printk("Does Alpha Blending\n");
		switch (bpp_mode_src)
		{
		case S3C_G2D_COLOR_RGBA_5551 :
		case S3C_G2D_COLOR_ARGB_1555 :	
		case S3C_G2D_COLOR_RGBA_8888 :	
		case S3C_G2D_COLOR_ARGB_8888 :
			alpha_reg |= S3C_G2D_ROP_REG_ABM_SRC_BITMAP;
			break;
		default:
			break;
		}
		alpha_reg |= S3C_G2D_ROP_REG_ABM_REGISTER |  G2D_ROP_SRC_ONLY;
		//__raw_writel(S3C_G2D_ROP_REG_OS_FG_COLOR | S3C_G2D_ROP_REG_ABM_REGISTER |  S3C_G2D_ROP_REG_T_OPAQUE_MODE | G2D_ROP_SRC_ONLY, s3c_g2d_base + S3C_G2D_ROP_REG);
		if(params->alpha_val > ALPHA_VALUE_MAX){
			printk(KERN_ALERT"s3c g2d dirver error: exceed alpha value range 0~255\n");
               	 return -ENOENT;
		}
		__raw_writel(params->alpha_val, s3c_g2d_base + S3C_G2D_ALPHA_REG);		
	}
	else
	{
		//printk("No Alpha Blending\n");
		alpha_reg |= S3C_G2D_ROP_REG_OS_FG_COLOR | S3C_G2D_ROP_REG_ABM_NO_BLENDING | S3C_G2D_ROP_REG_T_OPAQUE_MODE | G2D_ROP_SRC_ONLY;
		//__raw_writel(alpha_reg, s3c_g2d_base + S3C_G2D_ROP_REG);
		//__raw_writel(((0x0<<8) | (0xff<<0)), s3c_g2d_base + S3C_G2D_ALPHA_REG);
	}
	__raw_writel(alpha_reg, s3c_g2d_base + S3C_G2D_ROP_REG);

	/*set register for color key====================================*/
	if(params->color_key_mode == TRUE){
		tmp_reg = __raw_readl(s3c_g2d_base + S3C_G2D_ROP_REG);
		tmp_reg |= S3C_G2D_ROP_REG_T_TRANSP_MODE ;
		__raw_writel(tmp_reg , s3c_g2d_base + S3C_G2D_ROP_REG);
		__raw_writel(params->color_key_val, s3c_g2d_base + S3C_G2D_BS_COLOR_REG); 	
	}

	/*set register for rotation=====================================*/
	/*/ New Mathew K J 27 Feb 2009 
	s3c_g2d_check_fifo(8);
	//__raw_writel(S3C_G2D_ROTATRE_REG_R0_0 + (S3C_G2D_ROTATRE_REG_R0_0 << 16), s3c_g2d_base + S3C_G2D_ROT_OC_REG);
	__raw_writel(__raw_readl(s3c_g2d_base + S3C_G2D_ROT_OC_REG) & 0xf800f800, s3c_g2d_base + S3C_G2D_ROT_OC_REG);
	__raw_writel(__raw_readl(s3c_g2d_base + S3C_G2D_ROT_OC_X_REG) & 0xfffff800, s3c_g2d_base + S3C_G2D_ROT_OC_X_REG);
	__raw_writel(__raw_readl(s3c_g2d_base + S3C_G2D_ROT_OC_Y_REG) & 0xf800ffff, s3c_g2d_base + S3C_G2D_ROT_OC_Y_REG);
	__raw_writel((__raw_readl(s3c_g2d_base + S3C_G2D_ROTATE_REG) & 0xffffffc0)  | S3C_G2D_ROTATRE_REG_R0_0, s3c_g2d_base + S3C_G2D_ROTATE_REG);
	//~New Mathew K J 27 Feb 2009
	//  */
	
	__raw_writel(S3C_G2D_ROTATRE_REG_R0_0, s3c_g2d_base + S3C_G2D_ROT_OC_X_REG);
	__raw_writel(S3C_G2D_ROTATRE_REG_R0_0, s3c_g2d_base + S3C_G2D_ROT_OC_Y_REG);
	__raw_writel(S3C_G2D_ROTATRE_REG_R0_0, s3c_g2d_base + S3C_G2D_ROTATE_REG);
	
	return 0;
	
}

/* Removed temporaly becuase this code is not used */
/*
void s3c_2d_disable_effect(void)
{
	u32 val; 

	val = __raw_readl(s3c_g2d_base + S3C_G2D_ROP_REG);
	val &= ~(0x7<<10);
	__raw_writel(val, s3c_g2d_base + S3C_G2D_ROP_REG);
}
*/

void s3c_g2d_bitblt(u16 src_x1, u16 src_y1, u16 src_x2, u16 src_y2,
  	                  u16 dst_x1, u16 dst_y1, u16 dst_x2, u16 dst_y2)
{
	u32 uCmdRegVal;
	int is_stretch = FALSE;
	// At this point, x1 and y1 are never greater than x2 and y2. Also, destination work-dimensions are never zero.
	u16 src_work_width  = src_x2 - src_x1 + 1;
	u16 dst_work_width  = dst_x2 - dst_x1 + 1;
	u16 src_work_height = src_y2 - src_y1 + 1;
	u16 dst_work_height = dst_y2 - dst_y1 + 1;

	s3c_g2d_check_fifo(11);
   	
	__raw_writel(src_x1, s3c_g2d_base + S3C_G2D_COORD0_X_REG);
	__raw_writel(src_y1, s3c_g2d_base + S3C_G2D_COORD0_Y_REG);
    	__raw_writel(src_x2, s3c_g2d_base + S3C_G2D_COORD1_X_REG);
	__raw_writel(src_y2, s3c_g2d_base + S3C_G2D_COORD1_Y_REG);

   	 __raw_writel(dst_x1, s3c_g2d_base + S3C_G2D_COORD2_X_REG);
    	__raw_writel(dst_y1, s3c_g2d_base + S3C_G2D_COORD2_Y_REG);
    	__raw_writel(dst_x2, s3c_g2d_base + S3C_G2D_COORD3_X_REG);
    	__raw_writel(dst_y2, s3c_g2d_base + S3C_G2D_COORD3_Y_REG);

	// Set registers for X and Y scaling============================
	if ((src_work_width != dst_work_width) || (src_work_height != dst_work_height))
	{
		u32 x_inc_fixed;
		u32 y_inc_fixed;
		
		is_stretch = TRUE;
		
		x_inc_fixed = ((src_work_width / dst_work_width) << 11)
						+ (((src_work_width % dst_work_width) << 11) / dst_work_width);
	
		y_inc_fixed = ((src_work_height / dst_work_height) << 11)
						+ (((src_work_height % dst_work_height) << 11) / dst_work_height);
		
		__raw_writel(x_inc_fixed, s3c_g2d_base + S3C_G2D_X_INCR_REG);
		__raw_writel(y_inc_fixed, s3c_g2d_base + S3C_G2D_Y_INCR_REG);
		
	}
	uCmdRegVal=readl(s3c_g2d_base + S3C_G2D_CMD1_REG);
	uCmdRegVal = ~(0x3<<0);
	uCmdRegVal |= is_stretch ? S3C_G2D_CMD1_REG_S : S3C_G2D_CMD1_REG_N;
	__raw_writel(uCmdRegVal, s3c_g2d_base + S3C_G2D_CMD1_REG);
		

}


static void s3c_g2d_rotate_with_bitblt(s3c_g2d_params *params, ROT_TYPE rot_degree)
{
	u16 org_x=0, org_y=0;
	u32 uRotDegree;
	u32 src_x1, src_y1, src_x2, src_y2, dst_x1, dst_y1, dst_x2, dst_y2;
	u32 src_x2_resized, src_y2_resized;

	src_x1 = params->src_start_x;
	src_y1 = params->src_start_y;
	src_x2 = params->src_start_x + params->src_work_width  -1;
	src_y2 = params->src_start_y + params->src_work_height -1;

	dst_x1 = params->dst_start_x;
	dst_y1 = params->dst_start_y;
	dst_x2 = params->dst_start_x + params->dst_work_width  -1;
	dst_y2 = params->dst_start_y + params->dst_work_height -1;

	switch(rot_degree)
	{
		case ROT_0:
		case ROT_180:
		case ROT_X_FLIP:
		case ROT_Y_FLIP:
			src_x2_resized = src_x1 + params->dst_work_width  -1;
			src_y2_resized = src_y1 + params->dst_work_height -1;
			break;
		
		case ROT_90:
		case ROT_270:
			src_x2_resized = src_x1 + params->dst_work_height -1;
			src_y2_resized = src_y1 + params->dst_work_width  -1;
			break;
		
		default:
			break;
	}

	s3c_g2d_get_rotation_origin(src_x1, src_y1, src_x2_resized, src_y2_resized, dst_x1, dst_y1, rot_degree, &org_x, &org_y);
	
	s3c_g2d_check_fifo(8);

	__raw_writel(org_x, s3c_g2d_base + S3C_G2D_ROT_OC_X_REG);
	__raw_writel(org_y, s3c_g2d_base + S3C_G2D_ROT_OC_Y_REG);
	
	/*/ New Mathew K J 27 Feb 2009 
	
	__raw_writel((__raw_readl(s3c_g2d_base + S3C_G2D_ROT_OC_REG) & 0xf800f800) | (org_x + (org_y << 16)), s3c_g2d_base + S3C_G2D_ROT_OC_REG);
	__raw_writel((__raw_readl(s3c_g2d_base + S3C_G2D_ROT_OC_X_REG) & 0xfffff800) | org_x, s3c_g2d_base + S3C_G2D_ROT_OC_X_REG);
	__raw_writel((__raw_readl(s3c_g2d_base + S3C_G2D_ROT_OC_Y_REG) & 0xf800ffff) |org_y, s3c_g2d_base + S3C_G2D_ROT_OC_Y_REG);
	//__raw_writel(org_x + (org_y << 16), s3c_g2d_base + S3C_G2D_ROT_OC_REG);
	//~New Mathew K J 27 Feb 2009 
	//  */

	uRotDegree =
		(rot_degree == ROT_0) ? S3C_G2D_ROTATRE_REG_R0_0 :
		(rot_degree == ROT_90) ? S3C_G2D_ROTATRE_REG_R1_90 :
		(rot_degree == ROT_180) ? S3C_G2D_ROTATRE_REG_R2_180 : 
		(rot_degree == ROT_270) ? S3C_G2D_ROTATRE_REG_R3_270 :
		(rot_degree == ROT_X_FLIP) ? S3C_G2D_ROTATRE_REG_FX : S3C_G2D_ROTATRE_REG_FY;

       __raw_writel(uRotDegree, s3c_g2d_base + S3C_G2D_ROTATE_REG);
	//  __raw_writel((__raw_readl(s3c_g2d_base + S3C_G2D_ROTATE_REG) & 0xffffffc0)  | uRotDegree, s3c_g2d_base + S3C_G2D_ROTATE_REG);
	__raw_writel(S3C_G2D_INTEN_REG_CCF, s3c_g2d_base + S3C_G2D_INTEN_REG);

	switch(rot_degree)
	{
		case ROT_0:
		case ROT_X_FLIP:
		case ROT_Y_FLIP:
			s3c_g2d_bitblt(src_x1, src_y1, src_x2, src_y2, dst_x1 , dst_y1 , dst_x2, dst_y2);
			break;
			
		case ROT_90:
		case ROT_270:
		case ROT_180:
			s3c_g2d_bitblt(src_x1, src_y1, src_x2, src_y2, 
							src_x1+shift_x, src_y1+shift_y, src_x2_resized+shift_x, src_y2_resized+shift_y);
			break;

		default:
			break;
	}
	
}

static void s3c_g2d_get_rotation_origin(u16 src_x1, u16 src_y1, u16 src_x2, u16 src_y2, u16 dst_x1, u16 dst_y1,  ROT_TYPE rot_degree, u16* org_x, u16* org_y)
{
	//s3c_g2d_check_fifo(17);
	switch(rot_degree)
	{
		case ROT_0:
			return;
		case ROT_90:

			// workaround : if (org position < 0)
			if((int)dst_x1 - (int)dst_y1 + (int)src_x1 + (int)src_y2 >=0)	shift_x = 0;
			else shift_x = (int)dst_y1 - ((int)dst_x1 + (int)src_x1 + (int)src_y2);

			// workaround : if org position is float.
			shift_y = (int)(((int)dst_x1 - ((int)dst_y1 - shift_x) + (int)src_x1 + (int)src_y2))&0x1;	

			*org_x = ((int)(((int)dst_x1 - ((int)dst_y1 - shift_x) + (int)src_x1 + (int)(src_y2+shift_y)))>>1);
			*org_y = ((int)(((int)dst_x1 + ((int)dst_y1 - shift_x) - (int)src_x1 + (int)(src_y2+shift_y)))>>1);

			break;
		case ROT_180:
		case ROT_X_FLIP:
		case ROT_Y_FLIP:
			shift_x = (dst_x1 + src_x2)&0x1;
			shift_y = (dst_y1 + src_y2)&0x1;

			*org_x = ((dst_x1 + src_x2)>>1)+shift_x;
			*org_y = ((dst_y1 + src_y2)>>1)+shift_y;

			break;
		case ROT_270:
			shift_x = (int)(((int)dst_x1 + (int)dst_y1 + (int)src_x2 - (int)src_y1+shift_y))&0x1;
			shift_y = 0;

			*org_x = (int)(((int)dst_x1 + (int)dst_y1 + (int)(src_x2+shift_x) - (int)src_y1)>>1);
			*org_y = (int)(((int)dst_y1 - (int)dst_x1 + (int)(src_x2+shift_x) + (int)src_y1)>>1);

			break;
			
		default:
			break;
	}
}

static void s3c_g2d_rotate_clear(void)
{
	__raw_writel(S3C_G2D_ROTATRE_REG_R0_0, s3c_g2d_base + S3C_G2D_ROTATE_REG); // Initialize Rotation Mode Register
}

static void s3c_g2d_bitblt_start(s3c_g2d_params *params)
{
	u32 src_x1 = params->src_start_x;
	u32 src_y1 = params->src_start_y;
	u32 src_x2 = params->src_start_x + params->src_work_width  -1;
	u32 src_y2 = params->src_start_y + params->src_work_height -1;
	u32 dst_x1 = params->dst_start_x;
	u32 dst_y1 = params->dst_start_y;
	u32 dst_x2 = params->dst_start_x + params->dst_work_width  -1;
	u32 dst_y2 = params->dst_start_y + params->dst_work_height -1;

	s3c_g2d_bitblt(src_x1, src_y1, src_x2, src_y2, dst_x1, dst_y1, dst_x2, dst_y2);
}

static void s3c_g2d_set_alpha_blending(s3c_g2d_params *params)
{
	u32 val;
	val = __raw_readl(s3c_g2d_base + S3C_G2D_ROP_REG);
	val |= S3C_G2D_ROP_REG_ABM_REGISTER;
	__raw_writel(val , s3c_g2d_base + S3C_G2D_ROP_REG);
	__raw_writel( (params->alpha_val <<0), s3c_g2d_base + S3C_G2D_ALPHA_REG);
}

static void s3c_g2d_set_transparent(s3c_g2d_params *params)
{
	u32 val;
	val = __raw_readl(s3c_g2d_base + S3C_G2D_ROP_REG);
	val |= (S3C_G2D_ROP_REG_T_TRANSP_MODE |S3C_G2D_ROP_REG_B_BS_MODE_OFF| S3C_G2D_ROP_REG_ABM_NO_BLENDING |G2D_ROP_SRC_ONLY) ;
	__raw_writel(val , s3c_g2d_base + S3C_G2D_ROP_REG);

	__raw_writel(params->color_key_val, s3c_g2d_base + S3C_G2D_BS_COLOR_REG); 
}
#endif


static void s3c_g2d_rotator_start(s3c_g2d_params *params, ROT_TYPE rot_degree)
{

//	s3c_g2d_init_regs(params);
//	s3c_g2d_rotate_with_bitblt(params, rot_degree);

#if 0
	if(params->bpp_src == G2D_RGBA32)
		__raw_writel(1, s3c_g2d_base + 0x350);
	else
		__raw_writel(0, s3c_g2d_base + 0x350);

	/*set register for color=======================================*/
	__raw_writel(params->color_val[G2D_WHITE], s3c_g2d_base + S3C_G2D_FG_COLOR_REG); // set color to both font and foreground color
	__raw_writel(params->color_val[G2D_BLACK], s3c_g2d_base + S3C_G2D_BG_COLOR_REG);
	__raw_writel(params->color_val[G2D_BLUE], s3c_g2d_base + S3C_G2D_BS_COLOR_REG); // Set blue color to blue screen color

	/*set register for ROP & Alpha==================================*/
	u32 alpha_reg = __raw_readl(s3c_g2d_base + S3C_G2D_ROP_REG);
	alpha_reg = alpha_reg & 0xffffc000;

	if(params->alpha_mode == TRUE)
	{
		//printk("Does Alpha Blending\n");
		switch (params->bpp_src)
		{
		case G2D_RGBA16 :
		case G2D_ARGB16 :	
		case G2D_RGBA32 :	
		case G2D_ARGB32 :
			alpha_reg |= S3C_G2D_ROP_REG_ABM_SRC_BITMAP;
			break;
		default:
			break;
		}
		alpha_reg |= S3C_G2D_ROP_REG_ABM_REGISTER |  G2D_ROP_SRC_ONLY;
		//__raw_writel(S3C_G2D_ROP_REG_OS_FG_COLOR | S3C_G2D_ROP_REG_ABM_REGISTER |  S3C_G2D_ROP_REG_T_OPAQUE_MODE | G2D_ROP_SRC_ONLY, s3c_g2d_base + S3C_G2D_ROP_REG);
		if(params->alpha_val > ALPHA_VALUE_MAX){
			printk(KERN_ALERT"s3c g2d dirver error: exceed alpha value range 0~255\n");
				 return;
		}
		__raw_writel(params->alpha_val, s3c_g2d_base + S3C_G2D_ALPHA_REG);		
	}
	else
	{
		//printk("No Alpha Blending\n");
		alpha_reg |= S3C_G2D_ROP_REG_OS_FG_COLOR | S3C_G2D_ROP_REG_ABM_NO_BLENDING | S3C_G2D_ROP_REG_T_OPAQUE_MODE | G2D_ROP_SRC_ONLY;
		//__raw_writel(alpha_reg, s3c_g2d_base + S3C_G2D_ROP_REG);
		//__raw_writel(((0x0<<8) | (0xff<<0)), s3c_g2d_base + S3C_G2D_ALPHA_REG);
	}
	__raw_writel(alpha_reg, s3c_g2d_base + S3C_G2D_ROP_REG);

	/*set register for color key====================================*/
	if(params->color_key_mode == TRUE){
		u32 tmp_reg = __raw_readl(s3c_g2d_base + S3C_G2D_ROP_REG);
		tmp_reg |= S3C_G2D_ROP_REG_T_TRANSP_MODE ;
		__raw_writel(tmp_reg , s3c_g2d_base + S3C_G2D_ROP_REG);
		__raw_writel(params->color_key_val, s3c_g2d_base + S3C_G2D_BS_COLOR_REG);	
	}
#endif

	SetEndian(fimg.m_pG2DReg, params->bpp_src == G2D_RGBA32, params->bpp_dst == G2D_RGBA32);
	if (params->alpha_mode == true)
	{
		switch (params->bpp_src)
		{
		case G2D_RGBA16 :
		case G2D_ARGB16 :	
		case G2D_RGBA32 :	
		case G2D_ARGB32 :
			SetAlphaMode(&fimg, ALPHAFX_PERPIXELALPHA);
			break;
		default:
			SetAlphaMode(&fimg, ALPHAFX_CONSTALPHA);
			break;
		}
		SetAlphaValue(&fimg, params->alpha_val);
	}
	else
	{
		SetAlphaMode(&fimg, ALPHAFX_NOALPHA);
	}
	SetRopEtype(&fimg, ROP_SRC_ONLY);
	Set3rdOperand(fimg.m_pG2DReg, G2D_OPERAND3_FG);

	SetTransparentMode(&fimg, params->color_key_mode, params->color_key_val);

	SURFACE_DESCRIPTOR SrcSurface, DstSurface;
	RECT SrcRect, DstRect, ClipRect;

	SrcSurface.dwBaseaddr = params->src_base_addr;
	SrcSurface.dwColorMode = params->bpp_src;
	SrcSurface.dwHoriRes = params->src_full_width;
	SrcSurface.dwVertRes = params->src_full_height;

	DstSurface.dwBaseaddr = params->dst_base_addr;
	DstSurface.dwColorMode = params->bpp_dst;
	DstSurface.dwHoriRes = params->dst_full_width;
	DstSurface.dwVertRes = params->dst_full_height;

	SrcRect.left = params->src_start_x;
	SrcRect.top = params->src_start_y;
	SrcRect.right = SrcRect.left + params->src_work_width;
	SrcRect.bottom = SrcRect.top + params->src_work_height;
	
	DstRect.left = params->dst_start_x;
	DstRect.top = params->dst_start_y;
	DstRect.right = DstRect.left + params->dst_work_width;
	DstRect.bottom = DstRect.top + params->dst_work_height;

	ClipRect.left = params->cw_x1;
	ClipRect.top = params->cw_y1;
	ClipRect.right = params->cw_x2;
	ClipRect.bottom = params->cw_y2;

	SetSrcSurface(&fimg, &SrcSurface);
	SetDstSurface(&fimg, &DstSurface);
	SetClipWindow(fimg.m_pG2DReg, &ClipRect);
	StretchBlt(&fimg, &SrcRect, &DstRect, rot_degree);
}

irqreturn_t s3c_g2d_irq(int irq, void *dev_id)
{
#ifdef USE_G2D_DOMAIN_GATING
	if (g_flag_clk_enable == 1)
#endif
	{
	if(__raw_readl(s3c_g2d_base + S3C_G2D_INTC_PEND_REG) & S3C_G2D_PEND_REG_INTP_CMD_FIN){
	    	__raw_writel ( S3C_G2D_PEND_REG_INTP_CMD_FIN, s3c_g2d_base + S3C_G2D_INTC_PEND_REG );
		wake_up_interruptible(&waitq_g2d);
		s3c_g2d_poll_flag = 1;
	}
	}
	return IRQ_HANDLED;
}

#ifdef USE_G2D_DOMAIN_GATING
static int s3c_g2d_clk_enable(void)
{
	if(g_flag_clk_enable == 0)
	{
		s3c_set_normal_cfg(S3C64XX_DOMAIN_P, S3C64XX_ACTIVE_MODE, S3C64XX_2D);
		if(s3c_wait_blk_pwr_ready(S3C64XX_BLK_P)) {
			return -1;
		}
		clk_enable(s3c_g2d_clock);
		g_flag_clk_enable = 1;
	}

	return 0;
}

static int s3c_g2d_clk_disable(void)
{
	if(    g_flag_clk_enable        == 1
		&& g_num_of_nonblock_object == 0)
	{
		clk_disable(s3c_g2d_clock);
		s3c_set_normal_cfg(S3C64XX_DOMAIN_P, S3C64XX_LP_MODE, S3C64XX_2D);
		g_flag_clk_enable = 0;
	}
	return 0;
}

#ifdef USE_G2D_TIMER_FOR_CLK
static int s3c_g2d_domain_timer(unsigned long arg)
{
	spin_lock(&g2d_domain_lock);
	if(g2d_pwr_off_flag){
		if(    g_flag_clk_enable        == 1
                && g_num_of_nonblock_object == 0) {
                clk_disable(s3c_g2d_clock);
                s3c_set_normal_cfg(S3C64XX_DOMAIN_P, S3C64XX_LP_MODE, S3C64XX_2D);
                g_flag_clk_enable = 0;
		g_flag_timer = 0;
            }
	
	}
	else {
		if(g_flag_clk_enable        == 1
                && g_num_of_nonblock_object == 0 
		&& g_flag_timer == 1) {
		  mod_timer(&g_g2d_domain_timer, jiffies + HZ);		
		}
	}

	spin_unlock(&g2d_domain_lock);
}
#endif /* USE_G2D_TIMER_FOR_CLK */
#endif /* USE_G2D_DOMAIN_GATING */

 int s3c_g2d_open(struct inode *inode, struct file *file)
{
	s3c_g2d_params *params;
	params = (s3c_g2d_params *)kmalloc(sizeof(s3c_g2d_params), GFP_KERNEL);
	if(params == NULL){
		printk(KERN_ERR "Instance memory allocation was failed\n");
		return -1;
	}

	memset(params, 0, sizeof(s3c_g2d_params));

	file->private_data	= (s3c_g2d_params *)params;
	
	g_num_of_g2d_object++;

	if(file->f_flags & O_NONBLOCK)
		g_num_of_nonblock_object++;
#ifdef USE_G2D_DOMAIN_GATING		
#ifndef USE_G2D_TIMER_FOR_CLK
	if(g_num_of_g2d_object == 1)
	{
		mutex_init(&g_g2d_clk_mutex);
	}
#endif
#endif /* USE_G2D_DOMAIN_GATING */

#ifdef G2D_DEBUG	
	printk("s3c_g2d_open() <<<<<<<< NEW DRIVER >>>>>>>>>>>>>>\n"); 	
#endif

	return 0;
}


int s3c_g2d_release(struct inode *inode, struct file *file)
{
	s3c_g2d_params	*params;

	params	= (s3c_g2d_params *)file->private_data;
	if (params == NULL) {
		printk(KERN_ERR "Can't release s3c_rotator!!\n");
		return -1;
	}

	kfree(params);
	
	g_num_of_g2d_object--;

	if(file->f_flags & O_NONBLOCK)
		g_num_of_nonblock_object--;
#ifdef USE_G2D_DOMAIN_GATING
	s3c_g2d_clk_disable();
#ifndef USE_G2D_TIMER_FOR_CLK	

	if(g_num_of_g2d_object == 0)
	{
		mutex_destroy(&g_g2d_clk_mutex);
	}
#endif /* USE_G2D_TIMER_FOR_CLK */
#endif /* USE_G2D_DOMAIN_GATING */

#ifdef G2D_DEBUG
	printk("s3c_g2d_release() \n"); 
#endif

	return 0;
}

#ifdef USE_G2D_MMAP
int s3c_g2d_mmap(struct file* filp, struct vm_area_struct *vma) 
{
	unsigned long pageFrameNo=0;
	unsigned long size;
	
	size = vma->vm_end - vma->vm_start;

	// page frame number of the address for a source G2D_SFR_SIZE to be stored at. 
	//pageFrameNo = __phys_to_pfn(S3C6400_PA_G2D);
	
	if(size > G2D_SFR_SIZE) {
		printk("The size of G2D_SFR_SIZE mapping is too big!\n");
		return -EINVAL;
	}
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot); 
	
	if((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED)) {
		printk("Writable G2D_SFR_SIZE mapping must be shared !\n");
		return -EINVAL;
	}
	
	if(remap_pfn_range(vma, vma->vm_start, pageFrameNo, size, vma->vm_page_prot))
		return -EINVAL;
	
	return 0;
}
#endif

static int s3c_g2d_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	s3c_g2d_params	*params;
	ROT_TYPE 		eRotDegree;
	int             ret = 0;

	params	= (s3c_g2d_params*)file->private_data;
	if (copy_from_user(params, (s3c_g2d_params*)arg, sizeof(s3c_g2d_params)))
	{
		return -EFAULT;
	}
	
	mutex_lock(h_rot_mutex);
#ifdef USE_G2D_DOMAIN_GATING
#ifndef USE_G2D_TIMER_FOR_CLK
	mutex_lock(&g_g2d_clk_mutex);
	ret = s3c_g2d_clk_enable();
#else
	spin_lock(&g2d_domain_lock);
	g2d_pwr_off_flag = 0;	
	ret = s3c_g2d_clk_enable();
	spin_unlock(&g2d_domain_lock);
#endif	
#endif /* USE_G2D_DOMAIN_GATING */
	if(ret != 0){
		printk(KERN_ERR "\n%s: Waiting for g2d domain-on is timed-out\n", __FUNCTION__);
		return -EINVAL;
	}

//	static uint64_t _ellapse_ = 0;
//	static struct timeval _tv_start_, _tv_end_;
//	do_gettimeofday(&_tv_start_);
	switch(cmd)
	{
		case S3C_G2D_ROTATOR_0:
			eRotDegree = ROT_0;
			s3c_g2d_rotator_start(params, eRotDegree);
			break;
			
		case S3C_G2D_ROTATOR_90:
			eRotDegree = ROT_90;
			s3c_g2d_rotator_start(params, eRotDegree);
			break;

		case S3C_G2D_ROTATOR_180:
			eRotDegree = ROT_180;
			s3c_g2d_rotator_start(params, eRotDegree);
			break;
			
		case S3C_G2D_ROTATOR_270:
			eRotDegree = ROT_270;
			s3c_g2d_rotator_start(params, eRotDegree);
			break;

		case S3C_G2D_ROTATOR_X_FLIP:
			eRotDegree = FLIP_X;
			s3c_g2d_rotator_start(params, eRotDegree);
			break;

		case S3C_G2D_ROTATOR_Y_FLIP:
			eRotDegree = FLIP_Y;
			s3c_g2d_rotator_start(params, eRotDegree);
			break;

		
		default:
			ret = -EINVAL;
			goto err_cmd;
	}
	
	// block mode
	if(!(file->f_flags & O_NONBLOCK))
	{
		if (interruptible_sleep_on_timeout(&waitq_g2d, G2D_TIMEOUT) == 0)
		{
#ifdef G2D_DEBUG
			printk(KERN_ERR "\n%s: Waiting for interrupt is timeout\n", __FUNCTION__);
#endif
		}
//		do_gettimeofday(&_tv_end_);
//		_ellapse_ = (_tv_end_.tv_sec - _tv_start_.tv_sec) * 1000000 + (_tv_end_.tv_usec - _tv_start_.tv_usec);
//		printk("g2d cmd=%d, time=%llu\n", cmd, _ellapse_);
	}

err_cmd:

#ifdef USE_G2D_DOMAIN_GATING
#ifdef USE_G2D_TIMER_FOR_CLK
		spin_lock(&g2d_domain_lock);
		g2d_pwr_off_flag = 1;
		g_flag_timer = 1;
		mod_timer(&g_g2d_domain_timer, jiffies + HZ);	
		spin_unlock(&g2d_domain_lock);
#else
		s3c_g2d_clk_disable();
		mutex_unlock(&g_g2d_clk_mutex);
#endif /* USE_G2D_TIMER_FOR_CLK */
#endif /* USE_G2D_DOMAIN_GATING */
	mutex_unlock(h_rot_mutex);

	return ret;
	
}

static unsigned int s3c_g2d_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;

	poll_wait(file, &waitq_g2d, wait);
	if(s3c_g2d_poll_flag == 1)
	{
		mask = POLLOUT|POLLWRNORM;
		s3c_g2d_poll_flag = 0;
	}

	return mask;
}
 struct file_operations s3c_g2d_fops = {
	.owner 	= THIS_MODULE,
	.open 	= s3c_g2d_open,
	.release 	= s3c_g2d_release,
#ifdef USE_G2D_MMAP
	.mmap 	= s3c_g2d_mmap,
#endif
	.ioctl		=s3c_g2d_ioctl,
	.poll		=s3c_g2d_poll,
};


static struct miscdevice s3c_g2d_dev = {
	.minor		= G2D_MINOR,
	.name		= "s3c-g2d",
	.fops		= &s3c_g2d_fops,
};

int s3c_g2d_probe(struct platform_device *pdev)
{

	struct resource *res;
	int ret;

#ifdef G2D_DEBUG
	printk(KERN_ALERT"s3c_g2d_probe called\n");
#endif

	/* find the IRQs */
	s3c_g2d_irq_num = platform_get_irq(pdev, 0);
	if(s3c_g2d_irq_num <= 0) {
		printk(KERN_ERR "failed to get irq resouce\n");
                return -ENOENT;
	}

	ret = request_irq(s3c_g2d_irq_num, s3c_g2d_irq, IRQF_DISABLED, pdev->name, NULL);
	if (ret) {
		printk("request_irq(g2d) failed.\n");
		return ret;
	}


	/* get the memory region */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(res == NULL) {
			printk(KERN_ERR "failed to get memory region resouce\n");
			return -ENOENT;
	}

	s3c_g2d_mem = request_mem_region(res->start, res->end-res->start+1, pdev->name);
	if(s3c_g2d_mem == NULL) {
		printk(KERN_ERR "failed to reserve memory region\n");
                return -ENOENT;
	}


	s3c_g2d_base = ioremap_nocache(s3c_g2d_mem->start, s3c_g2d_mem->end - res->start + 1);
	if(s3c_g2d_base == NULL) {
		printk(KERN_ERR "failed ioremap\n");
                return -ENOENT;
	}

	s3c_g2d_clock = clk_get(&pdev->dev, "hclk_g2d");
	if (IS_ERR(s3c_g2d_clock)) {
			printk(KERN_ERR "failed to find g2d clock source\n");
			return -ENOENT;
	}

	init_waitqueue_head(&waitq_g2d);

	ret = misc_register(&s3c_g2d_dev);
	if (ret) {
		printk (KERN_ERR "cannot register miscdev on minor=%d (%d)\n",
			G2D_MINOR, ret);
		return ret;
	}

	h_rot_mutex = (struct mutex *)kmalloc(sizeof(struct mutex), GFP_KERNEL);
	if (h_rot_mutex == NULL)
		return -1;
	
	mutex_init(h_rot_mutex);
#ifdef USE_G2D_DOMAIN_GATING
#ifdef USE_G2D_TIMER_FOR_CLK
        init_timer(&g_g2d_domain_timer);
        g_g2d_domain_timer.function = s3c_g2d_domain_timer;
#endif /* USE_G2D_TIMER_FOR_CLK */
#endif /* USE_G2D_DOMAIN_GATING */

	memset(&fimg, 0, sizeof(fimg));
	Init(&fimg, s3c_g2d_base);

#ifdef G2D_DEBUG
	printk(KERN_ALERT" s3c_g2d_probe Success\n");
#endif

	return 0;  
}

static int s3c_g2d_remove(struct platform_device *dev)
{
	printk(KERN_INFO "s3c_g2d_remove called !\n");

	free_irq(s3c_g2d_irq_num, NULL);

	iounmap(fimg.m_pG2DReg);
	
	if (s3c_g2d_mem != NULL) {   
		printk(KERN_INFO "S3C G2D  Driver, releasing resource\n");
		iounmap(s3c_g2d_base);
		release_resource(s3c_g2d_mem);
		kfree(s3c_g2d_mem);
	}
	
	misc_deregister(&s3c_g2d_dev);
	printk(KERN_INFO "s3c_g2d_remove Success !\n");
	return 0;
}

static int s3c_g2d_suspend(struct platform_device *dev, pm_message_t state)
{
//	clk_disable(s3c_g2d_clock);
	return 0;
}
static int s3c_g2d_resume(struct platform_device *pdev)
{
//	clk_enable(s3c_g2d_clock);
	return 0;
}


static struct platform_driver s3c_g2d_driver = {
       .probe          = s3c_g2d_probe,
       .remove         = s3c_g2d_remove,
       .suspend        = s3c_g2d_suspend,
       .resume         = s3c_g2d_resume,
       .driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c-g2d",
	},
};

int __init  s3c_g2d_init(void)
{
 	if(platform_driver_register(&s3c_g2d_driver)!=0)
  	{
   		printk("platform device register Failed \n");
   		return -1;
  	}
	printk(" S3C G2D  Init : Done\n");
       	return 0;
}

void  s3c_g2d_exit(void)
{
	platform_driver_unregister(&s3c_g2d_driver);
	mutex_destroy(h_rot_mutex);
 	printk("S3C: G2D module exit\n");
}

module_init(s3c_g2d_init);
module_exit(s3c_g2d_exit);

MODULE_AUTHOR("");
MODULE_DESCRIPTION("S3C G2D Device Driver");
MODULE_LICENSE("GPL");
