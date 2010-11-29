/*
 * linux/drivers/power/s3c6410_battery.h
 *
 * Battery measurement code for S3C6410 platform.
 *
 * Copyright (C) 2009 Samsung Electronics.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#define DRIVER_NAME	"smdk6410-battery"

/*
 * SMDK6410 board ADC channel
 */
typedef enum s3c_adc_channel {
	S3C_ADC_VOLTAGE = 0,
	S3C_ADC_TEMPERATURE,
	ENDOFADC
} adc_channel_type;

#define GPIO_LEVEL_LOW                  0
#define GPIO_LEVEL_HIGH                 1
#define GPIO_LEVEL_NONE                 2