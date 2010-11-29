
/*
 * linux/drivers/tvenc/s3c-tvenc.c
 *
 * Revision 1.0  
 *
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 *	    S3C TV Encoder driver 
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
#include <linux/videodev2.h>
#include <linux/version.h>
#include <mach/hardware.h>
#include <mach/map.h>
#include <plat/regs-tvenc.h>
#include <plat/regs-lcd.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-gpio.h>
//#include <plat/egpio.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include "s3c-tvenc.h"

#define PFX "s3c_tvenc"

static struct clk *tvenc_clock;
static struct clk *h_clk;
static int s3c_tvenc_irq = NO_IRQ;
static struct resource *s3c_tvenc_mem;
static void __iomem *base;
static wait_queue_head_t waitq;
static tv_out_params_t tv_param = {0,};

/* Backup SFR value */
static u32 backup_reg[2];


/* Structure that declares the access functions*/

static void s3c_tvenc_switch(tv_enc_switch_t sw)
{
	if(sw == OFF) {
		__raw_writel(__raw_readl(base + S3C_TVCTRL)
			&~ S3C_TVCTRL_ON, base + S3C_TVCTRL);
#if 0	// sangsu fix
		egpio_set_value(EGPIO_TVOUT_SEL, 0);
		gpio_set_value(GPIO_TV_EN, GPIO_LEVEL_LOW);  /* VT Disable */
#endif
	} else if(sw == ON) {
		__raw_writel(__raw_readl(base + S3C_TVCTRL)
			| S3C_TVCTRL_ON, base + S3C_TVCTRL);
#if 0	// sangsu fix
		egpio_set_value(EGPIO_TVOUT_SEL, 1);
		gpio_set_value(GPIO_TV_EN, GPIO_LEVEL_HIGH);  /* VT Enable */
#endif
	} else
		printk("Error func:%s line:%d\n", __FUNCTION__, __LINE__);
}

static void s3c_tvenc_set_image_size(u32 width, u32 height)
{
	__raw_writel(IIS_WIDTH(width)| IIS_HEIGHT(height), 
		base + S3C_INIMAGESIZE);
}

#if 0
static void s3c_tvenc_enable_macrovision(tv_standard_t tvmode, macro_pattern_t pattern)
{
	switch(pattern) {
	case AGC4L :
		break;
	case AGC2L :
		break;
	case N01 :
		break;
	case N02 :
		break;
	case P01 :
		break;
	case P02 :	
		break;
	default :
		break;
	}
}

static void s3c_tvenc_disable_macrovision(void)
{
	__raw_writel(__raw_readl(base + S3C_MACROVISION0) 
		&~0xff, base + S3C_MACROVISION0);
}
#endif

