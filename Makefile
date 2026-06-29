all: demo_zos.hex demo_802.hex olirelay.hex

demo_zos.hex: demo_zos.asm zos.inc zosmacro.inc
	  gpasm -D__GPUTILS_VERSION_MAJOR demo_zos.asm | grep -v 302 | grep -v 305

demo_802.hex: demo_802.asm zos.inc zosmacro.inc zos80215.inc
	  gpasm -D__GPUTILS_VERSION_MAJOR demo_802.asm | grep -v 302 | grep -v 305

olirelay.hex: olirelay.asm zos.inc zosmacro.inc
	  gpasm -D__GPUTILS_VERSION_MAJOR olirelay.asm | grep -v 302 | grep -v 305
