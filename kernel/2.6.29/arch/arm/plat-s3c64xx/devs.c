/* linux/arch/arm/plat-s3c64xx/devs.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * Base S3C64XX resource and device definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/input.h>//add by hui 09-08-31
#include <linux/gpio_keys.h>//add by hui 09-08-31

#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <mach/hardware.h>
#include <mach/map.h>

#include <plat/devs.h>
#include <plat/adc.h>
#include <plat/spi.h>
#include <linux/android_pmem.h>
#include <plat/reserved_mem.h>

/* SMC9115 LAN via ROM interface */

static struct resource s3c_smc911x_resources[] = {
      [0] = {
              .start  = S3C64XX_PA_SMC9115,
              .end    = S3C64XX_PA_SMC9115 + S3C64XX_SZ_SMC9115 - 1,
              .flags  = IORESOURCE_MEM,
      },
	[1] = {
              .start = IRQ_EINT(10),
              .end   = IRQ_EINT(10),
              .flags = IORESOURCE_IRQ,
	},
};

struct platform_device s3c_device_smc911x = {
      .name           = "smc911x",
      .id             =  -1,
      .num_resources  = ARRAY_SIZE(s3c_smc911x_resources),
      .resource       = s3c_smc911x_resources,
};

/* NAND Controller */

static struct resource s3c_nand_resource[] = {
	[0] = {
		.start = S3C64XX_PA_NAND,
		.end   = S3C64XX_PA_NAND + S3C64XX_SZ_NAND - 1,
		.flags = IORESOURCE_MEM,
	}
};

struct platform_device s3c_device_nand = {
	.name		  = "s3c-nand",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_nand_resource),
	.resource	  = s3c_nand_resource,
};

EXPORT_SYMBOL(s3c_device_nand);

/* OneNAND Controller */
static struct resource s3c_onenand_resource[] = {
	[0] = {
		.start = S3C64XX_PA_ONENAND,
		.end   = S3C64XX_PA_ONENAND + S3C_SZ_ONENAND - 1,
		.flags = IORESOURCE_MEM,
	}
};

struct platform_device s3c_device_onenand = {
	.name		  = "onenand",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_onenand_resource),
	.resource	  = s3c_onenand_resource,
};

EXPORT_SYMBOL(s3c_device_onenand);

/* USB Host Controller */

static struct resource s3c_usb_resource[] = {
        [0] = {
                .start = S3C64XX_PA_USBHOST,
                .end   = S3C64XX_PA_USBHOST + S3C64XX_SZ_USBHOST - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_UHOST,
                .end   = IRQ_UHOST,
                .flags = IORESOURCE_IRQ,
        }
};

static u64 s3c_device_usb_dmamask = 0xffffffffUL;

struct platform_device s3c_device_usb = {
        .name             = "s3c2410-ohci",
        .id               = -1,
        .num_resources    = ARRAY_SIZE(s3c_usb_resource),
        .resource         = s3c_usb_resource,
        .dev              = {
                .dma_mask = &s3c_device_usb_dmamask,
                .coherent_dma_mask = 0xffffffffUL
        }
};

EXPORT_SYMBOL(s3c_device_usb);

/* USB Device (Gadget)*/

