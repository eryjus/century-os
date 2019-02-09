@@===================================================================================================================
@@
@@  SpinlockAtomicLock.s -- Atomically get exchange a value for a spinlock
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Atomically perform a lock for a spinlock, returning the old value
@@
@@  On entry, this is the state:
@@  r0 -- The address of the Spinlock_t, offset #0 is the actual lock
@@  r1 -- This is the expected value of the lock (which would be #0 for unlocked)
@@  r2 -- This is the value to assign to the lock (or #1)
@@
@@ -----------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2018-Nov-21  Initial   0.2.0         Initial version
@@  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
@@
@@===================================================================================================================


@@
@@ -- Now, we need some things from other functions imported
@@    ------------------------------------------------------
    .global     SpinlockAtomicLock


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- Perform the compare and exchange
@@    --------------------------------
SpinlockAtomicLock:
    mov     r2,r0                           @@ move the address to r2
    mov     r1,#1                           @@ we want to know if the lock is locked
    ldrex   r0,[r2]                         @@ get the status from the lock
    cmp     r0,#0                           @@ is the lock unlocked?
    strexeq r0,r1,[r2]                      @@ if unlocked, try to get the lock
    dmb                                     @@ identify the memory barrier
    mov     pc,lr                           @@ return
