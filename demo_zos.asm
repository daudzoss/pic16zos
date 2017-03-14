	processor 16f1847
	include p16f1847.inc
	
;__CONFIG

;;; uncomment to reduce zOS footprint by 100 words (at cost of zOS_FRK/EXE/FND):
;zOS_MIN	equ	1
	
	include zos.inc
	include zosmacro.inc
	
;;; uncomment to pre-load stack positions with indices (for debugging ZOS_ROL):
;	
 zOS_DBG

	pagesel	main
	goto	main


put_str
	zOS_STR	OUTCHAR		;void put_str(const char*){zOS_STR(OUTCHAR,"");}
	return			;
	
greet
	da	"Demo application for zOS"
crlf
	da	"\r\n",0
splash
	zOS_MY2	FSR1		;void splash(void) {
	zOS_ADR	greet,zOS_FLA	;
	pagesel	put_str		; zOS_MY2(&fsr1);
	call	put_str		; zOS_ADR(fsr0 ="Demo application for zOS\r\n");
	movlw	1		; put_str(fsr0);
	movwf	INDF1		; *fsr1 = 1; // will cause spitjob()s to unstick
#if 0
	movf	zOS_ME ;implicit
	zOS_ARG	0
#endif	
	zOS_SWI	zOS_END		; zOS_END(); // unschedule self
	nop			;}
	
spitjob	
	zOS_MY2	FSR0
	moviw	0[FSR0]		;void spitjob(void) {
	movwf	FSR1L		;
	moviw	1[FSR0]		; zOS_MY2(&fsr0);
	movwf	FSR1H		; fsr1 = *fsr0; // watch fsr1 for nonzero
	movf	zOS_ME		; w = zOS_ME(); // shouldn't get clobbered below
awaitgo
#if 1
	zOS_ARG	0;implicit since nonprivileged
	;;being nice is optional:
	zOS_SWI	zOS_YLD
#endif	
	movf	INDF1,f		; do {
	btfss	STATUS,Z	;
	bra	awaitgo		;  do {} while (*fsr1 == 0);
	andlw	1		;
	brw			;  switch (w & 1) {
	bra	asxbyte		; 
	bra	asascii		;  case 0:
asxbyte
	clrw			;   zOS_ARG(0, 0);
	zOS_ARG	0
	movf	zOS_ME		;   zOS_ARG(1, w);
	zOS_ARG	1
	bra	print		;   break;
asascii
	movlw	'0'		;  case 1:
	addwf	zOS_ME		;   zOS_ARG(0, w);
	zOS_ARG	0		;  }
print
	zOS_SWI	OUTCHAR		;  zOS_SWI(OUTCHAR);
	zOS_ADR	crlf,zOS_FLA	;  zOS_ADR(fsr0 = "\r\n");
	pagesel	put_str	
	call	put_str		;  put_str(fsr0);
#if 1
i	equ	0x20
j	equ	0x21
loop
	incfsz	j,f		;  for (int i = 0; i & 0xff; i++)
	bra	loop		;   for (int j = 0; j & 0xff; j++)
	incfsz	i,f		;    ;
	bra	loop		; } while (1);
#endif
	bra	awaitgo		;}
	
;;; while SWI handlers normally know what line the interupts will come in on,
;;; for flexibility of incorporation into any application this choice is not
;;; hardwired into zosmacro.inc library and any available line may be chosen:
	
OUTCHAR	equ	zOS_SI3
main	
	banksel	TRISB
	bcf	TRISB,RB5
	zOS_CON	1,20000000/9600,PIR1,PORTB,RB5
	movlw	OUTCHAR		;void main(void) {
	zOS_ARG	3		; zOS_CON(/*SSP*/1,20MHz/9600bps,PIR1,PORTB,5);
	zOS_LAU	WREG		; zOS_ARG(3, OUTCHAR);//handles without knowing!
	
	zOS_INT	0,0		; zOS_INT(0,0);//no interrupt handler for splash
	zOS_ADR	splash,zOS_UNP	; zOS_ADR(fsr0 = splash&~zOS_PRV);//unprivileged
	zOS_LAU	FSR1L		; zOS_LAU(&fsr1);// stash job then addr in FSR1L
	
	zOS_GLO	FSR1,FSR1L	; zOS_GLO(&fsr1,fsr1&0xff);// scary but it works
	zOS_INT	0,0		; zOS_INT(0,0);//no interrupt handler either
	zOS_ADR	spitjob,zOS_UNP	; zOS_ADR(fsr0 = spitjob&~zOS_PRV);//unprivilege
	zOS_LAU	FSR1H		; zOS_LAU(1 + &fsr1); // launch two copies...
	zOS_LAU	WREG		; zOS_LAU(&w);// remembering job# in FSR1H, WREG
	
	zOS_GLO	FSR0,WREG	; zOS_GLO(&fsr0, w); // mailboxes for spitjob()
	movf	FSR1L,w		;
	movwi	FSR0++		; // this spitjob() waits for "go" by watching
	clrf	INDF0		; *fsr0 = fsr1; // splash()'s global#0
	
	zOS_GLO	FSR0,FSR1H	; zOS_GLO(&fsr0, *(1 + &fsr1));
	movf	FSR1L,w		;
	movwi	FSR0++		; // this spitjob() waits for "go" by watching
	clrf	INDF0		; *fsr0 = fsr1; // splash()'s global#0 also
	
	clrf	INDF1		; *fsr1 = 0; // ...change from this 0 to nonzero
	
#if 1
;;; normal mode: use tmr0 as OS timer rather than an external transition counter
	banksel	OPTION_REG
	bcf	OPTION_REG,T0CS	; OPTION_REG &= ~(1<<TMR0CS);// off Fosc not pin
#else
;;; leave T0CS high to see if interrupts are happening, just overwhelming
	banksel	ANSELA		;
	bcf	ANSELA,RA4	; ANSELA 
#endif
;;;FIXME: set the prescaler appropriately so that the IRQ frequency isn't crazy
	zOS_RUN	INTCON,INTCON	; zOS_RUN(/*T0IE in*/INTCON, /*T0IF in*/INTCON);
	nop			;}
	
	end
	
