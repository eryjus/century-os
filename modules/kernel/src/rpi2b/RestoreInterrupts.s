@@===================================================================================================================
@@
@@  RestoreInterrupts.s -- Restore Interrupts -- rpi2b specific
@@
@@        Copyright (c)  2017-2018 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Restore Interrupts
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@    Date      Tracker  Version  Pgmr  Description
@@ -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@ 2018-Nov-21  Initial   0.2.0   ADCL  Initial version
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     RestoreInterrupts


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- RestoreInterrupts() -- Restore Interrupts for the rpi2b
@@    -------------------------------------------------------
RestoreInterrupts:
    push    {r1}                                    @@ be nice and save r1 even though we are not required to
    mrs     r1,cpsr                                 @@ Get the Current Program Status Register
    and     r1,#~0x000001c0                         @@ Clear the bits that disable interrupts
    orr     r1,r0                                   @@ Restore the bits that disable interrupts from the parameter
    msr     cpsr,r1                                 @@ store the Current Program Status Register again
    pop     {r1}                                    @@ restore r1 since we saved it
    mov     pc,lr                                   @@ return
