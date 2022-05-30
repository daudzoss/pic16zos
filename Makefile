demo_zos.hex: demo_zos.asm zos.inc zosmacro.inc
	  gpasm demo_zos.asm | grep -v 1302

demo_802.hex: demo_802.asm zos.inc zosmacro.inc zos80215.inc
	  gpasm demo_802.asm | grep -v 1302

olirelay.hex: olirelay.asm zos.inc zosmacro.inc
	  gpasm olirelay.asm | grep -v 1302

