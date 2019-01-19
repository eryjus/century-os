@@===================================================================================================================
@@
@@  SpinlockClear.s -- clear a spinlock
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Unlock a spinlock
@@
@@  On entry, this is the state:
@@  r0 -- The address of the Spinlock_t, offset #0 is the actual lock
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
    .global     SpinlockClear


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- Perform the compare and exchange
@@    --------------------------------
SpinlockClear:
    push    {r1,r2,fp,lr}                   @@ save the frame pointer and return register, be nice and save r1,r2
    mov     fp,sp                           @@ and create a new frame

    mov     r1,#0                           @@ Set the value that means unlocked
    dmb                                     @@ Set up memory synchronization
    str     r1,[r0]                         @@ attempt to lock the spinlock
    dsb                                     @@ Synchronize the memory
    sev                                     @@ wake up any sleeping processors

    mov     sp,fp                           @@ restore the stack
    pop     {r1,r2,fp,lr}                   @@ and the previous frame and return register
    mov     pc,lr                           @@ return
