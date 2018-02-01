;;; demo_hea.asm
;;;
;;; demonstration app for zOS running two heap allocators launched by zOS_HEA
;;; to build: gpasm -D GPASM demo_hea.asm
;;;
;;; after starting job #1 as a job management shell (zOS_MAN() in zosmacro.inc)
;;; to demonstrate privileged mode (able to kill or otherwise tweak other tasks)
;;; 
;;; it starts two instances of memory allocators as jobs #2 and 3, one for Large
;;; blocks of memory and one for Small (a distinction which is arbitrary but it
;;; helps to minimize fragmentation
;;; 
;;; it then starts a job #4 to start making malloc() and free() calls in order
;;; to observet the action of the help allocators
;;; 
;;; if only 4 of 5 possible task slots are used in this demo reducing the max
;;; allowed value by 1 will make scheduler run faster as well as freeing an extra
;;; 80 bytes for the heap itself:
;zOS_NUM	equ	4

	processor 16f1719
	include p16f1719.inc
	
	__CONFIG _CONFIG1,_FOSC_INTOSC & _WDTE_OFF & _PWRTE_OFF & _CP_OFF & _BOREN_ON & _CLKOUTEN_ON & _IESO_ON & _FCMEN_ON
	__CONFIG _CONFIG2,_WRT_OFF & _PPS1WAY_OFF & _ZCDDIS_ON & _PLLEN_ON & _STVREN_ON & _BORV_LO & _LPBOR_OFF & _LVP_ON
	
;;; uncomment to reduce zOS footprint by 100 words (at cost of zOS_FRK/EXE/FND):
;zOS_MIN	equ	1
	
	include zos.inc
	include zosmacro.inc
	
OUTCHAR	equ	zOS_SI3
	
SMALLOC	equ	zOS_SI4
SFREE	equ	zOS_SI5
LMALLOC	equ	zOS_SI6
LFREE	equ	zOS_SI7
MAXSRAM	equ	0x2400

	pagesel main
	goto	main

NEXT	equ	0x10
NEXTHI	equ	0x11
	
i	equ	0x20
smalls	equ	0x21
larges	equ	0x24
temp	equ	0x25
insert	equ	0x26
inserth	equ	0x27
	
newnode
	movwf	temp		;uint8_t* newnode(void* *fsr0, // previous head
	movlw	2		;                 void* *fsr1, uint8_t w) {
	zOS_ARG	0
	zOS_SWI	SMALLOC
	movf	WREG		; uint8_t temp = w; // job number to copy struct
	btfss	STATUS,Z	;
	bra	nncopy		; do {
	movf	zOS_ME		;  zOS_ARG(0, 2); // 16 bytes from bank 0, 2 ptr
	zOS_ARG	0
	zOS_SWI	zOS_YLD
	movf	temp,w		;  if ((w = zOS_SWI(SMALLOC)) == 0) {
	bra	newnode		;   zOS_ARG(0, bsr);
nncopy
	zOS_PTR	FSR1
	movf	FSR0H,w		;   zOS_SWI(zOS_YLD);}// hope coalescing happens
	movwi	NEXTHI[FSR1]	; } while (w == 0);
	movf	FSR0L,w		; *fsr1 = zOS_PTR(w);
	movwi	NEXT[FSR1]	; w = temp;

	movf	temp,w		; (*fsr1)->next = *fsr0;
	zOS_MEM	FSR0,WREG,0x10
	addfsr	FSR1,0x10	; zOS_MEM(fsr0,w,0x10); // 0x30, 0x40, ..., 0x70
nnloop
	moviw	--FSR0		; (*fsr1) += 0x10; 
	movwi	--FSR1		; for (int j = 0; j < 16; j++)
	movf	FSR0L,w		;
	andlw	0x0f		;
	btfss	STATUS,Z	;
	bra	nnloop		;  *--(*fsr1) = *--(*fsr0);
	
	moviw	NEXT[FSR1]	;
	movwf	FSR0L		;
	moviw	NEXTHI[FSR1]	; *fsr0 = (*fsr1)->next;
	movwf	FSR0H		; // now fsr1 is new head, fsr0 is tail=old head
	
	moviw	zOS_HDH[FSR1]	;
	btfsc	STATUS,Z	;
	bra	discard		; if (zOS_HDH[*fsr1]) {// head valid running job
	movf	FSR0H,f		;  // compare the handles for the head and tail
	btfsc	STATUS,Z	;  if (0xff00 & *fsr0 == 0)
	retlw	0		;   return 0; // null tail, so in order by def'n
	andlw	0x7f		;
	movwf	temp		;
	moviw	zOS_HDH[FSR0]	;
	andlw	0x7f		;
	subwf	temp,w		;  w = 0x7f&(HDH[*fsr1]) - 0x7f&(HDH[*fsr0]);
	btfss	STATUS,Z	;  if ((**fsr1 & 0x7f00) != (**fsr0 & 0x7f00))
	return			;   return w;//>0 if in correct order, <0 if out
	
	moviw	zOS_HDL[FSR1]	;
	movwf	temp		;
	moviw	zOS_HDL[FSR0]	;  w = 0x7f&(HDL[*fsr1]) - 0x7f&(HDL[*fsr0]);
	subwf	temp,w		;  return w;//>=0 if in correct order, <0 if out
	return			; } else {
discard
	zOS_PAG	FSR1		;  zOS_ARG(0, zOS_PAG(*fsr1));
	zOS_ARG	0		;  zOS_SWI(SFREE); // free the node back to heap
	zOS_SWI	SFREE		;  return (*fsr1 &= 0x00ff) >> 8;
	clrf	FSR1H		; }
	retlw	0		;} // newnode()

maklist
	clrf	FSR1H		;void maklist(void) {
	movlw	zOS_NUM		; fsr1 = (void*) 0;
	movwf	i		; for (uint8_t i = zOS_NUM; i; i--) {
makloop
	movf	FSR1L,w		;
	movwf	FSR0L		;
	movf	FSR1H,w		;
	movwf	FSR0H		;  fsr0 = fsr1; // fsr0 is head of list
	movf	i,w		;
	btfsc	STATUS,Z	;
	return			;
	pagesel	newnode		;
	call	newnode		;  // fsr1 will become new head, may need moving
	decfsz	i,f		;
	btfss	WREG,7		;
	bra	makloop		;  if (newnode(&fsr0/*tail*/, &fsr1/*head*/, i)
srtloop
	movf	FSR0L,w		;                 < 0) { // head is out of order
	movwf	insert		;
	movf	FSR0H,w		;
	movwf	inserth		;   insert = fsr0;
	
	moviw	NEXT[FSR0]	;
	movwf	temp		;
	moviw	NEXTHI[FSR0]	;
	btfsc	STATUS,Z	;
	bra	linsert		;   while (fsr0->next) { // march fsr0 down list
	movwf	FSR0H		;
	movf	temp,w		;
	movwf	FSR0L		;    fsr0 = fsr0->next;
	
	moviw	zOS_HDH[FSR0]	;
	andlw	0x7f		;
	movwf	temp		;
	moviw	zOS_HDH[FSR1]	;
	andlw	0x7f		;
	subwf	temp,w		;    w = 0x7f&(HDH[*fsr0]) - 0x7f&(HDH[*fsr1]);
	
	btfss	WREG,7		;    if (w < 0) // even latest node too small so
	btfsc	STATUS,Z	;     continue;
	bra	srtloop		;    else if (w > 0)
	bra	rewind		;     break;
	
	moviw	zOS_HDL[FSR0]	;
	andlw	0x7f		;
	movwf	temp		;
	moviw	zOS_HDL[FSR1]	;
	andlw	0x7f		;
	subwf	temp,w		;    w = 0x7f&(HDL[*fsr0]) - 0x7f&(HDL[*fsr1]);

	btfsc	WREG,7		;    if (w < 0) // even latest node too small so
	bra	srtloop		;     continue; // haven't found; next iteration
rewind
	movf	insert,w	;   
	movwf	FSR0L		;    fsr0 = insert; // found one, roll back fsr0
	movf	inserth,w	;    break;
	movwf	FSR0H		;   }
	
;;; we get here when fsr0's successor (as the first payload >= fsr1's payload)
;;; needs to become fsr1's successor, and the node at fsr0 will point to fsr1
;;; (being careful not to lose a pointer fsr1->next as the new list head node)
	
linsert
	moviw	NEXT[FSR1]	;
	movwf	insert		;
	moviw	NEXTHI[FSR1]	;   // save head of list so we don't lose it
	movwf	inserth		;   insert = fsr1->next;

	moviw	NEXT[FSR0]	;
	movwi	NEXT[FSR1]	;
	moviw	NEXTHI[FSR0]	;
	movwi	NEXTHI[FSR1]	;   fsr1->next = fsr0->next;

	movf	FSR1L,w		;
	movwi	NEXT[FSR0]	;
	movf	FSR1H,w		;
	movwi	NEXTHI[FSR0]	;   fsr0->next = fsr1;
	
	movf	insert,w	;  } 
	movwf	FSR0L		; }
	movf	inserth,w	; return fsr0 = insert; // return new head
	movwf	FSR0H		;}
	
	zOS_NAM	"heap-churning loop"
