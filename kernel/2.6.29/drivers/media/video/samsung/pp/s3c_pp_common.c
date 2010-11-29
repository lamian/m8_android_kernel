
/*
 * linux/drivers/video/s3c_pp_common.c
 *
 * Revision 1.0  
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

#include <linux/version.h>
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,16)
#include <linux/config.h>
#include <asm/arch/registers.h>
#else
#include <plat/regs-pp.h>
#endif

#include "s3c_pp_common.h"

#define PFX "s3c_pp"

// setting the source/destination color space
void set_data_format(s3c_pp_instance_context_t *pp_instance)
{
	// set the source color space
	switch(pp_instance->src_color_space)
	{
		case RGB16:
			//pp_instance->in_pixel_size  = 2;
			pp_instance->in_pixel_shift	= 1;
			
			break;
		case RGB24:
			//pp_instance->in_pixel_size  = 4;
			pp_instance->in_pixel_shift	= 2;
			break;
		case YC420:
		case YC422:
			//pp_instance->in_pixel_size  = 1;
			pp_instance->in_pixel_shift	= 0;
			break;
		case CRYCBY:
		case CBYCRY:
		case YCRYCB:
		case YCBYCR:
			//pp_instance->in_pixel_size  = 2;
			pp_instance->in_pixel_shift	= 1;
			break;
		default:
			break;
	}

	// set the destination color space
	if ( DMA_ONESHOT == pp_instance->out_path ) 
	{
		switch(pp_instance->dst_color_space)
		{
			case RGB16:
				//pp_instance->out_pixel_size  = 2;
				pp_instance->out_pixel_shift = 1;
				break;
			case RGB24:
				//pp_instance->out_pixel_size  = 4;
				pp_instance->out_pixel_shift = 2;
				break;
			case YC420:
			case YC422:
				//pp_instance->out_pixel_size  = 1;
				pp_instance->out_pixel_shift = 0;
				break;
			case CRYCBY:
			case CBYCRY:
			case YCRYCB:
			case YCBYCR:
				//pp_instance->out_pixel_size  = 2;
				pp_instance->out_pixel_shift = 1;
				break;
			default:
				break;
		}
	}
	else if ( FIFO_FREERUN == pp_instance->out_path ) 
	{
		if(pp_instance->dst_color_space == RGB30) 
		{
			//pp_instance->out_pixel_size  = 4;
			pp_instance->out_pixel_shift = 2;
		} 
		else if(pp_instance->dst_color_space == YUV444) 
		{
			//pp_instance->out_pixel_size	 = 4;
			pp_instance->out_pixel_shift = 2;
		} 
	}

	// setting the register about src/dst data format
	set_data_format_register(pp_instance);
}


void set_src_addr(s3c_pp_instance_context_t *pp_instance)
{
	s3c_pp_buf_addr_t	buf_addr;
	
	unsigned int        start_pos_rgb;
	unsigned int        end_pos_rgb;
	unsigned int        start_pos_y;
	unsigned int        end_pos_y;
	unsigned int        start_pos_cb_cr;
	unsigned int        end_pos_cb_cr;

	unsigned int        yuv_shift_value = 0;

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
			buf_addr.in_offset_rgb   = (pp_instance->src_full_width - pp_instance->src_width);
			buf_addr.in_offset_rgb <<=  pp_instance->in_pixel_shift;

			start_pos_rgb            = (pp_instance->src_full_width * pp_instance->src_start_y + pp_instance->src_start_x);
			start_pos_rgb          <<=  pp_instance->in_pixel_shift;

			end_pos_rgb              =  (pp_instance->src_width * pp_instance->src_height)
			                          + (pp_instance->src_height - 1) * (pp_instance->src_full_width - pp_instance->src_width);

			end_pos_rgb            <<=  pp_instance->in_pixel_shift;

			buf_addr.in_start_rgb    = pp_instance->src_buf_addr_phy_rgb_y + start_pos_rgb;
			buf_addr.in_end_rgb      = buf_addr.in_start_rgb               + end_pos_rgb;
			break;
		}
		case YC420 :
		{
			yuv_shift_value = 1;
			// go through
		}			
		case YC422 :
		{
			// y
			buf_addr.in_offset_y    = (pp_instance->src_full_width - pp_instance->src_width);
			buf_addr.in_offset_y  <<=  pp_instance->in_pixel_shift;
			
			start_pos_y             = (pp_instance->src_full_width * pp_instance->src_start_y + pp_instance->src_start_x);
			start_pos_y           <<=  pp_instance->in_pixel_shift;

			end_pos_y               =   (pp_instance->src_width * pp_instance->src_height)
			                          + ((pp_instance->src_height - 1) * (pp_instance->src_full_width - pp_instance->src_width));
			end_pos_y             <<= pp_instance->in_pixel_shift;
			
			buf_addr.in_start_y     = pp_instance->src_buf_addr_phy_rgb_y + start_pos_y;
			buf_addr.in_end_y       = buf_addr.in_start_y                 + end_pos_y;
		
			// cb, cr
			buf_addr.in_offset_cb   = (pp_instance->src_full_width - pp_instance->src_width) >> 1;
			buf_addr.in_offset_cb <<=  pp_instance->in_pixel_shift;
			buf_addr.in_offset_cr   = buf_addr.in_offset_cb;

			start_pos_cb_cr         = (pp_instance->src_full_width >> 1) * (pp_instance->src_start_y >> yuv_shift_value) + (pp_instance->src_start_x >> 1);
			start_pos_cb_cr       <<= pp_instance->in_pixel_shift;

			end_pos_cb_cr           =   ((pp_instance->src_width  >> 1) * (pp_instance->src_height >> yuv_shift_value))
			                          + (((pp_instance->src_height >> yuv_shift_value) - 1) * ((pp_instance->src_full_width - pp_instance->src_width) >> 1));
			end_pos_cb_cr         <<= pp_instance->in_pixel_shift;

			buf_addr.in_start_cb   = pp_instance->src_buf_addr_phy_cb + start_pos_cb_cr;	
			buf_addr.in_end_cb     = buf_addr.in_start_cb             + end_pos_cb_cr;

			buf_addr.in_start_cr   = pp_instance->src_buf_addr_phy_cr + start_pos_cb_cr;
			buf_addr.in_end_cr     = buf_addr.in_start_cr             + end_pos_cb_cr;
			break;
		}
		case CRYCBY :
		case CBYCRY :
		case YCRYCB :
		case YCBYCR :
		case YUV444 :
		{
			// y
			buf_addr.in_offset_y   = (pp_instance->src_full_width - pp_instance->src_width);
			buf_addr.in_offset_y <<=  pp_instance->in_pixel_shift;

			start_pos_y            = (pp_instance->src_full_width * pp_instance->src_start_y + pp_instance->src_start_x);
			start_pos_y          <<=  pp_instance->in_pixel_shift;

			end_pos_y              =  (pp_instance->src_width * pp_instance->src_height)
			                        + (pp_instance->src_height - 1) * (pp_instance->src_full_width - pp_instance->src_width);
			end_pos_y            <<=  pp_instance->in_pixel_shift;
			
			buf_addr.in_start_y    = pp_instance->src_buf_addr_phy_rgb_y + start_pos_y;
			buf_addr.in_end_y      = buf_addr.in_start_y                 + end_pos_y;
			break;
		}
		default:
		{
			printk(KERN_ERR "%s::not matched pp_instance->src_color_space(%d)\n", __FUNCTION__, pp_instance->src_color_space);
			// !!
			return;
			break;
		}
	}	

	set_src_addr_register(&buf_addr, pp_instance);
}

void set_dest_addr(s3c_pp_instance_context_t *pp_instance)
{
	s3c_pp_buf_addr_t	buf_addr;

	unsigned int        start_pos_rgb,   end_pos_rgb;
	unsigned int        start_pos_y,     end_pos_y;
	unsigned int        start_pos_cb_cr, end_pos_cb_cr;

	unsigned int        yuv_shift_value = 0;

	if(DMA_ONESHOT == pp_instance->out_path)
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
				buf_addr.out_offset_rgb   = (pp_instance->dst_full_width - pp_instance->dst_width);
				buf_addr.out_offset_rgb <<=  pp_instance->out_pixel_shift;

				start_pos_rgb             = (pp_instance->dst_full_width * pp_instance->dst_start_y + pp_instance->dst_start_x);
				start_pos_rgb           <<= pp_instance->out_pixel_shift;

				end_pos_rgb               =   (pp_instance->dst_width      * pp_instance->dst_height)
				                            + (pp_instance->dst_height - 1) * (pp_instance->dst_full_width - pp_instance->dst_width);
				end_pos_rgb             <<= pp_instance->out_pixel_shift;

				buf_addr.out_start_rgb    = pp_instance->dst_buf_addr_phy_rgb_y + start_pos_rgb;
				buf_addr.out_end_rgb      = buf_addr.out_start_rgb              + end_pos_rgb;
				break;
			}
			case YC420 :
			{
				yuv_shift_value = 1;
				// go through
			}			
			case YC422 :
			{
				// y
				buf_addr.out_offset_y    = (pp_instance->dst_full_width - pp_instance->dst_width);
				buf_addr.out_offset_y  <<= pp_instance->out_pixel_shift;

				start_pos_y              = (pp_instance->dst_full_width * pp_instance->dst_start_y + pp_instance->dst_start_x);
				start_pos_y            <<= pp_instance->out_pixel_shift;

				end_pos_y                =   (pp_instance->dst_width * pp_instance->dst_height)
				                           + (pp_instance->dst_height - 1) * (pp_instance->dst_full_width - pp_instance->dst_width);
				end_pos_y              <<= pp_instance->out_pixel_shift;
				
				buf_addr.out_start_y     = pp_instance->dst_buf_addr_phy_rgb_y + start_pos_y;
				buf_addr.out_end_y       = buf_addr.out_start_y + end_pos_y;

				// cb, cr
				buf_addr.out_offset_cb   = (pp_instance->dst_full_width - pp_instance->dst_width) >> 1;
				buf_addr.out_offset_cb <<= pp_instance->out_pixel_shift;
				buf_addr.out_offset_cr   = buf_addr.out_offset_cb;

				start_pos_cb_cr          = (pp_instance->dst_full_width >> 1) * (pp_instance->dst_start_y >> yuv_shift_value) + (pp_instance->dst_start_x >> 1);
				start_pos_cb_cr        <<= pp_instance->out_pixel_shift;

				end_pos_cb_cr            =   ((pp_instance->dst_width  >> 1) * (pp_instance->dst_height >> yuv_shift_value))  
								           + ((pp_instance->dst_height >> yuv_shift_value) - 1) * ((pp_instance->dst_full_width - pp_instance->dst_width) >> 1);
				end_pos_cb_cr          <<= pp_instance->out_pixel_shift;
			
				buf_addr.out_start_cb  = pp_instance->dst_buf_addr_phy_cb + start_pos_cb_cr;
				buf_addr.out_end_cb    = buf_addr.out_start_cb            + end_pos_cb_cr;
				buf_addr.out_start_cr  = pp_instance->dst_buf_addr_phy_cr + start_pos_cb_cr;
				buf_addr.out_end_cr    = buf_addr.out_start_cr            + end_pos_cb_cr;
				break;
			}
			case CRYCBY :
			case CBYCRY :
			case YCRYCB :
			case YCBYCR :
			case YUV444 :
			{
				buf_addr.out_offset_y   = (pp_instance->dst_full_width - pp_instance->dst_width);
				buf_addr.out_offset_y <<=  pp_instance->out_pixel_shift;

				start_pos_y             = (pp_instance->dst_full_width * pp_instance->dst_start_y + pp_instance->dst_start_x);
				start_pos_y           <<=  pp_instance->out_pixel_shift;
				
				end_pos_y               =   (pp_instance->dst_width * pp_instance->dst_height)
				                         +  (pp_instance->dst_height - 1) * (pp_instance->dst_full_width - pp_instance->dst_width);
				end_pos_y             <<= pp_instance->out_pixel_shift;
				
				buf_addr.out_start_y    = pp_instance->dst_buf_addr_phy_rgb_y + start_pos_y;
				buf_addr.out_end_y      = buf_addr.out_start_y + end_pos_y;
				break;
			}
			default:
			{
				printk(KERN_ERR "%s::not matched pp_instance->dst_color_space(%d)\n", __FUNCTION__, pp_instance->dst_color_space);
				// !!
				return;
				break;
			}
		}
		set_dest_addr_register (&buf_addr, pp_instance);
	}
}

void set_src_next_buf_addr(s3c_pp_instance_context_t *pp_instance)
{
	s3c_pp_buf_addr_t	buf_addr;
	unsigned int        start_pos_rgb;
	unsigned int        end_pos_rgb;
	unsigned int        start_pos_y;
	unsigned int        end_pos_y;
	
	switch(pp_instance->src_color_space)
	{
		case RGB30 :
		{
			buf_addr.in_offset_rgb   = (pp_instance->src_full_width - pp_instance->src_width);
			buf_addr.in_offset_rgb <<=  pp_instance->in_pixel_shift;
			
			start_pos_rgb            = (pp_instance->src_full_width * pp_instance->src_start_y + pp_instance->src_start_x);
			start_pos_rgb          <<=  pp_instance->in_pixel_shift;

			end_pos_rgb              =  (pp_instance->src_width * pp_instance->src_height)
			                          + (pp_instance->src_height - 1) * (pp_instance->src_full_width - pp_instance->src_width);
			end_pos_rgb            <<= pp_instance->in_pixel_shift;

			buf_addr.in_start_rgb    = pp_instance->src_buf_addr_phy_rgb_y + start_pos_rgb;
			buf_addr.in_end_rgb      = buf_addr.in_start_rgb + end_pos_rgb;
			break;
		}
		case YUV444 :
		{
			// y
			buf_addr.in_offset_y     = (pp_instance->src_full_width - pp_instance->src_width);
			buf_addr.in_offset_y   <<=  pp_instance->in_pixel_shift;

			start_pos_y              = (pp_instance->src_full_width * pp_instance->src_start_y + pp_instance->src_start_x);
			start_pos_y            <<=  pp_instance->in_pixel_shift;

			end_pos_y                =  (pp_instance->src_width * pp_instance->src_height)
			                          + (pp_instance->src_height - 1) * (pp_instance->src_full_width - pp_instance->src_width);
			end_pos_y              <<= pp_instance->in_pixel_shift;


			buf_addr.in_start_y      = pp_instance->src_next_buf_addr_phy_rgb_y + start_pos_y;
			buf_addr.in_end_y        = buf_addr.in_start_y + end_pos_y;
			break;
		}
		default:
		{
			printk(KERN_ERR "%s::not matched pp_instance->src_color_space(%d)\n",__FUNCTION__, pp_instance->src_color_space);
			return;
			break;
		}
	}

	set_src_next_addr_register(&buf_addr, pp_instance);	
}

static void get_pre_h_ratio(s3c_pp_instance_context_t *pp_instance, unsigned int *pre_h_ratio, unsigned int *h_shift)
{
	if((pp_instance->dst_width << 1) <= pp_instance->src_width)
	{
		*pre_h_ratio = 2;
		*h_shift = 1;

		if((pp_instance->dst_width << 2) <= pp_instance->src_width)
		{
			*pre_h_ratio = 4;
			*h_shift = 2;

			if((pp_instance->dst_width << 3) <= pp_instance->src_width)
			{
				*pre_h_ratio = 8;
				*h_shift = 3;		

				if((pp_instance->dst_width << 4) <= pp_instance->src_width)
				{
					*pre_h_ratio = 16;
					*h_shift = 4;

					if((pp_instance->dst_width << 5) <= pp_instance->src_width)
					{
						*pre_h_ratio = 32;
						*h_shift = 5;		
					}
				}
			}
		}
	}
	else
	{
		*pre_h_ratio = 1;
		*h_shift = 0;		
	}
}

static void get_pre_v_ratio(s3c_pp_instance_context_t *pp_instance, unsigned int *pre_v_ratio, unsigned int *v_shift)
{
	if((pp_instance->dst_height << 1) <= pp_instance->src_height)
	{
		*pre_v_ratio = 2;
		*v_shift = 1;

		if((pp_instance->dst_height << 2) <= pp_instance->src_height)
		{
			*pre_v_ratio = 4;
			*v_shift = 2;

			if((pp_instance->dst_height << 3) <= pp_instance->src_height)
			{
				*pre_v_ratio = 8;
				*v_shift = 3;

				if((pp_instance->dst_height << 4) <= pp_instance->src_height)
				{
					*pre_v_ratio = 16;
					*v_shift = 4;

					if((pp_instance->dst_height << 5) <= pp_instance->src_height)
					{
						*pre_v_ratio = 32;
						*v_shift = 5;		
					}
				}
			}
		}
	}
	else
	{
		*pre_v_ratio = 1;
		*v_shift = 0;		
	}		
}

int parameters_calibration(s3c_pp_instance_context_t *pp_instance)
{
	unsigned int	pre_h_ratio, h_shift;
	unsigned int	pre_v_ratio, v_shift;
	unsigned int	calibration_src_width, calibration_src_height;
	
	
	if((pp_instance->dst_width << 6) <= pp_instance->src_width)
	{
		printk(KERN_ERR "out of horizontal scale range\n");
		return -EINVAL;
	}
	if((pp_instance->dst_height << 6) <= pp_instance->src_height)
	{
		printk(KERN_ERR "out of vertical scale range\n");
		return -EINVAL;
	}

	get_pre_h_ratio(pp_instance, &pre_h_ratio, &h_shift);
	get_pre_v_ratio(pp_instance, &pre_v_ratio, &v_shift);

	// Source width must be 4's multiple of PreScale_H_Ratio and source height must be 2's multiple of PreScale_V_Ratio
	calibration_src_width   = pp_instance->src_width  - ( pp_instance->src_width  & ((pre_h_ratio << 2) - 1) );
	calibration_src_height  = pp_instance->src_height - ( pp_instance->src_height & ((pre_v_ratio << 1) - 1) );

#if 0
	if(pp_instance->src_width  != calibration_src_width)
		printk("pp_instance->src_width(%d) != calibration_src_width(%d)\n", pp_instance->src_width, calibration_src_width);
	if(pp_instance->src_height != calibration_src_height)
		printk("pp_instance->src_height(%d) != calibration_src_height(%d)\n", pp_instance->src_height, calibration_src_height);
#endif

	pp_instance->src_width  = calibration_src_width;
	pp_instance->src_height = calibration_src_height;

	return 0;
}

// setting the scaling information(source/destination size)
void set_scaler(s3c_pp_instance_context_t *pp_instance)
{
	unsigned int		 pre_h_ratio, h_shift;
	unsigned int		 pre_v_ratio, v_shift;
	s3c_pp_scaler_info_t scaler_info;

	// horizontal(width)
	get_pre_h_ratio(pp_instance, &pre_h_ratio, &h_shift);
	scaler_info.pre_h_ratio = pre_h_ratio;
	scaler_info.h_shift     = h_shift;

	scaler_info.pre_dst_width = pp_instance->src_width / scaler_info.pre_h_ratio;
	scaler_info.dx = (pp_instance->src_width << 8) / (pp_instance->dst_width << scaler_info.h_shift);

	// vertical(height)
	get_pre_v_ratio(pp_instance, &pre_v_ratio, &v_shift);
	scaler_info.pre_v_ratio = pre_v_ratio;
	scaler_info.v_shift     = v_shift;
	
	scaler_info.pre_dst_height = pp_instance->src_height / scaler_info.pre_v_ratio;
	scaler_info.dy = (pp_instance->src_height << 8) / (pp_instance->dst_height << scaler_info.v_shift);

	//
	scaler_info.sh_factor = 10 - (scaler_info.h_shift + scaler_info.v_shift);

	// setting the register about scaling information
	set_scaler_register(&scaler_info, pp_instance);
}

int cal_data_size(s3c_color_space_t color_space, unsigned int width, unsigned int height)
{
	switch(color_space)
	{
		case RGB16:
			return (width * height << 1);
		case RGB24:
			return (width * height << 2);
		case YC420:
			return (width * height * 3) >> 1;
		case YC422:
			return (width * height << 1);
		case CRYCBY:
		case CBYCRY:
		case YCRYCB:
		case YCBYCR:
			return (width * height << 1);
		case YUV444:
			return (width * height << 2);		
		default:
			printk(KERN_ERR "Input parameter(color_space : %d) is wrong\n", color_space);
			return -EINVAL;
	}
}

int get_src_data_size(s3c_pp_instance_context_t *pp_instance)
{
	return cal_data_size ( pp_instance->src_color_space, pp_instance->src_full_width, pp_instance->src_full_height );
}

int get_dest_data_size(s3c_pp_instance_context_t *pp_instance)
{
	return cal_data_size ( pp_instance->dst_color_space, pp_instance->dst_full_width, pp_instance->dst_full_height );
}


