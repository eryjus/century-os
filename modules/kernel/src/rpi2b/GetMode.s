@@===================================================================================================================
@@
@@  GetMode.s -- Get the current Mode -- rpi2b specific
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@    Date      Tracker  Version  Pgmr  Description
@@ -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@ 2018-Nov-28  Initial   0.2.0   ADCL  Initial version
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     GetMode


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- Get the current processor mode
@@    ------------------------------
GetMode:
    mrs     r0,cpsr                                 @@ Get the Current Program Status Register
    and     r0,#0x0000001f                          @@ Mask out the mode bits
    mov     pc,lr                                   @@ return
