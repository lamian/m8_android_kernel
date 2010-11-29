#################################################
# Makefile for JPEG Driver
# 2007 (C) Samsung Electronics 
# Author : Jiun. Yu <jiun.yu@samsung.com>
#################################################
KBUILD_CFLAGS += -DDIVX_ENABLE

s3c_mfc-y 		:= Prism_S_V13C.o BitProcBuf.o DataBuf.o FramBufMgr.o \
			 LogMsg.o MFC_HW_Init.o MFC_Inst_Pool.o MFC_Instance.o MfcMemory.o MfcMutex.o MfcSfr.o 	\
			 s3c-mfc.o MfcIntrNotification.o MfcSetConfig.o

obj-$(CONFIG_VIDEO_MFC)		:= s3c_mfc.o 





