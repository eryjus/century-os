@@===================================================================================================================
@@
@@  IRQTarget.s -- This is the target for handling an IRQ
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@    Date      Tracker  Version  Pgmr  Description
@@ -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@ 2018-Dec-14  Initial   0.2.0   ADCL  Initial version -- copied from the kernel
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     IRQTarget


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- This will set up the processor for handling an IRQ
@@    --------------------------------------------------
IRQTarget:
@@    cpsid   aif,#0x13                   @@ switch to svc mode; disable interrupts
@@    mov     sp,#0x4000                  @@ set up a stack
@@    mov     r0,#'#'
@@    bl      SerialPutChar
@@    bl      Halt
    sub         lr,lr,#4                                    @@ adjust the return address
    srsdb       sp!,#0x13                                   @@ save spsr and lr to the svc stack
    cpsid       ifa,#0x13                                   @@ switch to supervisor mode; disable interrupts

    push        {r0-r12}                                    @@ push r0 to r12 onto the stack

@@    sub         sp,#8                                       @@ make room for the user-space sp and lr registers
@@    stmia       sp,{sp,lr}^                                 @@ taking these one at a time, we will save the registers

    bl          IRQHandler                                  @@ Handle the interrupt

@@    ldmia       sp,{sp,lr}^                                 @@ restore the user sp and lr
@@    add         sp,#8                                       @@ update the tack pointer

    pop         {r0-r12}                                    @@ restore all the working registers

    rfeia       sp!                                         @@ return from the exception
