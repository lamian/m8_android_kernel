/*  
 * This include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
*/

#ifndef __ASM_ARCH_REGS_USB_OTG_HS_H
#define __ASM_ARCH_REGS_USB_OTG_HS_H

#define S3C_USBOTG_PHYREG(x)		((x) + S3C_VA_OTGSFR)

#define S3C_USBOTG_PHYPWR		S3C_USBOTG_PHYREG(0x0)
#define S3C_USBOTG_PHYCLK		S3C_USBOTG_PHYREG(0x4)
#define S3C_USBOTG_RSTCON		S3C_USBOTG_PHYREG(0x8)


/* USB2.0 OTG Controller register */
#define S3C_USBOTGREG(x) ((x) + S3C_VA_OTG)

//==============================================================================================
// Core Global Registers
#define S3C_UDC_OTG_GOTGCTL			S3C_USBOTGREG(0x000)		// OTG Control & Status
#define S3C_UDC_OTG_GOTGINT			S3C_USBOTGREG(0x004)		// OTG Interrupt
#define S3C_UDC_OTG_GAHBCFG			S3C_USBOTGREG(0x008)		// Core AHB Configuration
#define S3C_UDC_OTG_GUSBCFG			S3C_USBOTGREG(0x00C)		// Core USB Configuration
#define S3C_UDC_OTG_GRSTCTL			S3C_USBOTGREG(0x010)		// Core Reset
#define S3C_UDC_OTG_GINTSTS			S3C_USBOTGREG(0x014)		// Core Interrupt
#define S3C_UDC_OTG_GINTMSK			S3C_USBOTGREG(0x018)		// Core Interrupt Mask
#define S3C_UDC_OTG_GRXSTSR			S3C_USBOTGREG(0x01C)		// Receive Status Debug Read/Status Read
#define S3C_UDC_OTG_GRXSTSP			S3C_USBOTGREG(0x020)		// Receive Status Debug Pop/Status Pop
#define S3C_UDC_OTG_GRXFSIZ			S3C_USBOTGREG(0x024)		// Receive FIFO Size
#define S3C_UDC_OTG_GNPTXFSIZ		S3C_USBOTGREG(0x028)		// Non-Periodic Transmit FIFO Size
#define S3C_UDC_OTG_GNPTXSTS		S3C_USBOTGREG(0x02C)		// Non-Periodic Transmit FIFO/Queue Status

#define S3C_UDC_OTG_HPTXFSIZ		S3C_USBOTGREG(0x100)		// Host Periodic Transmit FIFO Size
#define S3C_UDC_OTG_DPTXFSIZ1		S3C_USBOTGREG(0x104)		// Device Periodic Transmit FIFO-1 Size
#define S3C_UDC_OTG_DPTXFSIZ2		S3C_USBOTGREG(0x108)		// Device Periodic Transmit FIFO-2 Size
#define S3C_UDC_OTG_DPTXFSIZ3		S3C_USBOTGREG(0x10C)		// Device Periodic Transmit FIFO-3 Size
#define S3C_UDC_OTG_DPTXFSIZ4		S3C_USBOTGREG(0x110)		// Device Periodic Transmit FIFO-4 Size
#define S3C_UDC_OTG_DPTXFSIZ5		S3C_USBOTGREG(0x114)		// Device Periodic Transmit FIFO-5 Size
#define S3C_UDC_OTG_DPTXFSIZ6		S3C_USBOTGREG(0x118)		// Device Periodic Transmit FIFO-6 Size
#define S3C_UDC_OTG_DPTXFSIZ7		S3C_USBOTGREG(0x11C)		// Device Periodic Transmit FIFO-7 Size
#define S3C_UDC_OTG_DPTXFSIZ8		S3C_USBOTGREG(0x120)		// Device Periodic Transmit FIFO-8 Size
#define S3C_UDC_OTG_DPTXFSIZ9		S3C_USBOTGREG(0x124)		// Device Periodic Transmit FIFO-9 Size
#define S3C_UDC_OTG_DPTXFSIZ10		S3C_USBOTGREG(0x128)		// Device Periodic Transmit FIFO-10 Size
#define S3C_UDC_OTG_DPTXFSIZ11		S3C_USBOTGREG(0x12C)		// Device Periodic Transmit FIFO-11 Size
#define S3C_UDC_OTG_DPTXFSIZ12		S3C_USBOTGREG(0x130)		// Device Periodic Transmit FIFO-12 Size
#define S3C_UDC_OTG_DPTXFSIZ13		S3C_USBOTGREG(0x134)		// Device Periodic Transmit FIFO-13 Size
#define S3C_UDC_OTG_DPTXFSIZ14		S3C_USBOTGREG(0x138)		// Device Periodic Transmit FIFO-14 Size
#define S3C_UDC_OTG_DPTXFSIZ15		S3C_USBOTGREG(0x13C)		// Device Periodic Transmit FIFO-15 Size

