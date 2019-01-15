;;; eg:	zOS_PLC ; calculate Phy Layer Convergence Protocol header, returns size
;	zOS_CR8 ; calculate the CRC for the whole packet?
;	zOS_SFD	; write the 8 Start Frame Delimter bytes (over any temp bytes)

;;; eg: zOS_HBC 1,32,buf,0x800,gcount,gtemp,gbank(last 3 accessible from bank 7)
;;; or: zOS_HBC 1,0x80|var,buf,0x800,gcount,gtemp,gbank(var accessible from BSR)

PREAMBL	macro	code32
	local	i,prev,curr,temp
prev = 0-1
temp = code32
i = 0
	while (i < 32)
curr = temp & 1
	if (curr - prev)
	 if (curr)
	  bsf	MDCON,MDBIT	;inline void PREAMBL(uint32_t code32) {
	  nop			; for (int i = 0; i < 32; i += 1) {
	 else
	  bcf	MDCON,MDBIT	;  MDCON = (MDCON&0xfe) | ((code32&(1<<i))?1:0);
	  nop			; } // each iteration must be 2 I-cycles apart
	 endif
	else
	 bra	$+1		;} // PREAMBL()
	endif
temp >>= 1
prev = curr
i += 1
	endw
	endm
	
zOS_PLC	macro	temp1,temp2,...
;;; with FSR1 pointing to the start of an array at least 8+? bytes long:
	
;;;  size of data payload in W
	movwf	INDF1
	endm
	
zOS_SFD	macro	temp1,temp2,...
;;; with FSR1 pointing to the start of an uninitialized array...
;;; fill in the 8 bytes of the start-frame delimiter
	endm

zOS_HBC	macro	shfdir,size,buf,table,gcount,gtemp,gbank
	if (table & 0x07ff)
#error "lookup table not aligned to 2KiW boundary"
	endif
	local	i,limit
	local	p_init,p_final
	local	prstart,prend,phstart,plcphend,postart,postend,restore
prstart	
p_init = $ >> 11
	
	movf	BSR,w		;void zOS_HBC(const shfdir, uint8_t size, const*
	movwf	gbank		;             buf, const* table, uint4_t* gcount
	if (size > 0x007f)
	 movf	0x007f&size,w	; // size can be a constant (in which case enter
	 movwf	gtemp		; // the while loop at top), or jump in if a var
	endif
	if (buf < 0x2000)
	zOS_LOC	FSR1,gbank,buf
	else
	 movlw	low buf		;
	 movwf	FSR1L		;
	 movlw	high buf	;
	 movwf	FSR1H		;
	endif
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
limit = (0x800 - prstart + prend - postart + postend - phstart + plcphend) / 
i = limit-1
	 while (i+1)
	  local	looptop#v(i)
	  goto	looptop#v(i)	; // jumps into middle of loop (limit maybe 90?)
i -= 1
	 endw
	else
limit = size	
	endif
prend
	
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
	 rrf	MDCON,w		;    if (shfdir < 0)
do0nops#v(i)
	 if (shfdir & 0x80)
	  rlf	gtemp,f		;     c = gtemp & 0x80, gtemp <<= 1;
	 else
	  rrf	gtemp,f		;    else
	 endif

	 if (i)
	 else
phstart
	 PREAMBL 0x419D275F
	 PREAMBL 0x180A5323
	 PREAMBL 0x419D275F
	 PREAMBL 0x180A5323
	 PREAMBL 0x419D275F
	 PREAMBL 0x180A5323
	 PREAMBL 0x419D275F
	 PREAMBL 0x180A5323
plcphend
	 endif
	 rlf	WREG		;     c = gtemp & 0x01, gtemp >>= 1;
	 movwf	MDCON		;    MDCON = (0xfe & MDCON) | (c ? 1 : 0);//edge
	 decfsz	gcount,f	;   }
	 goto	do9nops#v(i)	;  }
	 incf	FSR0H,f		; } // the key point of this loop is that any 2
	 btfss	FSR0H,2		;   // successive assignments to MDCON are 16
	 goto	do6nops#v(i)	;   // instruction cycles apart, no matter what
i += 1
	endw
	
postart
	bra	$+1		;
	bra	$+1		;
	bra	$+1		;
	nop
restore
	bcf	MDCON,MDEN	;
	movf	gbank,w		; zOS_ENA(); // interrupts back on!
	movwf	BSR		;} // zOS_HBC()
	zOS_ENA
	
	goto	postend		; // double-check these delays, always same PCH?
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
p_final = $ >> 11
	if (p_final - p_init)
#error "macro straddles a flash page boundary; anomalous behavior would result!"
	endif
postend
	endm