static struct resource s3c_usbgadget_resource[] = {
	[0] = {
		.start = S3C_PA_OTG,
		.end   = S3C_PA_OTG + S3C_SZ_OTG - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_OTG,
		.end   = IRQ_OTG,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device s3c_device_usbgadget = {
	.name		  = "s3c6410-usbgadget",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_usbgadget_resource),
	.resource	  = s3c_usbgadget_resource,
};

EXPORT_SYMBOL(s3c_device_usbgadget);

/* USB Device (OTG hcd)*/

static struct resource s3c_usb_otghcd_resource[] = {
	[0] = {
		.start = S3C_PA_OTG,
		.end   = S3C_PA_OTG + S3C_SZ_OTG - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_OTG,
		.end   = IRQ_OTG,
		.flags = IORESOURCE_IRQ,
	}
};

static u64 s3c_device_usb_otghcd_dmamask = 0xffffffffUL;

struct platform_device s3c_device_usb_otghcd = {
	.name		= "s3c_otghcd",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(s3c_usb_otghcd_resource),
	.resource	= s3c_usb_otghcd_resource,
        .dev              = {
                .dma_mask = &s3c_device_usb_otghcd_dmamask,
                .coherent_dma_mask = 0xffffffffUL
        }
};

EXPORT_SYMBOL(s3c_device_usb_otghcd);

/* LCD Controller */

static struct resource s3c_lcd_resource[] = {
	[0] = {
		.start = S3C64XX_PA_LCD,
		.end   = S3C64XX_PA_LCD + SZ_1M - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_LCD_VSYNC,
		.end   = IRQ_LCD_SYSTEM,
		.flags = IORESOURCE_IRQ,
	}
};

static u64 s3c_device_lcd_dmamask = 0xffffffffUL;

struct platform_device s3c_device_lcd = {
	.name		  = "s3c-lcd",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_lcd_resource),
	.resource	  = s3c_lcd_resource,
	.dev              = {
		.dma_mask		= &s3c_device_lcd_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};

/* FIMG-2D controller */
static struct resource s3c_g2d_resource[] = {
	[0] = {
		.start	= S3C64XX_PA_G2D,
		.end	= S3C64XX_PA_G2D + S3C64XX_SZ_G2D - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_2D,
		.end	= IRQ_2D,
		.flags	= IORESOURCE_IRQ,
	}
};

struct platform_device s3c_device_g2d = {
	.name		= "s3c-g2d",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(s3c_g2d_resource),
	.resource	= s3c_g2d_resource
};
EXPORT_SYMBOL(s3c_device_g2d);


/* FIMG-3D controller */
static struct resource s3c_g3d_resource[] = {
	[0] = {
		.start	= S3C64XX_PA_G3D,
		.end	= S3C64XX_PA_G3D + S3C64XX_SZ_G3D - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_S3C6410_G3D,
		.end	= IRQ_S3C6410_G3D,
		.flags	= IORESOURCE_IRQ,
	}
};

struct platform_device s3c_device_g3d = {
	.name		= "s3c-g3d",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(s3c_g3d_resource),
	.resource	= s3c_g3d_resource
};
EXPORT_SYMBOL(s3c_device_g3d);


/* VPP controller */
static struct resource s3c_vpp_resource[] = {                                                                         
        [0] = { 
            .start = S3C6400_PA_VPP,
            .end   = S3C6400_PA_VPP + S3C_SZ_VPP - 1, 
            .flags = IORESOURCE_MEM,
        },
        [1] = { 
               .start = IRQ_POST0, 
               .end   = IRQ_POST0,
	      .flags = IORESOURCE_IRQ,             
	 } 
};                                                                                                                    

struct platform_device s3c_device_vpp = {  
        .name             = "s3c-vpp",
	.id               = -1,
        .num_resources    = ARRAY_SIZE(s3c_vpp_resource),	
	.resource         = s3c_vpp_resource,                                                                         
};
EXPORT_SYMBOL(s3c_device_vpp);

/* TV encoder */
static struct resource s3c_tvenc_resource[] = {
	[0] = {
		.start = S3C6400_PA_TVENC,
		.end   = S3C6400_PA_TVENC + S3C_SZ_TVENC - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_TVENC,
		.end   = IRQ_TVENC,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device s3c_device_tvenc = {
	.name		  = "s3c-tvenc",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_tvenc_resource),
	.resource	  = s3c_tvenc_resource,
};

EXPORT_SYMBOL(s3c_device_tvenc);

/* MFC controller */
static struct resource s3c_mfc_resource[] = {
	[0] = {
		.start	= S3C6400_PA_MFC,
		.end	= S3C6400_PA_MFC + S3C_SZ_MFC - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_MFC,
		.end	= IRQ_MFC,
		.flags	= IORESOURCE_IRQ,
	}
};

struct platform_device s3c_device_mfc = {
	.name		= "s3c-mfc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(s3c_mfc_resource),
	.resource	= s3c_mfc_resource
};

EXPORT_SYMBOL(s3c_device_mfc);

/* TV scaler */
static struct resource s3c_tvscaler_resource[] = {
	[0] = {
		.start = S3C6400_PA_TVSCALER,
		.end   = S3C6400_PA_TVSCALER + S3C_SZ_TVSCALER - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SCALER,
		.end   = IRQ_SCALER,
		.flags = IORESOURCE_IRQ,
	}

};

/* rotator interface */
static struct resource s3c_rotator_resource[] = {
        [0] = {
                .start = S3C6400_PA_ROTATOR,
                .end   = S3C6400_PA_ROTATOR + S3C_SZ_ROTATOR - 1,
                .flags = IORESOURCE_MEM,
                },
        [1] = {
                .start = IRQ_ROTATOR,
                .end   = IRQ_ROTATOR,
                .flags = IORESOURCE_IRQ,
        }
};

struct platform_device s3c_device_rotator = {
        .name             = "s3c-rotator",
        .id               = -1,
        .num_resources    = ARRAY_SIZE(s3c_rotator_resource),
        .resource         = s3c_rotator_resource
};

EXPORT_SYMBOL(s3c_device_rotator);

/* JPEG controller  */
static struct resource s3c_jpeg_resource[] = {
        [0] = {
                .start = S3C6400_PA_JPEG,
                .end   = S3C6400_PA_JPEG + S3C_SZ_JPEG - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_JPEG,
                .end   = IRQ_JPEG,
                .flags = IORESOURCE_IRQ,
        }

};

struct platform_device s3c_device_jpeg = {
        .name             = "s3c-jpeg",
        .id               = -1,
        .num_resources    = ARRAY_SIZE(s3c_jpeg_resource),
        .resource         = s3c_jpeg_resource,
};

struct platform_device s3c_device_tvscaler = {
	.name		  = "s3c-tvscaler",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_tvscaler_resource),
	.resource	  = s3c_tvscaler_resource,
};

EXPORT_SYMBOL(s3c_device_tvscaler);


/* ADC */
static struct resource s3c_adc_resource[] = {
	[0] = {
		.start = S3C_PA_ADC,
		.end   = S3C_PA_ADC + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_PENDN,
		.end   = IRQ_PENDN,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start = IRQ_ADC,
		.end   = IRQ_ADC,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device s3c_device_adc = {
	.name		  = "s3c-adc",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_adc_resource),
	.resource	  = s3c_adc_resource,
};

void __init s3c_adc_set_platdata(struct s3c_adc_mach_info *pd)
{
	struct s3c_adc_mach_info *npd;

	npd = kmalloc(sizeof(*npd), GFP_KERNEL);
	if (npd) {
		memcpy(npd, pd, sizeof(*npd));
		s3c_device_adc.dev.platform_data = npd;
	} else {
		printk(KERN_ERR "no memory for ADC platform data\n");
	}
}
EXPORT_SYMBOL(s3c_device_adc);


static struct resource s3c_rtc_resource[] = {
	[0] = {
		.start = S3C_PA_RTC,
		.end   = S3C_PA_RTC + 0xff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_RTC_ALARM,
		.end   = IRQ_RTC_ALARM,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start = IRQ_RTC_TIC,
		.end   = IRQ_RTC_TIC,
		.flags = IORESOURCE_IRQ
	}
};

struct platform_device s3c_device_rtc = {
	.name		  = "s3c2410-rtc",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_rtc_resource),
	.resource	  = s3c_rtc_resource,
};

EXPORT_SYMBOL(s3c_device_rtc);


/* Keypad interface */
static struct resource s3c_keypad_resource[] = {
	[0] = {
		.start = S3C_PA_KEYPAD,
		.end   = S3C_PA_KEYPAD+ S3C_SZ_KEYPAD - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_KEYPAD,
		.end   = IRQ_KEYPAD,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device s3c_device_keypad = {
	.name		  = "s3c-keypad",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_keypad_resource),
	.resource	  = s3c_keypad_resource,
};
EXPORT_SYMBOL(s3c_device_keypad);

/* Watchdog */
/*
 * Meizu M8 GPIO Keys,add by meizu 09-08-31
 */

#define GPIO_MEIZU_KEY_HOME		148		//S3C64XX_GPN(4)
#define GPIO_MEIZU_KEY_VOL_UP 		146		//S3C64XX_GPN(2)
#define GPIO_MEIZU_KEY_VOL_DOWN	145		//S3C64XX_GPN(1)
#define GPIO_MEIZU_KEY_PLAY 		149		//S3C64XX_GPN(5)
#define GPIO_MEIZU_KEY_POWER		153		//S3C64XX_GPN(9)
/*
//state其实表示的就是press，1表按下，0表示抬起,先上报1，再上报0(按键动作完成，清除按键事件)，Linux规定的, modified by hui
*/
static struct gpio_keys_button meizu_m8_button_table[] = {
	{KEY_BACK,        		GPIO_MEIZU_KEY_HOME	,		1,	"Home button"	,	EV_KEY,	0,	20},
	{KEY_MENU,     		GPIO_MEIZU_KEY_POWER,    			1,	"Play button"	,	EV_KEY,	0,	20},
	{KEY_DOWN,   	GPIO_MEIZU_KEY_VOL_UP,   		1,	"Volume up"	,	EV_KEY,	0,	20},
	{KEY_HOME, 	GPIO_MEIZU_KEY_VOL_DOWN, 		1,	"Volume down",	EV_KEY,	0,	20},
	{KEY_LEFT,			GPIO_MEIZU_KEY_PLAY,			1,	"power",			EV_KEY,	1,	20}
};

static struct gpio_keys_platform_data meizum8_button_data = {
	.buttons  = meizu_m8_button_table,
	.nbuttons = ARRAY_SIZE(meizu_m8_button_table),
};

struct platform_device meizu_m8_buttons = {
	.name = "meizum8-buttons",
	.dev  = {
		.platform_data = &meizum8_button_data,
	},
	.id   = -1,
};
EXPORT_SYMBOL(meizu_m8_buttons);
/*
 * End Meizu M8 GPIO Keys
 */
static struct resource s3c_wdt_resource[] = {
	[0] = {
		.start = S3C64XX_PA_WATCHDOG,
		.end   = S3C64XX_PA_WATCHDOG + S3C64XX_SZ_WATCHDOG - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_WDT,
		.end   = IRQ_WDT,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device s3c_device_wdt = {
	.name             = "s3c2410-wdt",
	.id               = -1,
	.num_resources    = ARRAY_SIZE(s3c_wdt_resource),
	.resource         = s3c_wdt_resource,
};

EXPORT_SYMBOL(s3c_device_wdt);

/* AES */
static struct resource s3c_aes_resource[] = {
	[0] = {
		.start = S3C64XX_PA_AES,
		.end   = S3C64XX_PA_AES + S3C64XX_SZ_AES - 1,
		.flags = IORESOURCE_MEM,
	}
};

struct platform_device s3c_device_aes = {
	.name		  = "s3c-aes",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_aes_resource),
	.resource	  = s3c_aes_resource,
};

EXPORT_SYMBOL(s3c_device_aes);

/* Camif controller */

static struct resource s3c_camif_resource[] = {
	[0] = {
		.start = S3C64XX_PA_FIMC,
		.end   = S3C64XX_PA_FIMC + S3C64XX_SZ_FIMC - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_CAMIF_C,
		.end   = IRQ_CAMIF_C,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start = IRQ_CAMIF_P,
		.end   = IRQ_CAMIF_P,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device s3c_device_camif = {
	.name		= "s3c-camif",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(s3c_camif_resource),
	.resource	= s3c_camif_resource,
};

EXPORT_SYMBOL(s3c_device_camif);

static struct android_pmem_platform_data pmem_pdata = {
	.name		= "pmem",
	.no_allocator	= 1,
	.cached		= 1,
	.buffered	= 1,	//09.12.01 hoony: surfaceflinger optimize
};
 
static struct android_pmem_platform_data pmem_gpu1_pdata = {
	.name		= "pmem_gpu1",
	.no_allocator	= 0,
	.cached		= 1,
	.buffered	= 1,
};

static struct android_pmem_platform_data pmem_render_pdata = {
	.name		= "pmem_render",
	.no_allocator	= 1,
	.cached		= 0,
};

static struct android_pmem_platform_data pmem_stream_pdata = {
	.name		= "pmem_stream",
	.no_allocator	= 1,
	.cached		= 0,
};

static struct android_pmem_platform_data pmem_stream2_pdata = {
	.name		= "pmem_stream2",
	.no_allocator	= 1,
	.cached		= 0,
};

static struct android_pmem_platform_data pmem_preview_pdata = {
	.name		= "pmem_preview",
	.no_allocator	= 1,
	.cached		= 0,
};

static struct android_pmem_platform_data pmem_picture_pdata = {
	.name		= "pmem_picture",
	.no_allocator	= 1,
	.cached		= 0,
};

static struct android_pmem_platform_data pmem_jpeg_pdata = {
	.name		= "pmem_jpeg",
	.no_allocator	= 1,
	.cached		= 0,
};

static struct android_pmem_platform_data pmem_skia_pdata = {
	.name		= "pmem_skia",
	.no_allocator	= 1,
	.cached		= 0,
};
 
static struct platform_device pmem_device = {
	.name		= "android_pmem",
	.id		= 0,
	.dev		= { .platform_data = &pmem_pdata },
};
 
static struct platform_device pmem_gpu1_device = {
	.name		= "android_pmem",
	.id		= 1,
	.dev		= { .platform_data = &pmem_gpu1_pdata },
};

static struct platform_device pmem_render_device = {
	.name		= "android_pmem",
	.id		= 2,
	.dev		= { .platform_data = &pmem_render_pdata },
};

static struct platform_device pmem_stream_device = {
	.name		= "android_pmem",
	.id		= 3,
	.dev		= { .platform_data = &pmem_stream_pdata },
};

static struct platform_device pmem_stream2_device = {
	.name		= "android_pmem",
	.id		= 4,
	.dev		= { .platform_data = &pmem_stream2_pdata },
};

static struct platform_device pmem_preview_device = {
	.name		= "android_pmem",
	.id		= 5,
	.dev		= { .platform_data = &pmem_preview_pdata },
};

static struct platform_device pmem_picture_device = {
	.name		= "android_pmem",
	.id		= 6,
	.dev		= { .platform_data = &pmem_picture_pdata },
};

static struct platform_device pmem_jpeg_device = {
	.name		= "android_pmem",
	.id		= 7,
	.dev		= { .platform_data = &pmem_jpeg_pdata },
};

static struct platform_device pmem_skia_device = {
	.name		= "android_pmem",
	.id		= 8,
	.dev		= { .platform_data = &pmem_skia_pdata },
};

void __init s3c6410_add_mem_devices(struct s3c6410_pmem_setting *setting)
{
	if (setting->pmem_size) {
		pmem_pdata.start = setting->pmem_start;
		pmem_pdata.size = setting->pmem_size;
		platform_device_register(&pmem_device);
	}

	if (setting->pmem_gpu1_size) {
		pmem_gpu1_pdata.start = setting->pmem_gpu1_start;
		pmem_gpu1_pdata.size = setting->pmem_gpu1_size;
		platform_device_register(&pmem_gpu1_device);
	}
 
	if (setting->pmem_render_size) {
		pmem_render_pdata.start = setting->pmem_render_start;
		pmem_render_pdata.size = setting->pmem_render_size;
		platform_device_register(&pmem_render_device);
	}

	if (setting->pmem_stream_size) {
		pmem_stream_pdata.start = setting->pmem_stream_start;
	        pmem_stream_pdata.size = setting->pmem_stream_size;
	        platform_device_register(&pmem_stream_device);
	}

	if (setting->pmem_stream_size) {
		pmem_stream2_pdata.start = setting->pmem_stream_start;
		pmem_stream2_pdata.size = setting->pmem_stream_size;
		platform_device_register(&pmem_stream2_device);
	}

	if (setting->pmem_preview_size) {
		pmem_preview_pdata.start = setting->pmem_preview_start;
		pmem_preview_pdata.size = setting->pmem_preview_size;
		platform_device_register(&pmem_preview_device);
	}

	if (setting->pmem_picture_size) {
		pmem_picture_pdata.start = setting->pmem_picture_start;
		pmem_picture_pdata.size = setting->pmem_picture_size;
		platform_device_register(&pmem_picture_device);
	}

	if (setting->pmem_jpeg_size) {
		pmem_jpeg_pdata.start = setting->pmem_jpeg_start;
		pmem_jpeg_pdata.size = setting->pmem_jpeg_size;
		platform_device_register(&pmem_jpeg_device);
	}

	if (setting->pmem_skia_size) {
		pmem_skia_pdata.start = setting->pmem_skia_start;
		pmem_skia_pdata.size = setting->pmem_skia_size;
		platform_device_register(&pmem_skia_device);
	}
}