//==============================================================================================
// Host Mode Registers
//------------------------------------------------
// Host Global Registers
#define S3C_UDC_OTG_HCFG		S3C_USBOTGREG(0x400)		// Host Configuration
#define S3C_UDC_OTG_HFIR		S3C_USBOTGREG(0x404)		// Host Frame Interval
#define S3C_UDC_OTG_HFNUM		S3C_USBOTGREG(0x408)		// Host Frame Number/Frame Time Remaining
#define S3C_UDC_OTG_HPTXSTS		S3C_USBOTGREG(0x410)		// Host Periodic Transmit FIFO/Queue Status
#define S3C_UDC_OTG_HAINT		S3C_USBOTGREG(0x414)		// Host All Channels Interrupt
#define S3C_UDC_OTG_HAINTMSK	S3C_USBOTGREG(0x418)		// Host All Channels Interrupt Mask

//------------------------------------------------
// Host Port Control & Status Registers
#define S3C_UDC_OTG_HPRT		S3C_USBOTGREG(0x440)		// Host Port Control & Status

//------------------------------------------------
// Host Channel-Specific Registers
#define S3C_UDC_OTG_HCCHAR0		S3C_USBOTGREG(0x500)		// Host Channel-0 Characteristics
#define S3C_UDC_OTG_HCSPLT0		S3C_USBOTGREG(0x504)		// Host Channel-0 Split Control
#define S3C_UDC_OTG_HCINT0		S3C_USBOTGREG(0x508)		// Host Channel-0 Interrupt
#define S3C_UDC_OTG_HCINTMSK0	S3C_USBOTGREG(0x50C)		// Host Channel-0 Interrupt Mask
#define S3C_UDC_OTG_HCTSIZ0		S3C_USBOTGREG(0x510)		// Host Channel-0 Transfer Size
#define S3C_UDC_OTG_HCDMA0		S3C_USBOTGREG(0x514)		// Host Channel-0 DMA Address


//==============================================================================================
// Device Mode Registers
//------------------------------------------------
// Device Global Registers
#define S3C_UDC_OTG_DCFG		S3C_USBOTGREG(0x800)		// Device Configuration
#define S3C_UDC_OTG_DCTL		S3C_USBOTGREG(0x804)		// Device Control
#define S3C_UDC_OTG_DSTS		S3C_USBOTGREG(0x808)		// Device Status
#define S3C_UDC_OTG_DIEPMSK		S3C_USBOTGREG(0x810)		// Device IN Endpoint Common Interrupt Mask
#define S3C_UDC_OTG_DOEPMSK		S3C_USBOTGREG(0x814)		// Device OUT Endpoint Common Interrupt Mask
#define S3C_UDC_OTG_DAINT		S3C_USBOTGREG(0x818)		// Device All Endpoints Interrupt
#define S3C_UDC_OTG_DAINTMSK	S3C_USBOTGREG(0x81C)		// Device All Endpoints Interrupt Mask
#define S3C_UDC_OTG_DTKNQR1		S3C_USBOTGREG(0x820)		// Device IN Token Sequence Learning Queue Read 1
#define S3C_UDC_OTG_DTKNQR2		S3C_USBOTGREG(0x824)		// Device IN Token Sequence Learning Queue Read 2
#define S3C_UDC_OTG_DVBUSDIS	S3C_USBOTGREG(0x828)		// Device VBUS Discharge Time
#define S3C_UDC_OTG_DVBUSPULSE	S3C_USBOTGREG(0x82C)		// Device VBUS Pulsing Time
#define S3C_UDC_OTG_DTKNQR3		S3C_USBOTGREG(0x830)		// Device IN Token Sequence Learning Queue Read 3
#define S3C_UDC_OTG_DTKNQR4		S3C_USBOTGREG(0x834)		// Device IN Token Sequence Learning Queue Read 4

