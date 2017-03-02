	processor 16f1847
include p16f1847.inc
	
__CONFIG

;;; uncomment to reduce ZOS footprint by 100 words (at cost of FRK/EXE/FND SWI):
;;;ZOS_MIN equ 1
include zos.inc
include zosmacro.inc

;;; while SWI handlers normally know what line the interupts will come in on,
;;; for flexibility of incorporation into any application this choice is not
;;; hardwired into zosmacro.inc and any available line may be chosen:
OUTCHAR	equ	ZOS_SI3
	
	ZOS_MON	1,9600,PORTA,RA0
	movlw	OUTCHAR		;void main(void) {
	ZOS_ARG	3
retry1
	ZOS_SWI	ZOS_NEW
	btfsc	STATUS,Z	; ZOS_MON(/*SSP*/1,9600,PORTA,RA0/*hbeat*/);
	bra	retry1		; ZOS_ARG(3, OUTCHAR);//handles without knowing!
	
	movlw	low spitjob	; do {} while (ZOS_SWI(ZOS_NEW) == 0); // 1?
	movwf	FSR0L		; fsr0 = spitjob; // no interrupts
	movlw	high spitjob	; ZOS_ARG(0,0);
	movwf	FSR0H		; ZOS_ARG(1,0);
	clrw			; ZOS_ARG(2,0);
	nop			; ZOS_ARG(3,0);
	ZOS_ARG	0
	ZOS_ARG	1
	ZOS_ARG	2
	ZOS_ARG 3
retry2
	ZOS_SWI	ZOS_NEW
	btfsc	STATUS,Z	; do {} while (ZOS_SWI(ZOS_NEW) == 0); // 2?
	bra	retry2		; do {} while (ZOS_SWI(ZOS_NEW) == 0); // 3?
retry3
	ZOS_SWI	ZOS_NEW
	btfsc	STATUS,Z	; ZOS_RUN();
	bra	retry3		;}
	ZOS_RUN
spitjob	
	movf	BSR,w		;void spitjob(void) {
	addlw	'0'		; do {
	ZOS_ARG	0
	ZOS_SWI	OUTCHAR
i	equ	0x20
j	equ	0x21
loop
	incf	j,f		;  ZOS_ARG(0,'0'+bsr); // job number
	btfss	STATUS,Z	;  ZOS_SWI(OUTCHAR);
	bra	loop		;  for (int i = 0; i & 0xff; i++)
	incf	i,f		;   for (int j = 0; j & 0xff; j++)
	btfss	STATUS,Z	;    ;
	bra	loop		; } while (1);
	bra	spitjob		;}
	
	end
	
