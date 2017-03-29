;;; olirelay.asm

;;; example program to control the Olimex PIC-IO relay/optoisolator board loaded
;;; with a PIC16F1847 microcontroller, the schematic for which may be found at
;;; olimex.com/Products/PIC/Development/PIC-IO/resources/PIC-IO_revision_C.pdf

	processor 16f1847
	include	p16f1847.inc

	__CONFIG _CONFIG1,_FOSC_HS & _WDTE_OFF & _PWRTE_OFF & _MCLRE_ON & _CP_OFF & _CPD_OFF & _BOREN_ON & _CLKOUTEN_OFF & _IESO_ON & _FCMEN_ON

;;; uncomment to reduce zOS footprint by 100 words (at cost of zOS_FRK/EXE/FND):
;zOS_MIN	equ	1
	
	include zos.inc
	include zosmacro.inc
	
;;; uncomment to pre-load stack positions with indices (for debugging xOS_ROL):
;zOS_DBG

	pagesel	main
	goto	main
	

#define PORT(x) ((PORTA & ~0x1f) | ((x)>>3))
w2port	macro
	andlw	0xf8		;
	xorlw	PORTA<<3	;inline uint8_t* w2port(uint8_t w) {
	movlw	low PORTA	; return ((PORTA<<3) & 0xf8) == (w & 0xf8)) ?
	btfss	STATUS,Z	; PORTA : PORTB;
	movlw	low PORTB	;}
	endm
	
#define BIT(x) (1 << ((x) & 0x07))
w2bit	macro
	andlw	0x07		;inline uint8_t w2bit(uint8_t w) {
	brw			;
	retlw	1<<0		;
	retlw	1<<1		;
	retlw	1<<2		;
	retlw	1<<3		;
	retlw	1<<4		;
	retlw	1<<5		;
	retlw	1<<6		; return 1 << (w & 0x07);
	retlw	1<<7		;}
	endm

PORT1	equ	PORTA<<3
OPTO1	equ	RA4
PORT2	equ	PORTB<<3	
OPTO2	equ	RB0
PORT3	equ	PORTB<<3
OPTO3	equ	RB3	
PORT4	equ	PORTB<<3
OPTO4	equ	RB4
myopto1
	addlw	0-1		;uint8_t myopto1(uint8_t w) { switch (w) {
myopto
	andlw	0x03		; case 1: return (PORTA<<3) | RA4;
	brw			; case 2: return (PORTB<<3) | RB0;
	retlw	PORT1|OPTO1	; case 3: return (PORTB<<3) | RB3;
	retlw	PORT2|OPTO2	; case 4: return (PORTB<<3) | RB4;
	retlw	PORT3|OPTO3	; } // undefined for w < 1 or w > 4
	retlw	PORT4|OPTO4	;}

RELAY1	equ	RA3
RELAY2	equ	RA2
RELAY3	equ	RA1
RELAY4	equ	RA0
myrelay1
	addlw	0-1		;uint8_t myrelay1(uint8_t w) { switch (w) {
myrelay
	andlw	0x03		; case 1: return (PORTA<<3) | RA3;
	brw			; case 2: return (PORTA<<3) | RA2;
	retlw	PORTA|RELAY1	; case 3: return (PORTA<<3) | RA1;
	retlw	PORTA|RELAY2	; case 4: return (PORTA<<3) | RA0;
	retlw	PORTA|RELAY3	; } // undefined for w < 1 or w > 4
	retlw	PORTA|RELAY4	;}

mychan1
	addlw	0-1		;uint8_t mychan1() { switch (w) {
mychan
	andlw	0x03		; case 1: return 1<<3;
	brw			; case 2: return 1<<2;
	retlw	0x08		; case 3: return 1<<1;
	retlw	0x04		; case 4: return 1<<0;
	retlw	0x02		; } // undefined for w < 1 or w > 4
	retlw	0x01		;}
	

rel_isr
	