static void s3c_tvenc_set_tv_mode(tv_standard_t mode, tv_conn_type_t out)
{
	u32 signal_type = 0, output_type = 0;
	
	switch(mode) {
	case PAL_N :
		__raw_writel(VBP_VEFBPD_PAL|VBP_VOFBPD_PAL, 
			base + S3C_VBPORCH);
		__raw_writel(HBP_HSPW_PAL|HBP_HBPD_PAL, 
			base + S3C_HBPORCH);
		__raw_writel(HEO_DTO_PAL|HEO_HEOV_PAL, 
			base + S3C_HENHOFFSET);
		__raw_writel(EPC_PED_ON, 
			base + S3C_PEDCTRL);
		__raw_writel(YFB_YBW_26|YFB_CBW_06,
			base + S3C_YCFILTERBW);
		__raw_writel(SSC_HSYNC_PAL, 
			base + S3C_SYNCSIZECTRL);
		__raw_writel(BSC_BEND_PAL|BSC_BSTART_PAL, 
			base + S3C_BURSTCTRL);
		__raw_writel(MBS_BSTART_PAL, 
			base + S3C_MACROBURSTCTRL);
		__raw_writel(AVP_AVEND_PAL|AVP_AVSTART_PAL, 
			base + S3C_ACTVIDPOCTRL);
		break;
	case PAL_NC :
	case PAL_BGHID :
		__raw_writel(VBP_VEFBPD_PAL|VBP_VOFBPD_PAL, 
			base + S3C_VBPORCH);
		__raw_writel(HBP_HSPW_PAL|HBP_HBPD_PAL, 
			base + S3C_HBPORCH);
		__raw_writel(HEO_DTO_PAL|HEO_HEOV_PAL, 
			base + S3C_HENHOFFSET);
		__raw_writel(EPC_PED_OFF, 
			base + S3C_PEDCTRL);
		__raw_writel(YFB_YBW_26|YFB_CBW_06,
			base + S3C_YCFILTERBW);
		__raw_writel(SSC_HSYNC_PAL, 
			base + S3C_SYNCSIZECTRL);
		__raw_writel(BSC_BEND_PAL|BSC_BSTART_PAL, 
			base + S3C_BURSTCTRL);
		__raw_writel(MBS_BSTART_PAL, 
			base + S3C_MACROBURSTCTRL);
		__raw_writel(AVP_AVEND_PAL|AVP_AVSTART_PAL, 
			base + S3C_ACTVIDPOCTRL);
		break;
	case NTSC_443 :
		__raw_writel(VBP_VEFBPD_NTSC|VBP_VOFBPD_NTSC, 
			base + S3C_VBPORCH);
		__raw_writel(HBP_HSPW_NTSC|HBP_HBPD_NTSC, 
			base + S3C_HBPORCH);
		__raw_writel(HEO_DTO_NTSC|HEO_HEOV_NTSC, 
			base + S3C_HENHOFFSET);
		__raw_writel(EPC_PED_ON, 
			base + S3C_PEDCTRL);
		__raw_writel(YFB_YBW_26|YFB_CBW_06, 
			base + S3C_YCFILTERBW);
		__raw_writel(SSC_HSYNC_NTSC, 
			base + S3C_SYNCSIZECTRL);
		__raw_writel(BSC_BEND_NTSC|BSC_BSTART_NTSC, 
			base + S3C_BURSTCTRL);
		__raw_writel(MBS_BSTART_NTSC, 
			base + S3C_MACROBURSTCTRL);
		__raw_writel(AVP_AVEND_NTSC|AVP_AVSTART_NTSC, 
			base + S3C_ACTVIDPOCTRL);
		break;
	case NTSC_J :
		__raw_writel(VBP_VEFBPD_NTSC|VBP_VOFBPD_NTSC, 
			base + S3C_VBPORCH);
		__raw_writel(HBP_HSPW_NTSC|HBP_HBPD_NTSC, 
			base + S3C_HBPORCH);
		__raw_writel(HEO_DTO_NTSC|HEO_HEOV_NTSC, 
			base + S3C_HENHOFFSET);
		__raw_writel(EPC_PED_OFF, 
			base + S3C_PEDCTRL);
		__raw_writel(YFB_YBW_21|YFB_CBW_06, 
			base + S3C_YCFILTERBW);
		__raw_writel(SSC_HSYNC_NTSC, 
			base + S3C_SYNCSIZECTRL);
		__raw_writel(BSC_BEND_NTSC|BSC_BSTART_NTSC, 
			base + S3C_BURSTCTRL);
		__raw_writel(MBS_BSTART_NTSC, 
			base + S3C_MACROBURSTCTRL);
		__raw_writel(AVP_AVEND_NTSC|AVP_AVSTART_NTSC, 
			base + S3C_ACTVIDPOCTRL);
		break;
	case PAL_M 	:	
	case NTSC_M	:
	default :			
		__raw_writel(VBP_VEFBPD_NTSC|VBP_VOFBPD_NTSC, 
			base + S3C_VBPORCH);
		__raw_writel(HBP_HSPW_NTSC|HBP_HBPD_NTSC, 
			base + S3C_HBPORCH);
		__raw_writel(HEO_DTO_NTSC|HEO_HEOV_NTSC, 
			base + S3C_HENHOFFSET);
		__raw_writel(EPC_PED_ON, 
			base + S3C_PEDCTRL);
		__raw_writel(YFB_YBW_21|YFB_CBW_06, 
			base + S3C_YCFILTERBW);
		__raw_writel(SSC_HSYNC_NTSC, 
			base + S3C_SYNCSIZECTRL);
		__raw_writel(BSC_BEND_NTSC|BSC_BSTART_NTSC, 
			base + S3C_BURSTCTRL);
		__raw_writel(MBS_BSTART_NTSC, 
			base + S3C_MACROBURSTCTRL);
		__raw_writel(AVP_AVEND_NTSC|AVP_AVSTART_NTSC, 
			base + S3C_ACTVIDPOCTRL);
		break;			
	}

	if(out == S_VIDEO) {
		__raw_writel(YFB_YBW_60|YFB_CBW_06, 
			base + S3C_YCFILTERBW);
		output_type = S3C_TVCTRL_OUTTYPE_S;
	} else
		output_type = S3C_TVCTRL_OUTTYPE_C;

	switch(mode) {
	case NTSC_M :
		signal_type = S3C_TVCTRL_OUTFMT_NTSC_M;
		break;
	case NTSC_J :
		signal_type = S3C_TVCTRL_OUTFMT_NTSC_J;
		break;
	case PAL_BGHID :
		signal_type = S3C_TVCTRL_OUTFMT_PAL_BDG;
		break;
	case PAL_M :
		signal_type = S3C_TVCTRL_OUTFMT_PAL_M;
		break;
	case PAL_NC :
		signal_type = S3C_TVCTRL_OUTFMT_PAL_NC;
		break;
	default:
		printk("s3c_tvenc_set_tv_mode : No matching signal_type!\n");
		break;
	}

	__raw_writel((__raw_readl(base + S3C_TVCTRL)
		&~(0x1f<<4))| output_type | signal_type, 
		base + S3C_TVCTRL);

	__raw_writel(0x01, base + S3C_FSCAUXCTRL);
}

#if 0
static void s3c_tvenc_set_pedestal(tv_enc_switch_t sw)
{
	if(sw)
		__raw_writel(EPC_PED_ON, base + S3C_PEDCTRL);
	else
		__raw_writel(EPC_PED_OFF, base + S3C_PEDCTRL);
}

static void s3c_tvenc_set_sub_carrier_freq(u32 freq)
{
	__raw_writel(FSC_CTRL(freq), base + S3C_FSCCTRL);
}

static void s3c_tvenc_set_fsc_dto(u32 val)
{
	unsigned int temp;

	temp = (0x1<<31)|(val&0x7fffffff);
	__raw_writel(temp, base + S3C_FSCDTOMANCTRL);
}

static void s3c_tvenc_disable_fsc_dto(void)
{
	__raw_writel(__raw_readl(base + S3C_FSCDTOMANCTRL)&~(1<<31), 
		base + S3C_FSCDTOMANCTRL);
}

static void s3c_tvenc_set_bg(u32 soft_mix, u32 color, u32 lum_offset)
{
	unsigned int bg_color;
	switch(color) {
	case 0 :
		bg_color = BGC_BGCS_BLACK;
		break;
	case 1 :
		bg_color = BGC_BGCS_BLUE;
		break;
	case 2 :
		bg_color = BGC_BGCS_RED;
		break;
	case 3 :
		bg_color = BGC_BGCS_MAGENTA;
		break;
	case 4 :
		bg_color = BGC_BGCS_GREEN;
		break;
	case 5 :
		bg_color = BGC_BGCS_CYAN;
		break;
	case 6 :
		bg_color = BGC_BGCS_YELLOW;
		break;
	case 7 :
		bg_color = BGC_BGCS_WHITE;
		break;	
	}
	if(soft_mix)
		__raw_writel(BGC_SME_ENA|bg_color|BGC_BGYOFS(lum_offset), 
		base + S3C_BGCTRL);
	else
		__raw_writel(BGC_SME_DIS|bg_color|BGC_BGYOFS(lum_offset), 
		base + S3C_BGCTRL);
		
}

