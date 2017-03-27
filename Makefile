demo_zos.hex: demo_zos.asm zos.inc zosmacro.inc
	  gpasm demo_zos.asm -DGPASM

olirelay.hex: olirelay.asm zos.inc zosmacro.inc
	  gpasm olirelay.asm -DGPASM

