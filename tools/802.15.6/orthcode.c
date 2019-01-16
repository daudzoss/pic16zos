/* preambl4.c
 *
 * generates an includable assembly file (sized 1024 lines of PIC instructions)
 *  with the Table 85 orthogonal codes required of the FS-spreader by 802.15.6
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  static const uint16_t table85[16] = { 0xffff, // 0000
					0xaaaa, // 0001
					0xcccc, // 0010
					0x9999, // 0011

					0xf0f0, // 0100
					0xa5a5, // 0101
					0xc4c4, // 0110
					0x9696, // 0111

					0xff00, // 1000
					0xaa55, // 1001
					0xcc33, // 1010
					0x9966, // 1011

					0xf00f, // 1100
					0xa55a, // 1101
					0xc33c, // 1110
					0x9669};// 1111
  int page, nyb_h, nyb_l, offset = 0;

  for (page = 0; page < 4; page++)
    for (nyb_h = 0; nyb_h < 16; nyb_h++)
      for (nyb_l = 0; nyb_l < 16; nyb_l++) {

	uint32_t code = (table85[nyb_h] << 16) | table85[nyb_l];
	uint8_t byte = 0x000000ff & (code >> (8 * page));

	printf("\tmovlw\t0x%02x\t;offset 0x%03x (page %d, %x followed by %x)\n",
	       byte, offset++, page, nyb_h, nyb_l);
      }
  exit(0);
}
