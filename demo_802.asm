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
C_RATIO	equ	C_SYSTM/C_UART

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
	include	zos80215.inc

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
	;; clock setup: 16/32MHz internal or 21MHz external
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
	;; pin setup
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
	movwf	RXPPS		; RXPPS = (0<<3)|RA2; // HOST_TX input (to PIC)

	banksel	RA4PPS
	movlw	0x14		;
	movwf	RA4PPS		; RA4PPS = 0x14; // HOST_RX output (from PIC)
	movlw	0x04		;
	movwf	RC4PPS		; RC4PPS = 0x04; // CLC1OUT
	movlw	0x05		;
	movwf	RC5PPS		; RC5PPS = 0x05; // CLC2OUT

	banksel	PPSLOCK
	movlw	0x55		;
	movwf	PPSLOCK		; PPSLOCK = 0x55; // magic step 1
	movlw	0xaa		;
	movwf	PPSLOCK		; PPSLOCK = 0xaa; // magic step 2
	bsf	PPSLOCK,PPSLOCKED;PPSLOCK |= 1<<PPSLOCKED; // re-lock PPS

	;; SSP setup (as modulation bitstream)
	banksel	SSP1CON1
	movlw	0x01		;
	movwf	SSP1ADD		; SSP1ADD = 1; // fCLK = fINSTR/2 = fOSC/4(1+1)
	movlw	0x0a		;
	movwf	SSP1CON1	; SSP1CON1 = 0x0a; // no outputs, idle low, fCLK

	;; CLC setup (xoring with fOSC and feeding CWG)
	banksel	CLC1CON
	movlw	0x28	  	; // SPI SDO bit xor'ed with main oscillator
	movwf	CLC1SEL0	; CLC1SEL0 = 40; // "SDO" line from Table 1
	movwf	CLC2SEL0	; CLC2SEL0 = 40;
	movlw	0x21		;
	movwf	CLC1SEL2	; CLC1SEL2 = 33; // "FOSC" line from Table 1
	movwf	CLC2SEL2	; CLC2SEL2 = 33;
	movlw	0x02		; // only those two inputs into the xor:
	movwf	CLC1GLS0	; CLC1GLS0 = 0x02; // Icxd1T into Icxg1
	movwf	CLC2GLS0	; CLC1GLS0 = 0x02; // Icxd1T into Icxg1
	movlw	0x20		;
	movwf	CLC1GLS2	; CLC1GLS2 = 0x20; // Icxd3T into Icxg3
	movwf	CLC2GLS2	; CLC1GLS2 = 0x20; // Icxd3T into Icxg3
	clrf	CLC1POL		;
	movlw	0x80		; CLC1POL = 0; // no inversions on CLC1, but...
	movwf	CLC2POL	    	; CLC2POL = 0x80; // CLC2 = ~CLC1
	movlw	0x81		;
	movwf	CLC1CON		; CLC1CON = 0x81;
	movwf	CLC2CON		; CLC2CON = 0x81; // enabled, XOR function

	;; TMR1 setup (triggering a modulation, and PU replacement of console)

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
