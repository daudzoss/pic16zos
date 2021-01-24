;;; demo to measure AN2*AN6 (or just AN2)
;;;                 AN3*AN7 (or just AN3)
;;;                 AN4*AN0 (or just AN4)
;;;                 AN5*AN1 (or just AN5
;;; and transmit them periodically (with TMR2:TMR1 timestamps)
;;;  according to IEEE 802.15.6 HBC protocol
;;; console automatically quits and starts
;;;  app if no input within TBD sec
;;;
	
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
