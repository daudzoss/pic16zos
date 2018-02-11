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
	if (job)
	 lslf	job,w		;inline void zOS_GLO(int8_t**fsrnum,int8_t*job){
	else
	 lslf	zOS_ME		;                      
	endif
	andlw	0x0e		; int8_t w = 0x70 | ((job ? *job : bsr) << 1);
	iorlw	0x70		;
	movwf	FSR#v(fsrn)L	;// documentation suggests 5 but BSR now 6 bits!
	movlw	0x1f		; *fsrnum = (*fsrnum & 0x1f00) | w;
	andwf	FSR#v(fsrn)H,f	;} // zOS_GLO()
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
	if (job - FSR#v(fsrn)H)
	 lsrf	job,w		;
	 movwf	FSR#v(fsrn)H	; return (*fsrnum = (job<<7) | offset) >> 8;
	else
	 lsrf	job,f
	endif
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
	bcf	INTCON,GIE	; do {  w = zOS_SWI(zOS_NEW);
#ifdef CAUTIOUS
	movf	BSR,f		;  INTCON &= ~(1<<GIE); // prevent deadlock
	btfss	STATUS,Z	;  if (bsr)             // arising from an
	bsf	INTCON,GIE	;   INTCON &= 1<<GIE;   // interrupt right now
#endif
	movf	WREG,w		;
	btfsc	STATUS,Z	;
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
	lslf	FSR#v(fsrn)L,f	;inline void zOS_INI(uint8_t* fsrnum, uint8_t
	movlw	0x70		;                    val0, uint8_t val1) {
	iorwf	FSR#v(fsrn)L,f	; //fsrnum starts and ends as a launched job#
	clrf	FSR#v(fsrn)H	; fsrnum = 0x70 | (fsrnum << 1);
	movlw	val0		; // change global mailbox to non-0 if desired
	movwi	FSR#v(fsrn)++	; fsrnum[0] = val0;
	movlw	val1		;
	movwi	FSR#v(fsrn)--	; fsrnum[1] = val1;
	lsrf	FSR#v(fsrn),w	; fsrnum = (fsrnum >> 1) & 0x07; // unchanged
	andlw	0x07		;}
	endm


zOS_DIS	macro	fsrnum,job	;inline void zOS_DIS(int8_t* *fsr, int8_t job) {
	if (fsrnum & 3)
fsrn	 set 1
	else
fsrn	 set 0
	endif
	if (job)
	zOS_MEM	FSR#v(fsrn),job,zOS_HDH	; *fsr = 0x10 * (1+job) + zOS_HDH;//priv
	 btfsc	INDF#v(fsrn),zOS_PRB	; if (**fsr & (1<<zOS_PRB))
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
	btfss	INDF#v(fn),bsy	;  {                         // or never enabled
	bra	notbusy		;   zOS_ARG(0, bsr);
	movf	zOS_ME		;   zOS_SWI(zOS_YLD);
	zOS_ARG	0
	zOS_SWI	zOS_YLD		;  }
	bra	spinget		;  // interrupts now enabled if zOS_SWI called
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
	movf	zOS_ME		;  zOS_ARG(0, bsr);
	zOS_ARG	0
	zOS_SWI	zOS_YLD
	btfss	INDF#v(fn),bsy	;  zOS_YLD();
	bra	spinmul		; } while (**fsr & 1<<PID1BUSY);
	bcf	INTCON,GIE	; INTCON &= ~(1<<GIE);
	bcf	INDF#v(fn),enb	; // begin critical section (copying result)
	movlw	low inout	; **fsr &= ~(1<<enb); // disable MathACC to free
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
	movwf	zOS_AR0		; // end critical section (when ARx copy's done)
;;	bsf	INTCON,GIE	;} // zOS_MUL()
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
	bcf	FSR#v(fsrn)H,5	;
	swapf	FSR#v(fsrn)H,f	; 
	iorwf	FSR#v(fsrn)H,w	;
	swapf	FSR#v(fsrn)H,f	; return w = (fsrnum >> 4);
	bsf	FSR#v(fsrn)H,5	;} // zOS_PAG()
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

