@@===================================================================================================================
@@
@@  IdtSetAddr.s -- Set the address of the exception vector table for rpi2b
@@
@@        Copyright (c)  2017-2018 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Prototype:
@@  void IdtSetAddr(void);
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2018-Nov-28  Initial   0.2.0   ADCL  Initial version
@@
@@===================================================================================================================



    .globl      IdtSetAddr

    .section    .text


IdtSetAddr:
    mrc     p15,0,r0,c1,c0,0
    and     r0,r0,#(~(1<<13))
    mcr     p15,0,r0,c1,c0,0

    mov     r0,#0xff40
    lsl     r0,#16
    add     r0,#0x1000
    mcr     p15,0,r0,c12,c0,0

    mov     pc,lr


