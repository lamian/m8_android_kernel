	/* drivers/input/keyboard/s3c-keypad.c
 *
 * Driver core for Samsung SoC onboard UARTs.
 *
 * Kim Kyoungil, Copyright (c) 2006-2009 Samsung Electronics
 *      http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>

#include <linux/io.h>
#include <mach/hardware.h>
#include <asm/delay.h>
#include <asm/irq.h>

#include <plat/regs-gpio.h>
#include <plat/regs-keypad.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <plat/gpio-cfg.h>
#include "s3c-keypad.h"
#include "s3c-keypad-board.h"

#ifdef CONFIG_CPU_FREQ 
#include <plat/s3c64xx-dvfs.h>
#endif

//#define S3C_KEYPAD_DEBUG 
#undef S3C_KEYPAD_DEBUG 

#ifdef S3C_KEYPAD_DEBUG
#define DPRINTK(x...) printk("S3C-Keypad " x)
#else
#define DPRINTK(x...)		/* !!!! */
#endif

#define DEVICE_NAME "s3c-keypad"

#define TRUE 1
#define FALSE 0

#define FIRST_SCAN_INTERVAL    	(1)
#define SCAN_INTERVAL    	(HZ/50)

static struct timer_list keypad_timer;
static int is_timer_on = FALSE;
static struct clk *keypad_clock;
static u32 prevmask_low = 0, prevmask_high = 0;

static int keypad_scan(u32 *keymask_low, u32 *keymask_high)
{
	u32 i,cval,rval;

	for (i=0; i<KEYPAD_COLUMNS; i++) {
		cval = KEYCOL_DMASK & ~((1 << i) | (1 << (i+ 8)));   // clear that column number and 
		writel(cval, key_base+S3C_KEYIFCOL);               // make that Normal output.
								   // others shuld be High-Z output.

		udelay(KEYPAD_DELAY);

		rval = ~(readl(key_base+S3C_KEYIFROW)) & ((1<<KEYPAD_ROWS)-1) ;
		
#if (KEYPAD_COLUMNS>4)	
		if (i < 4)
			*keymask_low |= (rval << (i * 8));
		else 
			*keymask_high |= (rval << ((i-4) * 8));
#else
		*keymask_low |= (rval << (i * 8));
#endif
	}

	writel(KEYIFCOL_CLEAR, key_base+S3C_KEYIFCOL);

	return 0;
}

static void process_input_report (struct s3c_keypad *s3c_keypad, u32 prevmask, u32 keymask, u32 index)
{
	struct input_dev             *dev = s3c_keypad->dev;
	int i=0;
	u32 press_mask = ((keymask ^ prevmask) & keymask); 
	u32 release_mask = ((keymask ^ prevmask) & prevmask); 

	i = 0;
	while (press_mask) {
		if (press_mask & 1) {
			input_report_key(dev, GET_KEYCODE(i+index),1);
			DPRINTK(": Pressed (index: %d, Keycode: %d)\n", i+index, GET_KEYCODE(i+index));
			}
		press_mask >>= 1;
		i++;
	}

	i = 0;
	while (release_mask) {
		if (release_mask & 1) {
			input_report_key(dev,GET_KEYCODE(i+index),0);
			DPRINTK(": Released (index: %d, Keycode: %d)\n", i+index, GET_KEYCODE(i+index));
			}
		release_mask >>= 1;
		i++;
	}
}

static inline void process_special_key (struct s3c_keypad *s3c_keypad, u32 keymask_low, u32 keymask_high)
{
	struct input_dev              *dev = s3c_keypad->dev;
	struct s3c_keypad_extra       *extra = s3c_keypad->extra;
	struct s3c_keypad_special_key *special_key = extra->special_key;
	static int prev_bitmask = 0;
	int i;

	for (i=0; i<extra->special_key_num; i++, special_key+=1)
	{
	        if (keymask_low == special_key->mask_low 
		    && keymask_high == special_key->mask_high 
		    && !(prev_bitmask & (1<<i))) {
        	        input_report_key(dev, special_key->keycode, 1);
			DPRINTK(": Pressed (Keycode: %d, SPECIAL KEY)\n", special_key->keycode);
			prev_bitmask |= (1<<i);
			continue;
		}
		if ((prev_bitmask & (1<<i)) 
 		    && keymask_low == 0 
	    	    && keymask_high == 0)
		{
	       	        input_report_key(dev, special_key->keycode, 0);
			DPRINTK(": Released (Keycode: %d, SPECIAL KEY)\n", special_key->keycode);
			prev_bitmask ^= (1<<i);
		}
	}
}

