ZOS_SWI	macro	type
	movlw	type
	movlp	0x00
	call	0x02
	endm

ZOS_ARG	macro	argnum
	local num
num set (argnum & 0x03)
	if (num == 0)
	 bcf	INTCON,GIE
	endif
	movwf	ZOS_AR#v(num)
	endm

ZOS_RUN	macro	t0enable
	;; start a TMR0 interrupt since none found (most in INTCON, others PIE0)
	if (t0enable)
	 banksel t0enable
	 bsf	t0enable,T0IE
	endif
	
	;; advance the stack pointer to allow 5 stacks of 3 each (+1 if running)
	banksel	STACKPTR
	movlw	0x0c
	movwf	STACKPTR
	
	;; set the active job to the first (and potentially only)
	movlb	1

	;; wait for stuff to happen
	local loop
loop
	bra	loop
	endm

;;; simple output-only console job with circular buffer
ZOS_CON	macro	port,bps	;inline int8_t ZOS_CON(int8_t port, int8_t bps){
	local task,decl,isr
	bra	decl		;task:
task
	local p0,p1,l,h,buf,max	;

p0 set 0x20
p1 set 0x21
l set 0x22
h set 0x23
buf equ 0x24
max equ 0x70
	movlw	buf		; const char* max = 0x70;
	movwf	p0		; static char *p0, *p1, buf[max - buf];
	movwf	p1		; p1 = p2 = buf; // won't need context again
;;; FIXME: need to remember the value of port, store useful record of it in h:l
	ZOS_SWI	ZOS_SLP		; ZOS_SWI(ZOS_SLP);
isr
	;; make sure the task is sleeping or else the above initialization unrun
	local hw,done
	ZOS_MEM	FSR0,ZOS_JOB,0
	moviw	ZOS_PCH[FSR0]	;isr:
	btfss	WREG,ZOS_WAI	; fsr0 = 0x10 * (1+zos_job);
	bra	done		; if (ZOS_PCH[fsr0] & (1<<ZOS_WAI)) {
	
	;; for speed, check for validated SWI first since it will be in ZOS_MSK
	movf	ZOS_MSK,f	;
	btfss	STATUS,Z	;
	bra	hw		;  if (zos_msk) {
	
	
hw
	banksel	PIR1
	btfs_	PIR1,TXIF
	bra
	.
done
	ZOS_RFI
decl
	movlw	low task	; } else {
;;; FIXME: need to set up the hardware port at the requested bps rate here first
	movwf	FSR0L		;
	movlw	high task	;
	movwf	FSR0H		;  FSR0 = task & 0x7fff; //MSB 0 => unprivileged
	bcf	FSR0H,ZOS_PRB	;
	movlw	low isr		;  w = ZOS_ARG(0, isr & 0x00ff);
	ZOS_ARG	0
	movlw	high isr	;  w = ZOS_ARG(1, isr>>8);
	ZOS_ARG	1
	movlw	1<<TXIF		;  w = ZOS_ARG(2, 1<<TXIF); // OK ready for more
	ZOS_ARG	2
	endm			; }

;;; like ZOS_CON, but also accepts console input for a full job-management shell
ZOS_MON	macro	port,bps,hb,pin	;
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
	ZOS_RFI
decl
	movlw	low task	; } else {
	movwf	FSR0L		;  //job#0 means ZOS_RUN not encountered yet 
	movlw	high task	;
	andlw	0x7f		;
	movwf	FSR0H		;  FSR0 = isr & 0x7fff; //MSB 0 => unprivileged
	movlw	low isr		;  w = ZOS_ARG(0, isr & 0x00ff)
	ZOS_ARG	0
	movlw	high isr	;  w = ZOS_ARG(1, isr >> 8);
	ZOS_ARG	1
	movlw	1<<RCIF		;
	iorlw	1<<TXIF		;  w = ZOS_ARG(2, (1<<RCIF) | (1<<TXIF));
	ZOS_ARG	2
	endm
	
;;; wind the circular stack around from the running job# to the new job#
;;; (before restoring the new job's STKPTR and copying its return address there)
ZOS_STK	macro	oldj,newj 	; typically "ZOS_STK SHAD_BSR,BSR"
	.
	.
	.
	endm
;;; calculate the starting address of the *next highest* job #'s stack bottom
	rlf	BSR,w
	bcf	WREG,0
	addwf	BSR,w
;;; running job#: 1         2         3         4         5         6
;;; stack pos 15: 3rd(1)    3rd(2)    3rd(3)    3rd(4)    3rd(5)    15th(6)
;;; stack pos 14: 2nd(1)    2nd(2)    2nd(3)    2nd(4)    2nd(5)    14th(6)
;;; stack pos 13: 1st(1)    1st(2)    1st(3)    1st(4)    1st(5)    13th(6)
;;; stack pos 12: 0th(1)    0th(2)    0th(3)    0th(4)    0th(5)    12th(6)
;;; stack pos 11: 2nd(5)    2nd(1)    2nd(2)    2nd(3)    2nd(4)    11th(6)
;;; stack pos 10: 1st(5)    1st(1)    1st(2)    1st(3)    1st(4)    10th(6)
;;; stack pos 9:  0th(5)    0th(1)    0th(2)    0th(3)    0th(4)    9th(6)
;;; stack pos 8:  2nd(4)    2nd(5)    2nd(1)    2nd(2)    2nd(3)    8th(6)
;;; stack pos 7:  1st(4)    1st(5)    1st(1)    1st(2)    1st(3)    7th(6)
;;; stack pos 6:  0th(4)    0th(5)    0th(1)    0th(2)    0th(3)    6th(6)
;;; stack pos 5:  2nd(3)    2nd(4)    2nd(5)    2nd(1)    2nd(2)    5th(6)
;;; stack pos 4:  1st(3)    1st(4)    1st(5)    1st(1)    1st(2)    4th(6)
;;; stack pos 3:  0th(3)    0th(4)    0th(5)    0th(1)    0th(2)    3rd(6)
;;; stack pos 2:  2nd(2)    2nd(3)    2nd(4)    2nd(5)    2nd(1)    2nd(6)
;;; stack pos 1:  1st(2)    1st(3)    1st(4)    1st(5)    1st(1)    1st(6)
;;; stack pos 0:  0th(2)    0th(3)    0th(4)    0th(5)    0th(1)    0th(6)

