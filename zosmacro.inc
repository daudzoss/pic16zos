;;; zosmacro.inc
;;; potentially useful (but not mandatory) macros for zOS
;;;
;;; memory footprint:
;;; no memory words used upon inclusion (before expansion of a macro)
;;; _ 14-bit words if only zOS_CON() job is started to accept console output
;;; _ words if full-featured monitor and job manager shell zOS_MON() instead
	
#define zOS_ME BSR,w		; // "movf/andwf/xorwf zOS_ME" can't clobber BSR

zOS_GLO	macro	fsrnum,job
	local	fsrn
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif
	movlw	0x1f		;// documentation suggest 0f but BSR now 6 bits!
	andwf	FSR#v(fsrn)H,f	;inline int8_t zOS_GLO(int8_t* *fsrnum,
	if (job)
	 rlf	job,w		;                      int8_t* job) {
	else
	 rlf	zOS_ME		; int8_t w = 0x70| ((job?*job:bsr)<<1);
	endif
	andlw	0x0e		; *fsrnum = (*fsrnum & 0x1f00) | w;
	iorlw	0x70		; return w;
	movwf	FSR#v(fsrn)L	;}
	endm
	
zOS_MY2	macro	fsrnum		;inline int8_t zOS_MY2(int8_t**fsrnum){
	zOS_GLO	fsrnum,0	; return zOS_GLO(fsrnum, 0);
	endm			;}
	
zOS_LOC macro	fsrnum,job,offset
	local	fsrn
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif
	if (offset)
	 movlw	offset<<1	;inline int8_t zOS_LOC(int8_t* *fsrnum,
	 movwf	FSR#v(fsrn)L	;        int8_t* job, uint8_t offset) {
	else
	 clrf	FSR#v(fsrn)L	;
	endif
	rrf	zOS_ME		;
	movwf	FSR#v(fsrn)H	; return (*fsrnum=(job<<7)|offset)>>8;
	rrf	FSR#v(fsrn)L,f	;}
	endm

zOS_ADR	macro	a
	if (a)
	 movlw	low (a)		;inline void zOS_ADR(void* a) {
	 movwf	FSR0L		; // can be a -zOS_PRV or a |zOS_PRV for clarity
	 movlw	high (a)	; // at time of call
	 movwf	FSR0H		;
	else
	 clrf	FSR0L		; fsr0 = a;
	 clrf	FSR0H		;}
	endif
	endm

zOS_INT	macro	lhw,lsw
	if (lhw|lsw)
	movf	FSR0L,w		;inline void zOS_INT(const lhw, const lsw) {
	zOS_ARG	0
	movf	FSR0H,w		; if (lhw == 0 && lsw == 0) fsr0 = 0;
	zOS_ARG	1
	movlw	lhw		; zOS_ARG(0, fsr0 & 0x00ff);
	zOS_ARG 2
	movlw	lsw		; zOS_ARG(1, fsr0 >> 8);
	zOS_ARG 3
	else
	clrw			; zOS_ARG(2, lhw);
	movwf	FSR0L		; zOS_ARG(3, lsw);
	movwf	FSR0H		;}
	zOS_ARG	0
	zOS_ARG	1
	zOS_ARG	2
	zOS_ARG	3
	endif
	endm

zOS_LAU	macro	stash
	local retry
retry
	zOS_SWI	zOS_NEW
	movf	WREG,w
	btfsc	STATUS,Z
	bra	retry
	if (stash - WREG)
	 movwf	stash
	endif
	endm

zOS_SWI	macro	type		;inline void zOS_SWI(const int8_t type) {
	movlw	type		;
	movlp	0x00		; zos_swj(type);
	call	0x02		;}
	endm

zOS_ARG	macro	arg
	local num
num set (arg & 0x03)
	if (num == 0)
	 bcf	INTCON,GIE	;inline void zOS_ARG(const int8_t arg, int8_t w)
	endif
	movwf	zOS_AR#v(num)	;{if (!arg) INTCON &= 1<<GIE; zOS_AR0[arg] = w;}
	endm

zOS_RUN	macro	t0enable,t0flags
	;; start a TMR0 interrupt since none found (most in INTCON, others PIE0)
zOS_T0E	equ	t0enable
zOS_T0F	equ	t0flags	
	if (zOS_T0E)
	 banksel zOS_T0E
	 bsf	zOS_T0E,T0IE	;inline void zOS_RUN(uint8_t* t0enable) {
	endif
	
	;; advance the stack pointer to allow 5 stacks of 3 each (+1 if running)
	banksel	STKPTR		; if (t0enable)
	movlw	zOS_BOS		;  *t0enable |= 1<<T0IE; // guarantee interrupts
	movwf	STKPTR		; STKPTR = zOS_BOS; // every job bottom of stack
	
	;; set the active job to the first (and potentially only), interrupts ON
	movlb	1		; for (bsr = 1;;) // start with running job 1
	bsf	INTCON,GIE	; INTCON |= 1<<GIE;

	;; wait for stuff to happen
	local loop
loop
	clrwdt			;  clrwdt();
	bra	loop		;}
	endm

zOS_DBG	macro
	local	loop
	banksel	STKPTR
	clrf	STKPTR		;inline void zOS_DBG(void) {
	clrw			; for (int8_t w = STKPTR = 0;
loop
	clrf	TOSH		;      w < 16; w++){
	movwf	TOSL		;  TOSH = 0;
	incf	STKPTR,w	;  TOSL = w;
	andlw	0x0f		;
	movwf	STKPTR		;  STKPTR = (STKPTR + 1) % 16;
	btfss	STATUS,Z	; }
	bra	loop		; // still in job "0"
	movlb	0		;}
	endm

#ifdef	PID1CON
;;; 16x16bit signed multiply zOS_AR1:0 * zOS_AR3:2, core yielded during 7ms math
zOS_MUL	macro	fsrnum
	local fn,in,fac0L,fac0H,fac1L,fac1H,zeroH,start,con,setup,enb,bsy
	if (fsrnum & 3)
fn	 set 1
	else
fn	 set 0
	endif
inout	set	0x1f80 & PID1SETL
fac0L	set	0x1f & PID1K1L
fac0H	set	0x1f & PID1K1H
fac1L	set	0x1f & PID1SETL 
fac1H 	set	0x1f & PID1SETH 
zeroH	set     0x1f & PID1INH
start	set	0x1f & PID1INL
con	set	0x1f & PID1CON	
out0	set	0x1f & PID1OUTLL
out1	set	0x1f & PID1OUTLH
out2	set	0x1f & PID1OUTHL
out3	set	0x1f & PID1OUTHH
setup	set	(1<<PID1MODE1)
enb	set	PID1EN	
bsy	set	PID1BUSY
	local spinget,spinmul
	bcf	INTCON,GIE	;
	movlw	low PID1CON	;
	movwf	FSR#v(fn)L	;
	movlw	high PID1CON	;
	movwf	FSR#v(fn)H	;
	movlw	setup		;
	movwf	indf#v(fn)	;
	bsf	indf#v(fn),enb	;
spinget
	btfsc	INDF#v(fn),bsy	;
	bra	spinget		;
	movlw	low in		;
	movwf	FSR#v(fn)L	;
	movlw	high in		;
	movwf	FSR#v(fn)H	;
	movf	zOS_AR3,w	;
	movwi	fac0L[FSR#v(fn)];
	movf	zOS_AR2,w	;
	movwi	fac0H[FSR#v(fn)];
	movf	zOS_AR1,w	;
	movwi	fac1L[FSR#v(fn)];
	movf	zOS_AR0,w	;
	bsf	INTCON,GIE	;
	movwi	fac1H[FSR#v(fn)];
	clrw
	movwi	zeroH[FSR#v(fn)];
	movwi	start[FSR#v(fn)];
spinmul
	zOS_SWI	zOS_YLD
	btfss	INDF#v(fn),bsy	;
	bra	spinmul		;
	bcf	INTCON,GIE	;
	moviw	out3[FSR#v(fn)]	;
	movwf	zOS_AR3		;
	moviw	out2[FSR#v(fn)]	;
	movwf	zOS_AR2		;
	moviw	out1[FSR#v(fn)]	;
	movwf	zOS_AR1		;
	moviw	out0[FSR#v(fn)]	;
	movwf	zOS_AR0		;
	bcf	INTCON,GIE	;
	endm
#endif

;;; SWI behavior is malloc() with pointer (0 if too big) returned in zOS_AR1:0
;;;  if they were zero upon invoke, otherwise is a free() of memory it refers to
;;; (typically instantiated with base=zOS_STK+30, size = memory size - base)
zOS_HEA	macro	base,size
	endm
	
;;; simple output-only console job with circular buffer
zOS_HEX	macro
	andlw	0x0f		;
	addlw	0x06		;
	btfsc	WREG,4		; inline char zOS_HEX(uint8_t w) { 
	addlw	0x07		;  return (w & 0x0f > 9) ? '0'+w : 'A'+w-10;
	addlw	0x2a		; }
	endm
	
zOS_PUT	macro	max
	movwi	FSR0++		; inline void zOS_PUT(char* max, char w) {
	movf	FSR0L,w		;  *fsr0++ = w;
	andlw	0x7f		;
	xorlw	max		;
	btfss	STATUS,Z	;
	bra	$+3		;  if (fsr0 & 0x7f == max)
	moviw	2[FSR1]		;
	movwf	FSR0L		;   fsr0 = 2[fsr1] /*wrap*/;
	movf	FSR0L,w		;  1[fsr1] /*p1*/ = fsr0 & 0x00ff;
	movwi	1[FSR1]		; }
	endm

zOS_UNW	macro	job		; inline void zOS_UNW(int8_t job) { }
	zOS_MEM	FSR0,job,zOS_PCH;  fsr0 = 0x10 * (1 + job) + zOS_PCH;
	bcf	INDF0,zOS_WAI	;  *fsr0 &= ~(1 << zOS_WAI); // now runnable
	endm			; }
	
zOS_OUT	macro	swinum,string	; // 8 words per byte (+1) to avoid using stack
	local start,out
	if (out-start) > 255
	 pagesel out
	 goto	out		;
	else
	 bra	out		;
	endif
start
	dt	string
out		
	variable i
i = 0
	while i < (out-start)
	pagesel	i+start
	call	i+start
	zOS_ARG	0
	zOS_SWI	swinum
i += 1
	endw
	endm

zOS_PSH	macro	reg
	movf	zOS_ME		;
	;; bcf	INTCON,GIE
	banksel	TOSH
	movwf	TOSH		;
	if (reg-BSR)
	 movf	reg,w		;
	 mowwf	TOSL		;
	 movf	TOSH,w		;
	endif
	incf	STKPTR,f	;
	movwf	BSR		;
	;; bsf	INTCON,GIE
	endm

zOS_POP	macro	reg
	;; bcf	INTCON,GIE
	banksel	STKPTR
	decf	STKPTR,f	;
	if (reg-BSR)
	 movf	TOSL,w		;
	 movwf	reg		;
	endif
	movf	TOSH,w		;
	movwf	BSR		;
	;; bsf	INTCON,GIE
	endm
	
zOS_STR	macro	swinum,string
	local start,out,loop,done
	if (zero-start) < 255
	 bra	out
	else
	 pagesel out
	 goto	out		; // 0.5 words per byte (+62), use if strlen > 9
	endif
start
	da	string
zero
	if (zero-start)
	 da	0
	endif
out
	bcf	INTCON,GIE	;
;;; FIXME: need to autodetect TBLPTR vs EECON1 vs PMCON1
#ifdef EEADRL
	zOS_PSH	BSR
	banksel	EEADRL
	if (zero-start)
	 movlw	low start	;
	 movwf	EEADRL		;
	 movlw	high start	;
	 movwf	EEADRH		;
	else
	 movf	FSR0L,w		;
	 movwf	EEADRL		;
	 movf	FSR0H,w		;
	 movwf	EEADRH		;
	endif
loop	
	bcf	EECON1,CFGS	;
	bsf	EECON1,EEPGD	;
	bsf	EECON1,RD	;
	nop			;
	nop			;
	movf	EEDATH,w	;
	btfsc	STATUS,Z	;
	bra	done
	movwf	zOS_AR0		;
	zOS_POP	BSR
	zOS_SWI swinum
;;; FIXME: need to bit-correct per section 4.14 of 33014L.pdf
	bcf	INTCON,GIE	;
	zOS_PSH	BSR
	movf	EEDATL,w	;
	btfsc	STATUS,Z	;
	bra	done		;
	movwf	zOS_AR0		;
	zOS_POP	BSR
	zOS_SWI	swinum
	bcf	INTCON,GIE	;
	zOS_PSH	BSR
	incf	EEADRL,f	;
	btfsc	STATUS,Z	;
	incf	EEADRH,f	;
	bra	loop		;
done
	zOS_POP	BSR
#endif
	bsf	INTCON,GIE	;
	endm

zOS_CON	macro	p,bps,rts,hb,pin;inline void zOS_CON(int8_t p,int8_t bps,int8_t
	local	task,loop,decl,isr
	bra	decl		;                    rts,int8_t* hb,int8_t pin){
task
	;; initialize constants and variables
	local	t0div,t0rst,p0,p1,wrap,buf,max
t0div	set	0
t0rst	set	1
p0	set	0x20
p1	set	0x21
wrap	set	0x22
t0scale	set	0x23
buf	set	0x24
max	set	0x70
	
	local	sspbase,sspcon,sspstat,rtsflag
sspbase	set	SSP#v(p)CON1 & 0xff80
sspcon	set	SSP#v(p)CON1 & 0x1f
sspstat	set	SSP#v(p)STAT & 0x1f
	if (p == 1)
sspxmit	 set	TXIF
rtsflag	 set	TXIF
	endif

	movlw	high sspbase
	movwf	FSR0H		; goto decl;
	zOS_MY2	FSR0		;task:
	movlw	0xff		; fsr0 = 0x70 + (bsr<<1);//global always visible
	movwi	t0div[FSR0]	; 0[fsr0] = 0xff;// live TMR0 postscaler divider
	movwi	t0rst[FSR0]	; 1[fsr0] = 0xff; // live reset value for TMR0
	movlw	low sspbase	; const int8_t* sspbase = sspcon& 0xff80;
	movwf	FSR0L		; const int8_t* sspcon = (p==1)?SSP1CON:SSP2CON;
	rrf	zOS_ME		; const int8_t*sspstat=(p==1)?SSP1STAT:SSP2STAT;
	clrw			; const char* max = 0x70;
	rrf	WREG,w		; static char *p0, *p1, buf[]; //po:task, p1:ISR
	iorlw	buf		; const char* wrap = ((bsr&1)<<7) | buf;
	movwf	wrap		;
	movwf	p0		; fsr0 = sspbase;
	movwf	p1		; p0 = p1 = wrap; // reset value if they max out
	
loop
	zOS_SWI	zOS_SLP
	movlw	high rts	; do {
	movwf	FSR1H		;  zOS_SWI(zOS_SLP);
	movlw	low rts		;  // wait for SWI to store char(s) in buf[]
	movwf	FSR1L		;  // or HWI to indicate the last one got sent
	btfss	INDF1,rtsflag	;  if (*(fsr1 = rts) & (1<<rtsflag) == 0) //full
	bra	loop		;   continue; // sleep (sending previous char)
	rrf	zOS_ME 		;
	movwf	FSR1H		;
	movf	p0,w		;  // READY TO SEND, AND...       
	movwf	FSR1L		;
	xorwf	p1,w		;  fsr1 = (bsr<<7) | p0;
	btfsc	STATUS,Z	;  if (p0 == p1)
	bra	loop		;   continue; // nothing to do, somehow (!)
	moviw	FSR1++		;  // ...WE HAVE SOMETHING IN THE BUFFER TO SEND
	movwi	sspxmit[FSR0]	;  sspxmit[fsr0] = *fsr1++; // send a character
	movf	FSR1L,w		;
	movwf	p0		;  p0 = fsr1 & 0x00ff; // wrap around to buf+0
	andlw	0x7f		;
	xorlw	max		;
	btfss	STATUS,Z	;
	bra	loop		;  if (p0 & 0x7f == max) // ignore low bank bit
	movf	wrap,w		;   p0 = wrap; // =buf xor the lowest bank bit
	movwf	p0		;
	bra	loop		; } while (1);

	;; SWI will be coming from a job that wants to send a character
	;;  in which case the ISR stores it, advances p1 and clears zOS_WAI bit
	;;  and thus afer clearing zOS_WAI the job just re-sleeps if rtsflag==0
	;; 
	;; HWI will be coming from the SSP peripheral indicating ready-to-send
	;;  in which case ISR clears the job's zOS_WAI bit to get scheduled
	;; (or HWI will come from a tmr0 expiration for the blinking heartbeat)
	;; 
	;; Note: caller needs to make sure to check status of return value for
	;; != 0, just in case job is in between sleeps or with a full buffer
isr
	;; get fsr0 pointing to tmr0 postscaler/reset value, fsr1 to job struct;
	local err,done,do_swi,ascii,nottmr
	zOS_MEM	FSR1,zOS_JOB,0
	moviw	zOS_PCH[FSR1]	;isr:
	
	;; make sure the task is sleeping (else the above initialization unrun!)
	btfss	WREG,zOS_WAI	; fsr1 = 0x10 * (1+zos_job);
	bra	err		; if (zOS_PCH[fsr1] & (1<<zOS_WAI) == 0)
	movlw	high sspcon	;  goto done;
	movwf	FSR0H;[sic]	;
#if 0
	movf	zOS_ME		; // need to be sure in correct bank for zOS_JOB
	movwf	zOS_JOB		;
	movwf	BSR
#endif
	zOS_MY2	FSR0L		; fsr0 = 0x70 | (bsr << 1);

	;; if it's a simple and frequent timer overflow interrupt finish quickly
#if 0
	banksel	zOS_T0E
	btfss	zOS_T0E,T0IE	;
	bra	nottmr		; // TMR0 should always be enabled, by zOS_RUN()
#endif
	banksel	zOS_T0F
	btfss	zOS_T0F,T0IF	; if ((zOS_T0E & (1<<T0IE)) &&
	bra	nottmr		;     (zOS_T0F & (1<<T0IF))) { // timer overflow
	bcf	zOS_T0F,T0IF	;  zOS_T0F &= ~(1<<T0IF);// clear interrupt flag

	;; with fsr0 pointing to global pair, point fsr1 to local mem ("t0scale")
	zOS_LOC	FSR1,zOS_JOB,t0scale
	banksel	TMR0
	moviw	t0rst[FSR0]	;  fsr1 = (zOS_JOB << 7) | t0scale; // not bsr?!?
	movwf	TMR0		;  bsr = TMR0 >> 7; //now invalid for this branch
	decfsz	INDF1,f		;  TMR0 = t0rst[fsr0]; // reset TMR0
	bra	done		;  if (--*fsr1 == 0) {

	banksel	hb
	movf	INDF0,w		;
	movwf	INDF1		;   *fsr1 /*countdown*/ = *fsr0 /*postscaler*/;
	movlw	(1<<pin)	;
	xorwf	hb,f		;   hb ^= 1 << pin;
	bra	done		; } else {
	
	;; get fsr0 pointing to sspbase again, point fsr1 to p0
nottmr
	zOS_LOC	FSR1,zOS_JOB,p0
	movlw	low sspbase	;  fsr1 = (zOS_JOB << 7); // not bsr?!?
	movwf	FSR0L		;
	movlw	high sspbase	;
	movwf	FSR0H		;  fsr0 = sspbase;
	
	;; check for validated SWI first since it will be in zOS_MSK, else a HWI
	movf	zOS_MSK,w	;
	btfsc	STATUS,Z	;  if (zOS_MSK == 0) { // not a SWI so check HWI
	bra	do_swi		;   if (rts & (1<<rtsflag) == 0)
	banksel	rts
	btfsc	rts,rtsflag	;    zOS_RET(); // not the HWI we expected, ass
	bra	done		;  } else {
	zOS_RET
	;; check to make sure there are at least 2 characters free in the buffer
do_swi
	moviw	1[FSR1]		;   char* p1plus2 = 2 + (1[fsr1] /*p1*/ & 0x7f);
	andlw	0x7f		;
	addlw	0x12		;
	btfss	WREG,7		;   if (p1plus2 >= max)
	addlw	0x90+buf	;    p1plus2 -= (max - buf);
	addlw	0-0x10		;
	bcf	INDF1,7		;
	subwf	INDF1,w		;   char* w = p1plus2 - (0[fsr1] /*p0*/&= 0x7f);
	
	incf	FSR1L,f		;   // don't clobber w: OK if it's not 0 or 1
	btfsc	INDF1,7		;
	bra	$+4		;   if (1[fsr1++] /*p1*/ & 0x80) {
	decf	FSR1L,f		;    0[--fsr1] /*p0*/ |= 0x80; // p0 restored
	bsf	INDF1,7		;    fsr1++; // cancels the above decrement
	incf	FSR1L,f		;   }
	decf	FSR1L,f		;   fsr1--; // cancels increment from the "if"

	iorlw	0x00		;
	btfsc	STATUS,Z	;
	bra	err		;   if (w == 0)
	decf	WREG,w		;    goto err;// would wrap around, appear empty
	btfsc	STATUS,Z	;   else if (w == 1)
	bra	err		;    goto err; // would wrap around to be size 1

	moviw	1[FSR1]		;   fsr0 = 1[fsr1]; // stop examining p1 and use
	movwf	FSR0L		;
	movf	zOS_AR0,w	;   // we're now certain we won't exceed the buf
	btfss	STATUS,Z	;
	bra	ascii		;   if (zOS_AR0 == 0) { // print zOS_AR1 as hex
	
	swapf	zOS_AR1,w	;    zOS_PUT(max, zOS_HEX(zOS_AR1 >> 4));
	zOS_HEX
	zOS_PUT	max
	movf	zOS_AR1,w	;    zOS_PUT(max, zOS_HEX(zOS_AR1 >> 0));
	zOS_HEX
	zOS_PUT	max
	movlw	2		;    w = 2;
	bra	done		;   } else {
ascii
	zOS_PUT	max		;    zOS_PUT(max, zOS_AR0);
	movlw	1		;    w = 1;
	bra	done		;   }
err
	clrw			;  }
done
	zOS_UNW	zOS_JOB		; goto done; err: w = 0;
	zOS_RFI	WREG		; zOS_UNW(zOS_JOB); zOS_RFI (WREG); }
	
	;; intialize the UART peripheral, job handle and first three arguments
decl
;;; FIXME: need to set up the hardware port at the requested bps rate here first
	movlw	low task	;decl:
	movwf	FSR0L		;
	movlw	high task	;
	andlw	0x7f		;
	movwf	FSR0H		; FSR0 = task & 0x7fff;// MSB 0 => unprivileged
	bcf	FSR0H,zOS_PRB	; w = zOS_ARG(0, isr & 0x00ff);
	movlw	low isr		; w = zOS_ARG(1, isr>>8);
	zOS_ARG	0
	movlw	high isr	; w = zOS_ARG(2, (1<<TXIF)|(1<<T0IE));
	zOS_ARG	1
	movlw	(1<<TXIF)|(1<<T0IE)
	zOS_ARG	2
	endm			;}

#if 0
;;; like zOS_CON, but also accepts console input for a full job-management shell
zOS_MON	macro	port,bps,hb,pin	;
	local task,decl,isr
	bra	decl		;
task

loop

	bra 	loop
isr

	zOS_RFI
decl
	movlw	low task	; } else {
	movwf	FSR0L		;  //job#0 means zOS_RUN not encountered yet 
	movlw	high task	;
	andlw	0x7f		;
	movwf	FSR0H		;  FSR0 = isr & 0x7fff; //MSB 0 => unprivileged
	movlw	low isr		;  w = zOS_ARG(0, isr & 0x00ff)
	zOS_ARG	0
	movlw	high isr	;  w = zOS_ARG(1, isr >> 8);
	zOS_ARG	1
	movlw	1<<RCIF		;
	iorlw	1<<TXIF		;  w = zOS_ARG(2, (1<<RCIF) | (1<<TXIF));
	zOS_ARG	2
	endm
#endif	

