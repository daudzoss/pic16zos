;;; zosmacro.inc
;;; potentially useful (but not mandatory) macros for zOS
;;;
;;; total memory footprint (for a PIC16F1847, including the zOS base):
;;; no memory words used upon inclusion (before expansion of a macro)
;;;  ~256 14-bit words if only zOS_CON() job is started to buffer console output
;;;  _??_ 14-bit words for full-featured monitor zOS_MON()
;;;  _??_ 14-bit words for job manager shell zOS_MAN()
	
#ifdef UCFG
#define	zOS_ME BSR,w : xorlw 0x8; // advance zOS use past DPSRAM; FIXME:untested
#else
#define zOS_ME BSR,w		; // "movf/andwf/xorwf zOS_ME" can't clobber BSR
#endif

zOS_GLO	macro	fsrnum,job
	local	fsrn
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif
	movlw	0x1f		;// documentation suggests 5 but BSR now 6 bits!
	andwf	FSR#v(fsrn)H,f	;inline int8_t zOS_GLO(int8_t* *fsrnum,
	if (job)
	 rlf	job,w		;                      int8_t* job) {
	else
	 rlf	zOS_ME		; int8_t w = 0x70 | ((job ? *job : bsr) << 1);
	endif
	andlw	0x0e		; *fsrnum = (*fsrnum & 0x1f00) | w;
	iorlw	0x70		; return w;
	movwf	FSR#v(fsrn)L	;} // zOS_GLO()
	endm
	
zOS_MY2	macro	fsrnum		;inline int8_t zOS_MY2(int8_t**fsrnum){
	zOS_GLO	fsrnum,0	; return zOS_GLO(fsrnum, 0);
	endm			;} // zOS_MY2()

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
	rrf	job,w		;
	movwf	FSR#v(fsrn)H	; return (*fsrnum = (job<<7) | offset) >> 8;
	rrf	FSR#v(fsrn)L,f	;} // zOS_LOC()
	endm

zOS_ADR	macro	adr,msb
	movlw	low adr		;inline void zOS_ADR(void* a) {
	movwf	FSR0L		; if (msb) fsr0 = 0x8000 | a;
	movlw	high adr	; else fsr0 = 0x7fff & a;
	movwf	FSR0H		;} // zOS_ADR()
	if (msb)
	 bsf	FSR0H,7
	else
	 bcf	FSR0H,7
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
	movwf	FSR0H		;} // zOS_INT()
	zOS_ARG	0
	zOS_ARG	1
	zOS_ARG	2
	zOS_ARG	3
	endif
	endm

zOS_SWI	macro	type		;inline void zOS_SWI(const int8_t type) {
	movlw	type		;
	movlp	0x00		; zos_swj(type);
	call	0x02		;} // zOS_SWI()
	endm

zOS_TAI	macro	type		;inline void zOS_TAI(const int8_t type) {
	movlw	type		; w = type; goto zos_skp;
	pagesel	zos_skp
	goto	zos_skp		;} // zOS_TAI()
	endm

zOS_LAU	macro	stash		;inline void zOS_LAU(int8_t* stash) {
	local retry
retry
	zOS_SWI	zOS_NEW
	movf	WREG,w		; do {
	btfsc	STATUS,Z	;  w = zOS_SWI(zOS_NEW);
	bra	retry		; } while (w == 0);
	if (stash - WREG)
	 movwf	stash		; *stash = w;
	endif
	endm			;} // zOS_LAU()

zOS_INI	macro	fsrnum,val0,val1
	if (fsrnum & 3)
fsrn	 set 1
	else
fsrn	 set 0
	endif
;after:	zOS_LAU	FSR#v(fsrn)L
	rlf	FSR#v(fsrn)L,f	;inline void zOS_INI(uint8_t* fsrnum, uint8_t
	movlw	0x70		;                    val0, uint8_t val1) {
	iorwf	FSR#v(fsrn)L,f	; //fsrnum starts and ends as a launched job#
	bcf	FSR#v(fsrn)L,0	; fsrnum = 0x70 | (fsrnum << 1);
	clrf	FSR#v(fsrn)H	;
	movlw	val0		; // change global mailbox to non-0 if desired
	movwi	FSR#v(fsrn)++	; fsrnum[0] = val0;
	movlw	val1		;
	movwi	FSR#v(fsrn)--	; fsrnum[1] = val1;
	rrf	FSR#v(fsrn),w	; fsrnum = (fsrnum >> 1) & 0x07; // unchanged
	andlw	0x07		;}
	endm


zOS_DIS	macro	fsr,job		;inline void zOS_DIS(int8_t* *fsr, int8_t job) {
	if (job)
	zOS_MEM	fsr,job,zOS_HDH	; *fsr = 0x10 * (1 + job) + zOS_HDH;//priv check
	 btfsc	INDF0,zOS_PRB	; if (**fsr & (1<<zOS_PRB))
	endif
	bcf	INTCON,GIE	;  INTCON &= ~(1<<GIE);
	endm			;} // zOS_DIS()
	
zOS_ENA	macro			;inline void zOS_ENA(void) {
	bsf	INTCON,GIE	; INTCON |= 1<<GIE;
	endm			;} // zOS_ENA()

zOS_ARG	macro	arg
	local num
num set (arg & 0x03)
	if (num == 0)
	 bcf	INTCON,GIE	;inline void zOS_ARG(const int8_t arg, int8_t w)
	endif
	movwf	zOS_AR#v(num)	;{if (!arg) INTCON &=~(1<<GIE); zOS_AR0[arg]=w;}
	endm

zOS_RUN	macro	t0enable,t0flags
	;; start a TMR0 interrupt since none found (most in INTCON, others PIE0)
	local boot
zOS_T0E	equ	t0enable
zOS_T0F	equ	t0flags	
	if (zOS_T0E)
	 banksel zOS_T0E
	 bsf	zOS_T0E,T0IE	;inline void zOS_RUN(uint8_t* t0enable) {
	  if (zOS_T0E - INTCON)
	   bsf	INTCON,PEIE	; if (t0enable) { *t0enable |= 1<<T0IE;
	 endif
	endif
	;; advance the stack pointer to allow 5 stacks of 3 each (+1 if running)
	banksel	STKPTR		;  if (t0enable !=  INTCON) INTCON |= 1<<PEIE;
	movlw	zOS_BOS		; }
	movwf	STKPTR		; STKPTR = zOS_BOS; // every job bottom of stack
	
	;; set the active job to the first (and potentially only), interrupts ON
	movlw	1+zOS_NUM	; bsr_shad = w = 1+zOS_NUM; // will wrap around
	movwf	BSR_SHAD	; boot(); // run the scheduler to grab its PC
	pagesel	boot		;} // zOS_RUN()
	call	boot		;
boot	
	bsf	INTCON,GIE	;void boot(void) { INTCON |= 1<<GIE; zOS_RFI();}
	zOS_RFI
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
	bra	loop		; STKPTR = -1;
	decf	STKPTR,f	; // still in job "0"
	movlb	0		;} // zOS_DBG()
	endm

#ifdef	PID1CON
;;; 16x16bit signed multiply zOS_AR1:0 * zOS_AR3:2, core yielded during 7ms math
zOS_MUL	macro	fsrnum
	local fn,inout,fac0L,fac0H,fac1L,fac1H,zeroH,start,con,setup,enb,bsy
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
	
	movlw	low PID1CON	;void zOS_MUL(int16_t** fsr) {
	movwf	FSR#v(fn)L	; *fsr = &PID1CON;
	movlw	high PID1CON	;
	movwf	FSR#v(fn)H	; do {
spinget
	btfss	INDF#v(fn),enb	;  while ((**fsr&(1<<enb))&& // MATHACC for sure
	bra	notbusy		;         (**fsr&(1<<bsy)))  // ours if not busy
	btfss	INDF#v(fn),bsy	;                            // or never enabled
	bra	notbusy		;
	zOS_SWI	zOS_YLD		;   zOS_SWI(zOS_YLD);
	bra	spinget		;   // interrupts now enabled if zOS_SWI called
