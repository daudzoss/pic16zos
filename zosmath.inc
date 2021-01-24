;;; MULTACC
;;;
;;; multiplies two 8-bit unsigned numbers together for a 16-bit unsigned result
;;; (one of them must occupy the lower byte of a 16-bit register, upper cleared)
;;; prod16 should be cleared unless for multiply-accumulate with a prior result
MULTACC	macro	fact8,fact16,prod16	
#if 0;def zOS_MUL
	if (fact8 & 0x7e)
	 if (fact16 & 0x7e)
	  zOS_MUL fact16,fact8,0
	  if (fact16^prod16)
	   movf	fact16,w
	   addwf prod16
	   movf	1+fact16,w
	   addwfc 1+prod16
	  endif
	 else
	  error "bad arg for fact8"
	 endif
	else
	error "bad arg for fact16"
	endif
#else
	local	mulbegin,mulnext,muldone
	bcf	STATUS,C	;inline void MULTACC(uint8_t* fact8,
mulbegin	
	rrf	fact8,f		;             uint16_t* fact16,
	btfss	STATUS,C	;             uint16_t* prod16) {
	bra	mulnext		;
	movf	fact16,w	;
	addwf	prod16,f	;
	movf	1+fact16,w	;
	addwfc	1+prod16,f	;
	bcf	STATUS,C	;
mulnext
	rlf	fact16,f	;
	rlf	1+fact16,f	; *prod16 += *fact8 * (*fact16 & 0x00ff);
	movf	fact8,f		;
	btfss	STATUS,Z	;
	bra	mulbegin	;}
muldone
#endif
	endm

;;; DIVIDE
;;; 
;;; a higher dynamic range division that truncates only to a non-zero quotient;
;;; if the quotient is zero a rough logarithm is returned instead
;;; 
;;; dividend: uint15_t (0..32767)
;;; divisor: uint8_t (1..255)
;;; quotient: uint8_t (0..255, values in the range 256..32767 will clip to 511)
;;; quo_hi: uint8_t (256..32767, top byte of result if not overridden by 0 arg)
;;; WREG
;;; |   Z flag in STATUS
;;; |   |
;;; 0   0 divide-by-zero error (because divisor passed was zero)
;;; 0   1 actual quotient is exactly zero (because dividend passed was zero)
;;; >0  0 actual quotient was rounded to nearest integer (or clipped to 8 bits)
;;; >0  1 actual quotient was less than 1, WREG holds fractional remainder bits
;;;
;;; if fractional values between 0 and 1 are not relevant, to ignore the final
;;; condition in the table just follow the invocation with "btfsc STATUS,Z:clrw"
DIVIDE	macro	dividend,divisor,quotient,quo_hi
	local	divnot0,divsub1,divsubc,divloop,divchek,divbias,divroll,divdone
	clrf	quotient	;inline uint8_t DIVIDE(uint16_t* dividend,
	if (quo_hi)
	 clrf	quo_hi		;                      uint8_t divisor,
	endif
	comf	divisor,w	;                      uint8_t* quotient,
	incfsz	WREG		;                      uint8_t* quo_hi) {
	bra	divnot0		; if (divisor == 0)
	bra	divdone		;  return 0; // Z flag will be clear if div by 0
divnot0
	movf	dividend,f	; else if (*dividend == 0)
	btfss	STATUS,Z	;
	bra	divsub1		;
	movf	1+dividend,f	;
	btfss	STATUS,Z	;
	bra	divsub1		;
	clrw			;
	bra	divdone		;  return 0; // Z flag will be set if actually 0
divsub1
	decf	dividend,f	; else {
	incfsz	dividend,f	;  // subtract one from dividend to ease compare
	bra	divsubc		;
	decf	1+dividend,f	;
divsubc
	decf	dividend,f	;
divloop
	btfsc	1+dividend,7	;  uint16_t result = (*dividend) / divisor;
	bra	divchek		;
	addwf	dividend,f	;
	btfsc	STATUS,C	;  if (result > 0x00ff) {
	incf	1+dividend,f	;   if (quo_hi) {
	decf	1+dividend,f	;
	incfsz	quotient,f	;    *quotient = result & 0x00ff;
	bra	divloop		;    *quo_hi = result >> 8;
	if (quo_hi)
	 incf	quo_hi,f	;   } else
	 bra	divloop		;    *quotient = 0xff; // clamp to 8 bits
	else
	 decf	quotient,f	;  }
	endif
divchek
	sublw	0		;  // works with non-static divisor, not re-read
	addwf	dividend,f	;  if (*quotient)
	xorwf	dividend,f	;   return *quotient; // Z flag will be clear
	movf	quotient,w	;  else { // count # fewer leading 0s in divisor
	btfss	STATUS,Z	;   uint8_t w = 0;
	bra	divdone		;
	clrw			;   *dividend ^= divisor;
	addwfc	1+dividend,f	;   while (*dividend & 0x80 == 0)
divbias
	rlf	dividend,f	;    *dividend <<= 1;
	btfss	STATUS,C	;   while (*dividend & 0x80) {
	bra	divbias		;    *dividend <<= 1;
divroll
	rrf	WREG		;    w = 0x80 | (w >> 1);
	rlf	dividend,f	;   }
	btfsc	STATUS,C	;   return w; // Z flag will be set
	bra	divroll		;  }
	comf	WREG		; }
	movf	quotient,f	;} // DIVIDE()
divdone
	endm