static void s3c_tvenc_set_bg_vav_hav(u32 hav_len, u32 vav_len, u32 hav_st, u32 vav_st)
{
	__raw_writel(BVH_BG_HL(hav_len)|BVH_BG_HS(hav_st)|BVH_BG_VL(vav_len)|BVH_BG_VS(vav_st), 
		base + S3C_BGHVAVCTRL);
}
#endif

static void s3c_tvenc_set_hue_phase(u32 phase_val)
{
	__raw_writel(HUE_CTRL(phase_val), 
		base + S3C_HUECTRL);
}

#if 0
static u32 s3c_tvenc_get_hue_phase(void)
{
	return __raw_readl(base + S3C_HUECTRL)&0xff;
}
#endif

static void s3c_tvenc_set_contrast(u32 contrast)
{
	u32 temp;

	temp = __raw_readl(base + S3C_CONTRABRIGHT);

	__raw_writel((temp &~0xff)|contrast, 
		base + S3C_CONTRABRIGHT);
}

#if 0
static u32 s3c_tvenc_get_contrast(void)
{
	return (__raw_readl(base + S3C_CONTRABRIGHT)&0xff);
}
#endif

static void s3c_tvenc_set_bright(u32 bright)
{
	u32 temp;

	temp = __raw_readl(base + S3C_CONTRABRIGHT);

	__raw_writel((temp &~(0xff<<16))| (bright<<16), 
		base + S3C_CONTRABRIGHT);
}

#if 0
static u32 s3c_tvenc_get_bright(void)
{
	return ((__raw_readl(base + S3C_CONTRABRIGHT)&(0xff<<16))>>16);
}


static void s3c_tvenc_set_cbgain(u32 cbgain)
{
	u32 temp;

	temp = __raw_readl(base + S3C_CBCRGAINCTRL);

	__raw_writel((temp &~0xff)|cbgain, 
		base + S3C_CBCRGAINCTRL);
}


static u32 s3c_tvenc_get_cbgain(void)
{
	return (__raw_readl(base + S3C_CBCRGAINCTRL)&0xff);
}

static void s3c_tvenc_set_crgain(u32 crgain)
{
	u32 temp;

	temp = __raw_readl(base + S3C_CBCRGAINCTRL);

	__raw_writel((temp &~(0xff<<16))| (crgain<<16), 
		base + S3C_CBCRGAINCTRL);
}

static u32 s3c_tvenc_get_crgain(void)
{
	return ((__raw_readl(base + S3C_CBCRGAINCTRL)&(0xff<<16))>>16);
}
#endif

static void s3c_tvenc_enable_gamma_control(tv_enc_switch_t enable)
{
	u32 temp;

	temp = __raw_readl(base + S3C_GAMMACTRL);
	if(enable == ON)
		temp |= (1<<12);
	else
		temp &= ~(1<<12);

	__raw_writel(temp, base + S3C_GAMMACTRL);
}

static void s3c_tvenc_set_gamma_gain(u32 ggain)
{
	u32 temp;

	temp = __raw_readl(base + S3C_GAMMACTRL);

	__raw_writel((temp &~(0x7<<8))| (ggain<<8), 
		base + S3C_GAMMACTRL);
}

#if 0
static u32 s3c_tvenc_get_gamma_gain(void)
{
	return ((__raw_readl(base + S3C_GAMMACTRL)&(0x7<<8))>>8);
}

static void s3c_tvenc_enable_mute_control(tv_enc_switch_t enable)
{
	u32 temp;

	temp = __raw_readl(base + S3C_GAMMACTRL);
	if(enable == ON)
		temp |= (1<<12);
	else
		temp &= ~(1<<12);

	__raw_writel(temp, base + S3C_GAMMACTRL);
}

static void s3c_tvenc_set_mute(u32 y, u32 cb, u32 cr)
{
	u32 temp;

	temp = __raw_readl(base + S3C_MUTECTRL);

	temp &=~(0xffffff<<8);
	temp |= (cr & 0xff)<<24;
	temp |= (cb & 0xff)<<16;
	temp |= (y & 0xff)<<8;

	__raw_writel(temp, base + S3C_MUTECTRL);
}

static void s3c_tvenc_get_mute(u32 *y, u32 *cb, u32 *cr)
{
	u32 temp;

	temp = __raw_readl(base + S3C_MUTECTRL);

	*y = (temp&(0xff<<8))>>8;	
	*cb = (temp&(0xff<<16))>>16;	
	*cr = (temp&(0xff<<24))>>24;	
}
#endif

static void s3c_tvenc_get_active_win_center(u32 *vert, u32 *horz)
{
	u32 temp;

	temp = __raw_readl(base + S3C_HENHOFFSET);

	*vert = (temp&(0x3f<<24))>>24;
	*horz = (temp&(0xff<<16))>>16;
}

static void s3c_tvenc_set_active_win_center(u32 vert, u32 horz)
{
	u32 temp;

	temp = __raw_readl(base + S3C_HENHOFFSET);

	temp &=~(0x3ffff<<16);
	temp |= (vert&0x3f)<<24;
	temp |= (horz&0xff)<<16;

	__raw_writel(temp, base + S3C_HENHOFFSET);
}

// LCD display controller configuration functions
static void s3c_lcd_set_output_path(lcd_local_output_t out)
{
#if 0	// peter for 2.6.21 kernel	
	s3c_fb_set_output_path(out);
#else	// peter for 2.6.24 kernel
	s3cfb_set_output_path(out);
#endif
}

static void s3c_lcd_set_clkval(u32 clkval)
{
#if 0	// peter for 2.6.21 kernel	
	s3c_fb_set_clkval(clkval);
#else	// peter for 2.6.24 kernel
	s3cfb_set_clock(clkval);
#endif
}

static void s3c_lcd_enable_rgbport(u32 on_off)
{
#if 0	// peter for 2.6.21 kernel	
	s3c_fb_enable_rgbport(on_off);
#else	// peter for 2.6.24 kernel
	s3cfb_enable_rgbport(on_off);
#endif
}

