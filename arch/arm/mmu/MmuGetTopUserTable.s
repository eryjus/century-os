@@===================================================================================================================
@@
@@  MmuGetTopUserTable.s -- Return the top-level user MMU table value
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
@@  2020-Apr-27  Initial  v0.7.0a  ADCL  Rewrite the MMU code
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .globl MmuGetTopUserTable


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section .text


@@
@@ -- MmuGetTopUserTable -- get the TTBR0
@@    -----------------------------------
MmuGetTopUserTable:
    mrc     p15,0,r0,c2,c0,0
    mov     pc,lr

