;;; eg:	zOS_156 1/-1,buf,size,PCLAT,gtemp,gbank
	
zOS_156	macro	shftdir,buf,size,gcount4,gtemp,gbank
	local	i,do9nops,do0nops,newword,newbyte
	
	movf	BSR,w		;void zOS_156(const shftdir, uint8_t* buf,
	movwf	gbank		;             const size, uint4_t* gcount4, 
	zOS_DIS	FSR0,gbank
	banksel	MDCON		;             uint8_t gtemp, uint8_t* gbank) {
	
	movlw	low buf		; zOS_DIS(BSR); // works in job context, not ISR
	movwf	FSR1L		;
	movlw	high buf	;
	movwf	FSR1H		; uint8_t* fsr1 = buf;
i = 0
	while i < size
	 local	newbyte#v(i),do9nops#v(i),do0nops#v(i)
	 movlw	0x84		;
	 movwf	FSR0H		; for (uint8_t i = 0; i < size; i++) {
	 moviw	FSR1++		;  for (uint8_t* fsr0 = 0x8400 | fsr1[i];
	 movwf	FSR0L		;       fsr0 < 0x8800; fsr0 += 0x0100) {
newbyte#v(i)
	 movf	INDF0,w		;
	 movwf	gtemp		;   *gtemp = *fsr0;// next byte in 32-bit send word
	 movlw	8		;  
	 movwf	gcount4		;   for (*gcount4 = 8; *gcount4; (*gcount4)--) {
do9nops#v(i)
	 nop			;
	 nop			;
	 nop			;
	 nop			;
	 nop			;
	 nop			;
	 nop			;
	 nop			;
	 nop			;    uint8_t c;
do0nops#v(i)
	 rrf	MDCON,w		;    if (shftdir < 0)
	 if (shftdir & 2)
	  rlf	gtemp,f		;     c = 0x80 & *gtemp, *gtemp <<= 1;
	 else
	  rrf	gtemp,f		;    else
	 endif
	 rlf	WREG		;     c = 0x01 & *gtemp, *gtemp >>= 1;
	 movwf	MDCON		;    MDCON = (0xfe & MDCON) | (c ? 1 : 0);//edge!
	 decfsz	gcount4,f	;   }
	 bra	do9nops#v(i)	;
	 incf	FSR0H,f		;  }
	 btfss	FSR0H,3		;
	 bra	newbyte#v(i)	; }
i += 1
	endw
alldone
	movf	gbank,w		; zOS_ENA(); // interrupts back on!
	movwf	BSR		;} // zOS_156()
	zOS_ENA
	endm
	



newword
	movlw	0x84		;     goto newbyte;// updates MDCON in 16 cycles
	movwf	FSR0H		;
	moviw	FSR1++		;
	movwf	FSR0L		;
	aslf	FSR1L,w		;    if (fsr1 & 0x0070 < 0x0070) // more message
	addlw	0x20		;     goto newword; // unfortunately, 23 cycles!
	btfsc	STATUS,C	;    else
	bra	alldone		;     goto alldone;
newbyte
	movf	INDF0,w		;   }
	movwf	gtemp		;
	movlw	8		;  }
	movwf	gcount4		;
	bra	do0nops		; } alldone:
;;; this doesn't save too much time...although it does cost half a 2K flash!
