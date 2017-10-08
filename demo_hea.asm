;;; demo_hea.asm
;;;
;;; demonstration app for zOS running two heap allocators launched by zOS_HEA
;;; to build: gpasm -D GPASM demo_zos.asm
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
;;; since only 4 of 5 possible task slots are used in this demo reducing the max
;;; allowed value by 1 will make scheduler run faster as well as freeing an extra
;;; 80 bytes for the heap itself:
zOS_NUM	equ	4

	processor 16f1847
	include p16f1847.inc
	
	__CONFIG _CONFIG1,_FOSC_HS & _WDTE_OFF & _PWRTE_OFF & _MCLRE_ON & _CP_OFF & _CPD_OFF & _BOREN_ON & _CLKOUTEN_OFF & _IESO_ON & _FCMEN_ON

;;; uncomment to reduce zOS footprint by 100 words (at cost of zOS_FRK/EXE/FND):
;zOS_MIN	equ	1
	
	include zos.inc
	include zosmacro.inc
	
MAXSRAM	equ	0x2400
SMALLOC	equ	zOS_SI4
SFREE	equ	zOS_SI5
LMALLOC	equ	zOS_SI6
LFREE	equ	zOS_SI7
	include	zosalloc.asm

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
	zOS_SWI	zOS_YLD
	movf	temp,w		;  zOS_ARG(0, 2); // 16 bytes from bank 0, 2 ptr
	bra	newnode		;  if ((w = zOS_SWI(SMALLOC)) == 0)
nncopy
	zOS_PTR	FSR1
	movf	FSR0H,w		;   zOS_SWI(zOS_YLD); // hope coalescing happens
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
	btfss	STATUS,Z	;
	return			;
	pagesel	newnode		;
	call	newnode		;  // fsr1 will become new head, may need moving
	decfsz	i,f		;
	btfss	WREG,7		;
	bra	makloop		;  if (newnode(i) < 0) { // head is out of order
srtloop
	movf	FSR0L,w		;
	movwf	insert		;
	movf	FSR0H,w		;
	movwf	inserth		;   insert = fsr0;
	
	moviw	NEXT[FSR0]	;
	movwf	temp		;
	moviw	NEXTHI[FSR0]	;
	btfsc	STATUS,Z	;
	bra	linsert		;   while (fsr0->next) {
	movwf	FSR0H		;
	movf	temp,w		;
	movwf	FSR0L		;    fsr0 = fsr0->next;
	
	moviw	zOS_HDH[FSR0]	;
	andlw	0x7f		;
	movwf	temp		;
	moviw	zOS_HDH[FSR1]	;
	andlw	0x7f		;
	subwf	temp,w		;    w = 0x7f&(HDH[*fsr0]) - 0x7f&(HDH[*fsr1]);
	btfss	WREG,7		;    if (w < 0)
	bra	rewind

	

rewind
	movf	insert,w	;
	movwf	FSR0L		;
	movf	inserth,w	;
	movwf	FSR0H		;
	
;;; we get here when fsr0's successor (as the first payload >= fsr1's payload)
;;; needs to become fsr1's successor, and the node at fsr0 will point to fsr1
;;; (being careful not to lose a pointer fsr1->next as the new list head node)
	
linsert

	
;;; we get here if the node at fsr1 is the largest in the list, so goes at end

	
myprog
	zOS_SWI	zOS_YLD		;void myprog(void) {
	zOS_LOC	FSR1,BSR,larges	; uint8_t i, smalls[3], larges[3];
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
	bra	gettiny		;    w = zOS_SWI(32 >> 4);
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
	banksel	ANSELB
	bcf	ANSELB,RB5	; ANSELB &= ~(1<<RB5); // allow digital function

	banksel	TRISB
	bcf	TRISB,RB5	; TRISB &= ~(1<<RB5); // allow output heartbeat
	
	banksel	OPTION_REG
	bcf	OPTION_REG,PSA	; OPTION_REG &= ~(1<<PSA);// max timer0 prescale
	bcf	OPTION_REG,T0CS	; OPTION_REG &= ~(1<<TMR0CS);// off Fosc not pin

#if 0
OUTCHAR	equ	zOS_SI3
;	zOS_MAN	0,20000000/9600,PIR1,PORTB,RB5
	zOS_CON	0,20000000/9600,PIR1,PORTB,RB5
	movlw	OUTCHAR		;
	zOS_ARG	3		; zOS_CON(/*UART*/1,20MHz/9600bps,PIR1,PORTB,5);
#else	
	zOS_NUL	1<<T0IF
#endif
	zOS_LAU	WREG		; zOS_ARG(3,OUTCHAR/*only 1 SWI*/); zOS_LAU(&w);

	zOS_INT	0,0
	zOS_ADR	myprog,zOS_UNP
	zOS_LAU	WREG
	
	zOS_RUN	INTCON,INTCON
	end