static void keypad_timer_handler(unsigned long data)
{
	struct s3c_keypad *s3c_keypad = (struct s3c_keypad *)data;
	u32 keymask_low = 0, keymask_high = 0;

	keypad_scan(&keymask_low, &keymask_high);

	process_special_key(s3c_keypad, keymask_low, keymask_high);

	if (keymask_low != prevmask_low) {
		process_input_report (s3c_keypad, prevmask_low, keymask_low, 0);
		prevmask_low = keymask_low;
	}
#if (KEYPAD_COLUMNS>4)
	if (keymask_high != prevmask_high) {
		process_input_report (s3c_keypad, prevmask_high, keymask_high, 32);
		prevmask_high = keymask_high;
	}
#endif

	if (keymask_low | keymask_high) {
		mod_timer(&keypad_timer,jiffies + SCAN_INTERVAL);
		is_timer_on = TRUE;
	} else {
		writel(KEYIFCON_INIT, key_base+S3C_KEYIFCON);
		is_timer_on = FALSE;
	}	
}

static irqreturn_t s3c_keypad_isr(int irq, void *dev_id)
{
#ifdef  CONFIG_CPU_FREQ
	set_dvfs_perf_level();
#endif
	/* disable keypad interrupt and schedule for keypad timer handler */
	writel(readl(key_base+S3C_KEYIFCON) & ~(INT_F_EN|INT_R_EN), key_base+S3C_KEYIFCON);

	keypad_timer.expires = jiffies + FIRST_SCAN_INTERVAL;
	if ( is_timer_on == FALSE) {
		add_timer(&keypad_timer);
		is_timer_on = TRUE;
	} else {
		mod_timer(&keypad_timer,keypad_timer.expires);
	}
	/*Clear the keypad interrupt status*/
	writel(KEYIFSTSCLR_CLEAR, key_base+S3C_KEYIFSTSCLR);

	return IRQ_HANDLED;
}

static irqreturn_t slide_int_handler(int irq, void *dev_id)
{
	struct s3c_keypad       *s3c_keypad = (struct s3c_keypad *) dev_id;
	struct s3c_keypad_slide *slide      = s3c_keypad->extra->slide;
#ifdef CONFIG_MACH_SMDK6410
	static int	state = 1;
#else
	int state;
#endif /* CONFIG_MACH_SMDK6410 */

#ifdef  CONFIG_CPU_FREQ
	set_dvfs_perf_level();
#endif /* CONFIG_CPU_FREQ */
#ifdef CONFIG_MACH_SMDK6410
	if (gpio_get_value(slide->gpio) != slide->state_upset)
		state ^= 1;
	else
		return IRQ_HANDLED;
#else
	state = gpio_get_value(slide->gpio) ^ slide->state_upset;
#endif /* CONFIG_MACH_SMDK6410 */
	DPRINTK(": changed Slide state (%d)\n", state);

	input_report_switch(s3c_keypad->dev, SW_LID, state);
	input_sync(s3c_keypad->dev);

	return IRQ_HANDLED;
}