myprog
	movf	zOS_ME		;void myprog(void) {
	zOS_ARG	0
	zOS_SWI	zOS_YLD		; uint8_t i, smalls[3], larges[3];
	pagesel	maklist
	call	maklist		;
	zOS_LOC	FSR1,BSR,larges	; zOS_ARG(0, bsr);
	zOS_LOC	FSR0,BSR,smalls	; zOS_SWI(zOS_YLD); // let malloc(),free() init
	movlw	0x03		; while (1) {
	movwf	i		;  uint8_t* fsr1 = larges; 
getbig
	movlw	0x08		;  uint8_t* fsr0 = smalls;
	call	malloc		;
	movf	WREG		;  // grab three 128-byte cells
	btfsc	STATUS,Z	;  for (i = 3; i; i--) {
	bra	getbig		;   do {
	movwi	FSR1++		;    w = malloc(128 >> 4);
	decfsz	i,f		;   } while (!w); // eventually will fail
	bra	getbig		;   *fsr1++ = w;
	movlw	0x03		;  }
	movwf	i		;
gettiny
	movlw	0x02		;
	call	malloc		;  // grab three 32-byte cells
	movf	WREG		;  for (i = 3; i; i--) {
	btfsc	STATUS,Z	;   do {
	bra	gettiny		;    w = malloc(32 >> 4);
	movwi	FSR0++		;   } while (!w);
	decfsz	i,f		;   *fsr0++ = w;
	bra	gettiny		;  }

	moviw	-3[FSR0]	;  // free first two 32-byte cells
	call	free		;  free(-3[fsr0]);

	moviw	-2[FSR0]	;
	call	free		;  free(-2[fsr0]);
	
	moviw	-3[FSR1]	;  // free first two 128-byte cells
	call	free		;  free(-3[fsr1]);

	moviw	-2[FSR1]	;  free(-2[fsr1]);
	call	free		; }
	bra	myprog		;}
	
