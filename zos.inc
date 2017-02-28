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
	
ZOS_MEM	macro	fsrnum,job,offset;
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

;;; when searching for the correct hardware interrupt handler, without stack hit
ZOS_RET	macro
	pagesel	zos_nhw
	goto	zos_nhw		; next iteration of loop (mustn't clobber FSR0!)
	endm

;;; at the end of any interupt handler, goes back to scheduler without stack hit
ZOS_RFI	macro	
	pagesel	zos_sch
	goto	zos_sch
	endm

ZOS_LIV	macro	fsrnum,job,incr,unf
	local	fsrn,loop
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif
loop
	if (incr)
	 movlw	0x10		;
	else
	 movlw	0-0x10		;
	endif
	addwf	FSR#v(fsrn)L,f	;
	if (incr > 0)
	 incf	job,f		;
	 movlw	0xff-ZOS_NUM	;
	 addwf	job,w		;
	else
	 decf	job,f		;
	endif
	btfss	STATUS,Z	;
	if (unf)
	 bra	unf		;
	 moviw	ZOS_PCH[FSR#v(fsrn)]
	 btfss	STATUS,Z	;
	endif
	bra	loop		;
	endif
	endm
	
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
	clrf	ZOS_MSK		; ZOS_MSK = 0; // indicates a HWI (not SWI) type
	movlw	ZOS_NUM+1
	movwf	ZOS_JOB
	ZOS_MEM	FSR0,ZOS_JOB,0
zos_nhw	
	ZOS_LIV	FSR0,ZOS_JOB,0,zos_ign
	moviw	ZOS_HIM[FSR0]	;
	andwf	INTCON,w	;
	btfss	STATUS,Z	;
	bra	zos_doi		;
	banksel	PIE1		;
	variable i
i = 0
	while i < 10
	 ifdef (PIE#v(i))
	  moviw	ZOS_HIM[FSR0]	;
	  andwf	PIE#v(i),w	;
	  btfsc	STATUS,Z	;
	  bra	$+5		;
	  moviw	ZOS_HIM[FSR0]	;
	  andwf	PIR#v(i),w	;
	  btfss	STATUS,Z	;
	  bra	zos_doi		;
	 endif
i += 1
	endw
	bra	zos_nhw		;
	
	;; enter handler which will ZOS_RFI() to zos_sch if it's the correct one
	;; (after clearing the interrupt flag) else ZOS_RET() back up to zos_nhw
zos_doi
	moviw	ZOS_ISH[FSR0]	;
	movwf	PCLATH		;
	moviw	ZOS_ISR[FSR0]	;
	movwf	PCL		;
	
	;; scheduler
zos_ign
zos_sch
	;; look for the next-lowest-index runnable job in round-robin fashion
	decf	BSR_SHAD,w	;
	btfsc	STATUS,Z	;
	movlw	ZOS_NUM		;
	movwf	ZOS_JOB		;
	ZOS_MEM	FSR0,ZOS_JOB,0x10

	;; look for the next-lowest-index runnable job in round-robin fashion
	movf	BSR_SHAD,w	;
	movwf	ZOS_JOB		;
	ZOS_MEM	FSR0,ZOS_JOB,0
zos_rrb
	ZOS_LIV	FSR0,ZOS_JOB,0,zos_wra
	bra	zos_sav		;
	
	;; look for the next-highest-index runnable job in round-robin fashion
zos_wra
	movf	BSR_SHAD,w	;
	movwf	ZOS_JOB		;
	ZOS_MEM	FSR0,ZOS_JOB,0	;
zos_rrt
	ZOS_LIV	FSR0,ZOS_JOB,1,zos_don
	;; if this point is reached, a runnable job was found with job# ZOS_JOB
	;; (but we skip a whole bunch of trivial copies if ZOS_JOB==BSR_SHAD)

	;; copy the interrupted job's (BSR_SHAD) criticals into its bank 0 slot
zos_sav
	banksel	BSR_SHAD
	ZOS_MEM	FSR0,BSR_SHAD,ZOS_PCL
	movf	TOSL,w		;
	movwi	FSR0++		;
	movf	TOSH,w		;
	movwi	FSR0++		;
	
#if 1 ;speed (16 words, 16 cycles)
	movf	STATUS_SHAD,w	;
	movwi	FSR0++		;
	movf	WREG_SHAD,w	;
	movwi	FSR0++		;
	movf	STKPTR,w	;
	movwi	FSR0++		;
	movf	PCLATH_SHAD,w	;
	movwi	FSR0++		;
	movf	FSR0L_SHAD,w	;
	movwi	FSR0++		;
	movf	FSR0H_SHAD,w	;
	movwi	FSR0++		;
	movf	FSR1L_SHAD,w	;
	movwi	FSR0++		;
	movf	FSR1H_SHAD,w	;
	movwi	FSR0++		;
#else ;size (12 words, 61=4+7*7+1*6+2cycles)
	movlw	low STATUS_SHAD	;
	movwf	FSR1L		;
	movlw	high STATUS_SHAD;
	movwf	FSR1H		;
zos_sav
	moviw	FSR1++		;
	movwi	FSR0++		;
	movlw	ZOS_ISR		;
	xorlw	FSR0L,w		;
	btfss	STATUS,Z	;
	bra	zos_sav		;
	movf	STKPTR,w	;
	movwi	-6[FSR0]	;
#endif
	;; copy ZOS_JOB's criticals out of its bank 0 slot
	ZOS_MEM	FSR0,ZOS_JOB,ZOS_SST
	moviw	FSR0++		;
	movwf	STATUS_SHAD	;
	moviw	FSR0++		;
	movwf	WREG_SHAD	;
	movf	ZOS_JOB,w	;
	movwf	BSR_SHAD	; // point to correct 80-byte page of local SRAM
	moviw	++FSR0		;
	movwf	PCLATH_SHAD	;
	moviw	++FSR0		;
	movwf	FSR0L_SHAD	;
	moviw	++FSR0		;
	movwf	FSR0H_SHAD	;
	moviw	++FSR0		;
	movwf	FSR1L_SHAD	;
	moviw	++FSR0		;
	movwf	FSR1H_SHAD	;
	
	;; get the stack spun around to where ZOS_JOB expects it to be
	ZOS_ROL	BSR_SHAD,ZOS_JOB,ZOS_MSK,FSR0,ZOS_STK
zos_don
	retfie			;
	
	;; software interrupt processing
zos_swj
	;; save the shadow registers (for the ones that have them)
	bcf	INTCON,GIE	;
	movwf	ZOS_JOB		; // just convenient temporary global for WREG
	movf	STATUS,w	;
	movwf	ZOS_MSK		; // just convenient temporary global for STATUS
	movf	BSR,w		;
	banksel	BSR_SHAD	;
	movwf	BSR_SHAD	; // the job# that made the interrupt call
	movf	0xZOS_JOB,w	;
	movwf	WREG_SHAD	; // the software interrupt type
	movf	0xZOS_MSK,w	;
	movwf	STATUS_SHAD	;
	movf	PCLATH,w	;
	movwf	PCLATH_SHAD	;
	movf	FSR0L,w		;
	movwf	FSR0L_SHAD	;
	movf	FSR0H,w		;
	movwf	FSR0H_SHAD	;
	movf	FSR1L,w		;
	movwf	FSR1L_SHAD	;
	movf	FSR1H,w		;
	movwf	FSR1H_SHAD	;
	movf	BSR_SHAD,w	;
	movwf	BSR		; // BSR back to what it used to be

	;; see if the interrupt type is a system one (<8)
	movlp	zos_swh
	movlw	ZOS_SI7|ZOS_SI6|ZOS_SI5|ZOS_SI4|ZOS_SI3
	andwf	WREG_SHAD,w	; if (0 == /* call-type number: */ WREG_SHAD &
	btfss	STATUS,Z	;   (ZOS_SI7|ZOS_SI6|ZOS_SI5|ZOS_SI4|ZOS_SI3)) {
	goto	zos_swh		;  // handle a system ZOS_SWI call:

	;; ZOS_NEW requires us to search for a BSR value first among empty slots
	movf	WREG_SHAD,f	;
	btfss	STATUS,Z	;
	bra	zos_swp		;  if (ZOS_NEW == WREG_SHAD) {
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
;;; fork: duplicate find code above (functionize?) then do a copy of 0x20-0x6f



	ZOS_RFI
zos_sw6
;;; exec: just jump into new except with this job as the known slot
	


	ZOS_RFI
zos_sw7
;;; find: more find code, just for fun
	
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
