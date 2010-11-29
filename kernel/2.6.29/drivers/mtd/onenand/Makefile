#
# Makefile for the OneNAND MTD
#

# Core functionality.
obj-$(CONFIG_MTD_ONENAND)		+= onenand.o

# Board specific.
obj-$(CONFIG_MTD_ONENAND_GENERIC)	+= generic.o
obj-$(CONFIG_MTD_ONENAND_OMAP2)		+= omap2.o

# Simulator
obj-$(CONFIG_MTD_ONENAND_SIM)		+= onenand_sim.o

ifeq ($(CONFIG_CPU_S3C6400),y)
onenand-objs = s3c_onenand.o
else ifeq ($(CONFIG_CPU_S3C6410),y)
onenand-objs = s3c_onenand.o
else ifeq ($(CONFIG_CPU_S5PC100),y)
onenand-objs = s3c_onenand.o
else
onenand-objs = onenand_base.o
endif
#onenand-objs = onenand_base.o onenand_bbt.o
