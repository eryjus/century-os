@@===================================================================================================================
@@
@@  SpinLock.s -- Spin until we can get a lock
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
    .global     SpinLock


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- Get a spinlock (interrupts are disabled ahead of this call)
@@    -----------------------------------------------------------
SpinLock:
    mov     r1,#1                       @@ the lock value
1:  ldrex   r2,[r0]                     @@ read the value and start the atomic operation
    cmp     r2,#0                       @@ is the lock currently unlocked
    wfene                               @@ if not, wait for a signal to try again
    strexeq r2,r1,[r0]                  @@ store the value (r2 is 0 if successful)
    cmpeq   r2,#0                       @@ now check if successful
    bne     1b                          @@ if wasn't unlocked or store was not successful, try again
    dmb                                 @@ make sure all other cores can see the change

    mov     pc,lr