main
	banksel OSCCON		;{
	movlw	0x70		;    // SCS FOSC; SPLLEN disabled; IRCF 8MHz_HF; 
	movwf	OSCCON		;    OSCCON = 0x70;
	movlw	0x80		;    // SOSCR enabled; 
	movwf	OSCSTAT		;    OSCSTAT = 0x80;
	movlw	0x00		;    // TUN 0; 
	movwf	OSCTUNE		;    OSCTUNE = 0x00;
				;    // Wait for PLL to stabilize
	btfss	OSCSTAT,PLLR	;    while(PLLR == 0)
	bra	$-1		;     ;
	
	banksel	ANSELA
	movlw	0xaf		;
	movwf	ANSELA		; ANSELA = 0xaf; // allow heartbeat GPIO, CLKOUT
	movlw	0x3c		;
	movwf	ANSELC		; ANSELC = 0x3c; // allow serial port

	banksel	OPTION_REG
	bcf	OPTION_REG,PSA	; OPTION_REG &= ~(1<<PSA);// max timer0 prescale
	bcf	OPTION_REG,T0CS	; OPTION_REG &= ~(1<<TMR0CS);// off Fosc not pin

	banksel	TRISC
	bcf	TRISA,RA4	; TRISA &= ~(1<<RA4); // allow heartbeat output
;	bcf	TRISA,RA6	; TRISA &= ~(1<<RA6); // allow clock output
	movlw	0x7f
	movwf	TRISC

	banksel	PPSLOCK
	movlw	0x55
	movwf	PPSLOCK
	movlw	0xaa
	movwf	PPSLOCK
	bcf	PPSLOCK,PPSLOCKED
	movlw	0x16
	movwf	RXPPS
	
	banksel	RC7PPS
	movlw	0x14
	movwf	RC7PPS
	movlw	0x55
	movwf	PPSLOCK
	movlw	0xaa
	movwf	PPSLOCK
	bsf	PPSLOCK,PPSLOCKED

;	zOS_MAN	0,.032000000/.000009600,PIR1,LATA,RA4,0
	zOS_CLC	0,.032000000/.000009600,PIR1,LATA,RA4,0
	movlw	OUTCHAR		;
	movwi	0[FSR0]		; zOS_CLC(/*TX*/0,32MHz/9600bps,PIR1,LATA,RA4);

	include	zosalloc.inc
	
;	zOS_INT	0,0
;	zOS_ADR	myprog,zOS_UNP
;	zOS_LAU	WREG
	
	zOS_RUN	INTCON,INTCON
	end
