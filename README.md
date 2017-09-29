# pic16zos
zOS is a lightweight, small-footprint, preemptively multitasking RTOS for Microchip Technology's entire enhanced midrange 8-bit microcontroller family (14-bit core with part numbers beginning "PIC12" or "PIC16")

zOS supports up to 5 re-entrant jobs with a 2-byte IPC mailbox each and up to 80 bytes of local memory each, which may be created/killed/restarted/slept/paused/forked/replaced in any suitably privileged context as well as searched by handle

A library of macros for instantiating potentially useful jobs eventually will include:

* zOS_ADR/zOS_INT/zOS_LAU, for launching new jobs with a certain start address "handle" and interrupt lines in just a few lines of PIC assembly

* zOS_MUL, a multitasking-aware re-entrant 16x16bit signed multiply (for the PIC16F1616 family equipped with the MathACC core-independent peripheral) that will yield the core to other jobs during the calcuation and spin waits for exclusive access

* zOS_HEA, a set of heap management routines to allow malloc() and free() since jobs are other limited to their local memory of 80 bytes in their bank

* zOS_CON, an interrupt-driven console outputting to any ESUART

* zOS_MON, like zOS_CON but with a command-line interface for monitoring of the PIC memory space

* zOS_MAN, built on top of zOS_CON with additional commands for job management (forking, sleeping, killing, etc.)

* zOS_RUN, for setting up the timer0 peripheral used for the system tick and context-switching quantum...this is the "main loop" that starts everything going after the part and initial job(s) have been set up

Bank switching in the application should not be used or needed outside of the interrupt-handler context, as there is a per-job hardware ISR interface.  User-space communication with the kernel and with other jobs is accomplished with either software interrupts or the mailbox interface.  No job priority or starvation prevention is supported at this time due to the lightweight nature of the round-robin scheduler in zOS.

Minimum system requirements: enhanced 14-bit PIC core device with 512 words flash and 128 bytes SRAM
