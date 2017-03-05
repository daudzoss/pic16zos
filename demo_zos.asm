	processor 16f1847
include p16f1847.inc
	
__CONFIG

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
	
	zOS_MON	1,9600,PIR1,PORTA,RA0
	movlw	OUTCHAR		;void main(void) {
	zOS_ARG	3
retry1
	zOS_SWI	zOS_NEW
	btfsc	STATUS,Z	; zOS_MON(/*SSP*/1,9600,PIR1,PORTA,RA0/*beat*/);
	bra	retry1		; zOS_ARG(3, OUTCHAR);//handles without knowing!
	nop		    	; do {} while (zOS_SWI(zOS_NEW) == 0); // 1?
	
	movlw	low spitjob	; fsr0 = spitjob; // takes no interrupts
	movwf	FSR0L		; zOS_ARG(0, 0);
	movlw	high spitjob	; zOS_ARG(1, 0);
	movwf	FSR0H		; zOS_ARG(2, 0);
	clrw			; zOS_ARG(3, 0);
	zOS_ARG	0
	zOS_ARG	1
	zOS_ARG	2
	zOS_ARG 3
retry2
	zOS_SWI	zOS_NEW
	btfsc	STATUS,Z	; do {} while (zOS_SWI(zOS_NEW) == 0); // 2?
	bra	retry2		; do {} while (zOS_SWI(zOS_NEW) == 0); // 3?
retry3
	zOS_SWI	zOS_NEW
	btfsc	STATUS,Z	; zOS_RUN();
	bra	retry3		;}
	zOS_RUN
	
spitjob	
#if 0
	clrw			;// print as hexadecimal integer: OUTCHAR 0 int
	zOS_ARG	0
#else	
	movlw	'0'		;// print as ascii digit: OUTCHAR '0'+int
#endif	
	addwf	zOS_ME		;void spitjob(void) {
	movf	zOS_ME		; do {
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
	
