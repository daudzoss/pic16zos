INSTALLDIr=/opt/microchip/xc8/v3.00
INCPATH=$(INSTALLDIR)/pic/include/proc
ASSEMBLER=$(INSTALLDIR)/pic-as/bin/pic-as
ASMFLAGS=-Wa,-a -save-temps -xassembler-with-cpp


demo_zos.hex: demo_zos.asm zos.inc zosmacro.inc
	  $(ASSEMBLER) -I$(INCPATH) $(ASMFLAGS) -mcpu=16f1719 $< -o $@

demo_802.hex: demo_802.asm zos.inc zosmacro.inc zos80215.inc
	  $(ASSEMBLER) -I$(INCPATH) $(ASMFLAGS) -mcpu=16f1619 $< -o $@

olirelay.hex: olirelay.asm zos.inc zosmacro.inc
	  $(ASSEMBLER) -I$(INCPATH) $(ASMFLAGS) -mcpu=16f1847 $< -o $@

