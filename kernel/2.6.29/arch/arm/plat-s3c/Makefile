# arch/arm/plat-s3c/Makefile
#
# Copyright 2008 Simtec Electronics
#
# Licensed under GPLv2

obj-y				:=
obj-m				:=
obj-n				:=
obj-				:=

# Core support for all Samsung SoCs

obj-y				+= init.o

ifdef CONFIG_NO_HZ 
obj-y				+= hr-time.o

else

ifndef CONFIG_HIGH_RES_TIMERS 
obj-y				+= time.o
else
obj-y                           += hr-time.o
endif

endif

obj-y				+= clock.o
obj-y				+= pwm-clock.o
obj-y				+= gpio.o
obj-y				+= gpio-config.o
obj-$(CONFIG_S3C_DMA_PL080)	+= dma-pl080.o
obj-$(CONFIG_S3C_DMA_PL330)	+= dma-pl330.o 

# devices
obj-$(CONFIG_S3C_DEV_HSMMC)	+= dev-hsmmc.o
obj-$(CONFIG_S3C_DEV_HSMMC1)	+= dev-hsmmc1.o
obj-$(CONFIG_S3C_DEV_HSMMC2)	+= dev-hsmmc2.o
obj-y				+= dev-i2c0.o
obj-$(CONFIG_S3C_DEV_I2C1)	+= dev-i2c1.o
obj-$(CONFIG_TOUCHSCREEN_S3C)	+= dev-ts.o
