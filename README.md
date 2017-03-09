# pic16zos
zOS is a lightweight, small-footprint, preemptively multitasking RTOS for Microchip Technology's entire enhanced midrange 8-bit microcontroller family (14-bit core with part numbers beginning "PIC12" or "PIC16")

zOS supports up to 5 re-entrant jobs with a 2-byte IPC mailbox each and up to 80 bytes of local memory each, which may be created/killed/restarted/slept/paused/forked/replaced in any suitably privileged context as well as searched by handle

zOS_MUL implements a multitasking-aware re-entrant 16x16bit signed multiply (for the PIC16F1616 family equipped with the MathACC core-independent peripheral) that will yield the core to other jobs during the calcuation

A library of macros for instantiating potentially useful jobs eventually will include:
  zOS_CON, an interrupt-driven console outputting to any ESUART
  zOS_MON, like the above but with a command-line interface for job management and monitoring of the PIC memory space
  zOS_HEA, a simple memory management server for malloc() and fre()

Bank switching in the application should not be used or needed outside of the interrupt-handler context, as there is a per-job hardware ISR interface.  User-space communication with the kernel and with other jobs is accomplished with either software interrupts or the mailbox interface.  No job priority or starvation prevention is supported at this time due to the lightweight nature of zOS.

Minimum system requirements:
enhanced 14-bit PIC core
512 words flash
128 bytes SRAM
