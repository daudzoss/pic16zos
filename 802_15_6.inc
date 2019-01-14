;;; eg:	zOS_156 +1,32,buf,0x0800,PCLAT,gtemp,gbank
	
zOS_156	macro	shfdir,size,buf,table,gcount4,gtemp,gbank
	movf	BSR,w		;void zOS_156(const shfdir, const size, const* buf,
	movwf	gbank		;             const* table, uint4_t* gcount4, 
	zOS_LOC	FSR1,gbank,buf
	zOS_DIS	FSR0,gbank
	if (table & 0x07ff)
#error "lookup table not aligned to 2KiW boundary"
	endif
	movlw 	high table	;             uint8_t gtemp, uint8_t* gbank) {
	iorlw	0x80		; // workarounds to gain 2 cycles: gcount, fsr0h
	movwf	FSR0H		; uint8_t* fsr0, * fsr1 = buf, c;
	banksel	MDCON		; zOS_DIS(BSR); // works in job context, not ISR
	local	i
	clrf	gcount		;
	bcf	FSR0H,2		; for (uint8_t i = 0; i < size; i++) {
	moviw	FSR1++		;  for (fsr0 = 0x8000 | table | fsr1[i];
	movwf	FSR0L		;       fsr0 < table + 0x0400; fsr0 += 0x0100) {
	movf	INDF0,w		;//<--2 cycles instead of 1 due to read from flash!
	movwf	gtemp		;   *gtemp = *fsr0;// next byte in 32-bit send word
	bsf	gcount4,3	;   for (*gcount4 = 8; *gcount4; (*gcount4)--) {  
i = 0
	while i < size
	 local	do9nops#v(i),do0nops#v(i),shortc#v(i)
do0nops#v(i)
	 if (shfdir & 2)
	  rlf	gtemp,f		;
	 else
	  rrf	gtemp,f		;
	 endif
shortc#v(i)
	 movf	STATUS,w	;
	 xorlw	0x80		; //<--fixes issue of modulating first bit too soon
	 movwf	MDCON		;    MDCON = (0xfe & MDCON) | (c ? 1 : 0);//edge!
	 decfsz	gcount4,f	;   }
	 bra	do9nops#v(i)	;
	 incf	FSR0H,f		;  }
	 btfss	FSR0H,2		;
	 bra	do5nops#v(i)	; }
	 bcf	FSR0,2		;
	 moviw	FSR1++		;
	 movwf	FSR0L		;
	 if (shfdir & 2)
	  rlf	INDF0,w		;//<--2 cycles instead of 1 due to read from flash!
	 else
	  rrf	INDF0,w		;//<--2 cycles instead of 1 due to read from flash!
	 endif
	 movwf	gtemp		;
	 bsf	gcount4,3	;
	 bra	shortc#v(i+1)	;
do9nops#v(i)
	 nop			;
	 bra	$+1		;
do5nops#v(i)
	 bra	$+1		;
	 bra	$+1		;
	 bra	do0nops#v(i)	;
i += 1
	endw

	bra	$+1		;
	bra	$+1		;
	bra	$+1		;
	nop
	bcf	MDCON,MDEN	;
	movf	gbank,w		; zOS_ENA(); // interrupts back on!
	movwf	BSR		;} // zOS_156()
	zOS_ENA
	endm
