@	AREA    |.text|,CODE,READONLY
	.section .text, "x"

	.global s3c_aes_copyfifo_8w

s3c_aes_copyfifo_8w:
@	/* r0=uSrc, r1=uDst, r2=uCnt */
	push    {r4-r11}
	mov	r3, r1
		
loop:
	ldm     r0!,{r4-r11}		
	stm	r1!,{r4-r11}	

	subs    r2, r2, #1                
	mov	r1, r3                
	bgt     loop
	pop     {r4-r11}
	bx      lr
	.ltorg

	.global s3c_aes_getfifo_8w

s3c_aes_getfifo_8w:
@	/* r0=uSrc, r1=uDst, r2=uCnt */
	push    {r4-r11}
	mov	r3, r0
				
loop2:           
	ldm	r0!,{r4-r11}
	
	stm	r1!,{r4-r11}                                                                                                
	subs	r2, r2, #1
	mov	r0, r3 
	bgt	loop2
	pop	{r4-r11}
	bx	lr
	.ltorg               

.end
                
