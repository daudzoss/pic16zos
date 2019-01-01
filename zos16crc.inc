;;; adapted from 15.04.1999 CRC16_04.ASM original by Microchip's Thomas Schmidt
;;; http://ww1.microchip.com/downloads/en/appnotes/00730a.pdf
	
#define	PolynomLow	b'00000101'
#define	PolynomHigh	b'10000000'
#define	PolynomLength	0x10
#define	Iterations	8

CRCini	macro	crc16,crc_buff,bits
	moviw	FSR1++		;
	movwf	crc16		;
	moviw	FSR1++		;
	movwf	1+crc16		;
	movf	INDF1,w		;
	movwf	crc_buff	;
	movlw	Iterations	;
	movwf	bits		;
	endm
	
CRCeng	macro	crc16,crc_buff,bits
	local	CRC16Engine,NextBitEngine
	
CRC16Engine
	bcf	STATUS,C	;
	rlf	crc_buff,f	;
	rlf	1+crc_buff,f	;
	rlf	crc_buff,f	;
	btfss	STATUS,C	;
	bra	NextBitEngine	;
	
	movlw	PolynomHigh	;
	xorwf	crc_buff,f	;
	movlw	PolynomLow	;
	xorwf	1+crc_buff,f	;
NextBitEngine
	decfsz	bits,f		;
	bra	CRC16Engine	;
	endm
	
zOS_CRC	macro	crc16,crc_buff,bits,temp
;;; FSR0 must be pointing to after last item of data
;;; 	(where two bytes of CRC will be written BIG ENDIAN)
;;; FSR1 must be pointing to first item of data, at least 2 bytes less than FSR0
	local	NextCRC16,AppendZeroes,Reload,ReturnToMain
	
	CRCini	crc16,crc_buff,bits
	movlw	3
	movwf	temp
NextCRC16
	CRCeng	crc16,crc_buff,bits
	movf	FSR0L,w		;
	xorwf	FSR1L,w		;
	btfss	STATUS,Z	;
	bra	AppendZeroes	;
	movf	FSR0H,w		;
	xorwf	FSR1H,w		;
	btfss	STATUS,Z	;
	bra	Reload

	decfsz	temp,f		;
	bra	AppendZeroes	;
	bra	ReturnToMain	;
AppendZeroes
	clrf	crc_buff	;
	movlw	Iterations	;
	movwf	bits		;
	bra	NextCRC16	;
	
	;; Reload CRC buffer register with new data word.
Reload
	moviw	++FSR1		;
	movwf	crc_buff	;
	movlw	Iterations	;
	movwf	bits		;
	bra	NextCRC16	;
ReturnToMain
	;; append CRC to message
	movf	crc16,w		;
	movwi	FSR0++		;
	movf	1+crc16,w	;
	movwi	FSR0++		;
	endm


zOS_RCR	macro	crc16,crc_buff,bits,databytes,temp
	local	NextCRCRestore,ReloadRestore,ReturnToMain

;;; uses only FSR1 so that INDF0 can be the size variable
	CRCini	crc16,crc_buff,bits
	movlw	0x02		;
	addwf	databytes,f	;
NextCRCRestore
	CRCeng	crc16,crc_buff,bits
	decfsz	databytes,f	;
	bra	ReloadRestore	;

	; check if crc16 equal to zero (success)
	movf	crc16,w		;
	iorwf	1+crc16,w	;
	bra	ReturnToMain	;

	;; Reload CRC buffer register with new data word.
ReloadRestore
	moviw	++FSR1		;
	movwf	crc_buff	;
	movlw	Iterations	;
	movwf	bits		;
	bra	NextCRCRestore	;
ReturnToMain			;
	endm


zOS_C16	macro	putchar_swi,crc16,crc_buff,bits,databytes,temp
;decode in linear data space using global register interface where 1[] is starting address low byte and 0[] is length (not including the CRC)
;answer comes back in 1[] with zero in 0[] at which point a new CRC validation can be attempted
	local	crcdecl,crctask,crcreset,crcloop
	bra	crcdecl
	zOS_NAM	"CRC validator"
crctask
	zOS_SWI	zOS_YLD
	zOS_MY2	FSR0
crcreset
	clrf	INDF0		;
crcloop
	zOS_SWI	zOS_YLD
	zOS_MY2	FSR0
	movf	INDF0,w		;
	btfsc	STATUS,Z	;
	bra	crcloop		;
	movwf	FSR1L		;
	movlw	0x20		;
	movwf	FSR1H		;

	zOS_RCR	crc16,crc_buff,bits,databytes,temp
	movwi	1[FSR0]
	if (putchar_swi)
	movlw	0
	zOS_ARG	0
	movf	crc16,w		;
	zOS_ARG	1
	zOS_SWI	putchar_swi
	movlw	0
	zOS_ARG	0
	movf	1+crc16,w	;
	zOS_ARG	1
	zOS_SWI	putchar_swi
	endif
crcdecl	
	zOS_ADR	crctask,zOS_UNP
	zOS_LAU
	zOS_ACT	FSR0
	endm

;;; e.g. zOS_RCR zOS_SI7,0x78,0x7a,0x7b,INDF0,PCLATH