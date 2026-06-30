0000:  3181  movlp   0x01
0001:  2969  goto    0x0169
0002:  3180  movlp   0x00
0003:  28be  goto    0x00be
0004:  3006  movlw   0x06
0005:  00f0  movwf   0x70
0006:  0e70  swapf   0x70, 0x0
0007:  3e10  addlw   0x10
0008:  3970  andlw   0x70
0009:  0084  movwf   0x04
000a:  0185  clrf    0x05
000b:  30f0  movlw   0xf0
000c:  0784  addwf   0x04, 0x1
000d:  03f0  decf    0x70, 0x1
000e:  1903  btfsc   0x03, 0x2
000f:  33f4  bra     0x0004
0010:  3f03  moviw   .3[0]
0011:  1903  btfsc   0x03, 0x2
0012:  33f8  bra     0x000b
0013:  0064  clrwdt
0014:  0020  movlb   0x00
0015:  3f0e  moviw   .14[0]
0016:  1903  btfsc   0x03, 0x2
0017:  33f3  bra     0x000b
0018:  050b  andwf   0x0b, 0x0
0019:  1d03  btfss   0x03, 0x2
001a:  320e  bra     0x0029
001b:  0021  movlb   0x01
001c:  3f0e  moviw   .14[0]
001d:  0511  andwf   0x11, 0x0
001e:  1d03  btfss   0x03, 0x2
001f:  3209  bra     0x0029
0020:  3f0e  moviw   .14[0]
0021:  0512  andwf   0x12, 0x0
0022:  1d03  btfss   0x03, 0x2
0023:  3205  bra     0x0029
0024:  3f0e  moviw   .14[0]
0025:  0513  andwf   0x13, 0x0
0026:  1d03  btfss   0x03, 0x2
0027:  3201  bra     0x0029
0028:  33e2  bra     0x000b
0029:  01f1  clrf    0x71
002a:  3f0d  moviw   .13[0]
002b:  008a  movwf   0x0a
002c:  3f0c  moviw   .12[0]
002d:  0082  movwf   0x02
002e:  003f  movlb   0x1f
002f:  00e5  movwf   0x65
0030:  003f  movlb   0x1f
0031:  0866  movf    0x66, 0x0
0032:  1903  btfsc   0x03, 0x2
0033:  327e  bra     0x00b2
0034:  00f0  movwf   0x70
0035:  3003  movlw   0x03
0036:  00f1  movwf   0x71
0037:  327c  bra     0x00b4
0038:  3010  movlw   0x10
0039:  0784  addwf   0x04, 0x1
003a:  0af0  incf    0x70, 0x1
003b:  30fa  movlw   0xfa
003c:  0770  addwf   0x70, 0x0
003d:  1f89  btfss   0x09, 0x7
003e:  3274  bra     0x00b3
003f:  3f03  moviw   .3[0]
0040:  1903  btfsc   0x03, 0x2
0041:  33f6  bra     0x0038
0042:  0064  clrwdt
0043:  1b89  btfsc   0x09, 0x7
0044:  33f3  bra     0x0038
0045:  0866  movf    0x66, 0x0
0046:  0670  xorwf   0x70, 0x0
0047:  1903  btfsc   0x03, 0x2
0048:  3269  bra     0x00b2
0049:  0e66  swapf   0x66, 0x0
004a:  3e10  addlw   0x10
004b:  3970  andlw   0x70
004c:  3e02  addlw   0x02
004d:  0084  movwf   0x04
004e:  0185  clrf    0x05
004f:  086e  movf    0x6e, 0x0
0050:  001a  movwi   0++
0051:  086f  movf    0x6f, 0x0
0052:  001a  movwi   0++
0053:  0864  movf    0x64, 0x0
0054:  001a  movwi   0++
0055:  0865  movf    0x65, 0x0
0056:  001a  movwi   0++
0057:  086d  movf    0x6d, 0x0
0058:  001a  movwi   0++
0059:  0867  movf    0x67, 0x0
005a:  001a  movwi   0++
005b:  0868  movf    0x68, 0x0
005c:  001a  movwi   0++
005d:  0869  movf    0x69, 0x0
005e:  001a  movwi   0++
005f:  086a  movf    0x6a, 0x0
0060:  001a  movwi   0++
0061:  086b  movf    0x6b, 0x0
0062:  001a  movwi   0++
0063:  307c  movlw   0x7c
0064:  0684  xorwf   0x04, 0x1
0065:  1d03  btfss   0x03, 0x2
0066:  3208  bra     0x006f
0067:  300a  movlw   0x0a
0068:  0085  movwf   0x05
0069:  3072  movlw   0x72
006a:  0084  movwf   0x04
006b:  0103  clrw
006c:  001a  movwi   0++
006d:  0b85  decfsz  0x05, 0x1
006e:  33fd  bra     0x006c
006f:  3020  movlw   0x20
0070:  0086  movwf   0x06
0071:  3003  movlw   0x03
0072:  0087  movwf   0x07
0073:  0870  movf    0x70, 0x0
0074:  0266  subwf   0x66, 0x0
0075:  1903  btfsc   0x03, 0x2
0076:  321f  bra     0x0096
0077:  0309  decf    0x09, 0x0
0078:  1b89  btfsc   0x09, 0x7
0079:  3e05  addlw   0x05
007a:  00ed  movwf   0x6d
007b:  35ed  lslf    0x6d, 0x1
007c:  076d  addwf   0x6d, 0x0
007d:  3e02  addlw   0x02
007e:  00ed  movwf   0x6d
007f:  00f1  movwf   0x71
0080:  086e  movf    0x6e, 0x0
0081:  001e  movwi   0x1++
0082:  086f  movf    0x6f, 0x0
0083:  001e  movwi   0x1++
0084:  03ed  decf    0x6d, 0x1
0085:  300e  movlw   0x0e
0086:  1a6d  btfsc   0x6d, 0x4
0087:  00ed  movwf   0x6d
0088:  0871  movf    0x71, 0x0
0089:  066d  xorwf   0x6d, 0x0
008a:  1d03  btfss   0x03, 0x2
008b:  33f4  bra     0x0080
008c:  01ed  clrf    0x6d
008d:  0015  moviw   --0x1
008e:  00ef  movwf   0x6f
008f:  0015  moviw   --0x1
0090:  00ee  movwf   0x6e
0091:  0a6d  incf    0x6d, 0x0
0092:  00ed  movwf   0x6d
0093:  3c0e  sublw   0x0e
0094:  1f89  btfss   0x09, 0x7
0095:  33f7  bra     0x008d
0096:  0e70  swapf   0x70, 0x0
0097:  3e10  addlw   0x10
0098:  3970  andlw   0x70
0099:  3e04  addlw   0x04
009a:  0084  movwf   0x04
009b:  0185  clrf    0x05
009c:  0012  moviw   0++
009d:  00e4  movwf   0x64
009e:  0012  moviw   0++
009f:  00e5  movwf   0x65
00a0:  0870  movf    0x70, 0x0
00a1:  00e6  movwf   0x66
00a2:  0010  moviw   ++0
00a3:  00e7  movwf   0x67
00a4:  0010  moviw   ++0
00a5:  00e8  movwf   0x68
00a6:  0010  moviw   ++0
00a7:  00e9  movwf   0x69
00a8:  0010  moviw   ++0
00a9:  00ea  movwf   0x6a
00aa:  0010  moviw   ++0
00ab:  00eb  movwf   0x6b
00ac:  3f3b  moviw   -.5[0]
00ad:  00ed  movwf   0x6d
00ae:  3f37  moviw   -.9[0]
00af:  00ee  movwf   0x6e
00b0:  3f38  moviw   -.8[0]
00b1:  00ef  movwf   0x6f
00b2:  0009  retfie
00b3:  01f0  clrf    0x70
00b4:  0e70  swapf   0x70, 0x0
00b5:  3e10  addlw   0x10
00b6:  3970  andlw   0x70
00b7:  0084  movwf   0x04
00b8:  0185  clrf    0x05
00b9:  0bf1  decfsz  0x71, 0x1
00ba:  337d  bra     0x0038
00bb:  3348  bra     0x0004
00bc:  00f1  movwf   0x71
00bd:  3213  bra     0x00d1
00be:  138b  bcf     0x0b, 0x7
00bf:  00f1  movwf   0x71
00c0:  0803  movf    0x03, 0x0
00c1:  00f0  movwf   0x70
00c2:  0808  movf    0x08, 0x0
00c3:  003f  movlb   0x1f
00c4:  00e6  movwf   0x66
00c5:  0870  movf    0x70, 0x0
00c6:  00e4  movwf   0x64
00c7:  080a  movf    0x0a, 0x0
00c8:  00e7  movwf   0x67
00c9:  0804  movf    0x04, 0x0
00ca:  00e8  movwf   0x68
00cb:  0805  movf    0x05, 0x0
00cc:  00e9  movwf   0x69
00cd:  0806  movf    0x06, 0x0
00ce:  00ea  movwf   0x6a
00cf:  0807  movf    0x07, 0x0
00d0:  00eb  movwf   0x6b
00d1:  3181  movlp   0x01
00d2:  0871  movf    0x71, 0x0
00d3:  39f8  andlw   0xf8
00d4:  1d03  btfss   0x03, 0x2
00d5:  294b  goto    0x014b
00d6:  0871  movf    0x71, 0x0
00d7:  3a04  xorlw   0x04
00d8:  1903  btfsc   0x03, 0x2
00d9:  326e  bra     0x0148
00da:  0866  movf    0x66, 0x0
00db:  0088  movwf   0x08
00dc:  08f1  movf    0x71, 0x1
00dd:  1d03  btfss   0x03, 0x2
00de:  3223  bra     0x0102
00df:  01f0  clrf    0x70
00e0:  0e70  swapf   0x70, 0x0
00e1:  3e10  addlw   0x10
00e2:  3970  andlw   0x70
00e3:  0086  movwf   0x06
00e4:  0187  clrf    0x07
00e5:  3010  movlw   0x10
00e6:  0786  addwf   0x06, 0x1
00e7:  0af0  incf    0x70, 0x1
00e8:  30fa  movlw   0xfa
00e9:  0770  addwf   0x70, 0x0
00ea:  1903  btfsc   0x03, 0x2
00eb:  3212  bra     0x00fe
00ec:  3f43  moviw   .3[1]
00ed:  1d03  btfss   0x03, 0x2
00ee:  33f6  bra     0x00e5
00ef:  0804  movf    0x04, 0x0
00f0:  3fc0  movwi   .0[1]
00f1:  0805  movf    0x05, 0x0
00f2:  3fc1  movwi   .1[1]
00f3:  0888  movf    0x08, 0x1
00f4:  1903  btfsc   0x03, 0x2
00f5:  321f  bra     0x0115
00f6:  0e08  swapf   0x08, 0x0
00f7:  3e10  addlw   0x10
00f8:  3970  andlw   0x70
00f9:  0084  movwf   0x04
00fa:  0185  clrf    0x05
00fb:  3f01  moviw   .1[0]
00fc:  1b89  btfsc   0x09, 0x7
00fd:  3217  bra     0x0115
00fe:  01f0  clrf    0x70
00ff:  3180  movlp   0x00
0100:  0870  movf    0x70, 0x0
0101:  282e  goto    0x002e
0102:  0808  movf    0x08, 0x0
0103:  00f0  movwf   0x70
0104:  1903  btfsc   0x03, 0x2
0105:  3208  bra     0x010e
0106:  0e08  swapf   0x08, 0x0
0107:  3e10  addlw   0x10
0108:  3970  andlw   0x70
0109:  0086  movwf   0x06
010a:  0187  clrf    0x07
010b:  3f41  moviw   .1[1]
010c:  1f89  btfss   0x09, 0x7
010d:  3207  bra     0x0115
010e:  087c  movf    0x7c, 0x0
010f:  0088  movwf   0x08
0110:  0e08  swapf   0x08, 0x0
0111:  3e10  addlw   0x10
0112:  3970  andlw   0x70
0113:  0086  movwf   0x06
0114:  0187  clrf    0x07
0115:  0871  movf    0x71, 0x0
0116:  000b  brw
0117:  3207  bra     0x011f
0118:  3210  bra     0x0129
0119:  3215  bra     0x012f
011a:  3220  bra     0x013b
011b:  322c  bra     0x0148
011c:  322b  bra     0x0148
011d:  322a  bra     0x0148
011e:  3229  bra     0x0148
011f:  087c  movf    0x7c, 0x0
0120:  3fcc  movwi   .12[1]
0121:  087d  movf    0x7d, 0x0
0122:  397f  andlw   0x7f
0123:  3fcd  movwi   .13[1]
0124:  087e  movf    0x7e, 0x0
0125:  3fce  movwi   .14[1]
0126:  087f  movf    0x7f, 0x0
0127:  3fcf  movwi   .15[1]
0128:  3212  bra     0x013b
0129:  3f43  moviw   .3[1]
012a:  3880  iorlw   0x80
012b:  3fc3  movwi   .3[1]
012c:  3180  movlp   0x00
012d:  0870  movf    0x70, 0x0
012e:  282e  goto    0x002e
012f:  0808  movf    0x08, 0x0
0130:  003f  movlb   0x1f
0131:  0666  xorwf   0x66, 0x0
0132:  1903  btfsc   0x03, 0x2
0133:  01ef  clrf    0x6f
0134:  0666  xorwf   0x66, 0x0
0135:  0088  movwf   0x08
0136:  0103  clrw
0137:  3fc3  movwi   .3[1]
0138:  3180  movlp   0x00
0139:  0870  movf    0x70, 0x0
013a:  282e  goto    0x002e
013b:  3f40  moviw   .0[1]
013c:  3fc2  movwi   .2[1]
013d:  3f41  moviw   .1[1]
013e:  3880  iorlw   0x80
013f:  3fc3  movwi   .3[1]
0140:  300b  movlw   0x0b
0141:  3fc6  movwi   .6[1]
0142:  3570  lslf    0x70, 0x0
0143:  3870  iorlw   0x70
0144:  0086  movwf   0x06
0145:  0103  clrw
0146:  3fc0  movwi   .0[1]
0147:  3fc1  movwi   .1[1]
0148:  3180  movlp   0x00
0149:  0870  movf    0x70, 0x0
014a:  282e  goto    0x002e
014b:  003f  movlb   0x1f
014c:  0a66  incf    0x66, 0x0
014d:  0ff1  incfsz  0x71, 0x1
014e:  3006  movlw   0x06
014f:  03f1  decf    0x71, 0x1
0150:  00f0  movwf   0x70
0151:  0e70  swapf   0x70, 0x0
0152:  3e10  addlw   0x10
0153:  3970  andlw   0x70
0154:  0084  movwf   0x04
0155:  0185  clrf    0x05
0156:  30f0  movlw   0xf0
0157:  0784  addwf   0x04, 0x1
0158:  03f0  decf    0x70, 0x1
0159:  1903  btfsc   0x03, 0x2
015a:  320c  bra     0x0167
015b:  3f03  moviw   .3[0]
015c:  1903  btfsc   0x03, 0x2
015d:  33f8  bra     0x0156
015e:  3f0f  moviw   .15[0]
015f:  0571  andwf   0x71, 0x0
0160:  1903  btfsc   0x03, 0x2
0161:  33f4  bra     0x0156
0162:  00f1  movwf   0x71
0163:  3f0d  moviw   .13[0]
0164:  008a  movwf   0x0a
0165:  3f0c  moviw   .12[0]
0166:  0082  movwf   0x02
0167:  3180  movlp   0x00
0168:  282e  goto    0x002e
0169:  0020  movlb   0x00
016a:  307f  movlw   0x7f
016b:  0084  movwf   0x04
016c:  0185  clrf    0x05
016d:  0103  clrw
016e:  001b  movwi   0--
016f:  3060  movlw   0x60
0170:  0504  andwf   0x04, 0x0
0171:  1d03  btfss   0x03, 0x2
0172:  33fa  bra     0x016d
0173:  0021  movlb   0x01
0174:  3070  movlw   0x70
0175:  0099  movwf   0x19
0176:  3080  movlw   0x80
0177:  009a  movwf   0x1a
0178:  3000  movlw   0x00
0179:  0098  movwf   0x18
017a:  1f1a  btfss   0x1a, 0x6
017b:  33fe  bra     0x017a
017c:  0023  movlb   0x03
017d:  30af  movlw   0xaf
017e:  008c  movwf   0x0c
017f:  303c  movlw   0x3c
0180:  008e  movwf   0x0e
0181:  0021  movlb   0x01
0182:  1195  bcf     0x15, 0x3
0183:  1295  bcf     0x15, 0x5
0184:  0021  movlb   0x01
0185:  120c  bcf     0x0c, 0x4
0186:  307f  movlw   0x7f
0187:  008e  movwf   0x0e
0188:  003c  movlb   0x1c
0189:  3055  movlw   0x55
018a:  008f  movwf   0x0f
018b:  30aa  movlw   0xaa
018c:  008f  movwf   0x0f
018d:  100f  bcf     0x0f, 0x0
018e:  3016  movlw   0x16
018f:  00a4  movwf   0x24
0190:  003d  movlb   0x1d
0191:  3014  movlw   0x14
0192:  00a7  movwf   0x27
0193:  3055  movlw   0x55
0194:  008f  movwf   0x0f
0195:  30aa  movlw   0xaa
0196:  008f  movwf   0x0f
0197:  140f  bsf     0x0f, 0x0
0198:  3183  movlp   0x03
0199:  2baa  goto    0x03aa
019a:  082f  movf    0x2f, 0x0
019b:  1903  btfsc   0x03, 0x2
019c:  0008  return
019d:  3a47  xorlw   0x47
019e:  1d03  btfss   0x03, 0x2
019f:  3213  bra     0x01b3
01a0:  01af  clrf    0x2f
01a1:  082a  movf    0x2a, 0x0
01a2:  1903  btfsc   0x03, 0x2
01a3:  0008  return
01a4:  138b  bcf     0x0b, 0x7
01a5:  00fc  movwf   0x7c
01a6:  01aa  clrf    0x2a
01a7:  01ab  clrf    0x2b
01a8:  01ac  clrf    0x2c
01a9:  15ac  bsf     0x2c, 0x3
01aa:  14ac  bsf     0x2c, 0x1
01ab:  304a  movlw   0x4a
01ac:  00af  movwf   0x2f
01ad:  3005  movlw   0x05
01ae:  3180  movlp   0x00
01af:  2002  call    0x0002
01b0:  3907  andlw   0x07
01b1:  1903  btfsc   0x03, 0x2
01b2:  01af  clrf    0x2f
01b3:  082f  movf    0x2f, 0x0
01b4:  3a48  xorlw   0x48
01b5:  1d03  btfss   0x03, 0x2
01b6:  3218  bra     0x01cf
01b7:  01af  clrf    0x2f
01b8:  082a  movf    0x2a, 0x0
01b9:  042b  iorwf   0x2b, 0x0
01ba:  1903  btfsc   0x03, 0x2
01bb:  0008  return
01bc:  082a  movf    0x2a, 0x0
01bd:  138b  bcf     0x0b, 0x7
01be:  00fc  movwf   0x7c
01bf:  082b  movf    0x2b, 0x0
01c0:  00fd  movwf   0x7d
01c1:  01aa  clrf    0x2a
01c2:  01ab  clrf    0x2b
01c3:  01ac  clrf    0x2c
01c4:  15ac  bsf     0x2c, 0x3
01c5:  14ac  bsf     0x2c, 0x1
01c6:  304a  movlw   0x4a
01c7:  00af  movwf   0x2f
01c8:  3007  movlw   0x07
01c9:  3180  movlp   0x00
01ca:  2002  call    0x0002
01cb:  3907  andlw   0x07
01cc:  00aa  movwf   0x2a
01cd:  1903  btfsc   0x03, 0x2
01ce:  01af  clrf    0x2f
01cf:  082f  movf    0x2f, 0x0
01d0:  3a49  xorlw   0x49
01d1:  1d03  btfss   0x03, 0x2
01d2:  3227  bra     0x01fa
01d3:  01af  clrf    0x2f
01d4:  082d  movf    0x2d, 0x0
01d5:  01ad  clrf    0x2d
01d6:  138b  bcf     0x0b, 0x7
01d7:  00fc  movwf   0x7c
01d8:  082e  movf    0x2e, 0x0
01d9:  01ae  clrf    0x2e
01da:  00fd  movwf   0x7d
01db:  082a  movf    0x2a, 0x0
01dc:  01aa  clrf    0x2a
01dd:  01ab  clrf    0x2b
01de:  01ac  clrf    0x2c
01df:  15ac  bsf     0x2c, 0x3
01e0:  14ac  bsf     0x2c, 0x1
01e1:  39f8  andlw   0xf8
01e2:  1903  btfsc   0x03, 0x2
01e3:  3215  bra     0x01f9
01e4:  3180  movlp   0x00
01e5:  2002  call    0x0002
01e6:  138b  bcf     0x0b, 0x7
01e7:  00fc  movwf   0x7c
01e8:  00fd  movwf   0x7d
01e9:  06fc  xorwf   0x7c, 0x1
01ea:  30ff  movlw   0xff
01eb:  3180  movlp   0x00
01ec:  2002  call    0x0002
01ed:  300d  movlw   0x0d
01ee:  138b  bcf     0x0b, 0x7
01ef:  00fc  movwf   0x7c
01f0:  30ff  movlw   0xff
01f1:  3180  movlp   0x00
01f2:  2002  call    0x0002
01f3:  300a  movlw   0x0a
01f4:  138b  bcf     0x0b, 0x7
01f5:  00fc  movwf   0x7c
01f6:  30ff  movlw   0xff
01f7:  3180  movlp   0x00
01f8:  2002  call    0x0002
01f9:  178b  bsf     0x0b, 0x7
01fa:  082f  movf    0x2f, 0x0
01fb:  3a4a  xorlw   0x4a
01fc:  1d03  btfss   0x03, 0x2
01fd:  320e  bra     0x020c
01fe:  032a  decf    0x2a, 0x0
01ff:  3907  andlw   0x07
0200:  1909  btfsc   0x09, 0x2
0201:  3004  movlw   0x04
0202:  3e01  addlw   0x01
0203:  00aa  movwf   0x2a
0204:  138b  bcf     0x0b, 0x7
0205:  3182  movlp   0x02
0206:  0820  movf    0x20, 0x0
0207:  0621  xorwf   0x21, 0x0
0208:  1903  btfsc   0x03, 0x2
0209:  2aac  goto    0x02ac
020a:  178b  bsf     0x0b, 0x7
020b:  3400  retlw   0x00
020c:  082f  movf    0x2f, 0x0
020d:  3a4b  xorlw   0x4b
020e:  1d03  btfss   0x03, 0x2
020f:  3210  bra     0x0220
0210:  01af  clrf    0x2f
0211:  082a  movf    0x2a, 0x0
0212:  1903  btfsc   0x03, 0x2
0213:  0008  return
0214:  138b  bcf     0x0b, 0x7
0215:  00fc  movwf   0x7c
0216:  01aa  clrf    0x2a
0217:  01ab  clrf    0x2b
0218:  01ac  clrf    0x2c
0219:  15ac  bsf     0x2c, 0x3
021a:  14ac  bsf     0x2c, 0x1
021b:  304a  movlw   0x4a
021c:  00af  movwf   0x2f
021d:  3002  movlw   0x02
021e:  3180  movlp   0x00
021f:  2002  call    0x0002
0220:  082f  movf    0x2f, 0x0
0221:  3a4c  xorlw   0x4c
0222:  1d03  btfss   0x03, 0x2
0223:  3218  bra     0x023c
0224:  01af  clrf    0x2f
0225:  082a  movf    0x2a, 0x0
0226:  1903  btfsc   0x03, 0x2
0227:  0008  return
0228:  138b  bcf     0x0b, 0x7
0229:  00fc  movwf   0x7c
022a:  01aa  clrf    0x2a
022b:  01ab  clrf    0x2b
022c:  01ac  clrf    0x2c
022d:  15ac  bsf     0x2c, 0x3
022e:  14ac  bsf     0x2c, 0x1
022f:  304a  movlw   0x4a
0230:  00af  movwf   0x2f
0231:  3005  movlw   0x05
0232:  3180  movlp   0x00
0233:  2002  call    0x0002
0234:  3907  andlw   0x07
0235:  1903  btfsc   0x03, 0x2
0236:  01af  clrf    0x2f
0237:  138b  bcf     0x0b, 0x7
0238:  00fc  movwf   0x7c
0239:  3003  movlw   0x03
023a:  3180  movlp   0x00
023b:  2002  call    0x0002
023c:  082f  movf    0x2f, 0x0
023d:  3a4e  xorlw   0x4e
023e:  1d03  btfss   0x03, 0x2
023f:  3219  bra     0x0259
0240:  082a  movf    0x2a, 0x0
0241:  0084  movwf   0x04
0242:  082b  movf    0x2b, 0x0
0243:  0085  movwf   0x05
0244:  0103  clrw
0245:  138b  bcf     0x0b, 0x7
0246:  00fc  movwf   0x7c
0247:  00fd  movwf   0x7d
0248:  00fe  movwf   0x7e
0249:  00ff  movwf   0x7f
024a:  3000  movlw   0x00
024b:  3180  movlp   0x00
024c:  2002  call    0x0002
024d:  138b  bcf     0x0b, 0x7
024e:  00fc  movwf   0x7c
024f:  0103  clrw
0250:  138b  bcf     0x0b, 0x7
0251:  00fc  movwf   0x7c
0252:  01aa  clrf    0x2a
0253:  01ab  clrf    0x2b
0254:  01ac  clrf    0x2c
0255:  15ac  bsf     0x2c, 0x3
0256:  14ac  bsf     0x2c, 0x1
0257:  304a  movlw   0x4a
0258:  00af  movwf   0x2f
0259:  082f  movf    0x2f, 0x0
025a:  3a50  xorlw   0x50
025b:  1d03  btfss   0x03, 0x2
025c:  321a  bra     0x0277
025d:  01af  clrf    0x2f
025e:  082a  movf    0x2a, 0x0
025f:  1903  btfsc   0x03, 0x2
0260:  0008  return
0261:  304a  movlw   0x4a
0262:  00af  movwf   0x2f
0263:  0e2a  swapf   0x2a, 0x0
0264:  3e10  addlw   0x10
0265:  3970  andlw   0x70
0266:  3e03  addlw   0x03
0267:  0086  movwf   0x06
0268:  0187  clrf    0x07
0269:  0801  movf    0x01, 0x0
026a:  1903  btfsc   0x03, 0x2
026b:  01af  clrf    0x2f
026c:  3880  iorlw   0x80
026d:  0881  movf    0x01, 0x1
026e:  1d03  btfss   0x03, 0x2
026f:  0081  movwf   0x01
0270:  1903  btfsc   0x03, 0x2
0271:  3205  bra     0x0277
0272:  01aa  clrf    0x2a
0273:  01ab  clrf    0x2b
0274:  01ac  clrf    0x2c
0275:  15ac  bsf     0x2c, 0x3
0276:  14ac  bsf     0x2c, 0x1
0277:  082f  movf    0x2f, 0x0
0278:  3a51  xorlw   0x51
0279:  1d03  btfss   0x03, 0x2
027a:  3205  bra     0x0280
027b:  01af  clrf    0x2f
027c:  1017  bcf     0x17, 0x0
027d:  08aa  movf    0x2a, 0x1
027e:  1d03  btfss   0x03, 0x2
027f:  0063  sleep
0280:  082f  movf    0x2f, 0x0
0281:  3a52  xorlw   0x52
0282:  1d03  btfss   0x03, 0x2
0283:  321b  bra     0x029f
0284:  01af  clrf    0x2f
0285:  0e2a  swapf   0x2a, 0x0
0286:  062a  xorwf   0x2a, 0x0
0287:  3e01  addlw   0x01
0288:  1903  btfsc   0x03, 0x2
0289:  0001  reset
028a:  082a  movf    0x2a, 0x0
028b:  1903  btfsc   0x03, 0x2
028c:  0008  return
028d:  304a  movlw   0x4a
028e:  00af  movwf   0x2f
028f:  0e2a  swapf   0x2a, 0x0
0290:  3e10  addlw   0x10
0291:  3970  andlw   0x70
0292:  3e03  addlw   0x03
0293:  0086  movwf   0x06
0294:  0187  clrf    0x07
0295:  307f  movlw   0x7f
0296:  0581  andwf   0x01, 0x1
0297:  1d03  btfss   0x03, 0x2
0298:  3206  bra     0x029f
0299:  01aa  clrf    0x2a
029a:  01ab  clrf    0x2b
029b:  01ac  clrf    0x2c
029c:  15ac  bsf     0x2c, 0x3
029d:  14ac  bsf     0x2c, 0x1
029e:  01af  clrf    0x2f
029f:  082f  movf    0x2f, 0x0
02a0:  3a5a  xorlw   0x5a
02a1:  1d03  btfss   0x03, 0x2
02a2:  3208  bra     0x02ab
02a3:  01af  clrf    0x2f
02a4:  1417  bsf     0x17, 0x0
02a5:  352a  lslf    0x2a, 0x0
02a6:  1903  btfsc   0x03, 0x2
02a7:  3203  bra     0x02ab
02a8:  3801  iorlw   0x01
02a9:  0097  movwf   0x17
02aa:  0063  sleep
02ab:  3400  retlw   0x00
02ac:  0822  movf    0x22, 0x0
02ad:  00a0  movwf   0x20
02ae:  00a1  movwf   0x21
02af:  0e2a  swapf   0x2a, 0x0
02b0:  3e10  addlw   0x10
02b1:  3970  andlw   0x70
02b2:  0084  movwf   0x04
02b3:  0185  clrf    0x05
02b4:  3060  movlw   0x60
02b5:  0086  movwf   0x06
02b6:  3670  lsrf    0x70, 0x0
02b7:  0087  movwf   0x07
02b8:  0c86  rrf     0x06, 0x1
02b9:  300d  movlw   0x0d
02ba:  001e  movwi   0x1++
02bb:  300a  movlw   0x0a
02bc:  001e  movwi   0x1++
02bd:  082a  movf    0x2a, 0x0
02be:  390f  andlw   0x0f
02bf:  3e06  addlw   0x06
02c0:  1a09  btfsc   0x09, 0x4
02c1:  3e07  addlw   0x07
02c2:  3e2a  addlw   0x2a
02c3:  001e  movwi   0x1++
02c4:  3f01  moviw   .1[0]
02c5:  3980  andlw   0x80
02c6:  303a  movlw   0x3a
02c7:  1d03  btfss   0x03, 0x2
02c8:  302a  movlw   0x2a
02c9:  001e  movwi   0x1++
02ca:  3f01  moviw   .1[0]
02cb:  0e09  swapf   0x09, 0x0
02cc:  390f  andlw   0x0f
02cd:  3e06  addlw   0x06
02ce:  1a09  btfsc   0x09, 0x4
02cf:  3e07  addlw   0x07
02d0:  3e2a  addlw   0x2a
02d1:  001e  movwi   0x1++
02d2:  3f01  moviw   .1[0]
02d3:  390f  andlw   0x0f
02d4:  3e06  addlw   0x06
02d5:  1a09  btfsc   0x09, 0x4
02d6:  3e07  addlw   0x07
02d7:  3e2a  addlw   0x2a
02d8:  001e  movwi   0x1++
02d9:  3f00  moviw   .0[0]
02da:  0e09  swapf   0x09, 0x0
02db:  390f  andlw   0x0f
02dc:  3e06  addlw   0x06
02dd:  1a09  btfsc   0x09, 0x4
02de:  3e07  addlw   0x07
02df:  3e2a  addlw   0x2a
02e0:  001e  movwi   0x1++
02e1:  3f00  moviw   .0[0]
02e2:  390f  andlw   0x0f
02e3:  3e06  addlw   0x06
02e4:  1a09  btfsc   0x09, 0x4
02e5:  3e07  addlw   0x07
02e6:  3e2a  addlw   0x2a
02e7:  001e  movwi   0x1++
02e8:  3020  movlw   0x20
02e9:  001e  movwi   0x1++
02ea:  3050  movlw   0x50
02eb:  001e  movwi   0x1++
02ec:  3043  movlw   0x43
02ed:  001e  movwi   0x1++
02ee:  3f03  moviw   .3[0]
02ef:  3980  andlw   0x80
02f0:  303d  movlw   0x3d
02f1:  1d03  btfss   0x03, 0x2
02f2:  307a  movlw   0x7a
02f3:  001e  movwi   0x1++
02f4:  3f03  moviw   .3[0]
02f5:  0e09  swapf   0x09, 0x0
02f6:  390f  andlw   0x0f
02f7:  3e06  addlw   0x06
02f8:  1a09  btfsc   0x09, 0x4
02f9:  3e07  addlw   0x07
02fa:  3e2a  addlw   0x2a
02fb:  001e  movwi   0x1++
02fc:  3f03  moviw   .3[0]
02fd:  390f  andlw   0x0f
02fe:  3e06  addlw   0x06
02ff:  1a09  btfsc   0x09, 0x4
0300:  3e07  addlw   0x07
0301:  3e2a  addlw   0x2a
0302:  001e  movwi   0x1++
0303:  3f03  moviw   .3[0]
0304:  1903  btfsc   0x03, 0x2
0305:  3260  bra     0x0366
0306:  3f02  moviw   .2[0]
0307:  0e09  swapf   0x09, 0x0
0308:  390f  andlw   0x0f
0309:  3e06  addlw   0x06
030a:  1a09  btfsc   0x09, 0x4
030b:  3e07  addlw   0x07
030c:  3e2a  addlw   0x2a
030d:  001e  movwi   0x1++
030e:  3f02  moviw   .2[0]
030f:  390f  andlw   0x0f
0310:  3e06  addlw   0x06
0311:  1a09  btfsc   0x09, 0x4
0312:  3e07  addlw   0x07
0313:  3e2a  addlw   0x2a
0314:  001e  movwi   0x1++
0315:  3020  movlw   0x20
0316:  001e  movwi   0x1++
0317:  3f0d  moviw   .13[0]
0318:  1903  btfsc   0x03, 0x2
0319:  324c  bra     0x0366
031a:  3049  movlw   0x49
031b:  001e  movwi   0x1++
031c:  3053  movlw   0x53
031d:  001e  movwi   0x1++
031e:  3052  movlw   0x52
031f:  001e  movwi   0x1++
0320:  3040  movlw   0x40
0321:  001e  movwi   0x1++
0322:  3f0d  moviw   .13[0]
0323:  0e09  swapf   0x09, 0x0
0324:  390f  andlw   0x0f
0325:  3e06  addlw   0x06
0326:  1a09  btfsc   0x09, 0x4
0327:  3e07  addlw   0x07
0328:  3e2a  addlw   0x2a
0329:  001e  movwi   0x1++
032a:  3f0d  moviw   .13[0]
032b:  390f  andlw   0x0f
032c:  3e06  addlw   0x06
032d:  1a09  btfsc   0x09, 0x4
032e:  3e07  addlw   0x07
032f:  3e2a  addlw   0x2a
0330:  001e  movwi   0x1++
0331:  3f0c  moviw   .12[0]
0332:  0e09  swapf   0x09, 0x0
0333:  390f  andlw   0x0f
0334:  3e06  addlw   0x06
0335:  1a09  btfsc   0x09, 0x4
0336:  3e07  addlw   0x07
0337:  3e2a  addlw   0x2a
0338:  001e  movwi   0x1++
0339:  3f0c  moviw   .12[0]
033a:  390f  andlw   0x0f
033b:  3e06  addlw   0x06
033c:  1a09  btfsc   0x09, 0x4
033d:  3e07  addlw   0x07
033e:  3e2a  addlw   0x2a
033f:  001e  movwi   0x1++
0340:  3028  movlw   0x28
0341:  001e  movwi   0x1++
0342:  3068  movlw   0x68
0343:  001e  movwi   0x1++
0344:  3f0e  moviw   .14[0]
0345:  0e09  swapf   0x09, 0x0
0346:  390f  andlw   0x0f
0347:  3e06  addlw   0x06
0348:  1a09  btfsc   0x09, 0x4
0349:  3e07  addlw   0x07
034a:  3e2a  addlw   0x2a
034b:  001e  movwi   0x1++
034c:  3f0e  moviw   .14[0]
034d:  390f  andlw   0x0f
034e:  3e06  addlw   0x06
034f:  1a09  btfsc   0x09, 0x4
0350:  3e07  addlw   0x07
0351:  3e2a  addlw   0x2a
0352:  001e  movwi   0x1++
0353:  3073  movlw   0x73
0354:  001e  movwi   0x1++
0355:  3f0f  moviw   .15[0]
0356:  0e09  swapf   0x09, 0x0
0357:  390f  andlw   0x0f
0358:  3e06  addlw   0x06
0359:  1a09  btfsc   0x09, 0x4
035a:  3e07  addlw   0x07
035b:  3e2a  addlw   0x2a
035c:  001e  movwi   0x1++
035d:  3f0f  moviw   .15[0]
035e:  390f  andlw   0x0f
035f:  3e06  addlw   0x06
0360:  1a09  btfsc   0x09, 0x4
0361:  3e07  addlw   0x07
0362:  3e2a  addlw   0x2a
0363:  001e  movwi   0x1++
0364:  3029  movlw   0x29
0365:  001e  movwi   0x1++
0366:  3020  movlw   0x20
0367:  001e  movwi   0x1++
0368:  3022  movlw   0x22
0369:  001e  movwi   0x1++
036a:  3f03  moviw   .3[0]
036b:  1d03  btfss   0x03, 0x2
036c:  3216  bra     0x0383
036d:  3074  movlw   0x74
036e:  0084  movwf   0x04
036f:  3083  movlw   0x83
0370:  0085  movwf   0x05
0371:  30f1  movlw   0xf1
0372:  00af  movwf   0x2f
0373:  321c  bra     0x0390
0374:  0028  movlb   0x08
0375:  006e  dw      0x006e
0376:  006f  dw      0x006f
0377:  0074  dw      0x0074
0378:  0020  movlb   0x00
0379:  0072  dw      0x0072
037a:  0075  dw      0x0075
037b:  006e  dw      0x006e
037c:  006e  dw      0x006e
037d:  0069  dw      0x0069
037e:  006e  dw      0x006e
037f:  0067  tris    0x67
0380:  0029  movlb   0x09
0381:  0000  nop
0382:  00f2  movwf   0x72
0383:  3f00  moviw   .0[0]
0384:  00af  movwf   0x2f
0385:  3f01  moviw   .1[0]
0386:  3880  iorlw   0x80
0387:  0085  movwf   0x05
0388:  082f  movf    0x2f, 0x0
0389:  0084  movwf   0x04
038a:  0011  moviw   --0
038b:  38e0  iorlw   0xe0
038c:  00af  movwf   0x2f
038d:  0784  addwf   0x04, 0x1
038e:  1c03  btfss   0x03, 0x0
038f:  0385  decf    0x05, 0x1
0390:  0012  moviw   0++
0391:  1b89  btfsc   0x09, 0x7
0392:  3207  bra     0x039a
0393:  3ee0  addlw   0xe0
0394:  1b89  btfsc   0x09, 0x7
0395:  3204  bra     0x039a
0396:  3e20  addlw   0x20
0397:  001e  movwi   0x1++
0398:  0faf  incfsz  0x2f, 0x1
0399:  33f6  bra     0x0390
039a:  3022  movlw   0x22
039b:  001e  movwi   0x1++
039c:  300d  movlw   0x0d
039d:  001e  movwi   0x1++
039e:  300a  movlw   0x0a
039f:  001e  movwi   0x1++
03a0:  304a  movlw   0x4a
03a1:  00af  movwf   0x2f
03a2:  0806  movf    0x06, 0x0
03a3:  00a1  movwf   0x21
03a4:  082a  movf    0x2a, 0x0
03a5:  03aa  decf    0x2a, 0x1
03a6:  1903  btfsc   0x03, 0x2
03a7:  01af  clrf    0x2f
03a8:  178b  bsf     0x0b, 0x7
03a9:  0008  return
03aa:  3185  movlp   0x05
03ab:  2dc3  goto    0x05c3
03ac:  393f  andlw   0x3f
03ad:  1903  btfsc   0x03, 0x2
03ae:  0008  return
03af:  00fd  movwf   0x7d
03b0:  3008  movlw   0x08
03b1:  00fc  movwf   0x7c
03b2:  3608  lsrf    0x08, 0x0
03b3:  0085  movwf   0x05
03b4:  0821  movf    0x21, 0x0
03b5:  0084  movwf   0x04
03b6:  087c  movf    0x7c, 0x0
03b7:  1d03  btfss   0x03, 0x2
03b8:  322a  bra     0x03e3
03b9:  0e7d  swapf   0x7d, 0x0
03ba:  390f  andlw   0x0f
03bb:  3e06  addlw   0x06
03bc:  1a09  btfsc   0x09, 0x4
03bd:  3e07  addlw   0x07
03be:  3e2a  addlw   0x2a
03bf:  001a  movwi   0++
03c0:  0804  movf    0x04, 0x0
03c1:  397f  andlw   0x7f
03c2:  3a70  xorlw   0x70
03c3:  0e22  swapf   0x22, 0x0
03c4:  1d03  btfss   0x03, 0x2
03c5:  0e04  swapf   0x04, 0x0
03c6:  0e89  swapf   0x09, 0x1
03c7:  0084  movwf   0x04
03c8:  0620  xorwf   0x20, 0x0
03c9:  1903  btfsc   0x03, 0x2
03ca:  3227  bra     0x03f2
03cb:  0620  xorwf   0x20, 0x0
03cc:  00a1  movwf   0x21
03cd:  087d  movf    0x7d, 0x0
03ce:  390f  andlw   0x0f
03cf:  3e06  addlw   0x06
03d0:  1a09  btfsc   0x09, 0x4
03d1:  3e07  addlw   0x07
03d2:  3e2a  addlw   0x2a
03d3:  001a  movwi   0++
03d4:  0804  movf    0x04, 0x0
03d5:  397f  andlw   0x7f
03d6:  3a70  xorlw   0x70
03d7:  0e22  swapf   0x22, 0x0
03d8:  1d03  btfss   0x03, 0x2
03d9:  0e04  swapf   0x04, 0x0
03da:  0e89  swapf   0x09, 0x1
03db:  0084  movwf   0x04
03dc:  0620  xorwf   0x20, 0x0
03dd:  1903  btfsc   0x03, 0x2
03de:  3212  bra     0x03f1
03df:  0620  xorwf   0x20, 0x0
03e0:  00a1  movwf   0x21
03e1:  3002  movlw   0x02
03e2:  320f  bra     0x03f2
03e3:  001a  movwi   0++
03e4:  0804  movf    0x04, 0x0
03e5:  397f  andlw   0x7f
03e6:  3a70  xorlw   0x70
03e7:  0e22  swapf   0x22, 0x0
03e8:  1d03  btfss   0x03, 0x2
03e9:  0e04  swapf   0x04, 0x0
03ea:  0e89  swapf   0x09, 0x1
03eb:  0084  movwf   0x04
03ec:  0620  xorwf   0x20, 0x0
03ed:  1903  btfsc   0x03, 0x2
03ee:  3203  bra     0x03f2
03ef:  0620  xorwf   0x20, 0x0
03f0:  00a1  movwf   0x21
03f1:  3001  movlw   0x01
03f2:  3901  andlw   0x01
03f3:  1903  btfsc   0x03, 0x2
03f4:  0008  return
03f5:  0bfd  decfsz  0x7d, 0x1
03f6:  33bb  bra     0x03b2
03f7:  0008  return
03f8:  082b  movf    0x2b, 0x0
03f9:  01fc  clrf    0x7c
03fa:  00fd  movwf   0x7d
03fb:  3608  lsrf    0x08, 0x0
03fc:  0087  movwf   0x07
03fd:  0821  movf    0x21, 0x0
03fe:  0086  movwf   0x06
03ff:  087c  movf    0x7c, 0x0
0400:  1d03  btfss   0x03, 0x2
0401:  322a  bra     0x042c
0402:  0e7d  swapf   0x7d, 0x0
0403:  390f  andlw   0x0f
0404:  3e06  addlw   0x06
0405:  1a09  btfsc   0x09, 0x4
0406:  3e07  addlw   0x07
0407:  3e2a  addlw   0x2a
0408:  001e  movwi   0x1++
0409:  0806  movf    0x06, 0x0
040a:  397f  andlw   0x7f
040b:  3a70  xorlw   0x70
040c:  0e22  swapf   0x22, 0x0
040d:  1d03  btfss   0x03, 0x2
040e:  0e06  swapf   0x06, 0x0
040f:  0e89  swapf   0x09, 0x1
0410:  0086  movwf   0x06
0411:  0620  xorwf   0x20, 0x0
0412:  1903  btfsc   0x03, 0x2
0413:  3227  bra     0x043b
0414:  0620  xorwf   0x20, 0x0
0415:  00a1  movwf   0x21
0416:  087d  movf    0x7d, 0x0
0417:  390f  andlw   0x0f
0418:  3e06  addlw   0x06
0419:  1a09  btfsc   0x09, 0x4
041a:  3e07  addlw   0x07
041b:  3e2a  addlw   0x2a
041c:  001e  movwi   0x1++
041d:  0806  movf    0x06, 0x0
041e:  397f  andlw   0x7f
041f:  3a70  xorlw   0x70
0420:  0e22  swapf   0x22, 0x0
0421:  1d03  btfss   0x03, 0x2
0422:  0e06  swapf   0x06, 0x0
0423:  0e89  swapf   0x09, 0x1
0424:  0086  movwf   0x06
0425:  0620  xorwf   0x20, 0x0
0426:  1903  btfsc   0x03, 0x2
0427:  3212  bra     0x043a
0428:  0620  xorwf   0x20, 0x0
0429:  00a1  movwf   0x21
042a:  3002  movlw   0x02
042b:  320f  bra     0x043b
042c:  001e  movwi   0x1++
042d:  0806  movf    0x06, 0x0
042e:  397f  andlw   0x7f
042f:  3a70  xorlw   0x70
0430:  0e22  swapf   0x22, 0x0
0431:  1d03  btfss   0x03, 0x2
0432:  0e06  swapf   0x06, 0x0
0433:  0e89  swapf   0x09, 0x1
0434:  0086  movwf   0x06
0435:  0620  xorwf   0x20, 0x0
0436:  1903  btfsc   0x03, 0x2
0437:  3203  bra     0x043b
0438:  0620  xorwf   0x20, 0x0
0439:  00a1  movwf   0x21
043a:  3001  movlw   0x01
043b:  0008  return
043c:  3030  movlw   0x30
043d:  3205  bra     0x0443
043e:  3078  movlw   0x78
043f:  3203  bra     0x0443
0440:  3020  movlw   0x20
0441:  3201  bra     0x0443
0442:  300a  movlw   0x0a
0443:  00fc  movwf   0x7c
0444:  3001  movlw   0x01
0445:  00fd  movwf   0x7d
0446:  336b  bra     0x03b2
0447:  0870  movf    0x70, 0x0
0448:  0088  movwf   0x08
0449:  3184  movlp   0x04
044a:  30e0  movlw   0xe0
044b:  077c  addwf   0x7c, 0x0
044c:  1f89  btfss   0x09, 0x7
044d:  2444  call    0x0444
044e:  3901  andlw   0x01
044f:  3185  movlp   0x05
0450:  1903  btfsc   0x03, 0x2
0451:  2dc0  goto    0x05c0
0452:  087c  movf    0x7c, 0x0
0453:  3a7e  xorlw   0x7e
0454:  1d03  btfss   0x03, 0x2
0455:  3213  bra     0x0469
0456:  3184  movlp   0x04
0457:  243c  call    0x043c
0458:  3184  movlp   0x04
0459:  243e  call    0x043e
045a:  09aa  comf    0x2a, 0x1
045b:  092b  comf    0x2b, 0x0
045c:  00ab  movwf   0x2b
045d:  00af  movwf   0x2f
045e:  3183  movlp   0x03
045f:  23f8  call    0x03f8
0460:  082a  movf    0x2a, 0x0
0461:  00ab  movwf   0x2b
0462:  3183  movlp   0x03
0463:  23f9  call    0x03f9
0464:  082f  movf    0x2f, 0x0
0465:  00ab  movwf   0x2b
0466:  01af  clrf    0x2f
0467:  3180  movlp   0x00
0468:  2830  goto    0x0030
0469:  1b7c  btfsc   0x7c, 0x6
046a:  12fc  bcf     0x7c, 0x5
046b:  087c  movf    0x7c, 0x0
046c:  00af  movwf   0x2f
046d:  3a08  xorlw   0x08
046e:  307f  movlw   0x7f
046f:  1d03  btfss   0x03, 0x2
0470:  082f  movf    0x2f, 0x0
0471:  3a7f  xorlw   0x7f
0472:  1d03  btfss   0x03, 0x2
0473:  3205  bra     0x0479
0474:  300d  movlw   0x0d
0475:  3184  movlp   0x04
0476:  2443  call    0x0443
0477:  3185  movlp   0x05
0478:  2daa  goto    0x05aa
0479:  082f  movf    0x2f, 0x0
047a:  3a0d  xorlw   0x0d
047b:  1d03  btfss   0x03, 0x2
047c:  3216  bra     0x0493
047d:  300d  movlw   0x0d
047e:  3184  movlp   0x04
047f:  2443  call    0x0443
0480:  3184  movlp   0x04
0481:  2442  call    0x0442
0482:  082d  movf    0x2d, 0x0
0483:  0084  movwf   0x04
0484:  082e  movf    0x2e, 0x0
0485:  0085  movwf   0x05
0486:  0404  iorwf   0x04, 0x0
0487:  3185  movlp   0x05
0488:  1903  btfsc   0x03, 0x2
0489:  2daa  goto    0x05aa
048a:  082a  movf    0x2a, 0x0
048b:  1f85  btfss   0x05, 0x7
048c:  001a  movwi   0++
048d:  0804  movf    0x04, 0x0
048e:  00ad  movwf   0x2d
048f:  0805  movf    0x05, 0x0
0490:  00ae  movwf   0x2e
0491:  3185  movlp   0x05
0492:  2daa  goto    0x05aa
0493:  082f  movf    0x2f, 0x0
0494:  3a2c  xorlw   0x2c
0495:  3020  movlw   0x20
0496:  1903  btfsc   0x03, 0x2
0497:  00af  movwf   0x2f
0498:  082f  movf    0x2f, 0x0
0499:  3a20  xorlw   0x20
049a:  1903  btfsc   0x03, 0x2
049b:  3208  bra     0x04a4
049c:  082f  movf    0x2f, 0x0
049d:  3a2e  xorlw   0x2e
049e:  1903  btfsc   0x03, 0x2
049f:  3204  bra     0x04a4
04a0:  082f  movf    0x2f, 0x0
04a1:  3a3d  xorlw   0x3d
04a2:  1d03  btfss   0x03, 0x2
04a3:  3268  bra     0x050c
04a4:  082a  movf    0x2a, 0x0
04a5:  042b  iorwf   0x2b, 0x0
04a6:  1903  btfsc   0x03, 0x2
04a7:  3209  bra     0x04b1
04a8:  082a  movf    0x2a, 0x0
04a9:  00ad  movwf   0x2d
04aa:  082b  movf    0x2b, 0x0
04ab:  00ae  movwf   0x2e
04ac:  082f  movf    0x2f, 0x0
04ad:  3a20  xorlw   0x20
04ae:  3185  movlp   0x05
04af:  1903  btfsc   0x03, 0x2
04b0:  2dbb  goto    0x05bb
04b1:  1fae  btfss   0x2e, 0x7
04b2:  3235  bra     0x04e8
04b3:  3184  movlp   0x04
04b4:  243c  call    0x043c
04b5:  3184  movlp   0x04
04b6:  243e  call    0x043e
04b7:  082d  movf    0x2d, 0x0
04b8:  0084  movwf   0x04
04b9:  082e  movf    0x2e, 0x0
04ba:  0085  movwf   0x05
04bb:  0808  movf    0x08, 0x0
04bc:  003f  movlb   0x1f
04bd:  0aed  incf    0x6d, 0x1
04be:  00ef  movwf   0x6f
04bf:  0088  movwf   0x08
04c0:  0023  movlb   0x03
04c1:  0804  movf    0x04, 0x0
04c2:  0091  movwf   0x11
04c3:  0805  movf    0x05, 0x0
04c4:  0092  movwf   0x12
04c5:  0023  movlb   0x03
04c6:  1315  bcf     0x15, 0x6
04c7:  1415  bsf     0x15, 0x0
04c8:  0000  nop
04c9:  0000  nop
04ca:  0814  movf    0x14, 0x0
04cb:  00fc  movwf   0x7c
04cc:  003f  movlb   0x1f
04cd:  086f  movf    0x6f, 0x0
04ce:  03ed  decf    0x6d, 0x1
04cf:  0088  movwf   0x08
04d0:  087c  movf    0x7c, 0x0
04d1:  00ab  movwf   0x2b
04d2:  3183  movlp   0x03
04d3:  23f8  call    0x03f8
04d4:  082d  movf    0x2d, 0x0
04d5:  0084  movwf   0x04
04d6:  082e  movf    0x2e, 0x0
04d7:  0085  movwf   0x05
04d8:  0012  moviw   0++
04d9:  00aa  movwf   0x2a
04da:  0804  movf    0x04, 0x0
04db:  00ad  movwf   0x2d
04dc:  0805  movf    0x05, 0x0
04dd:  00ae  movwf   0x2e
04de:  082a  movf    0x2a, 0x0
04df:  3183  movlp   0x03
04e0:  23f9  call    0x03f9
04e1:  300d  movlw   0x0d
04e2:  3184  movlp   0x04
04e3:  2443  call    0x0443
04e4:  3184  movlp   0x04
04e5:  2442  call    0x0442
04e6:  3185  movlp   0x05
04e7:  2daa  goto    0x05aa
04e8:  3184  movlp   0x04
04e9:  243c  call    0x043c
04ea:  3184  movlp   0x04
04eb:  243e  call    0x043e
04ec:  082d  movf    0x2d, 0x0
04ed:  0084  movwf   0x04
04ee:  082e  movf    0x2e, 0x0
04ef:  0085  movwf   0x05
04f0:  0012  moviw   0++
04f1:  00ab  movwf   0x2b
04f2:  3183  movlp   0x03
04f3:  23f8  call    0x03f8
04f4:  082f  movf    0x2f, 0x0
04f5:  3a2e  xorlw   0x2e
04f6:  1d03  btfss   0x03, 0x2
04f7:  320b  bra     0x0503
04f8:  0804  movf    0x04, 0x0
04f9:  00ad  movwf   0x2d
04fa:  0805  movf    0x05, 0x0
04fb:  00ae  movwf   0x2e
04fc:  300d  movlw   0x0d
04fd:  3184  movlp   0x04
04fe:  2443  call    0x0443
04ff:  3184  movlp   0x04
0500:  2442  call    0x0442
0501:  3185  movlp   0x05
0502:  2daa  goto    0x05aa
0503:  082f  movf    0x2f, 0x0
0504:  3a2e  xorlw   0x2e
0505:  1d03  btfss   0x03, 0x2
0506:  3003  movlw   0x03
0507:  3183  movlp   0x03
0508:  23ac  call    0x03ac
0509:  01af  clrf    0x2f
050a:  3180  movlp   0x00
050b:  2830  goto    0x0030
050c:  082f  movf    0x2f, 0x0
050d:  3a58  xorlw   0x58
050e:  1d03  btfss   0x03, 0x2
050f:  3205  bra     0x0515
0510:  3010  movlw   0x10
0511:  00ac  movwf   0x2c
0512:  01af  clrf    0x2f
0513:  3180  movlp   0x00
0514:  2830  goto    0x0030
0515:  082f  movf    0x2f, 0x0
0516:  3a25  xorlw   0x25
0517:  1d03  btfss   0x03, 0x2
0518:  321a  bra     0x0533
0519:  309b  movlw   0x9b
051a:  072a  addwf   0x2a, 0x0
051b:  1b89  btfsc   0x09, 0x7
051c:  3202  bra     0x051f
051d:  3066  movlw   0x66
051e:  00aa  movwf   0x2a
051f:  082a  movf    0x2a, 0x0
0520:  307e  movlw   0x7e
0521:  052a  andwf   0x2a, 0x0
0522:  35aa  lslf    0x2a, 0x1
0523:  35aa  lslf    0x2a, 0x1
0524:  1803  btfsc   0x03, 0x0
0525:  3801  iorlw   0x01
0526:  07aa  addwf   0x2a, 0x1
0527:  1803  btfsc   0x03, 0x0
0528:  3801  iorlw   0x01
0529:  0c89  rrf     0x09, 0x1
052a:  0caa  rrf     0x2a, 0x1
052b:  082a  movf    0x2a, 0x0
052c:  00ab  movwf   0x2b
052d:  3183  movlp   0x03
052e:  23f8  call    0x03f8
052f:  01ab  clrf    0x2b
0530:  01af  clrf    0x2f
0531:  3180  movlp   0x00
0532:  2830  goto    0x0030
0533:  082f  movf    0x2f, 0x0
0534:  3a3f  xorlw   0x3f
0535:  1d03  btfss   0x03, 0x2
0536:  3204  bra     0x053b
0537:  30ff  movlw   0xff
0538:  00ad  movwf   0x2d
0539:  303b  movlw   0x3b
053a:  00af  movwf   0x2f
053b:  082f  movf    0x2f, 0x0
053c:  3a3b  xorlw   0x3b
053d:  1d03  btfss   0x03, 0x2
053e:  3224  bra     0x0563
053f:  3006  movlw   0x06
0540:  00f0  movwf   0x70
0541:  0e70  swapf   0x70, 0x0
0542:  3e10  addlw   0x10
0543:  3970  andlw   0x70
0544:  0084  movwf   0x04
0545:  0185  clrf    0x05
0546:  30f0  movlw   0xf0
0547:  0784  addwf   0x04, 0x1
0548:  03f0  decf    0x70, 0x1
0549:  1903  btfsc   0x03, 0x2
054a:  3218  bra     0x0563
054b:  3f03  moviw   .3[0]
054c:  1903  btfsc   0x03, 0x2
054d:  33f8  bra     0x0546
054e:  0608  xorwf   0x08, 0x0
054f:  1903  btfsc   0x03, 0x2
0550:  33f5  bra     0x0546
0551:  3f01  moviw   .1[0]
0552:  1f89  btfss   0x09, 0x7
0553:  33f2  bra     0x0546
0554:  3570  lslf    0x70, 0x0
0555:  390e  andlw   0x0e
0556:  3870  iorlw   0x70
0557:  0084  movwf   0x04
0558:  301f  movlw   0x1f
0559:  0585  andwf   0x05, 0x1
055a:  082a  movf    0x2a, 0x0
055b:  3f81  movwi   .1[0]
055c:  082d  movf    0x2d, 0x0
055d:  3f80  movwi   .0[0]
055e:  00ae  movwf   0x2e
055f:  3f01  moviw   .1[0]
0560:  00ad  movwf   0x2d
0561:  3185  movlp   0x05
0562:  2db9  goto    0x05b9
0563:  30d0  movlw   0xd0
0564:  07af  addwf   0x2f, 0x1
0565:  1baf  btfsc   0x2f, 0x7
0566:  323c  bra     0x05a3
0567:  30f0  movlw   0xf0
0568:  072f  addwf   0x2f, 0x0
0569:  1b89  btfsc   0x09, 0x7
056a:  3202  bra     0x056d
056b:  30f9  movlw   0xf9
056c:  07af  addwf   0x2f, 0x1
056d:  1d03  btfss   0x03, 0x2
056e:  3208  bra     0x0577
056f:  082a  movf    0x2a, 0x0
0570:  042b  iorwf   0x2b, 0x0
0571:  1d03  btfss   0x03, 0x2
0572:  3204  bra     0x0577
0573:  10ac  bcf     0x2c, 0x1
0574:  01af  clrf    0x2f
0575:  3180  movlp   0x00
0576:  2830  goto    0x0030
0577:  30f0  movlw   0xf0
0578:  052f  andwf   0x2f, 0x0
0579:  1d03  btfss   0x03, 0x2
057a:  3229  bra     0x05a4
057b:  1e2c  btfss   0x2c, 0x4
057c:  320f  bra     0x058c
057d:  0eab  swapf   0x2b, 0x1
057e:  30f0  movlw   0xf0
057f:  05ab  andwf   0x2b, 0x1
0580:  0e2a  swapf   0x2a, 0x0
0581:  390f  andlw   0x0f
0582:  04ab  iorwf   0x2b, 0x1
0583:  300f  movlw   0x0f
0584:  05af  andwf   0x2f, 0x1
0585:  05aa  andwf   0x2a, 0x1
0586:  0e2a  swapf   0x2a, 0x0
0587:  042f  iorwf   0x2f, 0x0
0588:  00aa  movwf   0x2a
0589:  01af  clrf    0x2f
058a:  3180  movlp   0x00
058b:  2830  goto    0x0030
058c:  082f  movf    0x2f, 0x0
058d:  39f0  andlw   0xf0
058e:  1d03  btfss   0x03, 0x2
058f:  3214  bra     0x05a4
0590:  35aa  lslf    0x2a, 0x1
0591:  0dab  rlf     0x2b, 0x1
0592:  082a  movf    0x2a, 0x0
0593:  35aa  lslf    0x2a, 0x1
0594:  0dab  rlf     0x2b, 0x1
0595:  35aa  lslf    0x2a, 0x1
0596:  0dab  rlf     0x2b, 0x1
0597:  1cac  btfss   0x2c, 0x1
0598:  3203  bra     0x059c
0599:  07aa  addwf   0x2a, 0x1
059a:  3000  movlw   0x00
059b:  3dab  addwfc  0x2b, 0x1
059c:  082f  movf    0x2f, 0x0
059d:  390f  andlw   0x0f
059e:  07aa  addwf   0x2a, 0x1
059f:  3000  movlw   0x00
05a0:  3dab  addwfc  0x2b, 0x1
05a1:  3180  movlp   0x00
05a2:  2830  goto    0x0030
05a3:  30c9  movlw   0xc9
05a4:  3037  movlw   0x37
05a5:  07af  addwf   0x2f, 0x1
05a6:  082a  movf    0x2a, 0x0
05a7:  00fd  movwf   0x7d
05a8:  3180  movlp   0x00
05a9:  2830  goto    0x0030
05aa:  082e  movf    0x2e, 0x0
05ab:  00ab  movwf   0x2b
05ac:  042d  iorwf   0x2d, 0x0
05ad:  3184  movlp   0x04
05ae:  1903  btfsc   0x03, 0x2
05af:  320b  bra     0x05bb
05b0:  243c  call    0x043c
05b1:  3184  movlp   0x04
05b2:  243e  call    0x043e
05b3:  3183  movlp   0x03
05b4:  23f8  call    0x03f8
05b5:  082d  movf    0x2d, 0x0
05b6:  00ab  movwf   0x2b
05b7:  3183  movlp   0x03
05b8:  23f9  call    0x03f9
05b9:  3184  movlp   0x04
05ba:  2440  call    0x0440
05bb:  01aa  clrf    0x2a
05bc:  01ab  clrf    0x2b
05bd:  01ac  clrf    0x2c
05be:  15ac  bsf     0x2c, 0x3
05bf:  14ac  bsf     0x2c, 0x1
05c0:  01af  clrf    0x2f
05c1:  3180  movlp   0x00
05c2:  2830  goto    0x0030
05c3:  3229  bra     0x05ed
05c4:  0063  sleep
05c5:  006f  dw      0x006f
05c6:  006e  dw      0x006e
05c7:  0073  dw      0x0073
05c8:  006f  dw      0x006f
05c9:  006c  dw      0x006c
05ca:  0065  tris    0x65
05cb:  0020  movlb   0x00
05cc:  0049  dw      0x0049
05cd:  002f  movlb   0x0f
05ce:  004f  dw      0x004f
05cf:  0000  nop
05d0:  00f4  movwf   0x74
05d1:  0829  movf    0x29, 0x0
05d2:  008a  movwf   0x0a
05d3:  0428  iorwf   0x28, 0x0
05d4:  1903  btfsc   0x03, 0x2
05d5:  3202  bra     0x05d8
05d6:  0828  movf    0x28, 0x0
05d7:  000a  callw
05d8:  0827  movf    0x27, 0x0
05d9:  008a  movwf   0x0a
05da:  0826  movf    0x26, 0x0
05db:  0082  movwf   0x02
05dc:  0870  movf    0x70, 0x0
05dd:  0088  movwf   0x08
05de:  0825  movf    0x25, 0x0
05df:  008a  movwf   0x0a
05e0:  0824  movf    0x24, 0x0
05e1:  0020  movlb   0x00
05e2:  1e91  btfss   0x11, 0x5
05e3:  0082  movwf   0x02
05e4:  1291  bcf     0x11, 0x5
05e5:  0023  movlb   0x03
05e6:  0819  movf    0x19, 0x0
05e7:  00fc  movwf   0x7c
05e8:  3184  movlp   0x04
05e9:  1d03  btfss   0x03, 0x2
05ea:  2c47  goto    0x0447
05eb:  3180  movlp   0x00
05ec:  2830  goto    0x0030
05ed:  32ed  bra     0x06db
05ee:  0063  sleep
05ef:  006f  dw      0x006f
05f0:  006e  dw      0x006e
05f1:  0073  dw      0x0073
05f2:  006f  dw      0x006f
05f3:  006c  dw      0x006c
05f4:  0065  tris    0x65
05f5:  0020  movlb   0x00
05f6:  0028  movlb   0x08
05f7:  006f  dw      0x006f
05f8:  0075  dw      0x0075
05f9:  0074  dw      0x0074
05fa:  0070  dw      0x0070
05fb:  0075  dw      0x0075
05fc:  0074  dw      0x0074
05fd:  002d  movlb   0x0d
05fe:  006f  dw      0x006f
05ff:  006e  dw      0x006e
0600:  006c  dw      0x006c
0601:  0079  dw      0x0079
0602:  0029  movlb   0x09
0603:  0000  nop
0604:  00ea  movwf   0x6a
0605:  3001  movlw   0x01
0606:  0085  movwf   0x05
0607:  3508  lslf    0x08, 0x0
0608:  390e  andlw   0x0e
0609:  3870  iorlw   0x70
060a:  0084  movwf   0x04
060b:  301f  movlw   0x1f
060c:  0585  andwf   0x05, 0x1
060d:  3f00  moviw   .0[0]
060e:  1d03  btfss   0x03, 0x2
060f:  3236  bra     0x0646
0610:  138b  bcf     0x0b, 0x7
0611:  30ff  movlw   0xff
0612:  3f80  movwi   .0[0]
0613:  3000  movlw   0x00
0614:  3f81  movwi   .1[0]
0615:  0c08  rrf     0x08, 0x0
0616:  0103  clrw
0617:  0c89  rrf     0x09, 0x1
0618:  3830  iorlw   0x30
0619:  00a2  movwf   0x22
061a:  00a0  movwf   0x20
061b:  00a1  movwf   0x21
061c:  178b  bsf     0x0b, 0x7
061d:  3213  bra     0x0631
061e:  000b  brw
061f:  345f  retlw   0x5f
0620:  345f  retlw   0x5f
0621:  3457  retlw   0x57
0622:  3465  retlw   0x65
0623:  346c  retlw   0x6c
0624:  3463  retlw   0x63
0625:  346f  retlw   0x6f
0626:  346d  retlw   0x6d
0627:  3465  retlw   0x65
0628:  345f  retlw   0x5f
0629:  3474  retlw   0x74
062a:  346f  retlw   0x6f
062b:  345f  retlw   0x5f
062c:  347a  retlw   0x7a
062d:  344f  retlw   0x4f
062e:  3453  retlw   0x53
062f:  345f  retlw   0x5f
0630:  345f  retlw   0x5f
0631:  3012  movlw   0x12
0632:  00af  movwf   0x2f
0633:  0808  movf    0x08, 0x0
0634:  138b  bcf     0x0b, 0x7
0635:  00fc  movwf   0x7c
0636:  3004  movlw   0x04
0637:  3180  movlp   0x00
0638:  2002  call    0x0002
0639:  082f  movf    0x2f, 0x0
063a:  3c12  sublw   0x12
063b:  3186  movlp   0x06
063c:  261e  call    0x061e
063d:  138b  bcf     0x0b, 0x7
063e:  00fc  movwf   0x7c
063f:  30ff  movlw   0xff
0640:  3180  movlp   0x00
0641:  2002  call    0x0002
0642:  0b89  decfsz  0x09, 0x1
0643:  33ef  bra     0x0633
0644:  0baf  decfsz  0x2f, 0x1
0645:  33f3  bra     0x0639
0646:  0808  movf    0x08, 0x0
0647:  138b  bcf     0x0b, 0x7
0648:  00fc  movwf   0x7c
0649:  3004  movlw   0x04
064a:  3180  movlp   0x00
064b:  2002  call    0x0002
064c:  3080  movlw   0x80
064d:  0084  movwf   0x04
064e:  3000  movlw   0x00
064f:  0087  movwf   0x07
0650:  3011  movlw   0x11
0651:  0086  movwf   0x06
0652:  1e01  btfss   0x01, 0x4
0653:  3212  bra     0x0666
0654:  3608  lsrf    0x08, 0x0
0655:  0087  movwf   0x07
0656:  138b  bcf     0x0b, 0x7
0657:  0820  movf    0x20, 0x0
0658:  0086  movwf   0x06
0659:  0621  xorwf   0x21, 0x0
065a:  1903  btfsc   0x03, 0x2
065b:  320a  bra     0x0666
065c:  0016  moviw   0x1++
065d:  3f9a  movwi   .26[0]
065e:  0806  movf    0x06, 0x0
065f:  00a0  movwf   0x20
0660:  397f  andlw   0x7f
0661:  3a70  xorlw   0x70
0662:  1d03  btfss   0x03, 0x2
0663:  3202  bra     0x0666
0664:  0822  movf    0x22, 0x0
0665:  00a0  movwf   0x20
0666:  178b  bsf     0x0b, 0x7
0667:  0e08  swapf   0x08, 0x0
0668:  3e10  addlw   0x10
0669:  3970  andlw   0x70
066a:  0084  movwf   0x04
066b:  0185  clrf    0x05
066c:  3f01  moviw   .1[0]
066d:  008a  movwf   0x0a
066e:  3f00  moviw   .0[0]
066f:  0082  movwf   0x02
0670:  0020  movlb   0x00
0671:  1d0b  btfss   0x0b, 0x2
0672:  321d  bra     0x0690
0673:  110b  bcf     0x0b, 0x2
0674:  0870  movf    0x70, 0x0
0675:  0088  movwf   0x08
0676:  3508  lslf    0x08, 0x0
0677:  390e  andlw   0x0e
0678:  3870  iorlw   0x70
0679:  0084  movwf   0x04
067a:  301f  movlw   0x1f
067b:  0585  andwf   0x05, 0x1
067c:  3046  movlw   0x46
067d:  0086  movwf   0x06
067e:  3670  lsrf    0x70, 0x0
067f:  0087  movwf   0x07
0680:  0c86  rrf     0x06, 0x1
0681:  0020  movlb   0x00
0682:  3f01  moviw   .1[0]
0683:  1f89  btfss   0x09, 0x7
0684:  0095  movwf   0x15
0685:  0b81  decfsz  0x01, 0x1
0686:  3252  bra     0x06d9
0687:  0022  movlb   0x02
0688:  0800  movf    0x00, 0x0
0689:  1903  btfsc   0x03, 0x2
068a:  3001  movlw   0x01
068b:  0080  movwf   0x00
068c:  0081  movwf   0x01
068d:  3010  movlw   0x10
068e:  068c  xorwf   0x0c, 0x1
068f:  3249  bra     0x06d9
0690:  08f1  movf    0x71, 0x1
0691:  1d03  btfss   0x03, 0x2
0692:  3202  bra     0x0695
0693:  3180  movlp   0x00
0694:  280b  goto    0x000b
0695:  0870  movf    0x70, 0x0
0696:  0088  movwf   0x08
0697:  3608  lsrf    0x08, 0x0
0698:  0085  movwf   0x05
0699:  0821  movf    0x21, 0x0
069a:  0084  movwf   0x04
069b:  087c  movf    0x7c, 0x0
069c:  1d03  btfss   0x03, 0x2
069d:  322a  bra     0x06c8
069e:  0e7d  swapf   0x7d, 0x0
069f:  390f  andlw   0x0f
06a0:  3e06  addlw   0x06
06a1:  1a09  btfsc   0x09, 0x4
06a2:  3e07  addlw   0x07
06a3:  3e2a  addlw   0x2a
06a4:  001a  movwi   0++
06a5:  0804  movf    0x04, 0x0
06a6:  397f  andlw   0x7f
06a7:  3a70  xorlw   0x70
06a8:  0e22  swapf   0x22, 0x0
06a9:  1d03  btfss   0x03, 0x2
06aa:  0e04  swapf   0x04, 0x0
06ab:  0e89  swapf   0x09, 0x1
06ac:  0084  movwf   0x04
06ad:  0620  xorwf   0x20, 0x0
06ae:  1903  btfsc   0x03, 0x2
06af:  3227  bra     0x06d7
06b0:  0620  xorwf   0x20, 0x0
06b1:  00a1  movwf   0x21
06b2:  087d  movf    0x7d, 0x0
06b3:  390f  andlw   0x0f
06b4:  3e06  addlw   0x06
06b5:  1a09  btfsc   0x09, 0x4
06b6:  3e07  addlw   0x07
06b7:  3e2a  addlw   0x2a
06b8:  001a  movwi   0++
06b9:  0804  movf    0x04, 0x0
06ba:  397f  andlw   0x7f
06bb:  3a70  xorlw   0x70
06bc:  0e22  swapf   0x22, 0x0
06bd:  1d03  btfss   0x03, 0x2
06be:  0e04  swapf   0x04, 0x0
06bf:  0e89  swapf   0x09, 0x1
06c0:  0084  movwf   0x04
06c1:  0620  xorwf   0x20, 0x0
06c2:  1903  btfsc   0x03, 0x2
06c3:  3212  bra     0x06d6
06c4:  0620  xorwf   0x20, 0x0
06c5:  00a1  movwf   0x21
06c6:  3002  movlw   0x02
06c7:  320f  bra     0x06d7
06c8:  001a  movwi   0++
06c9:  0804  movf    0x04, 0x0
06ca:  397f  andlw   0x7f
06cb:  3a70  xorlw   0x70
06cc:  0e22  swapf   0x22, 0x0
06cd:  1d03  btfss   0x03, 0x2
06ce:  0e04  swapf   0x04, 0x0
06cf:  0e89  swapf   0x09, 0x1
06d0:  0084  movwf   0x04
06d1:  0620  xorwf   0x20, 0x0
06d2:  1903  btfsc   0x03, 0x2
06d3:  3203  bra     0x06d7
06d4:  0620  xorwf   0x20, 0x0
06d5:  00a1  movwf   0x21
06d6:  3001  movlw   0x01
06d7:  3180  movlp   0x00
06d8:  282e  goto    0x002e
06d9:  3180  movlp   0x00
06da:  2830  goto    0x0030
06db:  0023  movlb   0x03
06dc:  139d  bcf     0x1d, 0x7
06dd:  121d  bcf     0x1d, 0x4
06de:  129e  bcf     0x1e, 0x5
06df:  159f  bsf     0x1f, 0x3
06e0:  121e  bcf     0x1e, 0x4
06e1:  151e  bsf     0x1e, 0x2
06e2:  3040  movlw   0x40
06e3:  009b  movwf   0x1b
06e4:  3003  movlw   0x03
06e5:  009c  movwf   0x1c
06e6:  121f  bcf     0x1f, 0x4
06e7:  179d  bsf     0x1d, 0x7
06e8:  131d  bcf     0x1d, 0x6
06e9:  161d  bsf     0x1d, 0x4
06ea:  169e  bsf     0x1e, 0x5
06eb:  0021  movlb   0x01
06ec:  1691  bsf     0x11, 0x5
06ed:  3005  movlw   0x05
06ee:  0084  movwf   0x04
06ef:  3006  movlw   0x06
06f0:  0085  movwf   0x05
06f1:  1785  bsf     0x05, 0x7
06f2:  3070  movlw   0x70
06f3:  138b  bcf     0x0b, 0x7
06f4:  00fc  movwf   0x7c
06f5:  3086  movlw   0x86
06f6:  00fd  movwf   0x7d
06f7:  3004  movlw   0x04
06f8:  00fe  movwf   0x7e
06f9:  0020  movlb   0x00
06fa:  3000  movlw   0x00
06fb:  3180  movlp   0x00
06fc:  2002  call    0x0002
06fd:  138b  bcf     0x0b, 0x7
06fe:  0809  movf    0x09, 0x0
06ff:  1903  btfsc   0x03, 0x2
0700:  33f9  bra     0x06fa
0701:  00f0  movwf   0x70
0702:  3907  andlw   0x07
0703:  1903  btfsc   0x03, 0x2
0704:  320f  bra     0x0714
0705:  1909  btfsc   0x09, 0x2
0706:  1c89  btfss   0x09, 0x1
0707:  3201  bra     0x0709
0708:  320b  bra     0x0714
0709:  0e09  swapf   0x09, 0x0
070a:  3e10  addlw   0x10
070b:  3970  andlw   0x70
070c:  3e03  addlw   0x03
070d:  0086  movwf   0x06
070e:  0187  clrf    0x07
070f:  307f  movlw   0x7f
0710:  0581  andwf   0x01, 0x1
0711:  0e06  swapf   0x06, 0x0
0712:  3907  andlw   0x07
0713:  3eff  addlw   0xff
0714:  3040  movlw   0x40
0715:  0086  movwf   0x06
0716:  3670  lsrf    0x70, 0x0
0717:  0087  movwf   0x07
0718:  0c86  rrf     0x06, 0x1
0719:  087c  movf    0x7c, 0x0
071a:  3fc4  movwi   .4[1]
071b:  087d  movf    0x7d, 0x0
071c:  3fc5  movwi   .5[1]
071d:  0804  movf    0x04, 0x0
071e:  3fc6  movwi   .6[1]
071f:  0805  movf    0x05, 0x0
0720:  3fc7  movwi   .7[1]
0721:  3000  movlw   0x00
0722:  3fc8  movwi   .8[1]
0723:  3fc9  movwi   .9[1]
0724:  3fca  movwi   .10[1]
0725:  3fcb  movwi   .11[1]
0726:  3fcd  movwi   .13[1]
0727:  3fce  movwi   .14[1]
0728:  3fcf  movwi   .15[1]
0729:  300a  movlw   0x0a
072a:  3fcc  movwi   .12[1]
072b:  0d06  rlf     0x06, 0x0
072c:  0d07  rlf     0x07, 0x0
072d:  0e09  swapf   0x09, 0x0
072e:  3e10  addlw   0x10
072f:  3970  andlw   0x70
0730:  0084  movwf   0x04
0731:  0185  clrf    0x05
0732:  30d1  movlw   0xd1
0733:  3f80  movwi   .0[0]
0734:  3f82  movwi   .2[0]
0735:  3085  movlw   0x85
0736:  3f83  movwi   .3[0]
0737:  3880  iorlw   0x80
0738:  3f81  movwi   .1[0]
0739:  310c  addfsr  4, .12
073a:  30dc  movlw   0xdc
073b:  001a  movwi   0++
073c:  3085  movlw   0x85
073d:  001a  movwi   0++
073e:  087e  movf    0x7e, 0x0
073f:  3820  iorlw   0x20
0740:  001a  movwi   0++
0741:  309a  movlw   0x9a
0742:  3fc8  movwi   .8[1]
0743:  3081  movlw   0x81
0744:  3fc9  movwi   .9[1]
0745:  3008  movlw   0x08
0746:  3f80  movwi   .0[0]
0747:  0103  clrw
0748:  0084  movwf   0x04
0749:  0085  movwf   0x05
074a:  138b  bcf     0x0b, 0x7
074b:  00fc  movwf   0x7c
074c:  00fd  movwf   0x7d
074d:  00fe  movwf   0x7e
074e:  00ff  movwf   0x7f
074f:  30ae  movlw   0xae
0750:  0084  movwf   0x04
0751:  3007  movlw   0x07
0752:  0085  movwf   0x05
0753:  1385  bcf     0x05, 0x7
0754:  3000  movlw   0x00
0755:  3180  movlp   0x00
0756:  2002  call    0x0002
0757:  138b  bcf     0x0b, 0x7
0758:  0809  movf    0x09, 0x0
0759:  1903  btfsc   0x03, 0x2
075a:  33f9  bra     0x0754
075b:  3907  andlw   0x07
075c:  1903  btfsc   0x03, 0x2
075d:  320f  bra     0x076d
075e:  1909  btfsc   0x09, 0x2
075f:  1c89  btfss   0x09, 0x1
0760:  3201  bra     0x0762
0761:  320b  bra     0x076d
0762:  0e09  swapf   0x09, 0x0
0763:  3e10  addlw   0x10
0764:  3970  andlw   0x70
0765:  3e03  addlw   0x03
0766:  0084  movwf   0x04
0767:  0185  clrf    0x05
0768:  307f  movlw   0x7f
0769:  0580  andwf   0x00, 0x1
076a:  0e04  swapf   0x04, 0x0
076b:  3907  andlw   0x07
076c:  3eff  addlw   0xff
076d:  0103  clrw
076e:  0084  movwf   0x04
076f:  0085  movwf   0x05
0770:  138b  bcf     0x0b, 0x7
0771:  00fc  movwf   0x7c
0772:  00fd  movwf   0x7d
0773:  00fe  movwf   0x7e
0774:  00ff  movwf   0x7f
0775:  30c1  movlw   0xc1
0776:  0084  movwf   0x04
0777:  3007  movlw   0x07
0778:  0085  movwf   0x05
0779:  1385  bcf     0x05, 0x7
077a:  3000  movlw   0x00
077b:  3180  movlp   0x00
077c:  2002  call    0x0002
077d:  138b  bcf     0x0b, 0x7
077e:  0809  movf    0x09, 0x0
077f:  1903  btfsc   0x03, 0x2
0780:  33f9  bra     0x077a
0781:  3907  andlw   0x07
0782:  1903  btfsc   0x03, 0x2
0783:  320f  bra     0x0793
0784:  1909  btfsc   0x09, 0x2
0785:  1c89  btfss   0x09, 0x1
0786:  3201  bra     0x0788
0787:  320b  bra     0x0793
0788:  0e09  swapf   0x09, 0x0
0789:  3e10  addlw   0x10
078a:  3970  andlw   0x70
078b:  3e03  addlw   0x03
078c:  0084  movwf   0x04
078d:  0185  clrf    0x05
078e:  307f  movlw   0x7f
078f:  0580  andwf   0x00, 0x1
0790:  0e04  swapf   0x04, 0x0
0791:  3907  andlw   0x07
0792:  3eff  addlw   0xff
0793:  0020  movlb   0x00
0794:  168b  bsf     0x0b, 0x5
0795:  003f  movlb   0x1f
0796:  300b  movlw   0x0b
0797:  00ed  movwf   0x6d
0798:  3006  movlw   0x06
0799:  00e6  movwf   0x66
079a:  3187  movlp   0x07
079b:  279c  call    0x079c
079c:  178b  bsf     0x0b, 0x7
079d:  3180  movlp   0x00
079e:  2830  goto    0x0030
079f:  0069  dw      0x0069
07a0:  006e  dw      0x006e
07a1:  0066  tris    0x66
07a2:  0069  dw      0x0069
07a3:  006e  dw      0x006e
07a4:  0069  dw      0x0069
07a5:  0074  dw      0x0074
07a6:  0065  tris    0x65
07a7:  0020  movlb   0x00
07a8:  006c  dw      0x006c
07a9:  006f  dw      0x006f
07aa:  006f  dw      0x006f
07ab:  0070  dw      0x0070
07ac:  0000  nop
07ad:  00f2  movwf   0x72
07ae:  33ff  bra     0x07ae
07af:  0063  sleep
07b0:  006f  dw      0x006f
07b1:  006f  dw      0x006f
07b2:  0070  dw      0x0070
07b3:  0065  tris    0x65
07b4:  0072  dw      0x0072
07b5:  0061  halt
07b6:  0074  dw      0x0074
07b7:  0069  dw      0x0069
07b8:  0076  dw      0x0076
07b9:  0065  tris    0x65
07ba:  0020  movlb   0x00
07bb:  006c  dw      0x006c
07bc:  006f  dw      0x006f
07bd:  006f  dw      0x006f
07be:  0070  dw      0x0070
07bf:  0000  nop
07c0:  00ef  movwf   0x6f
07c1:  0808  movf    0x08, 0x0
07c2:  138b  bcf     0x0b, 0x7
07c3:  00fc  movwf   0x7c
07c4:  3004  movlw   0x04
07c5:  3180  movlp   0x00
07c6:  2002  call    0x0002
07c7:  33f9  bra     0x07c1
8007:  37e4  dw      0x37e4
8008:  3ffb  dw      0x3ffb