static irqreturn_t gpio_int_handler(int irq, void *dev_id)
{
	struct s3c_keypad          *s3c_keypad = (struct s3c_keypad *) dev_id;
	struct input_dev           *dev = s3c_keypad->dev;
	struct s3c_keypad_extra    *extra = s3c_keypad->extra;
	struct s3c_keypad_gpio_key *gpio_key = extra->gpio_key;
	int i,state;

   	DPRINTK(": gpio interrupt (IRQ: %d)\n", irq);

#ifdef  CONFIG_CPU_FREQ
	set_dvfs_perf_level();
#endif
	for (i=0; i<extra->gpio_key_num; i++)
	{
		if (gpio_key[i].eint == irq)
		{
			gpio_key = &gpio_key[i];
			break;
		}
	}

	if (gpio_key != NULL)
	{
		state = gpio_get_value(gpio_key->gpio);
       	DPRINTK(": gpio state (%d, %d)\n", state , state ^ gpio_key->state_upset);
		state ^= gpio_key->state_upset;

        if(state) {
   	        input_report_key(dev, gpio_key->keycode, 1);
           	DPRINTK(": Pressed (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);
        }
       	else  {
			input_report_key(dev, gpio_key->keycode, 0);
			DPRINTK(": Released (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);
		}
	}
 
	return IRQ_HANDLED;
}

static int __init s3c_keypad_probe(struct platform_device *pdev)
{
	struct resource *res, *keypad_mem, *keypad_irq = NULL;
	struct input_dev *input_dev;
	struct s3c_keypad *s3c_keypad;
	int ret, size, key;
	struct s3c_keypad_extra    	*extra = NULL;
	struct s3c_keypad_slide    	*slide = NULL;
	struct s3c_keypad_special_key    *special_key;
	struct s3c_keypad_gpio_key 	*gpio_key;
	int i;
	char * input_dev_name;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev,"no memory resource specified\n");
		return -ENOENT;
	}

	size = (res->end - res->start) + 1;

	keypad_mem = request_mem_region(res->start, size, pdev->name);
	if (keypad_mem == NULL) {
		dev_err(&pdev->dev, "failed to get memory region\n");
		ret = -ENOENT;
		goto err_req;
	}

	key_base = ioremap(res->start, size);
	if (key_base == NULL) {
		printk(KERN_ERR "Failed to remap register block\n");
		ret = -ENOMEM;
		goto err_map;
	}

	keypad_clock = clk_get(&pdev->dev, "keypad");
	if (IS_ERR(keypad_clock)) {
		dev_err(&pdev->dev, "failed to find keypad clock source\n");
		ret = PTR_ERR(keypad_clock);
		goto err_clk;
	}

	clk_enable(keypad_clock);
	
	s3c_keypad = kzalloc(sizeof(struct s3c_keypad), GFP_KERNEL);
	input_dev = input_allocate_device();
	input_dev_name = (char *)kmalloc(sizeof("s3c-keypad-revxxxx"), GFP_KERNEL);

	if (!s3c_keypad || !input_dev || !input_dev_name) {
		ret = -ENOMEM;
		goto out;
	}

	platform_set_drvdata(pdev, s3c_keypad);

	DPRINTK(": system_rev 0x%04x\n", system_rev);
	for (i=0; i<sizeof(s3c_keypad_extra)/sizeof(struct s3c_keypad_extra); i++)
	{
//		if (s3c_keypad_extra[i].board_num == g_board_num) {
		if (s3c_keypad_extra[i].board_num == system_rev) {
			extra = &s3c_keypad_extra[i];
			sprintf(input_dev_name, "%s%s%04x", DEVICE_NAME, "-rev", system_rev); 
			DPRINTK(": board rev 0x%04x is detected!\n", s3c_keypad_extra[i].board_num);
			break;
		}
	}

	if(!extra) {
		extra = &s3c_keypad_extra[0];
		sprintf(input_dev_name, "%s%s", DEVICE_NAME, "-rev0000"); 			//default revison 
		DPRINTK(": failed to detect board rev. set default rev00\n");
	}
	DPRINTK(": input device name: %s.\n", input_dev_name);

	s3c_keypad->dev = input_dev;
	s3c_keypad->extra = extra;
	slide = extra->slide;
	special_key = extra->special_key;
	gpio_key = extra->gpio_key;
	
	writel(KEYIFCON_INIT, key_base+S3C_KEYIFCON);
	writel(KEYIFFC_DIV, key_base+S3C_KEYIFFC);

	/* Set GPIO Port for keypad mode and pull-up disable*/
	s3c_setup_keypad_cfg_gpio(KEYPAD_ROWS, KEYPAD_COLUMNS);

	writel(KEYIFCOL_CLEAR, key_base+S3C_KEYIFCOL);

	for(key = 0; key < 64; key++){
        	input_set_capability(input_dev, EV_KEY, key+1);
	}

	for (i=0; i<extra->special_key_num; i++ ){
        	input_set_capability(input_dev, EV_KEY, (special_key+i)->keycode);
	}

	for (i=0; i<extra->gpio_key_num; i++ ){
        	input_set_capability(input_dev, EV_KEY, (gpio_key+i)->keycode);
	}

	if (extra->slide != NULL)
        	input_set_capability(input_dev, EV_SW, SW_LID);

	input_dev->name = input_dev_name;
	input_dev->phys = "s3c-keypad/input0";
	
	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0001;

	/* Scan timer init */
	init_timer(&keypad_timer);
	keypad_timer.function = keypad_timer_handler;
	keypad_timer.data = (unsigned long)s3c_keypad;

	/* For IRQ_KEYPAD */
	keypad_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (keypad_irq == NULL) {
		dev_err(&pdev->dev, "no irq resource specified\n");
		ret = -ENOENT;
		goto err_clk;
	}

	if (slide != NULL)
	{

		s3c_gpio_cfgpin(slide->gpio, S3C_GPIO_SFN(slide->gpio_af));
		s3c_gpio_setpull(slide->gpio, S3C_GPIO_PULL_NONE);

		set_irq_type(slide->eint, IRQ_TYPE_EDGE_BOTH);

		ret = request_irq(slide->eint, slide_int_handler, IRQF_DISABLED,
		    "s3c_keypad gpio key", (void *)s3c_keypad);
		if (ret) {
			printk(KERN_ERR "request_irq(%d) failed (IRQ for SLIDE) !!!\n", slide->eint);
			ret = -EIO;
			goto err_irq;
		}
	}

	for (i=0; i<extra->gpio_key_num; i++, gpio_key+=1)
	{
		s3c_gpio_cfgpin(gpio_key->gpio, S3C_GPIO_SFN(gpio_key->gpio_af));
		s3c_gpio_setpull(gpio_key->gpio, S3C_GPIO_PULL_NONE);

		set_irq_type(gpio_key->eint, IRQ_TYPE_EDGE_BOTH);

		ret = request_irq(gpio_key->eint, gpio_int_handler, IRQF_DISABLED,
			    "s3c_keypad gpio key", (void *)s3c_keypad);
		if (ret) {
			printk(KERN_ERR "request_irq(%d) failed (IRQ for GPIO KEY) !!!\n", gpio_key->eint);
			ret = -EIO;
			goto err_irq;
		}
	}

	ret = request_irq(keypad_irq->start, s3c_keypad_isr, IRQF_SAMPLE_RANDOM,
		DEVICE_NAME, (void *) pdev);
	if (ret) {
		printk("request_irq failed (IRQ_KEYPAD) !!!\n");
		ret = -EIO;
		goto err_irq;
	}

	ret = input_register_device(input_dev);
	if (ret) {
		printk("Unable to register s3c-keypad input device!!!\n");
		goto out;
	}

	keypad_timer.expires = jiffies + (HZ/10);

	if (is_timer_on == FALSE) {
		add_timer(&keypad_timer);
		is_timer_on = TRUE;
	} else {
		mod_timer(&keypad_timer,keypad_timer.expires);
	}

	printk( DEVICE_NAME " Initialized\n");
	return 0;

out:
	input_free_device(input_dev);
	kfree(s3c_keypad);

err_irq:
	free_irq(keypad_irq->start, input_dev);
	free_irq(keypad_irq->end, input_dev);
	
	if (slide != NULL)
		free_irq(extra->slide->eint, s3c_keypad);

	gpio_key = extra->gpio_key;
	for (i=0; i<extra->gpio_key_num; i++, gpio_key+=1)
		free_irq(gpio_key->eint, s3c_keypad);
	
err_clk:
	clk_disable(keypad_clock);
	clk_put(keypad_clock);

err_map:
	iounmap(key_base);

err_req:
	release_resource(keypad_mem);
	kfree(keypad_mem);

	return ret;
}