zOS_HEA	macro	base,size,mi,fi	;void zOS_HEA(void* base, void* size, uint8_t
	local	isr,decl,task	;             mi/*malloc*/,uint8_t fi/*free*/) {

	bra	decl		; goto decl;

	local	maxnon0,alloced,always0,temp,adrarry,tblsize
	local	tblrows,sizarry,memroun,mem3nyb,membase,memsize
maxnon0	set	0x6c
alloced	set	0x6d
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
	local	mloop,mcandid,mexact,mnotall,groloop
	local	free,floop,ffound,invalid,done
	
	movf	zOS_JOB,w	; isr:
	movwf	BSR		; bsr = zOS_JOB;

	zOS_MY2	FSR1		; fsr1 = 0x70|(bsr<<1);
	moviw	FSR1++		;
	iorwf	INDF1,w		;
	btfsc	STATUS,Z	; if (0[fsr1] | 1[fsr1])
	bra	invalid		;  goto invalid;// not init'ed according to mbox

#if (mi - fi)
	movf	zOS_MSK,w	;
	andlw	mi		; //////////////////////////////////////////////
	btfsc	STATUS,Z	; /////             malloc()                  //
	bra	free		; if (((mi != fi) && (zOS_MSK & mi)) ||
#else
	movf	zOS_AR1,w	;    ((mi == fi) && (zOS_AR0=/*sic*/zOS_AR1))) {
	movf	zOS_AR0,f	;  // can either assign separate SWIs for malloc
	movwf	zOS_AR0		;  // and free or if nearing the SWI limit of 5,
	btfsc	STATUS,Z	;  // put the parameter in ARG1 instead of ARG0
	bra	free		;  // and ARG0!=0 for malloc() or ==0 for free()
#endif
	zOS_LOC	FSR0,BSR,adrarry;  for (fsr0 = (bsr<<7)+adrarry,
	zOS_LOC	FSR1,BSR,sizarry;       fsr1 = (bsr<<7)+sizarry;
mloop
	moviw	FSR0++		;       (alloced = temp = *fsr0++);// next poss.
	btfsc	STATUS,Z	;       fsr1++) {
	bra	invalid		;
	movwf	temp		;
	movwf	alloced		;
	moviw	FSR1++		;   w = *fsr1++; // number of bytes used,0=freed
	btfss	STATUS,Z	;
	bra	mloop		;   if (w == 0) { // allocatable
mcandid
	moviw	0[FSR0]		;    w = *fsr0;// upper limit to allocating here
	btfsc	STATUS,Z	;    if (w == 0)
	bra	invalid		;     goto invalid; // past the highest address

	bsf	STATUS,C	;    // temp is now the address of this candidate
	comf	temp,f		;    // w is now the next address past candidate
	addwfc	temp,w		;
	movwf	temp		;
	subwf	zOS_AR0,w	;    else if ((w = zOS_AR0 - (temp = w-temp))>0)
	btfsc	STATUS,Z	;
	bra	mexact		;    // -w now holds extra space beyond requested
	btfss	WREG,7		;    // temp now holds total available at alloced
	bra	mloop		;
	bra	mnotall		;     continue; // not enough allocatable here

mexact
	movf	zOS_AR0,w	;    if (w == 0) { // exactly enough!
	movwi	-1[FSR1]	;     -1[fsr1] = zOS_AR0; // allocated size
	moviw	-1[FSR0]	;     w = -1[fsr0]; // recycled handle
	bra	done		;     goto done;
mnotall	
	movf	maxnon0,f	;    } else if (adrarry[tblrows-2] != 0) // full
	btfss	STATUS,Z	;     goto invalid;
	bra	invalid		;

	movf	zOS_AR0,w	; // w == addr to insert, temp == size to insert
	movwi	-1[FSR1]	;    -1[fsr1] = zOS_AR0; // record it as granted
	clrf	temp		;    temp = 0;
	addwf	alloced,w	;    for (w = -1[fsr0] + temp; *fsr0; fsr0++,fsr1++) {
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
	movwi	0[FSR1]		;    *fsr1 = w = temp;
	movf	alloced,w	;    w = alloced;
	bra	done		;    goto done; // return the fsr0 address added

free
	movf	zOS_MSK,w	; //////////////////////////////////////////////
	andlw	fi		; //////////       free()                ///////
	btfsc	STATUS,Z	;
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
	btfss	STATUS,Z	;
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
	
	zOS_NAM "heap allocator"
;	zOS_NAM	"malloc(),free(),garbage coll"
task
	local	iniarry,coalesc,coaloop,coscoot
	
	bcf	INTCON,GIE	;task:
	zOS_LOC	FSR0,BSR,0x70
iniarry
	clrw			; INTCON &= ~(1<<GIE);
	movwi	--FSR0		; for (fsr0 = (bsr<<7)|(adrarry+tblsize);
	movlw	adrarry		;      fsr > adrarry; fsr--)
	xorwf	FSR0L,w		;  *fsr = 0; // zero each address and size entry
	andlw	0x7f		;
	btfss	STATUS,Z	;
	bra	iniarry		;
	
	zOS_MY2	FSR1

	movlw	membase		; // except first address entry is start of heap
	movwi	0[FSR1]		; (0x70|(bsr<<1))[0] =
	movwi	0[FSR0]		; adrarry[0] = membase; // first allocatable
	movlw	membase+memsize	; // and second addres entry is the end of heap
	movwi	1[FSR1]		; (0x70|(bsr<<1))[1] =
	movwi	1[FSR0]		; adrarry[1] = membase+memsize;//max allocatable

coalesc
	movf	zOS_ME		; do { // combine adjacent rows whose size are 0
	zOS_ARG	0
	zOS_SWI	zOS_YLD
	zOS_LOC	FSR0,BSR,adrarry+1
	zOS_LOC	FSR1,BSR,sizarry
coaloop
	bcf	INTCON,GIE	;  zOS_ARG(0, bsr);
	moviw	++FSR0		;  zOS_SWI(zOS_YLD); // only 1 pass per schedule
	btfsc	STATUS,Z	;  INTCON &= ~(1<<GIE); // critical section (
	bra	coalesc		;  for (fsr0 = &adrarry[1], fsr1 = &sizarry[0];
	moviw	FSR1++		;       *++fsr0; fsr1++)
	btfss	STATUS,Z	;   if (0[fsr1] === 0 && 1[fsr1] == 0) {
	bra	coaloop		;    INTCON |= 1<<GIE;
	moviw	0[FSR1]		;    do {// fsr1->redun row siz,trails fsr0->adr
	btfss	STATUS,Z	;     INTCON &= ~(1<<GIE); // critical section (
	bra	coaloop		;     uint8_t w = *++fsr1;
coscoot
	moviw	++FSR1		;     -1[fsr1] = w;
	movwi	-1[FSR1]	;     w = *fsr0++;
	moviw	FSR0++		;    } while ((-2[fsr0] = w) != 0);
	movwi	-2[FSR0]	;    break; // ) critical section ended by SWI
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
	movwi	FSR#v(dst)++	;} // zOS_IHF()
	endm


zOS_UNW	macro	job		;inline void zOS_UNW(int8_t job) { }
	zOS_MEM	FSR0,job,zOS_PCH; fsr0 = 0x10 * (1 + job) + zOS_PCH;
	bcf	INDF0,zOS_WAI	; *fsr0 &= ~(1 << zOS_WAI); // now runnable
	endm			;} // zOS_UNW()

zOS_OUT	macro	swinum,str,temp
	local	agent,pre,post,setup,len,sloop,loop
	bra	setup		;inline void zOS_OUT(uint8_t swinum, char* str,
agent
	brw			;                    uint8_t* temp) { // no '\0'
pre	
	dt	str
post
len	set	post-pre
	if (len > 254)
	 error "string too long"
	endif
	
	if (len)
setup
	 movlw	len		; zOS_SWI(zOS_YLD); // get buffer empty as poss.
	 movwf	temp		; for (*temp = strlen(str); *temp; --*temp) {
sloop
	 movf	zOS_ME		;
	zOS_ARG	0
	zOS_SWI	zOS_YLD
loop
	 movf	temp,w		;  zOS_ARG(0, w = str[strlen(str) - *temp]);
	 sublw	len		;  while (zOS_SWI(swinum) != 1) { // buffer full
	pagesel	agent
	 call	agent		;   zOS_SWI(zOS_YLD); // flush buffer, retry
	zOS_ARG 0
	
	else
sloop
	movf	zOS_ME		;
	zOS_ARG	0
	zOS_SWI	zOS_YLD
setup
	 if (temp - zOS_AR0)
	  if (temp - WREG)
	   movf	temp,w		;
	  endif
	  zOS_ARG 0
	 endif
	endif
	
	zOS_SWI	swinum
	decfsz	WREG		;   zOS_ARG(0, w = str[strlen(str) - *temp]);
	bra	sloop		;  }
	
	if (len)
	 decfsz	temp,f		; }
	 bra	loop		;} // zOS_OUT()
	endif
	endm

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
	banksel	EECON1
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
	banksel	PMCON1
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
	banksel	NVMCON1
	bcf	NVMCON1,NVMREGS	;inline void zOS_RDF(void) { // for NVM micros
	bsf	NVMCON1,RD	; NVMCON1 &= ~(1<<CFGS); NVMCON1 |= 1<<RD;
#endif
#endif
#endif
	endm			;} // zOS_RDF()
	
zOS_STR	macro	swinum
	local loop,done
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
	zOS_OUT	swinum,"",zOS_AR0
	bcf	INTCON,GIE	;   zOS_POP(&bsr); // back to the expected bank
	zOS_PSH	BSR
	banksel	zOS_RDL
	movf	zOS_RDL,w	;   zOS_OUT(swinum,"",zOS_AR0); // print ASCII
	andlw	0x7f		;   INTCON &= ~(1<<GIE); // undo SWI GIE toggle
	btfsc	STATUS,Z	;   zOS_PSH(&bsr);
	bra	done		;   if ((w = zOS_RDL & 0x7f) != '\0') {
	movwf	zOS_AR0		;    zOS_ARG(0, w);
	zOS_POP	BSR
	zOS_OUT	swinum,"",zOS_AR0
	bcf	INTCON,GIE	;    zOS_POP(&bsr); // back to the expected bank
	zOS_PSH	BSR
	banksel	zOS_ADL
	incfsz	zOS_ADL,f	;    zOS_SWI(swinum,"",zOS_AR0); // print ASCII
	bra	loop		;    INTCON &= ~(1<<GIE); // undo SWI GIE toggle
	incf	zOS_ADH,f	;    zOS_PSH(&bsr);
	bra	loop		;   } else break;
done
	zOS_POP	BSR		;  } else break;
	bsf	INTCON,GIE	; } zOS_POP(&bsr); INTCON |= 1<<GIE;
	endm			;} // zOS_STR()

zOS_PUT	macro	fsrnum,max,wrap,p
	local	fsrn
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif
	movwi	FSR#v(fsrn)++	;inline int8_t zOS_PUT(char**fsrnum,uint7_t max,
	movf	FSR#v(fsrn)L,w	;                 char* wrap, char* p, char w) {
	andlw	0x7f		; *(*fsrnum)++ = w;
	xorlw	max		; // w gets put in buffer regardless, but caller
	swapf	wrap,w		; // only updates the local pointer if not full
	btfss	STATUS,Z	; // (i.e. Z not set) by xor return value with p
	swapf	FSR#v(fsrn)L,w	; *fsrnum = (*fsrnum&0x7f==max) ? wrap :*fsrnum;
	swapf	WREG		; return (*fsrnum & 0x00ff) ^ p; //0 if full, or
	movwf	FSR#v(fsrn)L	;              // new pointer value xor p if not
	xorwf	p,w		;} // zOS_PUT()
	endm
	
zOS_BUF	macro	fsrnum,max,ptr
	local	ascii,err1,done
	local	fsrn
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif
	lsrf	zOS_ME		;inline int8_t zOS_BUF(char**fsrnum,uint7_t max,
	movwf	FSR#v(fsrn)H	;          char** ptr, char w) { // p0, p1, wrap
	movf	1+ptr,w		; // must be in job bank already, interrupts off
	movwf	FSR#v(fsrn)L	; fsr0 = (bsr<<7) | ptr[1]; // insertion pointer

	movf	zOS_AR0,w	; if ((w = zOS_AR0) == 0) { // 2-digit hex byte
	btfss	STATUS,Z	;  w = zOS_HEX(zOS_AR1>>4); // convert high nyb
	bra	ascii		;  w = zOS_PUT(fsrnum, max, ptr[0], w); // room?

	swapf	zOS_AR1,w	;  if (w == 0)
	zOS_HEX
	zOS_PUT	fsrnum,max,2+ptr,ptr
	btfsc	STATUS,Z	;   return 0; // buffer was full
	bra	done		;  ptr[1] = w^ptr[0]; // correctly updated
	xorwf	ptr,w		;  w = zOS_HEX(zOS_AR1);// convert low nybble
	movwf	1+ptr		;  w = zOS_PUT(fsrnum, max, ptr[0], w); // room?
	
	movf	zOS_AR1,w	;  if (w == 0)
	zOS_HEX
	zOS_PUT	fsrnum,max,2+ptr,ptr
	btfsc	STATUS,Z	;   return 1; // buffer filled after first char
	bra	err1		;  ptr[1] = w^ptr[0]; // correctly updated
	xorwf	ptr,w		;  w = 2;
	movwf	1+ptr		; } else { // print an ascii character
	movlw	2		;  if ((w = zOS_PUT(fsrnum,max,ptr[0],w)) == 0)
	bra	done		;   return 0; // buffer was full
ascii
	zOS_PUT	fsrnum,max,2+ptr,ptr
	btfsc	STATUS,Z	;  ptr[1] = w^ptr[0]; // correctly updated
	bra	done		;  w = 1;
	xorwf	ptr,w		; }
	movwf	1+ptr		; return w; // num of characters added to buffer
err1
	movlw	1		;} // zOS_BUF()
done
	endm

zOS_NUL	macro	hwflag		;void zOS_NUL(void) { // replacement for zOS_CON
	bra	decl		; goto decl;
	local	task,isr,decl	; task: do {
task
	movf	zOS_ME		;  zOS_ARG(0, bsr);
	zOS_ARG	0
	zOS_SWI	zOS_YLD		;  zOS_SWI(zOS_YLD);
	bra	task		; } while (1);
	
isr
	banksel	zOS_T0F		; isr:
	bcf	zOS_T0F,T0IF	;  zOS_T0F &= ~(1<<T0IF);// clear interrupt flag
	zOS_RFI			;  zOS_RFI(); // and go back to scheduler
	
decl
	zOS_ADR	task,zOS_UNP	; fsr0 = task & 0x7fff;// MSB 0 => unprivileged
	movlw	low isr		; w = zOS_ARG(0, isr & 0x00ff);
	zOS_ARG	0
	movlw	high isr	; w = zOS_ARG(1, isr>>8);
	zOS_ARG	1		; w = zOS_ARG(2, 1<<T0IF);
	movlw	hwflag		; w = zOS_ARG(3, 0 /* no SWI */);
	zOS_ARG	2
	clrw		  	;} // zOS_NUL()
	zOS_ARG	3
	movlb   0               ; // still in job "0": don't forget this!!!!
	endm

zOS_CON	macro	p,rat,rts,hb,pin;inline void zOS_CON(int8_t p,int8_t rat,int8_t
	local	contask,conisr,inited,conloop,condecl
	bra	condecl		;                    rts,int8_t* hb,int8_t pin){

	;; initialize constants and variables
	local	t0div,t0rst
t0div	set 0
t0rst	set 1

	local	p0,p1,wrap,t0scale,isradrl,isradrh,tskadrl,tskadrh,optadrl
	local	optadrh,accumul,accumuh,numbase,destreg,destreh,char_io,buf,max

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
max	set	0x70

;copy the preceding lines rather than including this file, as definitions for
;zOS_MON()-derived macros referring to these local variables wouldn't open it
;until expansion and would throw an undefined-var error during the processing

	local	uatbase,uatxmit
	if (p == 0)
uatbase	 set	TXREG & 0xff80
uatxmit	 set	TXREG & 0x001f ; mask off just the SFR space
rtsflag	 set	TXIF	
	else
uatbase	 set	TX#v(p)REG & 0xff80
uatxmit	 set	TX#v(p)REG & 0x001f ; mask off just the sfr SFR
rtsflag	 set	TX#v(p)IF	
	endif
	zOS_NAM	"console (output-only)"
contask
	movlw	high uatbase	; goto decl;
	movwf	FSR0H		;task:// all init that requires knowledge of BSR
	zOS_MY2	FSR0
	moviw	t0div[FSR0]	; do {
	btfss	STATUS,Z	;  fsr0 = (uatbase & 0xff00) | 0x0070 |(bsr<<1);
	bra	inited		;  if (1[fsr0] == 0) { // not initialized yet
	zOS_DIS	GIE,0
	movlw	0xff		;  zOS_DIS(&fsr0, zOS_JOB); // interrupts off!
	movwi	t0div[FSR0]	;  0[fsr0] = 0xff;// live TMR0 postscaler divider
	movlw	0x00		;
	movwi	t0rst[FSR0]	;  1[fsr0] = 0x00; // live reset value for TMR0
	rrf	zOS_ME		;
	clrw			;  const char* max = 0x70;
	rrf	WREG		;  static char *p0, *p1, buf[]; //p0:task, p1:ISR
	iorlw	buf		;  const char* wrap = ((bsr&1)<<7) | buf;
	movwf	wrap		;  p0 = p1 = wrap; // reset value if they max out
	movwf	p0		;  zOS_ENA(); // interrupts on after init done
	movwf	p1		;  puts("\r\nWelcome to zOS\r\n");
	zOS_ENA	;//FIXME: superfluous due to subsequent SWI
	zOS_OUT	0xff,"\r\nWelcome to zOS\r\n",char_io
inited	
	movf	zOS_ME		;  zOS_ARG(0, bsr);
	zOS_ARG	0
	zOS_SWI	zOS_YLD		; 
	movlw	low uatbase	;  const int8_t* uatbase = uatxmit & 0xff80;
	movwf	FSR0L		;  fsr0 = uatbase;
	movlw	high rts	;  zOS_ARG(0, bsr);
 	movwf	FSR1H		;  zOS_SWI(zOS_YLD);
	movlw	low rts		;  // wait for SWI to store char(s) in buf[]
	movwf	FSR1L		;
	btfss	INDF1,rtsflag	;  if (*(fsr1 = rts) & (1<<rtsflag) == 0) //full
	bra	conloop		;   continue; //yield (still sending or no char)
	lsrf	zOS_ME 		;
	movwf	FSR1H		;  // READY TO SEND, AND...       
	zOS_DIS	GIE,0
	movf	p0,w		;  // begin critical section (freeze pointers)
	movwf	FSR1L		;
	xorwf	p1,w		;  fsr1 = (bsr<<7) | p0;
	btfsc	STATUS,Z	;  if (p0 == p1)
	bra	conloop		;   continue; // nothing to do
	moviw	FSR1++		;
	movwi	uatxmit[FSR0]	;  uatxmit[fsr0] = *fsr1++; // send a character
	movf	FSR1L,w		;
	movwf	p0		;  p0 = fsr1 & 0x00ff; // wrap around to buf+0
	andlw	0x7f		;
	xorlw	max		;
	btfss	STATUS,Z	;
	bra	conloop		;  if (p0 & 0x7f == max) // ignore low bank bit
	movf	wrap,w		;   p0 = wrap; // =buf xor the lowest bank bit
	movwf	p0		;  // end critical section
conloop
	zOS_ENA
	zOS_MEM	FSR0,BSR,0
	moviw	zOS_HDH[FSR0]	;
	movwf	PCLATH		;
	moviw	zOS_HDL[FSR0]	;
	movwf	PCL		; } while (1); // e.g. might run zOS_INP's task

	;; HWI will be coming from a tmr0 expiration, for the blinking heartbeat
	;; 
	;; SWI will be coming from a job that wants to send a character
	;;  in which case the ISR stores it, advancing p1 and returning the
	;;  number of characters stored in the buffer
	;; Note: caller needs to make sure to check status of return value for
	;; != 0, just in case job is in between sleeps or with a full buffer
conisr
	local	done,do_swi,nottmr
	
	;; if it's a simple and frequent timer overflow interrupt finish quickly
	banksel	zOS_T0F
	btfss	zOS_T0F,T0IF	; if (/*presumed true:(zOS_T0E & (1<<T0IE)) &&*/
	bra	nottmr		;     (zOS_T0F & (1<<T0IF))) { // timer overflow
	bcf	zOS_T0F,T0IF	;  zOS_T0F &= ~(1<<T0IF);// clear interrupt flag

	;; get fsr0 pointing to tmr0 postscaler/reset value
	movf	zOS_JOB,w	;isr:
	movwf	BSR		; bsr = zos_job;
	zOS_MY2	FSR0L		; fsr0 = 0x70 | (bsr < 1);

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
	movlw	1		;   if (*fsr0 == 0) // disallow zero postscaler
	movwf	INDF0		;    *fsr0 = 1;
	movwf	INDF1		;   *fsr1 /*countdown*/ = *fsr0 /*postscaler*/;
	movlw	(1<<pin)	;
	xorwf	hb,f		;   hb ^= 1 << pin;
	bra	done		; } else {
	
	;; check for validated SWI first since it will be in zOS_MSK, else a HWI
nottmr
	movf	zOS_MSK,f	;  if (zOS_MSK) { // a SWI to buffer a character
	btfss	STATUS,Z	;   w = zOS_BUF(&fsr0, max, p0); // zOS_AR0,_AR1
	bra	do_swi		;   zOS_RFS(w); } else zOS_RET(); // not ours(!)
	zOS_RET
	
	;; point fsr0 to uatbase (again?), point fsr1 to p0
do_swi
	movf	zOS_JOB,w	;
	movwf	BSR		;
	zOS_BUF	FSR0,max,p0	; }
	zOS_RFS	WREG		; zOS_RFI(); // HWI finished
done
	zOS_RFI			;
	
	;; intialize the UART peripheral, job handle and first three arguments
condecl
	banksel	uatbase
	bcf	RCSTA,SPEN	;decl: // all init that is BSR independent here
	bcf	RCSTA,CREN	; RCSTA &= ~((1<<SPEN)|(1<<CREN));
	bcf	TXSTA,TXEN	; TXSTA &= ~(1<<TXEN);
	local brgval,brgvalm,brgvalh,brgvall
#ifdef BRG16
brgval	set	rat>>2
brgvalm	set	brgval-1
brgvalh	set	high brgvalm
brgvall	set	low brgvalm
	bsf	BAUDCON,BRG16	; // section 26.1.2.8 of 16F1847 steps below:
#ifdef SYNC
	bcf	TXSTA,SYNC	; // (1) "Initialize..the desired baud rate"
#else
	bcf	TXSTA,SYNC_TXSTA
#endif
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
	movlw	brgvall		;
	movwf	SPBRG		; SPBRG = (rat/16) - 1;
#endif
	bsf	RCSTA,SPEN	; // (3) "Enable..by setting..SPEN"
	bcf	RCSTA,RX9	; RCSTA &= ~(1<<RX9);  // (5) "9-bit..set..RX9"
	bsf	RCSTA,CREN	; RCSTA |= (1<<SPEN) | (1<<CREN); // (6) "CREN" 
	bsf	TXSTA,TXEN	; TXSTA |= 1<<TXEN; // (5) "Enable..by..TXEN"
	banksel	PIE1
	bsf	PIE1,RCIE	; PIE1 |= 1<<RCIE; //(4) "Set..RCIE..and..PEIE"
	zOS_ADR	contask,zOS_PRB	; fsr0 = contask & 0x7fff;// MSB 1 => privileged
	movlw	low conisr	; w = zOS_ARG(0, conisr & 0x00ff);
	zOS_ARG	0
	movlw	high conisr	; w = zOS_ARG(1, conisr>>8);
	zOS_ARG	1		; w = zOS_ARG(2, (0<<TXIF)|(1<<T0IF));
	movlw (0<<TXIF)|(1<<T0IF)
	zOS_ARG	2
	movlb	0		; // still in job "0": don't forget this!!!!
	endm			;} // zOS_CON()

	;; remnants of an early experiment to allow bank changing outside ISR
	;; to read SFR's is now deprectated, only known use is in olirelay.asm
zOS_R	macro	file,bankf,prsrv;inline int8_t zOS_R(const int8_t* file, int8_t bank, int8_t prsrv) {
	if (prsrv)
	 movf	INTCON,w
	 bcf	INTCON,GIE
	 movwf	zOS_AR1
	else
	 bcf	INTCON,GIE
	endif
	if file & 0x60
	 error "tried to access disallowed RAM range (global or another job's)"
	endif
	banksel	file	   	; INTCON &= ~(1<<GIE); // access zOS_AR* globals
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
	
;;; like zOS_CON, but also accepts console input for command-line interaction
zOS_INP	macro	p,ra,rt,h,pi,isr;inline void zOS_INP(int8_t p, int8_t ra, int8_t
	local	rxtask,no_opt,rxisr,rxdecl
	bra	rxdecl		;      rt, int8_t* h, int8_t pi, void(*isr)()) {
	
	;; reserve constants and variables
	local	p0,p1,wrap,t0scale,isradrl,isradrh,tskadrl,tskadrh,optadrl
	local	optadrh,accumul,accumuh,numbase,destreg,destreh,char_io,buf,max

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
max	set	0x70

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
	
	zOS_NAM	"console I/O"
;;; FIXME: haven't actually written the var init code for zOS_MON et al yet
rxtask
	movf	optadrh,w	; goto rxdecl;
	movwf	PCLATH		;rxtask:
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
	movwf	PCL	;callw	; // will retreive its own address as a loop

rxisr	
	movf	zOS_JOB,w	;rxisr:
	movwf	BSR		; bsr = zOS_JOB; // isr starts with unknown bank
	
	movf	isradrh,w	;
	movwf	PCLATH		;
	movf	isradrl,w	; if (rt && (1<<RCIF) == 0) // SWI, not inp char
	banksel	rt
	btfss	rt,rxflag	;  goto (isradrh<<8)|isradrl;//zOS_CON takes SWI
	movwf	PCL		; else {
	bcf	rt,rxflag	;  rt &= ~(1<<RCIF);
#ifdef CAUTIOUS
	btfss	RCSTA,OERR	;
	bra	noovrrn		;  if ((uarbase | RCSTA) & (1<<OERR)) {
	movlw	'!'		;   zOS_AR0 = '!';
	movwf	zOS_AR0		;   zOS_BUF(zOS_JOB, p0);
	zOS_BUF	FSR0,max,p0	;  }
noovrrn
#endif
	banksel	uarbase
	movf	uarecv,w	;  // this read removes it from the FIFO
#ifdef CAUTIOUS
	btfss	RCSTA,OERR	;  if (RCSTA & (1<<OERR)) // rx overrun
	bcf	RCSTA,CREN	;   RCSTA &= ~(1<<CREN); // cleared by disable
	bsf	RCSTA,CREN	;  RCSTA |= 1<<CREN; // (re-)enable reception
#endif
	if (isr)
	 movwf	zOS_AR0		;  zOS_AR0 = RCREG;
	pagesel	isr		;  if (zOS_AR0)
	 btfss	STATUS,Z	;   goto isr; // continue with parser
	 goto	isr		;  zOS_RFI(); //return from interrupt
	endif
	zOS_RFI			; }

	local	vars,arg0,arg1,adrl,adrh,optl,opth,accl,acch,base,dstl,dsth,chio
vars	set	0x20
arg0	set	isradrl-vars	
arg1	set	isradrh-vars
adrl	set	tskadrl-vars
adrh	set	tskadrh-vars	
optl	set	optadrl-vars
opth	set	optadrh-vars	
accl	set	accumul-vars
acch	set	accumuh-vars
base	set	numbase-vars
dstl	set	destreg-vars
dsth	set	destreh-vars
chio	set	char_io-vars

rxdecl
	zOS_CON	p,ra,rt,h,pi
	zOS_LAU	FSR1H
	zOS_LOC	FSR1L,FSR1H,vars
	movf	zOS_AR0,w	;rxdecl:
	movwi	arg0[FSR1]	; zOS_CON(p,rat,rts,hb,pin);// extend zOS_CON()
	movf	zOS_AR1,w	; zOS_LAU(&fsr1);// by rewriting after launch
	movwi	arg1[FSR1]	; fsr1 <<= 7;
	movf	FSR0L,w		; isradr[fsr1] = (zOS_AR1<<8) | zOS_AR0;
	movwi	adrl[FSR1]	;
	movf	FSR0H,w		;
	movwi	adrh[FSR1]	; tskadr[fsr1] = fsr0; // still zOS_CON's handle
	movlw	0		;
	movwi	optl[FSR1]	; // caller sets optional task
	movwi	opth[FSR1]	; optadr[fsr1] = ((*void)()) 0; // no func
	movwi	accl[FSR1]	;
	movwi	acch[FSR1]	;
	movwi	dstl[FSR1]	;
	movwi	dsth[FSR1]	;
	movwi	chio[FSR1]	; char_io[fsr1] = 0; // zero = no action to take
	movlw	0x0a		;
	movwi	base[FSR1]	;
	rlf	FSR1L,w		; w = fsr1 >> 7; // restore zOS_LAU() job number
	rlf	FSR1H,w		;
	zOS_MEM	FSR0,WREG,0
	movlw	low rxtask	; fsr0 = 0x10 + w << 4;
	movwi	zOS_HDL[FSR0]	;
	movwi	zOS_PCL[FSR0]	;
	movlw	high rxtask	;
	movwi	zOS_PCH[FSR0]	; zOS_PC[fsr0] = rxtask;
	iorlw	0x80		;
	movwi	zOS_HDH[FSR0]	; zOS_HD[fsr0] = rxtask | 0x8000; 
	addfsr	FSR0,zOS_ISR	; fsr0 += zOS_ISR; // last 4 bytes of job record
	movlw	low rxisr	; *fsr0++ = rxisr & 0x00ff;
	movwi	FSR0++
	movlw	high rxisr	; *fsr0++ = rxisr >> 8;
	movwi	FSR0++
	movf	zOS_AR2,w	; *fsr0++ |= (1<<RCIF);// |(0<<TXIF)|(1<<T0IF));
	iorlw	1<<rxflag	; // still in job "0"; caller sets any SWI value
	movwi	FSR0++		;} // zOS_INP()
	endm


zOS_ACC	macro	valregs,basereg
	clrf	valregs		;inline uint8_t zOS_ACC(uint8_t* valregs,uint8_t
	clrf	1+valregs	;                   *basereg) { // w unclobbered
	clrf	basereg		; *valregs = 0;
	bsf	basereg,3	; return *basereg = 10; // decimal by default
	bsf	basereg,1	;} // zOS_ACC()
	endm
	

zOS_PCT	macro	reg
	movlw	0x7e 		; // 0 <= reg <= 100
	andwf	reg,w		; w = reg & 0x7e; // 0 <= w <= reg (even, trunc)
	lslf	reg,f 		;
	lslf	reg,f		; uint16_t c = reg *= 4; // 0 <= reg <= 400
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
	local	monchr5,monchr6,monchr7,monchr8,monchr9,monprmp,monlast,monpctg
	local	endmon
	
	pagesel	endmon		;      rt, int8_t* h, int8_t pi, void(*isr)()) {
	goto	endmon		; zOS_INP(p,ra,rt,h,pi,monisr); }// isr may be 0

	local	p0,p1,wrap,t0scale,isradrl,isradrh,tskadrl,tskadrh,optadrl
	local	optadrh,accumul,accumuh,numbase,destreg,destreh,char_io,buf,max

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
max	set	0x70

;copy the preceding lines rather than including this file, as definitions for
;zOS_MON()-derived macros referring to these local variables wouldn't open it
;until expansion and would throw an undefined-var error during the processing

monback
	andlw	0x3f		;void monback(uint3_t job, uint8_t ptr, char w){
	btfsc	STATUS,Z	; if (w &= 0x3f) {
	return			;  // 63 \b's should be enough in a buffer of 64
	movwf	zOS_AR1		;  
#if 0	
monbac2
	movf	p0,w		; // don't actually want to wind back buffer;
	xorwf	p1,w		; // the point is show what will be overwritten
	btfsc	STATUS,Z	;
	bra	monbarn		;
	movf	p1,w		;
	xorwf	wrap,w		;
	movlw	max-1		;
	btfss	STATUS,Z	;
	movwf	p1		;
	btfsc	wrap,7		;
	bsf	p1,7		;
	decf	p1,f		;
	decfsz	zOS_AR1,f	;
	bra	monbac2		;
	return			;
monbarn
#endif
	movlw	0x08		;
	movwf	zOS_AR0		;  zOS_AR0 = '\b'; // FIXME: or '\0177'?

monloop
	zOS_BUF	FSR0,max,p0
	andlw	0x1		;  for (zOS_AR1 = w; zOS_AR1; zOS_AR1--) {
	btfsc	STATUS,Z	;   if (zOS_BUF(job, ptr) == 0) // buff full
	return			;    return;
	decfsz	zOS_AR1,f	;  }
	bra	monloop		; }
	return			;} // monback() monloop()
	
monout
	movf	wreg		;void monout(char w, char zOS_AR1) {
	btfss	STATUS,Z	; if (w) monbufs(w);
	bra	monbufs		; else
	movf	zOS_AR1,w	;  monlsb(w = zOS_AR1);
	bra	monlsb		;}
monhex
	movf	accumuh,w	;void monhex(void) { monlsb(,,w = accumuh); }
monlsb
	clrf	zOS_AR0		;void monlsb(uint3_t job, uint8_t ptr, char w) {
	movwf	zOS_AR1		;
	zOS_BUF	FSR1,max,p0	; return zOS_BUF(job,ptr,w); } // 0/1/2 printed
	return			;} // monlsb
	
mon0
	movlw	'0'		;void mon0(void) { zOS_AR0 = '0'; monbufs(ptr);
	bra	monbufs		;}

monx
	movlw	'x'		;void monx(void) { zOS_AR0 = '0'; monbufs(ptr);
	bra	monbufs		;}

monspc
	movlw	' '		;void monspc(void) { zOS_AR0 = ' '; monbufs(ptr);
	bra	monbufs		;}
monlf
	movlw	'\n'		; return zOS_BUF(zos_job, ptr, w);
monbufs
	movwf	zOS_AR0		;} // moncrlf() monlf()
monbufd
	movlw	1		;void monbufs(uint8_t ptr, char w) {
	movwf	zOS_AR1		; goto monloop();
	bra	monloop		;} //FIXME: these comments above are useless
	
monisr
	movf	zOS_JOB,w	;void monisr(void) {
	movwf	BSR		;  bsr = zos_job;// to access char_io var et al
	pagesel	monbufd
	movlw	0xe0		; // from zOS_INP isr with char zOS_AR0>0
	addwf	zOS_AR0,w	;
	btfss	WREG,7		; // refuse to echo unprintable characters
	call	monbufd		; if (zOS_AR0 > 31 && monbuf(zos_job,p0) > 0) {
	andlw	0x1		;  // successful echo into circular buffer
	pagesel	monlast
	btfsc	STATUS,Z	;
	goto	monlast		;
	
	movf	zOS_AR0,w	;  // handle '~' before the tolower() conversion
	xorlw	'~'		;
	btfss	STATUS,Z	;
	bra	monchr1		;  if (zOS_AR0 == '~') {
	pagesel	mon0
	call	mon0		;
	pagesel	monx
	call	monx		;
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
	movlw	0x7f		;
	btfss	STATUS,Z	;  case '\b':
	movf	char_io,w	;
	xorlw	0x7f		;
	btfss	STATUS,Z	;  case '\0177':
	bra	monchr2		;
	movlw	'\r'		;
	pagesel	monbufs
	call	monbufs		;   monbuf(zos_job, p0, '\r');
	bra	monprmp		;   goto monprmp;
	
monchr2
	movf	char_io,w	;
#if 0
	xorlw	0x0a		;
	movlw	0x0d		;
	btfss	STATUS,Z	;  case '\n':
	movf	char_io,w	;
#endif	
	xorlw	0x0d		;
	btfss	STATUS,Z	;  case '\r':
	bra	monchr3		;   monbuf(zos_job, p0, '\n');// follows the \r
	movlw	'\r'		;
	pagesel	monbufs		;
	call	monbufs		;
	movlw	'\n'		;
	pagesel	monbufs		;
	call	monbufs		;
	
	movf	destreg,w	;   // repeat \r's can set a whole range of
	movwf	FSR0L		;   // addresses to zero???
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
	xorlw	','		;
	movlw	' '		;
	btfsc	STATUS,Z	;
	movwf	char_io		;  case ',': // synonym for ' '
	movf	char_io,w	;
	xorlw	' '		;
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
	movf	char_io,w	;   if (char_io == ' ') {
	xorlw	' '		;    char_io = 0; // all we do is a destreg xfer
	btfsc	STATUS,Z	;    break;
	bra	monzero		;   }
mondest
	btfss	1+destreg,7	;   if (destreg & 0x8000) { // flash, not RAM
	bra	monram		;
	pagesel	mon0
	call	mon0		;    putchar('0');
	pagesel	monx
	call	monx		;    putchar('x');
	movf	destreg,w	;
	movwf	FSR0L		;
	movf	1+destreg,w	;
	movwf	FSR0H		;    fsr0 = destreg;
	zOS_PSH	BSR
	banksel	zOS_ADL
	movf	FSR0L,w		;    zOS_PSH(&bsr);
	movwf	zOS_ADL		;
	movf	FSR0H,w		;
	movwf	zOS_ADH		;    zOS_AD = fsr0;
	zOS_RDF
	movf	zOS_RDH,w	;    zOS_RDF();
	movwf	zOS_AR0		;    zOS_ARG(0,zOS_RDH); // only way to access
	zOS_POP	BSR
	movf	zOS_AR0,w	;    zOS_POP(&bsr);
	movwf	accumuh		;
	pagesel	monhex
	call	monhex		;    monhex(zos_job, p0, accumuh=0);// high byte
	movf	destreg,w	;
	movwf	FSR0L		;
	movf	1+destreg,w	;
	movwf	FSR0H		;    fsr0 = destreg; // monhex() clobbered fsr0
	moviw	FSR0++		;
	movwf	accumul		;
	movf	FSR0L,w		;
	movwf	destreg		;    accumuh = *fsr0++;
	movf	FSR0H,w		;    destreg = fsr0;
	movwf	1+destreg	;    monlsb(zos_job, p0, accumuh);  //      LSB
	movf	accumul,w	;
	pagesel	monlsb	
	call	monlsb		;    moncrlf(zos_job, p0);          //     \r\n
#ifdef zos_opc
	pagesel	zos_opc
	goto	zos_opc		;    zos_opc(); // disassemble accumu, jump back
zos_opr
#endif	
	movlw	'\r'
	pagesel	monbufs
	call	monbufs
	pagesel	monlf
	call	monlf		;    goto monprmp;
	bra	monprmp		;   }
	
monram
	pagesel	mon0
	call	mon0		;
	pagesel	monx
	call	monx		;
	movf	destreg,w	;
	movwf	FSR0L		;
	movf	1+destreg,w	;
	movwf	FSR0H		;   fsr0 = destreg;
	moviw	FSR0++		;
	movwf	accumuh		;   accumuh = *fsr0++;
	pagesel	monhex
	call	monhex		;   monhex(p0, accumuh);

	movf	char_io,w	;
	xorlw	'.'		;   // then exits in the '.' case to just print
	btfss	STATUS,Z	;   if (char_io == '.') {
	bra	monramd		;
	movf	FSR0L,w		;
	movwf	destreg		;
	movf	FSR0H,w		;
	movwf	1+destreg	;    destreg = fsr0;
	movlw	'\r'		;    monbufs('\r');
	pagesel	monbufs
	call	monbufs		;    monbufs('\n');
	pagesel	monlf
	call	monlf		;    goto monprmp;
	bra	monprmp		;   }
monramd
	movf	char_io,w	;   // or follow by 3 backspaces in the '=' case
	xorlw	'.'		;   // to show that \r will result in a 0 write
	btfss	STATUS,Z	;
	movlw	3		;
	pagesel	monback
	call	monback		;   monback(zos_job, p0, (char_io == '.')?0:3);
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
	btfsc	WREG,7		;
	bra	monpctg		;   if (accumul > 102)
	movlw	0x66		;
	movwf	accumul		;    accumul = 102;
monpctg
	movf	accumul,w	;   accumul = zOS_PCT(accumul);
	zOS_PCT	accumul
	movf	accumul,w	;  monecho:
	movwf	accumuh		;   accumuh = accumul;
	pagesel	monhex		;   monhex(zos_job, p0); print as e.g. 50%0x7d
	call	monhex		;   accumuh = 0;
	clrf	accumuh		;   char_io = 0;
	clrf	char_io		;  break;
	zOS_RFI
	
monchr6
	movlw	0-0x30		;  default:
	addwf	char_io,f	;
	btfsc	char_io,7	;
	bra	monchr9 	;   if ((char_io -= ('0'&0xdf /*0x10*/)) >= 0) {
	movlw	0-0x10		;
	addwf	char_io,w	;
	btfsc	WREG,7		;    if (char_io > 0x10)
	bra	$+3		;
	movlw	0xf9		;
	addwf	char_io,f	;     char_io -= 0x07;// 0x41->0x11->0x0a... so
#if 0;seems unnec 18 Jan
	movf	char_io,f	;                     // now in range 0x00-0x09,
#endif
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
	movlw	0xf0		;
	andwf	char_io,w	;
	btfss	STATUS,Z	;    } else if ((char_io & 0xf0 == 0) // 0-9,a-f
	bra	monsave		;              && (numbase & 0x10)) { // base 16
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
	iorwf	char_io,w	;     accumul = (accumul << 4) | char_io;
	movwf	accumul		;     char_io = 0;
	clrf	char_io		;     break;
	zOS_RFI
	
monchr8
	movf	char_io,w	;    } else /*if (char_io <= 9)*/ {
	andlw	0xf0		;     uint16_t sum;
	btfss	STATUS,Z	;     accumuh <<= 1;
	bra	monsave		;     accumuh |= (accumul & 0x80) ? 1 : 0;
				;     accumul <<= 1;
	lslf	accumul,f	;     w = accumul;//w keeps original accumul<<1
	rlf	accumuh,f	;     accumuh <<= 1;
	movf	accumul,w	;     accumuh |= (accumul & 0x80) ? 1 : 0;
				;     accumul <<= 1;
	lslf	accumul,f	;     accumuh |= (accumul & 0x80) ? 1 : 0;
	rlf	accumuh,f	;     accumul <<= 1; // accumuh:accumul <<= 3;
				;     if (numbase & 2) { // base 10 presumed
	lslf	accumul,f	;      sum = (accumuh<<8)+accumul + w;
	rlf	accumuh,f	;      accumul = sum & 0x00ff;
	btfss	numbase,1	;      accumuh = sum >> 8;
	bra	$+4		;     }
	addwf	accumul,f	;     sum = (accumuh<<8)+accumul + char_io&0x0f;
	movlw	0		;     accumul = sum & 0x00ff;
	addwfc	accumuh,f	;     accumuh = sum >> 8;
	movf	char_io,w	;     break;
	andlw	0x0f		;    }
	addwf	accumul,f	;   } // if we get here, restore input character
	movlw	0		;   char_io += 0x37; // 0x10->'G',0x11->'H' etc.
	addwfc	accumuh,f	;   zOS_AR1 = accumul;
	zOS_RFI
monchr9
	movlw	0-0x37		;   if (isr) goto isr; // with zOS_AR1=accumul
monsave
	movlw	0x37		;  } // switch ()
	addwf	char_io,f	;  char_io = 0;
	movf	accumul,w	; } // if () // was less than 32 so aborts
	movwf	zOS_AR1		;
	if (isr)
	pagesel	isr
	 goto	isr		; zOS_RFI(); // reached only if isr == 0
	else
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
	pagesel	monspc
	call	monspc		;    putchar(' ');
monzero
	zOS_ACC	accumul,numbase
monlast
	clrf	char_io		;} // zOS_MON()
	zOS_RFI
