@@===================================================================================================================
@@
@@  kStrCmp.s -- compare 2 strings
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  compare 2 strings to each other by performing the subtraction str1[n] - *str2[n] until we have result other
@@  than 0 and neither character is null.
@@
@@  Prototype:
@@  void kStrCmp(char *str1, char *Str2);
@@
@@ -----------------------------------------------------------------------------------------------------------------
@@
@@    Date      Tracker  Version  Pgmr  Description
@@ -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@ 2017-Oct-12                          Initial version
@@ 2018-Jun-23  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
@@
@@===================================================================================================================



@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .globl  kStrCmp


@@
@@ -- Compare 2 strings
@@    -----------------
kStrCmp:
    mov     r2,r0                       @@ str1 is in r2

.loop:
    ldrb    r0,[r2]                     @@ get the character
    and     r0,#0xff                    @@ filter out the character
    ldrb    r3,[r1]                     @@ perform the subtraction
    and     r3,#0xff                    @@ filter out the character
    sub     r0,r3                       @@ subtract
    cmp     r0,#0                       @@ is there a difference?
    bne     .out                        @@ if there is a difference, exit

    ldrb    r0,[r2]                     @@ get the character
    and     r0,#0xff                    @@ filter out the character
    cmp     r0,#0                       @@ end of the string?
    beq      .out                       @@ if null, we're done

    ldrb    r0,[r1]                     @@ get the character
    and     r0,#0xff                    @@ filter out the character
    cmp     r0,#0                       @@ end of this string?
    beq     .out                        @@ if null, we're done

    add     r2,#1                       @@ next char
    add     r1,#1                       @@ next char
    b       .loop

.out:
    mov     pc,lr



