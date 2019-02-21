;;; eg:	zOS_PLC ; calculate Phy Layer Convergence Protocol header, returns size
;	zOS_CR8 ; calculate the CRC for the whole packet?
;	zOS_SFD	; write the 8 Start Frame Delimter bytes (over any temp bytes)

;;; eg: zOS_HBC 1,32,buf,0x800,gcount,gtemp,gbank(last 3 accessible from bank 7)
;;; or: zOS_HBC 1,0x80|var,buf,0x800,gcount,gtemp,gbank(var accessible from BSR)

#ifdef MDCON
#else
#define MDCON MD1CON0		;// changed between PIC16F183xx and 184xx series
#endif

PREAMBL	macro	code32
	local	i,prev,curr,temp
prev=0-1
temp=code32
i=0
	while (i < 0x20)
curr=temp & 1
	if (curr - prev)
	 if (curr)
	  bsf	MDCON,MDBIT	;inline void PREAMBL(uint32_t code32) {
	 else
	  bcf	MDCON,MDBIT	; for (int i = 0; i < 32; i += 1) {
	 endif
	 if (i)
	  nop			;  MDCON =0x80|(MDCON&0xfe)|(code32&(1<<i)?1:0);
	 else
	  bsf	MDCON,MDEN	; } // each iteration must be 2 I-cycles apart
	 endif
	else
	 bra	$+1		;} // PREAMBL()
	endif
temp>>=1
prev=curr
i+=1
	endw
	endm
	
zOS_PLC	macro	fsrnum,seed,temp1,temp2
	local	fsrn,H0,H1,H2,H3,nocrc
	if (fsrnum & 3)
fsrn	set	1
	else
fsrn	set	0
	endif
