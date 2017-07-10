;;; zos.inc
;;; a lightweight, small-footprint, preemptively multitasking RTOS for Microchip
;;;  Technology's entire enhanced midrange 8-bit PIC microcontroller family:
;;;
;;; jobs (up to 5) are never allowed to manipulate the BSR directly, as that is
;;; the prerogative of zOS (it being used as the current job #) and the bank may
;;; never end up greater than zOS_NUM in user space with interrupts enabled!!!

;;; memory footprint:
;;;  ~613 14-bit words for base RTOS i.e. main() starts at 0x0263
;;;  ~511 words if zOS_MIN is defined to omit FRK/EXE/FND (thus SWI#4~7=zOS_YLD)
;;; 
;;; SRAM footprint:
;;; 86 bank-0 bytes claimed by RTOS, 30 bytes of stack scratch space relocatable
;;; 
;;; available bytes    possible jobs with    local bytes/job (+any heap, besides
;;;  on PIC device     80 bytes RAM each     2 global bytes) if zOS_NUM set to 5
;;; ===============    ==================    ===================================
;;;      128                 0                          0 (+2)
;;;      256                 1				0 (+130)
;;;      384                 3                          0 (+258)
;;;      512                 4                          0 (+386)
;;;      768                 5                          80 (+2)
;;;  >=1,024                 5                          80 (+258...)

;;; you may redefine a constant zOS_NUM with the maximum job number (<6,
;;; as determined by where the general purpose register memory stops, as
;;; the guaranteed 2 bytes global memory isn't sufficient for most jobs)
#ifdef zOS_NUM
#else
zOS_NUM	set	5
#endif

;;; you may redefine the location of the scratch space for restoring the stack
;;; after each context switch (by default it is 0x20 in bank zOS_NUM+1, but can
;;; be pulled in on small devices into unused local storage, or pushed out for
;;; a heap-management job space)
#ifdef zOS_STK
#else
zOS_STK	set	(((zOS_NUM+1)<<7)|0x20)
#endif

;;; software interrupt infrastructure zOS is based on (even with interrupts off)
	
;;; 5 user-definable software interrupt lines:
zOS_SB7	equ	7
zOS_SI7	equ	(1<<zOS_SB7)
zOS_SB6	equ	6
zOS_SI6	equ	(1<<zOS_SB6)
zOS_SB5	equ	5
zOS_SI5	equ	(1<<zOS_SB5)
zOS_SB4	equ	4
zOS_SI4	equ	(1<<zOS_SB4)
zOS_SB3	equ	3
zOS_SI3	equ	(1<<zOS_SB3)

;;; 7 system softare interrupts for job management:
zOS_FND	equ	0x07		; find a running job <=AR2 by its handle AR1:AR0
zOS_EXE	equ	0x06		; replace this job with a new job (unpriv'ed)
zOS_FRK	equ	0x05		; copy a running job into a new job
zOS_YLD	equ	0x04		; (in)voluntarily cede processor before next irq
zOS_RST	equ	0x03		; restart job at its start address (vs. END+NEW)
zOS_END	equ	0x02		; job killed, slot# available for NEW
zOS_SLP	equ	0x01		; indicate job waiting on its ISR, so don't run
zOS_NEW	equ	0x00		; create a job (FSR0==addr,AR1:0==isr,AR3:2==IM)

;;; global memory space for 2 scratch registers plus message-passing mailboxes
zOS_JOB	equ	0x70		; next job to run (0 if unknown)
zOS_MSK	equ	0x71		; masked-off sofware interrupt for ISR to handle
zOS_J1L	equ	0x72		;  (repurposeable as scratch after zOS_RFI call)
zOS_J1H	equ	0x73
zOS_J2L	equ	0x74
zOS_J2H	equ	0x75
zOS_J3L	equ	0x76
zOS_J3H	equ	0x77
zOS_J4L	equ	0x78
zOS_J4H	equ	0x79
zOS_J5L	equ	0x7a
zOS_J5H	equ	0x7b
	;; must disable interrupts e.g. with zOS_ARG(0) before writing SWI args:
zOS_AR0	equ	0x7c
zOS_AR1	equ	0x7d
zOS_AR2	equ	0x7e
zOS_AR3	equ	0x7f

;;; job/shadow register offsets from zOS_J0M, zOS_J1M,...
zOS_HDL	equ	0x00		; handle, the start address of the job
zOS_HDH	equ	0x01		;
zOS_PRB	equ	7		;  MSB of HDL indicates privilege(manage others)
zOS_RAM	equ	0		;
zOS_FLA	equ	1		;
zOS_UNP	equ	0		;
zOS_PCL	equ	0x02		; address to resume execution
zOS_PCH	equ	0x03		;  "impossible" PCH 0x00==not runnable
zOS_WAI	equ	7		;  MSB of PCH indicates sleeping (wait for int)
zOS_SST	equ	0x04		; shadow STATUS
zOS_SWR	equ	0x05		; shadow WREG
zOS_SSP	equ	0x06		; STKPTR to be restored (BSR implied by base)
zOS_SPH	equ	0x07		; PCLATH to be restored
zOS_SF0	equ	0x08		; shadow FSR0
zOS_SF1	equ	0x0a		; shadow FSR1
zOS_ISR	equ	0x0c		; interrupt service routine address for the job
zOS_ISH	equ	0x0d		; interrupt service routine address for the job
zOS_HIM	equ	0x0e		; mask for hardware interrupts to process (0=no)
zOS_SIM	equ	0x0f		; mask for software interrupts (low 3 always==1)
	
zOS_TOS	equ	0x0e  		; STKPTR for full stack (0x0f reserved for ISRs)
zOS_BOS	equ	0x0b		; STKPTR for empty stack (first push is to 0x0c)
	
;;; bank 0 memory space for managing jobs, 1@0x20, 2@0x30, ... , 5@0x60
zOS_J1M	equ	0x20
zOS_J2M	equ	0x30
zOS_J3M	equ	0x40
zOS_J4M	equ	0x50	
zOS_J5M	equ	0x60	
	
zOS_MEM	macro	fsrnum,job,offset
	local	fsrn
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif
	swapf	job,w		;inline void zOS_MEM(int8_t* *fsrnum,
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
;;; typically: zOS_ROL BSR_SHAD,JOB_NUM(BSR?),zOS_TMP,FSR0,zOS_STK
;;; note: caller is responsible for making sure the STKPTR/_SHAD bank is active
zOS_ROL	macro	old,new,temp,fsrnum,base
	local fsrn,loop1,loop2,done
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif
	movlw	low base	;inline void zOS_ROL(const int8_t* old,
	movwf	FSR#v(fsrn)L	;                    const int8_t* new,
	movlw	high base	;                    int8_t* temp,
	movwf	FSR#v(fsrn)H	;                    int16_t* *fsrnum,
	movf	new,w		;                    int8_t* base) {
	subwf	old,w		; //responsibility of caller to banksel STKPTR
	btfsc	STATUS,Z	; if (*new == *old) // nothing to do
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
	movwi	FSR#v(fsrn)++	;      STKPTR = (STKPTR>0) ? (STKPTR-1):zOS_TOS)
	movf	TOSH,w		;
	movwi	FSR#v(fsrn)++	;  *(*fsrnum)++ = (TOSH << 8) | TOSL;
	decf	STKPTR,f	;
	movlw	zOS_TOS		;
	btfsc	STKPTR,4	;
	movwf	STKPTR		;
	movf	temp,w		;
	xorwf	STKPTR,w	;
	btfss	STATUS,Z	; // now rebuild the unrolled stack
	bra	loop1		;
	clrf	STKPTR		; for (STKPTR = 0;
loop2
	moviw	--FSR#v(fsrn)	;      STKPTR <= zOS_TOS;
	movwf	TOSH		;      STKPTR++) {
	moviw	--FSR#v(fsrn)	;  TOSH = *(*fsrnum) >> 8;
	movwf	TOSL		;  TOSL = *--(*fsrnum) & 0x00ff;
	incf	STKPTR,w	; }
	movwf	STKPTR		;
	sublw	zOS_TOS		;
	btfss	WREG,7		;
	bra	loop2		;}
done
	endm
	
#ifdef GPASM
zOS_RTL	equ	(STATUS_SHAD-FSR1H_SHAD-2)
zOS_RTH	equ	(STATUS_SHAD-FSR1H_SHAD-1)
zOS_RTS	equ	(STATUS_SHAD-FSR1H_SHAD+2)
#else
zOS_RTL	equ	((STATUS_SHAD-FSR1H_SHAD-2)&0x3f)
zOS_RTH	equ	((STATUS_SHAD-FSR1H_SHAD-1)&0x3f)
zOS_RTS	equ	((STATUS_SHAD-FSR1H_SHAD+2)&0x3f)	
#endif

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
zOS_RET	macro
	pagesel	zos_nhw
	goto	zos_nhw		;#define zOS_RET() goto zos_nhw
	endm

;;; at the end of any interrupt handler goes back to scheduler without stack hit
zOS_RFI	macro	retreg
	pagesel	zos_sch		;inline void zOS_RFI(int8_t* retreg) {
	if (retreg-WREG)
	 movf	retreg,w	; w = *retreg; goto zos_sch;
	endif
	goto	zos_sch		;}
	endm

;;; find something runnable (i.e. PCH != 0, but sleep MSB is OK), at job+/-1
;;; according to incr then branch to unf if job-1 == 0 or job+1 > zOS_NUM,
;;; with fsrnum pointing to job's bank 0 structure and then incremented +/-16
zOS_LIV	macro	fsrnum,job,incr,unf
	local	fsrn,loop
	if (fsrnum & 3)
fsrn set 1
	else
fsrn set 0
	endif
loop
	if (incr)
	 movlw	0x10		;inline int8_t zOS_LIV(int8_t* *fsrnum,
	else
	 movlw	0-0x10		;     uint8_t *job, int8_t incr, void *(unf)()) {
	endif
	addwf	FSR#v(fsrn)L,f	; do {
	if (incr)
	 incf	job,f		;  *fsrnum += incr ? 0x10 : -0x10;// next struct
	 movlw	0xff-zOS_NUM	;  job += incr ? 1 : -1; // next job#
	 addwf	job,w		;  if ((job == 0) || (job >= zOS_NUM+1)) {//past
	 btfss	WREG,7		;
	else
	 decf	job,f		;   goto unf; // Z was set
	 btfsc	STATUS,Z	;  } else if (zOS_PCH[fsrnum]) // found runnable
	endif
	bra	unf		;   return w = zOS_PCH[fsrnum]; // Z was cleared
	moviw	zOS_PCH[FSR#v(fsrn)]
	btfsc	STATUS,Z	; } while (1); // job is runnable (or unf was 0)
	bra	loop		;}
	endm
	
#ifdef FSR0
#else
FSR0	 equ	FSR0L
#endif
#ifdef FSR1
#else
FSR1	 equ	FSR1L
#endif

	;; a job switch is attempted with every incoming interrupt
	;; user jobs are responsible for processing their own interrupts
	;; with an interrupt handler registered at the time of creation

	org	0x0000
	pagesel	zos_ini
	goto	zos_ini		;<--zos_ini is run upon reset to bootstrap zOS

	org	0x0002
	pagesel	zos_swj
	goto	zos_swj		;<--zOS_SWI is call to 0x0002, a jump to zos_swj

	;; enter handler which will zOS_RFI() to zos_sch if it's the correct one
	;; (and we're not still in the bank-0 initialization before interrupts),
	;; after clearing the interrupt flag...else zOS_RET() back up to zos_nhw

	org	0x0004
	;; find first willing handler for an enabled interrupt matching _xIM bit
#ifdef PIE0
zOS_PIE	 equ	PIE0
#else
zOS_PIE	 equ	INTCON
#endif
zos_004
	movlw	zOS_NUM+1	;__isr void zos_004(void) {
	movwf	zOS_JOB		; zOS_JOB = zOS_NUM+1;// search from high to low
	zOS_MEM	FSR0,zOS_JOB,0	; fsr0 = 0x10 * (1 + zOS_JOB);
zos_nhw
	zOS_LIV	FSR0,zOS_JOB,0,zos_004
	clrwdt			; do { // until serviceable by running ISR since
	banksel	zOS_PIE
	moviw	zOS_HIM[FSR0]	;  int8_t w = 0;  // no runnable job schedulable
	andwf	zOS_PIE,w	;  clrwdt();
	btfss	STATUS,Z	;  while (zOS_LIV(&fsr0, &zOS_JOB, 0)) {
	bra	zos_cmp		;  //match enabled interrupts against HIM fields
#ifdef PIE1
	moviw	zOS_HIM[FSR0]	;   if ((w = zOS_HIM[fsr0] & zOS_PIE))
	banksel	PIE1
	andwf	PIE1,w		;    break;
	btfss	STATUS,Z	;   if ((w = zOS_HIM[fsr0] & zOS_PIE1))
	bra	zos_cmp		;    break;
#endif
#ifdef PIE2
	moviw	zOS_HIM[FSR0]	;
	andwf	PIE2,w		;
	btfss	STATUS,Z	;   if ((w = zOS_HIM[fsr0] & zOS_PIE2))
	bra	zos_cmp		;    break;
#endif
#ifdef PIE3
	moviw	zOS_HIM[FSR0]	;
	andwf	PIE3,w		;
	btfss	STATUS,Z	;   if ((w = zOS_HIM[fsr0] & zOS_PIE3))
	bra	zos_cmp		;    break;
#endif
#ifdef PIE4
	moviw	zOS_HIM[FSR0]	;
	andwf	PIE4,w		;
	btfss	STATUS,Z	;   if ((w = zOS_HIM[fsr0] & zOS_PIE4))
	bra	zos_cmp		;    break;
#endif
#ifdef PIE5
	moviw	zOS_HIM[FSR0]	;
	andwf	PIE5,w		;
	btfss	STATUS,Z	;   if ((w = zOS_HIM[fsr0] & zOS_PIE5))
	bra	zos_cmp		;    break;
#endif
#ifdef PIE6
	moviw	zOS_HIM[FSR0]	;
	andwf	PIE6,w		;
	btfss	STATUS,Z	;   if ((w = zOS_HIM[fsr0] & zOS_PIE6))
	bra	zos_cmp		;    break;
#endif
#ifdef PIE7
	moviw	zOS_HIM[FSR0]	;
	andwf	PIE7,w		;
	btfss	STATUS,Z	;   if ((w = zOS_HIM[fsr0] & zOS_PIE7))
	bra	zos_cmp		;    break;
#endif
#ifdef PIE8
	moviw	zOS_HIM[FSR0]	;
	andwf	PIE8,w		;
	btfss	STATUS,Z	;   if ((w = zOS_HIM[fsr0] & zOS_PIE8))
	bra	zos_cmp		;    break;
#endif
#ifdef PIE9
	moviw	zOS_HIM[FSR0]	;
	andwf	PIE9,w		;
	btfss	STATUS,Z	;   if ((w = zOS_HIM[fsr0] & zOS_PIE9))
	bra	zos_cmp		;    break; // found a potential handler for any
#endif
	bra	zos_nhw		;  }      // interrupt flag in this bit position
zos_cmp
	clrf	zOS_MSK		;  if (w) {
	moviw	zOS_ISH[FSR0]	;   zOS_MSK = 0; //indicates HWI (not SWI) type
	movwf	PCLATH		;   *(zOS_ISR[fsr0])();
	moviw	zOS_ISR[FSR0]	;  }
	movwf	PCL		; } // if handler refuses, loops to the next job
	
	;; scheduler begins here, called either after HWI/SWI done or zOS_RUN():
zos_sch
	banksel	WREG_SHAD
	movwf	WREG_SHAD	;zos_sch:
	movf	BSR_SHAD,w	;  // stay in _SHAD/STKPTR/TOS bank until retfie
	btfsc	STATUS,Z	;  WREG_SHAD = w; // sent via ZOS_RFI()
	bra	zos_don		;  if ((zOS_JOB = BSR_SHAD)!= 0)//2x max or '004
	movwf	zOS_JOB		;   for (zOS_MSK = 2; zOS_MSK; zOS_MSK--) {
	movlw	3		;
	movwf	zOS_MSK		;    //zOS_MSK=2 first time through,1 after wrap
	bra	zos_1st		;    zOS_MEM(fsr0,zOS_JOB,0);
zos_itr
	zOS_LIV	FSR0,zOS_JOB,1,zos_wra
	clrwdt			;   //zOS_LIV leaves PCH in WREG, test runnable?
	btfsc	WREG,zOS_WAI	;    while(zOS_LIV(fsr0,zOS_JOB,1)&(1<<zOS_WAI))
	bra	zos_itr		;     clrwdt();
	
	;; if this point is reached, a runnable job was found with job# zOS_JOB
	;; (but we skip a whole bunch of trivial copies if zOS_JOB==BSR_SHAD)
	movf	BSR_SHAD,w	;
	xorwf	zOS_JOB,w	;
	btfsc	STATUS,Z	;
	bra	zos_don		;    if (zOS_JOB != BSR_SHAD) {

	;; copy the interrupted job's (BSR_SHAD) criticals into its bank 0 slot;
	;; by pure chance this clobbers the "unused" range 0x72~0x7b on 1st run!
	zOS_MEM	FSR0,BSR_SHAD,zOS_PCL
	movf	TOSL,w		;     fsr0 = 0x10 * (1+BSR_SHAD) + zOS_PCL;
	movwi	FSR0++		;     *fsr0++ = TOSL; // return address from IRQ
	movf	TOSH,w		;
	movwi	FSR0++		;     *fsr0++ = TOSH;
	
	movf	STATUS_SHAD,w	;
	movwi	FSR0++		;     *fsr0++ = STATUS_SHAD;
	movf	WREG_SHAD,w	;
	movwi	FSR0++		;     *fsr0++ = WREG_SHAD;
	movf	STKPTR,w	;
	movwi	FSR0++		;     *fsr0++ = STKPTR; // not BSR_SHAD
	movf	PCLATH_SHAD,w	;
	movwi	FSR0++		;     *fsr0++ = PCLATH_SHAD;
	movf	FSR0L_SHAD,w	;
	movwi	FSR0++		;     *fsr0++ = FSR0L_SHAD;
	movf	FSR0H_SHAD,w	;
	movwi	FSR0++		;     *fsr0++ = FSR0H_SHAD;
	movf	FSR1L_SHAD,w	;
	movwi	FSR0++		;     *fsr0++ = FSR1L_SHAD;
	movf	FSR1H_SHAD,w	;
	movwi	FSR0++		;     *fsr0++ = FSR1H_SHAD;

	;; get stack spun around to where zOS_JOB expects it on return from ISR
	zOS_ROL	BSR_SHAD,zOS_JOB,zOS_MSK,FSR1,zOS_STK
	
	;; copy zOS_JOB's criticals out of its bank 0 slot
	zOS_MEM	FSR0,zOS_JOB,zOS_SST
	moviw	FSR0++		;     fsr0 = 0x10 * (1+zOS_JOB) + zOS_SST;
	movwf	STATUS_SHAD	;     STATUS_SHAD = *fsr0++;
	moviw	FSR0++		;
	movwf	WREG_SHAD	;     WREG_SHAD = *fsr0++;
	movf	zOS_JOB,w	;     //point to correct 80-byte local SRAM page
	movwf	BSR_SHAD	;     BSR_SHAD = zOS_JOB; // not STKPTR
	moviw	++FSR0		;
	movwf	PCLATH_SHAD	;     PCLATH_SHAD = *++fsr0;
	moviw	++FSR0		;
	movwf	FSR0L_SHAD	;     FSR0L_SHAD = *++fsr0;
	moviw	++FSR0		;
	movwf	FSR0H_SHAD	;     FSR0H_SHAD = *++fsr0;
	moviw	++FSR0		;
	movwf	FSR1L_SHAD	;     FSR1L_SHAD = *++fsr0;
	moviw	++FSR0		;
	movwf	FSR1H_SHAD	;     FSR1H_SHAD = *++fsr0;
	
	;; set new job stack pointer, last step before completing context switch
	moviw	zOS_RTS[FSR0]	;
	movwf	STKPTR		;     STKPTR = zOS_SSP[FSR0-11];
	moviw	zOS_RTL[FSR0]	;     TOSL = zOS_PCL[FSR0-11];
	movwf	TOSL		;     TOSH = zOS_PCH[FSR0-11];
	moviw	zOS_RTH[FSR0]	;     return (void)__isr;
	movwf	TOSH		;    }
zos_don
	retfie			;    //if this point is reached, search wrapped:
zos_wra
	clrf	zOS_JOB		;    fsr0 = 0x10 * (1 + (zOS_JOB = 0));
zos_1st
	zOS_MEM	FSR0,zOS_JOB,0	;   }// wrap around only once, else wait for IRQ
	decfsz	zOS_MSK,f	; } while (1); // (since no job is schedulable)
	bra	zos_itr		;}
	bra	zos_004		;int8_t zos_swj(int8_t w){ // call vector at 002

	;; software interrupt processing reached by jumping to 0x0002 with W set
	;; which then calls to zos_swj, or by jumping to zos_skp after already
	;; processing a previous interrupt (since there is only 1 level of SHAD)
	;; to skip the copy into the shadow registers
zos_skp
	movwf	zOS_MSK		;
	bra	zos_sk2		;
zos_swj
	;; save the shadow registers (for the ones that have them) to use retfie
	bcf	INTCON,GIE	; INTCON &= ~(1<<GIE); // interrupt would be bad
	movwf	zOS_MSK		; zOS_MSK = WREG; // the software interrupt type
	movf	STATUS,w	;
	movwf	zOS_JOB		; // only convenient temporary global for STATUS
	movf	BSR,w		;
	banksel	BSR_SHAD	; // BSR = the job# that made the interrupt call
	movwf	BSR_SHAD	; BSR_SHAD = BSR;
	movf	zOS_JOB,w	;
	movwf	STATUS_SHAD	; STATUS_SHAD = zos_job = STATUS;
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
zos_sk2
	;; see if the interrupt type is a system one (<8)
	pagesel	zos_swh
	movlw	zOS_SI7|zOS_SI6|zOS_SI5|zOS_SI4|zOS_SI3
	andwf	zOS_MSK,w	; if (0 == /* call-type number: */ WREG_SHAD &
	btfss	STATUS,Z	;   (zOS_SI7|zOS_SI6|zOS_SI5|zOS_SI4|zOS_SI3)) {
	goto	zos_swh		;  // handle a system zOS_SWI call:

	;; zOS_NEW requires us to search for a BSR value first among empty slots
	movf	BSR_SHAD,w	;
	movwf	BSR		;  // BSR unchanged from what it had been at call
	movf	zOS_MSK,f	;
	btfss	STATUS,Z	;  if (zOS_MSK == zOS_NEW /*==0*/) {
	bra	zos_swp		;  zos_cre:
zos_cre
	clrf	zOS_JOB		;   zos_job = 0;
	zOS_MEM	FSR1,zOS_JOB,0
zos_emp
	movlw	0x10		;   for (fsr1 = 0x10*(1+zos_job);
	addwf	FSR1L,f		;
	incf	zOS_JOB,f	;        zos_job++ <= zOS_NUM;
	movlw	0xff-zOS_NUM	;
	addwf	zOS_JOB,w	;
	btfsc	STATUS,Z	;        fsr1 += 0x10) {
	bra	zos_err		;    if (zOS_PCH[FSR1] == 0)
	moviw	zOS_PCH[FSR1]	;     break;
	btfss	STATUS,Z	;   }
	bra	zos_emp		;   if (zos_job <= zOS_NUM) {
zos_dup
	movf	FSR0L,w		;    // save handle now so we can re-use fsr0
	movwi	zOS_HDL[FSR1]	;    // (no harm if we don't validate it as PCH)
	movf	FSR0H,w		;    zOS_HDL[fsr1] = fsr0 & 0x00ff;
	movwi	zOS_HDH[FSR1]	;    zOS_HDH[fsr1] = fsr0 >> 8;
	movf	BSR,f		;    if (bsr == 0)
	btfsc	STATUS,Z	;     goto zos_swk; // job#0 (launcher) has perm
	bra	zos_swk		;    fsr0 = 0x10 * (1+bsr); // struct for caller
	zOS_MEM	FSR0,BSR,0
	moviw	zOS_HDH[FSR0]	;    if (zOS_HDH[fsr0] & (1<<zOS_PRB))
	btfsc	WREG,zOS_PRB	;     goto zos_swk; // job has privileged perms
	bra	zos_swk		;   }
zos_err	
	clrf	zOS_JOB		;   zos_job = 0;
	zOS_RFI	zOS_JOB		;   zOS_RFI(zOS_JOB); // perms error or no empty
	
	;; see if we're not running inside a job context (1 <= job# <= zOS_NUM)
	;; in which case need to grab the targeted job from AR0 (if not zOS_NEW)
	;; or find a targetable slot (if zOS_NEW)
	;; unprivileged jobs can only do most things to themselves
zos_swp
	movf	BSR,w		;  } else {
	movwf	zOS_JOB		;   zos_job = bsr;
	btfsc	STATUS,Z	;   if (bsr != 0) {
	bra	zos_elv		;    fsr1 = 0x10 * (1+bsr); // struct for job
	zOS_MEM	FSR1,BSR,0
	moviw	zOS_HDH[FSR1]	;    if (zOS_HDH[fsr1] & (1<<zOS_PRB) == 0)
	btfss	WREG,zOS_PRB	;     goto zos_swk; // disallowed job in zOS_AR0
	bra	zos_swk		;   }
	
	;; desired job# (instead of this one) into BSR from AR0 (if not zOS_NEW)
zos_elv
	movf	zOS_AR0,w	;   // access granted, bring the patient to me
	movwf	BSR		;   bsr = zOS_AR0;
	zOS_MEM	FSR1,BSR,0
zos_swk
	movf	zOS_MSK,w	;  }
	brw			;  switch (zOS_MSK) { // guaranteed < 8
	bra	zos_sw0		;
	bra	zos_sw1		;
	bra	zos_sw2		;
	bra	zos_sw3		;
	bra	zos_sw4		;
	bra	zos_sw5		;
	bra	zos_sw6		;
	bra	zos_sw7		;  case zOS_NEW:
zos_sw0
	movf	zOS_AR0,w	;
	movwi	zOS_ISR[FSR1]	;   zOS_ISR[fsr1] = zOS_AR0;
	movf	zOS_AR1,w	;
	movwi	zOS_ISH[FSR1]	;   zOS_ISH[fsr1] = zOS_AR1;
	movf	zOS_AR2,w	;
	movwi	zOS_HIM[FSR1]	;   zOS_HIM[fsr1] = zOS_AR2;
	movf	zOS_AR3,w	;
	movwi	zOS_SIM[FSR1]	;   zOS_SIM[fsr1] = zOS_AR3;
	bra	zos_sw3		;   goto zos_sw3;
zos_sw1
	moviw	zOS_PCH[FSR1]	;  case zOS_SLP:
	iorlw	0x80		;   zOS_PCH[fsr1] |= 0x80;
	movwi	zOS_PCH[FSR1]	;   zOS_RFI(zOS_JOB);
	zOS_RFI	zOS_JOB
zos_sw2
	clrw			;  case zOS_END: zOS_PCH[fsr1] = 0;
	movwi	zOS_PCH[FSR1]	;   zOS_RFI(zOS_JOB); // killing is so quick
	zOS_RFI	zOS_JOB
zos_sw3
	moviw	zOS_HDL[FSR1]	;  case zOS_RST: zos_sw3:
	movwi	zOS_PCL[FSR1]	;   // retain HDL MSB (which indicate privilege)
	moviw	zOS_HDH[FSR1]	;   zOS_PCL[fsr1] = zOS_HDL[fsr1];
	andlw	0x7f		;   // clear PC MSB (which indicates sleepiness)
	movwi	zOS_PCH[FSR1]	;   zOS_PCH[fsr1] = zOS_HDH[fsr1] & 0x7f;
	movlw	zOS_BOS		;   zOS_SSP[fsr1] = zOS_BOS;
	movwi	zOS_SSP[FSR1]	;  case zOS_YLD: zOS_RFI(zOS_JOB);
zos_sw4

#ifdef zOS_MIN
zos_sw5
zos_sw6
zos_sw7
	zOS_RFI	zOS_JOB
#else
	zOS_RFI	zOS_JOB
zos_sw5
	;; copy job BSR's 0x20-0x6f into every non-running bank first
	clrf	FSR1L		;  case zOS_FRK:
	clrf	FSR1H		;   fsr1 = 1 << 7;
	clrf	zOS_JOB		;   for (zos_job = 1;
zos_cp1
	movlw	0x80		;        zos_job++ <= zOS_NUM; fsr1 += 0x80) {
	andwf	FSR1L,f		;    fsr1 &= 0xff80;
	addwf	FSR1L,f		;
	clrw			;
	addwfc	FSR1H,f		;    fsr1 += 0x80;     
	incf	zOS_JOB,f	;
	movlw	0xff-zOS_NUM	;
	addwf	zOS_JOB,w	;
	btfsc	STATUS,Z	;
	bra	zos_cpd		;        
	
	zOS_MEM	FSR0,BSR,0
	moviw	zOS_PCH[FSR0]	;    fsr0 = 0x10 * (1+BSR);
	btfss	STATUS,Z	;    if (zOS_PCH[fsr0] == 0)
	bra	zos_cp1		;     continue; // can't touch a running job
	
	rrf	BSR,w		;
	andlw	0x3		;
	movwf	FSR0H		;
	clrf	FSR0L		;
	rrf	FSR0L,f		;
	movlw	0x6f		;
	iorwf	FSR0L,f		;    fsr0 = (BSR << 7) | 0x6f;

	iorwf	FSR1L,f		;    for (fsr1 |= 0x6f; fsr1 & 0x7f >= 0x20;         
zos_cp2
	moviw	FSR0--		;
	movwi	FSR1--		;         *fsr1-- = *fsr0--)
	movlw	0x60		;
	andwf	FSR0L,w		;
	btfss	STATUS,Z	;
	bra	zos_cp2		;     ;
	bra	zos_cp1		;   }
zos_cpd
	;; now copy job BSR's bank0 struct to the zOS_AR registers and zOS_NEW()
	movf	BSR,w		;
	movwf	zOS_JOB		;   zOS_JOB = BSR;
	zOS_MEM	FSR1,zOS_JOB,0
	moviw	zOS_PCH[FSR1]	;
	btfsc	STATUS,Z	;
	bra	zos_sw4		;   if (zOS_PCH[fsr1]) {
	moviw	zOS_HDL[FSR1]	;
	movwf	FSR0L		;
	moviw	zOS_HDH[FSR1]	;
	movwf	FSR0H		;    fsr0 = (zOS_HDH[fsr1]<<8) | zOS_HDL[fsr1];
	moviw	zOS_ISR[FSR1]	;
	movwf	zOS_AR0		;    zOS_AR0 = zOS_ISR[fsr1];
	moviw	zOS_ISH[FSR1]	;
	movwf	zOS_AR1		;    zOS_AR1 = zOS_ISH[fsr1];
	moviw	zOS_HIM[FSR1]	;
	movwf	zOS_AR2		;    zOS_AR2 = zOS_HIM[fsr1];
	moviw	zOS_SIM[FSR1]	;
	movwf	zOS_AR3		;    zOS_AR3 = zOS_SIM[fsr1];
	banksel	WREG_SHAD
	clrf	WREG_SHAD	;    WREG_SHAD = zOS_NEW;
	movlb	0		;    goto zos_cre;//spoof privilege to fork self
	bra	zos_cre		;   }
zos_sw6
	movf	BSR,w		;  case zOS_EXE:
	movwf	zOS_JOB		;   zOS_JOB = BSR;
	zOS_MEM	FSR1,zOS_JOB,0
	banksel	WREG_SHAD	;   fsr1 = 0x10 * (1+zOS_JOB);
	clrf	WREG_SHAD	;   WREG_SHAD = zOS_NEW;
	movlb	0		;   //spoof privilege to overwrite
	bra	zos_dup		;   goto zos_dup;
zos_sw7
	movf	zOS_AR2,w	;  case zOS_FND:
	btfss	STATUS,Z	;
	movlw	zOS_NUM		;
	addlw	1		;
	movwf	zOS_JOB		;
	addlw	0xfe-zOS_NUM	;   if (zOS_AR2 && ((uint8_t)zOS_AR2<=zOS_NUM))
	btfss	WREG,7		;    zOS_JOB = zOS_AR2 + 1;
	movlw	1+zOS_NUM	;   else
	movwf	zOS_JOB		;    zOS_JOB = zOS_NUM + 1;
	zOS_MEM	FSR1,zOS_JOB,0	;   fsr1 = 0x10 * (1 + zOS_JOB);
zos_nxt
	zOS_LIV	FSR1,zOS_JOB,0,zos_bad
	moviw	zOS_HDL[FSR1]	;   while (zOS_LIV(&fsr0, &zOS_JOB, 0)) {
	xorwf	zOS_AR0,w	;
	btfss	STATUS,Z	;
	bra	zos_nxt		;    
	moviw	zOS_HDH[FSR1]	;    int16_t a=(zOS_AR1<<8)|zOS_AR0;
	xorwf	zOS_AR1,w	;    int16_t b=(zOS_HDH[fsr0]<<8)|zOS_HDL[fsr0];
	andlw	0x7f		;
	btfss	STATUS,Z	;    if (a & 0x7f == b & 0x7f)
	bra	zos_nxt		;     zOS_RFI (zOS_JOB);
	zOS_RFI	zOS_JOB		;   }
zos_bad
	clrw			;
	zOS_RFI	WREG		;   zOS_RFI(w = 0);
#endif
	
	;; else handle the software interrupt with the first registered handler
;;; FIXME: rewrite with zOS_LIV?
zos_swh
	movlw	zOS_NUM		;
	movwf	zOS_JOB		; zos_job = zOS_NUM;
	zOS_MEM	FSR0,zOS_JOB,0x10
zos_swl
	movlw	0-0x10	      	; for (fsr0 = 0x10*(1+zos_job);
	addwf	FSR0L,w		;
	movwf	FSR0L		;
	andlw	0x60		;      fsr0 > 0x10;
	btfsc	STATUS,Z	;
	bra	zos_swm		;      fsr0 -= 0x10) {
	decf	zOS_JOB,f	;  zos_job--;
	moviw	zOS_PCH[FSR0]	;
	btfsc	STATUS,Z	;  if (zOS_PCH[fsr0] == 0)
	bra	zos_swl		;   continue; // job killed or slot just empty
	moviw	zOS_SIM[FSR0]	;
	andwf	zOS_MSK,f	;
	btfsc	STATUS,Z	;
	bra	zos_swl		;  else if ((zos_msk &= zOS_SIM[fsr0]) != 0)
	moviw	zOS_ISH[FSR0]	;
	movwf	PCLATH		;   goto (void*)(zOS_ISR[fsr0]); // will zOS_RFI
	moviw	zOS_ISR[FSR0]	; }
	movwf	PCL		; zOS_RFI(WREG = 0);
	
	;; no registered SWI handler: jump into the hardware interrupt scheduler
zos_swm
	zOS_RFI	WREG

zos_ini
	;; clear out page 0 to reflect no running tasks, set global data to 0's
	movlb	0		; "invalid" job# used to get perms for zOS_NEW
	movlw	0x7f		; bsr = 0;
	movwf	FSR0L		;
	clrf	FSR0H		; for (fsr0 = 0x007f; fsr >= 0x0020; fsr--)
zos_zer
	clrw			;
	movwi	FSR0--		;  *fsr = 0; // only zOS_PCH is critical
	movlw	0x60		;
	andwf	FSR0L,w		;
	btfss	STATUS,Z	;
	bra	zos_zer		;
zos_end
	;; your program starts here, with a launcher job (#6) that ends after:
	;; 1) setting up oscillators, timers, other peripherals, etc.
	;;    (with the appropriate and ineviatable bank switching)
	;; 2) starting jobs with calls to zOS_NEW or its zOS_LAU wrapper
	;;    (being sure to stay in bank 0 or using job macros zOS_CON/zos_MON)
	;; 3) calling zOS_RUN (which will enable interrupts) to start job 1

