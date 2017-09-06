	;; 0x20~24 reserved for zOS_CON
;p0		set	0x20
;p1		set	0x21
;wrap		set	0x22
;t0scale	set	0x23
	
	;; 0x24~28 reserved for zOS_INP
;isradrl	set	0x24
;isradrh	set	0x25
;tskadrl	set	0x26
;tskadrh	set	0x27
	
	local	accumul,accumuh,numbase,destreg,destreh,char_io
	;; 0x28~2F reserved for zOS_MON and derivations e.g. zOS_MAN
optadrl	set	0x28
optadrh	set	0x29
accumul	set	0x2a
accumuh	set	0x2b
numbase	set	0x2c	
destreg	set	0x2d
destreh	set	0x2e
char_io	set	0x2f