H0	set	0x8
H1	set	0x9
H2	set	0xa
H3	set	0xb
;;; with FSR pointing to the start of an array at least 12+w bytes long:
;;;  size of data payload in W
	movwf	temp1		;uint8_t zOS_PLC(uint8_t w, uint1_t seed,
	movlw	0x18		;                uint8_t* temp1, uint8_t* temp2)
	movwi	H0[FSR#v(fsrn)]	;{// 164kbps, no pilot insertion in H0 byte:
	swapf	seed,w		; 8[fsr] = 0x18;
	andlw	0x10		; // no burst mode, scrambler seed in H1 byte:
	movwi	H1[FSR#v(fsrn)]	; 9[fsr] = (seed & 1) << 4;
	movf	temp1,w		; // gross PSDU payload length in H2 byte: 
	movwi	H2[FSR#v(fsrn)]	; 10[fsr] = w;
	movlw	3		;
	movwf	seed		;
	addfsr	FSR#v(fsrn),+8	;
	zOS_CRC	#v(fsrn),seed,temp1,temp2
	addfsr	FSR#v(fsrn),-8-3; // CRC8 for the previous 3 bytes in H3 byte:
	movwi	H3[FSR#v(fsrn)]	; 11[fsr] = zOS_CRC(fsr, 3, temp1, temp2);
	moviw	H1[FSR#v(fsrn)]	;
	swapf	WREG		;
	movwf	seed		; // seed bit unchanged (for caller to negate)
	moviw	H2[FSR#v(fsrn)]	; return w + 4; // return total send length
	addlw	0x04		;} // zOS_PLC()
;;;now write the MAC header?
	endm

zOS_SFD	macro	fsrnum
	local	fsrn
	if (fsrnum & 3)
fsrn	set	1
	else
fsrn	set	0
	endif
;;; with FSR1 pointing to the start an uninitialized array...
;;; fill in the 8 bytes of the start-frame delimiter
	;; values from IEEE 802.15.6 Table 79
	movlw	0x6a		;inline void zOS_SFD(uint8_t** fsr) {
	movwi	FSR#v(fsrn)++	; *(*fsr)++ = 0x6a;
	movlw	0xba		;
	movwi	FSR#v(fsrn)++	; *(*fsr)++ = 0xba;
	movlw	0xdb		;
	movwi	FSR#v(fsrn)++	; *(*fsr)++ = 0xdb;
	movlw	0x53		;
	movwi	FSR#v(fsrn)++	; *(*fsr)++ = 0x53;
	movlw	0x1a		;
	movwi	FSR#v(fsrn)++	; *(*fsr)++ = 0x1a;
	movlw	0x64		;
	movwi	FSR#v(fsrn)++	; *(*fsr)++ = 0x64;
	movlw	0x5e		;
	movwi	FSR#v(fsrn)++	; *(*fsr)++ = 0x5e;
	movlw	0xb3		; *(*fsr)++ = 0xb3;
	movwi	FSR#v(fsrn)++	;}
	endm

zOS_WHT	macro	fsrnum,len,bits,temp,seed32
	local	fsrn,nextbyt,nextbit
	if (fsrnum & 3)
fsrn	set	1
	else
fsrn	set	0
	endif
	addfsr	FSR1,-1		;
	movwf	len		;void zOS_WHT(uint8_t* fsr, uint8_t w) {
	movlw	0x96		; uint32_t seed32 = (w & 0x80 == 0) ?
	movwf	0+seed32	;
	movlw	0x2a		;
	movwf	1+seed32	;
	movlw	0x80		;
	movwf	2+seed32	;
	movlw	0x4a		;
	movwf	3+seed32	;                   0x4a802a96 :
	btfss	len,7		;
	bra	nextbyt		;
	movlw	0x51		;
	movwf	0+seed32	;
	movlw	0xfa		;
	movwf	1+seed32	;
	movlw	0x46		;
	movwf	2+seed32	;
	movlw	0xf8		;
	movwf	3+seed32	;                   0xf846fa51;
	bcf	len,7		; uint7_t len = w & 0x7f;
nextbyt
	movlw	8		; do {
	movwf	bits		;  for (uint4_t bits = 8; bits; bits--) {
	addfsr	FSR1,+1		;
nextbit
	rlf	3+seed32,w	;   uint32_t c = (seed32 & 0x80000000) >> 24;
	movf	INDF#v(fsrn),w	;   *fsr = (c ^ ((*fsr) & 1)) | ((*fsr) >> 1);
	xorwf	STATUS,f	;   c = ((c << 23) ^ seed32) ? 0x400 : 0;
	rrf	INDF#v(fsrn),f	;   c = (c ^ seed32) ? 1 : 0;
	movf	3+seed32,w	;   seed32 = (seed32 << 1) | c;
	
	rlf	3+seed32,f	; // bits 31:24 done
	xorwf	3+seed32,w	; // WREG bit 7 holds pre-roll value z32^z31
	rlf	2+seed32,f	; // bits 23:16 done
	rlf	1+seed32,f	; // bits 15:8 done
	swapf	WREG		; // WREG bit 3 holds pre-roll value z32^z31
	xorwf	1+seed32,w	; // WREG bit 3 holds pre-roll value z32^z32^z11
	swapf	WREG		; // WREG bit 7 holds pre-roll value z32^z32^z11
	rlf	WREG		; // C contains pre-roll value z32^z32^z11
	rlf	0+seed32,f	; // bits 7:0 done
	decfsz	bits,f		;  }
	bra	nextbit		;  fsr++;
	decfsz	len,f		; } while (len--);
	bra	nextbyt		;} // zOS_WHT()
	endm

zOS_CRC	macro	fsrnum,len,bits,reg
	local	fsrn,nextbyt,nextbit
	if (fsrnum & 3)
fsrn	set	1
	else
fsrn	set	0
	endif
	movwf	len		;uint8_t zOS_CRC(uint8_t* fsr, uint8_t w) {
	movlw	0xff		; uint8_t reg = 0xff;
	movwf	reg		;
	addfsr	FSR#v(fsrn),-1	; do {
nextbyt
	movlw	8		;  
	movwf	bits		;  for (uint4_t bits = 8; bits; bits--) {
	addfsr	FSR#v(fsrn),+1	;
	rrf	INDF#v(fsrn),w	;
nextbit
	movf	reg,w		;
	xorwf	STATUS,w	;


;rrf	INDF#v(fsrn),w	;   uint1_t c = (*fsr) & 1;
;movf	reg,w		;
;andlw	0x01		;
;xorwf	STATUS,f	;   c ^= reg & 1;

	lsrf	INDF#v(fsrn),f	;
	btfsc	STATUS,C	;
	bsf	INDF#v(fsrn),7	;
	movlw	0x62		;   if (c)
	btfss	STATUS,C	;    reg ^= 0x62;
	movlw	0		;
	xorwf	reg,f		;
	rrf	reg,f		;   reg = (c ? 0x80 : 0) | (reg >> 1);
	decfsz	bits,f		;  }
	bra	nextbit		;  fsr++;
	decfsz	len,f		; } while (w--);
	bra	nextbyt		;//FIXME: is always returning 0
	addfsr	FSR#v(fsrn),+1	; return reg; // CCITT CRC8: x^8+x^7+x^3+x^2+1
	movf	reg,w		;} // zOS_CRC()
	endm
	
zOS_HBC	macro	shfdir,size,buf,table,gcount,gtemp,gbank
	if (table & 0x07ff)
	 error	"lookup table not aligned to 2KiW boundary"
	endif
	local	i,usable,myoffst,limit
	local	p_init,p_final
	local	prstart,prend,plcpcsize,postart,restore,postend,macroend
prstart	
p_init=prstart>>0xb
	
	rlf	BSR,w		;void zOS_HBC(const shfdir, uint8_t size, const*
	movwf	gbank		;             buf, const* table, uint4_t* gcount
	if (size > 0x007f)
	 movf	0x007f&size,w	; // size can be a constant (in which case enter
	 movwf	gtemp		; // the while loop at top), or jump in if a var
	endif
	if (buf < 0x2000)
	zOS_LOC	FSR1,BSR,buf
	else
	 movlw	low buf		;
	 movwf	FSR1L		;
	 movlw	high buf	;
	 movwf	FSR1H		;
	endif
	
	rlf	INTCON,w	;             , uint8_t gtemp, uint8_t* gbank) {
	rrf	gbank,f		; gbank = (INTCON&(1<<GIE))|bsr;// preserve both
	movlw 	high table	;
;	iorlw	0x80		; // workarounds to gain 2 cycles: gcount, fsr0h
	movwf	FSR0H		; uint8_t* fsr0, * fsr1 = buf, c;
	banksel	MDCON		;
	clrf	gcount		;
	movlp	p_init<<3	;
	if (size > 0x007f)
	 movf	gtemp,w		;
	 brw			;
	 goto	restore		; // requesting to  send a zero-length buffer
prend
#ifdef __PAGE_SIZE
#else
#define	__PAGE_SIZE 0x800	; usually the case
#endif
usable=__PAGE_SIZE - (prstart - (p_init<<0xb))
plcpsize=4*0x60
limit=(usable + prstart-prend + 9 + plcpsize - 1)/(0x20)

i=limit-1
	 while (i+1)
	  local	looptop#v(i)
	  goto	looptop#v(i)	; // jumps into middle of loop (limit maybe 90?)
i-=1
	 endw
	else
limit=size
	endif
	
i=0
	while (i < limit)
	 local	do9nops#v(i),do0nops#v(i)
looptop#v(i)
	 bcf	FSR0H,2		; for (uint8_t i = 0; i < size; i++) {
	 moviw	FSR1++		;  for (fsr0 = 0x8000 | table | fsr1[i];
	 movwf	FSR0L		;       fsr0 < table + 0x0400; fsr0 += 0x0100) {
	 movf	INDF0,w	;	;//<--2 cycles instead of 1 due to reading flash
	 movwf	gtemp		;   *gtemp = *fsr0;// next byte in 32-bit sendee
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
	;; IEEE 802.15.6 Table 76
myoffst
	 PREAMBL 0x419D275F
	 PREAMBL 0x180A5323
	 PREAMBL 0x419D275F
	 PREAMBL 0x180A5323
	 PREAMBL 0x419D275F
	 PREAMBL 0x180A5323
	 PREAMBL 0x419D275F
	 PREAMBL 0x180A5323
	 org	 myoffst+plcpsize
	 endif

	 rlf	WREG		;     c = gtemp & 0x01, gtemp >>= 1;
	 movwf	MDCON		;    MDCON = (0xfe & MDCON) | (c ? 1 : 0);//edge
	 decfsz	gcount,f	;   }
	 goto	do9nops#v(i)	;  }
	 incf	FSR0H,f		; } // the key point of this loop is that any 2
	 btfss	FSR0H,2		;   // successive assignments to MDCON are 16
	 goto	do6nops#v(i)	;   // instruction cycles apart, no matter what
i+=1
	endw
	
	bra	$+1		;
	bra	$+1		;
	bra	$+1		;
	nop
restore
	bcf	MDCON,MDEN	;
	movf	gbank,w		;
	movwf	BSR		; if (gbank & 0x80)
	btfsc	gbank,GIE	;  INTCON |= 1<<GIE; // interrupts back on!
	bsf	INTCON,GIE	;} // zOS_HBC()
	goto	macroend	; // double-check these delays, always same PCH?
i=0
	while (i < limit)
do9nops#v(i)
	 nop			;
	 bra	$+1		;
do6nops#v(i)
	 bra	$+1		;
	 bra	$+1		;
	 goto	do0nops#v(i)	;
i+=1
	endw
macroend
p_final=macroend>>0x0b
	if (p_final - p_init)
	 error	"macro straddles a flash page boundary; anomalous behavior would result!"
	endif
	endm
