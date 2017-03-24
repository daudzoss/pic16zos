;;; zosmacro.inc
;;; potentially useful (but not mandatory) macros for zOS
;;;
;;; memory footprint:
;;; no memory words used upon inclusion (before expansion of a macro)
;;;  14-bit words if only zOS_CON() job is started to accept console output
;;;  words if full-featured monitor and job manager shell zOS_MON() instead
	
#define zOS_ME BSR,w		; // "movf/andwf/xorwf zOS_ME" can't clobber BSR

zOS_GLO	macro	fsrnum,job
	local	fsrn
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif
	movlw	0x3f		;// documentation suggests 5 but BSR now 6 bits!
	andwf	FSR#v(fsrn)H,f	;inline int8_t zOS_GLO(int8_t* *fsrnum,
	if (job)
	 rlf	job,w		;                      int8_t* job) {
	else
	 rlf	zOS_ME		; int8_t w = 0x70 | ((job ? *job : bsr) << 1);
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
	movwf	FSR#v(fsrn)H	; return (*fsrnum = (job<<7) | offset) >> 8;
	rrf	FSR#v(fsrn)L,f	;}
	endm

zOS_ADR	macro	adr,msb
	 movlw	low adr		;inline void zOS_ADR(void* a) {
	 movwf	FSR0L		; if (msb) fsr0 = 0x8000 | a;
	 movlw	high adr	; else fsr0 = 0x7fff & a;
	 movwf	FSR0H		;}
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
	movwf	FSR0H		;}
	zOS_ARG	0
	zOS_ARG	1
	zOS_ARG	2
	zOS_ARG	3
	endif
	endm

zOS_SWI	macro	type		;inline void zOS_SWI(const int8_t type) {
	movlw	type		;
	movlp	0x00		; zos_swj(type);
	call	0x02		;}
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
	  if (zOS_T0E - INTCON)
	   bsf	INTCON,PEIE	; if (t0enable) { *t0enable |= 1<<T0IE;
	 endif
	endif
	;; advance the stack pointer to allow 5 stacks of 3 each (+1 if running)
	banksel	STKPTR		;  if (t0enable !=  INTCON) INTCON |= 1<<PEIE;
	movlw	zOS_BOS		; }
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
	bra	loop		; STKPTR = -1;
	decf	STKPTR,f	; // still in job "0"
	movlb	0		;}
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
	bsf	INTCON,GIE	;}
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

zOS_ASC	macro	file,str,nulterm;inline uint8_t zOS_ASC(char*file, const char*
	local	start,out
	bra	out		;                       str, int nulterm) {
start
	dt	str
	if nulterm
	 dt	0
	endif
out
	variable i,sum,prev,curr
;prev = 0xff
i = 0
	while i < (out-start)	; for (int i=0; str[i]||(nulterm&&!str[i]); i++)
;sum = i+start
;curr = high sum
;	if curr-prev	
	 pagesel sum
;	endif
	call	sum		;  file[i] = str[i];
	movwf	file+i		; return i;
;prev = curr
i += 1
	endw
	movlw	i+nulterm	;}
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
	endm			;}
#else
zOS_OUT	macro	swinum,revstr,temp; // 1 word per byte (+) to use stack+file
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
	callw			;  zOS_SWI(swinum);
	zOS_ARG	0
	zOS_SWI	swinum
	bra	loop		; } // zOS_ARG 0 is 2, zOS_SWI 3 words, bra 1
pre
	dt	revstr	;}
post
#endif

zOS_PSH	macro	reg
	movf	zOS_ME		;inline void zOS_PSH(uint8_t* reg) {
	;; bcf	INTCON,GIE
	banksel	TOSH
	movwf	TOSH		; TOSH = bsr;// must store bsr so we can go back
	if (reg-BSR)
	 movf	reg,w		; if (reg != &bsr)
	 mowwf	TOSL		;  TOSL = *reg;
	 movf	TOSH,w		; STKPTR++;
	endif
	incf	STKPTR,f	; bsr = TOSH;
	movwf	BSR		;} // caller should have turned interrupts off
	;; bsf	INTCON,GIE
	endm

zOS_POP	macro	reg
	;; bcf	INTCON,GIE
	banksel	STKPTR
	decf	STKPTR,f	;inline void zOS_POP(uint8_t* reg) {
	if (reg-BSR)
	 movf	TOSL,w		; STKPTR--;
	 movwf	reg		; if (reg != &bsr) *reg = TOSL;
	endif
	movf	TOSH,w		; bsr = TOSH;
	movwf	BSR		;} // caller should have turned interrupts off
	;; bsf	INTCON,GIE
	endm
	
zOS_RDF macro
#ifdef EEADRL
zOS_ADL	equ	EEADRL
zOS_ADH	equ	EEADRH
zOS_RDL	equ	EEDATL
zOS_RDH	equ	EEDATH
	bcf	EECON1,CFGS	;inline void zOS_RDF(void) {
	bsf	EECON1,EEPGD	; EECON1 &= ~(1<<CFGS);
	bsf	EECON1,RD	; EECON1 |= 1<<EEPGD;
	nop			; EECON1 |= 1<<RD;
	nop			;}
