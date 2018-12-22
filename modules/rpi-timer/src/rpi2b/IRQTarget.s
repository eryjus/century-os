@@===================================================================================================================
@@
@@  IRQTarget.s -- This is the target for handling an IRQ
@@
@@        Copyright (c)  2017-2018 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@    Date      Tracker  Version  Pgmr  Description
@@ -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@ 2018-Dec-14  Initial   0.2.0   ADCL  Initial version -- copied from the kernel
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     IRQTarget


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- This will set up the processor for handling an IRQ
@@    --------------------------------------------------
IRQTarget:
    cpsid   aif,#0x13                   @@ switch to svc mode; disable interrupts
    mov     sp,#0x4000                  @@ set up a stack
    mov     r0,#'#'
    bl      SerialPutChar
    bl      Halt
