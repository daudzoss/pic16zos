INSTALLDIR=/opt/microchip/xc8/v3.00
INCPATH=$(INSTALLDIR)/pic/include/proc
ASSEMBLER=$(INSTALLDIR)/pic-as/bin/pic-as
ASMFLAGS=-Wa,-a -save-temps -xassembler-with-cpp

all: demo_zos.hex olirelay.hex

clean:
	rm demo_zos.hex olirelay.hex

demo_zos.hex: demo_zos.asm zos.inc zosmacro.inc
	$(ASSEMBLER) -I$(INCPATH) $(ASMFLAGS) -mcpu=16f1719 $< -o $@

olirelay.hex: olirelay.asm zos.inc zosmacro.inc
	$(ASSEMBLER) -I$(INCPATH) $(ASMFLAGS) -mcpu=16f1847 $< -o $@