//------------------------------------------------
// Device Logical IN Endpoint-Specific Registers
#define S3C_UDC_OTG_DIEPCTL0		S3C_USBOTGREG(0x900)		// Device IN Endpoint 0 Control
#define S3C_UDC_OTG_DIEPINT0		S3C_USBOTGREG(0x908)		// Device IN Endpoint 0 Interrupt
#define S3C_UDC_OTG_DIEPTSIZ0		S3C_USBOTGREG(0x910)		// Device IN Endpoint 0 Transfer Size
#define S3C_UDC_OTG_DIEPDMA0		S3C_USBOTGREG(0x914)		// Device IN Endpoint 0 DMA Address

#define S3C_UDC_OTG_DIEPCTL1		S3C_USBOTGREG(0x920)		// Device IN Endpoint 2 Control
#define S3C_UDC_OTG_DIEPINT1		S3C_USBOTGREG(0x928)		// Device IN Endpoint 2 Interrupt
#define S3C_UDC_OTG_DIEPTSIZ1		S3C_USBOTGREG(0x930)		// Device IN Endpoint 2 Transfer Size
#define S3C_UDC_OTG_DIEPDMA1		S3C_USBOTGREG(0x934)		// Device IN Endpoint 2 DMA Address

#define S3C_UDC_OTG_DIEPCTL2		S3C_USBOTGREG(0x940)		// Device IN Endpoint 2 Control
#define S3C_UDC_OTG_DIEPINT2		S3C_USBOTGREG(0x948)		// Device IN Endpoint 2 Interrupt
#define S3C_UDC_OTG_DIEPTSIZ2		S3C_USBOTGREG(0x950)		// Device IN Endpoint 2 Transfer Size
#define S3C_UDC_OTG_DIEPDMA2		S3C_USBOTGREG(0x954)		// Device IN Endpoint 2 DMA Address

#define S3C_UDC_OTG_DIEPCTL3		S3C_USBOTGREG(0x960)		// Device IN Endpoint 3 Control
#define S3C_UDC_OTG_DIEPINT3		S3C_USBOTGREG(0x968)		// Device IN Endpoint 3 Interrupt
#define S3C_UDC_OTG_DIEPTSIZ3		S3C_USBOTGREG(0x970)		// Device IN Endpoint 3 Transfer Size
#define S3C_UDC_OTG_DIEPDMA3		S3C_USBOTGREG(0x974)		// Device IN Endpoint 3 DMA Address

#define S3C_UDC_OTG_DIEPCTL4		S3C_USBOTGREG(0x980)		// Device IN Endpoint 4 Control
#define S3C_UDC_OTG_DIEPINT4		S3C_USBOTGREG(0x988)		// Device IN Endpoint 4 Interrupt
#define S3C_UDC_OTG_DIEPTSIZ4		S3C_USBOTGREG(0x990)		// Device IN Endpoint 4 Transfer Size
#define S3C_UDC_OTG_DIEPDMA4		S3C_USBOTGREG(0x994)		// Device IN Endpoint 4 DMA Address

#define S3C_UDC_OTG_DIEPCTL5		S3C_USBOTGREG(0x9a0)		// Device IN Endpoint 5 Control
#define S3C_UDC_OTG_DIEPINT5		S3C_USBOTGREG(0x9a8)		// Device IN Endpoint 5 Interrupt
#define S3C_UDC_OTG_DIEPTSIZ5		S3C_USBOTGREG(0x9b0)		// Device IN Endpoint 5 Transfer Size
#define S3C_UDC_OTG_DIEPDMA5		S3C_USBOTGREG(0x9b4)		// Device IN Endpoint 5 DMA Address

