@@===================================================================================================================
@@
@@  SpinUnlock.s -- Unlock a spinlock
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2019-Jun-13  Initial   0.4.6   ADCL  Initial version
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     SpinUnlock


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- Unlock a spinlock
@@    -----------------
SpinUnlock:
    mov     r1,#0                       @@ the unlock value
    dmb                                 @@ make sure all other cores complete their reads
    str     r1,[r0]                     @@ unlock the lock
    dsb                                 @@ synchronize the update
    sev                                 @@ send an event to release any waiting cores

    mov     pc,lr


