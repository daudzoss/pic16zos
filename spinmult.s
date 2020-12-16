zOS_M16	macro	mem0,mem1
	local	fact0,fact1,retry,done
	if (mem0 > mem1)
fact0	equ	mem1
fact1	equ	mem0
	else
fact0	equ	mem0
fact1	equ	mem1
	endif
retry
		movf	fact0,w
		zOS_ARG	0
	if (fact0 != zOS_AR0)
	 if (fact0 == INDF0)
		moviw	1[FSR0]
	 else
		movf	1+fact0,w
	 endif
		zOS_ARG	1
	endif
	if (fact1 != zOS_AR2)
		movf	fact1,w
		zOS_ARG	2
	 if (fact1 == INDF1)
		moviw	1[FSR1]
	 else
		movf	1+fact1,w
	 endif
		zOS_ARG	3
	endif

	zOS_MUL
	btfsc	STATUS,Z
	bra	done
	movf	zOS_ME
	zOS_ARG	0
	zOS_SWI	zOS_YLD
	bra	retry
done
	endm

