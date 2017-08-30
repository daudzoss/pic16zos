;;; demo_zos.asm
;;;
;;; demonstration (and, frankly, bring-up) app for zOS
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
zOS_NUM	equ	4

	processor 16f1847
	include p16f1847.inc
	
	__CONFIG _CONFIG1,_FOSC_HS & _WDTE_OFF & _PWRTE_OFF & _MCLRE_ON & _CP_OFF & _CPD_OFF & _BOREN_ON & _CLKOUTEN_OFF & _IESO_ON & _FCMEN_ON

;;; uncomment to reduce zOS footprint by 100 words (at cost of zOS_FRK/EXE/FND):
;zOS_MIN	equ	1
	
	include zos.inc
	include zosmacro.inc
	
OUTCHAR	equ	zOS_SI3
	
;;; uncomment to pre-load stack positions with indices (for debugging ZOS_ROL):
;	
 zOS_DBG

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
	decf	SPLVAR,w	;  zOS_ARG(2, splvar);      
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
	
;;; while SWI handlers normally know what line the interupts will come in on,
;;; for flexibility of incorporation into any application this choice is not
;;; hardwired into zosmacro.inc library and any available line may be chosen:
	
main
	banksel	ANSELB
	bcf	ANSELB,RB5	; ANSELB &= ~(1<<RB5); // allow digital function

	banksel	TRISB
	bcf	TRISB,RB5	; TRISB &= ~(1<<RB5); // allow output heartbeat
	
	banksel	OPTION_REG
	bcf	OPTION_REG,PSA	; OPTION_REG &= ~(1<<PSA);// max timer0 prescale
	bcf	OPTION_REG,T0CS	; OPTION_REG &= ~(1<<TMR0CS);// off Fosc not pin

	zOS_CON	0,20000000/9600,PIR1,PORTB,RB5
	movlw	OUTCHAR		;void main(void) {
	zOS_ARG	3		; zOS_CON(/*UART*/1,20MHz/9600bps,PIR1,PORTB,5);
	zOS_LAU	WREG		; zOS_ARG(3,OUTCHAR/*only 1 SWI*/); zOS_LAU(&w);

	zOS_INT	0,0		; zOS_INT(0,0);//no interrupt handler for splash
	zOS_ADR	splash,zOS_PRB	; zOS_ADR(fsr0 = splash&~zOS_PRV);// privileged
	zOS_LAU	WREG		; zOS_LAU(&w);
	
	zOS_INT	0,0		; zOS_INT(0,0);//no interrupt handler either
	zOS_ADR	spitjob,zOS_UNP	; zOS_ADR(fsr0 = spitjob&~zOS_PRV);//unprivilege
	zOS_LAU	WREG		; zOS_LAU(&w);
	zOS_LAU	WREG		; zOS_LAU(&w); // launch two copies
	
	zOS_RUN	INTCON,INTCON	; zOS_RUN(/*T0IE in*/INTCON, /*T0IF in*/INTCON);
	end			;}
	
