;;; ZOSMACRO.INC
;;; potentially useful (but not mandatory) macros for zOS
;;;
;;; memory footprint:
;;; no memory words used upon inclusion (before expansion of a macro)
;;; _ 14-bit words if only zOS_CON() job is started to accept console output
;;; _ words if full-featured monitor and job manager shell zOS_MON() instead
	
#define zOS_ME BSR,w		; // "movf/andwf/xorwf zOS_ME" can't clobber BSR

zOS_MY2	macro	fsrnum
	local	fsrn
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif
	movlw	0x1f		;// documentation suggest 0f but BSR now 6 bits!
	andwf	FSR#v(fsrn)H,f	;inline void zOS_MY2(int8_t* *fsrnum) {
	rlf	zOS_ME		;
	andlw	0x0e		; int8_t w = bsr << 1;
	iorlw	0x70		; *fsrnum = 0x70 | (w & 0x0e);
	movwf	FSR#v(fsrn)L	;}
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

zOS_RUN	macro	t0enable
	;; start a TMR0 interrupt since none found (most in INTCON, others PIE0)
	if (t0enable)
	 banksel t0enable
	 bsf	t0enable,T0IE	;inline void zOS_RUN(uint8_t* t0enable) {
	
;;FIXME: set the prescaler appropriately so that the IRQ frequency isn't crazy

	endif
	
	;; advance the stack pointer to allow 5 stacks of 3 each (+1 if running)
	banksel	STKPTR		; if (t0enable)
	movlw	zOS_BOS		;  *t0enable |= 1<<T0IE; // guarantee interrupts
	bsf	INTCON,GIE	; INTCON |= 1<<GIE;
	movwf	STKPTR		; STKPTR = zOS_BOS; // every job bottom of stack
	
	;; set the active job to the first (and potentially only)
	movlb	1		; for (bsr = 1;;) // start with running job 1

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
	andlw	0x0f		;  STKPTR++; 
	btfss	STATUS,Z	; }
	bra	loop		;}
	end

#ifdef	PID1CON
;;; 16x16bit multiply zOS_AR1:0 by zOS_AR3:2, core yielded during 7.5ms CIP math
zOS_MUL	macro
	end
#endif

;;; SWI behavior is malloc() with pointer (0 if too big) returned in zOS_AR1:0
;;;  if they were zero upon invoke, otherwise is a free() of memory it refers to
;;; (typically instantiated with base=zOS_STK+30, size = memory size - base)
zOS_HEA	macro	base,size
	end
	
