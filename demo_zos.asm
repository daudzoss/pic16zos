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
	
OUTCHAR	equ	zOS_SI3
	zOS_CON	1,9600,PIR1,PORTB,RB5
	movlw	OUTCHAR		;void main(void) {
	zOS_ARG	3		; zOS_CON(/*SSP*/1,9600,PIR1,PORTB,RB5/*beat*/);
	zOS_LAU	WREG		; zOS_ARG(3, OUTCHAR);//handles without knowing!
	
	zOS_INT	0,0		; zOS_INT(0,0);//no interrupt handler for splash
	zOS_ADR	splash -zOS_PRV	; zOS_ADR(splash -zOS_PRV); // unprivileged job
	zOS_LAU	FSR1L		; zOS_LAU(&fsr1); // stash ID in FSR1L until end
	
	zOS_INT	0,0		; zOS_INT(0,0);//no interrupt handler either
	zOS_ADR	spitjob -zOS_PRV; zOS_ADR(spitjob -zOS_PRV);// unprivileged jobs
	zOS_LAU	FSR1H		; zOS_LAU(1 + &fsr1); // launch two copies...
	zOS_LAU	WREG		; zOS_LAU(&w);// remembering job# in FSR1H, WREG
	
	zOS_GLO	FSR0,WREG	; zOS_GLO(&fsr0, w); // mailboxes for spitjob()
	movf	FSR1L,w		;
	movwi	FSR0++		; fsr0 = fsr1; // this spitjob() waits for GO!
	clrf	INDF0		; *fsr0 = 0; // by watching splash()'s global#0
	
	zOS_GLO	FSR0,FSR1H	; zOS_GLO(&fsr0, *(1 + &fsr1));
	movf	FSR1L,w		;
	movwi	FSR0++		; fsr0 = fsr1; // this spitjob() waits for GO!
	clrf	INDF0		; *fsr0 = 0; // by watching splash()'s global#0
	
	clrf	FSR1H		;
	clrf	INDF1		; *fsr1 = 0; // ...change from this 0 to nonzero
	
	banksel	OPTION_REG
	bcf	OPTION_REG,T0CS	; OPTION_REG &= ~(1<<TMR0CS);// off Fosc not pin
;;;FIXME: set the prescaler appropriately so that the IRQ frequency isn't crazy
	zOS_RUN	INTCON,INTCON	; zOS_RUN(/*T0IE in*/INTCON, /*T0IF in*/INTCON);
	nop			;}
	
splash
	zOS_MY2	FSR0		;void splash(void) {
	zOS_STR	OUTCHAR,"Hi\r\n"; zOS_MY2(&fsr0);
	movlw	1		; zOS_STR(OUTCHAR, "Hi\r\n");
	comf	INDF0,f		; *fsr0 = 1; // will cause spitjob()s to unstick
#if 0
	movf	zOS_ME ;implicit
	zOS_ARG	0
#endif	
	zOS_SWI	zOS_END		; zOS_END(); // unschedule self
	nop			;}
	
spitjob	
	zOS_MY2	FSR0
	moviw	0[FSR0],w	;void splash(void) {
	movwf	FSR1L		;
	moviw	1[FSR0],w	; zOS_MY2(&fsr0);
	movwf	FSR1H		; fsr1 = *fsr0;
	movf	zOS_ME		; zOS_ME(&w); // shouldn't be clobbered by below
awaitgo
#if 0
	zOS_ARG	0
	;; being nice is optional
	zOS_SWI	zOS_YLD		;
#endif	
	movf	INDF1,f		;
	btfss	awaitgo		;
	
	btfss


#if 0
	clrw			;// if even, print as hexadecimal integer: OUTCHAR 0 int
	zOS_ARG	0
#else	
	movlw	'0'		;// if odd, print as ascii digit: OUTCHAR '0'+int
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
	