#define S3C_UDC_OTG_DIEPCTL6		S3C_USBOTGREG(0x9c0)		// Device IN Endpoint 6 Control
#define S3C_UDC_OTG_DIEPINT6		S3C_USBOTGREG(0x9c8)		// Device IN Endpoint 6 Interrupt
#define S3C_UDC_OTG_DIEPTSIZ6		S3C_USBOTGREG(0x9d0)		// Device IN Endpoint 6 Transfer Size
#define S3C_UDC_OTG_DIEPDMA6		S3C_USBOTGREG(0x9d4)		// Device IN Endpoint 6 DMA Address

#define S3C_UDC_OTG_DIEPCTL7		S3C_USBOTGREG(0x9e0)		// Device IN Endpoint 7 Control
#define S3C_UDC_OTG_DIEPINT7		S3C_USBOTGREG(0x9e8)		// Device IN Endpoint 7 Interrupt
#define S3C_UDC_OTG_DIEPTSIZ7		S3C_USBOTGREG(0x9f0)		// Device IN Endpoint 7 Transfer Size
#define S3C_UDC_OTG_DIEPDMA7		S3C_USBOTGREG(0x9f4)		// Device IN Endpoint 7 DMA Address

#define S3C_UDC_OTG_DIEPCTL8		S3C_USBOTGREG(0xa00)		// Device IN Endpoint 8 Control
#define S3C_UDC_OTG_DIEPINT8		S3C_USBOTGREG(0xa08)		// Device IN Endpoint 8 Interrupt
#define S3C_UDC_OTG_DIEPTSIZ8		S3C_USBOTGREG(0xa10)		// Device IN Endpoint 8 Transfer Size
#define S3C_UDC_OTG_DIEPDMA8		S3C_USBOTGREG(0xa14)		// Device IN Endpoint 8 DMA Address

#define S3C_UDC_OTG_DIEPCTL9		S3C_USBOTGREG(0xa20)		// Device IN Endpoint 9 Control
#define S3C_UDC_OTG_DIEPINT9		S3C_USBOTGREG(0xa28)		// Device IN Endpoint 9 Interrupt
#define S3C_UDC_OTG_DIEPTSIZ9		S3C_USBOTGREG(0xa30)		// Device IN Endpoint 9 Transfer Size
#define S3C_UDC_OTG_DIEPDMA9		S3C_USBOTGREG(0xa34)		// Device IN Endpoint 9 DMA Address

//------------------------------------------------
// Device Logical OUT Endpoint-Specific Registers
#define S3C_UDC_OTG_DOEPCTL0		S3C_USBOTGREG(0xB00)		// Device OUT Endpoint 0 Control
#define S3C_UDC_OTG_DOEPINT0		S3C_USBOTGREG(0xB08)		// Device OUT Endpoint 0 Interrupt
#define S3C_UDC_OTG_DOEPTSIZ0		S3C_USBOTGREG(0xB10)		// Device OUT Endpoint 0 Transfer Size
#define S3C_UDC_OTG_DOEPDMA0		S3C_USBOTGREG(0xB14)		// Device OUT Endpoint 0 DMA Address

#define S3C_UDC_OTG_DOEPCTL1		S3C_USBOTGREG(0xB20)		// Device OUT Endpoint 1 Control
#define S3C_UDC_OTG_DOEPINT1		S3C_USBOTGREG(0xB28)		// Device OUT Endpoint 1 Interrupt
#define S3C_UDC_OTG_DOEPTSIZ1		S3C_USBOTGREG(0xB30)		// Device OUT Endpoint 1 Transfer Size
#define S3C_UDC_OTG_DOEPDMA1		S3C_USBOTGREG(0xB34)		// Device OUT Endpoint 1 DMA Address

#define S3C_UDC_OTG_DOEPCTL2		S3C_USBOTGREG(0xB40)		// Device OUT Endpoint 2 Control
#define S3C_UDC_OTG_DOEPINT2		S3C_USBOTGREG(0xB48)		// Device OUT Endpoint 2 Interrupt
#define S3C_UDC_OTG_DOEPTSIZ2		S3C_USBOTGREG(0xB50)		// Device OUT Endpoint 2 Transfer Size
#define S3C_UDC_OTG_DOEPDMA2		S3C_USBOTGREG(0xB54)		// Device OUT Endpoint 2 DMA Address

