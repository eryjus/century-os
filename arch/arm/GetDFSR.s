@@===================================================================================================================
@@
@@  GetDFSR.s -- Get the Data Fault Status Register (DFSR)
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Pull the DFSR value from cp15 and return its value in r0
@@
@@  Prototype:
@@  ptrsize_t GetDFSR(void);
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2019-Feb-17  Initial   0.3.0   ADCL  Initial Version
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     GetDFSR


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- Get the current processor mode
@@    ------------------------------
GetDFSR:
    mrc     p15,0,r0,c5,c0,0                @@ This gets the cp15 register 2 sub-reg 0
    mov     pc,lr                           @@ return
