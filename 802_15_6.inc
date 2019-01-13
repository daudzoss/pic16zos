;;; eg:	zOS_156 +1,buf,size,PCLAT,gtemp,gbank
	
zOS_156	macro	shftdir,buf,size,gcount4,gtemp,gbank
	movf	BSR,w		;void zOS_156(const shftdir, uint8_t* buf,
	movwf	gbank		;             const size, uint4_t* gcount4, 
	zOS_LOC	FSR1,gbank,buf
	zOS_DIS	FSR0,gbank
	movlw	low buf		;             uint8_t gtemp, uint8_t* gbank) {
	movwf	FSR1L		;
	movlw	high buf	;
	movwf	FSR1H		;
	banksel	MDCON		; uint8_t* fsr1 = buf, c;
	bsf	MDCON,MDEN	; zOS_DIS(BSR); // works in job context, not ISR
	local	i
	clrf	gcount		; // 0 each iter (extreme workaround to gain 1 cyc)
i = 0
	while i < size
	 local	do9nops#v(i),do0nops#v(i),newword#v(i)
	 movlw	0x84		; MDCON |= 1<<MDEN; // FIXME: 17 cycles too soon
	 movwf	FSR0H		; for (uint8_t i = 0; i < size; i++) {
	 moviw	FSR1++		;  for (uint8_t* fsr0 = 0x8400 | fsr1[i];
	 movwf	FSR0L		;       fsr0 < 0x8800; fsr0 += 0x0100) {
	 movf	INDF0,w		;//<--2 cycles instead of 1 due to read from flash!
	 movwf	gtemp		;   *gtemp = *fsr0;// next byte in 32-bit send word
	 bsf	gcount4,3	;   for (*gcount4 = 8; *gcount4; (*gcount4)--) {  
do0nops#v(i)
#if 0
	 if (shftdir & 2)
	  rlf	gtemp,f		;
	 else
	  rrf	gtemp,f		;
	 endif
	 movf	STATUS,w	;
	 xorwf	0x80		; //<--fixes issue of modulating first bit too soon
#else
	 rrf	MDCON,w		;    if (shftdir < 0)
	 if (shftdir & 2)
	  rlf	gtemp,f		;     c = 0x80 & *gtemp, *gtemp <<= 1;
	 else
	  rrf	gtemp,f		;    else
	 endif
	 rlf	WREG		;     c = 0x01 & *gtemp, *gtemp >>= 1;
#endif
	 movwf	MDCON		;    MDCON = (0xfe & MDCON) | (c ? 1 : 0);//edge!
	 decfsz	gcount4,f	;   }
	 bra	do9nops#v(i)	;
	 incf	FSR0H,f		;  }
	 btfsc	FSR0H,3		;
	 bra	newword#v(i)	; }
	 movf	INDF0,w		;//<--2 cycles instead of 1 due to read from flash!
	 movwf	gtemp		;
	 movlw	8		;
	 movwf	gcount4		;
	 bra	do0nops#v(i)	;
do9nops#v(i)
	 nop			;
	 nop			;
	 nop			;
	 nop			;
	 nop			;
	 nop			;
	 nop			;
	 bra	do0nops#v(i)	;
i += 1
newword#v(i)
	endw

	nop
	nop
	nop
	nop
	nop
	nop
	nop
	
	bcf	MDCON,MDEN	;
	movf	gbank,w		; zOS_ENA(); // interrupts back on!
	movwf	BSR		;} // zOS_156()
	zOS_ENA
	endm
