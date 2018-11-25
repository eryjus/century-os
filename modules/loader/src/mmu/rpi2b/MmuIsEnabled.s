@@===================================================================================================================
@@
@@  MmuIsEnabled.s -- Determine if the MMU is enabled
@@
@@        Copyright (c)  2017-2018 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Query CP15 Register 1 to determine the MMU state.  Mask out the MMU bit and return it.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@  2018-Nov-24  Initial   0.2.0   ADCL  Initial version
@@
@@===================================================================================================================


.section        .text
.globl          MmuIsEnabled


@@
@@ -- Get the cp15 Register 1 value and return if the MMU is enabled
@@    --------------------------------------------------------------
MmuIsEnabled:
    mrc     p15,0,r0,c1,c0,0                @@ This gets the cp15 register 1 and puts it in r0
    and     r0,#1                           @@ mask out bit 0
    mov     pc,lr                           @@ return

