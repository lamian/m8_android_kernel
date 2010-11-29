/* linux/arch/arm/plat-s3c/include/mach/dma-pl080.h
 *
 */

#ifndef __ARM_MACH_DMA_PL080_H
#define __ARM_MACH_DMA_PL080_H

/*
 * This is the maximum DMA address(physical address) that can be DMAd to.
 *
 */
#define MAX_DMA_ADDRESS			0x40000000
#define MAX_DMA_TRANSFER_SIZE   	0x100000 /* Data Unit is half word  */

#define DMACH_LOW_LEVEL			(1<<28)	/* use this to specifiy hardware ch no */

/* We have 4 dma controllers - DMA0, DMA1, SDMA0, SDMA1 */
#define S3C_DMA_CONTROLLERS        	(4)
#define S3C_CHANNELS_PER_DMA       	(8)
#define S3C_CANDIDATE_CHANNELS_PER_DMA  (16)
#define S3C_DMA_CHANNELS		(S3C_DMA_CONTROLLERS*S3C_CHANNELS_PER_DMA)

/* flags */
#define S3C2410_DMAF_SLOW         	(1<<0)   /* slow, so don't worry about */
#define S3C2410_DMAF_AUTOSTART    	(1<<1)   /* auto-start if buffer queued */

/* DMA Register definitions */
#define S3C2410_DCON_AUTORELOAD 	(0<<22)
#define S3C2410_DCON_NORELOAD   	(1<<22)

/*=================================================*/
/*   DMA Register Definitions for S3C6400          */

#define S3C_DMAC_INT_STATUS   		(0x00)
#define S3C_DMAC_INT_TCSTATUS   	(0x04)
#define S3C_DMAC_INT_TCCLEAR   		(0x08)
#define S3C_DMAC_INT_ERRORSTATUS   	(0x0c)
#define S3C_DMAC_INT_ERRORCLEAR   	(0x10)
#define S3C_DMAC_RAW_INTTCSTATUS   	(0x14)
#define S3C_DMAC_RAW_INTERRORSTATUS   	(0x18)
#define S3C_DMAC_ENBLD_CHANNELS	   	(0x1c)
#define S3C_DMAC_SOFTBREQ	   	(0x20)
#define S3C_DMAC_SOFTSREQ	   	(0x24)
#define S3C_DMAC_SOFTLBREQ	   	(0x28)
#define S3C_DMAC_SOFTLSREQ	   	(0x2c)
#define S3C_DMAC_CONFIGURATION   	(0x30)
#define S3C_DMAC_SYNC   		(0x34)

#define S3C_DMAC_CxSRCADDR   		(0x00)
#define S3C_DMAC_CxDESTADDR   		(0x04)
#define S3C_DMAC_CxLLI   		(0x08)
#define S3C_DMAC_CxCONTROL0   		(0x0C)
#define S3C_DMAC_CxCONTROL1   		(0x10)
#define S3C_DMAC_CxCONFIGURATION   	(0x14)

#define S3C_DMAC_C0SRCADDR   		(0x100)
#define S3C_DMAC_C0DESTADDR   		(0x104)
#define S3C_DMAC_C0LLI   		(0x108)
#define S3C_DMAC_C0CONTROL0   		(0x10C)
#define S3C_DMAC_C0CONTROL1   		(0x110)
#define S3C_DMAC_C0CONFIGURATION   	(0x114)

#define S3C_DMAC_C1SRCADDR   		(0x120)
#define S3C_DMAC_C1DESTADDR   		(0x124)
#define S3C_DMAC_C1LLI   		(0x128)
#define S3C_DMAC_C1CONTROL0   		(0x12C)
#define S3C_DMAC_C1CONTROL1   		(0x130)
#define S3C_DMAC_C1CONFIGURATION   	(0x134)

#define S3C_DMAC_C2SRCADDR   		(0x140)
#define S3C_DMAC_C2DESTADDR   		(0x144)
#define S3C_DMAC_C2LLI   		(0x148)
#define S3C_DMAC_C2CONTROL0   		(0x14C)
#define S3C_DMAC_C2CONTROL1   		(0x150)
#define S3C_DMAC_C2CONFIGURATION   	(0x154)

#define S3C_DMAC_C3SRCADDR   		(0x160)
#define S3C_DMAC_C3DESTADDR   		(0x164)
#define S3C_DMAC_C3LLI   		(0x168)
#define S3C_DMAC_C3CONTROL0   		(0x16C)
#define S3C_DMAC_C3CONTROL1   		(0x170)
#define S3C_DMAC_C3CONFIGURATION   	(0x174)

#define S3C_DMAC_C4SRCADDR   		(0x180)
#define S3C_DMAC_C4DESTADDR   		(0x184)
#define S3C_DMAC_C4LLI   		(0x188)
#define S3C_DMAC_C4CONTROL0   		(0x18C)
#define S3C_DMAC_C4CONTROL1   		(0x190)
#define S3C_DMAC_C4CONFIGURATION   	(0x194)

#define S3C_DMAC_C5SRCADDR   		(0x1A0)
#define S3C_DMAC_C5DESTADDR   		(0x1A4)
#define S3C_DMAC_C5LLI   		(0x1A8)
#define S3C_DMAC_C5CONTROL0   		(0x1AC)
#define S3C_DMAC_C5CONTROL1   		(0x1B0)
#define S3C_DMAC_C5CONFIGURATION   	(0x1B4)

#define S3C_DMAC_C6SRCADDR   		(0x1C0)
#define S3C_DMAC_C6DESTADDR   		(0x1C4)
#define S3C_DMAC_C6LLI   		(0x1C8)
#define S3C_DMAC_C6CONTROL0   		(0x1CC)
#define S3C_DMAC_C6CONTROL1   		(0x1D0)
#define S3C_DMAC_C6CONFIGURATION   	(0x1D4)

