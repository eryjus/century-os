@@===================================================================================================================
@@
@@  IdtSetAddr.s -- Set the address of the exception vector table for rpi2b
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
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
@@  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
@@
@@===================================================================================================================


    .include "constants.inc"

    .globl      IdtSetAddr

    .section    .text


IdtSetAddr:
    mrc     p15,0,r0,c1,c0,0
    and     r0,r0,#(~(1<<13))
    mcr     p15,0,r0,c1,c0,0

    mov     r0,#(EXCEPT_VECTOR_TABLE & 0xffff)
    movt    r0,#(EXCEPT_VECTOR_TABLE >> 16)
    mcr     p15,0,r0,c12,c0,0

    mov     pc,lr


