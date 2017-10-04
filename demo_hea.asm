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
	
MAXSRAM	equ	0x2400
SMALLOC	equ	zOS_SI4
SFREE	equ	zOS_SI5
	
	include zos.inc
	include zosmacro.inc
	include	zosalloc.asm

	pagesel main
	goto	main

myprog
i	equ	0x20
smalls	equ	0x21
larges	equ	0x24
	
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
