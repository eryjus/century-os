@@===================================================================================================================
@@
@@  GetTTBR0.s -- Get the TTBR0 Register
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Pull the TTBR0 value from cp15 and return its value in r0
@@
@@  Prototype:
@@  ptrsize_t GetTTBR0(void);
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
    .global     GetTTBR0


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- Get the current processor mode
@@    ------------------------------
GetTTBR0:
    mrc     p15,0,r0,c2,c0,0                @@ This gets the cp15 register 2 sub-reg 0
    mov     pc,lr                           @@ return
