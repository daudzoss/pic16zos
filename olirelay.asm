;;; olirelay.asm

	processor 16f1847
	include	p16f1847.inc

	__CONFIG _CONFIG1,_FOSC_HS & _WDTE_OFF & _PWRTE_OFF & _MCLRE_ON & _CP_OFF & _CPD_OFF & _BOREN_ON & _CLKOUTEN_OFF & _IESO_ON & _FCMEN_ON
	
;;; example program to control the Olimex PIC-IO relay/optoisolator board loaded
;;; with a PIC16F1847 microcontroller, the schematic for which may be found at
;;; olimex.com/Products/PIC/Development/PIC-IO/resources/PIC-IO_revision_C.pdf
;;;            __________ __________
;;;           |          U          |
;;;      OUT2_|  1 (RA2)   (RA1) 18 |_OUT3
;;;           |                     |
;;;      OUT1_|  2 (RA3)   (RA0) 17 |_OUT4
;;;           |                     |
;;;       IN1_|  3 (RA4)   (RA7) 16 |_OSC1
;;;           |                     |     20MHz xtal
;;;     /MCLR_|  4 (RA5)   (RA6) 15 |_OSC2
;;;           |                     |
;;;       GND_|  5               14 |_VDD
;;;           |                     |
;;;       IN2_|  6 (RB0)   (RB7) 13 |_PGD (ICSP pin 4)
;;;           |                     |
;;; TXH = RXD_|  7 (RB1)   (RB6) 12 |_PGC (ICSP pin 5)
;;;           |                     |
;;; RXH = TXD_|  8 (RB2)   (RB5) 11 |_HBEAT LED (on timer 0)
;;;           |                     |
;;;       IN3_|  9 (RB3)   (RB4) 10 |_IN4 (ICSP pin 6)
;;;           |_____________________|

PORT1	equ	PORTA<<3
OPTO1	equ	RA4
PORT2	equ	PORTB<<3	
OPTO2	equ	RB0
PORT3	equ	PORTB<<3
OPTO3	equ	RB3	
PORT4	equ	PORTB<<3
OPTO4	equ	RB4
	
RPORT	equ	PORTA<<3
RELAY1	equ	RA3
RELAY2	equ	RA2
RELAY3	equ	RA1
RELAY4	equ	RA0
	
;;; this board uses an 18-pin PIC with an external crystal to watch four opto-
;;; isolators and drive four relays; running this example zOS application each
;;; input/output pair (numbered 1 to 4, coinciding with its job) runs in its own
;;; copy of the relay() re-entrant function and its re-entrant IRS counterpart
;;; optoisr() to reflect respectively the commanded output state from its odd-
;;; numbered global to the relay and input state from the optoisolator into the
;;; even-numbered global:
RLY1OUT	equ	0x72
OPT1IN	equ	0x73
RLY2OUT	equ	0x74
OPT2IN	equ	0x75
RLY3OUT	equ	0x76
OPT3IN	equ	0x77
RLY4OUT	equ	0x78
OPT4IN	equ	0x79
ALL_IOC	equ	0x7a		; logical OR of all IOC flags to watch rise/fall
TMP_IOC	equ	0x7b		; scratch var (globals for init loop then job 5)
	
;;; the fifth available job is intended to be the monitor application with which
;;; the board can be controlled directly, replaced with a custom application via
;;; the zOS_EXE system call, or for killing relay tasks that are not used and
;;; thus freeing space


;;; uncomment to reduce zOS footprint by 100 words (at cost of zOS_FRK/EXE/FND):
;zOS_MIN	equ	1
	
	include zos.inc
	include zosmacro.inc
	
;;; uncomment to pre-load stack positions with indices (for debugging xOS_ROL):
;
 zOS_DBG

;; software interrupt lines used: SI3 to print chars to console, SI4 for RA4 IOC
OUTCHAR	equ	zOS_SI3
NON_IOC	equ	zOS_SI4

	pagesel	main
	goto	main
	
input2w	macro
	movf	OPT1IN,w	;inline uint8_t input2w() { // AND of all inputs
	andwf	OPT2IN,w	; // since an all-zero register means task unrun
	andwf	OPT3IN,w	; return OPT1IN & OPT2IN & OPT3IN & OPT4IN;     
	andwf	OPT4IN,w	;}
	endm

