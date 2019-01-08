@@===================================================================================================================
@@
@@  MmuEnablePaging.s -- Turn paging on
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Sets the TTLR0, TTLR1 and the number of control bits to determine which TTLR to use, and then enables the MMU.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@  2018-Nov-24  Initial   0.2.0   ADCL  Initial version
@@
@@===================================================================================================================


.section        .text
.globl          MmuEnablePaging


@@
@@ -- Enable the MMU
@@    --------------
MmuEnablePaging:
    mcr     p15,0,r0,c2,c0,0                @@ write the ttl1 table to the TTLR0 register
    mcr     p15,0,r0,c2,c0,1                @@ write the ttl1 table to the TTLR1 register

    mov     r0,#0                           @@ This is the number of bits to use to determine which table
    mcr     p15,0,r0,c2,c0,2                @@ write these to the control register

    mov     r0,#0xffffffff                  @@ All domains can access all things by default
    mcr     p15,0,r0,c3,c0,0                @@ write these to the domain access register

    mrc     p15,0,r0,c1,c0,0                @@ This gets the cp15 register 1 and puts it in r0
    orr     r0,#1                           @@ set bit 0
    mcr     p15,0,r0,c1,c0,0                @@ Put the cp15 register 1 back, with the MMU enabled

    mov     pc,lr

