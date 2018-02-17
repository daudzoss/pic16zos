;;; demo_zos.asm
;;;
;;; demonstration (and, frankly, bring-up) app for zOS
;;; to build: gpasm -D GPASM demo_zos.asm
;;;
;;; after starting job #1 as a console output buffer (zOS_CON() in zosmacro.inc)
;;; to demonstrate privileged mode (able to kill or otherwise tweak other tasks)
;;; 
;;; two final processes (initially numbered jobs 3 and 4) run in re-entrant
;;; functions dummy and dummy2
;;; 
;;; if fewer than the 5 possible job slots are used, as in this demo, reducing
;;; the max allowed value to 4 or lower will waste less time in the scheduler:
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
	movlw	0x7f
	movwf	TRISC

	banksel	PPSLOCK
	movlw	0x55
	movwf	PPSLOCK
	movlw	0xaa
	movwf	PPSLOCK
	bcf	PPSLOCK,PPSLOCKED
	movlw	0x16
	movwf	RXPPS
	
	banksel	RC7PPS
	movlw	0x14
	movwf	RC7PPS
	movlw	0x55
	movwf	PPSLOCK
	movlw	0xaa
	movwf	PPSLOCK
	bsf	PPSLOCK,PPSLOCKED

;;; while SWI handlers normally know what line the interrupts will come in on,
;;; for flexibility of incorporation into any application this choice is not
;;; hardwired into zosmacro.inc library and any available line may be chosen:
;	zOS_MAN	0,.32000000/.9600,PIR1,LATA,RA4,0
	zOS_CLC	0,.32000000/.9600,PIR1,LATA,RA4,0
	movlw	OUTCHAR		;void main(void) {
	movwi	0[FSR0]		; zOS_xxx(/*UART*/1,32MHz/9600bps,PIR1,LATA,4);
	
	zOS_INT	0,0
	zOS_ADR	dummy,zOS_UNP
	zOS_LAU	WREG
	zOS_ACT	FSR0

	zOS_INT	0,0
	zOS_ADR	dummy2,zOS_UNP
	zOS_LAU	WREG
	zOS_ACT	FSR0

	zOS_RUN	INTCON,INTCON	; zOS_RUN(/*T0IE in*/INTCON, /*T0IF in*/INTCON);
	
	zOS_NAM	"infinite loop"
dummy
	bra	dummy

	zOS_NAM	"cooperative loop"
dummy2
	movf	zOS_ME,w
	zOS_ARG	0
	zOS_SWI	zOS_YLD
	bra	dummy2

	end			;}
	
