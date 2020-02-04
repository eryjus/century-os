@@===================================================================================================================
@@
@@  kMemSetW.s -- Set a block of memory to the specified value by half words
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Set a block of memory to the specified value.  This function operates with half words being passed into the
@@  function, so cnt contains the number of half words to fill.
@@
@@  The EABI specifies that function arguments 1-4 are passed in using R0-R3.  Therefore the registers that are
@@  set for this function are:
@@  R0 -- the buffer to set
@@  R1 -- The half words to use as the fill value
@@  R2 -- The number of half words to fill
@@  LR -- The return instruction counter
@@
@@  Prototype:
@@  void kMemSetW(void *buf, uint16_t hw, size_t cnt);
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2018-Nov-13  Initial   0.2.0   ADCL  Initial version
@@  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
@@
@@===================================================================================================================



.globl      kMemSetW


.section    .text


kMemSetW:
    cmp     r2,#0                                   @@ have we filled all the half words?
    moveq   pc,lr                                   @@ if we are done, leave

    strh    r1,[r0]                                 @@ store the value in r1 to the mem at addr r0
    add     r0,#2                                   @@ move to the next address
    sub     r2,#1                                   @@ decrement the numebr of half words
    b       kMemSetW                                @@ loop
