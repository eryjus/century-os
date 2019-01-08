@@===================================================================================================================
@@
@@  kStrLen.s -- Get the length of a NULL terminated string
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  The EABI specifies that function arguments 1-4 are passed in using R0-R3.  Therefore the registers that are
@@  set for this function are:
@@  R0 -- The string to measure
@@  LR -- The return instruction counter
@@
@@  Prototype:
@@  void kStrLen(char *src);
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2018-Nov-13  Initial   0.2.0   ADCL  Initial version
@@
@@===================================================================================================================



.globl      kStrLen


.section    .text


kStrLen:
    push    {r1,r3}                                 @@ we're going to use r3, so we will be nice and save it
    mov     r3,r0                                   @@ use r3 as our working var
    eor     r0,r0                                   @@ clear our return value to 0

.loop:
    ldrb    r1,[r3]                                 @@ get the next character
    add     r3,#1                                   @@ update the address
    cmp     r1,#0                                   @@ is this the end of the string?
    beq     .out                                    @@ if so we leave

    add     r0,#1                                   @@ increment the character count
    b       .loop                                   @@ loop

.out:
    pop     {r1,r3}                                 @@ restore r3
    mov     pc,lr                                   @@ leave
