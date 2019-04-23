	processor	16f18325
	include		p16f18325.inc
	include		../../zos80215.inc

	org	0x1000
	nop
	nop
	zOS_HBC	1,0xa0,0x2000,table,0x79,0x7a,0x7b

	org	0x1800
table
	include	../../orthcode.inc
	end
	
