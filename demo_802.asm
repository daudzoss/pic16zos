;;; demo_802.asm
;;;
;;; demo to measure AN2*AN6 (or just AN2)
;;;                 AN3*AN7 (or just AN3)
;;;                 AN4*AN0 (or just AN4)
;;;                 AN5*AN1 (or just AN5
;;; and transmit them periodically (with TMR2:TMR1 timestamps)
;;;  according to IEEE 802.15.6 HBC protocol
;;; console automatically quits and starts
;;;  app if no input within TBD sec (preferable TBD is also the tx interval)
;;; modes (rates, dual channel vs. product) selectable through mailbox
	
	processor 16lf1619
	include p16lf1619.inc
	
C_UART	equ	.000009600;GHz
#ifdef EX21MHZ
C_SYSTM	equ	.021000000;GHz
	__CONFIG _CONFIG1,_CLKOUTEN_OFF & _BOREN_NSLEEP & _CP_OFF & _MCLRE_ON & _PWRTE_ON & _FOSC_ECH
	__CONFIG _CONFIG2,_LVP_OFF & _DEBUG_ON & _LPBOR_ON & BORV_LO & _STVREN_OFF & _PLLEN_OFF & _nZCD_OFF & _PPS1WAY_OFF & _WRT_OFF
#else
#ifdef  IN32MHZ
C_SYSTM	equ	.032000000;GHz
	__CONFIG _CONFIG1,_CLKOUTEN_OFF & _BOREN_NSLEEP & _CP_OFF & _MCLRE_ON & _PWRTE_ON & _FOSC_INTOSC
	__CONFIG _CONFIG2,_LVP_OFF & _DEBUG_ON & _LPBOR_ON & BORV_LO & _STVREN_OFF & _PLLEN_ON & _nZCD_OFF & _PPS1WAY_OFF & _WRT_OFF
#else
C_SYSTM	equ	.016000000;GHz
	__CONFIG _CONFIG1,_CLKOUTEN_OFF & _BOREN_NSLEEP & _CP_OFF & _MCLRE_ON & _PWRTE_ON & _FOSC_INTOSC
	__CONFIG _CONFIG2,_LVP_OFF & _DEBUG_ON & _LPBOR_ON & BORV_LO & _STVREN_OFF & _PLLEN_OFF & _nZCD_OFF & _PPS1WAY_OFF & _WRT_OFF
#endif
#endif
C_RATIO	equ	C_SYSTEM/C_UART
	
	__CONFIG _CONFIG3,_WDTCCS_SWC & _WDTCWS_WDTCWSSW & _WDTE_SWDTEN & _WDTCPS_WDTCPS1F
	
;;; MPN    MPN    prog SRAM
;;; '1614  '1618  4096 512
;;; '1615  '1619  8192 1024
;;; 
;;; 16-QFN 20-QFN ana. dig. dir purpose
;;; pin 12 pin 16 AN0  RA0   D  ICSPDAT
;;; pin 11 pin 15 AN1  RA1   D  ICSPCLK
;;; pin 10 pin 14 AN2  RA2   I  HOST_TX
;;; pin 3  pin 1       RA3   I  MCLR_N
;;; pin 2  pin 20 AN3  RA4   O  CLKOUT/HOST_RX
;;; pin 1  pin 19      RA5   I  CLKIN
;;;;       pin 10 AN10 RB4  
;;;;       pin 9  AN9  RB5  
;;;;       pin 8       RB6  
;;;;       pin 7       RB7  
;;; pin 9  pin 13 AN4  RC0   A  AN4
;;; pin 8  pin 12 AN5  RC1   A  AN5
;;; pin 7  pin 11 AN6  RC2   A  AN6
;;; pin 6  pin 4  AN7  RC3   A  AN7
;;; pin 5  pin 3       RC4   O  MDOUT_P
;;; pin 4  pin 2       RC5   O  MDOUT_N
;;;;       pin 5  AN8  RC6
;;;;       pin 6  AN9  RC7

	include	zos.inc
	include	zosmacro.inc

OUTCHAR	equ	zOS_SI3
ADC4SWI	equ	zOS_SI4
ADC5SWI	equ	zOS_SI5	
ADC6SWI	equ	zOS_SI6
ADC7SWI	equ	zOS_SI7	

	pagesel	main
	goto	main

	zOS_NAM	"ADC control, buffer"
adcmeas
	bra

isr_adc

isr_ctx

main
	banksel	OSCCON
	movlw	0xf<<IRCF0	;void main(void) {
	movwf	OSCCON		; OSCCON = 0x38;// 16MHz (32MHz with PLL) if int
#ifdef EX21MHZ
#else
#ifdef IN32MHZ
	btfss	OSCSTAT,PLLR	; while (OSCSTAT & (1<<PLLR) == 0)
	bra	$-1		;  ; // 32MHz only: await PLL "ready"
#endif
	btfss	OSCSTAT,HFIOFR	; while (OSCSTAT & (1<<HFIOFR) == 0)
	bra	$-1		;  ; // 16/32MHz only: await HFINTOSC "ready"
	btfss	OSCSTAT,HFIOFL	; while (OSCSTAT & (1<<HFIOFL) == 0)
	bra	$-1		;  ; // 16/32MHz only: await HFINTOSC "locked"
	btfss	OSCSTAT,HFIOFS	; while (OSCSTAT & (1<<HFIOFS) == 0)
	bra	$-1		;  ; // 16/32MHz only: await HFINTOSC "stable"
#endif

	banksel	ANSELA
	clrf	ANSELA		; ANSELA = 0; // no analog inputs among RA pins
	banksel	ANSELC		;
	movlw	0x0f		;
	movwf	ANSELC		; ANSELC = 15; // use AN4 to AN7 (on RC0 to RC3)

	banksel	OPTION_REG
	bcf	OPTION_REG,PSA	; OPTION_REG &= ~(1<<PSA);// max timer0 prescale
	bcf	OPTION_REG,T0CS	; OPTION_REG &= ~(1<<TMR0CS);// off Fosc not pin

	banksel	TRISA
	movlw	0x2f		; TRISA = (1<<RA5) | (0<<RA4) | (1<<RA3) |
	movwf	TRISA		;         (1<<RA2) | (1<<RA1) | (1<<RA0);
	banksel	TRISC
	movlw	0x0f		; TRISC = (0<<RC5) | (0<<RC4) | (1<<RC3) |
	movwf	TRISC		;         (1<<RC2) | (1<<RC1) | (1<<RC0);
	
	banksel	HIDRVC
	bsf	HIDRVC,RC4	;
	bsf	HIDRVC,RC5	; HIDRVC |= (1 << RC5) | (1 << RC4); // 100mA
	banksel	SLRCONC
	bcf	SLRCONC,RC4	;
	bcf	SLRCONC,RC5	; SLRCONC &= ~((1 << RC5) | (1 << RC4)); // slew

	banksel	PPSLOCK
	movlw	0x55		;
	movwf	PPSLOCK		; PPSLOCK = 0x55; // magic step 1
	movlw	0xaa		;
	movwf	PPSLOCK		; PPSLOCK = 0xaa; // magic step 2
	bcf	PPSLOCK,PPSLOCKED;PPSLOCK &= ~(1<<PPSLOCKED); // unlock PPS
	movlw	RA2		;
	movwf	RXPPS		; RXPPS = (0<<3) | RA2; // HOST_TX input

	banksel	RA4PPS
	movlw	0x14		;
	movwf	RA4PPS		; RA4PPS = 0x14; // HOST_RX output
	movlw	0x10		;
	movwf	RC4PPS		; RC4PPS = 0x10; // CWG1OUTA
	movlw	0x11		;
	movwf	RC5PPS		; RC5PPS = 0x11; // CWG1OUTB

	banksel	PPSLOCK
	movlw	0x55		;
	movwf	PPSLOCK		; PPSLOCK = 0x55; // magic step 1
	movlw	0xaa		;
	movwf	PPSLOCK		; PPSLOCK = 0xaa; // magic step 2
	bsf	PPSLOCK,PPSLOCKED;PPSLOCK |= 1<<PPSLOCKED; // re-lock PPS

	;; SSP setup (as modulation clock)
	;; CLC setup (feeding CWG)
	;; CWG setup
	
	zOS_CLC	0,C_RATIO,PIR1,LATA,RA3,isr_ctx
	movlw	OUTCHAR		; zOS_CLC(0,C_RATIO,PIR1,LATA,RA3,isr_ctx);//...
	movwi	0[FSR0]		;} // main()
i=4
	while i < 8
	zOS_ADR	isr_adc,0
	zOS_INT	0/ADIF,ADC#v(i)SWI
	zOS_ADR	adcmeas	
	zOS_LAU	WREG
	zOS_ACT	FSR0
i+=1
	endw
	zOS_RUN	INTCON,INTCON
	end
