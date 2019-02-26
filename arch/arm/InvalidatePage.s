@@===================================================================================================================
@@
@@  InvalidatePage.s -- Invalidate a page from the TLB
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2019-Feb-16  Initial   0.3.0   ADCL  Initial version
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     InvalidatePage


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- Get the current processor mode
@@    ------------------------------
InvalidatePage:
    mcr     p15,0,r0,c8,c7,3                @@ Perform an tlbimvaa invalidate
    dsb                                     @@ Synchronize the memory operations
    isb                                     @@ Flush the fetch cache
    mov     pc,lr                           @@ return
