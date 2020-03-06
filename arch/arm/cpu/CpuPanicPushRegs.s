@@===================================================================================================================
@@
@@  CpuPanicPushRegs.s -- Panic halt all the cpus, pushing the registers
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2020-Mar-04  Initial  v0.5.0h  ADCL  Initial Version
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     CpuPanicPushRegs


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- Push all the registers and then Panic all CPUs
@@    ----------------------------------------------
CpuPanicPushRegs:
    srsdb       sp!,#0x13                                   @@ save spsr and lr to the svc stack
    cpsid       ifa,#0x13                                   @@ switch to supervisor mode; disable interrupts
    push        {r0-r12}
    push        {lr}
    push        {sp}
    push        {lr}
    sub         sp,#8
    stmia       sp,{sp,lr}^
    mov         r1,sp

    bl          CpuPanic

