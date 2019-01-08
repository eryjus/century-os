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
    push    {r3,fp,lr}                      @@ save the frame pointer and return register, be nice and save r3
    mov     fp,sp                           @@ and create a new frame

loop:
    strex   r3,r2,[r0]                      @@ attempt to lock the spinlock
    cmp     r3,r1                           @@ did we get the lock?
    bne     loop                            @@ if not, try again

    mov     sp,fp                           @@ restore the stack
    pop     {r3,fp,lr}                      @@ and the previous frame and return register
    mov     pc,lr                           @@ return
