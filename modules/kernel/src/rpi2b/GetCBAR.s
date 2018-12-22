@@===================================================================================================================
@@
@@  GetCBAR.s -- Get the Configuration Base Address Register (CBAR)
@@
@@        Copyright (c)  2017-2018 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Pull the CBAR address from cp15, c15 and return its value in r0
@@
@@  Prototype:
@@  ptrsize_t GetCBAR(void);
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@    Date      Tracker  Version  Pgmr  Description
@@ -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@ 2018-Dec-02  Initial   0.2.0   ADCL  Initial version
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     GetCBAR


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- Get the current processor mode
@@    ------------------------------
GetCBAR:
    mrc     p15,4,r0,c15,c0,0               @@ This gets the cp15 register 15 sub-reg 0
    mov     pc,lr                           @@ return