#else
#ifdef PMADRL
zOS_ADL	equ	PMADRL
zOS_ADH	equ	PMADRH
zOS_RDL	equ	PMDATL
zOS_RDH	equ	PMDATH
	bcf	PMCON1,CFGS	;inline void zOS_RDF(void) {
	bsf	PMCON1,RD	; PMCON1 &= ~(1<<CFGS);
	nop			; PMCON1 |= 1<<RD;
	nop			;}
#else
#ifdef NVMADRL
zOS_ADL	equ	NVMADRL
zOS_ADH	equ	NVMADRH
zOS_RDL	equ	NVMDATL
zOS_RDH	equ	NVMDATH
	bcf	NVMCON1,NVMREGS	;inline void zOS_RDF(void) { // no nop's needed!
	bsf	NVMCON1,RD	; NVMCON1 &= ~(1<<CFGS); NVMCON1 |= 1<<RD; }
#endif
#endif
#endif
	endm
	
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
	endm			;}

zOS_CON	macro	p,rat,rts,hb,pin;inline void zOS_CON(int8_t p,int8_t rat,int8_t
	local	task,loop,decl,isr
	bra	decl		;                    rts,int8_t* hb,int8_t pin){
	
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
	
	local	uatbase,uatxmit,rtsflag
	if (p == 1)
uatbase	 set	TXREG & 0xff80
uatxmit	 set	TXREG & 0x007f
rtsflag	 set	TXIF
	else
uatbase	 set	TX#v(p)REG & 0xff80
uatxmit	 set	TX#v(p)REG
rtsflag	 set	TX#v(p)IF
	endif
task
	movlw	high uatbase	; goto decl;
	movwf	FSR0H		;
	zOS_MY2	FSR0		;task:
	movlw	0xff		; fsr0 = 0x70 + (bsr<<1);//global always visible
	movwi	t0div[FSR0]	; 0[fsr0] = 0xff;// live TMR0 postscaler divider
	movlw	0x00		;
	movwi	t0rst[FSR0]	; 1[fsr0] = 0x00; // live reset value for TMR0
	movlw	low uatbase	; const int8_t* uatbase = uatxmit & 0xff80;
	movwf	FSR0L		; fsr0 = uatbase;
	rrf	zOS_ME		;
	clrw			; const char* max = 0x70;
	rrf	WREG,w		; static char *p0, *p1, buf[]; //p0:task, p1:ISR
	iorlw	buf		;
	movwf	wrap		; const char* wrap = ((bsr&1)<<7) | buf;
	movwf	p0		;
	movwf	p1		; p0 = p1 = wrap; // reset value if they max out
	zOS_ASC	buf,"\r\nzOS>",1
	addwf	p1,f		;
	
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

	;; SWI will be coming from a job that wants to send a character
	;;  in which case the ISR stores it, advances p1 and clears zOS_WAI bit
	;;  and thus afer clearing zOS_WAI the job just re-sleeps if rtsflag==0
	;; 
	;; HWI will be coming from the UART peripheral indicating ready-to-send
	;;  in which case ISR clears the job's zOS_WAI bit to get scheduled
	;; (or HWI will come from a tmr0 expiration for the blinking heartbeat)
	;; 
	;; Note: caller needs to make sure to check status of return value for
	;; != 0, just in case job is in between sleeps or with a full buffer
isr
	local err,done,do_swi,ascii,nottmr
	
#if 0
	;; get fsr0 pointing to tmr0 postscaler/reset value
	movlw	high uatxmit	;
	movwf	FSR0H		;
	zOS_MY2	FSR0L		; fsr0 = 0x70 | (bsr << 1);

	;; if it's a simple and frequent timer overflow interrupt finish quickly
	banksel	zOS_T0F
	btfss	zOS_T0F,T0IF	; if (/*presumed true:(zOS_T0E & (1<<T0IE)) &&*/
	bra	nottmr		;     (zOS_T0F & (1<<T0IF))) { // timer overflow
	bcf	zOS_T0F,T0IF	;  zOS_T0F &= ~(1<<T0IF);// clear interrupt flag
#else
	;; if it's a simple and frequent timer overflow interrupt finish quickly
	banksel	zOS_T0F
	btfss	zOS_T0F,T0IF	; if (/*presumed true:(zOS_T0E & (1<<T0IE)) &&*/
	bra	nottmr		;     (zOS_T0F & (1<<T0IF))) { // timer overflow
	bcf	zOS_T0F,T0IF	;  zOS_T0F &= ~(1<<T0IF);// clear interrupt flag

	;; get fsr0 pointing to tmr0 postscaler/reset value
	movlw	high uatxmit	;
	movwf	FSR0H		;
	zOS_MY2	FSR0L		; fsr0 = 0x70 | (bsr << 1);
#endif

	;; with fsr0 pointing to global pair, point fsr1 to local mem("t0scale")
	zOS_LOC	FSR1,zOS_JOB,t0scale
	banksel	TMR0
	moviw	t0rst[FSR0]	;  fsr1 = (zOS_JOB << 7) | t0scale;// not bsr?!?
	btfss	STATUS,Z	;  bsr = TMR0 >> 7;//now invalid for this branch
	movwf	TMR0		;  if (t0rst[fsr0])
	decfsz	INDF1,f		;   TMR0 = t0rst[fsr0]; // TMR0 reset specified
	bra	done		;  if (--*fsr1 == 0) {

	banksel	hb
	movf	INDF0,w		;
	movwf	INDF1		;   *fsr1 /*countdown*/ = *fsr0 /*postscaler*/;
	movlw	(1<<pin)	;
	xorwf	hb,f		;   hb ^= 1 << pin;
	bra	done		; } else {
	
	;; make sure the task is sleeping (else the task() initialization unrun)
nottmr
	zOS_MEM	FSR0,zOS_JOB,0
	moviw	zOS_PCH[FSR0]	;isr: point fsr0 to job struct;
	btfss	WREG,zOS_WAI	; fsr0 = 0x10 * (1+zos_job);
	bra	err		; if (zOS_PCH[fsr0] & (1<<zOS_WAI) == 0)

	;; point fsr0 to uatbase again, point fsr1 to p0
	movlw	low uatbase	;
	movwf	FSR0L		;
	movlw	high uatbase	;  fsr0 = uatbase;
	movwf	FSR0H		;  fsr1 = (zOS_JOB << 7);
	zOS_LOC	FSR1,zOS_JOB,p0
	
	;; check for validated SWI first since it will be in zOS_MSK, else a HWI
	movf	zOS_MSK,w	;
	btfsc	STATUS,Z	;  if (zOS_MSK == 0) { // not a SWI so check HWI
	bra	do_swi		;   if (rts & (1<<rtsflag) == 0)
	banksel	rts
	btfsc	rts,rtsflag	;    zOS_RET(); // not the HWI we expected, SWI?
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
	movwf	zOS_MSK	    	;
	zOS_UNW	zOS_JOB		; err: w = 0;
	zOS_RFI	zOS_MSK		; done: zOS_UNW(zOS_JOB); zOS_RFI (w);}
	
	;; intialize the UART peripheral, job handle and first three arguments
decl
#if 0
	;; wait out inevitable garbage TX (not sure why this delay is necessary)
	clrf	0x20
	clrf	0x21
delay0
	incfsz	0x21,f
	bra	delay0
	incfsz	0x20,f
	bra	delay0
#endif
#if 0
	banksel	RCSTA
	bcf	RCSTA,SPEN	;decl:
	bcf	RCSTA,CREN	; RCSTA &= ~((1<<SPEN)|(1<<CREN));
#endif
	banksel	TXSTA
	bcf	TXSTA,TXEN	; TXSTA &= ~(1<<TXEN);
	local brgval,brgvalm,brgvalh,brgvall
#ifdef BRG16
brgval	set	rat>>2
brgvalm	set	brgval-1
brgvalh	set	high brgvalm
brgvall	set	low brgvalm
	banksel	BAUDCON
	bsf	BAUDCON,BRG16	; // section 26.1.2.8 of 16F1847 steps below:
	banksel	TXSTA
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
	banksel	SPBRG
	movlw	brgvall		;
	movwf	SPBRG		; SPBRG = (rat/16) - 1;
#endif
#if 0
	banksel	RCSTA
	bsf	RCSTA,SPEN	; // (3) "Enable..by setting..SPEN"
	bcf	RCSTA,RX9	; RCSTA &= ~(1<<RX9);  // (5) "9-bit..set..RX9"
	bsf	RCSTA,CREN	; RCSTA |= (1<<SPEN) | (1<<CREN); // (6) "CREN" 
#endif
	banksel	TXSTA
	bsf	TXSTA,TXEN	; TXSTA |= 1<<TXEN; // (5) "Enable..by..TXEN"
#if 0
	banksel	PIE1
	bsf	PIE1,RCIE	; PIE1 |= 1<<RCIE; //(4) "Set..RCIE..and..PEIE"
#endif
	zOS_ADR	task,zOS_UNP	; fsr0 = task & 0x7fff;// MSB 0 => unprivileged
	movlw	low isr		; w = zOS_ARG(0, isr & 0x00ff);
	zOS_ARG	0
	movlw	high isr	; w = zOS_ARG(1, isr>>8);
	zOS_ARG	1		; w = zOS_ARG(2, (1<<TXIF)|(1<<T0IE));
	movlw (1<<TXIF)|(1<<T0IE)
	zOS_ARG	2
	movlb	0		; // still in job "0"
	endm			;}

#if 0
;;; like zOS_CON, but also accepts console input for a full job-management shell
zOS_MON	macro	port,rat,hb,pin	;
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
