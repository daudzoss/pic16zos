#ifndef zOS_FRE
 error "must define zOS_FRE with lowest linear memory address available for heap before including this file"	
#endif
	
#ifndef MAXSRAM
 error "must define MAXSRAM with 1 + highest linear memory address available for heap before including this file"	
#endif

HEAPRAM	equ	MAXSRAM-zOS_FRE
HEAPSML	equ	HEAPRAM/4
HEAPLRG	equ	HEAPSML*3
HEAPTHR	equ	7
HEAP1	equ	zOS_FRE
HEAP2	equ	zOS_FRE+HEAPSML

#ifdef LMALLOC
	
	zOS_HEA	HEAP1,HEAPSML,SMALLOC,SFREE
	movlw	SMALLOC|SFREE
	zOS_ARG	3
	zOS_LAU	WREG

	zOS_HEA	HEAP2,HEAPLRG,LMALLOC,LFREE
	movlw	LMALLOC|LFREE
	zOS_ARG	3
	zOS_LAU	WREG

#else
#ifdef SMALLOC

	zOS_HEA	HEAP1,HEAPRAM,SMALLOC,SFREE
	movlw	SMALLOC|SFREE
	zOS_ARG	3
	zOS_LAU	WREG
	
#else
 error "must define SMALLOC and SFREE software interrupt masks (and optionally LMALLOC and LFREE) before including this file"
#endif
#endif	
	
	bra	endalloc
	
malloc
	zOS_ARG	0		;void* malloc(uint8_t w) { // w is numbytes/16
#ifdef LMALLOC
	addlw	0-HEAPTHR	; zOS_ARG(0, w); // turns interrupts off
	btfss	WREG,7		; if (w <= HEAPTHR)
	bra	bigallo		;  w = zOS_SWI(SMALLOC); // allocated address/16
#endif
	zOS_SWI	SMALLOC
	movf	WREG		; if ((w == 0) || (w > HEAPTHR)) // too big/full
	btfss	STATUS,Z	;  w = zOS_SWI(LMALLOC); // allocated address/16
	return			; return w;
#ifdef LMALLOC
bigallo
	zOS_SWI	LMALLOC
#endif
	return			;}
	

	;; large-bytecount (128=16*HEAPTHR+16) table has fewer entries so faster
free
	zOS_ARG	0		;uint8_t free(void* w) { // w is address/16
#ifdef LMALLOC
	zOS_SWI	LFREE
	btfss	STATUS,Z	; zOS_ARG(0, w); // turns interrupts off
	return			; return (w=zOS_SWI(LFREE)) ? w: zOS_SWI(SFREE);
#endif
	zOS_SWI	SFREE
	return			;}
	
endalloc
