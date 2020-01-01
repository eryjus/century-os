@@===================================================================================================================
@@
@@  EnterSystemMode.s -- Change the CPU to system mode
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Since there are several registers that will be banked which also include the `sp`, we will trash several
@@  registers that the ABI says we are allowed to trash.  When leaving this function, we need to stack to be the
@@  same as it was going in.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@  2018-Dec-01  Initial   0.2.0   ADCL  Initial version
@@  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     EnterSystemMode


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


EnterSystemMode:
    mov     r0,sp                           @@ get the stack register
    mov     r1,lr                           @@ get the return point
    mrs     r2,cpsr                         @@ get the current status register
    and     r2,#~0x001f                     @@ mask out the mode bits so we do not update those

    cps     #0x1f                           @@ switch to system mode, use the user registers

    mov     sp,r0                           @@ and restore the stack to the one we were using
    mrs     r0,cpsr                         @@ now, get system mode status register
    and     r0,#0x001f                      @@ and only keep the mode bits
    orr     r0,r0,r2                        @@ overlay the other status bits
    msr     cpsr,r0                         @@ and store the status register

    mov     pc,r1                           @@ and return from this function

