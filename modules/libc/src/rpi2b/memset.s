@@===================================================================================================================
@@
@@  memset.s -- Set a block of memory to the specified value by bytes
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  Set a block of memory to the specified value.  This function operates with bytes being passed into the function,
@@  so cnt contains the number of bytes to fill.
@@
@@  The EABI specifies that function arguments 1-4 are passed in using R0-R3.  Therefore the registers that are
@@  set for this function are:
@@  R0 -- the buffer to set
@@  R1 -- The byte to use as the fill value
@@  R2 -- The number of bytes to fill
@@  LR -- The return instruction counter
@@
@@  Prototype:
@@  void memset(void *buf, uint8_t byt, size_t cnt);
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2019-Jan-19  Initial   0.2.0   ADCL  Copied this file from kMemSetB and modified
@@
@@===================================================================================================================



.globl      memset


.section    .text


memset:
    cmp     r2,#0                                   @@ have we filled all the bytes?
    moveq   pc,lr                                   @@ if we are done, leave

    strb    r1,[r0]                                 @@ store the value in r1 to the mem at addr r0
    add     r0,#1                                   @@ increment the address
    sub     r2,#1                                   @@ decrement the numebr of bytes
    b       memset                                  @@ loop
