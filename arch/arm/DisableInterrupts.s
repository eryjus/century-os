@@===================================================================================================================
@@
@@  DisableInterrupts.s -- Enable Interrupts -- rpi2b specific
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Disable Interrupts
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2018-Nov-21  Initial   0.2.0   ADCL  Initial version
@@  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     DisableInterrupts


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- DisableInterrupts() -- Disable Interrupts for the rpi2b
@@    -------------------------------------------------------
DisableInterrupts:
    push    {r1}                                    @@ be nice and save r1 even though we are not required to
    mrs     r1,cpsr                                 @@ Get the Current Program Status Register
    mov     r0,r1                                   @@ return the previous state
    and     r0,#0x000000c0                          @@ Mask out the interrupt flags
    orr     r1,#0x000000c0                          @@ Set the bits that disable interrupts
    msr     cpsr,r1                                 @@ store the Current Program Status Register again
    pop     {r1}                                    @@ restore r1 since we saved it
    mov     pc,lr                                   @@ return
