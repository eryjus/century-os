@@===================================================================================================================
@@
@@  kMemMove.s -- Move a counted number of bytes from one mem location to another
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  The EABI specifies that function arguments 1-4 are passed in using R0-R3.  Therefore the registers that are
@@  set for this function are:
@@  R0 -- the destination buffer
@@  R1 -- the source buffer
@@  R2 -- the nuymber of bytes to move
@@  LR -- The return instruction counter
@@
@@  Prototype:
@@  void kMemMove(void *tgt, void *src, size_t cnt);
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2018-Nov-13  Initial   0.2.0   ADCL  Initial version
@@
@@===================================================================================================================



.globl      kMemMove


.section    .text


kMemMove:
    push    {r3}                                    @@ we're going to use r3, so we will be nice and save it

.loop:
    ldrb    r3,[r1]                                 @@ get the next character
    add     r1,#1                                   @@ update the address
    cmp     r2,#0                                   @@ is this the end of the string?
    beq     .out                                    @@ if so we leave

    strb    r3,[r0]                                 @@ store the value in r3 to the mem at addr r0
    add     r0,#1                                   @@ update the address
    sub     r2,#1                                   @@ decrement the number of bytes
    b       .loop                                   @@ loop

.out:
    pop     {r3}                                    @@ restore r3
    mov     pc,lr                                   @@ leave
