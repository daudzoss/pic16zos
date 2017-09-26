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

void stk_line(int* stk, int index, int offset) {
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

void stk_dump(int* stk, int offset) {
  for (int i = 15; i >= 0; i--)
    stk_line(stk, i, offset);
}

void stk_roll(int* stk, int from, int to, int offset) {
  // simulate the zOS_ROL() macro code
  int fsr0 = ;
}

void main(int argc, char** argv) { // won't show corruption of STKPTR, only vals
  int zOS_STK[16], from, to, offset;

  from = (argc > 1) ? atoi(argv[1]) : 0;
  to = (argc > 2) ? atoi(argv[2]) : 0
  offset = (argc > 3) ? atoi(argv[3]) : 0;

  // initial values onto stack (default or redirected stdin)
  if (stk_init(zOS_STK))
    stk_dump(zOS_STK, offset); // user didn't supply all values, so echo back

  if (from && to) {
    stk_roll(zOS_STK, from, to, offset);
    stk_dump(zOS_STK, offset);
  }
}