w2port	macro
	andlw	0xf8		;inline uint8_t* w2port(uint8_t w) {
	xorlw	PORTA<<3	; return ((w & 0xf8) == ((PORTA<<3) & 0xf8)) ?
	movlw	low PORTA	;        PORTA :
	btfss	STATUS,Z	;        PORTB;
	movlw	low PORTB	;}
	endm
	
w2bit	macro	file
	andlw	0x07		;inline uint8_t w2bit(uint8_t* file) {
	bsf	STATUS,C	;
	clrf	file		;
	brw			; return 1 << (*file & 0x07);
	rrf	file,f		;
	rrf	file,f		;
	rrf	file,f		;
	rrf	file,f		;
	rrf	file,f		;
	rrf	file,f		;
	rrf	file,f		;
	rrf	file,f		;}
	endm

myopto1
	addlw	0-1		;uint8_t myopto1(uint8_t w) { switch (w) {
myopto
	andlw	0x03		; case 1: return (PORTA<<3) | RA4;
	brw			; case 2: return (PORTB<<3) | RB0;
	retlw	PORT1|OPTO1	; case 3: return (PORTB<<3) | RB3;
	retlw	PORT2|OPTO2	; case 4: return (PORTB<<3) | RB4;
	retlw	PORT3|OPTO3	; } // undefined for w < 1 or w > 4
	retlw	PORT4|OPTO4	;}

myrelay1
	addlw	0-1		;uint8_t myrelay1(uint8_t w) { switch (w) {
myrelay
	andlw	0x03		; case 1: return (PORTA<<3) | RA3;
	brw			; case 2: return (PORTA<<3) | RA2;
	retlw	RPORT|RELAY1	; case 3: return (PORTA<<3) | RA1;
	retlw	RPORT|RELAY2	; case 4: return (PORTA<<3) | RA0;
	retlw	RPORT|RELAY3	; } // undefined for w < 1 or w > 4
	retlw	RPORT|RELAY4	;}

mychan1
	addlw	0-1		;uint8_t mychan1() { switch (w) {
mychan
	andlw	0x03		; case 1: return 1<<3;
	brw			; case 2: return 1<<2;
	retlw	0x08		; case 3: return 1<<1;
	retlw	0x04		; case 4: return 1<<0;
	retlw	0x02		; } // undefined for w < 1 or w > 4
	retlw	0x01		;}
	

RELAYID	equ	0x20
OPTOID	equ	0x21
RELAYP	equ	0x22
OPTOP	equ	0x23
RELAYB	equ	0x24
OPTOB	equ	0x25
OPTOCUR	equ	0x26
OPTOLST	equ	0x27
MYMASK	equ	0x28
SAID_HI	equ	0x29
	
optoisr
	zOS_MY2	FSR0
	movf	RELAYP,w	;__isr void optoisr(uint8_t zOS_JOB) {
	movwf	FSR1L		; uint8_t fsr0 = 0x70 | (bsr<<1); // out,0xff&in
	movlw	high PORTA	; uint8_t fsr1;
	movwf	FSR1H		; fsr1 = (relayp==PORTA&0xff) ? &PORTA : &PORTB;
	movf	zOS_JOB,w	;
	movwf	BSR		; bsr = zOS_JOB;
	moviw	1[FSR0]		;
	btfss	STATUS,Z	;
	bra	optordy		; if (1[fsr0]) { // initialization has completed
	zOS_RET
optordy
	movf	OPTOB,w		;  w = OPTOB;// our job's single bit of interest
	movf	zOS_MSK,f	;  if (zOS_MSK == 0) {
	btfss	STATUS,Z	;   if (INTCON & 1<<IOCIF == 0)
	bra	optoswi		;    zOS_RET(); // not an IOC, maybe timer0 ovf.
	btfsc	INTCON,IOCIF	;
	bra	optohwi		;   bsr = &IOCBF >> 7;
	zOS_RET
optohwi
	banksel	IOCBF
	andwf	IOCBF,w		;   w = OPTOB & IOCBF; // mask for the port bits
	btfss	STATUS,Z	;   if (w) { // our opto is (at least 1) trigger
	bra	optoioc		;    zOS_MSK = w; // use as scratch var for zero
	zOS_RET
optoioc
	movwf	zOS_MSK		;    IOCBF ^= w; // clear the IOC flag
	xorwf	IOCBF,f		;   } else
optoswi
	andwf	INDF1,w		;    zOS_RET(); // probably belongs to other job
	btfsc	STATUS,Z	;  }
	bra	opto_lo		;  1[FSR0] = (w & *fsr1) ? 0xff : ~zOS_MSK;
opto_hi
	movlw	0xff		;  zOS_ARG(0,(w & *fsr1) ? 'H' : 'L');
	movwi	1[FSR0]		;  zOS_TAI(OUTCHAR);
	movlw	'H'		;
	bra	optoclr		;  // zOS_RFI() implicitly done after zOS_TAI()
opto_lo
	comf	zOS_MSK,w	; }
	movwi	1[FSR0]		; zOS_RET();
	movlw	'L'		;}
