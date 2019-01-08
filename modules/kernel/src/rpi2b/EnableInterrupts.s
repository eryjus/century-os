@@===================================================================================================================
@@
@@  EnableInterrupts.s -- Enable Interrupts -- rpi2b specific
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Enable Interrupts
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
    .global     EnableInterrupts


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- EnableInterrupts() -- Enable Interrupts for the rpi2b
@@    -----------------------------------------------------
EnableInterrupts:
    mrs     r0,cpsr                                 @@ Get the Current Program Status Register
    and     r0,#~0x000001c0                         @@ Clear the bits that disable interrupts
    msr     cpsr,r0                                 @@ store the Current Program Status Register again
    mov     pc,lr                                   @@ return
