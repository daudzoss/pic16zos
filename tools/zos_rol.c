#include <stdio.h>
#include <stdlib.h>

int stk_init(int* stk) {
  int retval = 0;

  for (int i = 15; i >= 0; i--)
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

  printf("%d\n", val);

  switch (lev) {
  case 0: c = 'L'; break;
  case 1: c = 'M'; break;
  case 2: c = 'H'; break;
  }

  if (job < 6)
    fprintf(stderr, "0x%04X: %*d%c", index, job*5, job, c);
  else
    fprintf(stderr, "0x%04X: spare stack slot (for interrupts)\n", index);
}

void stk_dump(int* stk) {
  for (int i = 15; i >= 0; i--)
    stk_line(stk, i);
}

void stk_vars(const char* instr,
	      int wreg, int stkptr, int temp, int fsr, int base, int* mem) {

  fprintf(stderr, "after '%s' w=0x%02X,stkptr=0x%02X,temp=0x%02X,FSR=0x%04X\n",
	  instr, wreg, stkptr, temp, fsr);
  fprintf(stderr, " [0x%04X]", base);
  for (int i = 0; i < 30; i++)
    fprintf(stderr, " %02x", mem[i-base]);
  fprintf(stderr, " [0x%04X]\n", base+30);
}

inline void stk_echo(const char* instr) {
  fprintf(stderr, "performing '%s'\n", instr);
}

int stk_test(const char* instr, int file, int bit) {
  int flag = file & (1<<bit);
  
  fprintf("testing '%s': flag is %1d(%s)\n", instr, flag, flag ? "set":"clear");
	  
  return flag ? 1 : 0;
}

#define STATUS_Z(setter) (((setter)==0)?(1):(0))
#define SKIP_CLEAR 1
#define SKIP_SET 0

void stk_roll(int* stk, int oldjob, int newjob, int base) {
  // simulate the zOS_ROL() macro code
  int w, stkptr, temp, fsr, mem[30];

  stk_vars("movlw low base : movwf FSRL : movlw high base : movwf FSRH",
	   w = base>>8, stkptr, temp, fsr = base, base, mem);
  stk_vars("movf new,w",
	   w = newjob, stkptr, temp, fsr, base, mem);
  stk_vars("subwf old,w",
	   w -= oldjob, stkptr, temp, fsr, base, mem);
  if (stk_test("btfsc STATUS,Z", STATUS_Z(w), 0) == SKIP_CLEAR) {
    stk_echo("bra done");
    goto done;
  }
  stk_vars("decf WREG,w"





	   
}

void main(int argc, char** argv) { // won't show corruption of STKPTR, only vals
  int stack[16], oldjob, newjob, base;

  oldjob = (argc > 1) ? atoi(argv[1]) : 0;
  newjob = (argc > 2) ? atoi(argv[2]) : 0
  base = (argc > 3) ? atoi(argv[3]) : 0;

  // initial values onto stack (default or redirected stdin)
  if (stk_init(stack))
    stk_dump(stack, base); // user didn't supply all values, so echo back

  if (oldjob && newjob) {
    stk_roll(stack, oldjob, newjob, base);
    stk_dump(stack, base);
  }
}
