	local	p0,p1,wrap,t0scale,isradrl,isradrh,tskadrl,tskadrh,optadrl
	local	optadrh,accumul,accumuh,numbase,destreg,destreh,char_io,buf,max
	;; 0x20~24 reserved for zOS_CON
p0	set	0x20
p1	set	0x21
wrap	set	0x22
t0scale	set	0x23
	
	;; 0x24~28 reserved for zOS_INP
isradrl	set	0x24
isradrh	set	0x25
tskadrl	set	0x26
tskadrh	set	0x27

	;; 0x28~2F reserved for zOS_MON and derivations e.g. zOS_MAN
optadrl	set	0x28
optadrh	set	0x29
accumul	set	0x2a
accumuh	set	0x2b
numbase	set	0x2c	
destreg	set	0x2d
destreh	set	0x2e
char_io	set	0x2f
buf	set	0x30
max	set	0x70		;FIXME: "max" has no purpose (just advisory that
				; local variable space is capped at the bottom
				; of the globals), so it can be nixed

;copy the preceding lines rather than including this file, as definitions for
;zOS_MON()-derived macros referring to these local variables wouldn't open it
;until expansion and would throw an undefined-var error during the processing