#define S3C_UDC_OTG_DOEPCTL3		S3C_USBOTGREG(0xB60)		// Device OUT Endpoint 3 Control
#define S3C_UDC_OTG_DOEPINT3		S3C_USBOTGREG(0xB68)		// Device OUT Endpoint 3 Interrupt
#define S3C_UDC_OTG_DOEPTSIZ3		S3C_USBOTGREG(0xB70)		// Device OUT Endpoint 3 Transfer Size
#define S3C_UDC_OTG_DOEPDMA3		S3C_USBOTGREG(0xB74)		// Device OUT Endpoint 3 DMA Address

#define S3C_UDC_OTG_DOEPCTL4		S3C_USBOTGREG(0xB80)		// Device OUT Endpoint 4 Control
#define S3C_UDC_OTG_DOEPINT4		S3C_USBOTGREG(0xB88)		// Device OUT Endpoint 4 Interrupt
#define S3C_UDC_OTG_DOEPTSIZ4		S3C_USBOTGREG(0xB90)		// Device OUT Endpoint 4 Transfer Size
#define S3C_UDC_OTG_DOEPDMA4		S3C_USBOTGREG(0xB94)		// Device OUT Endpoint 4 DMA Address

#define S3C_UDC_OTG_DOEPCTL5		S3C_USBOTGREG(0xBa0)		// Device OUT Endpoint 5 Control
#define S3C_UDC_OTG_DOEPINT5		S3C_USBOTGREG(0xBa8)		// Device OUT Endpoint 5 Interrupt
#define S3C_UDC_OTG_DOEPTSIZ5		S3C_USBOTGREG(0xBb0)		// Device OUT Endpoint 5 Transfer Size
#define S3C_UDC_OTG_DOEPDMA5		S3C_USBOTGREG(0xBb4)		// Device OUT Endpoint 5 DMA Address

#define S3C_UDC_OTG_DOEPCTL6		S3C_USBOTGREG(0xBc0)		// Device OUT Endpoint 6 Control
#define S3C_UDC_OTG_DOEPINT6		S3C_USBOTGREG(0xBc8)		// Device OUT Endpoint 6 Interrupt
#define S3C_UDC_OTG_DOEPTSIZ6		S3C_USBOTGREG(0xBd0)		// Device OUT Endpoint 6 Transfer Size
#define S3C_UDC_OTG_DOEPDMA6		S3C_USBOTGREG(0xBd4)		// Device OUT Endpoint 6 DMA Address

#define S3C_UDC_OTG_DOEPCTL7		S3C_USBOTGREG(0xBe0)		// Device OUT Endpoint 7 Control
#define S3C_UDC_OTG_DOEPINT7		S3C_USBOTGREG(0xBe8)		// Device OUT Endpoint 7 Interrupt
#define S3C_UDC_OTG_DOEPTSIZ7		S3C_USBOTGREG(0xBf0)		// Device OUT Endpoint 7 Transfer Size
#define S3C_UDC_OTG_DOEPDMA7		S3C_USBOTGREG(0xBf4)		// Device OUT Endpoint 7 DMA Address

#define S3C_UDC_OTG_DOEPCTL8		S3C_USBOTGREG(0xc00)		// Device OUT Endpoint 8 Control
#define S3C_UDC_OTG_DOEPINT8		S3C_USBOTGREG(0xc08)		// Device OUT Endpoint 8 Interrupt
#define S3C_UDC_OTG_DOEPTSIZ8		S3C_USBOTGREG(0xc10)		// Device OUT Endpoint 8 Transfer Size
#define S3C_UDC_OTG_DOEPDMA8		S3C_USBOTGREG(0xc14)		// Device OUT Endpoint 8 DMA Address