endmon
	zOS_INP	p,ra,rt,h,pi,monisr 
	endm	

zOS_DEC	macro	putch,enc,retadr;e.g. zOS_DEC monout,accumul,zos_opr
	
	local	puts,loop,done	; NOT zos_opc because my purpose is to define it
	local	
	local	top1bcf,top1bsf,top1btc,top1bts,calllit,gotolit,endopc
	local	literal,litbyte,calllit,gotolit,lit11,opc_lit,opccall,opcgoto
	local	destreg,namereg,putname,flagreg,opc_reg
	local	regnam0,regnam1,regnam2,regnam3,regnam4,regnam5
	local	regnam6,regnam7,regnam8,regnam9,regnamA,regnamB
	local	overld0,omnibus,overld1,clr_reg,overld2,paging,overld3,moviwwi

puts
	banksel	zOS_ADL
	movf	FSR0L,w		;void puts(char w) {
	movwf	zOS_ADL		;
	movf	FSR0H,w		;//FIXME: needs comments
	movwf	zOS_ADH		;
loop
	zOS_RDF			;
	rlf	zOS_RDL,w	;  zOS_RDF(); // read packed 14-bit contents
	rlf	zOS_RDH,w	;
	btfsc	STATUS,Z	;
	bra	done		;  if ((w = (zOS_RDH<<1)|(zOS_RDL>>7)) != '\0'){
	pagesel	putch
	call	putch		;
	
	banksel	zOS_RDL
	movf	zOS_RDL,w	;
	andlw	0x7f		;
	btfsc	STATUS,Z	;
	bra	done		;
	pagesel	putch
	call	putch		;

	banksel	zOS_ADL		;
	incfsz	zOS_ADL,f	;
	bra	loop		;
	incf	zOS_ADH,f	;
	bra	loop		;
done
	return			;} // puts()
zos_opc	
	movlw	0x3f		;void zos_opc(uint14_t enc) {
	andwf	1+enc,w		; uint8_t w = (enc &= 0x3fff) >> 8;
	movwf	1+enc		; /*00--nn=>00,01--nn=>01,10--nn=>10,11--nn=>00/
	btfsc	1+enc,5		; if (enc & 0x0800 == enc & 0x0400)
	btfsc	1+enc,4		;  enc &= 0x00ff; /*for literal/destreg offset*/
	clrf	1+enc		; switch (w) { case 0: /*
	brw			;movwf/callw/movlb/brw/retfie/return/clrwdt/nop/
	bra	overld0		;option/reset/sleep/tris/mov[wi]*/ goto overld0;
	bra	overld1		;/* 0x01nn=>clrf/clrw*/ case 1: goto overld1;
	bra	destreg-0x12	;/* 0x02nn => subwf  */ case 2: goto destreg-18;
	bra	destreg-0x11	;/* 0x03nn => decf   */ case 3: goto destreg-17;
	bra	destreg-0x10	;/* 0x04nn => iorwf  */ case 4: goto destreg-16;
	bra	destreg-0xf	;/* 0x05nn => andwf  */ case 5: goto destreg-15;
	bra	destreg-0xe	;/* 0x06nn => xorwf  */ case 6: goto destreg-14;
	bra	destreg-0xd	;/* 0x07nn => addwf  */ case 7: goto destreg-13;
	bra	destreg-0xc	;/* 0x08nn => movf   */ case 8: goto destreg-12;
	bra	destreg-0xb	;/* 0x09nn => comf   */ case 9: goto destreg-11;
	bra	destreg-0xa	;/* 0x0ann => incf   */case 10: goto destreg-10;
	bra	destreg-9	;/* 0x0bnn => decfsz */case 11: goto destreg-9;
	bra	destreg-8	;/* 0x0cnn => rrf    */case 12: goto destreg-8;
	bra	destreg-7	;/* 0x0dnn => rlf    */case 13: goto destreg-7;
	bra	destreg-6	;/* 0x0enn => swapf  */case 14: goto destreg-6;
	bra	destreg-5	;/* 0x0fnn => incfsz */case 15: goto destreg-5;

	bra	top1bcf		;/*0x10nn to 0x13nn => bcf*/ case 16:
	bra	top1bcf		;                           case 17:
	bra	top1bcf		;                          case 18:
	bra	top1bcf		;                         case 19: goto top1bcf;
	bra	top1bsf		;/*0x14nn to 0x17nn => bsf*/ case 20:
	bra	top1bsf		;                           case 21:
	bra	top1bsf		;                          case 22:
	bra	top1bsf		;                         case 23: goto top1bsf;
	bra	top1btc		;/*0x18nn to 0x1bnn => btfsc*/case 24:
	bra	top1btc		;                           case 25:
	bra	top1btc		;                          case 26:
	bra	top1btc		;                         case 27: goto top1btc;
	bra	top1bts		;/*0x1cnn to 0x1fnn => btfss*/case 28:
	bra	top1bts		;                           case 29:
	bra	top1bts		;                          case 30:
	bra	top1bts		;                         case 31: goto top1bts;

	bra	calllit		;/* 0x20nn to 0x27nn => call */ case 32:
	bra	calllit		;                              case 33:
	bra	calllit		;                             case 34:
	bra	calllit		;                            case 35:
	bra	calllit		;                           case 36:
	bra	calllit		;                          case 37:
	bra	calllit		;                         case 38:
	bra	calllit		;                        case 39: goto calllit;
	bra	gotolit		;/* 0x28nn to 0x2fnn => goto */ case 40:
	bra	gotolit		;                              case 41:
	bra	gotolit		;                             case 42:
	bra	gotolit		;                            case 43:
	bra	gotolit		;                           case 44:
	bra	gotolit		;                          case 45:
	bra	gotolit		;                         case 46:
	bra	gotolit		;                        c:ase 47: goto gotolit;

	bra	literal-6	;/* 0x30nn => movlw */ case 48: goto literal-6;
	bra	overld2		;/*0x31nn => movlp/addfsr*/case 49:goto overld2;
	bra	brafwd		;/*0x32nn => bra(negative)*/case 50:goto brafwd;
	bra	brarev		;/*0x33nn => bra(positive)*/case 51:goto brarev;
	bra	literal-5	;/* 0x34nn => retlw */ case 52: goto literal-5;
	bra	destreg-4	;/* 0x35nn => lslf  */ case 53: goto destreg-4;
	bra	destreg-3	;/* 0x36nn => lsrf  */ case 54: goto destreg-3;
	bra	destreg-2	;/* 0x37nn => asrf  */ case 55: goto destreg-2;
	bra	literal-4	;/* 0x38nn => iorlw */ case 56: goto literal-4;
	bra	literal-3	;/* 0x39nn => andlw */ case 57: goto literal-3;
	bra	literal-2	;/* 0x3ann => xorlw */ case 58: goto literal-2;
	bra	destreg-1	;/* 0x3bnn => subwfb*/ case 59: goto destreg-1;
	bra	literal-1	;/* 0x3cnn => sublw */ case 60: goto literal-1;
	bra	destreg-0	;/* 0x3dnn => addwfc*/ case 61: goto destreg-0;
	bra	literal-0	;/* 0x3enn => addlw */ case 62: goto literal-0;
	bra	ovrld3		;/* 0x3fnn => movwi(offset)/moviw(offset)
	
;;; FIXME
top1bcf
	movwf	1+enc
top1bsf
	movwf	1+enc
top1btc
	movwf	1+enc
top1bts	
	movwf	1+enc
;;; FIXME
	
	incf	1+enc,f		; // opc_lit[6] = "movlw 0"
	incf	1+enc,f		; // opc_lit[5] = "retlw 0"
	incf	1+enc,f		; // opc_lit[4] = "iorlw 0"
	incf	1+enc,f		; // opc_lit[3] = "andlw 0"
	incf	1+enc,f		; // opc_lit[2] = "xorlw 0"
	incf	1+enc,f		; // opc_lit[1] = "sublw 0"
literal
	lslf	1+enc,w		; // opc_lit[0] = "addlw 0"
	lslf	WREG		;
	addlw	low opc_lit	;
	movwf	FSR0L		;
	movlw	high opc_lit	;
	movwf	FSR0H		;
	clrw			;
	addwfc	FSR0H,f		;
	pagesel	print
	call	print		; print(fsr0 = opc_lit[w]);
	movf	enc,f		;
	btfsc	STATUS,Z	;
	bra	endopc		; if (enc) {
	movlw	'x'		;
	pagesel	putch
	call	putch		;  putch('x');
litbyte
	movf	enc,w		;
	movwf	zOS_AR1		;
	clrw			;
	pagesel	putch
	call	putch		;
	bra	endopc		;
calllit
	movwf	1+enc		;
	zOS_ADR	opccall
	pagesel	print
	call	print		;
	bra	lit11		;
gotolit
	movwf	1+enc		;
	zOS_ADR	opcgoto
	pagesel	print
	call	print		;
lit11
	movf	1+enc,w		;
	andlw	0x07		;
	movwf	zOS_AR0		;
	clrw			;
	pagesel	putch
	call	putch		;
	bra	litbyte		;
	

	incf	1+enc,f		; // opc_reg[18] = "subwf  "
	incf	1+enc,f		; // opc_reg[17] = "decf   "
	incf	1+enc,f		; // opc_reg[16] = "iorwf  "
	incf	1+enc,f		; // opc_reg[15] = "andwf  "
	incf	1+enc,f		; // opc_reg[14] = "xorwf  "
	incf	1+enc,f		; // opc_reg[13] = "addwf  "
	incf	1+enc,f		; // opc_reg[12] = "movf   "
	incf	1+enc,f		; // opc_reg[11] = "comf   "
	incf	1+enc,f		; // opc_reg[10] = "incf   "
	incf	1+enc,f		; // opc_reg[9] = "decfsz "
	incf	1+enc,f		; // opc_reg[8] = "rrf    "
	incf	1+enc,f		; // opc_reg[7] = "rlf    "
	incf	1+enc,f		; // opc_reg[6] = "swapf  "
	incf	1+enc,f		; // opc_reg[5] = "incfsz "
	incf	1+enc,f		; // opc_reg[4] = "lslf   "
	incf	1+enc,f		; // opc_reg[3] = "lsrf   "
	incf	1+enc,f		; // opc_reg[2] = "asrf   "
	incf	1+enc,f		; // opc_reg[1] = "subwfb "
destreg
	lslf	1+enc,w		; // opc_reg[0] = "addwfc "
	lslf	WREG		;
	addlw	low opc_reg	;
	movwf	FSR0L		;
	movlw	high opc_reg	;
	movwf	FSR0H		;
	clrw			;
	addwfc	FSR0H,f		;
	pagesel	print
	call	print		;
	movf	enc,w		;
	andlw	0x7f		;
	movwf	zOS_AR1		;
	addlw	0-0x0c		;
	btfsc	WREG,7		;
	bra	namereg		;
	clrw			;
	pagesel	putch
	call	putch		;
	bra	flagreg		;
namereg
	movf	enc,w		;
	callw			;
	bra	putname		;
	retlw	regnam1-regnam0	;
	retlw	regnam2-regnam0	;
	retlw	regnam3-regnam0	;
	retlw	regnam4-regnam0	;
	retlw	regnam5-regnam0	;
	retlw	regnam6-regnam0	;
	retlw	regnam7-regnam0	;
	retlw	regnam8-regnam0	;
	retlw	regnam9-regnam0	;
	retlw	regnamA-regnam0	;
	retlw	regnamB-regnam0	;
putname
	addlw	low regnam0	;
	movwf	FSR0L		;
	movlw	high regnam0	;
	movwf	FSR0H		;
	clrw			;
	addwfc	FSR0H,f		;
	pagesel	print
	call	print		;
flagreg
	movlw	','		;
	pagesel	putch
	call	putch		;
	movlw	'f'		;
	btfss	enc,7		;
	movlw	'w'		;
	pagesel	putch
	call	putch		;
endopc
	pagesel	retadr
	goto	retadr		;
overld0
	pagesel	omnibus
	goto	omnibus		;
overld1
	pagesel	clr_reg
	goto	clr_reg		;
overld2
	pagesel	paging
	goto	paging		;
overld3
	pagesel	moviwwi
	goto	moviwwi		;
	
omnibus

clr_reg
	
paging
	
moviwwi
;;; if we arrive from omnibus, must have enc already swapf'ed
;;; FIXME: use some branching, don't need to always use carry flag to select < 3
	clrw			;
	btfsc	enc,7		;
	movlw	opc_mwi-opc_miw	;
	addlw	low opc_miw	;
	movwf	FSR0L		;
	movlw	high opc_miw	;
	movwf	FSR0H		;
	clrw			;
	addwfc	FSR0H,f		;
	pagesel	print
	call	print		;
	
	btfsc	1+enc,0		;
	bra	movoffs		;
	btfsc	enc,5		;
	bra	postinc
	
	

	pagesel	retadr
	goto	retadr		;
	
movoffs
	movlw	'0'		;
	pagesel	putch
	call	putch		;
	movlw	'x'		;
	pagesel	putch
	call	putch		;
	movlw	0x3f		;
	andwf	enc,w		;
	btfsc	enc,5		;
	bsf	enc,6		;
	btfsc	enc,5		;
	bsf	enc,7		;
	movwf	zOS_AR0		;
	clrw			;
	pagesel	putch
	call	putch		;
	zOS_ADR	low offset0	;
	movlw	0		;
	btfsc	enc,6		;
	movlw	offset1-offset0	;
	addwf	FSR0L		;
	movlw	0		;
	addwfc	FSR0H		;
	pagesel	print
	call	print		;
	pagesel	retadr
	goto	retadr		;
	
offset0
	da	"[FSR0]"
offset1
	da	"[FSR1]"
minfsr	
	da	"--FSR"
minmin
	da	"--"
plufsr
	da	"++FSR"
pluplu
	da	"++"
opc_miw	
	da	"moviw  "
opc_mwi
	da	"movwi  "
opc_lit
	da	"addlw 0"
	da	"sublw 0"
	da	"xorlw 0"
	da	"andlw 0"
	da	"iorlw 0"
	da	"retlw 0"
	da	"movlw 0"
opc_reg
	da	"addwfc "
	da	"subwfb "
	da	"asrf   "
	da	"lsrf   "
	da	"lslf   "
	da	"incfsz "
	da	"swapf  "
	da	"rlf    "
	da	"rrf    "
	da	"decfsz "
	da	"incf   "
	da	"comf   "
	da	"movf   "
	da	"addwf  "
	da	"xorwf  "
	da	"andwf  "
	da	"iorwf  "
	da	"decf   "
	da	"subwf  "
opccall
	da	"call 0x"
opcgoto
	da	"goto 0x"
regnam0
	da	"INDF0"
regnam1
	da	"INDF1"
regnam2
	da	"PCL"
regnam3
	da	"STATUS"
regnam4
	da	"FSR0L"
regnam5
	da	"FSR0H"
regnam6
	da	"FSR1L"
regnam7
	da	"FSR1H"
regnam8
	da	"BSR"
regnam9
	da	"WREG"
regnamA
	da	"PCLATH"
regnamB
	da	"INTCON"
	endm


zOS_NAM	macro	str
	local	start
start
	dt	str
	dt	0
	dt	start-$
	endm

zOS_MAN	macro	p,rat,rts,hb,pin,isr ;inline void zOS_MAN(int8_t p, int8_t rat,
	pagesel	endman
	goto	endman		;                      int8_t* hb, int8_t pin) {
	
	local	mantask,manisr,manchr,manchr0,reenable,manchr1,manchr2,manchr3
	local	manchr4,manchr5,manchr6,manchr7,manchr8,manchr9,mannone,jobinfo
	local	manname,manloop,crlf,stkinfo,stkloop,endman

	local	p0,p1,wrap,t0scale,isradrl,isradrh,tskadrl,tskadrh,optadrl
	local	optadrh,accumul,accumuh,numbase,destreg,destreh,char_io,buf,max

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
max	set	0x70

;copy the preceding lines rather than including this file, as definitions for
;zOS_MON()-derived macros referring to these local variables wouldn't open it
;until expansion and would throw an undefined-var error during the processing

	
mantask
#if 0;seems unnec 18 Jan
	movf	zOS_JOB,w	;int8_t mantask(void) {//destreg,accumul,char_io
	movwf	BSR		; bsr = zos_job; // to access char_io
#endif
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
	andlw	0x07		;   goto caseJ; // success, prints in job list
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
	andlw	0x07		;   goto caseJ; // FIXME: table, from match down
	movwf	accumul		;
	btfsc	STATUS,Z	;  else
	clrf	char_io		;   break;
	
manchr0
	movf	char_io,w	;
	xorlw	'I'		;
	btfss	STATUS,Z	; caseI:
	bra	manchr1		; case 'I': // send a software Interrupt > 7
	clrf	char_io		;  char_io = 0; // with destreg zOS_AR1:zOS_AR0
	
	movf	destreg,w	;  zOS_ARG(0, destreg);
	clrf	destreg		;
	zOS_ARG	0
	movf	1+destreg,w	;  zOS_ARG(1, destreh);
	clrf	1+destreg	;
	zOS_ARG	1
	movf	accumul,w	;  w = accumul;
	zOS_ACC	accumul,numbase
	andlw	0xf8		;  zOS_ACC(&accumul, &numbase); // reset
	btfsc	STATUS,Z	;  if (w & 0xf8) {
	bra	reenabl		;   int w = zOS_SWI(accumul); // disable again
	movlp	0		;   INTCON &= ~(1<<GIE);// for zOS_AR and _BUF()
	call	0x02		;   zOS_ARG(1, w);
	zOS_ARG	0		;   zOS_ARG(0, 0);
#if 0
	clrf	zOS_AR1		;   zOS_BUF(zos_job, p0); // print hex SWI result
	xorwf	zOS_AR1,f	;   zOS_ENA();
	xorwf	zOS_AR0,f	;   goto caseJ;
	zOS_BUF	FSR0,max,p0
#else
	zOS_ARG	1		;
	xorwf	zOS_AR0,f	;
	zOS_SWI	0xff
	movlw	'\r'		;
	zOS_ARG	0		;
	zOS_SWI	0xff		;
	movlw	'\n'		;
	zOS_ARG	0		;
	zOS_SWI	0xff		;
#endif
	
reenabl
	zOS_ENA

manchr1
	movf	char_io,w	; }
	xorlw	'J'		;
	btfss	STATUS,Z	; caseJ:
	bra	manchr2		; case 'J': // List struct for all running jobs

	decf	accumul,w	; // keep char_io='J' until last job line prints
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
	andlw	0x07		;   zOS_ARG(0,w); zOS_SWI(zOS_RST);
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
	movwf	FSR0H		;
	clrw			;
	zOS_ARG	0
	zOS_ARG	1
	zOS_ARG	2
	zOS_ARG	3
	zOS_SWI	zOS_NEW
	zOS_ARG	0
;	zOS_BUF	FSR0,max,p0
;	movlw	'J'		;
;	movwf	char_io		;

;	movf	accumul,w	;  if (accumul == 0)
;	btfsc	STATUS,Z	;   return 0;
;	return			;  zOS_ARG(0, accumul);
	clrw			;
	zOS_ARG	0
	zOS_ACC	accumul,numbase
	movlw	'J'		;  zOS_ACC(&accumul, &numbase);
	movwf	char_io		;  if ((w = zOS_SWI(zOS_SLP)) != 0) {
;	zOS_SWI	zOS_SLP
;	andlw	0xff		;   accumul = w;
;	movwf	accumul		;   goto caseJ;
;	btfsc	STATUS,Z	;  } else
;	clrf	char_io		;   break;

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

	swapf	accumul,w	;  if (accumul == 0x5a /*e.g.*/)
	xorwf	accumul,w	;
	addlw	1		;
	btfsc	STATUS,Z	;
	reset			;   reset();
	
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
	clrf	char_io		;   break;
	
manchr8
	movf	char_io,w	;  }
	xorlw	'S'		;
	btfss	STATUS,Z	;
	bra	manchr9		; case 'S': // Stack dump is actually scratch
	clrf	char_io		;  char_io = 0; // always succeeds, no arg

	decf	accumul,w	; // keep char_io='S' until last job line prints
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
	
	bsf	WDTCON,SWDTEN	;  if (w = accumul<<1) { // WDT prescalre
	lslf	accumul,w	;   w |= 1<<SWDTEN; // enable the wakeup
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
	return			;} // stkinfo()
	
	;guaranteed to arrive with p0=p1, interrupts off and in the correct bank
jobinfo
	movf	wrap,w		;int8_t jobinfo(void) {
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
	btfss	STATUS,Z	;
	movlw	'*'		; p1 += sprintf(p1, "%c", (zOS_HDH[fsr0] &
	movwi	FSR1++		;                    (1<<zOS_PRB)) ? '*' : ':');
	
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
	btfss	STATUS,Z	;
	movlw	'z'		; p1 += sprintf(p1, "%c", (zOS_PCH[fsr0] &
	movwi	FSR1++		;                    (1<<zOS_WAI)) ? 'z' : ':');
	
	zOS_IHF	zOS_PCH,FSR0,FSR1
	moviw	zOS_PCH[FSR0]	; // drop out after PCH if 0 (job is deleted)
	btfsc	STATUS,Z	; p1 += sprintf(p1, "%02X", zOS_PCH[fsr0]);
	bra	manname		; if (zOS_PCH[fsr0] & 0xff00) {
	zOS_IHF	zOS_PCL,FSR0,FSR1
	movlw	' '		;  // print the low byte of program counter
	movwi	FSR1++		;  p1 += sprintf(p1, "%02X", zOS_PCL[fsr0]); 
	moviw	zOS_ISH[FSR0]	;
	btfsc	STATUS,Z	;  // drop out after PCL if no interrupt routine
	bra	manname		;  if (zOS_ISH[fsr0] & 0xff00) {
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
	zOS_IHF	zOS_HIM,FSR0,FSR1
	movlw	's'		;
	movwi	FSR1++		;  // print (hw HwIMask sw SwIMask) scrunched up
	zOS_IHF	zOS_SIM,FSR0,FSR1
	movlw	')'		;   p1 += sprintf(p1, "(h%02Xs%02X) ",
	movwi	FSR1++		;                 zOS_HIM[fsr0], zOS_SIM[fsr0]);
manname
	movlw	' '		;
	movwi	FSR1++		;
	movlw	0x22 ;'"'	;
	movwi	FSR1++		;
	moviw	zOS_PCH[FSR0]	;
	btfss	STATUS,Z	;
	bra	manlive		;   if (zOS_PCH[fsr0] == 0) {
	movlw 	low mandead	;    static char mandead = "<not running>";
	movwf	FSR0L		;
	movlw	high mandead	;
	movwf	FSR0H		;    fsr0 = mandead;
	movlw	mandead-manlive	;
	movwf	char_io		;    char_io = strlen(mandead);
	bra	manloop		;
mandead
	zOS_NAM	"<not running>"
manlive
	moviw	zOS_HDL[FSR0]	;   } else {
	movwf	char_io		;
	moviw	zOS_HDH[FSR0]	;
	iorlw	0x80		;
	movwf	FSR0H		;    fsr0 = 0x8000 | (zOS_HDH[fsr0] << 8) ;
	movf	char_io,w	;
	movwf	FSR0L		;    fsr0 |= zOS_HDL[fsr0];
	moviw	--FSR0		;
	iorlw	0xe0		;
	movwf	char_io		;    char_io = 0xe0 | *--fsr0; // max 32? chars
#if 1
	addwf	FSR0L,f		;
	btfss	STATUS,C	;
	decf	FSR0H,f		;   for (fsr0 -= char_io; ++char_io; fsr1++) {
#else	
	local	manbit0,manbit1
	movf	FSR0L,w		;
	addwf	char_io,w	;
	btfss	WREG,7		;
	bra	manbit0		;
	btfss	FSR0L,7		;
	decf	FSR0H,f		;
	bra	manbit1		;
manbit0
	btfsc	FSR0L,7		;
	decf	FSR0H,f		;
manbit1
	movwf	FSR0L		;   for (fsr0 -= char_io; ++char_io; fsr1++) {
#endif
manloop
	moviw	FSR0++		;    char w = *fsr0++ ;
	btfsc	WREG,7		;
	bra	crlf		;    if ((w > '\0177') ||
	addlw	0-0x20		;
	btfsc	WREG,7		;
	bra	crlf		;        (w < ' '))
	addlw	0x20		;     break;
	movwi	FSR1++		;    *fsr1 = w; // added to buffer
	incfsz	char_io,f	;
	bra	manloop		;   }
crlf	
	movlw	0x22 ;'"'	;
	movwi	FSR1++		;
	movlw	'\r'		;  }
	movwi	FSR1++		; }
	movlw	'\n'		; // print a second \r\n, double-spacing table  
	movwi	FSR1++		; p1 += sprintf(p1, "\r\n");
	
	movlw	'J'		;
	movwf	char_io		;
	movf	FSR1L,w		;
	movwf	p1		; w = accumul--; // return with w as nonzero job
	movf	accumul,w	; if (accumul == 0)
	decf	accumul,f	;  char_io = 0;// final row in table was printed
	btfsc	STATUS,Z	; zOS_ENA(); // interrupts back ON!
	clrf	char_io		; return w;
	zOS_ENA
	return			;
endman
	local	vars,manl,manh
vars	set	0x20
manl	set	optadrl-vars
manh	set	optadrh-vars
	
	zOS_MON	p,rat,rts,hb,pin,isr
	movlw	low mantask	; zOS_MON(p,ra,rt,h,pi,manisr); //fsr0=swi,1=adr
	movwi	manl[FSR1]	; optadrl = mantask & 0x00ff;
	movlw	high mantask	; optadrh = mantask >> 8;
	movwi	manh[FSR1]	;} // zOS_MAN()
	endm

;;; zOS_CLC is an extension of the zOS_MAN() job manager shell into an rpn calc-
;;; ulator, as an example of how to use and customize the above console macros
;;;
;;; Note: because the max call depth of zOS_MON's ISR is nonzero (1), the max
;;; call depth for jobs in a system invoking these macros is reduced from 3 to 2
;;; 
;;; (job 0)
;;; zOS_CLC is invoked with an optional isr routine (for any custom extensions):
;;;  First a jump over the clcisr code ends the macro expansion
;;;  zOS_MAN is invoked with all the zOS_CON arguments and its clcisr address:
;;;   zOS_MON is invoked with all the zOS_CON arguments (and the clcisr address)
;;;    First a jump over zOS_MON's monisr and all its support functions (no task)
;;;    zOS_INP is invoked with all the zOS_CON arguments (and monisr's address)
;;;     Immediately a near branch to rxdecl over the rxtask and rxisr code:
;;;     When run, rxtask first calls any code at nonzero optadrh:optadrl address
;;;     then jumps to the mandatorily nonzero tskadrh:tskadrl task of zOS_CON
;;;     When handling an interrupt, rxisr either handles a received character or
;;;     jumps to the mandatorily nonzero isradrh:isradrl isr address of zOS_CON
;;;     and if a received character the ISR in this case jumps to nonzero monisr
;;; 	Unlike most declarations, rxdecl not only declares but launches, tweaks:
;;;     zOS_CON is invoked with the port,rate,rtsflag,heartbeat,pin arguments:
;;;      Immediately a near branch to decl over the task and isr code:
;;;      When run, task initializes the global pair, circular buffer and greets
;;;      (if the pair was still zero) then cedes the core awaiting a character
;;;      which it then sends and loops back (to the zOS_INP task, not its own!)
;;;      When handling an interrupt, isr handles the heartbeat and Timer0 stuff
;;;      (if hardware) else assumes that a software interrupt is a char to send
;;;      since any other applicable situation was handled by rxisr pre-jump
;;;     end of zOS_CON expansion
;;;     zOS_LAU then immediately assigns a job bank to the zOS_CON instance and
;;;     uses FSR1 to set locals isradrh:isradrl,tskadrh:tskadrl,optadrh:optadrl
;;;     to values zOS_CON just put in zOS_ARG1:zOS_ARG0, FSR0 (left at latter)
;;;     at which point it overwrites the Program Counter and HanDle fields with
;;;     rxtask, ISR field with rxisr and RX HWI mask using FSR0 (left at SWI)
;;;    end of zOS_INP expansion
;;;   FSR1 (pointing to optadrh:optadrl) then gets the address of the ensuing
;;;   mantask code (no ISR) which is then jumped over
;;;   end of zOS_MON expansion
;;;  end of zOS_MAN expansion
;;; end of zOS_CLC expansion
;;; (job 0)
;;; Since the end of zOS_INP, FSR0 has been pointing to the job information byte
;;; for the SWI mask that the job is to listen on for characters to output, so
;;; movwi 0[FSR0] with w set to the appropriate value: 8, 16, 32, 64 or 128

zOS_CLC	macro	p,ra,rt,h,pi,isr;inline void zOS_CLC(int8_t p, int8_t ra, int8_t
	local	endclc,clcisr,clcprmp,endclc

	pagesel	endclc
	goto	endclc		;      rt, int8_t* h, int8_t pi, void(*isr)()) {
	
	local	p0,p1,wrap,t0scale,isradrl,isradrh,tskadrl,tskadrh,optadrl
	local	optadrh,accumul,accumuh,numbase,destreg,destreh,char_io,buf,max

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
max	set	0x70

;copy the preceding lines rather than including this file, as definitions for
;zOS_MON()-derived macros referring to these local variables wouldn't open it
;until expansion and would throw an undefined-var error during the processing

	local	clctbl;,clcsize	; throws "Duplicate label or redefining symbol"
clcisr
	movf	zOS_AR0,w	; switch (char_io = zOS_AR0) {
	zOS_T63			;
clctbl
	retlw	' '
	retlw   '!'
	retlw	0x22
	retlw	'#'
	retlw	'$'
	retlw	'%'
	retlw	'&'
	retlw	"'"
	retlw	'('
	retlw	')'
	retlw	'*';0 ;zos_mac() not defined for '*'
	retlw	'+'
	retlw	','
	retlw	'-'
	retlw	'.'
	retlw	'/';0 ;zos_div() not defined for '/'
	retlw	'0'
	retlw	'1'
	retlw	'2'
	retlw	'3'
	retlw	'4'
	retlw	'5'
	retlw	'6'
	retlw	'7'
	retlw	'8'
	retlw	'9'
	retlw	':'
	retlw	0x3b
	retlw	'<'
	retlw	'='
	retlw	'>'
	retlw	'?'
	retlw	'@'
	retlw	'A'
	retlw	'B'
	retlw	'C'
	retlw	'D'
	retlw	'E'
	retlw	'F'
	retlw	'G'
	retlw	'H'
	retlw	'I'
	retlw	'J'
	retlw	'K'
	retlw	'L'
	retlw	'M'
	retlw	'N'
	retlw	'O'
	retlw	'P'
	retlw	'Q'
	retlw	'R'
	retlw	'S'
	retlw	'T'
	retlw	'U'
	retlw	'V'
	retlw	'W'
	retlw	'X'
	retlw	'Y'
	retlw	'Z'
	retlw	'[' ; '{'	;
	retlw	'\\' ; '|'	;
	retlw	']' ; '}'	;
	retlw	'^' ; '~'	;
clcsize	equ	$-clctbl
	if clcsize-0x3f
	 error "bad size: ASCII translation table expected to span 0x20 to 0x5e"
	endif
	movwf	char_io		;
	xorlw	'+'		;
	btfss	STATUS,Z	;
	bra	clcchr2		; case '+': // 16-bit signed/unsigned add

	movf	accumul,w	;
	addwf	destreg,f	;
	movf	accumuh,w	;
	addwfc	1+destreg,f	;  destreg += (accumuh << 8) | accumul;
	bra	clcprmp		; break;

clcchr2
	movf	char_io,w	;
	xorlw	'-'		;
	btfss	STATUS,Z	;
	bra	clcchr3		; case '-': // 16-bit signed/unsigned subtract

	movf	accumul,w	;
	subwf	destreg,f	;
	movf	accumuh,w	;
	subwfb	1+destreg,f	;  destreg -= (accumuh << 8) | accumul;
	bra	clcprmp		; break;
	
clcchr3
	movf	char_io,w	;
	xorlw	'*'		;
	btfss	STATUS,Z	;
	bra	clcchr4		; case '*': // 8-bit by 8-bit unsigned multiply
#ifdef zos_mac
	clrf	zOS_AR0		;  // invoker of macro must implement zos_mac():
	clrf	zOS_AR1		;  //  input arg zOS_AR1:zOS_AR0 (accumulator)
	movf	accumul,w	;  //                    zOS_AR2 (factor 1)
	movwf	zOS_AR2		;  //                    zOS_AR3 (factor 2)
	movf	destreg,w	;  //  output arg zOS_AR1:zOS_AR0 (product)
	movwf	zOS_AR3		;  zOS_AR0 = (uint16_t) 0;
				;  zOS_AR2 = accumul & 0x00ff;
	zOS_LOC	FSR0,zOS_JOB,char_io
	pagesel	zos_mac
	call	zos_mac		;  zOS_AR3 = destreg & 0x00ff;
	movf	zOS_AR0,w	;  fsr0 = &char_io; // temp register (as INDF0)
	movwf	destreg		;  zos_mac(&zOS_AR0 /* += */,
	movf	zOS_AR1,w	;          &zOS_AR2 /* * */, &zOS_AR3, fsr0);
	movwf	1+destreg	;  destreg = (uint16_t) zOS_AR0;
#endif
	bra	clcprmp		; break;

clcchr4
	movf	char_io,w	;
	xorlw	'/'		;
	btfss	STATUS,Z	;
	bra	clcchr5		; case '/': // 15-bit by 8-bit unsigned divide
#ifdef zos_div
	movf	destreg,w	;  // invoker of macro must implement zos_div():
	movwf	zOS_AR0		;  //  input arg zOS_AR1:zOS_AR0 (dividend)
	movf	1+destreg,w	;  //                    zOS_AR2 (divisor)
	andlw	0x7f		;  //  output arg zOS_AR1:zOS_AR0 (quotient/exc)
	movwf	zOS_AR1		;  zOS_AR0 = (uint16_t) destreg & 0x7fff;
	movf	accumul,w	;  zOS_AR2 = accumul & 0xff;
	movwf	zOS_AR2		;  fsr0 = &char_io; // temp register (as INDF0)  
	zOS_LOC	FSR0,zOS_JOB,char_io
	pagesel	zos_div
	call	zos_div		;  zos_div(&zOS_AR0 /* /= */
	movf	zOS_AR0,w	;          &zOS_AR2, &zOS_AR3/*scratch*/, fsr0);
	movwf	destreg		;
	movf	zOS_AR1,w	;
	movwf	1+destreg	;  destreg = (uint16_t) zOS_AR0;
#endif
	bra	clcprmp		; break;

clcchr5
	movf	char_io,w	;
	xorlw	'^'		;
	btfss	STATUS,Z	;
	bra	clcchr6		; case '^': // 8-bit by 8-bit exponentiation
#ifdef zos_mac
	movlw	0x01		;  // invoker of macro must implement zos_mac():
	clrf	zOS_AR1		;  //  input arg zOS_AR1:zOS_AR0 (accumulator)
	movf	accumul,f	;  //                    zOS_AR2 (factor 1)
	btfsc	STATUS,Z	;  //                    zOS_AR3 (factor 2)
	bra	clcexp1		;  //  output arg zOS_AR1:zOS_AR0 (product)
clcexp0
	clrf	zOS_AR0		;  zOS_AR1 = 0;
	clrf	zOS_AR1		;  for (uint8_t w = 1; accumul > 0; accumul--) {
	movwf	zOS_AR2		;   zOS_AR0 = (uint16_t) 0;
	movf	destreg,w	;   zOS_AR2 = w;
	movwf	zOS_AR3		;   zOS_AR3 = destreg & 0x00ff;
	zOS_LOC	FSR0,zOS_JOB,char_io
	pagesel	zos_mac
	call	zos_mac		;   fsr0 = &char_io; // temp register (as INDF0)
	movf	zOS_AR0,w	;   zos_mac(&zOS_AR0 /* += */,           
	decfsz	accumul,f	;           &zOS_AR2 /* * */, &zOS_AR3, fsr0);
	bra	clcexp0		;   w = zOS_AR0;
clcexp1
	movwf	destreg		;  }
	clrf	1+destreg	;  destreg = ((uint16_t) zOS_AR1) << 8) | w;
#endif
	bra	clcprmp		; break;

clcchr6
	movf	char_io,w	;
	xorlw	'!'		;
	btfss	STATUS,Z	;
	bra	clcchr7		; case '!': // 3-bit factorial
#ifdef zos_mac
	movlw	0x01		;  // invoker of macro must implement zos_mac():
	clrf	zOS_AR1		;  //  input arg zOS_AR1:zOS_AR0 (accumulator)
	movf	accumul,f	;  //                    zOS_AR2 (factor 1)
	btfsc	STATUS,Z	;  //                    zOS_AR3 (factor 2)
	bra	clcexp1		;  //  output arg zOS_AR1:zOS_AR0 (product)
	decfsz	accumul,f	;
	bra	clcexp1		;
clcfac0
	clrf	zOS_AR0		;  zOS_AR1 = 0;
	clrf	zOS_AR1		;  for (uint8_t w = 1; accumul-- > 1; accumul--) {
	movwf	zOS_AR2		;   zOS_AR0 = (uint16_t) 0;
	movf	destreg,w	;   zOS_AR2 = w;
	decf	destreg,f	;   zOS_AR3 = destreg-- & 0x00ff;
	movwf	zOS_AR3		;   fsr0 = &char_io; // temp register (as INDF0)
	zOS_LOC	FSR0,zOS_JOB,char_io
	pagesel	zos_mac
	call	zos_mac		;   zos_mac(&zOS_AR0 /* += */,
	movf	zOS_AR0,w	;           &zOS_AR2 /* * */, &zOS_AR3, fsr0);
	decfsz	accumul,f	;   w = zOS_AR0;
	bra	clcexp0		;  }
clcfac1
	movwf	destreg		;  destreg = ((uint16_t) zOS_AR1) << 8) | w;
	clrf	1+destreg	;  // 1 <= destreg <= 720
#endif
	bra	clcprmp		; break;
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
	movlw	'\r'		;
	pagesel	monbufs
	call	monbufs		;
	movlw	'\n'		;
	pagesel	monbufs
	call	monbufs		;clcprmp:
	movf	1+destreg,w	; moncrlf(zos_job, p0);
	movwf	accumuh		; accumuh = destreg>>8; monhex(zos_job, p0);
	pagesel	monhex
	call	monhex		; accumuh = destreg & 0xff; monlsb(zos_job, p0);
	movf	destreg,w	; moncrlf(zos_job, p0);
	movwf	accumuh		;clclast: 
	pagesel	monlsb
	call	monlsb		; zOS_ACC(&accumul,&numbase); zOS_RFI();
	movlw	'\r'		;
	pagesel	monbufs
	call	monbufs		;
	movlw	'\n'		;
	pagesel	monbufs
	call	monbufs		; char_io = 0;
	zOS_ACC	accumul,numbase
clclast
	clrf	char_io		;} // zOS_CLC()
	zOS_RFI
endclc
	zOS_MAN	p,ra,rt,h,pi,clcisr
	endm


zOS_T63	macro
	local	chrtran
	addlw	0-0x1f		;#define zOS_T63(w) \
	btfsc	WREG,7		;\
	clrw			;\
	andlw	0x3f		;\
	pagesel	chrtran		;\
	call	chrtran		; w = table[(w >= ' ') ? (w & 0x3f) : 0];\
	bra	$+0x42		; /*must be followed by 63-char retlw string:*/\
chrtran
	brw			; static char table[64] = "\0\
	retlw	0		;/* zOS_T63() */ 
	endm

