@@===================================================================================================================
@@
@@  MmuSetTopUserTable.s -- Set the top-level user MMU table value
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2020-Apr-21  Initial  v0.7.0a  ADCL  Initial version
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .globl MmuSetTopUserTable


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section .text


@@
@@ -- MmuSetTopUserTable -- set the TTBR0
@@    -----------------------------------
MmuSetTopUserTable:
    mcr     p15,0,r0,c2,c0,0
    mov     pc,lr

