	processor 16f1847
	include	p16f1847.inc

	__CONFIG _CONFIG1,_FOSC_HS & _WDTE_OFF & _PWRTE_OFF & _MCLRE_ON & _CP_OFF & _CPD_OFF & _BOREN_ON & _CLKOUTEN_OFF & _IESO_ON & _FCMEN_ON

;;; uncomment to reduce zOS footprint by 100 words (at cost of zOS_FRK/EXE/FND):
;zOS_MIN	equ	1
	
	include zos.inc
	include zosmacro.inc
	
;;; uncomment to pre-load stack positions with indices (for debugging ZOS_ROL):
;zOS_DBG

	pagesel	main
	goto	main
	
relay	

OUTCHAR	equ	zOS_SI3
main
	zOS_MON	1,20000000/9600,PIR1,PORTB,RB5
	movlw	OUTCHAR		;void main(void) {
	zOS_ARG	3		; zOS_CON(/*SSP*/1,20MHz/9600bps,PIR1,PORTB,5);
	zOS_LAU	WREG		; zOS_ARG(3,OUTCHAR/*only 1 SWI*/); zOS_LAU(&w);
#ifdef ANSELB
	banksel	ANSELB
	bcf	ANSELB,RB5	; ANSELB &= ~(1<<RB5); // allow digital function
#endif
	banksel	TRISB
	bcf	TRISB,RB5	; TRISB &= ~(1<<RB5); // allow output heartbeat
	
	banksel	OPTION_REG
	bcf	OPTION_REG,T0CS	; OPTION_REG &= ~(1<<TMR0CS);// off Fosc not pin
	bcf	OPTION_REG,PSA	; OPTION_REG &= ~(1<<TMR0CS);// use max prescale

	zOS_RUN	INTCON,INTCON	; zOS_RUN(/*T0IE in*/INTCON, /*T0IF in*/INTCON);
	nop			;}
	
	end
