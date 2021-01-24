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
	
	processor 16f1619
	include p16f1619.inc
	
;;; 16-QFN 20-QFN ana. dig. purpose
;;; pin 12 pin 16 AN0  RA0  AN0/DAC/ICSPDAT
;;; pin 11 pin 15 AN1  RA1  AN1
;;; pin 10 pin 14 AN2  RA2  AN2
;;; pin 3  pin 1       RA3  MCLR_N
;;; pin 2  pin 20 AN3  RA4  AN3
;;; pin 1  pin 19      RA5  
;;;;       pin 10 AN10 RB4  
;;;;       pin 9  AN9  RB5  
;;;;       pin 8       RB6  
;;;;       pin 7       RB7  
;;; pin 9  pin 13 AN4  RC0  AN4
;;; pin 8  pin 12 AN5  RC1  AN5
;;; pin 7  pin 11 AN6  RC2  AN6
;;; pin 6  pin 4  AN7  RC3  AN7
;;; pin 5  pin 3       RC4
;;; pin 4  pin 2       RC5
;;;;       pin 5  AN8  RC6
;;;;       pin 6  AN9  RC7

	include	zos.inc
	include	zosmacro.inc

OUTCHAR	equ	zOS_SI3
AN4xAN0	equ	zOS_SI4
AN5xAN1	equ	zOS_SI5	
AN2xAN6	equ	zOS_SI6
AN3xAN7	equ	zOS_SI7	

	pagesel	main
	goto	main

	zOS_NAM	"ADC control, buffer"
adcmeas
	bra

isr_adc

isr_ctx

main
	banksel	OSCCON

	banksel	ANSELA
	banksel	ANSELC

	banksel	OPTION_REG

	banksel	TRISC
	
	banksel	PPSLOCK
	banksel	RC7PPS
	
	zOS_CLC	0,.032000000/.000009600,PIR1,LATA,RA3,isr_ctx
	movlw	OUTCHAR		;
	movwi	0[FSR0]		; zOS_CLC(/*TX*/0,32MHz/9600bps,PIR1,LATA,mCLR);

	zOS_ADR	isr_adc,0
	zOS_INT	ADC_HWI,AN2xAN6
	zOS_ADR	adcmeas	
	zOS_LAU	WREG
	zOS_ACT	FSR0
	
	zOS_ADR	isr_adc,0
	zOS_INT	ADC_HWI,AN3xAN7
	zOS_ADR	adcmeas	
	zOS_LAU	WREG
	zOS_ACT	FSR0
	
	zOS_ADR	isr_adc,0
	zOS_INT	ADC_HWI,AN4xAN0
	zOS_ADR	adcmeas	
	zOS_LAU	WREG
	zOS_ACT	FSR0
	
	zOS_ADR	isr_adc,0
	zOS_INT	ADC_HWI,AN5xAN1
	zOS_ADR	adcmeas	
	zOS_LAU	WREG
	zOS_ACT	FSR0
	
	zOS_RUN	INTCON,INTCON
	end