#define S3C_UDC_OTG_DOEPCTL9		S3C_USBOTGREG(0xc20)		// Device OUT Endpoint 9 Control
#define S3C_UDC_OTG_DOEPINT9		S3C_USBOTGREG(0xc28)		// Device OUT Endpoint 9 Interrupt
#define S3C_UDC_OTG_DOEPTSIZ9		S3C_USBOTGREG(0xc30)		// Device OUT Endpoint 9 Transfer Size
#define S3C_UDC_OTG_DOEPDMA9		S3C_USBOTGREG(0xc34)		// Device OUT Endpoint 9 DMA Address

//------------------------------------------------
// Endpoint FIFO address
#define S3C_UDC_OTG_EP0_FIFO		S3C_USBOTGREG(0x1000)
#define S3C_UDC_OTG_EP1_FIFO		S3C_USBOTGREG(0x2000)
#define S3C_UDC_OTG_EP2_FIFO		S3C_USBOTGREG(0x3000)
#define S3C_UDC_OTG_EP3_FIFO		S3C_USBOTGREG(0x4000)
#define S3C_UDC_OTG_EP4_FIFO		S3C_USBOTGREG(0x5000)
#define S3C_UDC_OTG_EP5_FIFO		S3C_USBOTGREG(0x6000)
#define S3C_UDC_OTG_EP6_FIFO		S3C_USBOTGREG(0x7000)
#define S3C_UDC_OTG_EP7_FIFO		S3C_USBOTGREG(0x8000)
#define S3C_UDC_OTG_EP8_FIFO		S3C_USBOTGREG(0x9000)
#define S3C_UDC_OTG_EP9_FIFO		S3C_USBOTGREG(0xa000)


//==============================================================================================
// Power and Clock Gating Registers
//------------------------------------------------
#define S3C_UDC_OTG_PCGCCTL		S3C_USBOTGREG(0x0E00)

//=====================================================================
//definitions related to CSR setting

// S3C_UDC_OTG_PCGCCTL : Power and Clock Gating Registers
#define STOP_PCLK_BIT		(0)
#define GATE_HCLK_BIT		(1)

// S3C_UDC_OTG_GOTGCTL
#define B_SESSION_VALID				(0x1<<19)
#define A_SESSION_VALID				(0x1<<18)

//S3C_UDC_OTG_DSTS: Device Status
#define	SUSPEND_STS		(0)

// S3C_UDC_OTG_GAHBCFG
#define PTXFE_HALF				(0<<8)
#define PTXFE_ZERO				(1<<8)
#define NPTXFE_HALF				(0<<7)
#define NPTXFE_ZERO				(1<<7)
#define MODE_SLAVE				(0<<5)
#define MODE_DMA				(1<<5)
#define BURST_SINGLE			(0<<1)
#define BURST_INCR				(1<<1)
#define BURST_INCR4				(3<<1)
#define BURST_INCR8				(5<<1)
#define BURST_INCR16			(7<<1)
#define GBL_INT_UNMASK			(1<<0)
#define GBL_INT_MASK			(0<<0)

// S3C_UDC_OTG_GRSTCTL
#define AHB_MASTER_IDLE			(0x1<<31)
#define CORE_SOFT_RESET			(0x1<<0)

// S3C_UDC_OTG_GINTSTS/S3C_UDC_OTG_GINTMSK core interrupt register
#define INT_RESUME				(0x1<<31)
#define INT_DISCONN				(0x1<<29)
#define INT_CONN_ID_STS_CNG		(0x1<<28)
#define INT_OUT_EP				(0x1<<19)
#define INT_IN_EP				(0x1<<18)
#define INT_ENUMDONE			(0x1<<13)
#define INT_RESET				(0x1<<12)
#define INT_SUSPEND				(0x1<<11)
#define INT_EARLY_SUSPEND		(0x1<<10)
#define	INT_GOUTNakEff			(0x1<<7)
#define	INT_GINNakEff			(0x1<<6)
#define INT_NP_TX_FIFO_EMPTY	(0x1<<5)
#define INT_RX_FIFO_NOT_EMPTY	(0x1<<4)
#define INT_SOF					(0x1<<3)
#define INT_OTG					(0x1<<2)
#define INT_MODE_MIS			(0x1<<1)
#define INT_CUR_MOD				(0x1<<0)

