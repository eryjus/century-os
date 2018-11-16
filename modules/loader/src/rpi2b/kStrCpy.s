@@===================================================================================================================
@@
@@  kStrCpy.s -- Copy a NULL terminated string from one location to another
@@
@@        Copyright (c)  2017-2018 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  The EABI specifies that function arguments 1-4 are passed in using R0-R3.  Therefore the registers that are
@@  set for this function are:
@@  R0 -- the destination buffer
@@  R1 -- the source buffer
@@  LR -- The return instruction counter
@@
@@  Prototype:
@@  void kStrCpy(char *dest, char *src);
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2018-Nov-13  Initial   0.2.0   ADCL  Initial version
@@
@@===================================================================================================================



.globl      kStrCpy


.section    .text


kStrCpy:
    push    {r3}                                    @@ we're going to use r3, so we will be nice and save it

.loop:
    ldrb    r3,[r1]!                                @@ get the next character and update the address
    cmp     r3,#0                                   @@ is this the end of the string?
    beq     .out                                    @@ if so we leave

    strb    r3,[r0]!                                @@ store the value in r3 to the mem at addr r0 and update r0
    b       .loop                                   @@ loop

.out:
    pop     {r3}                                    @@ restore r3
    mov     pc,lr                                   @@ leave