notbusy
	bcf	INTCON,GIE	;  INTCON &= ~(1<<GIE);
	btfsc	INDF#v(fn),enb	;  // begin critical section (seizing MATHACC)
	bra	spinget		;
	bsf	INDF#v(fn),bsy	;
	bra	spinget		; } while ((**fsr&(1<<enb))||(**fsr&(1<<bsy)));
	movlw	setup		;
	movwf	indf#v(fn)	; **fsr = 1<<PIDMODE1; // unsigned mult no accum
	bsf	indf#v(fn),enb	; **fsr |= 1<<PID1EN; // selected, then enabled
	movlw	low inout	;
	movwf	FSR#v(fn)L	;
	movlw	high inout	;
	movwf	FSR#v(fn)H	; *fsr = &PID1SETL & 0x1f80; // just bank bits
	movf	zOS_AR3,w	;
	movwi	fac0H[FSR#v(fn)]; (0x1f & PID1K1H)[*fsr] = zOS_AR3;
	movf	zOS_AR2,w	;
	movwi	fac0L[FSR#v(fn)]; (0x1f & PID1K1L)[*fsr] = zOS_AR2;
	movf	zOS_AR1,w	;
	movwi	fac1H[FSR#v(fn)]; (0x1f & PID1SETH)[*fsr] = zOS_AR1;
	movf	zOS_AR0,w	;
	movwi	fac1L[FSR#v(fn)]; (0x1f & PID1SETL)[*fsr] = zOS_AR0;
	clrw			; (0x1f & PID1INH)[*fsr] = 0;
	movwi	zeroH[FSR#v(fn)]; (0x1f & PID1INL)[*fsr] = 0; // start multiply
	movwi	start[FSR#v(fn)]; // end critical section (seizing MATHACC)
	bsf	INTCON,GIE	; INTCON |= 1<<GIE;
	movlw	low PID1CON	;
	movwf	FSR#v(fn)L	;
	movlw	high PID1CON	; *fsr = &PID1CON;
	movwf	FSR#v(fn)H	; do {
spinmul
#if 0
	clrwdt			;  clrwdt();
#endif
	zOS_SWI	zOS_YLD
	btfss	INDF#v(fn),bsy	;  zOS_YLD();
	bra	spinmul		; } while (**fsr & 1<<PID1BUSY);
	bcf	INTCON,GIE	; INTCON &= ~(1<<GIE);
	bcf	INDF#v(fn),enb	; // begin critical section (copying result)
	movlw	low inout	; **fsr &= ~(1<<bsy); // disable MathACC to free
	movwf	FSR#v(fn)L	;
	movlw	high inout	;
	movwf	FSR#v(fn)H	; *fsr = &PID1SETL & 0x1f80; // just bank bits
	moviw	out3[FSR#v(fn)]	; zOS_AR3 = (0x1f & PID1OUTHH)[*fsr];
	movwf	zOS_AR3		;
	moviw	out2[FSR#v(fn)]	; zOS_AR2 = (0x1f & PID1OUTHL)[*fsr];
	movwf	zOS_AR2		;
	moviw	out1[FSR#v(fn)]	; zOS_AR1 = (0x1f & PID1OUTLH)[*fsr];
	movwf	zOS_AR1		;
	moviw	out0[FSR#v(fn)]	; zOS_AR0 = (0x1f & PID1OUTLL)[*fsr];
	movwf	zOS_AR0		; INTCON |= 1<<GIE; // end critical section
	bsf	INTCON,GIE	;} // zOS_MUL()
	endm
#endif

zOS_PAG	macro	fsrnum
	local	fsrn
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif

	swapf	FSR#v(fsrn)L,w	;uint8_t zOS_PAG(void* fsrnum) {
	andlw	0x0f		;
	bcf	FSR#v(fsrn)L,4	;
	swapf	FSR#v(fsrn)L,f	; 
	iorwf	FSR#v(fsrn)L,w	;
	swapf	FSR#v(fsrn)L,f	; return w = (fsrnum >> 4);
	bsf	FSR#v(fsrn)L,4	;} // zOS_PAG()
	endm
	
zOS_PTR	macro	fsrnum
	local	fsrn
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif

	swapf	WREG,w	     	;void zOS_PTR(void** fsrnum, uint8_t w) {
	movwf	FSR#v(fsrn)H	;
	movwf	FSR#v(fsrn)L	;
	movlw	0x0f		;
	andwf	FSR#v(fsrn)H,f	;
	bsf	FSR#v(fsrn)H,4	;
	movlw	0xf0		; *fsrnum = 0x2000 | w<<4;
	andwf	FSR#v(fsrn)L,f	;} // zOS_PTR()
	endm

;;; must be defined with 2 SWI flags: one for malloc(), a different for free()
;;; (typically instantiated with base=0x2210, size = memory size - base)
;;; SWI behavior for malloc(w) is to return pointer in w of 2 middle nybbles
;;;  in linear address space, e.g. 0x21 for first cell on a 5-job system, or 0
;;;  in w if no free memory of size zOS_AR0*16 bytes was available
;;; SWI behavior for free(w) is to return in w the number of bytes now free/16
;;;  intersecting with the address whose middle nybble is zOS_AR0, or 0 in w if
;;;  zOS_AR0 didn't point to a valid (i.e. previously allocated) block of bytes
;;; 
;;; FIXME: demo idea would be two heap allocators running for two differently
;;; targeted (quantum) allocation heaps, leaving final SWI remaining for zOS_CON
zOS_HEA	macro	base,size,mi,fi	;void zOS_HEA(void* base, void* size, uint8_t
	local	isr,decl,task	;             mi/*malloc*/,uint8_t fi/*free*/) {

	bra	decl		; goto decl;

	local	maxnon0,temp1,always0,temp,adrarry,sizarry
	local	tblsize,tblrows,memroun,membase,memsize
maxnon0	set	0x6c
temp1	set	0x6d
always0	set	0x6e
temp	set	0x6f	
adrarry	set	0x20
tblsize set	0x50
tblrows	set	tblsize/2	
sizarry	set	adrarry+tblrows
memroun	set	base+0xf
mem3nyb	set	memroun&0xfff
membase	set	mem3nyb>>4
memsize	set	size>>4
	
isr
	local	mloop,mnoroom,mcandid,mnotall,groloop
	local	free,floop,ffound,invalid,done
	
	movf	zOS_JOB,w	; isr:
	movwf	BSR		; bsr = zOS_JOB;

	zOS_MY2	FSR1		; fsr1 = 0x70|(bsr<<1);
	moviw	FSR1++		;
	iorwf	INDF1,w		;
	btfsc	STATUS,Z	; if (0[fsr1] | 1[fsr1])
	bra	invalid		;  goto invalid;// not init'ed according to mbox

	movf	zOS_MSK,w	;
	andlw	mi		; //////////////////////////////////////////////
	btfsc	STATUS,Z	; /////             malloc()                  //
	bra	free		; if (zOS_MSK & mi) {

	zOS_LOC	FSR0,BSR,adrarry;  for (fsr0 = (bsr<<7)+adrarry,
	zOS_LOC	FSR1,BSR,sizarry;       fsr1 = (bsr<<7)+sizarry;
mloop
	moviw	FSR0++		;       (temp = *fsr0++);// next allocated/-able
	btfss	STATUS,Z	;       fsr1++) {
	bra	invalid		;
	movwf	temp		;
	moviw	FSR1++		;   w = *fsr1++; // number of bytes used,0=freed
	btfsc	STATUS,Z	;
	bra	mcandid		;   if (w == 0) { // allocatable
mnoroom
	movlw	adrarry+tblrows	;
	xorwf	FSR0L,w		;
	andlw	0x7f		;
	btfsc	STATUS,Z	;
	bra	mloop		;
mcandid
	moviw	0[FSR0]		;    w = *fsr0;// upper limit to allocating here
	btfss	STATUS,Z	;    if (w == 0)
	bra	invalid		;     goto invalid; // past the highest address
	subwf	temp,w		;
	movwf	temp		;
	subwf	zOS_AR0,w	;
	btfsc	WREG,7		;    else if ((temp = w - temp) < zOS_AR0)
	bra	mnoroom		;     continue; // not enough allocatable here
	btfss	STATUS,Z	;
	bra	mnotall		;
#if 0
	movf	FSR1L,w		;// dynamically use the mailbox as range for mgt
	movwf	temp1		;
	zOS_MY2	FSR1		;
	moviw	-1[FSR0]	;
	subwf	INDF1,w		;
	btfsc	WREG,7		;
	bra	mnoroom		;
	addfsr	FSR1,+1		;
	moviw	-1[FSR0]	;
	subwf	INDF1,w		;
	btfss	WREG,7		;
	bra	mnoroom		;
	zOS_LOC	FSR1,BSR,sizarry;
	movf	temp1,w		;
	movwf	FSR1L		;
#endif	
	movf	temp,w		;    if (temp == zOS_AR0) { // exactly enough!
	movwi	-1[FSR1]	;     w = -1[fsr1] = temp;
	bra	done		;     goto done;
mnotall	
	movf	maxnon0,f	;    } else if (adrarry[tblrows-2] == 0) // full
	btfss	STATUS,Z	;     goto invalid;
	bra	invalid		;
	movwf	temp		;    -1[fsr1] = zOS_AR0;
	movf	zOS_AR0,w	;    for (w = -1[fsr0] + (temp -= zOS_AR0);
#if 0
	movf	FSR1L,w		; // dynamically use the mailbox as range for mgt
	movwf	temp1		;
	zOS_MY2	FSR1		;
	moviw	-1[FSR0]	;
	subwf	INDF1,w		;
	btfsc	WREG,7		;
	bra	mnoroom		;
	addfsr	FSR1,+1		;
	moviw	-1[FSR0]	;
	subwf	INDF1,w		;
	btfss	WREG,7		;
	bra	mnoroom		;
	zOS_LOC	FSR1,BSR,sizarry;
	movf	temp1,w		;
	movwf	FSR1L		;
#endif	
	movwi	-1[FSR1]	; // w == addr to insert, temp == size to insert
	moviw	-1[FSR0]	;         *fsr0;
	addwf	temp,w		;         fsr0++, fsr1++) {
groloop		
	xorwf	INDF0,f		;   // w  == contents for inserted cell for fsr0
	xorwf	INDF0,w		;   // *fsr0 == contents to overwrite in fsr0
	xorwf	INDF0,f		;     swap(&w, fsr0);

	xorwf	temp,f		;   // w == contents just overwritten in fsr0
	xorwf	temp,w		;   // temp == contents for inserted cell (fsr1)
	xorwf	temp,f		;     swap(&w, &temp);
	
	xorwf	INDF1,f		;   // w == contents for inserted cell in fsr1
	xorwf	INDF1,w		;   // *fsr1 == contents to overwrite in fsr1
	xorwf	INDF1,f		;     swap(&w, fsr1);

	xorwf	temp,f		;   // w == contents just overwritten in fsr1
	xorwf	temp,w		;   // temp == contents just overwritten in fsr0
	xorwf	temp,f		;     swap(&w, &temp);

	addfsr	FSR0,+1		;   // w == contents just overwritten in fsr0
	addfsr	FSR1,+1		;   // temp = contents just overwritten in fsr1

	movf	INDF0,f		;
	btfss	STATUS,Z	;
	bra	groloop		;    }
	
	movwi	0[FSR0]		;    // append the final overwritten contents
	movf	temp,w		;    *fsr0 = w; // this will be maxnon0 for last
	movwi	0[FSR1]		;    *fsr1 = temp;
	movf	zOS_AR0,w	;    w = zOS_AR0;
	bra	done		;    goto done;

free
	movf	zOS_MSK,w	; //////////////////////////////////////////////
	andlw	fi		; //////////       free()                ///////
	btfss	STATUS,Z	;
	bra	invalid		; } else if (zOS_MSK & fi)

	zOS_LOC	FSR0,BSR,adrarry
floop
	moviw	FSR0++		;  for (fsr0 = (bsr<<7) + adrarry;
	xorwf	zOS_AR0,w	;       fsr0 < adrarry + tblrows;//FIXME:sorted!
	btfsc	STATUS,Z	;       fsr0++)              //could quit early!
	bra	ffound		;
	movlw	adrarry+tblrows ;
	xorwf	FSR0L,w		;
	andlw	0x7f		;
	btfsc	STATUS,Z	;
	bra	floop		;

	bra	invalid		;   if (*fsr0 == zOS_AR0) {
ffound
	if (tblrows & 0x20)
	 addfsr	FSR0,0x1f	;
	 addfsr	FSR0,tblrows-0x1f;
	else
	 addfsr	FSR0,tblrows	;    fsr0 = sizarry + (fsr0 - adrarry);
	endif
	moviw	--FSR0		;    w = *--fsr0;
	clrf	INDF0		;    *fsr0 = 0;
	bra	done		;   }
invalid
	clrw			; else invalid: w = 0; // can't malloc nor free
done
	zOS_RFS	WREG		; done: return w;
	
	
task
	local	iniarry,coalesc,coaloop,coscoot
	
	zOS_DIS	GIE,0
	zOS_LOC	FSR0,BSR,0x70
iniarry
	clrw			; INTCON &= ~(1<<GIE);
	movwi	--FSR0		; for (fsr0 = (bsr<<7)|(adrarry+tblsize);
	movlw	adrarry		;      fsr > adrarry; fsr--)
	xorwf	FSR0L,w		;  *fsr = 0; // zero each address and size entry
	btfss	STATUS,Z	;
	bra	iniarry		;
	
	zOS_MY2	FSR1

	movlw	membase		; // except first address entry is start of heap
	movwi	0[FSR1]		; (0x70|(bsr<<1))[0] =
	movwi	0[FSR0]		; adrarry[0] = membase; // first allocatable
	movlw	membase+memsize	; // and second addres entry is the end of heap
	movwi	1[FSR1]		; (0x70|(bsr<<1))[1] =
	movwi	2[FSR0]		; adrarry[2] = membase+memsize;//max allocatable
	zOS_ENA

coalesc
	zOS_SWI	zOS_YLD
	zOS_LOC	FSR0,BSR,adrarry+1
	zOS_LOC	FSR1,BSR,sizarry
coaloop
	moviw	++FSR0		; do { // combine adjacent rows whose size are 0
	btfsc	STATUS,Z	;  zOS_SWI(zOS_YLD); // only 1 pass per schedule
	bra	coalesc		;  for (fsr0 = &adrarry[1], fsr1 = &sizarry[0];
	moviw	FSR1++		;       *++fsr0;
	btfss	STATUS,Z	;       fsr1++)
	bra	coaloop		;   if (0[fsr1] === 0 && 1[fsr1] == 0) {
	moviw	0[FSR1]		;    // fsr1->redundant row siz, trails fsr0->adr
	btfss	STATUS,Z	;    do {
	bra	coaloop		;     uint8_t w = *++fsr1;
coscoot
	moviw	++FSR1		;     -1[fsr1] = w;
	movwi	-1[FSR1]	;     w = *fsr0++;
	moviw	FSR0++		;    } while ((-2[fsr0] = w) != 0);
	movwi	-2[FSR0]	;    break;
	btfss	STATUS,Z	;   }
	bra	coscoot		; } while (1);
	bra	coalesc		;decl:

decl
	zOS_ADR	task,zOS_UNP	; fsr0 = task & 0x7fff;// MSB 0 => unprivileged
	movlw	low isr		; w = zOS_ARG(0, isr & 0x00ff);
	zOS_ARG	0
	movlw	high isr	; w = zOS_ARG(1, isr>>8);
	zOS_ARG	1
	movlw	0		; w = zOS_ARG(2, 0); // no hardware interrupts
	zOS_ARG	2
	movlb	0		; // still in job "0": don't forget this!!!!
#if 0
	movlw	mi|fi		; w = zOS_ARG(3, mi/*malloc()*/ | fi/*free()*/);
	zOS_ARG	3
	zOS_LAU	FSR0
#endif	
	endm			;} // zOS_HEA()
	
;;; simple output-only console job with circular buffer
zOS_HEX	macro
	andlw	0x0f		;
	addlw	0x06		;
	btfsc	WREG,4		;inline char zOS_HEX(uint8_t w) { 
	addlw	0x07		; return (w & 0x0f > 9) ? '0'+w : 'A'+w-10;
	addlw	0x2a		;} // zOS_HEX()
	endm
	
zOS_IHF	macro	ofs,fsrsrc,fsrdst
	local	src,dst
	if (fsrsrc & 3)
src set 1	
	else
src set 0
	endif
	if (fsrdst & 3)
dst set 1	
	else
dst set 0
	endif
	
	moviw	ofs[FSR#v(src)]	;inline void zOS_IHF(int8_t ofs, int fsrnum,
	swapf	WREG,w		;                                char* file) {
	zOS_HEX
	movwi	FSR#v(dst)++	; file[0] = zOS_HEX(ofs[fsrnum] >> 4);
	moviw	ofs[FSR#v(src)]	; file[1] = zOS_HEX(ofs[fsrnum]);
	zOS_HEX
	movwi	FSR#v(dst)++	;}
	endm


zOS_PUT	macro	max
	movwi	FSR0++		;inline void zOS_PUT(char* max, char w) {
	movf	FSR0L,w		; // fsr0 must be pointing into buffer at p0
	andlw	0x7f		; // fsr1 must be pointing @variables p0,p1,wrap
	xorlw	max		; *fsr0++ = w;
	btfss	STATUS,Z	;
	bra	$+3		; if (fsr0 & 0x7f == max)
	moviw	2[FSR1]		;  fsr0 = 2[fsr1] /*wrap*/;
	movwf	FSR0L		;
	movf	FSR0L,w		; 1[fsr1] /*p1*/ = fsr0 & 0x00ff;
	movwi	1[FSR1]		;} // zOS_PUT()
	endm

zOS_UNW	macro	job		;inline void zOS_UNW(int8_t job) { }
	zOS_MEM	FSR0,job,zOS_PCH; fsr0 = 0x10 * (1 + job) + zOS_PCH;
	bcf	INDF0,zOS_WAI	; *fsr0 &= ~(1 << zOS_WAI); // now runnable
	endm			;} // zOS_UNW()

#if 0
;;; FIXME: major reworking needed still
zOS_ASC	macro	file,str,nulterm;inline uint8_t zOS_ASC(char*file, const char*
	local	start,out
	variable i,sum,prev,curr
prev = 0xff
i = 0
	while i < (out-start)	; for (int i=0; str[i]||(nulterm&&!str[i]); i++)
sum = start
curr = high sum
	if curr-prev	
	 pagesel sum
	endif
	call	sum		;  file[i] = str[i];
	movwf	file+i		; return i;
prev = curr
sum += 1
	endw
	movlw	i+nulterm	;}
	bra	out		;                       str, int nulterm) {
start
	dt	str
	if nulterm
	 dt	0
	endif
out
	endm

#if 1
zOS_OUT	macro	swinum,string	; // 8 words per byte (+1) to avoid using stack
	local start,out
	if (out-start) > 255
	 pagesel out
	 goto	out		;inline void zOS_OUT(int8_t swinum,
	else
	 bra	out		; const char* string){//unpacked (dt) with retlw
	endif
start
	dt	string
out		
	variable i,sum,prev,curr
prev = 0xff
i = 0
	while i < (out-start)	; for (int i = 0; i < strlen(string); i++) {
sum = i+start
curr = high sum
	if curr-prev
	 pagesel sum
	endif
	call	sum		;  zOS_ARG(0, *(string[i])());
	zOS_ARG	0		;  zOS_SWI(swinum);
	zOS_SWI	swinum		; }
prev = curr	
i += 1
	endw			;
	endm			;} // zOS_OUT()
#else
zOS_OUT	macro	swinum,revstr,temp; // 1 word per byte (+13) to use stack+file
	local	pre,post,callnxt,offset,loop
offset	set	callnxt
	movlw	post-pre	;inline void zOS_OUT(int8_t swinum, const char*
	movwf	temp		;                    revstr, int8_t* temp) {
loop
	decfsz	temp,f		; static const s[] = revstr;
	bra	post		;
	movf	temp,w		; for (*temp = strlen(revstr); *temp; (*temp)--){
	addlw	offset		;  zOS_ARG(0, s[*temp]);
callnxt
	callw	;<---probably wrong since PCLATH unset!	;  zOS_SWI(swinum);
	zOS_ARG	0
	zOS_SWI	swinum
	bra	loop		; } // zOS_ARG 0 is 2, zOS_SWI 3 words, bra 1
pre
	dt	revstr		;} // zOS_OUT()
post
#endif
#endif

zOS_PSH	macro	reg
	movf	zOS_ME		;inline void zOS_PSH(uint8_t* reg) {
	;; bcf	INTCON,GIE
	banksel	TOSH
	incf	STKPTR,f	; STKPTR++;// caller should've masked interrupts
	movwf	TOSH		; TOSH = bsr;// must store bsr so we can go back
	if (reg-BSR)
	 movf	reg,w		; if (reg != &bsr)
	 movwf	TOSL		;  TOSL = *reg;
	 movf	TOSH,w		; bsr = TOSH;
	endif
	movwf	BSR		;} // zOS_PSH()
	;; bsf	INTCON,GIE
	endm

zOS_POP	macro	reg
	;; bcf	INTCON,GIE
	banksel	STKPTR
	if (reg-BSR)
	 movf	TOSL,w		;inline void zOS_POP(uint8_t* reg) {
	 movwf	reg		; if (reg != &bsr) *reg = TOSL;
	endif
	movf	TOSH,w		; bsr = TOSH;
	decf	STKPTR,f	; STKPTR--;// caller should've masked interrupts
	movwf	BSR		;} // zOS_POP()
	;; bsf	INTCON,GIE
	endm
	
zOS_RDF macro
#ifdef EEADRL
zOS_ADL	equ	EEADRL
zOS_ADH	equ	EEADRH
zOS_RDL	equ	EEDATL
zOS_RDH	equ	EEDATH
	bcf	EECON1,CFGS	;inline void zOS_RDF(void) { // for EEADR micros
	bsf	EECON1,EEPGD	; EECON1 &= ~(1<<CFGS);
	bsf	EECON1,RD	; EECON1 |= 1<<EEPGD;
	nop			; EECON1 |= 1<<RD;
	nop			;} // zOS_RDF()
#else
#ifdef PMADRL
zOS_ADL	equ	PMADRL
zOS_ADH	equ	PMADRH
zOS_RDL	equ	PMDATL
zOS_RDH	equ	PMDATH
	bcf	PMCON1,CFGS	;inline void zOS_RDF(void) { // for PMADR micros
	bsf	PMCON1,RD	; PMCON1 &= ~(1<<CFGS);
	nop			; PMCON1 |= 1<<RD;
	nop			;} // zOS_RDF()
#else
#ifdef NVMADRL
zOS_ADL	equ	NVMADRL
zOS_ADH	equ	NVMADRH
zOS_RDL	equ	NVMDATL
zOS_RDH	equ	NVMDATH
	bcf	NVMCON1,NVMREGS	;inline void zOS_RDF(void) { // for NVM micros
	bsf	NVMCON1,RD	; NVMCON1 &= ~(1<<CFGS); NVMCON1 |= 1<<RD;
#endif
#endif
#endif
	endm			;} // zOS_RDF()
	
zOS_STR	macro	swinum
	local loop,readl,readh,done
	bcf	INTCON,GIE	;inline void zOS_STR(const char* fsr0,
	zOS_PSH	BSR
	banksel	zOS_ADL
	movf	FSR0L,w		;                    uint8_t swinum) {
	movwf	zOS_ADL		; INTCON &= ~(1<<GIE);
	movf	FSR0H,w		; zOS_PSH(&bsr); // need a bank change for reads
	movwf	zOS_ADH		; for (zOS_AD = fsr0; *zOS_AD; zOS_AD++) {
loop	
	zOS_RDF
	rlf	zOS_RDL,w	;  zOS_RDF(); // read packed 14-bit contents
	rlf	zOS_RDH,w	;
	btfsc	STATUS,Z	;
	bra	done		;  if ((w = (zOS_RDH<<1)|(zOS_RDL>>7)) != '\0'){
	movwf	zOS_AR0		;   zOS_ARG(0, w);
	zOS_POP	BSR
	zOS_SWI swinum	   	;
	bcf	INTCON,GIE	;   zOS_POP(&bsr); // back to the expected bank
	zOS_PSH	BSR
	banksel	zOS_ADL
	movf	zOS_RDL,w	;   zOS_SWI(swinum);; // print the ASCII char
	andlw	0x7f		;   INTCON &= ~(1<<GIE); // undo SWI GIE toggle
	btfsc	STATUS,Z	;   zOS_PSH(&bsr);
	bra	done		;   if ((w = zOS_RDL & 0x7f) != '\0') {
	movwf	zOS_AR0		;    zOS_ARG(0, w);
	zOS_POP	BSR
	zOS_SWI	swinum
	bcf	INTCON,GIE	;    zOS_POP(&bsr); // back to the expected bank
	zOS_PSH	BSR
	banksel	zOS_ADL
	incfsz	zOS_ADL,f	;    zOS_SWI(swinum); // print the ASCII char
	bra	loop		;    INTCON &= ~(1<<GIE); // undo SWI GIE toggle
	incf	zOS_ADH,f	;    zOS_PSH(&bsr);
	bra	loop		;   } else break;
done
	zOS_POP	BSR		;  } else break;
	bsf	INTCON,GIE	; } zOS_POP(&bsr); INTCON |= 1<<GIE;
	endm			;} // zOS_STR()

	
zOS_BUF	macro	job,buf,ptr	;inline int zOS_BUF(uint3_t job, uint8_t ptr) {
	local	ascii,err,done
	zOS_LOC	FSR1,job,ptr
	movwf	FSR0H		; // ASCII parameter in zOS_AR0, zOS_AR1 for hex
	movlw	0x80		; fsr0 = zOS_LOC(fsr1,job,ptr)<<8;//(job<<7)|ptr
	andwf	FSR1L,w		; // fsr1 now points at ptr variables p0,p1,wrap
	iorwf	INDF1,w		; fsr0 |= (fsr1 & 0x0080) ? 0x0080 : 0;
	movwf	FSR0L		; fsr0 |= *fsr1; // fsr0 now points into buf @p0
	
	;; check to make sure there are at least 2 characters free in the buffer
	moviw	1[FSR1]		; fsrnum = (zOS_JOB << 7) + p0;
	andlw	0x7f		; char* p1plus2 = 2 + (1[fsrnum] /*p1*/ & 0x7f);
	addlw	0x12		;
	btfss	WREG,7		; if (p1plus2 >= max)
	addlw	0x90+buf	;  p1plus2 -= (max - buf);
	addlw	0-0x10		;
	bcf	INDF1,7		;
	subwf	INDF1,w		; char* w = p1plus2 - (0[fsrnum] /*p0*/&= 0x7f);
	
	incf	FSR1L,f		; // don't clobber w: OK if it's not 0 or 1
	btfsc	INDF1,7		;
	bra	$+4		; if (1[fsrnum++] /*p1*/ & 0x80) {
	decf	FSR1L,f		;  0[--fsrnum] /*p0*/ |= 0x80; // p0 restored
	bsf	INDF1,7		;  fsrnum++; // cancels the above decrement
	incf	FSR1L,f		; }
	decf	FSR1L,f		; fsrnum--; // cancels increment from the "if"

	iorlw	0x00		;
	btfsc	STATUS,Z	;
	bra	err		; if (w == 0)
	decf	WREG,w		;  goto err;// would wrap around, appear empty
	btfsc	STATUS,Z	; else if (w == 1)
	bra	err		;  goto err; // would wrap around to be size 1

	moviw	1[FSR1]		; fsr0 = 1[fsrnum]; // stop examining p1 and use
	movwf	FSR0L		;
	movf	zOS_AR0,w	; // we're now certain we won't exceed the buf
	btfss	STATUS,Z	;
	bra	ascii		; if (zOS_AR0 == 0) { // print zOS_AR1 as hex
	
	swapf	zOS_AR1,w	;  zOS_PUT(max, zOS_HEX(zOS_AR1 >> 4));
	zOS_HEX
	zOS_PUT	0x70
	movf	zOS_AR1,w	;  zOS_PUT(max, zOS_HEX(zOS_AR1 >> 0));
	zOS_HEX
	zOS_PUT	0x70
	movlw	2		;  return 2 /* characters added */;
	bra	done		; } else {
ascii
	zOS_PUT	0x70		;  zOS_PUT(max, zOS_AR0);
	movlw	1		;  return 1 /* character added */;
	bra	done		; }
err
	clrw			; err: return 0 /* characters added */;
done
	endm			;} // zOS_BUF()

zOS_NUL	macro	hwflag
	bra	decl
	local	task,isr,decl
task
	zOS_SWI	zOS_YLD
	bra	task
	
isr
	banksel	zOS_T0F
	bcf	zOS_T0F,T0IF	;  zOS_T0F &= ~(1<<T0IF);// clear interrupt flag
	zOS_RFI
	
decl
	zOS_ADR	task,zOS_UNP	; fsr0 = task & 0x7fff;// MSB 0 => unprivileged
	movlw	low isr		; w = zOS_ARG(0, isr & 0x00ff);
	zOS_ARG	0
	movlw	high isr	; w = zOS_ARG(1, isr>>8);
	zOS_ARG	1		; w = zOS_ARG(2, 1<<T0IF);
	movlw	hwflag		;
	zOS_ARG	2
	clrw		  	;
	zOS_ARG	3
	movlb	0		; // still in job "0": don't forget this!!!!

	endm

zOS_CON	macro	p,rat,rts,hb,pin;inline void zOS_CON(int8_t p,int8_t rat,int8_t
	local	task,isr,loop,decl
	bra	decl		;                    rts,int8_t* hb,int8_t pin){
	
	;; initialize constants and variables
	local	t0div,t0rst
t0div	set 0
t0rst	set 1
	
	local	p0,p1,wrap,t0scale,isradrl,isradrh,tskadrl,tskadrh,accumul
	local	accumuh,numbase,destreg,destreh,char_io,buf,max

	;; 0x20~24 reserved for zOS_CON
p0	set	0x20
p1	set	0x21
wrap	set	0x22
t0scale	set	0x23
	
	;; 0x24~28 reserved for zOS_INP
isradrl	set	0x24
isradrh	set	0x25
tskadrl	set	0x26
tskadrh	set	0x27

	;; 0x28~2F reserved for zOS_MON and derivations e.g. zOS_MAN
optadrl	set	0x28
optadrh	set	0x29
accumul	set	0x2a
accumuh	set	0x2b
numbase	set	0x2c	
destreg	set	0x2d
destreh	set	0x2e
char_io	set	0x2f
buf	set	0x30
max	set	0x70		;FIXME: "max" has no purpose (just advisory that
				; local variable space is capped at the bottom
				; of the globals), so it can be nixed

;copy the preceding lines rather than including this file, as definitions for
;zOS_MON()-derived macros referring to these local variables wouldn't open it
;until expansion and would throw an undefined-var error during the processing

	local	uatbase,uatxmit
	if (p == 0)
uatbase	 set	TXREG & 0xff80
uatxmit	 set	TXREG & 0x007f
rtsflag	 set	TXIF	
	else
uatbase	 set	TX#v(p)REG & 0xff80
uatxmit	 set	TX#v(p)REG & 0x007f
rtsflag	 set	TX#v(p)IF	
	endif
task
	zOS_DIS	FSR0,zOS_JOB	; goto decl;
	movlw	high uatbase	;// all init that requires knowledge of BSR here
	movwf	FSR0H		;task:
	zOS_MY2	FSR0		; zOS_DIS(&fsr0, zOS_JOB); // interrupts off!
	movlw	0xff		; fsr0 = 0x70 + (bsr<<1);//global always visible
	movwi	t0div[FSR0]	; 0[fsr0] = 0xff;// live TMR0 postscaler divider
	movlw	0x00		;
	movwi	t0rst[FSR0]	; 1[fsr0] = 0x00; // live reset value for TMR0
	movlw	low uatbase	;
	movwf	FSR0L		; const int8_t* uatbase = uatxmit & 0xff80;
	rrf	zOS_ME		; fsr0 = uatbase;
	clrw			; const char* max = 0x70;
	rrf	WREG,w		; static char *p0, *p1, buf[]; //p0:task, p1:ISR
	iorlw	buf		;
	movwf	wrap		; const char* wrap = ((bsr&1)<<7) | buf;
	movwf	p0		;
	movwf	p1		; p0 = p1 = wrap; // reset value if they max out
	zOS_ENA			; zOS_ENA(); // interrupts on after init done
#if 0
	zOS_ASC	buf,"\r\nWelcome to zOS\r\n",1
	;; FIXME: zOS_ASC won't build under MPASM
	addwf	p1,f		; p1 += strlen(strcpy(buf,"\r\nzOS>")) + 1;
#endif
	
loop
	zOS_SWI	zOS_YLD		; do {
	movlw	high rts	;  zOS_YLD();
 	movwf	FSR1H		;
	movlw	low rts		;  // wait for SWI to store char(s) in buf[]
	movwf	FSR1L		;
	btfss	INDF1,rtsflag	;  if (*(fsr1 = rts) & (1<<rtsflag) == 0) //full
	bra	loop		;   continue; // yield (still sending or no char)
	rrf	zOS_ME 		;
	andlw	0x7f		;
	movwf	FSR1H		;
	movf	p0,w		;  // READY TO SEND, AND...       
	movwf	FSR1L		;
	xorwf	p1,w		;  fsr1 = (bsr<<7) | p0;
	btfsc	STATUS,Z	;  if (p0 == p1)
	bra	loop		;   continue; // nothing to do
	moviw	FSR1++		;
	movwi	uatxmit[FSR0]	;  uatxmit[fsr0] = *fsr1++; // send a character
	movf	FSR1L,w		;
	movwf	p0		;  p0 = fsr1 & 0x00ff; // wrap around to buf+0
	andlw	0x7f		;
	xorlw	max		;
	btfss	STATUS,Z	;
	bra	loop		;  if (p0 & 0x7f == max) // ignore low bank bit
	movf	wrap,w		;   p0 = wrap; // =buf xor the lowest bank bit
	movwf	p0		;
	bra	loop		; } while (1);

	;; HWI will be coming from a tmr0 expiration, for the blinking heartbeat
	;; 
	;; SWI will be coming from a job that wants to send a character
	;;  in which case the ISR stores it, advancing p1 and returning the
	;;  number of characters stored in the buffer
	;; Note: caller needs to make sure to check status of return value for
	;; != 0, just in case job is in between sleeps or with a full buffer
isr
	local	done,do_swi,nottmr
	
	;; get fsr0 pointing to tmr0 postscaler/reset value
	movf	zOS_JOB,w	;isr:
	movwf	BSR		; bsr = zOS_JOB; // isr starts with unknown bank
	movlw	high uatxmit	;
	movwf	FSR0H		;
	zOS_MY2	FSR0L		; fsr0 = 0x70 | (bsr << 1);

	;; if it's a simple and frequent timer overflow interrupt finish quickly
	banksel	zOS_T0F
	btfss	zOS_T0F,T0IF	; if (/*presumed true:(zOS_T0E & (1<<T0IE)) &&*/
	bra	nottmr		;     (zOS_T0F & (1<<T0IF))) { // timer overflow
	bcf	zOS_T0F,T0IF	;  zOS_T0F &= ~(1<<T0IF);// clear interrupt flag

	;; with fsr0 pointing to global pair, point fsr1 to local mem("t0scale")
	zOS_LOC	FSR1,zOS_JOB,t0scale
	banksel	TMR0
	moviw	t0rst[FSR0]	;  fsr1 = (zOS_JOB << 7) | t0scale;
	btfss	WREG,7		;  bsr = TMR0 >> 7;//now invalid for this branch
	movwf	TMR0		;  if (t0rst[fsr0] < 128)// max 7 bit TMR0 reset
	decfsz	INDF1,f		;   TMR0 = t0rst[fsr0]; // or chance of deadlock
	bra	done		;  if (--*fsr1 == 0) {  

	banksel	hb
	movf	INDF0,w		;
	btfsc	STATUS,Z	;
	incf	INDF0,w		;   if (*fsr0 == 0)
	movwf	INDF0		;    *fsr0 = 1;
	movwf	INDF1		;   *fsr1 /*countdown*/ = *fsr0 /*postscaler*/;
	movlw	(1<<pin)	;
	xorwf	hb,f		;   hb ^= 1 << pin;
	bra	done		; } else {
	
	;; check for validated SWI first since it will be in zOS_MSK, else a HWI
nottmr
	movf	zOS_MSK,f	;  if (zOS_MSK) { // a SWI to buffer a character
	btfss	STATUS,Z	;   w = zOS_BUF(zos_job, p0); /*prints zOS_AR0*/
	bra	do_swi		;   zOS_RFS(w);
	zOS_RET
	
	;; point fsr0 to uatbase again, point fsr1 to p0
do_swi
	zOS_BUF	zOS_JOB,buf,p0	;  } else done:
	zOS_RFS	WREG		;   zOS_RFI(); // HWI finished
done
	zOS_RFI			; }
	;; intialize the UART peripheral, job handle and first three arguments
decl
#if 0
	banksel	uatbase
	bcf	RCSTA,SPEN	;decl: // all init that is BSR independent here
	bcf	RCSTA,CREN	; RCSTA &= ~((1<<SPEN)|(1<<CREN));
#endif
	banksel	uatbase
	bcf	TXSTA,TXEN	; TXSTA &= ~(1<<TXEN);
	local brgval,brgvalm,brgvalh,brgvall
#ifdef BRG16
brgval	set	rat>>2
brgvalm	set	brgval-1
brgvalh	set	high brgvalm
brgvall	set	low brgvalm
	banksel	uatbase
	bsf	BAUDCON,BRG16	; // section 26.1.2.8 of 16F1847 steps below:
	banksel	uatbase
	bcf	TXSTA,SYNC	; // (1) "Initialize..the desired baud rate"
	bsf	TXSTA,BRGH	; BAUDCON |= 1<<BRG16; // 16-bit generator
	movlw	brgvall		; TXSTA &= ~(1<<SYNC); // async mode
	movwf	SPBRGL		; TXSTA |= 1<<BRGH;    // high speed
	movlw	brgvalh		;
	movwf	SPBRGH		; SPBRG = (rat/4) - 1;
	bcf	BAUDCON,SCKP	; BAUDCON &= ~(1<<SCKP); // "SCKP..if inverted"
#else	
brgval	set	rat>>4
brgvalm	set	brgval-1
brgvalh	set	0
brgvall	set	low brgvalm
	bsf	TXSTA,BRGH	; TXSTA |= 1<<BRGH; // (1) the desired baud rate
	banksel	uatbase
	movlw	brgvall		;
	movwf	SPBRG		; SPBRG = (rat/16) - 1;
#endif
#if 0
	banksel	uatbase
	bsf	RCSTA,SPEN	; // (3) "Enable..by setting..SPEN"
	bcf	RCSTA,RX9	; RCSTA &= ~(1<<RX9);  // (5) "9-bit..set..RX9"
	bsf	RCSTA,CREN	; RCSTA |= (1<<SPEN) | (1<<CREN); // (6) "CREN" 
#endif
	banksel	uatbase
	bsf	TXSTA,TXEN	; TXSTA |= 1<<TXEN; // (5) "Enable..by..TXEN"
#if 0
	banksel	PIE1
	bsf	PIE1,RCIE	; PIE1 |= 1<<RCIE; //(4) "Set..RCIE..and..PEIE"
#endif
	zOS_ADR	task,zOS_UNP	; fsr0 = task & 0x7fff;// MSB 0 => unprivileged
	movlw	low isr		; w = zOS_ARG(0, isr & 0x00ff);
	zOS_ARG	0
	movlw	high isr	; w = zOS_ARG(1, isr>>8);
	zOS_ARG	1		; w = zOS_ARG(2, (0<<TXIF)|(1<<T0IF));
	movlw (0<<TXIF)|(1<<T0IF)
	zOS_ARG	2
	movlb	0		; // still in job "0": don't forget this!!!!
	endm			;} // zOS_CON()

	;; macro checks for safety (SFR, not global or another job's local RAM)
zOS_RW	macro	file
	if file & 0x60
	 error "tried to access disallowed RAM range (global or another job's)"
	else
	 movlb	file >> 7
	endif
	endm

zOS_R	macro	file,bankf,prsrv;inline int8_t zOS_R(const int8_t* file, int8_t
	if prsrv
	 movf	INTCON,w	;                    bank, int8_t prsrv) {
	endif
	bcf	INTCON,GIE	; if (prsrv)
	if prsrv
	 movwf	zOS_AR1		;  zOS_AR1 = INTCON;
	endif
	zOS_RW	file		; INTCON &= ~(1<<GIE); // access zOS_AR* globals
	movf	file,w		; bsr = file >> 7;
	movwf	zOS_AR0		; zOS_AR0 = *file; // any 0-0x1f SFR in any bank
	movf	bankf,w		; bsr = bankf;
	movwf	BSR		; w = zOS_AR0;
	movf	zOS_AR0,w	; if (prsrv && (zOS_AR1 & (1<<GIE)))
	if prsrv
	 btfss	zOS_AR1,GIE	;  INTCON |= 1<<GIE; // restore interrupt state
	endif
	bsf	INTCON,GIE	; return w;
	endm			;} // zOS_R()

zOS_W	macro	file,bankf	;inline int8_t zOS_W(const int8_t* file, int8_t
	zOS_RW	file		;                    bankf, uint8_t w) {
	movwf	file		; bsr = file >> 7;
	movf	bankf,w		; *file = w;
	movwf	BSR		; return bsr = bankf;
	endm			;} // zOS_W()

;;; like zOS_CON, but also accepts console input for command-line interaction
zOS_INP	macro	p,ra,rt,h,pi,isr;inline void zOS_INP(int8_t p, int8_t ra, int8_t
	local	rxtask,no_opt,rxisr,rxdecl
	bra	rxdecl		;      rt, int8_t* h, int8_t pi, void(*isr)()) {
	
	;; reserve constants and variables
	local	p0,p1,wrap,t0scale,isradrl,isradrh,tskadrl,tskadrh,accumul
	local	accumuh,numbase,destreg,destreh,char_io,buf,max
	;; 0x20~24 reserved for zOS_CON
p0	set	0x20
p1	set	0x21
wrap	set	0x22
t0scale	set	0x23
	
	;; 0x24~28 reserved for zOS_INP
isradrl	set	0x24
isradrh	set	0x25
tskadrl	set	0x26
tskadrh	set	0x27

	;; 0x28~2F reserved for zOS_MON and derivations e.g. zOS_MAN
optadrl	set	0x28
optadrh	set	0x29
accumul	set	0x2a
accumuh	set	0x2b
numbase	set	0x2c	
destreg	set	0x2d
destreh	set	0x2e
char_io	set	0x2f
buf	set	0x30
max	set	0x70		;FIXME: "max" has no purpose (just advisory that
				; local variable space is capped at the bottom
				; of the globals), so it can be nixed

;copy the preceding lines rather than including this file, as definitions for
;zOS_MON()-derived macros referring to these local variables wouldn't open it
;until expansion and would throw an undefined-var error during the processing

	local	uarbase,uarecv,rxflag
	if (p == 0)
uarbase	 set	RCREG & 0xff80
uarecv	 set	RCREG & 0x7f
rxflag	 set	RCIF
	else
uarbase	 set	RC#v(p)REG & 0xff80
uarecv	 set	RC#v(p)REG & 0x7f
rxflag	 set	RC#v(p)IF
	endif
	
;;; FIXME: haven't actually written the var init code for zOS_MON et al yet
rxtask
	movf	zOS_JOB,w	; goto rxdecl;
	movwf	BSR		;rxtask:
	movf	optadrh,w	;
	movwf	PCLATH		;
	iorwf	optadrl,w	;
	btfsc	STATUS,Z	;
	bra	no_opt		;
	movf	optadrl,w	;  if ((optadrh<<8) | optadrl)
	callw			;   (*(optadrh<<8) | optadrl)) (); //returns to:
;;; FIXME: do anything interesting with return value? 0 sent if nothing happened
no_opt
	movf	tskadrh,w	;
	movwf	PCLATH		;  goto (tskadrh<<8) | tskadrl;// zOS_CON() code
	movf	tskadrl,w	;
	movwf	PCL	;callw	;

rxisr	
	movf	zOS_JOB,w	;rxisr:
	movwf	BSR		; bsr = zOS_JOB; // isr starts with unknown bank
#if 0
	movlw	low uarbase	;
	movwf	FSR0L		;
	movlw	high uarbase	;
	movwf	FSR0H		; fsr0 = uarbase;
	zOS_LOC	FSR1,zOS_JOB,buf,p0	;
#endif
	
	movf	isradrh,w	;
	movwf	PCLATH		;
	movf	isradrl,w	; if (rt && (1<<RCIF) == 0) // SWI, not inp char
	banksel	uarbase
	btfss	rt,rxflag	;  goto (isradrh<<8)|isradrl;//zOS_CON takes SWI
	movwf	PCL		; else {
	bcf	rt,rxflag	;  rt &= ~(1<<RCIF);
#ifdef CAUTIOUS
	btfss	RCSTA,OERR	;
	bra	noovrrn		;  if ((uarbase | RCSTA) & (1<<OERR)) {
	movlw	'!'		;   zOS_AR0 = '!';
	movwf	zOS_AR0		;   zOS_BUF(zOS_JOB, p0);
	zOS_BUF	zOS_JOB,buf,p0	;  }
noovrrn
#endif
	movf	RCREG,w		;  // this read removes it from the FIFO
	movwf	zOS_AR0		;  zOS_AR0 = RCREG;
#ifdef CAUTIOUS
	btfss	RCSTA,OERR	;  if (RCSTA & (1<<OERR)) // rx overrun
	bcf	RCSTA,CREN	;   RCSTA &= ~(1<<CREN); // cleared by disable
	bsf	RCSTA,CREN	;  RCSTA |= 1<<CREN; // (re-)enable reception
#endif
	pagesel	isr		;  if (zOS_AR0)
	btfss	STATUS,Z	;   goto isr; // continue with parser
	goto	isr		;  zOS_RFI(); //return from interrupt
	zOS_RFI			; }
rxdecl
	zOS_CON	p,rat,rts,hb,pin
	movf	zOS_AR0,w	;rxdecl:
	movwf	isradrl		; zOS_CON(p,rat,rts,hb,pin);// extend zOS_CON()
	movf	zOS_AR1,w	; isradrl = zOS_AR0;
	movwf	isradrh		; isradrh = zOS_AR1; // will forward non-rx irq
	movf	FSR0L,w		;
	movwf	tskadrl		; tskadrl = fsr0 & 0x00ff;
	movf	FSR0H,w		;
	movwf	tskadrh		; tskadrh = fsr0 >> 8; // all non-rx tasks here
	clrf	optadrl		;
	clrf	optadrh		; optadrh = optadrl = ((*void)()) 0; // no func
	clrf	char_io		; char_io = 0; // nonzero means action to take
	zOS_ADR	rxtask,zOS_PRB
	movlw	low rxisr	; w = zOS_ARG(0, rxisr & 0x00ff)
	zOS_ARG	0
	movlw	high rxisr	; w = zOS_ARG(1, rxisr >> 8);
	zOS_ARG	1
	movf	zOS_AR2,w	; w = zOS_ARG(2, (1<<RCIF)|(0<<TXIF)|(1<<T0IF));
	iorlw	1<<rxflag	;} // zOS_INP()
	zOS_ARG	2
	movlb	0		; // still in job "0": don't forget this!!!!
	endm			;


zOS_ACC	macro	valregs,basereg
	clrf	valregs		;inline uint8_t zOS_ACC(uint8_t* valregs,uint8_t
	clrf	1+valregs	;                   *basereg) { // w unclobbered
	clrf	basereg		; *valregs = 0;
	bsf	basereg,4	; return *basereg = 10; // decimal by default
	bsf	basereg,2	;} // zOS_ACC()
	endm
	

zOS_PCT	macro	reg
	movlw	0x7e 		; // 0 <= reg <= 100
	andwf	reg,w		; w = reg & 0x7e; // 0 <= w <= reg (even, trunc)
	bcf	STATUS,C	;
	rlf	reg,f 		;
	rlf	reg,f		; uint16_t c = reg *= 4; // 0 <= reg <= 400
	btfsc	STATUS,C	; if (c > 0xff)
	iorlw	0x01		;  w |= 1;
	addwf	reg,f		; c = reg += w;
	btfsc	STATUS,C	; if (c > 0xff)
	iorlw	0x01		;  w |= 1;
	rrf	WREG		; // 0 <= (w&1)*256 + reg <= 500
	rrf	reg,f		; reg = ((w&1)*256 + reg)/2; // 0 <= reg <= 250
	endm
	
zOS_MON	macro	p,ra,rt,h,pi,isr;inline void zOS_MON(int8_t p, int8_t ra, int8_t
	local	monisr,monchr1,monchr2,monchr3,mondump,mondest,monram,monchr4
	local	monchr5,monchr6,monchr7,monchr8,monchr9,monprmp,monlast,endmon
	
	zOS_INP	p,ra,rt,h,pi,monisr 
	pagesel	endmon		;      rt, int8_t* h, int8_t pi, void(*isr)()) {
	goto	endmon		; zOS_INP(p,ra,rt,h,pi,monisr); }// isr may be 0

	local	p0,p1,wrap,t0scale,isradrl,isradrh,tskadrl,tskadrh,accumul
	local	accumuh,numbase,destreg,destreh,char_io,buf,max
	;; 0x20~24 reserved for zOS_CON
p0	set	0x20
p1	set	0x21
wrap	set	0x22
t0scale	set	0x23
	
	;; 0x24~28 reserved for zOS_INP
isradrl	set	0x24
isradrh	set	0x25
tskadrl	set	0x26
tskadrh	set	0x27

	;; 0x28~2F reserved for zOS_MON and derivations e.g. zOS_MAN
optadrl	set	0x28
optadrh	set	0x29
accumul	set	0x2a
accumuh	set	0x2b
numbase	set	0x2c	
destreg	set	0x2d
destreh	set	0x2e
char_io	set	0x2f
buf	set	0x30
max	set	0x70		;FIXME: "max" has no purpose (just advisory that
				; local variable space is capped at the bottom
				; of the globals), so it can be nixed

;copy the preceding lines rather than including this file, as definitions for
;zOS_MON()-derived macros referring to these local variables wouldn't open it
;until expansion and would throw an undefined-var error during the processing

monback
	andlw	0x3f		;void monback(uint3_t job, uint8_t ptr, char w){
	btfsc	STATUS,Z	; if (w &= 0x3f) {
	return			;  // 63 \b's should be enough in a buffer of 64
	movwf	zOS_AR1		;  
	movlw	0x08		;
	movwf	zOS_AR0		;  zOS_AR0 = '\b';

monloop
	zOS_BUF	zOS_JOB,buf,p0
	andlw	0x1		;  for (zOS_AR1 = w; zOS_AR1; zOS_AR1--) {
	btfss	STATUS,Z	;   if (zOS_BUF(job, ptr) == 0) // buff full
	return			;    return;
	decfsz	zOS_AR1,f	;  }
	bra	monloop		; }
	return			;} // monback()
	
monhex
	movlw	'0'		;void monhex(uint3_t job, uint8_t ptr) {
	movwf	zOS_AR0		; extern uint8_t accumuh;
	zOS_BUF	zOS_JOB,buf,p0
	andlw	0x1		; zOS_AR0 = '0';
	btfss	STATUS,Z	; if (zOS_BUF(job, ptr) == 0) // buf full
	return			;  return;
	movlw	'x'		; 
	movwf	zOS_AR0		; zOS_AR0 = 'x';
	zOS_BUF	zOS_JOB,buf,p0
	andlw	0x1		; if (zOS_BUF(job, ptr) == 0) // buf full
	btfss	STATUS,Z	;  return;
	return			; monlsb(job, ptr, w = accumuh); // not accumul
	movf	accumuh,w	;} // monhex()

monlsb
	clrf	zOS_AR0		;void monlsb(uint3_t job, uint8_t ptr, char w) {
	movwf	zOS_AR1		; zOS_AR0 = 0; zOS_AR1 = w; monbuf(job, ptr);
	bra	monbuf		;} // monlsb()
	
moncrlf
	movlw	'\r'		;void moncrlf(uint3_t job, uint8_t ptr, char w){
	movwf	zOS_AR0		; zOS_AR0 = '\r';
	zOS_BUF	zOS_JOB,buf,p0	; if (zOS_BUF(zos_job, ptr) < 1)
	andlw	0x1		;  return 0;
	btfss	STATUS,Z	;
	return			; zOS_AR0 = '\n';
	
monlf
	movlw	'\n'		; return zOS_BUF(zos_job, ptr, w);
	movwf	zOS_AR0		;} // moncrlf() monlf()
	
monbuf
	zOS_BUF	zOS_JOB,buf,p0	;void monbuf(uint3_t job, uint8_t ptr, char w) {
	return			; return zOS_BUF(job,ptr,w); } // 0/1/2 printed
	
monisr
	pagesel	monbuf		;void monisr(void) {
	movlw	0xe0		; // from zOS_INP isr with char zOS_AR0>0
	addwf	zOS_AR0,w	;
	btfss	WREG,7		; // refuse to echo unprintable characters
	call	monbuf		; if (zOS_AR0 > 31 && monbuf(zos_job,p0) > 0) {
	andlw	0x1		;  // successful echo into circular buffer
	btfsc	STATUS,Z	;
	bra	monlast		;
	movf	zOS_JOB,w	;
	movwf	BSR		;  bsr = zos_job;// to access char_io var et al
	
	movf	zOS_AR0,w	;  // handle '~' before the tolower() conversion
	xorlw	'~'		;
	btfss	STATUS,Z	;
	bra	monchr1		;  if (zOS_AR0 == '~') {
	comf	accumul,f	;   accumul = ~accumul;
	comf	accumuh,w	;
	movwf	accumuh		;
	movwf	char_io		;   char_io = accumuh = ~accumuh; // preserve
	pagesel	monhex
	call	monhex		;   monhex(zos_job, p0);
	movf	accumul,w	;   accumuh = accumul; // accumuh overwritten
	movwf	accumuh		;   monlsb(zos_job, p0);
	pagesel	monlsb
	call	monlsb		;   accumuh = char_io; // accumuh now restored
	movf	char_io,w	;   char_io = 0; // completely handled in ISR
	movwf	accumuh		;   zOS_RFI();
	clrf	char_io		;  }
	zOS_RFI

monchr1
	btfsc	zOS_AR0,6	;  if (zOS_AR0 & 0x40)
	bcf	zOS_AR0,5	;   zOS_AR0 &= 0xdf; // zOS_AR0=tolower(zOS_AR0)
	movf	zOS_AR0,w	;//FIXME: ` { | } ~ DEL mapped onto @ [ \ ] ^ _
	movwf	char_io		;
	xorlw	0x08		;  switch (char_io = zOS_AR0) {
	btfss	STATUS,Z	;  case '\b':
	bra	monchr2		;
	movlw	'\r'		;
	pagesel	monbuf
	call	monbuf		;   monbuf(zos_job, p0, '\r');
	bra	monprmp		;   goto monprmp;
	
monchr2
	movf	char_io,w	;
	xorlw	0x0a		;
	btfss	STATUS,Z	;  case '\r':
	bra	monchr3		;   monbuf(zos_job, p0, '\n');// follows the \r
	pagesel	monlf		
	call	monlf		;
	
	movf	destreg,w	;   // repeat \r's can set a whole range of
	movwf	FSR0L		;   // addresses to zero
	movf	1+destreg,w	;
	movwf	FSR0H		;   fsr0 = destreg;
	iorwf	FSR0L,w		;
	btfsc	STATUS,Z	;
	bra	monprmp		;   if (fsr0) { // destreg was set by ' ' or =
	movf	accumul,w	;    if (fsr0 & 0x8000 == 0)
	btfss	FSR0H,7		;
	movwi	FSR0++		;     *fsr0 = accumul & 0x00ff; // not in flash
	movf	FSR0L,w		;
	movwf	destreg		;
	movf	FSR0H,w		;    destreg++; // advances for next access
	movwf	1+destreg	;   }
	bra	monprmp		;   goto monprmp;

monchr3
	movf	char_io,w	;
	xorlw	0x20		;
	btfsc	STATUS,Z	;  case ' ':
	bra	mondump		;
	movf	char_io,w	;
	xorlw	'.'		;
	btfsc	STATUS,Z	;  case '.':
	bra	mondump		;
	movf	char_io,w	;
	xorlw	'='		;
	btfss	STATUS,Z	;  case '=':
	bra	monchr4		;

mondump
	movf	accumul,w	;   // pressing ' ' or '.' or '=' should apply
	iorwf	accumuh,w	;   // to the recently incremented address from
	btfsc	STATUS,Z	;   // a previous operation (if any) or to an
	bra	mondest		;   // an address typed immediately before it
	movf	accumul,w	;
	movwf	destreg		;
	movf	accumuh,w	;   if (accumul) // typed a value before ' '/=
	movwf	1+destreg	;    destreg = accumul; // otherwise no clobber
	
mondest
	movf	destreg,w	;
	movwf	FSR0L		;
	movf	1+destreg,w	;
	movwf	FSR0H		;   fsr0 = destreg;
	
	btfsc	1+destreg,7	;   if (destreg & 0x8000) { // flash, not RAM
	bra	monram		;
;;; FIXME: access upper byte in Flash instead of printing it as zero
	clrf	accumuh		;
	pagesel	monhex
	call	monhex		;    monhex(zos_job, p0, accumuh=0);// put 0x00
	movf	destreg,w	;
	movwf	FSR0L		;
	movf	1+destreg,w	;
	movwf	FSR0H		;    fsr0 = destreg; // monhex() clobbered fsr0
	moviw	FSR0++		;
	movwf	accumuh		;
	movf	FSR0L,w		;
	movwf	destreg		;    accumuh = *fsr0++;
	movf	FSR0H,w		;    destreg = fsr0;
	movwf	1+destreg	;    monlsb(zos_job, p0, accumuh);  //      LSB
	pagesel	monlsb	
	call	monlsb		;    moncrlf(zos_job, p0);          //     \r\n
;;; FIXME: disassemble the instruction here once the upper 6 bits are available
	pagesel	moncrlf
	call	moncrlf		;    goto monprmp;
	bra	monprmp		;   }
	
monram
	moviw	FSR0++		;
	movf	FSR0L,w		;
	movwf	destreg		;
	movf	FSR0H,w		;
	movwf	1+destreg	;
	movwf	accumuh		;   accumuh = *(destreg = fsr0++);
	pagesel	monhex
	call	monhex		;   monhex(

	movf	char_io,w	;
	xorlw	'.'		;   // then exits in the '.' case to just print
	pagesel	moncrlf
	btfss	STATUS,Z	;   if (char_io == '.')
	goto	moncrlf		;    goto moncrlf;

	movf	char_io,w	;   // or follow by 3 backspaces in the ' ' case
	xorlw	'='		;   // to show that \r will result in a 0 write
	btfss	STATUS,Z	;
	movlw	3		;
	pagesel	monback
	call	monback		;   monback(zos_job, p0, (char_io == '=')?0:3);
	clrf	char_io		;   char_io = 0;
	zOS_RFI			;  break;

monchr4
	movf	char_io,w	;
	xorlw	'X'		;
	btfss	STATUS,Z	;  case 'X':
	bra	monchr5		;
	movlw	0x10		;   numbase = 16;
	movwf	numbase		;   char_io = 0;
	clrf	char_io		;  break;
	zOS_RFI

monchr5
	movf	char_io,w	;
	xorlw	'%'		;
	btfss	STATUS,Z	;  case '%':
	bra	monchr6		;
	movlw	0x9b		;
	addwf	accumul,w	;
	movlw	0x66		;
	btfss	WREG,7		;   if (accumul > 102)
	movwf	accumul		;    accumul = 102;
	zOS_PCT	accumul
	movwf	accumul		;   accumul = zOS_PCT(accumul);
	movwf	accumuh		;   accumuh = accumul;
	pagesel	monhex		;   monhex(zos_job, p0); print as e.g. 50%0x7d
	call	monhex		;   accumuh = 0;
	clrf	accumuh		;   char_io = 0;
	clrf	char_io		;  break;
	zOS_RFI
	
monchr6
	movlw	0-0x10		;  default:
	addwf	char_io,f	;
	btfsc	char_io,7	;
	bra	monchr9 	;   if ((char_io -= ('0'&0xdf /*0x10*/)) >= 0) {
	addwf	char_io,w	;
	btfsc	WREG,7		;    if (char_io > 0x10)
	bra	$+3		;
	movlw	0xf9		;
	addwf	char_io,f	;     char_io -= 0x07;// 0x41->0x31->0x2a... so
	movf	char_io,f	;                     // now in range 0x00-0x09,
	btfss	STATUS,Z	;                     // or :=0x0a,...,?=0x0f,
	bra	monchr7		;                     // or A=0x2a,B=0x2b,...
	movf	accumul,w	;                     // G=0x30,...,Z=0x43
	iorwf	accumuh,w	;    if ((char_io == 0) &&
	btfss	STATUS,Z	;        (accumul == 0) && (accumuh == 0)) {
	bra	monchr7		;     numbase &= ~2; // digit(s) leading 0(s),
	bcf	numbase,1	;     char_io = 0;
	clrf	char_io		;     break;         // just go into octal mode
	zOS_RFI
	
monchr7
	movlw	0x50		;
	andwf	char_io,w	;
	btfss	STATUS,Z	;    } else if ((char_io & 0x50 == 0) // 0-9,a-f
	bra	monchr8		;              && (numbase & 0x10)) { // base 16
	btfss	numbase,4	;
	bra	monchr8		;
	swapf	accumuh,f	;
	movlw	0xf0		;
	andwf	accumuh,f	;     accumuh <<= 4;
	swapf	accumul,w	;
	andlw	0x0f		;
	iorwf	accumuh,f	;     accumuh |= accumul >> 4;
	movlw	0x0f		;
	andwf	char_io,f	;     char_io &= 0x0f;
	andwf	accumul,f	;     accumul &= 0x0f;
	swapf	accumul,w	;
	iorwf	char_io,w	;
	movwf	accumul		;     accumul = (accumul << 4) | char_io;
	clrf	char_io		;     char_io = 0;
	zOS_RFI			;     break;
	
monchr8
	movf	char_io,w	;    } else if (char_io <= 9) { //dec only<=99?
	andlw	0xf0		;     uint16_t sum;
	btfss	STATUS,Z	;     accumuh <<= 1;
	bra	monchr9		;     accumuh |= (accumul & 0x80) ? 1 : 0;
	bcf	STATUS,C	;     accumul <<= 1;
	rlf	accumul,f	;     w = accumul;//w keeps original accumul<<1
	rlf	accumuh,f	;     accumuh <<= 1;
	movf	accumul,w	;     accumuh |= (accumul & 0x80) ? 1 : 0;
	bcf	STATUS,C	;     accumul <<= 1;
	rlf	accumul,f	;     accumuh |= (accumul & 0x80) ? 1 : 0;
	rlf	accumuh,f	;     accumul <<= 1; // accumuh:accumul <<= 3;
	bcf	STATUS,C	;     if (numbase & 2) { // base 10 presumed
	rlf	accumul,f	;      sum = (accumuh<<8)+accumul + w;
	rlf	accumuh,f	;      accumul = sum & 0x00ff;
	btfss	numbase,1	;      accumuh = sum >> 8;
	bra	$+4		;     }
	addwf	accumul,f	;     sum = (accumuh<<8)+accumul + char_io&0x0f;
	movlw	0		;     accumul = sum & 0x00ff;
	addwfc	accumuh,f	;     accumuh = sum >> 8;
	movf	char_io,w	;     break;
	andlw	0x0f		;    }
	addwf	accumul,f	;   } // if ()
	movlw	0		;   char_io = 0;
	addwfc	accumuh,f	;   zOS_AR1 = accumul;
	clrf	char_io		;   if (isr) goto isr; // with zOS_AR1=accumul
	zOS_RFI
	
monchr9
	movf	accumul,w	;  } // switch ()
	movwf	zOS_AR1		; } // if ()
	pagesel	isr
	if (isr)
	 goto	isr		; char_io = 0; // unhandled
	else
	 clrf	char_io		; zOS_RFI(); // reached only if isr == 0
	 zOS_RFI
	endif

;;; 
monprmp
	movf	1+destreg,w	;monprmp:
	movwf	accumuh		; accumuh = destreg>>8;
	iorwf	destreg,w	; if (destreg) { // prompt with destreg if nonzero
	pagesel	monhex
	btfsc	STATUS,Z	;  monhex(zos_job, p0);
	bra	$+6		;  accumuh = destreg & 0xff;
	call	monhex		;  monlsb(zos_job, p0);
	movf	destreg,w	; }
	movwf	accumuh		;monlast: zOS_ACC(&accumul,&numbase); zOS_RFI();
	pagesel	monlsb
	call	monlsb		;         char_io = 0;
	zOS_ACC	accumul,numbase
monlast
	clrf	char_io		;} // zOS_MON()
	zOS_RFI
endmon
	endm	

zOS_CLC	macro	p,ra,rt,h,pi,isr;inline void zOS_CLC(int8_t p, int8_t ra, int8_t
	local	endclc,clcisr,clcprmp,endclc

	zOS_MON	p,ra,rt,h,pi,clcisr
	pagesel	endclc
	goto	endclc		;      rt, int8_t* h, int8_t pi, void(*isr)()) {
	
	local	p0,p1,wrap,t0scale,isradrl,isradrh,tskadrl,tskadrh,accumul
	local	accumuh,numbase,destreg,destreh,char_io,buf,max
	;; 0x20~24 reserved for zOS_CON
p0	set	0x20
p1	set	0x21
wrap	set	0x22
t0scale	set	0x23
	
	;; 0x24~28 reserved for zOS_INP
isradrl	set	0x24
isradrh	set	0x25
tskadrl	set	0x26
tskadrh	set	0x27

	;; 0x28~2F reserved for zOS_MON and derivations e.g. zOS_MAN
optadrl	set	0x28
optadrh	set	0x29
accumul	set	0x2a
accumuh	set	0x2b
numbase	set	0x2c	
destreg	set	0x2d
destreh	set	0x2e
char_io	set	0x2f
buf	set	0x30
max	set	0x70		;FIXME: "max" has no purpose (just advisory that
				; local variable space is capped at the bottom
				; of the globals), so it can be nixed

;copy the preceding lines rather than including this file, as definitions for
;zOS_MON()-derived macros referring to these local variables wouldn't open it
;until expansion and would throw an undefined-var error during the processing

clcisr
	movf	zOS_AR0,w	; switch (char_io = zOS_AR0) {
	movwf	char_io		;
	xorlw	'+'		;
	btfss	STATUS,Z	;
	bra	clcchr2		; case '+':

	movf	accumul,w	;
	addwf	destreg,f	;
	movf	accumuh,w	;
	addwfc	1+destreg,f	;  destreg += (accumuh << 8) | accumul;
	bra	clcprmp		; break;

clcchr2
	movf	char_io,w	;
	xorlw	'-'		;
	btfss	STATUS,Z	;
	bra	clcchr3		; case '-':

	movf	accumul,w	;
	subwf	destreg,f	;
	movf	accumuh,w	;
	subwfc	1+destreg,f	;  destreg -= (accumuh << 8) | accumul;
	bra	clcprmp		; break;
	
clcchr3	;FIXME: multiplication not yet implemented
	movf	char_io,w	;
	xorlw	'*'		;
	btfss	STATUS,Z	;
	bra	clcchr4		; case '*':

clcchr4	;FIXME: division not yet implemented
	movf	char_io,w	;
	xorlw	'/'		;
	btfss	STATUS,Z	;
	bra	clcchr5		; case '/':

clcchr5	;FIXME: exponentiation not implemented
	movf	char_io,w	;
	xorlw	'^'		;
	btfss	STATUS,Z	;
	bra	clcchr6		; case '^':

clcchr6	;FIXME: factorial not yet implemented
	movf	char_io,w	;
	xorlw	'!'		;
	btfss	STATUS,Z	;
	bra	clcchr7		; case '!':

clcchr7
	movf	accumul,w	; default: zOS_AR1 = accumul; if (isr) goto isr;
	movwf	zOS_AR1		; }// caller may use zOS_AR1 or accumuh:accumul
	pagesel	isr		;
	if(isr)
	 goto	isr		; zOS_RFI();
	else
	 zOS_RFI
	endif
	
clcprmp
	pagesel	moncrlf
	call	moncrlf		;clcprmp:
	movf	1+destreg,w	; moncrlf(zos_job, p0);
	movwf	accumuh		; accumuh = destreg>>8; monhex(zos_job, p0);
	pagesel	monhex
	call	monhex		; accumuh = destreg & 0xff; monlsb(zos_job, p0);
	movf	destreg,w	; moncrlf(zos_job, p0);
	movwf	accumuh		;clclast: 
	pagesel	monlsb
	call	monlsb		; zOS_ACC(&accumul,&numbase); zOS_RFI();
	pagesel	moncrlf
	call	moncrlf		; char_io = 0;
	zOS_ACC	accumul,numbase
clclast
	clrf	char_io		;}
	zOS_RFI
endclc
	endm


zOS_MAN	macro	p,rat,rts,hb,pin;inline void zOS_MAN(int8_t p, int8_t rat,
	local	mantask,manisr,manchr,manchr0,reenable,manchr1,manchr2,manchr3
	local	manchr4,manchr5,manchr6,manchr7,manchr8,manchr9,mannone,jobinfo
	local	crlf,stkinfo,stkloop,endman

	zOS_MON	p,rat,rts,hb,pin,0
	movlw	low mantask	;                      int8_t* hb, int8_t pin) {
	movwf	optadrl		; zOS_MON(p,ra,rt,h,pi,manisr);
	movlw	high mantask	; optadrl = mantask & 0x00ff;
	movwf	optadrh		; optadrh = mantask >> 8;
	pagesel	endman
	goto	endman		;}
	
	local	p0,p1,wrap,t0scale,isradrl,isradrh,tskadrl,tskadrh,accumul
	local	accumuh,numbase,destreg,destreh,char_io,buf,max
	;; 0x20~24 reserved for zOS_CON
p0	set	0x20
p1	set	0x21
wrap	set	0x22
t0scale	set	0x23
	
	;; 0x24~28 reserved for zOS_INP
isradrl	set	0x24
isradrh	set	0x25
tskadrl	set	0x26
tskadrh	set	0x27

	;; 0x28~2F reserved for zOS_MON and derivations e.g. zOS_MAN
optadrl	set	0x28
optadrh	set	0x29
accumul	set	0x2a
accumuh	set	0x2b
numbase	set	0x2c	
destreg	set	0x2d
destreh	set	0x2e
char_io	set	0x2f
buf	set	0x30
max	set	0x70		;FIXME: "max" has no purpose (just advisory that
				; local variable space is capped at the bottom
				; of the globals), so it can be nixed

;copy the preceding lines rather than including this file, as definitions for
;zOS_MON()-derived macros referring to these local variables wouldn't open it
;until expansion and would throw an undefined-var error during the processing

	
mantask
	movf	zOS_JOB,w	;int8_t mantask(void) {//destreg,accumul,char_io
	movwf	BSR		; bsr = zos_job; // to access char_io
	movf	char_io,w	; if (char_io == 0)
	btfsc	STATUS,Z	;  return 0; // back to zOS_CON task
	return			; switch (char_io) {

	xorlw	'G'		;
	btfss	STATUS,Z	; caseG:
	bra	manchr		; case 'G': // Generate a fork/duplicate of job
	clrf	char_io		;  char_io = 0; // presume failure, so no retry
	
	movf	accumul,w	;  if (accumul == 0)
	btfsc	STATUS,Z	;   return 0;
	return			;  zOS_ARG(0, accumul);
	zOS_ARG	0
	zOS_ACC	accumul,numbase
	movlw	'J'		;  zOS_ACC(&accumul, &numbase); // reset
	movwf	char_io		;  if (zOS_SWI(zOS_FRK))
	zOS_SWI	zOS_FRK
	andlw	0x00		;   goto caseJ; // success, prints in job list
	btfsc	STATUS,Z	;  else
	clrf	char_io		;   break; // failure, drop to end of switch()

manchr
	movf	char_io,w	;
	xorlw	'H'		;
	btfss	STATUS,Z	; caseH:
	bra	manchr0		; case 'H': // find jobs by Handle (start addr)
	clrf	char_io		;  char_io = 0;

	movf	accumul,w	;  if (accumul == 0)
	iorwf	accumuh,w	;
	btfsc	STATUS,Z	;   return 0;
	return			;  zOS_ARG(0, accumul);
	movf	accumul,w	;
	zOS_ARG	0
	movf	accumuh,w	;
	zOS_ARG	1
	zOS_ACC	accumul,numbase
	movlw	'J'		;  zOS_ACC(&accumul, &numbase);
	movwf	char_io		;  if (zOS_SWI(zOS_FND))
	zOS_SWI	zOS_FND
	andlw	0x00		;   goto caseJ; // FIXME: table, from match down
	btfsc	STATUS,Z	;  else
	clrf	char_io		;   break;
	
manchr0
	movf	char_io,w	;
	xorlw	'I'		;
	btfss	STATUS,Z	; caseI:
	bra	manchr1		; case 'I': // send a software Interrupt > 7
	clrf	char_io		;  char_io = 0; // with destreg zOS_AR1:zOS_AR0
	
	movf	destreg,w	;  zOS_ARG(0, destreg);
	zOS_ARG	0
	movf	1+destreg,w	;  zOS_ARG(1, destreh);
	zOS_ARG	1
	movlw	0xf8		;  zOS_ACC(&accumul, &numbase); // reset
	andwf	accumul,w	;
	zOS_ACC	accumul,numbase
	btfsc	STATUS,Z	;  if (accumul) {
	bra	reenabl		;   int w = zOS_SWI(accumul); // disable again
	movlp	0		;   INTCON &= ~(1<<GIE);// for zOS_AR and _BUF()
	call	0x02		;   zOS_ARG(1, w);
	bcf	INTCON,GIE	;   zOS_ARG(0, 0);
	clrf	zOS_AR1		;   zOS_BUF(zos_job, p0); // print hex SWI result
	xorwf	zOS_AR1,f	;   zOS_ENA();
	xorwf	zOS_AR0,f	;   goto caseJ;
	zOS_BUF	zOS_JOB,buf,p0
	movlw	'J'		;  } else
	movwf	char_io		;   zOS_ENA(); break;
reenabl
	zOS_ENA

manchr1
	movf	char_io,w	;
	xorlw	'J'		;
	btfss	STATUS,Z	; caseJ:
	bra	manchr2		; case 'J': // List struct for all running jobs

	decf	accumul,w	; // keep char_io='S' until last job line prints
	andlw	0x07		;
	btfsc	WREG,2		;  if ((accumul < 1) || (accumul > 5))
	movlw	zOS_NUM-1	;
	addlw	0x01		;
	movwf	accumul		;   accumul = zOS_NUM;
	bcf	INTCON,GIE	;  INTCON &= ~(1<<GIE); // to keep p0==p1 atomic
	pagesel	jobinfo
	movf	p0,w		;
	xorwf	p1,w		;  if (p0 == p1)
	btfsc	STATUS,Z	;   return jobinfo(); // will decrement accumul
	goto	jobinfo		;  zOS_ENA(); // re-enable interrupts if p0!=p1
	zOS_ENA
	retlw	0		;  return 0;//try again after caller advances p0

manchr2
	movf	char_io,w	;
	xorlw	'K'		;
	btfss	STATUS,Z	; caseK:
	bra	manchr3		; case 'K': // Kill a single job (# mandatory)
	clrf	char_io		;  char_io = 0;

	movf	accumul,w	;  if (accumul == 0)
	btfsc	STATUS,Z	;   return 0;
	return			;  zOS_ARG(0, accumul);
	zOS_ARG	0
	zOS_ACC	accumul,numbase
	movlw	'J'		;  zOS_ACC(&accumul, &numbase);
	movwf	char_io		;  zOS_SWI(zOS_END); // listed indicates failure
	zOS_SWI	zOS_END
;;; FIXME: put J at bottom so K onward don't pay a performance penalty awaiting
	
manchr3
	movf	char_io,w	;
	xorlw	'L'		;
	btfss	STATUS,Z	; caseL:
	bra	manchr4		; case 'L': // Launch a fresh instance of a job
	clrf	char_io		;  char_io = 0;

	movf	accumul,w	;  if (accumul == 0)
	btfsc	STATUS,Z	;   return 0;
	return			;  zOS_ARG(0, accumul);
	zOS_ARG	0
	zOS_ACC	accumul,numbase
	movlw	'J'		;  zOS_ACC(&accumul, &numbase); // reset
	movwf	char_io		;  if ((w = zOS_SWI(zOS_FRK)) != 0) {
	zOS_SWI	zOS_FRK
	andlw	0x00		;   zOS_ARG(0,w); zOS_SWI(zOS_RST);
	btfsc	STATUS,Z	;   goto caseJ; // success, prints in job list
	clrf	char_io		;  } else
	zOS_ARG	0
	zOS_SWI	zOS_RST		;   break; // failure, drop to end of switch()

manchr4
	movf	char_io,w	;
	xorlw	'N'		;
	btfss	STATUS,Z	; caseN:
	bra	manchr5		; case 'N': // New (parameterless) job at addr

	movf	accumul,w	;
	movwf	FSR0L		;
	movf	accumuh,w	;
	movwf	FSR0L		;
	clrw			;
	zOS_ARG	0
	zOS_ARG	1
	zOS_ARG	2
	zOS_ARG	3
	zOS_SWI	zOS_NEW
	zOS_ARG	0
	zOS_BUF	zOS_JOB,buf,p0
	movlw	'J'		;
	movwf	char_io		;

	movf	accumul,w	;  if (accumul == 0)
	btfsc	STATUS,Z	;   return 0;
	return			;  zOS_ARG(0, accumul);
	zOS_ARG	0
	zOS_ACC	accumul,numbase
	movlw	'J'		;  zOS_ACC(&accumul, &numbase);
	movwf	char_io		;  if ((w = zOS_SWI(zOS_SLP)) != 0) {
	zOS_SWI	zOS_SLP
	andlw	0xff		;   accumul = w;
	movwf	accumul		;   goto caseJ;
	btfsc	STATUS,Z	;  } else
	clrf	char_io		;   break;

manchr5
	movf	char_io,w	;
	xorlw	'P'		;
	btfss	STATUS,Z	; caseP:
	bra	manchr6		; case 'P': // Pause job by putting it to Sleep
	clrf	char_io		;  char_io = 0;

	movf	accumul,w	;  if (accumul == 0)
	btfsc	STATUS,Z	;   return 0;
	return			;  fsr1 = 0x10 * (1 + accumul) + zOS_PCH;
	movlw	'J'		;
	movwf	char_io		;
	zOS_MEM	FSR1,accumul,zOS_PCH
	movf	INDF1,w		;  if (*fsr1) { // is a valid (PCH not 0x00) job
	btfsc	STATUS,Z	;   *fsr |= 0x80;
	clrf	char_io		;   goto caseJ;
	iorlw	0x80		;  } else {
	movf	INDF1,f		;
	btfss	STATUS,Z	;
	movwf	INDF1		;   zOS_ACC(&accumul, &numbase);
	btfsc	STATUS,Z	;   break; // only clear accumul if not caseJ
	bra	manchr6		;  }
	zOS_ACC	accumul,numbase
	
manchr6
	movf	char_io,w	;
	xorlw	'Q'		;
	btfss	STATUS,Z	; caseQ:
	bra	manchr7		; case 'Q': // Quit without wake (off)
	clrf	char_io		;  char_io = 0;
	
	bcf	WDTCON,SWDTEN	;  WDTCON &= ~(1<<SWDTEN);
	movf	accumul,f	;
	btfss	STATUS,Z	;  if (accumul)
	sleep			;   sleep(); // never wakes up

manchr7
	movf	char_io,w	;
	xorlw	'R'		;
	btfss	STATUS,Z	; caseR:
	bra	manchr8		; case 'R': // Resume a pause/asleep job 
	clrf	char_io		;  char_io = 0;

	movf	accumul,w	;  if (accumul == 0)
	btfsc	STATUS,Z	;   return 0;
	return			;  fsr1 = 0x10 * (1 + accumul) + zOS_PCH;
	movlw	'J'		;
	movwf	char_io		;  if (*fsr1 &= ~(1<<zOS_WAI)) {
	zOS_MEM	FSR1,accumul,zOS_PCH
	movlw	0x7f		;   goto caseJ; // valid job won't be 0 or 0x80
	andwf	INDF1,f		;  } else {
	btfss	STATUS,Z	;   zOS_ACC(&accumul, &numbase);
	bra	manchr8		;
	zOS_ACC	accumul,numbase
	clrf	char_io		;   break; // only clear accumul if not caseJ
	
manchr8
	movf	char_io,w	;  }
	xorlw	'S'		;
	btfss	STATUS,Z	;
	bra	manchr9		; case 'S': // Stack dump is actually scratch
	clrf	char_io		;  char_io = 0; // always succeeds, no arg

	decf	accumul,w	; // keep char_io='J' until last job line prints
	andlw	0x07		;
	btfsc	WREG,2		;  if ((accumul < 1) || (accumul > 5))
	movlw	zOS_NUM-1	;
	addlw	0x01		;
	movwf	accumul		;   accumul = zOS_NUM;
	bcf	INTCON,GIE	;  INTCON &= ~(1<<GIE); // to keep p0==p1 atomic
	pagesel	stkinfo
	movf	p0,w		;
	xorwf	p1,w		;  if (p0 == p1)
	btfsc	STATUS,Z	;   return jobinfo(); // will decrement accumul
	goto	stkinfo		;  zOS_ENA(); // re-enable interrupts if p0!=p1
	zOS_ENA
	retlw	0		;  return 0;//try again after caller advances p0

manchr9
	movf	char_io,w	;
	xorlw	'Z'		;
	btfss	STATUS,Z	;
	bra	mannone		; case 'Z': // go to low-power Zz mode for time
	clrf	char_io		;  char_io = 0;
	
	bsf	WDTCON,SWDTEN	;
	bcf	STATUS,C	;  if (w = accumul<<1) { // WDT prescaler
	rlf	accumul,w	;   w |= 1<<SWDTEN; // enable the wakeup
	btfsc	STATUS,Z	;   
	bra	mannone		; 
	iorlw	1<<SWDTEN	;
	movwf	WDTCON		;
	sleep			;  break;  // wakes up according to prescaler

mannone
	retlw	0		; } return 0; //naught to do }

	;guaranteed to arrive with p0=p1, interrupts off and in the correct bank
stkinfo
	movf	wrap,f		;int8_t stkinfo(void) {
	movwf	p0		; p0 = p1 = wrap;
	movwf	p1		;
	movlw	low zOS_STK	;
	movwf	FSR0L		;
	movlw	high zOS_STK	;
	movwf	FSR0H		;
	decf	accumul,w	;
	brw			;
	addfsr	FSR0,6		;
	addfsr	FSR0,6		;
	addfsr	FSR0,6		;
	addfsr	FSR0,6		; fsr0 = zOS_STK + 6 * (5 - accumul);
	zOS_LOC	FSR1,zOS_JOB,buf
	movlw	'\r'		; fsr1 = (zOS_JOB << 7) + buf;
	movwi	FSR1++		;
	movlw	'\n'		;
	movwi	FSR1++		;
	movlw	'-'		;
	movwi	FSR1++		;
	movf	accumul,w	;
	addlw	-12		; // print this stack offset as -0/-1/-2/-3/-4
	zOS_HEX
	movwi	FSR1++		; p1 += sprintf(p1, "\r\n-%1X", accumul & 7);
	movlw	3		;
	movwf	accumuh		; for (accumuh = 3; accumuh; accumuh--) {
stkloop
	movlw	' '		;
	movwi	FSR1++		;  p1 += sprintf(p1, " %04X", *((int*) fsr0));
	moviw	--FSR0		;
	movwi	FSR1++		;
	moviw	--FSR0		;
	movwi	FSR1++		;
	decfsz	accumuh,f	;
	bra	stkloop		; }
	
	movf	FSR1L,w		;
	movwf	p1		; w = accumul--; // return with w as nonzero job
	movf	accumul,w	; if (accumul == 0)
	decf	accumul,f	;  char_io = 0;// final row in table was printed
	btfsc	STATUS,Z	; zOS_ENA(); // interrupts back ON!
	clrf	char_io		; return w;
	zOS_ENA
	return			;}
	
	;guaranteed to arrive with p0=p1, interrupts off and in the correct bank
jobinfo
	movf	wrap,f		;int8_t jobinfo(void) {
	movwf	p0		; p0 = p1 = wrap;
	movwf	p1		; fsr0 = 0x10 * (1 + accumul); //FIXME: 2+
	zOS_MEM	FSR0,accumul,0
	zOS_LOC	FSR1,zOS_JOB,buf
	movlw	'\r'		; fsr1 = (zOS_JOB << 7) + buf;
	movwi	FSR1++		;
	movlw	'\n'		;
	movwi	FSR1++		;
	movf	accumul,w	; // print this job number 5/4/3/2/1
	zOS_HEX
	movwi	FSR1++		; p1 += sprintf(p1, "\r\n%1X", accumul);
	
	moviw	zOS_HDH[FSR0]	;
	andlw	1<<zOS_PRB	;
	movlw	':'		; // print '*' if the job is privileged else ':'
	btfsc	STATUS,Z	;
	movlw	'*'		; p1 += sprintf(p1, "%c", (zOS_HDH[fsr0] &
	moviw	FSR1++		;                    (1<<zOS_PRB)) ? '*' : ':');
	
	zOS_IHF	zOS_HDH,FSR0,FSR1
	zOS_IHF	zOS_HDL,FSR0,FSR1
	movlw	' '		; 
	movwi	FSR1++		;
	movlw	'P'		; // print the 4-hex-digit header then PC
	movwi	FSR1++		;
	movlw	'C'		; p1 += sprintf(p1, "%04X PC",
	movwi	FSR1++		;         (zOS_HDH[fsr0] << 8) + zOS_HDL[fsr0]);
	
	moviw	zOS_PCH[FSR0]	;
	andlw	1<<zOS_WAI	;
	movlw	'='		; // print '=' if the job is sleeping else 'z'
	btfsc	STATUS,Z	;
	movlw	'z'		; p1 += sprintf(p1, "%c", (zOS_PCH[fsr0] &
	movwi	FSR1++		;                    (1<<zOS_WAI)) ? 'z' : ':');
	
	zOS_IHF	zOS_PCH,FSR0,FSR1
	moviw	zOS_PCH[FSR0]	; // drop out after PCH if 0 (job is deleted)
	btfsc	STATUS,Z	; p1 += sprintf(p1, "%02X", zOS_PCH[fsr0]);
	bra	crlf		; if (zOS_PCH[fsr0] & 0xff00) {
	zOS_IHF	zOS_PCL,FSR0,FSR1
	movlw	' '		;  // print the low byte of program counter
	movwi	FSR1++		;  p1 += sprintf(p1, "%02X", zOS_PCL[fsr0]); 
	moviw	zOS_ISH[FSR0]	;
	btfss	STATUS,Z	;  // drop out after PCL if no interrupt routine
	bra	crlf		;  if (zOS_ISH[fsr0] & 0xff00) {
	movlw	'I'		;
	movwi	FSR1++		;
	movlw	'S'		;
	movwi	FSR1++		;
	movlw	'R'		;
	movwi	FSR1++		;
	movlw	'@'		;
	movwi	FSR1++		;   // print ISR@ then 4-hex-digit routine addr
	zOS_IHF	zOS_ISH,FSR0,FSR1
	zOS_IHF	zOS_ISR,FSR0,FSR1
	movlw	'('		;   p1 += sprintf(p1, " ISR@%04X",
	movwi	FSR1++		;         (zOS_ISH[fsr0] << 8) + zOS_ISR[fsr0]);
	movlw	'h'		;
	movwi	FSR1++		;
	movlw	'w'		;
	movwi	FSR1++		;
	zOS_IHF	zOS_HIM,FSR0,FSR1
	movlw	's'		;
	movwi	FSR1++		;
	movlw	'w'		;
	movwi	FSR1++		;  // print (hw HwIMask sw SwIMask) scrunched up
	zOS_IHF	zOS_SIM,FSR0,FSR1
	movlw	')'		;   p1 += sprintf(p1, "(hw%02Xsw%02X)",
	movwi	FSR1++		;                 zOS_HIM[fsr0], zOS_SIM[fsr0]);
crlf
	movlw	'\r'		;  }
	movwi	FSR1++		; }
	movlw	'\n'		; // print a second \r\n, double-spacing table  
	movwi	FSR1++		; p1 += sprintf(p1, "\r\n");
	
	movf	FSR1L,w		;
	movwf	p1		; w = accumul--; // return with w as nonzero job
	movf	accumul,w	; if (accumul == 0)
	decf	accumul,f	;  char_io = 0;// final row in table was printed
	btfsc	STATUS,Z	; zOS_ENA(); // interrupts back ON!
	clrf	char_io		; return w;
	zOS_ENA
	return			;}
endman
	endm