#define S3C_DMAC_C7SRCADDR   		(0x1E0)
#define S3C_DMAC_C7DESTADDR   		(0x1E4)
#define S3C_DMAC_C7LLI   		(0x1E8)
#define S3C_DMAC_C7CONTROL0   		(0x1EC)
#define S3C_DMAC_C7CONTROL1   		(0x1F0)
#define S3C_DMAC_C7CONFIGURATION   	(0x1F4)

/* DMACConfiguration(0x30) */
#define S3C_DMA_CONTROLLER_ENABLE 	(1<<0)

/* DMACCxControl0 : Channel control register 0 */
#define S3C_DMACONTROL_TC_INT_ENABLE 	(1<<31)
#define S3C_DMACONTROL_DEST_NO_INC	(0<<27)
#define S3C_DMACONTROL_DEST_INC		(1<<27)
#define S3C_DMACONTROL_SRC_NO_INC	(0<<26)
#define S3C_DMACONTROL_SRC_INC		(1<<26)
#define S3C_DMACONTROL_DEST_AXI_SPINE	(0<<25)
#define S3C_DMACONTROL_DEST_AXI_PERI	(1<<25)
#define S3C_DMACONTROL_SRC_AXI_SPINE	(0<<24)
#define S3C_DMACONTROL_SRC_AXI_PERI	(1<<24)
#define S3C_DMACONTROL_DEST_WIDTH_BYTE	(0<<21)
#define S3C_DMACONTROL_DEST_WIDTH_HWORD	(1<<21)
#define S3C_DMACONTROL_DEST_WIDTH_WORD	(2<<21)
#define S3C_DMACONTROL_SRC_WIDTH_BYTE	(0<<18)
#define S3C_DMACONTROL_SRC_WIDTH_HWORD	(1<<18)
#define S3C_DMACONTROL_SRC_WIDTH_WORD	(2<<18)

#define S3C_DMACONTROL_DBSIZE_1		(0<<15)
#define S3C_DMACONTROL_DBSIZE_4		(1<<15)
#define S3C_DMACONTROL_DBSIZE_8		(2<<15)
#define S3C_DMACONTROL_DBSIZE_16	(3<<15)
#define S3C_DMACONTROL_DBSIZE_32	(4<<15)
#define S3C_DMACONTROL_DBSIZE_64	(5<<15)
#define S3C_DMACONTROL_DBSIZE_128	(6<<15)
#define S3C_DMACONTROL_DBSIZE_256	(7<<15)

#define S3C_DMACONTROL_SBSIZE_1		(0<<12)
#define S3C_DMACONTROL_SBSIZE_4		(1<<12)
#define S3C_DMACONTROL_SBSIZE_8		(2<<12)
#define S3C_DMACONTROL_SBSIZE_16	(3<<12)
#define S3C_DMACONTROL_SBSIZE_32	(4<<12)
#define S3C_DMACONTROL_SBSIZE_64	(5<<12)
#define S3C_DMACONTROL_SBSIZE_128	(6<<12)
#define S3C_DMACONTROL_SBSIZE_256	(7<<12)


/* Channel configuration register, DMACCxConfiguration */
#define S3C_DMACONFIG_HALT		(1<<18) /*The contents of the channels FIFO are drained*/
#define S3C_DMACONFIG_ACTIVE		(1<<17) /*Check channel fifo has data or not*/
#define S3C_DMACONFIG_LOCK		(1<<16)
#define S3C_DMACONFIG_TCMASK	 	(1<<15) /*Terminal count interrupt mask*/
#define S3C_DMACONFIG_ERRORMASK	 	(1<<14) /*Interrup error mask*/
#define S3C_DMACONFIG_FLOWCTRL_MEM2MEM	(0<<11)
#define S3C_DMACONFIG_FLOWCTRL_MEM2PER	(1<<11)
#define S3C_DMACONFIG_FLOWCTRL_PER2MEM	(2<<11)
#define S3C_DMACONFIG_FLOWCTRL_PER2PER	(3<<11)
#define S3C_DMACONFIG_ONENANDMODEDST	(1<<10)	/* Reserved: OneNandModeDst */
#define S3C_DMACONFIG_DESTPERIPHERAL(x)	((x)<<6)
#define S3C_DMACONFIG_ONENANDMODESRC	(1<<5)	/* Reserved: OneNandModeSrc */
#define S3C_DMACONFIG_SRCPERIPHERAL(x)	((x)<<1)
#define S3C_DMACONFIG_CHANNEL_ENABLE	(1<<0)

#define S3C_DMA1			16

#define S3C_DEST_SHIFT 			6
#define S3C_SRC_SHIFT 			1


/* #define S3C_DMAC_CSRCADDR(ch)   	S3C_DMAC_C##ch##SRCADDR */
#define S3C_DMAC_CSRCADDR(ch)   	(S3C_DMAC_C0SRCADDR+ch*0x20)
#define S3C_DMAC_CDESTADDR(ch)   	(S3C_DMAC_C0DESTADDR+ch*0x20)
#define S3C_DMAC_CLLI(ch)   		(S3C_DMAC_C0LLI+ch*0x20)
#define S3C_DMAC_CCONTROL0(ch)   	(S3C_DMAC_C0CONTROL0+ch*0x20)
#define S3C_DMAC_CCONTROL1(ch)   	(S3C_DMAC_C0CONTROL1+ch*0x20)
#define S3C_DMAC_CCONFIGURATION(ch)   	(S3C_DMAC_C0CONFIGURATION+ch*0x20)



#endif //__ARM_MACH_DMA_PL080_H
