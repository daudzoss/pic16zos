;;; demo_zos.asm
;;;
;;; demonstration (and, frankly, bring-up) app for zOS
;;; to build: gpasm -D GPASM demo_zos.asm
;;;
;;; after starting job #1 as a console output buffer (zOS_CON() in zosmacro.inc)
;;; to demonstrate privileged mode (able to kill or otherwise tweak other tasks)
;;; 
;;; it starts a splash() job #2 to copy a packed ascii greeting into the buffer
;;; (using the SWI line zOS_SI3) character by character, also privileged so that
;;; it can un-wait the two unprivileged tasks (to guarantee they don't overwrite
;;; the potential long greeting)
;;; 
;;; two final processes (should end up numbered jobs 3 and 4) run in re-entrant
;;; function splitjob() printing their own job numbers to the console
;;; 
;;; since only 4 of 5 possible task slots are used in this demo reducing the max
;;; allowed value by 1 will make scheduler run faster:
zOS_NUM	equ	1
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
	
;;; uncomment to pre-load stack positions with indices (for debugging ZOS_ROL):
;	zOS_DBG

	pagesel	main
	goto	main

greet
	da	"Demo application for zOS"
crlf
	da	"\r\n",0
put_str
	zOS_STR	OUTCHAR
	return			;void put_str(const char*) { zOS_STR(OUTCHAR); }
SPLVAR	equ	0x20
splash
	movf	zOS_ME		;void splash(void) {
	zOS_ARG	0		; // ceding processor to let both spitjob()s run
	zOS_SWI	zOS_YLD		; zOS_ARG(0, bsr);
	movf	zOS_ME		; zOS_SWI(zOS_YLD);
	zOS_ARG	0		; zOS_ARG(0, bsr);
	zOS_SWI	zOS_YLD		; zOS_SWI(zOS_YLD);
	zOS_ADR	greet,zOS_FLA	;
	pagesel	put_str		; zOS_ADR(fsr0 ="Demo application for zOS\r\n");
	call	put_str		; put_str(fsr0);
	movlw	zOS_NUM+1	; uint8_t splvar = zOS_NUM + 1;
	movwf	SPLVAR		; while (--splvar) {
splalp
	movlw	low spitjob	;  zOS_ARG(0, spitjob & 0x00ff);
	zOS_ARG	0
	movlw	high spitjob	;  zOS_ARG(1, spitjob >> 8);
	zOS_ARG	1
	decf	SPLVAR,w	;  zOS_ARG(2, splvar);  // max job# to find
	btfsc	STATUS,Z	;  splvar = zOS_SWI(zOS_FND);      
	bra	spldone		;  if (splvar)
	zOS_ARG	2
	zOS_SWI	zOS_FND
	movwf	SPLVAR		;   zOS_UNW(splvar); // un-wait found spitjob()s
	movf	SPLVAR,f	;  else
	btfsc	STATUS,Z	;   break; // until none found at all
	bra	spldone		; }
	zOS_UNW	SPLVAR
	bra	splalp		; zOS_ARG(0, bsr);
spldone
	movf	zOS_ME		; zOS_SWI(zOS_END); // unschedule self
	zOS_ARG	0		;}
	zOS_SWI	zOS_END
	
spitjob	
	zOS_SWI	zOS_WAI		;void spitjob(void) {
reprint
	movf	zOS_ME		; zOS_SWI(zOS_SLP); // splash() wakes when done
	andlw	1		; do {
	brw			;  w = zOS_ME();// shouldn't get clobbered below
	bra	asxbyte		;  switch (w & 1) { 
	bra	asascii		;  case 0:
asxbyte
	clrw			;   zOS_ARG(0, 0);
	zOS_ARG	0
	movf	zOS_ME		;   zOS_ARG(1, w); // print as numeric "02"/"03"
	zOS_ARG	1
	bra	print		;   break;
asascii
	movlw	'0'		;  case 1:
	addwf	zOS_ME		;   zOS_ARG(0, w); // print as character '2'/'3'
	zOS_ARG	0		;  }
print
	zOS_SWI	OUTCHAR		;  zOS_SWI(OUTCHAR);
	zOS_ADR	crlf,zOS_FLA	;  zOS_ADR(fsr0 = "\r\n");
	pagesel	put_str	
	call	put_str		;  put_str(fsr0);
#if 1
spit_i	equ	0x20
spit_j	equ	0x21
loop
	incfsz	spit_j,f	;  for (int i = 0; i & 0xff; i++)
	bra	loop		;   for (int j = 0; j & 0xff; j++)
	incfsz	spit_i,f	;    ;
	bra	loop		; } while (1);
#endif
	bra	reprint		;}
	
;;; while SWI handlers normally know what line the interrupts will come in on,
;;; for flexibility of incorporation into any application this choice is not
;;; hardwired into zosmacro.inc library and any available line may be chosen:
	
main
	banksel OSCCON			;{
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
	movlw	0xbf
	movwf	TRISC

	banksel	PPSLOCK
	movlw	0x55
	movwf	PPSLOCK
	movlw	0xaa
	movwf	PPSLOCK
	bcf	PPSLOCK,PPSLOCKED
	movlw	0x17
	movwf	RXPPS
	
	banksel	RC6PPS
	movlw	0x14
	movwf	RC6PPS
	movlw	0x55
	movwf	PPSLOCK
	movlw	0xaa
	movwf	PPSLOCK
	bsf	PPSLOCK,PPSLOCKED
;	zOS_CON	0,8000000/9600,PIR1,LATA,RA4
	zOS_CON	0,32000000/9600,PIR1,LATA,RA4
;	zOS_MAN	0,32000000/9600,PIR1,LATA,RA4
	movlw	OUTCHAR		;void main(void) {
	zOS_ARG	3		; zOS_CON(/*UART*/1,20MHz/9600bps,PIR1,PORTB,5);
	zOS_LAU	WREG		; zOS_ARG(3,OUTCHAR/*only 1 SWI*/); zOS_LAU(&w);
 zOS_RUN INTCON,INTCON
 end
	zOS_INT	0,0		; zOS_INT(0,0);//no interrupt handler for splash
	zOS_ADR	splash,zOS_PRB	; zOS_ADR(fsr0 = splash&~zOS_PRV);// privileged
	zOS_LAU	WREG		; zOS_LAU(&w);
	
	zOS_INT	0,0		; zOS_INT(0,0);//no interrupt handler either
	zOS_ADR	spitjob,zOS_UNP	; zOS_ADR(fsr0 = spitjob&~zOS_PRV);//unprivilege
	zOS_LAU	WREG		; zOS_LAU(&w);
	zOS_LAU	WREG		; zOS_LAU(&w); // launch two copies
	
	zOS_RUN	INTCON,INTCON	; zOS_RUN(/*T0IE in*/INTCON, /*T0IF in*/INTCON);
	end			;}
	