relay
RELAYID	equ	0x20
OPTOID	equ	0x21
RELAYP	equ	0x22
OPTOP	equ	x023
RELAYB	equ	0x24
OPTOB	equ	0x25
MYMASK	equ	0x26
	zOS_MY2	FSR0
	decf	zOS_ME		;void relay(void) { // 1<= bsr (job#) <= 4
	pagesel	myrelay	
	call	myrelay		; uint8_t* fsr0 = 0x70 | (bsr << 1);
	movwf	RELAYID		; static uint8_t relayid = myrelay1(bsr);
	
	w2port
	movwf	RELAYP		; static uint8_t relayp = w2port(relayid);
	movf	RELAYID,w	;
	w2bit
	movwf	RELAYB		; static uint8_t relayb = w2bit(relayid);
	
	decf	zOS_ME		;
	pagesel	myopto
	call	myopto		;
	movwf	OPTOID		; static uint8_t optoid = myopto1(bsr);

	w2port
	movwf	OPTOP		; static uint8_t optop = w2port(optoid);
	movf	OPTOID,w	;
	w2bit
	movwf	OPTOB		; static uint8_t optob = w2bit(optoid);

	decf	zOS_ME		;
	w2chan
	movwf	MYMASK		; static uint8_t mymask = w2chan1(bsr);
	
	movf	RELAYP		;
	movwf	FSR1L		; uint8_t* fsr1;
	movlw	high PORTA	;
	movwf	FSR1H		; fsr1 = (relayp==PORTA&0xff) ? &PORTA : &PORTB;
relaylp
	movf	MYMASK,w	; do {
	andwf	INDF0,w		;  if (*fsr0 & mymask)
	pagesel	relay1
	btfsc	STATUS,Z	;   relay1();
	call	relay1		;  else
	pagesel	relay0
	btfsc	STATUS,Z	;   relay0();
	call	relay0		;  zOS_SWI(zOS_YLD); // let another job run
relaysl
	zOS_SWI	zOS_YLD		; } while (1);
	bra	relaylp		;}
relay1
	movf	RELAYB,w
	iorwf	INDF1,f
	return
relay0
	comf	RELAYB,w
	andwf	INDF1,f
	return
	

OUTCHAR	equ	zOS_SI3
main
	zOS_INT	1<<IOCIF,0	;void main(void) {
	zOS_ADR	relay,zOS_UNP	; zOS_INT(1<<IOCIF, 0); // HWI only, watches IOC
	zOS_LAU	WREG		; zOS_ADR(relay, zOS_UNP);
	zOS_LAU	WREG		;
	zOS_LAU	WREG		; for (int job = 1; job <= 4; job++)
	zOS_LAU	WREG		;  zOS_LAU(&w);
	sublw	zOS_NUM-1	;
	btfsc	WREG,7		; if (w >= zOS_NUM) // job remaining for zOS_MON
	reset			;  reset();

	zOS_CON	1,20000000/9600,PIR1,PORTB,RB5
	movlw	OUTCHAR		; zOS_MON(/*SSP*/1,20MHz/9600bps,PIR1,PORTB,5);
	zOS_ARG	3		; zOS_ARG(3,OUTCHAR/*only 1 SWI*/);
	zOS_LAU	WREG		; zOS_LAU(&w);
	
	;;don't forget to set up Interrupt-On-Change here

	banksel	ANSELB
	bcf	ANSELB,RB5	; ANSELB &= ~(1<<RB5); // allow digital function
	banksel	TRISB
	bcf	TRISB,RB5	; TRISB &= ~(1<<RB5); // allow output heartbeat
	
	banksel	OPTION_REG
	bcf	OPTION_REG,T0CS	; OPTION_REG &= ~(1<<TMR0CS);// off Fosc not pin
	bcf	OPTION_REG,PSA	; OPTION_REG &= ~(1<<TMR0CS);// use max prescale

	zOS_RUN	INTCON,INTCON	; zOS_RUN(/*T0IE in*/INTCON, /*T0IF in*/INTCON);
	end			;}
