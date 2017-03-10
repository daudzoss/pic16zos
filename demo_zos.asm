	processor 16f1847
	include p16f1847.inc
	
;__CONFIG

;;; uncomment to reduce zOS footprint by 100 words (at cost of zOS_FRK/EXE/FND):
;
zOS_MIN	equ	1
	
	include zos.inc
	include zosmacro.inc
	
;;; uncomment to pre-load stack positions with indices (for debugging ZOS_ROL):
;
	zOS_DBG

;;; while SWI handlers normally know what line the interupts will come in on,
;;; for flexibility of incorporation into any application this choice is not
;;; hardwired into zosmacro.inc library and any available line may be chosen:
	
	zOS_CON	1,9600,PIR1,PORTB,RB5
OUTCHAR	equ	zOS_SI3
	movlw	OUTCHAR		;void main(void) {
	zOS_ARG	3
retry1
	zOS_SWI	zOS_NEW
	movf	WREG,w		;
	btfsc	STATUS,Z	; zOS_MON(/*SSP*/1,9600,PIR1,PORTA,RA0/*beat*/);
	bra	retry1		; zOS_ARG(3, OUTCHAR);//handles without knowing!
	
	movlw	low splash	; do {} while (zOS_SWI(zOS_NEW) == 0);
	movwf	FSR0L		; fsr0 = splash & 0x7fff;
	movlw	high splash	; zOS_ARG(0, 0);
	andlw	0x7f		; zOS_ARG(1, 0);
	movwf	FSR0H		; zOS_ARG(2, 0);
	clrw			; zOS_ARG(3, 0);// takes no interrupts
	zOS_ARG	0
	zOS_ARG	1
	zOS_ARG	2
	zOS_ARG 3
retry2
	zOS_SWI	zOS_NEW
	movf	WREG,w		;
	btfsc	STATUS,Z	; do {} while (zOS_SWI(zOS_NEW) == 0); // 2?
	bra	retry2		;
	
	movlw	low spitjob	; do {} while (zOS_SWI(zOS_NEW) == 0);
	movwf	FSR0L		; fsr0 = spitjob & 0x7fff;
	movlw	high spitjob	; zOS_ARG(0, 0);
	andlw	0x7f		; zOS_ARG(1, 0);
	movwf	FSR0H		; zOS_ARG(2, 0);
	clrw			; zOS_ARG(3, 0);// FIXME: needs interrupt to GO!
	zOS_ARG	0
	zOS_ARG	1
	zOS_ARG	2
	zOS_ARG 3
retry3
	zOS_SWI	zOS_NEW
	movf	WREG,w		;
	btfsc	STATUS,Z	; do {} while (zOS_SWI(zOS_NEW) == 0); // 2?
	bra	retry3		; do {} while (zOS_SWI(zOS_NEW) == 0); // 3?
retry4
	zOS_SWI	zOS_NEW
	movf	WREG,w		;
	btfsc	STATUS,Z	; zOS_RUN(/*T0IE in*/INTCON, /*T0IF in*/INTCON);
	bra	retry4		;}
	
	banksel	OPTION_REG
	bcf	OPTION_REG,T0CS	; OPTION_REG &= ~(1<<TMR0CS);// off Fosc not pin
;;FIXME: set the prescaler appropriately so that the IRQ frequency isn't crazy
	zOS_RUN	INTCON,INTCON
	
splash
	zOS_STR	OUTCHAR,"hello!\r\n"
	zOS_SWI	zOS_END
	
spitjob	
#if 0
	clrw			;// print as hexadecimal integer: OUTCHAR 0 int
	zOS_ARG	0
#else	
	movlw	'0'		;// print as ascii digit: OUTCHAR '0'+int
#endif	
	addwf	zOS_ME		;void spitjob(void) { do {
#if 0
	zOS_ARG	1
#else
	zOS_ARG	0
#endif
	zOS_SWI	OUTCHAR
#if 0
i	equ	0x20
j	equ	0x21
loop
	incf	j,f		;  zOS_ARG(0,'0'+bsr); // job number
	btfss	STATUS,Z	;  zOS_SWI(OUTCHAR);
	bra	loop		;  for (int i = 0; i & 0xff; i++)
	incf	i,f		;   for (int j = 0; j & 0xff; j++)
	btfss	STATUS,Z	;    ;
	bra	loop		; } while (1);
#endif
	bra	spitjob		;}
	
	end
	
