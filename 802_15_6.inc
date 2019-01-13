zOS_156	macro	shftdir,gcount4,gtemp,gbank
	local	do9nops,do0nops,newword,newbyte,alldone
	
	movf	BSR,w		;void zOS_156(const shftdir, uint4_t* gcount4,
	movwf	gbank		;             uint8_t* gtemp, uint8_t* gbank,
	zOS_DIS	FSR0,gbank
	banksel	MDCON		;             uint8_t* fsr1) {

	movlw	0x84		; zOS_DIS(BSR); // works in job context, not ISR
	movwf	FSR0H		; while (1) {
	moviw	FSR1++		; newword:
	movwf	FSR0L		;  uint8_t* fsr0 = 0x8400 | *(fsr1++);
	movf	INDF0,w		; newbyte:
	movwf	gtemp		;  *gtemp = *fsr0;
	movlw	8		;  *gcount4 = 8;
	movwf	gcount4		;  while (1) {
do9nops	
	nop			;
	nop			;
	nop			;
	nop			;
	nop			;
	nop			;
	nop			;
	nop			;
	nop			;   uint8_t c;
do0nops
#if 1
	if (shftdir)
	 rlf	gtemp,f		;   if (shftdir)
	else
	 rrf	gtemp,f		;    c = 0x80 & *gtemp, *gtemp <<= 1;
	endif
	movf	STATUS,w	;   else
	xorlw	0x80		;    c = 0x01 & *gtemp, *gtemp >>= 1;
	movwf	MDCON		;   MDCON = (0xfe & MDCON) | (c ? 1 : 0);//edge!
#else
	rrf	MDCON,w		;   if (shftdir)
	if (shftdir)
	 rlf	gtemp,f		;    c = 0x80 & *gtemp, *gtemp <<= 1;
	else
	 rrf	gtemp,f		;   else
	endif
	rlf	WREG		;    c = 0x01 & *gtemp, *gtemp >>= 1;
	 iorwf	0x80		;//<--adds an extra instruction to break 16 cyc!
	movwf	MDCON		;   MDCON = (0xfe & MDCON) | (c ? 1 : 0);//edge!
#endif
	decfsz	gcount4,f	;   if (--*gcount4) // still sending from gtemp,
	bra	do9nops		;    continue;// must update MDCON in 16 cycles
	incf	FSR0H,f		;   else {
	btfss	FSR0H,3		;    fsr0 += 0x0100;
	bra	newbyte		;    if (fsr0 < 0x8800) // not sent 4 sym bytes
newword
	movlw	0x84		;     goto newbyte;// updates MDCON in 16 cycles
	movwf	FSR0H		;
	moviw	FSR1++		;
	movwf	FSR0L		;
	aslf	FSR1L,w		;    if (fsr1 & 0x0070 < 0x0070) // more message
	addlw	0x20		;     goto newword; // unfortunately, 23 cycles!
	 bcf	MDCON,MDEN	; //<--better to disable modulation for syncing?
	btfsc	STATUS,C	;    else
	bra	alldone		;     goto alldone;
newbyte
	movf	INDF0,w		;   }
	movwf	gtemp		;
	movlw	8		;  }
	movwf	gcount4		;
	bra	do0nops		; } alldone:
alldone
	movf	gbank,w		; zOS_ENA(); // interrupts back on!
	movwf	BSR		;} // zOS_156()
	zOS_ENA
	endm
;;; this doesn't save too much time...although it does cost half a 2K flash!
