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
@@ 2018-Dec-14  Initial   0.2.0   ADCL  Initial version
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     GetCBAR
    .global     GetMode
    .global     GetVBAR
    .global     GetSCTLR
    .global     Undef


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


@@
@@ -- Get the current vector address
@@    ------------------------------
GetVBAR:
    mrc     p15,0,r0,c12,c0,0               @@ This gets the cp15 register 12 sub-reg 0
    mov     pc,lr                           @@ return


@@
@@ -- Get system control register
@@    ---------------------------
GetSCTLR:
    mrc     p15,0,r0,c1,c0,0                @@ This gets the control register
    mov     pc,lr                           @@ return


@@
@@ -- Get current mode
@@    ----------------
GetMode:
    mrs     r0,cpsr                         @@ This gets the current program status register
    and     r0,#0x1f                        @@ mask the bits to return
    mov     pc,lr                           @@ return


@@
@@ -- execute an undefined instruction
@@    --------------------------------
Undef:
    .word 0xffffffff