static void s3c_lcd_start(void)
{
#if 0	// peter for 2.6.21 kernel	
	s3c_fb_start_lcd();
#else	// peter for 2.6.24 kernel
	s3cfb_start_lcd();
#endif
}

static void s3c_lcd_stop(void)
{
#if 0	// peter for 2.6.21 kernel		
	s3c_fb_stop_lcd();
#else	// peter for 2.6.24 kernel
	s3cfb_stop_lcd();
#endif
}


static void s3c_lcd_set_config(void)
{
	backup_reg[0] = __raw_readl(S3C_VIDCON0);
	backup_reg[1] = __raw_readl(S3C_VIDCON2);

	s3c_lcd_set_output_path(LCD_TVRGB);
	tv_param.lcd_output_mode = LCD_TVRGB;
	
	s3c_lcd_set_clkval(4);
	s3c_lcd_enable_rgbport(1);	
}

static void s3c_lcd_exit_config(void)
{
	__raw_writel(backup_reg[0], S3C_VIDCON0);
	__raw_writel(backup_reg[1], S3C_VIDCON2);
	tv_param.lcd_output_mode = LCD_RGB;
}

static int scaler_test_start(void)
{
	tv_param.sp.DstFullWidth = 640;
	tv_param.sp.DstFullHeight= 480;
	tv_param.sp.DstCSpace = RGB16;
	
	s3c_tvscaler_config(&tv_param.sp);

	s3c_tvscaler_int_enable(1);

	s3c_tvscaler_start();

	return 0;
}

static int scaler_test_stop(void)
{
	s3c_tvscaler_int_disable();

	return 0;
}


static int tvout_start(void)
{
	u32 width, height;
	tv_standard_t type;
	tv_conn_type_t conn;

	tv_param.sp.DstFullWidth *= 2;		// For TV OUT
	
	width = tv_param.sp.DstFullWidth;
	height = tv_param.sp.DstFullHeight;
	type = tv_param.sig_type;
	conn = tv_param.connect;

	/* Set TV-SCALER parameter */
	switch(tv_param.v2.input->type) {
	case V4L2_INPUT_TYPE_FIFO:		// LCD FIFO-OUT
		tv_param.sp.Mode = FREE_RUN;
		tv_param.sp.DstCSpace = YCBYCR;
		/* Display controller setting */
		s3c_lcd_stop();
		s3c_lcd_set_config();
		break;
	case V4L2_INPUT_TYPE_MSDMA:		// MSDMA		
		tv_param.sp.Mode = FREE_RUN;		
		tv_param.sp.DstCSpace = YCBYCR;			
		break;
	default:
		return -EINVAL;
	}

	s3c_tvenc_set_tv_mode(type, conn);	
	s3c_tvenc_set_image_size(width, height);	
	s3c_tvenc_switch(ON);
	
	s3c_tvscaler_config(&tv_param.sp);	// for setting DstStartX/Y, DstWidth/Height
	s3c_tvscaler_set_interlace(1);
	if(tv_param.v2.input->type == V4L2_INPUT_TYPE_FIFO)
		s3c_tvscaler_int_disable();
	else
		s3c_tvscaler_int_enable(1);
	s3c_tvscaler_start();

	if(tv_param.v2.input->type == V4L2_INPUT_TYPE_FIFO)
		s3c_lcd_start();
	
	return 0;
}

static int tvout_stop(void)
{

	s3c_tvscaler_set_interlace(0);
	s3c_tvscaler_stop_freerun();
	s3c_tvscaler_int_disable();
	s3c_tvenc_switch(OFF);
	
	switch(tv_param.v2.input->type) {
	case V4L2_INPUT_TYPE_FIFO:	// LCD FIFO-OUT
		/* Display controller setting */
		s3c_lcd_stop();
		s3c_lcd_exit_config();
		s3c_lcd_start();
		break;
	default:
		break;
	}
	return 0;
}

/* ------------------------------------------ V4L2 SUPPORT ----------------------------------------------*/
/* ------------- In FIFO and MSDMA, v4l2_input supported by S3C TVENC controller ------------------*/
static struct v4l2_input tvenc_inputs[] = {
	{
		.index		= 0,
		.name		= "LCD FIFO_OUT",
		.type		= V4L2_INPUT_TYPE_FIFO,
		.audioset		= 1,
		.tuner		= 0, /* ignored */
		.std			= 0,
		.status		= 0,
	}, 
	{
		.index		= 1,
		.name		= "Memory input (MSDMA)",
		.type		= V4L2_INPUT_TYPE_MSDMA,
		.audioset		= 2,
		.tuner		= 0,
		.std			= 0,
		.status		= 0,
	} 
};

/* ------------ Out FIFO and MADMA, v4l2_output supported by S3C TVENC controller ----------------*/
static struct v4l2_output tvenc_outputs[] = {
	{
		.index		= 0,
		.name		= "TV-OUT",
		.type		= V4L2_OUTPUT_TYPE_ANALOG,
		.audioset		= 0,
		.modulator	= 0,
		.std			= V4L2_STD_PAL | V4L2_STD_NTSC_M,
	}, 
	{
		.index		= 1,
		.name		= "Memory output (MSDMA)",
		.type		= V4L2_OUTPUT_TYPE_MSDMA,
		.audioset		= 0,
		.modulator	= 0, 
		.std			= 0,
	}, 

};

const struct v4l2_fmtdesc tvenc_input_formats[] = {
	{
		.index    = 0,
		.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.description = "16 bpp RGB, le",
		.pixelformat   = V4L2_PIX_FMT_RGB565,
		.flags    = FORMAT_FLAGS_PACKED,
	},
	{
		.index    = 1,
		.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags    = FORMAT_FLAGS_PACKED,
		.description = "24 bpp RGB, le",
		.pixelformat   = V4L2_PIX_FMT_RGB24,
	},
	{
		.index     = 2,
		.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags     = FORMAT_FLAGS_PLANAR,
		.description = "4:2:2, planar, Y-Cb-Cr",
		.pixelformat = V4L2_PIX_FMT_YUV422P,

	},
	{
		.index    = 3,
		.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags    = FORMAT_FLAGS_PLANAR,
		.description     = "4:2:0, planar, Y-Cb-Cr",
		.pixelformat   = V4L2_PIX_FMT_YUV420,
	}
};


