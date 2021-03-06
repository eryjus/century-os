@@===================================================================================================================
@@
@@  ResetTarget.s -- This is the target for handling a Reset
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2018-Dec-01  Initial   0.2.0   ADCL  Initial version
@@  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     ResetTarget


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- This will set up the processor for handling a Reset
@@    ---------------------------------------------------
ResetTarget:
    cpsid       ifa,#0x13                                   @@ switch to supervisor mode; disable interrupts
    stm         sp,{r14}^                                   @@ taking these one at a time, we will save the registers
    stm         sp,{r13}^
    push        {r12}
    push        {r11}
    push        {r10}
    push        {r9}
    push        {r8}
    push        {r7}
    push        {r6}
    push        {r5}
    push        {r4}
    push        {r3}
    push        {r2}
    push        {r1}
    push        {r0}
    mrs         r0,spsr
    push        {r0}
    mov         r0,sp
    bl          ResetHandler                                @@ never returns
