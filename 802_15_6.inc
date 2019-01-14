;;; eg:	zOS_156 1,32,buf,0x800,gcount,gtemp,gbank(last 3 accessible from bank 7)
;;; or: zOS_156	1,var,buf,0x800,gcount,gtemp,gbank(with var accessible from BSR)
	
zOS_156	macro	shfdir,size,buf,table,gcount,gtemp,gbank
	if (table & 0x07ff)
#error "lookup table not aligned to 2KiW boundary"
	endif
	local	i,limit
	local	p_init,p_final
	local	restore,postmac
p_init	equ	$ >> 11
	movf	BSR,w		;void zOS_156(const shfdir, uint8_t size, const*
	movwf	gbank		;             buf, const* table, uint4_t* gcount
	if (size > 0x007f)
	 movf	size,w		; // size can be a constant (in which case enter
	 movwf	gtemp		; // the while loop at top), or jump in if a var
	endif
	zOS_LOC	FSR1,gbank,
	zOS_DIS	FSR0,gbank
	movlw 	high table	;             , uint8_t gtemp, uint8_t* gbank) {
	iorlw	0x80		; // workarounds to gain 2 cycles: gcount, fsr0h
	movwf	FSR0H		; uint8_t* fsr0, * fsr1 = buf, c;
	banksel	MDCON		; zOS_DIS(BSR); // works in job context, not ISR
	clrf	gcount		;
	movlp	p_init		;
	if (size > 0x007f)
	 movf	gtemp,w		;
	 brw			;
	 goto	restore		; // requesting to  send a zero-length buffer
limit = 100
i = limit-1
	 while (i+1)
	  local	looptop#v(i)
	  goto	looptop#v(i)	; // jumps into middle of loop (limit maybe 90?)
i -= 1
	 endw
	else
limit = size	
	endif
i = 0
	while (i < limit)
	 local	do9nops#v(i),do0nops#v(i)
looptop#v(i)
	 bcf	FSR0H,2		; for (uint8_t i = 0; i < size; i++) {
	 moviw	FSR1++		;  for (fsr0 = 0x8000 | table | fsr1[i];
	 movwf	FSR0L		;       fsr0 < table + 0x0400; fsr0 += 0x0100) {
	 movf	INDF0,w	;	;//<--2 cycles instead of 1 due to read from flash!
	 movwf	gtemp		;   *gtemp = *fsr0;// next byte in 32-bit send word
	 bsf	gcount,3	;   for (*gcount = 8; *gcount; (*gcount)--) {  
do0nops#v(i)
	 if (shfdir & 2)
	  rlf	gtemp,f		;
	 else
	  rrf	gtemp,f		;
	 endif
shortc#v(i)
	 movf	STATUS,w	;
	 xorlw	0x80		; //<--fixes issue of modulating 1st bit too soon
	 movwf	MDCON		;    MDCON = (0xfe & MDCON) | (c ? 1 : 0);//edge!
	 decfsz	gcount,f	;   }
	 goto	do9nops#v(i)	;
	 incf	FSR0H,f		;  }
	 btfss	FSR0H,2		;
	 goto	do6nops#v(i)	; }
i += 1
	endw
	bra	$+1		;
	bra	$+1		;
	bra	$+1		;
	nop
restore
	bcf	MDCON,MDEN	;
	movf	gbank,w		; zOS_ENA(); // interrupts back on!
	movwf	BSR		;} // zOS_156()
	zOS_ENA
	
	goto	postmac		; // double-check these delays, always same PCH?
i = 0
	while i < size
do9nops#v(i)
	nop			;
	bra	$+1		;
do6nops#v(i)
	bra	$+1		;
	bra	$+1		;
	goto	do0nops#v(i)	;
i += 1
	endw
p_final	equ	$ >> 11
	if (p_final - p_init)
#error "macro straddles a flash page boundary; anomalous behavior would result!"
	endif
postmac
	endm