const struct v4l2_fmtdesc tvenc_output_formats[] = {
	{
		.index    = 0,
		.type     = V4L2_BUF_TYPE_VIDEO_OUTPUT,
		.description = "16 bpp RGB, le",
		.pixelformat   = V4L2_PIX_FMT_RGB565,
		.flags    = FORMAT_FLAGS_PACKED,
	},
	{
		.index    = 1,
		.type     = V4L2_BUF_TYPE_VIDEO_OUTPUT,
		.flags    = FORMAT_FLAGS_PACKED,
		.description = "24 bpp RGB, le",
		.pixelformat   = V4L2_PIX_FMT_RGB24,
	},
	{
		.index     = 2,
		.type      = V4L2_BUF_TYPE_VIDEO_OUTPUT,
		.flags     = FORMAT_FLAGS_PLANAR,
		.description = "4:2:2, planar, Y-Cb-Cr",
		.pixelformat = V4L2_PIX_FMT_YUV422P,

	},
	{
		.index    = 3,
		.type     = V4L2_BUF_TYPE_VIDEO_OUTPUT,
		.flags    = FORMAT_FLAGS_PLANAR,
		.description     = "4:2:0, planar, Y-Cb-Cr",
		.pixelformat   = V4L2_PIX_FMT_YUV420,
	}
};

const struct v4l2_standard tvout_standards[] = {
	{
		.index    = 0,
		.id     = V4L2_STD_NTSC_M,
		.name = "NTSC type",
	},
	{
		.index    = 1,
		.id     = V4L2_STD_PAL,
		.name = "PAL type",
	}	
};

#define NUMBER_OF_INPUT_FORMATS  	ARRAY_SIZE(tvenc_input_formats)
#define NUMBER_OF_OUTPUT_FORMATS  	ARRAY_SIZE(tvenc_output_formats)
#define NUMBER_OF_INPUTS	        ARRAY_SIZE(tvenc_inputs)
#define NUMBER_OF_OUTPUTS	        ARRAY_SIZE(tvenc_outputs)
#define NUMBER_OF_STANDARDS	        ARRAY_SIZE(tvout_standards)

static int s3c_tvenc_g_fmt(struct v4l2_format *f)
{
	int size = sizeof(struct v4l2_pix_format);

	memset(&f->fmt.pix, 0, size);
	memcpy(&f->fmt.pix, &tv_param.v2.pixfmt, size);

	return 0;	
}

static int s3c_tvenc_s_fmt(struct v4l2_format *f)
{	
	/* update our state informations */
	tv_param.v2.pixfmt= f->fmt.pix;

	// peter LCD output related operation
	if (tv_param.v2.pixfmt.pixelformat == V4L2_PIX_FMT_RGB565 ) {	
	
	tv_param.sp.SrcFullWidth = tv_param.v2.pixfmt.width;
	tv_param.sp.SrcFullHeight = tv_param.v2.pixfmt.height;
	tv_param.sp.SrcStartX = 0;
	tv_param.sp.SrcStartY = 0;	
	tv_param.sp.SrcWidth = tv_param.sp.SrcFullWidth;
	tv_param.sp.SrcHeight = tv_param.sp.SrcFullHeight;	
	
	printk("TV-OUT: LCD path operation set\n");

	// peter for padded data of mfc output		
	} else if (tv_param.v2.pixfmt.pixelformat == V4L2_PIX_FMT_YUV420) { 	
	
#ifdef DIVX_TEST		// padded output 	
	tv_param.sp.SrcFullWidth = tv_param.v2.pixfmt.width + 2*16;
	tv_param.sp.SrcFullHeight = tv_param.v2.pixfmt.height + 2*16;
	tv_param.sp.SrcStartX = 16;
	tv_param.sp.SrcStartY = 16;
	tv_param.sp.SrcWidth = tv_param.sp.SrcFullWidth - 2*tv_param.sp.SrcStartX;
	tv_param.sp.SrcHeight = tv_param.sp.SrcFullHeight - 2*tv_param.sp.SrcStartY;
#else	// not padded output 		
	tv_param.sp.SrcFullWidth = tv_param.v2.pixfmt.width;
	tv_param.sp.SrcFullHeight = tv_param.v2.pixfmt.height;
	tv_param.sp.SrcStartX = 0;
	tv_param.sp.SrcStartY = 0;
	tv_param.sp.SrcWidth = tv_param.sp.SrcFullWidth;
	tv_param.sp.SrcHeight = tv_param.sp.SrcFullHeight;
#endif	
	
	printk("TV-OUT: MFC path operation set\n");
	
	}	
	
	switch(tv_param.v2.pixfmt.pixelformat) {
	case V4L2_PIX_FMT_RGB565:
		tv_param.sp.SrcCSpace = RGB16;
		break;
	case V4L2_PIX_FMT_RGB24:
		tv_param.sp.SrcCSpace = RGB24;
		break;
	case V4L2_PIX_FMT_YUV420:
		tv_param.sp.SrcCSpace = YC420;
		break;
	case V4L2_PIX_FMT_YUV422P:
		tv_param.sp.SrcCSpace = YC422;
		break;
	default:
		return -EINVAL;
	}
	
//	camif_convert_into_camif_cfg_t(cfg, 1);
	return 0;
}

