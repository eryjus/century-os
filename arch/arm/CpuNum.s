@@===================================================================================================================
@@
@@  CpuNum.s -- Get the current CPU Number the current process is executing on
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@  2019-Jun-07  Initial   0.4.5   ADCL  Initial version
@@
@@===================================================================================================================


@@
@@ -- explose some global symbols
@@    ---------------------------
    .global     CpuNum


@@
@@ -- figure out which CPU we are on; only CPU 0 continues after this
@@    ---------------------------------------------------------------
CpuNum:
    mrc     p15,0,r0,c0,c0,5            @@ Read Multiprocessor Affinity Register
    and     r0,r0,#0x3                  @@ Extract CPU ID bits
    mov     pc,lr                       @@ return
