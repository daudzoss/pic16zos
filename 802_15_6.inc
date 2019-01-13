;;; eg:	zOS_156 +1,buf,size,PCLAT,gtemp,gbank
	
zOS_156	macro	shftdir,buf,size,gcount4,gtemp,gbank
	local	i,do9nops,do0nops,newword,newbyte
	
	movf	BSR,w		;void zOS_156(const shftdir, uint8_t* buf,
	movwf	gbank		;             const size, uint4_t* gcount4, 
	zOS_DIS	FSR0,gbank
	movlw	low buf		;             uint8_t gtemp, uint8_t* gbank) {
	movwf	FSR1L		;
	movlw	high buf	;
	movwf	FSR1H		; uint8_t* fsr1 = buf;
	banksel	MDCON		;
	bsf	MDCON,MDEN	; zOS_DIS(BSR); // works in job context, not ISR
i = 0
	while i < size
	 local	newbyte#v(i),do9nops#v(i),do0nops#v(i),do?nops#v(i),newword#v(i)
	 movlw	0x84		;
	 movwf	FSR0H		; for (uint8_t i = 0; i < size; i++) {
	 moviw	FSR1++		;  for (uint8_t* fsr0 = 0x8400 | fsr1[i];
	 movwf	FSR0L		;       fsr0 < 0x8800; fsr0 += 0x0100) {
	 bra	newbyte#v(i)	;   *gtemp = *fsr0;// next byte in 32-bit send word
;	 movf	INDF0,w		;
;	 movwf	gtemp		;
;	 movlw	8		;  
;	 movwf	gcount4		;
do9nops#v(i)
	 nop			;
	 nop			;
	 nop			;
	 nop			;
	 nop			;
	 nop			;
	 nop			;
	 nop			;   for (*gcount4 = 8; *gcount4; (*gcount4)--) {
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
	 btfsc	FSR0H,3		;
	 bra	newword#v(i)	; }
newbyte#v(i)
	 movf	INDF0,w		;//<--2 cycles instead of 1 due to read from flash!
	 movwf	gtemp		;
	 movlw	8		;
	 movwf	gcount4		;
	 bra	do0nops#v(i)	;
i += 1
newword#v(i)
	endw
	movf	gbank,w		; zOS_ENA(); // interrupts back on!
	movwf	BSR		;} // zOS_156()
	zOS_ENA
	endm
