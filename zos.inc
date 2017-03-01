#if 1
processor 16f1847
include p16f1847.inc
#endif
	
;;; ZOS.INC
;;; a lightweight, small-footprint, preemptively multitasking RTOS for Microchip
;;;  Technology's entire enhanced midrange 8-bit PIC microcontroller family:
;;;
;;; memory footprint:
;;; _ 14-bit words for base RTOS i.e. main() starts at 0x0___
;;; 
;;; SRAM footprint:
;;; 86 bank-0 bytes claimed by RTOS, 30 bytes of stack scratch space relocatable
;;; 
;;; available bytes    possible jobs with    local bytes/job (+any heap, besides
;;;  on PIC device     80 bytes RAM each     2 global bytes) if ZOS_NUM set to 5
;;; ===============    ==================    ===================================
;;;      128                 0                          0 (+2)
;;;      256                 1				0 (+130)
;;;      384                 3                          0 (+258)
;;;      512                 4                          0 (+386)
;;;      768                 5                          80 (+2)
;;;  >=1,024                 5                          80 (+258...)

;;; you may redefine a constant ZOS_NUM with the maximum job number (<6,
;;; as determined by where the general purpose register memory stops, as
;;; the guaranteed 2 bytes global memory isn't sufficient for most jobs)
 ifdef (ZOS_NUM)
 else
ZOS_NUM	set	5
 endif
	
;;; you may redefine the location of the scratch space for restoring the stack
;;; after each context switch (by default it is 0x20 in bank ZOS_NUM+1, but must
;;; be pulled in on small devices or pushed out for a heap-management job space)
 ifdef (ZOS_STK)
ZOS_STK	set	(((ZOS_NUM+1)<<7)|0x20)
 endif

;;; software interrupt infrastructure the OS is based on (with interrupts off!)
;;; user-definable:
ZOS_SB7	equ	7
ZOS_SI7	equ	(1<<ZOS_SB7)
ZOS_SB6	equ	6
ZOS_SI6	equ	(1<<ZOS_SB6)
ZOS_SB5	equ	5
ZOS_SI5	equ	(1<<ZOS_SB5)
ZOS_SB4	equ	4
ZOS_SI4	equ	(1<<ZOS_SB4)
ZOS_SB3	equ	3
ZOS_SI3	equ	(1<<ZOS_SB3)
;;; system:
ZOS_FND	equ	0x07		; find a running job by its handle AR1:AR0
ZOS_EXE	equ	0x06		; replace this job with a new job (unpriv'ed)
ZOS_FRK	equ	0x05		; copy a running job into a new job
ZOS_YLD	equ	0x04		; (in)voluntarily cede processor before next irq
ZOS_RST	equ	0x03		; restart job at its start address (vs. END+NEW)
ZOS_END	equ	0x02		; job killed, slot# available for NEW
ZOS_SLP	equ	0x01		; indicate job waiting on its ISR, so don't run
ZOS_NEW	equ	0x00		; create a job (FSR0==addr,AR1:0==isr,AR3:2==IM)
;;; FND runs high->low, _END/_SLP/_RST can only be run by a _PRV job or *it* got

;;; job/shadow register offsets from ZOS_J0M, ZOS_J1M,...
ZOS_HDL	equ	0x00		; handle, the start address of the job
ZOS_HDH	equ	0x01		;
ZOS_PRB	equ	7		;  MSB of HDL indicates privilege(manage others)
ZOS_PRV	equ	(1<<ZOS_PRB)	;
ZOS_PCL	equ	0x02		; address to resume execution
ZOS_PCH	equ	0x03		;  "impossible" PCH 0x00==not runnable
ZOS_WAI	equ	7		;  MSB of PCH indicates sleeping (wait for int)
ZOS_SST	equ	0x04		; shadow STATUS
ZOS_SWR	equ	0x05		; shadow WREG
ZOS_SSP	equ	0x06		; STKPTR to be restored (BSR never restorable!)
ZOS_SPH	equ	0x07		; PCLATH to be restored
ZOS_SF0	equ	0x08		; shadow FSR0
ZOS_SF1	equ	0x0a		; shadow FSR1
ZOS_ISR	equ	0x0c		; interrupt service routine address for the job
ZOS_ISH	equ	0x0d		; interrupt service routine address for the job
ZOS_HIM	equ	0x0e		; mask for hardware interrupts to process (0=no)
ZOS_SIM	equ	0x0f		; mask for software interrupts (low 3 always==1)
	
;;; global memory space for message-passing boxes (or J6M?!?)
ZOS_JOB	equ	0x70		; next job to run (0 if unknown)
ZOS_MSK	equ	0x71		; masked-off sofware interrupt for ISR to handle
ZOS_J1L	equ	0x72		;  (repurposeable as scratch after ZOS_RFI call)
ZOS_J1H	equ	0x73
ZOS_J2L	equ	0x74
ZOS_J2H	equ	0x75
ZOS_J3L	equ	0x76
ZOS_J3H	equ	0x77
ZOS_J4L	equ	0x78
ZOS_J4H	equ	0x79
ZOS_J5L	equ	0x7a
ZOS_J5H	equ	0x7b
	;; must disable interrupts e.g. with ZOS_ARG(0) before writing SWI args:
ZOS_AR0	equ	0x7c
ZOS_AR1	equ	0x7d
ZOS_AR2	equ	0x7e
ZOS_AR3	equ	0x7f

ZOS_TOS	equ	0x0e  		; STKPTR for full stack (0x0f reserved for ISRs)
ZOS_BOS	equ	0x0b		; STKPTR for empty stack (first push is to 0x0c)
	
;;; bank 0 memory space for managing jobs, 1@0x20, 2@0x30, ... , 5@0x60
ZOS_J1M	equ	0x20
ZOS_J2M	equ	0x30
ZOS_J3M	equ	0x40
ZOS_J4M	equ	0x50	
ZOS_J5M	equ	0x60	
	
ZOS_MEM	macro	fsrnum,job,offset
	local	fsrn
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif
	swapf	job,w		;inline void ZOS_MEM(int8_t* *fsrnum,
	addlw	0x10		;                    const int8_t* job,
	andlw	0x70		;                    const
	if (offset)
	 addlw	offset		;                    int8_t offset) {
	endif
	movwf	FSR#v(fsrn)L	; *fsrnum = ((job & 0x07) << 4) + offset;
	clrf	FSR#v(fsrn)H	;}
	endm

;;; macro to wind the circular stack around from the running job# to the new job
;;; (before restoring the new job's STKPTR and copying its return address there)
;;; typically: ZOS_ROL BSR_SHAD,JOB_NUM(BSR?),ZOS_TMP,FSR0,ZOS_STK
ZOS_ROL	macro	old,new,temp,fsrnum,base
	local fsrn,loop1,loop2,done
	if (fsrn & 3)
fsrn set 1
	else
fsrn set 0
	endif
	banksel	STKPTR
	movlw	low base	;inline void ZOS_ROL(const int8_t* old,
	movwf	FSR#v(fsrn)L	;                    const int8_t* new,
	movlw	high base	;                    int8_t* temp,
	movwf	FSR#v(fsrn)H	;                    int16_t* *fsrnum,
	movf	new,w		;                    int8_t* base) {
	subwf	old,w		;
	btfsc	STATUS,Z	; if (new == old) // nothing to do
	bra	done		;  return;
	decf	WREG,w		; w = new - old - 1;
	btfsc	WREG,7		;
	addlw	5		; // set STKPTR to the current location of the
	movwf	STKPTR		; // stack cell that needs to be rotated into
	rlf	STKPTR,f	; // STK_TOP, then record this value in temp for
	bcf	STKPTR,0	; // comparison to know when to exit the loop
	addwf	STKPTR,w	; // that copies the entire stack (except 0x0f)
	addlw	2		; // into 30-byte scratch in the unrolled order
	movwf	STKPTR		;
	movwf	temp		; for (STKPTR = *temp = 2+3*((w<0) ? (w+5) : w);
loop1
	movf	TOSL,w		;      STKPTR != *temp + 1;
	movwf	FSR#v(fsrn)++	;      STKPTR = (STKPTR>0) ? (STKPTR-1):ZOS_TOS)
	movf	TOSH,w		;
	movwf	FSR#v(fsrn)++	;  *(*fsrnum)++ = (TOSH << 8) | TOSL;
	decf	STKPTR,f	;
	movlw	ZOS_TOS		;
	btfsc	STKPTR,4	;
	movwf	STKPTR		;
	movf	temp,w		;
	xorwf	STKPTR,w	;
	btfss	STATUS,Z	; // now rebuild the unrolled stack
	bra	loop1		;
	clrf	STKPTR		; for (STKPTR = 0;
loop2
	moviw	--FSR#v(fsrn)	;      STKPTR <= ZOS_TOS;
	movwf	TOSH		;      STKPTR++) {
	moviw	--FSR#v(fsrn)	;  TOSH = *(*fsrnum) >> 8;
	movwf	TOSL		;  TOSL = *--(*fsrnum) & 0x00ff;
	incf	STKPTR,w	; }
	movwf	STKPTR		;
	sublw	ZOS_TOS		;
	btfss	WREG,7		;
	bra	loop2		;}
	endm
;;; running job#: 1         2         3         4         5
;;; stack pos 15: 3rd(1)    3rd(2)    3rd(3)    3rd(4)    3rd(5)
;;; stack pos 14: 2nd(1)    2nd(2)    2nd(3)    2nd(4)    2nd(5)
;;; stack pos 13: 1st(1)    1st(2)    1st(3)    1st(4)    1st(5)
;;; stack pos 12: 0th(1)    0th(2)    0th(3)    0th(4)    0th(5)
;;; stack pos 11: 2nd(5)    2nd(1)    2nd(2)    2nd(3)    2nd(4)
;;; stack pos 10: 1st(5)    1st(1)    1st(2)    1st(3)    1st(4)
;;; stack pos 9:  0th(5)    0th(1)    0th(2)    0th(3)    0th(4)
;;; stack pos 8:  2nd(4)    2nd(5)    2nd(1)    2nd(2)    2nd(3)
;;; stack pos 7:  1st(4)    1st(5)    1st(1)    1st(2)    1st(3)
;;; stack pos 6:  0th(4)    0th(5)    0th(1)    0th(2)    0th(3)
;;; stack pos 5:  2nd(3)    2nd(4)    2nd(5)    2nd(1)    2nd(2)
;;; stack pos 4:  1st(3)    1st(4)    1st(5)    1st(1)    1st(2)
;;; stack pos 3:  0th(3)    0th(4)    0th(5)    0th(1)    0th(2)
;;; stack pos 2:  2nd(2)    2nd(3)    2nd(4)    2nd(5)    2nd(1)
;;; stack pos 1:  1st(2)    1st(3)    1st(4)    1st(5)    1st(1)
;;; stack pos 0:  0th(2)    0th(3)    0th(4)    0th(5)    0th(1)

;;; continue with next iteration of HWI-searching loop (mustn't clobber FSR0!)
;;; when searching for the correct hardware interrupt handler, without stack hit
ZOS_RET	macro
	pagesel	zos_nhw
	goto	zos_nhw		;#define ZOS_RET() goto zos_nhw
	endm

;;; at the end of any interupt handler, goes back to scheduler without stack hit
ZOS_RFI	macro	
	pagesel	zos_sch
	goto	zos_sch		;#define ZOS_RFI() goto zos_sch
	endm

;;; find something runnable (i.e. PCH != 0, but sleep MSB is OK), at job+/-1
;;; according to incr and branch to unf if job-1 == 0 or job+1 > ZOS_NUM,
;;; with fsrnum pointing to job's bank 0 structure and then incremented +/-16
ZOS_LIV	macro	fsrnum,job,incr,unf
	local	fsrn,loop
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif
loop
	if (incr)
	 movlw	0x10		;inline int8_t ZOS_LIV(int8_t* *fsrnum,
	else
	 movlw	0-0x10		;     int8_t *job, int8_t incr, void *(unf)()) {
	endif
	addwf	FSR#v(fsrn)L,f	; do {
	if (incr)
	 incf	job,f		;  *fsrnum += incr ? 0x10 : -0x10;// next struct
	 movlw	0xff-ZOS_NUM	;  job += incr ? 1 : -1; // next job#
	 addwf	job,w		;  if ((job == 0) || (job == ZOS_NUM+1)) {//past
	else
	 decf	job,f		;   if (unf) goto unf; else return random();
	endif
	btfss	STATUS,Z	;  } else if (ZOS_PCH[fsrnum]) // found runnable
	if (unf)
	 bra	unf		;   return w = ZOS_PCH[fsrnum];
	 moviw	ZOS_PCH[FSR#v(fsrn)]
	 btfss	STATUS,Z	; } while (1); // job is runnable (or unf was 0)
	endif
	bra	loop		;}
	endm
	
	ifdef (PIE0)
ZOS_PE0	 equ	PIE0
ZOS_PIE	 equ	PIE0-1
	else
ZOS_PE0	 equ	PIE1-1
ZOS_PIE	 equ	INTCON-1
	endif
	ifdef (PIE1)
ZOS_MAX	set	PIE1
	endif
	ifdef (PIE2)
ZOS_MAX	set	PIE2
	endif
	ifdef (PIE3)
ZOS_MAX	set	PIE3
	endif
	ifdef (PIE4)
ZOS_MAX	set	PIE4
	endif
	ifdef (PIE5)
ZOS_MAX	set	PIE5
	endif
	ifdef (PIE6)
ZOS_MAX	set	PIE6
	endif
	ifdef (PIE7)
ZOS_MAX	set	PIE7
	endif
	ifdef (PIE8)
ZOS_MAX	set	PIE8
	endif
	ifdef (PIE9)
ZOS_MAX	set	PIE9
	endif

	;; a job switch is attempted with every incoming interrupt
	;; user jobs are responsible for processing their own interrupts
	;; with an interrupt handler registered at the time of creation

	org	0x0000
	pagesel	zos_ini
	goto	zos_ini

	org	0x0002
	pagesel	zos_swj
	goto	zos_swj

	org	0x0004
	;; find first handler for an enabled and flagged interrupt
zos_004
	movlw	low ZOS_PIE	;void __isr zos_004(void) {
	movwf	FSR1L		; // TMR0,PEIE may be outside INTCON or not
	movlw	high ZOS_PIE	;
	movwf	FSR1H		; fsr1 = ZOS_PIE; // -1 + either INTCON or PEIE

		
	movlw	ZOS_NUM+1	;
	movwf	ZOS_JOB		; ZOS_JOB = ZOS_NUM+1;// search from high to low
	ZOS_MEM	FSR0,ZOS_JOB,0	; fsr0 = 0x10 * (1 + ZOS_JOB);
zos_nhw	
	ZOS_LIV	FSR0,ZOS_JOB,0,zos_sch
	moviw	ZOS_HIM[FSR0]	; while (ZOS_LIV(&fsr0, &ZOS_JOB, 0)) {
	movwf	ZOS_MSK		;
	moviw	++FSR1		;                  /*job#*/   /*PIE#*/
	andwf	ZOS_MSK,f	;  ZOS_MSK = ZOS_HIM[fsr0] & *++fsr1;
	movlw	high ZOS_PE0	;
	subwf	FSR1H,w		;
	btfsc	WREG,7		;
	bra	zos_cmp		;
	movlw	low ZOS_PE0	;
	subwf	FSR1L,w		;
	btfsc	WREG,7		;
	bra	zos_cmp		;
	movlw	low PIE1	;
	movwf	FSR1L		;
	movlw	high PIE1	;  if (fsr1 < ZOS_PE0+1)
	movwf	FSR1H		;   fsr1 = PIE1;//handle the INTCON+1->PIE1 jump
	
	;; enter handler which will ZOS_RFI() to zos_sch if it's the correct one
	;; (after clearing the interrupt flag) else ZOS_RET() back up to zos_nhw
zos_cmp
	movf	ZOS_MSK,w	;
	btfsc	STATUS,Z	;
	bra	zos_nhw		;  if (ZOS_MSK) {

	clrf	ZOS_MSK		;   ZOS_MSK = 0; // indicates HWI (not SWI) type 
	moviw	ZOS_ISH[FSR0]	;
	movwf	PCLATH		;   *(ZOS_ISR[fsr0])();
	moviw	ZOS_ISR[FSR0]	;  }
	movwf	PCL		; }
	
	;; scheduler
zos_sch
	banksel	BSR_SHAD
	movf	BSR_SHAD,w	; ZOS_JOB = BSR_SHAD;
	movwf	ZOS_JOB		; do { // might loop until an interrupt comes in
	ZOS_MEM	FSR0,ZOS_JOB,0
zos_itr
	ZOS_LIV	FSR0,ZOS_JOB,1,zos_wra
	clrwdt			;  // ZOS_LIV leaves PCH in WREG since zos_wra>0
	btfsc	WREG,ZOS_WAI	;  while (ZOS_LIV(fsr0,ZOS_JOB,1)&(1<<ZOS_WAI))
	bra	zos_itr		;   clrwdt();
	
	;; if this point is reached, a runnable job was found with job# ZOS_JOB
	;; (but we skip a whole bunch of trivial copies if ZOS_JOB==BSR_SHAD)
	movf	BSR_SHAD,w	;
	xorwf	ZOS_JOB,w	;
	btfsc	STATUS,Z	;
	bra	zos_don		;  if (ZOS_JOB != BSR_SHAD) {

	;; copy the interrupted job's (BSR_SHAD) criticals into its bank 0 slot
zos_sav
	ZOS_MEM	FSR0,BSR_SHAD,ZOS_PCL
	movf	TOSL,w		;   fsr0 = 0x10 * (1+BSR_SHAD) + ZOS_PCL;
	movwi	FSR0++		;   *fsr0++ = TOSL;
	movf	TOSH,w		;
	movwi	FSR0++		;   *fsr0++ = TOSH;
	
	movf	STATUS_SHAD,w	;
	movwi	FSR0++		;   *fsr0++ = STATUS_SHAD;
	movf	WREG_SHAD,w	;
	movwi	FSR0++		;   *fsr0++ = WREG_SHAD;
	movf	STKPTR,w	;
	movwi	FSR0++		;   *fsr0++ = STKPTR; // not BSR_SHAD
	movf	PCLATH_SHAD,w	;
	movwi	FSR0++		;   *fsr0++ = PCLATH_SHAD;
	movf	FSR0L_SHAD,w	;
	movwi	FSR0++		;   *fsr0++ = FSR0L_SHAD;
	movf	FSR0H_SHAD,w	;
	movwi	FSR0++		;   *fsr0++ = FSR0H_SHAD;
	movf	FSR1L_SHAD,w	;
	movwi	FSR0++		;   *fsr0++ = FSR1L_SHAD;
	movf	FSR1H_SHAD,w	;
	movwi	FSR0++		;   *fsr0++ = FSR1H_SHAD;

	;; copy ZOS_JOB's criticals out of its bank 0 slot
	ZOS_MEM	FSR0,ZOS_JOB,ZOS_SST
	moviw	FSR0++		;   fsr0 = 0x10 * (1+ZOS_JOB) + ZOS_SST;
	movwf	STATUS_SHAD	;   STATUS_SHAD = *fsr0++;
	moviw	FSR0++		;
	movwf	WREG_SHAD	;   WREG_SHAD = *fsr0++;
	movf	ZOS_JOB,w	; // point to correct 80-byte page of local SRAM
	movwf	BSR_SHAD	;   BSR_SHAD = ZOS_JOB; // not STKPTR
	moviw	++FSR0		;
	movwf	PCLATH_SHAD	;   PCLATH_SHAD = *++fsr0;
	moviw	++FSR0		;
	movwf	FSR0L_SHAD	;   FSR0L_SHAD = *++fsr0;
	moviw	++FSR0		;
	movwf	FSR0H_SHAD	;   FSR0H_SHAD = *++fsr0;
	moviw	++FSR0		;
	movwf	FSR1L_SHAD	;   FSR1L_SHAD = *++fsr0;
	moviw	++FSR0		;
	movwf	FSR1H_SHAD	;   FSR1H_SHAD = *++fsr0;
	
	;; get stack spun around to where ZOS_JOB expects it, return from ISR
	ZOS_ROL	BSR_SHAD,ZOS_JOB,ZOS_MSK,FSR0,ZOS_STK
zos_don
	retfie			;
zos_wra
	clrf	ZOS_JOB		;  fsr0 = 0x10 * (1 + (ZOS_JOB = 0));
	ZOS_MEM	FSR0,ZOS_JOB,0	; }
	bra	zos_itr		;}

	;; software interrupt processing reached by jumping to 0x0002 with W set
zos_swj
	;; save the shadow registers (for the ones that have them) to use retfie
	bcf	INTCON,GIE	; INTCON &= ~(1<<GIE); // interrupt would be bad
	movwf	ZOS_JOB		; // just convenient temporary global for WREG
	movf	STATUS,w	;
	movwf	ZOS_MSK		; // just convenient temporary global for STATUS
	movf	BSR,w		;
	banksel	BSR_SHAD	; // BSR = the job# that made the interrupt call
	movwf	BSR_SHAD	; BSR_SHAD = BSR;
	movf	ZOS_JOB,w	; // WREG = the software interrupt type
	movwf	WREG_SHAD	; WREG_SHAD = WREG;
	movf	ZOS_MSK,w	;
	movwf	STATUS_SHAD	; STATUS_SHAD = STATUS;
	movf	PCLATH,w	;
	movwf	PCLATH_SHAD	; PCLATH_SHAD = PCLATH;
	movf	FSR0L,w		;
	movwf	FSR0L_SHAD	; FSR0L_SHAD = FSR0L;
	movf	FSR0H,w		;
	movwf	FSR0H_SHAD	; FSR0H_SHAD = FSR0H;
	movf	FSR1L,w		;
	movwf	FSR1L_SHAD	; FSR1L_SHAD = FSR1L;
	movf	FSR1H,w		;
	movwf	FSR1H_SHAD	; FSR1H_SHAD = FSR1H;
	movf	BSR_SHAD,w	;
	movwf	BSR		; // BSR unchanged from what it had been at call

	;; see if the interrupt type is a system one (<8)
	movlp	zos_swh
	movlw	ZOS_SI7|ZOS_SI6|ZOS_SI5|ZOS_SI4|ZOS_SI3
	andwf	WREG_SHAD,w	; if (0 == /* call-type number: */ WREG_SHAD &
	btfss	STATUS,Z	;   (ZOS_SI7|ZOS_SI6|ZOS_SI5|ZOS_SI4|ZOS_SI3)) {
	goto	zos_swh		;  // handle a system ZOS_SWI call:

	;; ZOS_NEW requires us to search for a BSR value first among empty slots
	movf	WREG_SHAD,f	;
	btfss	STATUS,Z	;  if (ZOS_NEW == WREG_SHAD) {
	bra	zos_swp		;  zos_cre:
zos_cre
	clrf	ZOS_JOB		;   zos_job = 0;
	ZOS_MEM	FSR1,ZOS_JOB,0
zos_emp
	movlw	0x10		;   for (fsr1 = 0x10*(1+zos_job);
	addwf	FSR1L,f		;
	incf	ZOS_JOB,f	;        zos_job++ <= ZOS_NUM;
	movlw	0xff-ZOS_NUM	;
	addwf	ZOS_JOB,w	;
	btfsc	STATUS,Z	;        fsr1 += 10) {
	bra	zos_err		;    if (ZOS_PCH[FSR1] == 0)
	moviw	ZOS_PCH[FSR1]	;     break;
	btfss	STATUS,Z	;   }
	bra	zos_emp		;   if (zos_job <= ZOS_NUM) {
zos_dup
	movf	FSR0L,w		;    // save handle now so we can re-use fsr0
	movwi	ZOS_HDL[FSR1]	;    // (no harm if we don't validate it as PCH)
	movf	FSR0H,w		;    ZOS_HDL[fsr1] = fsr0 & 0x00ff;
	movwi	ZOS_HDH[FSR1]	;    ZOS_HDH[fsr1] = fsr0 >> 8;
	movf	BSR,f		;    if (bsr == 0)
	btfsc	STATUS,Z	;     goto zos_swk; // job#0 (launcher) has perm
	bra	zos_swk		;    fsr0 = 0x10 * (1+bsr); // struct for caller
	ZOS_MEM	FSR0,BSR,0
	moviw	ZOS_HDH[FSR0]	;    if (ZOS_HDH[fsr0] & ZOS_PRV)
	btfsc	WREG,ZOS_PRB	;     goto zos_swk; // job has privileged perms
	bra	zos_swk		;   }
zos_err	
	clrf	ZOS_JOB		;   zos_job = 0; ZOS_RFI(); // perm/full error!
	ZOS_RFI
	
	;; see if we're not running inside a job context (1 <= job# <= ZOS_NUM)
	;; in which case need to grab the targeted job from AR0 (if not ZOS_NEW)
	;; or find a targetable slot (if ZOS_NEW)
	;; unprivileged jobs can only do most things to themselves
zos_swp
	movf	BSR,f		;  } else {
	btfss	STATUS,Z	;   if (bsr != 0) {
	bra	zos_elv		;    fsr1 = 0x10 * (1+bsr); // struct for job
	ZOS_MEM	FSR1,BSR,0
	moviw	ZOS_HDH[FSR1]	;    if (ZOS_HDH[fsr1] & ZOS_PRV == 0)
	btfss	WREG,ZOS_PRB	;     goto zos_swk;
	bra	zos_swk		;   }
	
	;; desired job# (instead of this one) into BSR from AR0 (if not ZOS_NEW)
zos_elv
	movf	ZOS_AR0,w	;   // access granted, bring the patient to me
	movwf	BSR		;   bsr = ZOS_AR0;
zos_swk
	movf	WREG_SHAD,w	;  }
	brw			;
	bra	zos_sw0		;
	bra	zos_sw1		;
	bra	zos_sw2		;
	bra	zos_sw3		;
	bra	zos_sw4		;
	bra	zos_sw5		;
	bra	zos_sw6		;
	bra	zos_sw7		;  switch (WREG_SHAD) { // guaranteed < 8
zos_sw0
	moviw	ZOS_HDL[FSR1]	;  case ZOS_NEW:
	movwi	ZOS_PCL[FSR1]	;   ZOS_PCL[fsr1] = ZOS_HDL[fsr1];
	moviw	ZOS_HDH[FSR1]	;   // retain HDL MSB (which indicate privilege)
	andlw	0x7f		;   // clear PC MSB (which indicates sleepiness)
	movwi	ZOS_PCH[FSR1]	;   ZOS_PCH[fsr1] = ZOS_HDH[fsr1] & 0x7f;
	movlw	ZOS_BOS		;   ZOS_SSP[fsr1] = ZOS_BOS;
	movwi	ZOS_SSP[FSR1]	;
	movf	ZOS_AR0,w	;   ZOS_ISR[fsr1] = ZOS_AR0;
	movwi	ZOS_ISR[FSR1]	;
	movf	ZOS_AR1,w	;   ZOS_ISH[fsr1] = ZOS_AR1;
	movwi	ZOS_ISH[FSR1]	;
	movf	ZOS_AR2,w	;   ZOS_HIM[fsr1] = ZOS_AR2;
	movwi	ZOS_HIM[FSR1]	;
	movf	ZOS_AR3,w	;   ZOS_ISR[fsr1] = ZOS_AR3;
	movwi	ZOS_SIM[FSR1]	;   ZOS_RFI();
	ZOS_RFI
zos_sw1
	moviw	ZOS_PCH[FSR1]	;  case ZOS_SLP:
	iorlw	0x80		;   ZOS_PCH[fsr1] |= 0x80;
	movwi	ZOS_PCH[FSR1]	;   ZOS_RFI();
	ZOS_RFI
zos_sw2
	clrw			;  case ZOS_END: ZOS_PCH[fsr1] = 0;
	movwi	ZOS_PCH[FSR1]	;   ZOS_RFI(); // killing is so quick
	ZOS_RFI
zos_sw3
	moviw	ZOS_HDL[FSR1]	;  case ZOS_RST:
	movwi	ZOS_PCL[FSR1]	;   ZOS_PCL[fsr1] = ZOS_HDL[fsr1];
	moviw	ZOS_HDH[FSR1]	;
	andlw	0x7f		;   // clear PC MSB (which indicates sleepiness)
	movwi	ZOS_PCH[FSR1]	;   ZOS_PCH[fsr1] = ZOS_HDH[fsr1] & 0x7f;
	movlw	ZOS_BOS		;   ZOS_SSP[fsr1] = ZOS_BOS; 
	movwi	ZOS_SSP[FSR1]	;  case ZOS_YLD: ZOS_RFI(); // nothing to do
zos_sw4
	ZOS_RFI
	
zos_sw5
;;; FIXME: need to do a copy of 0x20-0x6f...into every non-running slot?!?
	movf	ZOS_AR0,w	;  case ZOS_FRK:
	movwf	ZOS_JOB		;   ZOS_JOB = ZOS_AR0;
	ZOS_MEM	FSR1,ZOS_JOB,0
	moviw	ZOS_PCH[FSR1]	;
	btfsc	STATUS,Z	;
	bra	zos_sw4		;   if (ZOS_PCH[fsr1]) {
	moviw	ZOS_HDL[FSR1]	;
	movwf	FSR0L		;
	moviw	ZOS_HDH[FSR1]	;
	movwf	FSR0H		;    fsr0 = (ZOS_HDH[fsr1]<<8) | ZOS_HDL[fsr1];
	moviw	ZOS_ISR[FSR1]	;
	movwf	ZOS_AR0		;    ZOS_AR0 = ZOS_ISR[fsr1];
	moviw	ZOS_ISH[FSR1]	;
	movwf	ZOS_AR1		;    ZOS_AR1 = ZOS_ISH[fsr1];
	moviw	ZOS_HIM[FSR1]	;
	movwf	ZOS_AR2		;    ZOS_AR2 = ZOS_HIM[fsr1];
	moviw	ZOS_SIM[FSR1]	;
	movwf	ZOS_AR3		;    ZOS_AR3 = ZOS_SIM[fsr1];
	banksel	WREG_SHAD
	clrf	WREG_SHAD	;    WREG_SHAD = ZOS_NEW;
	movlb	0		;    goto zos_cre;//spoof privilege to fork self
	bra	zos_cre		;   }
zos_sw6
	movf	BSR,w		;  case ZOS_EXE:
	movwf	ZOS_JOB		;   ZOS_JOB = BSR;
	ZOS_MEM	FSR1,ZOS_JOB,0	;   fsr1 = 0x10 * (1+ZOS_JOB);
	banksel	WREG_SHAD	;
	clrf	WREG_SHAD	;    WREG_SHAD = ZOS_NEW;
	movlb	0		;    //spoof privilege to overwrite
	bra	zos_dup		;    goto zos_dup;
zos_sw7
	movlw	ZOS_NUM+1	;  case ZOS_FND:
	movwf	ZOS_JOB		;   ZOS_JOB = ZOS_NUM + 1;
	ZOS_MEM	fsr1,ZOS_JOB,0	;   fsr1 = 0x10 * (1 + ZOS_JOB);
zos_nxt
	ZOS_LIV	fsr1,ZOS_JOB,0,zos_bad
	moviw	ZOS_HDL[FSR1]	;   while (ZOS_LIV(&fsr0, &ZOS_JOB, 0)) {
	xorwf	ZOS_AR0,w	;
	btfss	STATUS,Z	;
	bra	zos_nxt		;    
	moviw	ZOS_HDH[FSR1]	;    int16_t a=(ZOS_AR1<<8)|ZOS_AR0;
	xorwf	ZOS_AR1,w	;    int16_t b=(ZOS_HDH[fsr0]<<8)|ZOS_HDL[fsr0];
	andlw	0x7f		;
	btfss	STATUS,Z	;    if (a & 0x7f == b & 0x7f)
	bra	zos_nxt		;     WREG_SHAD = ZOS_JOB;
	banksel	WREG_SHAD
	movf	ZOS_JOB,w	;   }
	movwf	WREG_SHAD	;   ZOS_RET();
zos_bad
	ZOS_RET
	
	;; else handle the software interrupt with the first registered handler
zos_swh
	movlw	ZOS_NUM		;
	movwf	ZOS_JOB		; zos_job = ZOS_NUM;
	ZOS_MEM	FSR0,ZOS_JOB,0x10
zos_swl
	movlw	0-0x10	      	; for (fsr0 = 0x10*(1+zos_job);
	addwf	FSR0L,w		;
	movwf	FSR0L		;
	andlw	0x60		;      fsr0 > 0x10;
	btfsc	STATUS,Z	;
	bra	zos_swm		;      fsr0 -= 0x10) {
	decf	ZOS_JOB,f	;  zos_job--;
	moviw	ZOS_PCH[FSR0]	;
	btfsc	STATUS,Z	;  if (ZOS_PCH[fsr0] == 0)
	bra	zos_swl		;   continue; // job killed or slot just empty
	moviw	ZOS_SIM[FSR0]	;
	andwf	WREG_SHAD,w	;
	movwf	ZOS_MSK		;
	btfsc	STATUS,Z	;
	bra	zos_swl		;  else if ((zos_msk = ZOS_SIM[fsr0]&WREG_SHAD))
	moviw	ZOS_ISH[FSR0]	;
	movwf	PCLATH		;
	moviw	ZOS_ISR[FSR0]	;   goto (void*)(ZOS_ISR[fsr0]); // will ZOS_RFI
	movwf	PCL		; }
	
	;; no registered SWI handler: jump into the hardware interrupt scheduler
zos_swm
	ZOS_RFI

zos_ini
	;; clear out page 0 to reflect no 
	movlb	0		; "invalid" job#
zos_end
	;; your program starts here, with a launcher job (#6) that ends after:
	;; 1) setting up oscillators, timers, other peripherals, etc.
	;; 2) starting other jobs with calls to ZOS_NEW
	;; 3) calling to ZOS_END (which will enable interrupts)
	;; 
	;; jobs are never allowed to manipulate the BSR directly, as that
	;; is the prerogative of ZOS (it being used as the current job #)
	;; and the BSR may never end up greater than ZOS_NUM in user space!!!
	;;