;;; simple output-only console job with circular buffer
zOS_CON	macro	p,bps,rts,hb,pin;inline void zOS_CON(int8_t p,int8_t bps,int8_t
	local	task,loop,decl,isr
	bra	decl		;                    rts,int8_t* hb,int8_t pin){
task	
	;; initialize constants and variables
	local	p0,p1,wrap,buf,max
p0	equ	0x20
p1	equ	0x21
wrap	equ	0x22
buf	equ	0x23
max	equ	0x70
	movlw	high SSP#v(p)CON;
	movwf	FSR0H		; goto decl;
	zOS_MY2	FSR0L		;task:
	movlw	0xff		; fsr0 = 0x70 + (bsr<<1);
	movwf	0[fsr0]		; 0[fsr0] = 0xff; // live TMR0 postscaler divider
	movwf	1[fsr0]		; 1[fsr0] = 0xff; // reset value for TMR0
	movlw	low SSP#v(p)CON	;
	andlw	0x80		; const int8_t* SSP = (p==1)? SSP1CON : SSP2CON;
	movwf	FSR0L		; const int8_t* fsr0 = SSP & 0xff80; 
	rrf	zOS_ME		;
	clrw			; const char* max = 0x70;
	rrf	WREG,w		; static char *p0, *p1, buf[]; //po:task, p1:ISR
	iorlw	buf		; char* fsr1;
	movwf	wrap		; const char* wrap = ((bsr&1)<<7) | buf;
	movwf	p0		;
	movwf	p1		; p0 = p1 = wrap; // reset value if they max out
	
	local	sspbase,sspcon,sspstat,rtsflag
sspbase	equ	SSP#v(p)CON & 0xff80
sspcon	equ	SSP#v(p)CON & 0x1f
sspstat	equ	SSP#v(p)STAT & 0x1f
	if (p == 1)
	 ifdef (TX#v(p)REG)
sspxmit	  set	TX#v(p)REG	
	 else
sspxmit	  set	TXIF
	 endif
	 ifdef (TX#v(p)IF)
rtsflag	  set	TX#v(p)IF
	 else
rtsflag	  set	TXIF
	 endif
	endif
loop
	zOS_SWI	zOS_SLP
	movlw	high rts	; do {
	movwf	FSR1H		;  zOS_SWI(zOS_SLP);
	movlw	low rts		;  // wait for SWI to store char(s) in buf[]
	movwf	FSR1L		;  // or HWI to indicate the last one got sent
	btfss	INDF1,rtsflag	;  if (*(fsr1 = rts) & (1<<rtsflag) == 0)
	bra	loop		;   continue; // sleep (sending previous char)
	rrf	zOS_ME 		;  // READY TO SEND, AND...
	movwf	FSR1H		;
	movf	p0,w		;       
	movwf	FSR1L		;
	xorwf	p1,w		;  fsr1 = (bsr<<7) | p0;
	btfsc	STATUS,Z	;  if (p0 == p1)
	bra	loop		;   continue; // nothing to do, somehow (!)
	moviw	FSR1++		;  // ...WE HAVE SOMETHING IN THE BUFFER TO SEND
	movwi	sspxmit[FSR0]	;  sspxmit[fsr0] = *fsr1++; // send a character
	movf	FSR1L,w		;
	movwf	p0		;  p0 = fsr1 & 0x00ff); // wrap around to buf+0
	andlw	0x7f		;
	xorlw	max		;
	btfss	STATUS,Z	;
	bra	loop		;
	movf	wrap,w		;
	movwf	p0		;
	bra	loop		; } while (1);

isr
	;; SWI will be coming from a device that wants to send a character
	;;  in which case the ISR stores it, advances p1 and clears zOS_WAI bit
	;;  and thus should just keep task asleep if rtsflag not set
	;; HWI will be coming from the SSP peripheral indicating ready-to-send
	;;  in which case ISR clears the job's zOS_WAI bit to get scheduled
	;; (or HWI will come from a tmr0 expiration for the blinking heartbeat)
;;; FIXME : SWI has to clear the sleep bit!!!!!!!!!!!


	;; make sure the task is sleeping (else the above initialization unrun!)
	local hw,done
	zOS_MEM	FSR0,zOS_JOB,0
	moviw	zOS_PCH[FSR0]	;isr:
	btfss	WREG,zOS_WAI	; fsr0 = 0x10 * (1+zos_job);
	bra	done		; if (zOS_PCH[fsr0] & (1<<zOS_WAI)) {
	
	;; for speed, check for validated SWI first since it will be in zOS_MSK
	movf	zOS_MSK,f	;
	btfss	STATUS,Z	;
	bra	hw		;  if (zos_msk) {
	
	
	movlw	high SSP#v(p)CON;
	movwf	FSR0H		;
	movlw	low SSP#v(p)CON	;
	andlw	0x80		; const int8_t* SSP = (p==1)? SSP1CON : SSP2CON;
	movwf	FSR0L		; const int8_t* fsr0 = SSP & 0xff80; 
	
hw
	banksel	PIR1
	btfs_	PIR1,TXIF
	bra
	.
done
	zOS_RFI	zOS_HIM		;
	
;;; FIXME: need to set up the hardware port at the requested bps rate here first
	
	;; intialize the 
decl
	movwf	FSR0L		;decl:
	movlw	high task	;
	movwf	FSR0H		;  FSR0 = task & 0x7fff;// MSB 0 => unprivileged
	bcf	FSR0H,zOS_PRB	;
	movlw	low isr		;  w = zOS_ARG(0, isr & 0x00ff);
	zOS_ARG	0
	movlw	high isr	;  w = zOS_ARG(1, isr>>8);
	zOS_ARG	1
	movlw	1<<TXIF		;  w = zOS_ARG(2, 1<<TXIF);// OK, ready for more
	zOS_ARG	2
	endm			; }

;;; like zOS_CON, but also accepts console input for a full job-management shell
zOS_MON	macro	port,bps,hb,pin	;
	local task,decl,isr
	bra	decl		;
task
	.
	.
loop
	.
	.
	bra 	loop
isr
	.
	.
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
	