static int s3c_tvenc_s_input(int index)
{
	
	tv_param.v2.input = &tvenc_inputs[index];	
	switch(tv_param.v2.input->type) {
	case V4L2_INPUT_TYPE_FIFO:		// LCD FIFO-OUT
		tv_param.sp.InPath = POST_FIFO;
		break;
	case V4L2_INPUT_TYPE_MSDMA:		// MSDMA
		tv_param.sp.InPath = POST_DMA;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int s3c_tvenc_s_output(int index)
{
	tv_param.v2.output = &tvenc_outputs[index];
	switch(tv_param.v2.output->type) {
	case V4L2_OUTPUT_TYPE_ANALOG:	// TV-OUT (FIFO-OUT)
		tv_param.sp.OutPath = POST_FIFO;
		break;
	case V4L2_OUTPUT_TYPE_MSDMA:	// MSDMA
		tv_param.sp.OutPath = POST_DMA;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int s3c_tvenc_s_std(v4l2_std_id *id)
{
//	printk("s3c_tvenc_s_std: *id=0x%x",*id);
	switch(*id) {
	case V4L2_STD_NTSC_M:
		tv_param.sig_type = NTSC_M;		
		tv_param.sp.DstFullWidth = 720;		
		tv_param.sp.DstFullHeight = 480;					
		break;
	case V4L2_STD_PAL:
		tv_param.sig_type = PAL_M;
		tv_param.sp.DstFullWidth = 720;
		tv_param.sp.DstFullHeight = 576;		
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int s3c_tvenc_v4l2_control(struct v4l2_control *ctrl)
{
	switch(ctrl->id) {
		
	// peter added for MFC related op.
	case V4L2_CID_MPEG_STREAM_PID_VIDEO:
	{
		tv_param.sp.SrcFrmSt = ctrl->value;
		return 0;
	}
	
	case V4L2_CID_CONNECT_TYPE:
	{
		if(ctrl->value == 0) { // COMPOSITE
			tv_param.connect = COMPOSITE;
		} else if(ctrl->value == 1) {	//S-VIDEO
			tv_param.connect = S_VIDEO;
		} else {
			return -EINVAL;
		}
		return 0;
	}

	case V4L2_CID_BRIGHTNESS:
	{
		s32 val = ctrl->value;	
		if((val > 0xff)||(val < 0))
			return -EINVAL;
		else
			s3c_tvenc_set_bright(val);

		return 0;
	}

	case V4L2_CID_CONTRAST:
	{
		s32 val = ctrl->value;	
		if((val > 0xff)||(val < 0))
			return -EINVAL;
		else
			s3c_tvenc_set_contrast(val);

		return 0;
	}

	case V4L2_CID_GAMMA:
	{
		s32 val = ctrl->value;	
		if((val > 0x3)||(val < 0)) {
			return -EINVAL;
		} else {
			s3c_tvenc_enable_gamma_control(ON);
			s3c_tvenc_set_gamma_gain(val);
			s3c_tvenc_enable_gamma_control(OFF);
		}
		return 0;
	}		

	case V4L2_CID_HUE:
	{
		s32 val = ctrl->value;	
		if((val > 0xff)||(val < 0))
			return -EINVAL;
		else
			s3c_tvenc_set_hue_phase(val);

		return 0;		
	}

	case V4L2_CID_HCENTER:
	{
		s32 val = ctrl->value;
		u32 curr_horz, curr_vert;
		
		if((val > 0xff)||(val < 0)) {
			return -EINVAL;
		} else {
			s3c_tvenc_get_active_win_center(&curr_vert, &curr_horz);
			s3c_tvenc_set_active_win_center(curr_vert, val);
		}

		return 0;				
	}

	case V4L2_CID_VCENTER:
	{
		s32 val = ctrl->value;
		u32 curr_horz, curr_vert;
		
		if((val > 0x3f)||(val < 0)) {
			return -EINVAL;
		} else {
			s3c_tvenc_get_active_win_center(&curr_vert, &curr_horz);
			s3c_tvenc_set_active_win_center(val, curr_horz);
		}

		return 0;				
	}
	
	default:
		return -EINVAL;
	}
	return 0;
}

int s3c_tvenc_open(struct inode *inode, struct file *filp) 
{ 
	int err;

	// 2008.03.27 added for tv-out debugging by hyunkyung
	//printk("[hk] TV-OUT: tvenc_open function is called\n");

	err = video_exclusive_open(inode, filp);	// One function of V4l2 driver

	if(err < 0) 
	{
		printk("TV-OUT: video_exclusive_open is failed\n");
		return err;
	}
	filp->private_data = &tv_param;

	s3c_tvscaler_init();
	
	/* Success */
	return 0;
}

int s3c_tvenc_release(struct inode *inode, struct file *filp) 
{
	video_exclusive_release(inode, filp);
	
	/* Success */
	return 0;
}

static int s3c_tvenc_do_ioctl(struct inode *inode,struct file *filp,unsigned int cmd,void *arg)
{
	int ret;
	
	switch(cmd){
	case VIDIOC_QUERYCAP:
		{
			struct v4l2_capability *cap = arg;
			strcpy(cap->driver, "S3C TV-OUT driver");
			strlcpy(cap->card, tv_param.v->name, sizeof(cap->card));
			sprintf(cap->bus_info, "ARM AHB BUS");
			cap->version = 0;
			cap->capabilities = V4L2_CAP_VIDEO_OUTPUT| V4L2_CAP_VIDEO_CAPTURE;
			return 0;
		}
	
	case VIDIOC_OVERLAY:
		{
			int on = *(int *)arg;
			
			printk("TV-OUT: VIDIOC_OVERLAY on:%d\n", on);
			if (on != 0) {
				ret = tvout_start();
			} else {
				ret = tvout_stop();
			}
			return ret;
		}	

	case VIDIOC_ENUMINPUT:
		{
			struct v4l2_input *i = arg;
			printk("TV-OUT: VIDIOC_ENUMINPUT : index = %d\n", i->index);
			
			if ((i->index) >= NUMBER_OF_INPUTS) {
				return -EINVAL;
			}
			memcpy(i, &tvenc_inputs[i->index], sizeof(struct v4l2_input));
			return 0;
		}

	case VIDIOC_S_INPUT:	// 0 -> LCD FIFO-OUT, 1 -> MSDMA
		{
			int index = *((int *)arg);
			printk("TV-OUT: VIDIOC_S_INPUT \n");
				
			if (index >= NUMBER_OF_INPUTS) {
				return -EINVAL;
			}
			else {
				s3c_tvenc_s_input(index);
				return 0;
			}
		}
	
	case VIDIOC_G_INPUT:
		{
			u32 *i = arg;
			printk("TV-OUT: VIDIOC_G_INPUT \n");
			*i = tv_param.v2.input->type;
			return 0;
		}	
	
	case VIDIOC_ENUMOUTPUT:
		{
			struct v4l2_output *i = arg;
			printk("TV-OUT: VIDIOC_ENUMOUTPUT : index = %d\n", i->index);
			
			if ((i->index) >= NUMBER_OF_OUTPUTS) {
				return -EINVAL;
			}
			memcpy(i, &tvenc_outputs[i->index], sizeof(struct v4l2_output));
			return 0;
		}	

	case VIDIOC_S_OUTPUT:	// 0 -> TV / FIFO , 1 -> MSDMA
		{
			int index = *((int *)arg);
			printk("TV-OUT: VIDIOC_S_OUTPUT \n");
		
			if (index >= NUMBER_OF_OUTPUTS) {
				return -EINVAL;
			}
			else {
				s3c_tvenc_s_output(index);
				return 0;
			}
		}

	case VIDIOC_G_OUTPUT:
		{
			u32 *i = arg;
			printk("VIDIOC_G_OUTPUT \n");
			*i = tv_param.v2.output->type;
			return 0;
		}

	case VIDIOC_ENUM_FMT:
		{	struct v4l2_fmtdesc *f = arg;
			enum v4l2_buf_type type = f->type;
			int index = f->index;

			printk("C: VIDIOC_ENUM_FMT : index = %d\n", index);
			if (index >= NUMBER_OF_INPUT_FORMATS) 
				return -EINVAL;
			
			switch (type) {
				case V4L2_BUF_TYPE_VIDEO_CAPTURE:
					break;
				case V4L2_BUF_TYPE_VIDEO_OUTPUT:
				default:
					return -EINVAL;
			}
			memset(f, 0, sizeof(*f));
			memcpy(f, tv_param.v2.fmtdesc+index, sizeof(*f));			
			return 0;
		}	
	
	case VIDIOC_G_FMT:
		{
			struct v4l2_format *f = arg;
			printk("C: VIDIOC_G_FMT \n");
			ret = s3c_tvenc_g_fmt(f);
			return ret;		
		}
	
	case VIDIOC_S_FMT:
		{	
			struct v4l2_format *f = arg;
			printk("C: VIDIOC_S_FMT \n");
			ret = s3c_tvenc_s_fmt(f);
			if(ret != 0) {
				printk("s3c_tvenc_set_fmt() failed !\n");
				return -EINVAL;
			}
			return ret;
		}
	
	case VIDIOC_S_CTRL:
		{
			struct v4l2_control *ctrl = arg;
			//printk("P: VIDIOC_S_CTRL \n");
			ret = s3c_tvenc_v4l2_control(ctrl);
			return ret;
		}

	case VIDIOC_ENUMSTD:
		{
			struct v4l2_standard *e = arg;
			unsigned int index = e->index;

			if (index >= NUMBER_OF_STANDARDS)
				return -EINVAL;
			v4l2_video_std_construct(e, tvout_standards[e->index].id,
						 &tvout_standards[e->index].name);
			e->index = index;
			return 0;
		}
	
	case VIDIOC_G_STD:
		{
			v4l2_std_id *id = arg;
			*id = tvout_standards[0].id;
			return 0;
		}
	
	case VIDIOC_S_STD:
		{
			v4l2_std_id *id = arg;
			unsigned int i;

			for (i = 0; i < NUMBER_OF_STANDARDS; i++) {
				//printk("P: *id = %d,  tvout_standards[i].id = %d\n", *id, tvout_standards[i].id);
				if (*id & tvout_standards[i].id)
					break;
			}
			if (i == NUMBER_OF_STANDARDS)
				return -EINVAL;

			ret = s3c_tvenc_s_std(id);
			return ret;
		}

	case VIDIOC_S_TVOUT_ON:
		{
			//int *SrcFrmSt = arg;
			//printk("---peter VIDIOC_S_TVOUT_ON : SrcFrmSt = 0x%08x\n", *SrcFrmSt);
			ret = tvout_start();
			return ret;
		}

	case VIDIOC_S_TVOUT_OFF:
		{
			ret = tvout_stop();
			return ret;
		}
	
	case VIDIOC_S_SCALER_TEST:
		{
			ret = scaler_test_start();
			mdelay(1);
			ret = scaler_test_stop();
			return ret;
		}

	default:
 		return -EINVAL;
	}
	return 0;
}

static int s3c_tvenc_ioctl_v4l2(struct inode *inode, struct file *filp, unsigned int cmd,
		    unsigned long arg)
{
	return video_usercopy(inode, filp, cmd, arg, s3c_tvenc_do_ioctl);
}

int s3c_tvenc_read(struct file *filp, char *buf, size_t count,
			loff_t *f_pos)
{
	return 0;
}

int s3c_tvenc_write(struct file *filp, const char *buf, size_t 
			count, loff_t *f_pos)
{
	return 0;
}

int s3c_tvenc_mmap(struct file *filp, struct vm_area_struct *vma)
{
	u32 size = vma->vm_end - vma->vm_start;
	u32 max_size;
	u32 page_frame_no;

	page_frame_no = __phys_to_pfn(POST_BUFF_BASE_ADDR);

	max_size = RESERVE_POST_MEM + PAGE_SIZE - (RESERVE_POST_MEM % PAGE_SIZE);

	if(size > max_size) {
		return -EINVAL;
	}

	vma->vm_flags |= VM_RESERVED;

	if( remap_pfn_range(vma, vma->vm_start, page_frame_no,
		size, vma->vm_page_prot)) {
		printk(KERN_ERR "%s: mmap_error\n", __FUNCTION__);
		return -EAGAIN;

	}

	return 0;
}

struct file_operations s3c_tvenc_fops = {
	.owner = THIS_MODULE,
	.open = s3c_tvenc_open,
	.ioctl = s3c_tvenc_ioctl_v4l2,
	.release = s3c_tvenc_release,
	.read = s3c_tvenc_read,
	.write = s3c_tvenc_write,
	.mmap = s3c_tvenc_mmap,
};

void s3c_tvenc_vdev_release (struct video_device *vdev) {
	kfree(vdev);
}

struct video_device tvencoder = {
	.name = "TVENCODER",
	.fops = &s3c_tvenc_fops,
	.release  = s3c_tvenc_vdev_release,
	.minor = TVENC_MINOR,
};

irqreturn_t s3c_tvenc_isr(int irq, void *dev_id)
{
	u32 mode;
printk("1");
	mode = __raw_readl(base + S3C_TVCTRL);

	// Clear FIFO under-run status pending bit
	mode |= (1<<12);

	__raw_writel(mode, base + S3C_TVCTRL);

	wake_up_interruptible(&waitq);
	return IRQ_HANDLED;
}

static int s3c_tvenc_probe(struct platform_device *pdev)
{
	struct resource *res;

        int ret;

	// 2008.03.27 added for tv-out debugging by hyunkyung
	//printk("[hk] entered the s3c_tvenc_probe function\n");

	/* find the IRQs */
	s3c_tvenc_irq = platform_get_irq(pdev, 0);
	if(s3c_tvenc_irq <= 0) {
		printk(KERN_ERR PFX "failed to get irq resouce\n");
                return -ENOENT;
	}

        /* get the memory region for the tv scaler driver */

        res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        if(res == NULL) {
                printk(KERN_ERR PFX "failed to get memory region resouce\n");
                return -ENOENT;
        }

	s3c_tvenc_mem = request_mem_region(res->start, res->end-res->start+1, pdev->name);
	if(s3c_tvenc_mem == NULL) {
		printk(KERN_ERR PFX "failed to reserve memory region\n");
                return -ENOENT;
	}


	base = ioremap(s3c_tvenc_mem->start, s3c_tvenc_mem->end - res->start + 1);
	if(s3c_tvenc_mem == NULL) {
		printk(KERN_ERR PFX "failed ioremap\n");
                return -ENOENT;
	}

	tvenc_clock = clk_get(&pdev->dev, "tv_encoder");
        if(tvenc_clock == NULL) {
                printk(KERN_ERR PFX "failed to find tvenc clock source\n");
                return -ENOENT;
        }

        clk_enable(tvenc_clock);

	h_clk = clk_get(&pdev->dev, "hclk");
        if(h_clk == NULL) {
                printk(KERN_ERR PFX "failed to find h_clk clock source\n");
                return -ENOENT;
        }

	init_waitqueue_head(&waitq);

	tv_param.v = video_device_alloc();
	if(!tv_param.v) {
		printk(KERN_ERR "s3c-tvenc: video_device_alloc() failed\n");
		return -ENOMEM;
	}
	memcpy(tv_param.v, &tvencoder, sizeof(tvencoder));
	if(video_register_device(tv_param.v, VFL_TYPE_GRABBER, TVENC_MINOR) != 0) {
		printk("s3c_camera_driver.c : Couldn't register this codec driver.\n");
		return 0;
	}

	ret = request_irq(s3c_tvenc_irq, s3c_tvenc_isr, IRQF_DISABLED,
			"TV_ENCODER", NULL);
	if (ret) {
		printk("request_irq(TV_ENCODER) failed.\n");
		return ret;
	}

#if 0 // sangsu fix
    /* GPIO_TV_EN Diable */
    if (gpio_is_valid(GPIO_TV_EN)) {
        if (gpio_request(GPIO_TV_EN, S3C_GPIO_LAVEL(GPIO_TV_EN)))
            printk(KERN_ERR "Failed to request GPIO_TV_EN!\n");
        gpio_direction_output(GPIO_TV_EN, GPIO_LEVEL_LOW);
    }
    s3c_gpio_setpull(GPIO_TV_EN, S3C_GPIO_PULL_NONE);
#endif	
	printk(" Success\n");
        return 0;
}

static int s3c_tvenc_remove(struct platform_device *dev)
{
	printk(KERN_INFO "s3c_tvenc_remove called !\n");
        clk_disable(tvenc_clock);
	free_irq(s3c_tvenc_irq, NULL);
	if (s3c_tvenc_mem != NULL) {
		pr_debug("s3-tvenc: releasing s3c_tvenc_mem\n");
		iounmap(base);
		release_resource(s3c_tvenc_mem);
		kfree(s3c_tvenc_mem);
	}
//	video_unregister_device(tv_param.v);
	return 0;
}

static int s3c_tvenc_suspend(struct platform_device *dev, pm_message_t state)
{
       clk_disable(tvenc_clock);
	return 0;
}

static int s3c_tvenc_resume(struct platform_device *pdev)
{
	clk_enable(tvenc_clock);
	return 0;
}

static struct platform_driver s3c_tvenc_driver = {
       .probe           	= s3c_tvenc_probe,
       .remove        	= s3c_tvenc_remove,
       .suspend       	= s3c_tvenc_suspend,
       .resume         = s3c_tvenc_resume,
       .driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c-tvenc",
	},
};

static char banner[] __initdata = KERN_INFO "S3C6410 TV encoder Driver, (c) 2008 Samsung Electronics\n";

static int __init  s3c_tvenc_init(void)
{

	printk(banner);
	
 	if(platform_driver_register(&s3c_tvenc_driver) != 0)
  	{
   		printk("Platform Device Register Failed \n");
   		return -1;
  	}
	
	printk(" S3C6410 TV encoder Driver module init OK. \n");
      	return 0;
}

static void __exit  s3c_tvenc_exit(void)
{

	video_unregister_device(tv_param.v);
       platform_driver_unregister(&s3c_tvenc_driver);
	   
 	printk("S3C6410 TV encoder Driver module exit. \n");
}


module_init(s3c_tvenc_init);
module_exit(s3c_tvenc_exit);


MODULE_AUTHOR("Peter, Oh");
MODULE_DESCRIPTION("S3C TV Encoder Device Driver");
MODULE_LICENSE("GPL");
