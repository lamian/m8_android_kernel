/* linux/arch/arm/plat-s3c64xx/changefreq.S
 *
 * Copyright 2009 Samsung Electronics
 *	
 *	http://samsung.samsungsemi.com/
 *
 * S3C64XX CPU clock change code
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/linkage.h>
#include <asm/assembler.h>
#include <mach/map.h>

#define I_Bit	(0x80)
#define F_Bit	(0x40)
#define R1_BP	(0x800)	

	.text

	/*---------------------------------
	 *	s3c6410_changedivider
	 *--------------------------------- */
ENTRY(s3c6410_changedivider)

	stmfd	sp!, {r0-r5, r14}

	mov		r5, r0
	bl		System_DisableIRQ
	bl		System_DisableBP

	mov		r2, #0
	mov		r3, #0
loopcd:
	add 		r3, r3, #1
	mov 		r4, #0
	mcr 		p15, 0, r2, c7, c10, 4 /* data synchronization barrier instruction */
	mcr 		p15, 0, r2, c7, c10, 5 /* data memory barrier operation */
	cmp 		r3, #2
	streq 		r5, [r1]

loop1000:
	add 		r4, r4, #1
	cmp 		r4, #0x100
	bne 		loop1000
	cmp 		r3, #2
	bne 		loopcd

	bl		System_EnableBP
	bl		System_EnableIRQ

	ldmfd   	sp!, {r0-r5, r14}
	bx		lr
		
	/*---------------------
	 *	Disable IRQ
	 *-------------------- */
System_DisableIRQ:
	mrs		r0,cpsr
	orr		r0,r0,#I_Bit
	msr		cpsr_cxsf,r0
	bx		lr

	/*---------------------
	 *	Enable IRQ
 	 *-------------------- */
System_EnableIRQ:
	mrs		r0,cpsr
	bic		r0,r0,#I_Bit
	msr		cpsr_cxsf,r0
	bx		lr

	/*---------------------------------
	 *	Enable Branch Prediction
	 *-------------------------------- */
System_EnableBP:
	mrc		p15,0,r0,c1,c0,0
	orr		r0,r0,#R1_BP
	mcr		p15,0,r0,c1,c0,0
	bx		lr

	/*---------------------------------
	 *	Disable Branch Prediction
	 *-------------------------------- */
System_DisableBP:
	mrc		p15,0,r0,c1,c0,0
	bic		r0,r0,#R1_BP
	mcr		p15,0,r0,c1,c0,0
	bx		lr

	.data

	.end