static int s3c_keypad_remove(struct platform_device *pdev)
{
	struct s3c_keypad *s3c_keypad = platform_get_drvdata(pdev);
	struct input_dev  *dev        = s3c_keypad->dev;
	struct s3c_keypad_extra *extra = s3c_keypad->extra;
	struct s3c_keypad_slide *slide = extra->slide;
	struct s3c_keypad_gpio_key *gpio_key = extra->gpio_key;

	writel(KEYIFCON_CLEAR, key_base+S3C_KEYIFCON);

	if(keypad_clock) {
		clk_disable(keypad_clock);
		clk_put(keypad_clock);
		keypad_clock = NULL;
	}

	free_irq(IRQ_KEYPAD, (void *) s3c_keypad);

	del_timer(&keypad_timer);	

	if (slide)
	        free_irq(slide->eint, (void *) s3c_keypad);

	if (gpio_key)
	{
		int i;
		for (i=0; i<extra->gpio_key_num; i++, gpio_key+=1)
        		free_irq(gpio_key->eint, (void *) s3c_keypad);
	}

	input_unregister_device(dev);
	iounmap(key_base);
	kfree(s3c_keypad);

	printk(DEVICE_NAME " Removed.\n");
	return 0;
}

#ifdef CONFIG_PM
#include <plat/pm.h>

