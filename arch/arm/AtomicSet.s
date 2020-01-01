@@===================================================================================================================
@@
@@  AtomicSet.s -- Set the Atomic Integer to a value, returning the previous value.
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Prototype:
@@  int32_t AtomicSet(AtomicInt *a, int32_t value);
@@
@@ -----------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2019-Apr-28  Initial   0.4.2   ADCL  Initial version
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     AtomicSet


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- Set an AtomicInt_t variable to a value
@@    --------------------------------------
AtomicSet:
    mov    r2,r0                @@ -- move the address out of the way
1:  ldrex  r0,[r2]              @@ -- get the return value from the structure
    strex  r3,r1,[r2]           @@ -- store the new value; r3 == 0 when successful
    tst    r3,#0                @@ -- check for success
    bne    1b                   @@ -- the value was updated; try again
    dmb                         @@ -- make sure memory is updated

    mov     pc,lr


