datodw	macro
	local	opc_bra,zero,offset0,offset1,fsrprei,fsrpred,fsrposi,fsrposd
	local	opc_miw,opc_mwi,opc_lit,opc_mlb,opc_mlp,opc_af0,opc_af1,opc_reg
	local	opc_mov,opc_bit,opccall
	local	opcgoto,opcclrw,opcomni,hexpref,hex_neg
	local	regnam0,regnam1,regnam2,regnam3,regnam4,regnam5
	local	regnam6,regnam7,regnam8,regnam9,regnamA,regnamB

opc_bra
	da	"bra    0x"
zero
	da	"0"
offset0
	da	"[FSR0]",0
offset1
	da	"[FSR1]",0
fsrprei
	da	"++FSR0",0
	da	"++FSR1",0
fsrpred
	da	"--FSR0",0
	da	"--FSR1",0
	
fsrposi
	da	"FSR0++",0
	da	"FSR1++",0
fsrposd
	da	"FSR0--",0
	da	"FSR1--",0
opc_miw	
	da	"moviw  "
opc_mwi
	da	"movwi  "
opc_lit
	da	"addlw  "
	da	"sublw  "
	da	"xorlw  "
	da	"andlw  "
	da	"iorlw  "
	da	"retlw  "
	da	"movlw  "
opc_mlb
	da	"movlb  "
opc_mlp
	da	"movlp  "
opc_af0
	da	"addfsr FSR0,",0
opc_af1
	da	"addfsr FSR1,",0
	
opc_reg
	da	"addwfc "
	da	"subwfb "
	da	"asrf   "
	da	"lsrf   "
	da	"lslf   "
	da	"incfsz "
	da	"swapf  "
	da	"rlf    "
	da	"rrf    "
	da	"decfsz "
	da	"incf   "
	da	"comf   "
	da	"movf   "
	da	"addwf  "
	da	"xorwf  "
	da	"andwf  "
	da	"iorwf  "
	da	"decf   "
	da	"subwf  "
opc_mov
	da	"movwf  "

opc_bit
	da	"bcf    "
	da	"bsf    "
	da	"btfsc  "
	da	"btfss  "
	
opccall
	da	"call   0x"
opcgoto
	da	"goto   0x"
	
opcclrw
	da	"clrw   "
	da	"clrf   "

opcomni
	da	"nop    "
	da	"reset  "
	da	"option "
	da	"sleep  "
	da	"clrwdt "
	da	"tris A "
	da	"tris B "
	da	"tris C "
	da	"return "
	da	"retfie "
	da	"callw  "
	da	"brw    "
	da	"invalid"
	da	"invalid"
	da	"invalid"
	da	"invalid"
	
hexpref
	da	"0x",0
hex_neg
	da	"-0x"
regnam0
	da	"INDF0"
regnam1
	da	"INDF1"
regnam2
	da	"PCL"
regnam3
	da	"STATUS",0
regnam4
	da	"FSR0L"
regnam5
	da	"FSR0H"
regnam6
	da	"FSR1L"
regnam7
	da	"FSR1H"
regnam8
	da	"BSR"
regnam9
	da	"WREG",0
regnamA
	da	"PCLATH",0
regnamB
	da	"INTCON",0
	endm