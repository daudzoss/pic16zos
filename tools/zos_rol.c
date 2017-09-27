#include <stdio.h>
#include <stdlib.h>

// Daud Zoss
// 26 Sep 2017
// usage: yes | ./zos_rol | ./zos_rol 5 [new] | .zos_rol [new] [new2] | ...

int stk_init(int* stk) {
  int retval = 0;
  int i;

  for (i = 15; i >= 0; i--)
    if (scanf("%d", &stk[i]) != 1) {
      stk[i] = i;
      retval = 1;
    }
  return retval; // 0 if 100% initalized from stdin, 1 if any values auto-filled
}

void stk_line(int* stk, int index) {
  int val = stk[index];
  int job = 1 + (int) (val/3);
  int lev = val % 3;
  char c;

  printf("%d ", val);

  switch (lev) {
  case 0: c = 'L'; break;
  case 1: c = 'M'; break;
  case 2: c = 'H'; break;
  }

  if (job < 6)
    fprintf(stderr, "0x%04X: %*d%c\n", index, job*5, job, c);
  else
    fprintf(stderr, "0x%04X: spare stack slot (for interrupts)\n", index);
}

void stk_dump(int* stk) {
  int i;

  for (i = 15; i >= 0; i--)
    stk_line(stk, i);
  printf("\n"); // since stack elements are printed on a single line
}

void stk_vars(const char* instr,
	      int wreg, int stkptr, int temp, int fsr, int base, int* mem) {

  fprintf(stderr, "after '%s'\nWREG=0x%02X STKPTR=0x%02X temp=0x%02X FSR=0x%04X\n",
	  instr, wreg, stkptr, temp, fsr);

  if (mem) { 
    int i;

    fprintf(stderr, " [0x%04X] ", base);
    for (i = 0; i < 30; i++)
      if (i == fsr - base)
	fprintf(stderr, "<%02x>", mem[i] & 0xff);
      else
	fprintf(stderr, " %02x ", mem[i] & 0xff);
    fprintf(stderr, " [0x%04X]\n", base+29);
  }
}

inline void stk_echo(const char* instr) {
  fprintf(stderr, "%s\n", instr);
}

int stk_test(const char* instr, int file, int bit) {
  int flag = file & (1<<bit);
  
  fprintf(stderr, "test '%s': flag is %1d(%s)\n", instr, flag, flag?"set":"clear");
	  
  return flag ? 1 : 0;
}

#define STATUS_Z(setter) (((setter)==0)?(1):(0))
#define SKIP_CLEAR (1)
#define SKIP_SET (0)
#define zOS_TOS (0x0e)

void stk_roll(int* stk, int oldjob, int newjob, int base) {
  // simulate the zOS_ROL() macro code
  int w, stkptr, temp, fsr, mem[30];

  stk_vars("movlw low base : movwf FSRL : movlw high base : movwf FSRH",
	   w = base>>8, stkptr, temp, fsr = base, base, mem);

  stk_vars("movf new,w",
	   w = newjob, stkptr, temp, fsr, base, 0);

  stk_vars("subwf old,w",
	   w -= oldjob, stkptr, temp, fsr, base, 0);

  if (stk_test("btfsc STATUS,Z", STATUS_Z(w), 0) == SKIP_CLEAR) {
    stk_echo("performing 'bra done'");
    goto done;
  } else
    stk_echo("skipping 'bra done'");

  stk_vars("decf WREG,w",
	   w -= 1, stkptr, temp, fsr, base, 0);

  if (stk_test("btfsc WREG,7", w, 7) == SKIP_CLEAR)
    stk_vars("addlw 5",
	     w += 5, stkptr, temp, fsr, base, 0);

  stk_vars("movwf STKPTR",
	   w, stkptr = w, temp, fsr, base, 0);

  stk_vars("rlf STKPTR,f : bcf STKPTR,0",
	   w, stkptr <<= 1, temp, fsr, base, 0);

  stk_vars("addwf STKPTR,w : addlw 2",
	   w += stkptr + 2, stkptr, temp, fsr, base, 0);

  stk_vars("movwf STKPTR : movwf temp",
	   w, stkptr = w, temp = w, fsr, base, 0);

 loop1:
  mem[fsr++ - base] = w = stk[stkptr] & 0x00ff;
  stk_vars("movf TOSL,w : movwi FSR++",
	   w, stkptr, temp, fsr, base, mem);

  mem[fsr++ - base] = w = stk[stkptr] >> 8;
  stk_vars("movf TOSH,w : movwi FSR++",
	   w, stkptr, temp, fsr, base, mem);

  stk_vars("decf STKPTR,f : movlw zOS_TOS",
	   w = zOS_TOS, stkptr -= 1, temp, fsr, base, 0);

  if (stk_test("btfsc STKPTR,4", stkptr, 4) == SKIP_CLEAR)
    stk_vars("movwf STKPTR",
	     w, stkptr = w, temp, fsr, base, 0);

  stk_vars("movf temp,w",
	   w = temp, stkptr, temp, fsr, base, 0);
	   
  stk_vars("xorwf STKPTR,w",
	   w ^= stkptr, stkptr, temp, fsr, base, 0);

  if (stk_test("btfss STATUS,Z", STATUS_Z(w), 0) == SKIP_SET) {
    stk_echo("performing 'bra loop1'");
    goto loop1;
  } else
    stk_echo("skipping 'bra loop1'");
	   
  stk_vars("clrf STKPTR",
	   w, stkptr = 0, temp, fsr, base, 0);

 loop2:
  stk[stkptr] = (w = mem[--fsr - base]) << 8;
  stk_vars("moviw --FSR : movwf TOSH",
	   w, stkptr, temp, fsr, base, mem);

  stk[stkptr] |= (w = mem[--fsr - base]) & 0x00ff;
  stk_vars("moviw --FSR : movwf TOSL",
	   w, stkptr, temp, fsr, base, mem);

  stk_vars("incf STKPTR,w",
	   w = stkptr + 1, stkptr, temp, fsr, base, 0);

  stk_vars("movwf STKPTR",
	   w, stkptr = w, temp, fsr, base, 0);

  stk_vars("sublw zOS_TOS",
	   w = zOS_TOS - w, stkptr, temp, fsr, base, 0);

  if (stk_test("btfss WREG,7", w, 7) == SKIP_SET) {
    stk_echo("performing 'bra loop2'");
    goto loop2;
  } else
    stk_echo("skipping 'bra loop2'");

 done:
//  stk_vars("(completion)",
//	   w, stkptr, temp, fsr, base, mem);
  stk_echo("(next instruction after macro expansion)");
}

void main(int argc, char** argv) { // won't show corruption of STKPTR, only vals
  int stack[16], oldjob, newjob, base;

  oldjob = (argc > 1) ? atoi(argv[1]) : 5; // call without argument to init 1-5
  newjob = (argc > 2) ? atoi(argv[2]) : 0;
  base = (argc > 3) ? atoi(argv[3]) : 0;

  // initial values onto stack (default or redirected stdin)
  if (stk_init(stack))
    stk_dump(stack); // user didn't supply all values, so echo back

  if (oldjob && newjob) {
    fprintf(stderr, "\n\nRolling stack from %d to %d:\n", oldjob, newjob);

    stk_roll(stack, oldjob, newjob, base);
    stk_dump(stack);
  }
}