optoclr
	zOS_ARG	0
	zOS_TAI	OUTCHAR

greet
	da	"\r\nActivated relay ",0
relay
	zOS_MY2	FSR0
	decf	zOS_ME		;void relay(void) { // 1<= bsr (job#) <= 4
	pagesel	myrelay	
	call	myrelay		; const char* greet = "\r\nActivated relay ";
	movwf	RELAYID		; _relay: uint8_t* fsr0 = 0x70 | (bsr << 1);

	w2port
	movwf	RELAYP		; static uint8_t relayid = myrelay1(bsr);
	movf	RELAYID,w	; static uint8_t relayp = w2port(relayid);
	w2bit	RELAYB

	decf	zOS_ME		; static uint8_t relayb = w2bit(relayid);
	pagesel	myopto
	call	myopto		;
	movwf	OPTOID		; static uint8_t optoid = myopto1(bsr);

	w2port
	movwf	OPTOP		; static uint8_t optop = w2port(optoid);
	movf	OPTOID,w	; static uint8_t optob = w2bit(optoid);
	w2bit	OPTOB
	movwf	OPTOLST		; static uint8_t optolst = optob;// for RA4 only

	pagesel	mychan
	decf	zOS_ME		;
	call	mychan		;
	movwf	MYMASK		; static uint8_t mymask = mychan1(bsr);

	movf	RELAYP,w	;
	movwf	FSR1L		; uint8_t* fsr1;
	movlw	high PORTA	;
	movwf	FSR1H		; fsr1 = (relayp==PORTA&0xff) ? &PORTA : &PORTB;
	
	movlw	0xff		;
	movwi	1[FSR0]		; 1[fsr0] = 0xff; // bits nonzero indicates init
	clrf	SAID_HI		; said_hi = 0;
relaylp
	movf	SAID_HI,w	; do {
	brw			;  if (!said_hi && // haven't announced self yet
relayhi
	zOS_ARG	0
	movf	ALL_IOC,w	;      all_ioc) { // and job 5 running zOS_CON()
	btfsc	STATUS,Z	;   said_hi = !said_hi;
	bra	relayrd		;   zOS_ADR(fsr0 = &greet);
	movlw	relayrd-relayhi	;   zOS_STR(OUTCHAR); // "\r\nActivated relay "
	movwf	SAID_HI		;   zOS_ARG(0,0);
	zOS_ADR	greet,zOS_FLA
	zOS_STR	OUTCHAR		
	movf	zOS_ME		;   zOS_ARG(1,bsr);
	zOS_ARG	1
	zOS_SWI	OUTCHAR
	bra	relay		;   zOS_SWI(OUTCHAR);// "01", "02", "03" or "04"
relayrd
	movf	MYMASK,w	;   goto _relay;
	andwf	INDF0,w		;  }
	btfss	STATUS,Z	;
	bra	relay0		;
	movf	RELAYB,w	;  if (*fsr0 & mymask)
	iorwf	INDF1,f	   	;   *fsr1 |= relayb; // commanded to 1 by global
	bra	relayop	   	;
relay0
	comf	RELAYB,w	;  else
	andwf	INDF1,f		;   *fsr1 &= ~relayb;// commanded to 0 by global
relayop
	movf	OPTOP,w		;  if (OPTOP == PORTA) { // watch in tight loop
	xorlw	low PORTA	;   if (OPTOLST != OPTOB & PORTA) { // changed!
	btfss	STATUS,Z	;    OPTOLST = OPTOB & PORTA; // save new value
	bra	relayld		;    zOS_SWI(NON_IOC); // and tell ISR to look
	movf	OPTOB,w		;   }
	andwf	PORTA,w		;   continue; // get pre-empted; can't afford to 
	xorwf	OPTOLST,f	;  }
	btfsc	STATUS,Z	;
	bra	relaylp		;  zOS_SWI(zOS_YLD);// let next job run (no ARG)
	zOS_SWI	NON_IOC
	bra	relaylp		; } while (1);
relayld
	zOS_SWI	zOS_YLD
	bra	relaylp		;}


