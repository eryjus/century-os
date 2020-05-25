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
@@  2020-Apr-30  Initial  v0.7.0a  ADCL  Rewrite the MMU code
@@
@@===================================================================================================================


@@
@@ -- make sure that if the required symbols are defined
@@    --------------------------------------------------
.ifndef ENABLE_BRANCH_PREDICTOR
    .equ        ENABLE_BRANCH_PREDICTOR,0
.endif


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
    mov     r1,#0

    mcrr    p15,0,r0,r1,c2                      @@ write the ttl1 table to the TTBR0 register
    mcr     p15,0,r0,c8,c7,0                    @@ shoot down the entire TLB

.if ENABLE_BRANCH_PREDICTOR
    mcrne   p15,0,r0,c7,c5,6                @@ invalidate the branch predictor (required maintenance when enabled)
.endif

    mov     pc,lr