static struct sleep_save s3c_keypad_save[] = {
	SAVE_ITEM(KEYPAD_ROW_GPIOCON),
	SAVE_ITEM(KEYPAD_COL_GPIOCON),
	SAVE_ITEM(KEYPAD_ROW_GPIOPUD),
	SAVE_ITEM(KEYPAD_COL_GPIOPUD),
};

static int s3c_keypad_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct s3c_keypad *s3c_keypad = platform_get_drvdata(pdev);
	struct s3c_keypad_extra *extra = s3c_keypad->extra;
	struct s3c_keypad_slide *slide = extra->slide;
	struct s3c_keypad_gpio_key *gpio_key = extra->gpio_key;

	writel(KEYIFCON_CLEAR, key_base+S3C_KEYIFCON);

	s3c6410_pm_do_save(s3c_keypad_save, ARRAY_SIZE(s3c_keypad_save));
	
	if (!extra->wakeup_by_keypad)
	{
		writel(~(0x0fffffff), KEYPAD_ROW_GPIOCON);
		writel(~(0xffffffff), KEYPAD_COL_GPIOCON);
	}
	
	disable_irq(IRQ_KEYPAD);

	if (slide)
		disable_irq(slide->eint);

	if (gpio_key)
	{
		int i;
		for (i=0; i<extra->gpio_key_num; i++, gpio_key+=1)
        		disable_irq(gpio_key->eint);
	}

	clk_disable(keypad_clock);

	return 0;
}

static int s3c_keypad_resume(struct platform_device *pdev)
{
	struct s3c_keypad *s3c_keypad = platform_get_drvdata(pdev);
	struct s3c_keypad_extra *extra = s3c_keypad->extra;
	struct s3c_keypad_slide *slide = extra->slide;
	struct s3c_keypad_gpio_key *gpio_key = extra->gpio_key;

	s3c6410_pm_do_restore(s3c_keypad_save, ARRAY_SIZE(s3c_keypad_save));

	clk_enable(keypad_clock);

	if (is_timer_on)
		del_timer (&keypad_timer);

	is_timer_on = FALSE;
	prevmask_low = 0;
	prevmask_high = 0;
	
	keypad_timer_handler((unsigned long)s3c_keypad);
	if (is_timer_on)
	{
		/* disable keypad interrupt and schedule for keypad timer handler */
		writel(readl(key_base+S3C_KEYIFCON) & ~(INT_F_EN|INT_R_EN), key_base+S3C_KEYIFCON);
	}
	
	enable_irq(IRQ_KEYPAD);

	if (slide)
	{
		slide_int_handler (slide->eint, (void *) s3c_keypad);
		enable_irq(slide->eint);
	}

	if (gpio_key)
	{
		int i;
		for (i=0; i<extra->gpio_key_num; i++, gpio_key+=1)
		{
			gpio_int_handler (gpio_key->eint, (void *) s3c_keypad);
        		enable_irq(gpio_key->eint);
		}
	}

	writel(KEYIFCON_INIT, key_base+S3C_KEYIFCON);
	writel(KEYIFFC_DIV, key_base+S3C_KEYIFFC);

	writel(KEYIFCOL_CLEAR, key_base+S3C_KEYIFCOL);

	return 0;
}
#else
#define s3c_keypad_suspend NULL
#define s3c_keypad_resume  NULL
#endif /* CONFIG_PM */

static struct platform_driver s3c_keypad_driver = {
	.probe		= s3c_keypad_probe,
	.remove		= s3c_keypad_remove,
	.suspend	= s3c_keypad_suspend,
	.resume		= s3c_keypad_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c-keypad",
	},
};

static int __init s3c_keypad_init(void)
{
	int ret;

	ret = platform_driver_register(&s3c_keypad_driver);
	
	if(!ret)
	   printk(KERN_INFO "S3C Keypad Driver\n");

	return ret;
}

static void __exit s3c_keypad_exit(void)
{
	platform_driver_unregister(&s3c_keypad_driver);
}

module_init(s3c_keypad_init);
module_exit(s3c_keypad_exit);

MODULE_AUTHOR("Samsung");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("KeyPad interface for Samsung S3C");