main
	clrw			;void main(void) {
	clrf	ALL_IOC		; volatile uint_8t all_ioc = 0; //job 5 clobbers
create	
	pagesel	myopto
	call	myopto		; for (w = 0; w < 4; zOS_LAU(&w)) {//1 job/relay
	movwf	TMP_IOC		;  volatile uint8_t tmp_ioc = myopto(w);
	zOS_ADR	optoisr,zOS_FLA
	movf	TMP_IOC,w	;  fsr0 = &optoisr;
	andlw	0xf8		;
	xorlw	PORTA<<3	;  if (tmp_ioc & 0xf8 == (PORTA<<3) & 0xf8)
	btfss	STATUS,Z	;   zOS_INT(0,NON_IOC); // use a SWI from main()
	bra	use_hwi		;  else { // since Port A has no IOC capability
	zOS_INT	0,NON_IOC
	bra	use_swi		;   all_ioc |= w2bit(tmp_ioc); // Port B use IOC
use_hwi
	movf	TMP_IOC,w	;   zOS_INT(1<<IOCIF,0);// though so register it
	w2bit	TMP_IOC
	movf	TMP_IOC,w	;
	iorwf	ALL_IOC,f	;  }
	zOS_INT	1<<IOCIF,0
use_swi
	zOS_ADR	relay,zOS_UNP
	zOS_LAU	WREG
	btfss	WREG,2		;  fsr0 = &relay 0x7fff; // relay() unpriv'ed
	bra	create		; }
	
	sublw	zOS_NUM-1	;
	btfsc	WREG,7		; if (w == zOS_NUM)// no job remains for zOS_MON
	reset			;  reset();

	banksel	IOCBP
	movf	ALL_IOC,w	;
	movwf	IOCBP		; IOCBP = all_ioc; // IOCIF senses rising optos
	movwf	IOCBN		; IOCBN = all_ioc; // IOCIF senses falling optos
	bsf	INTCON,IOCIE	; INTCON |= 1<<IOCIE; // enable edge sensing HWI
	clrf	ALL_IOC		; ALL_IOC = 0; // will go nonzero once zOS_CON()

	banksel	ANSELB
	bcf	ANSELB,RB5	; ANSELB &= ~(1<<RB5); // allow digital function
	banksel	TRISB
	bcf	TRISB,RB5	; TRISB &= ~(1<<RB5); // allow output heartbeat
	
	banksel	OPTION_REG
	bcf	OPTION_REG,T0CS	; OPTION_REG &= ~(1<<TMR0CS);// off Fosc not pin
	bcf	OPTION_REG,PSA	; OPTION_REG &= ~(1<<PSA);// using max prescaler
	
	zOS_CON	1,20000000/9600,PIR1,PORTB,RB5
	movlw	OUTCHAR		; zOS_MON(/*UART*/1,20MHz/9600bps,PIR1,PORTB,5);
	zOS_ARG	3		; zOS_ARG(3, OUTCHAR/*only 1 SWI*/);
	zOS_LAU	WREG		; zOS_LAU(&w);
	zOS_RUN	INTCON,INTCON	; zOS_RUN(/*T0IE in*/INTCON, /*T0IF in*/INTCON);
	end			;}
