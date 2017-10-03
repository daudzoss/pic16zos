;;; demo_zos.asm
;;;
;;; demonstration app for zOS running two heap allocators
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
;;; allowed value by 1 will make scheduler run faster:
zOS_NUM	equ	4

	processor 16f1847
	include p16f1847.inc
	
	__CONFIG _CONFIG1,_FOSC_HS & _WDTE_OFF & _PWRTE_OFF & _MCLRE_ON & _CP_OFF & _CPD_OFF & _BOREN_ON & _CLKOUTEN_OFF & _IESO_ON & _FCMEN_ON

;;; uncomment to reduce zOS footprint by 100 words (at cost of zOS_FRK/EXE/FND):
;zOS_MIN	equ	1
	
	include zos.inc
	include zosmacro.inc

OUTCHAR	equ	zOS_SI3
#if 0
LMALLOC	equ	zOS_SI6
L_FREE	equ	zOS_SI7
#else
LMALLOC	equ	zOS_SI4
L_FREE	equ	zOS_SI5	
#endif
SMALLOC	equ	zOS_SI6
S_FREE	equ	zOS_SI7

MAXSRAM	equ	0x2400
HEAPRAM	equ	MAXSRAM-zOS_FRE
HEAPSIZ	equ	HEAPRAM/2
HEAP1	equ	zOS_FRE
HEAP2	equ	zOS_FRE+HEAPSIZ

	pagesel main
	goto	main

i	equ	0x20
smalls	equ	0x21
larges	equ	0x24
	
myprog
	zOS_LOC	FSR1,BSR,larges	;void myprog(void) {
	zOS_LOC	FSR0,BSR,smalls	; uint8_t larges[3], smalls[3];
	movlw	0x03		; while (1) {  // grab three 128-byte cells
	movwf	i		;  uint8_t* fsr1 = larges; 
getbig
	movlw	0x08		;  uint8_t* fsr0 = smalls;
	zOS_ARG	0
	zOS_SWI LMALLOC
	movf	WREG		;  for (i = 3; i; i--) {
	btfsc	STATUS,Z	;   zOS_ARG(0,3 /*units*/);
	bra	getbig		;   do {
	movwi	FSR1++		;    w = zOS_SWI(LMALLOC);
	decfsz	i,f		;   } while (!w); // eventually will fail
	bra	getbig		;   *fsr1++ = w;

	movlw	0x03		;  }
	movwf	i		;  // grab three 32-byte cells
gettiny
	movlw	0x02		;  for (i = 3; i; i--) {
	zOS_ARG	0
	zOS_SWI	SMALLOC		;
	movf	WREG		;   zOS_ARG(0,3 /*units*/);
	btfsc	STATUS,Z	;   do {
	bra	gettiny		;    w = zOS_SWI(SMALLOC);
	movwi	FSR0++		;   } while (!w);
	decfsz	i,f		;   *fsr0++ = w;
	bra	gettiny		;  }

	moviw	-3[FSR0]	;  // free first two 32-byte cells
	zOS_ARG	0		;  zOS_ARG(0,-3[fsr0]);
	zOS_SWI	S_FREE		;  zOS_SWI(S_FREE);

	moviw	-2[FSR0]	;
	zOS_ARG	0		;  zOS_ARG(0,-2[fsr0]);
	zOS_SWI	S_FREE		;  zOS_SWI(S_FREE);
	
	moviw	-3[FSR1]	;  // free first two 128-byte cells
	zOS_ARG	0		;  zOS_ARG(0,-3[fsr1]);
	zOS_SWI	L_FREE		;  zOS_SWI(S_FREE);

	moviw	-2[FSR1]	;  zOS_ARG(0,-2[fsr1]);
	zOS_ARG	0		;  zOS_SWI(S_FREE);
	zOS_SWI	L_FREE		; }
	bra	myprog		;}
	
main
	banksel	ANSELB
	bcf	ANSELB,RB5	; ANSELB &= ~(1<<RB5); // allow digital function

	banksel	TRISB
	bcf	TRISB,RB5	; TRISB &= ~(1<<RB5); // allow output heartbeat
	
	banksel	OPTION_REG
	bcf	OPTION_REG,PSA	; OPTION_REG &= ~(1<<PSA);// max timer0 prescale
	bcf	OPTION_REG,T0CS	; OPTION_REG &= ~(1<<TMR0CS);// off Fosc not pin

;	zOS_MAN	0,20000000/9600,PIR1,PORTB,RB5
;	zOS_CON	0,20000000/9600,PIR1,PORTB,RB5
;	movlw	OUTCHAR		;void main(void) {
;	zOS_ARG	3		; zOS_CON(/*UART*/1,20MHz/9600bps,PIR1,PORTB,5);
	zOS_NUL	1<<T0IF
	zOS_LAU	WREG		; zOS_ARG(3,OUTCHAR/*only 1 SWI*/); zOS_LAU(&w);

	zOS_HEA	HEAP1,HEAPSIZ,LMALLOC,L_FREE
	movlw	LMALLOC|L_FREE
	zOS_ARG	3
	zOS_LAU	WREG

#if 0
	zOS_HEA	HEAP2,HEAPSIZ,SMALLOC,S_FREE
	movlw	SMALLOC|S_FREE
	zOS_ARG	3
	zOS_LAU	WREG
#endif

	zOS_INT	0,0
	zOS_ADR	myprog,zOS_UNP
	zOS_LAU	WREG
	
	zOS_RUN	INTCON,INTCON
	end