#define FULL_SPEED_CONTROL_PKT_SIZE		(8)
#define FULL_SPEED_BULK_PKT_SIZE		(64)

#define HIGH_SPEED_CONTROL_PKT_SIZE		(64)
#define HIGH_SPEED_BULK_PKT_SIZE		(512)

#define RX_FIFO_SIZE				(2048)
#define NPTX_FIFO_START_ADDR		RX_FIFO_SIZE
#define NPTX_FIFO_SIZE				(2048)
#define PTX_FIFO_SIZE				(2048)

#define DEPCTL_TXFNUM_0			(0x0<<22)
#define DEPCTL_TXFNUM_1			(0x1<<22)
#define DEPCTL_TXFNUM_2			(0x2<<22)
#define DEPCTL_TXFNUM_3			(0x3<<22)
#define DEPCTL_TXFNUM_4			(0x4<<22)


// Enumeration speed
#define USB_HIGH_30_60MHZ			(0x0<<1)
#define USB_FULL_30_60MHZ			(0x1<<1)
#define USB_LOW_6MHZ				(0x2<<1)
#define USB_FULL_48MHZ				(0x3<<1)

// S3C_UDC_OTG_GRXSTSP STATUS
#define OUT_PKT_RECEIVED			(0x2<<17)
#define OUT_TRANSFER_COMPLELTED		(0x3<<17)
#define SETUP_TRANSACTION_COMPLETED	(0x4<<17)
#define SETUP_PKT_RECEIVED			(0x6<<17)
#define GLOBAL_OUT_NAK				(0x1<<17)


// S3C_UDC_OTG_DCTL device control register
#define NORMAL_OPERATION			(0x1<<0)
#define SOFT_DISCONNECT				(0x1<<1)

// S3C_UDC_OTG_DIEPCTL0/DOEPCTL0 device control IN/OUT endpoint 0 control register
#define DEPCTL_EPENA				(0x1<<31)
#define DEPCTL_EPDIS				(0x1<<30)
#define DEPCTL_SNAK					(0x1<<27)
#define DEPCTL_CNAK					(0x1<<26)
#define DEPCTL_CTRL_TYPE			(0x0<<18)
#define DEPCTL_ISO_TYPE				(0x1<<18)
#define DEPCTL_BULK_TYPE			(0x2<<18)
#define DEPCTL_INTR_TYPE			(0x3<<18)
#define DEPCTL_USBACTEP				(0x1<<15)
#define DEPCTL0_MPS_64				(0x0<<0)
#define DEPCTL0_MPS_32				(0x1<<0)
#define DEPCTL0_MPS_16				(0x2<<0)
#define DEPCTL0_MPS_8				(0x3<<0)
#define DEPCTL_MPS_BULK_512			(512<<0)
#define DEPCTL_MPS_INT_MPS_16		(16<<0)
#define DIEPCTL_NEXT_EP_BIT			(11)


// S3C_UDC_OTG_DIEPMSK/DOEPMSK device IN/OUT endpoint common interrupt mask register
// S3C_UDC_OTG_DIEPINTn/DOEPINTn device IN/OUT endpoint interrupt register
#define BACK2BACK_SETUP_RECEIVED		(0x1<<6)
#define INTKNEPMIS						(0x1<<5)
#define INTKN_TXFEMP					(0x1<<4)
#define NON_ISO_IN_EP_TIMEOUT			(0x1<<3)
#define CTRL_OUT_EP_SETUP_PHASE_DONE	(0x1<<3)
#define AHB_ERROR						(0x1<<2)
#define EPDISBLD						(0x1<<1)
#define TRANSFER_DONE					(0x1<<0)

//DIEPTSIZ0 / DOEPTSIZ0

// DEPTSIZ common bit
#define DEPTSIZ_PKT_CNT_BIT 	(19)
#define DEPTSIZ_XFER_SIZE_BIT	(0)

#define	DEPTSIZ_SETUP_PKCNT_1	(1<<29)
#define	DEPTSIZ_SETUP_PKCNT_2	(2<<29)
#define	DEPTSIZ_SETUP_PKCNT_3	(3<<29)


#endif
