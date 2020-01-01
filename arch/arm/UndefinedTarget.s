@@===================================================================================================================
@@
@@  UndefinedTarget.s -- This is the target for handling an Undefined Instruction
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2018-Dec-01  Initial   0.2.0   ADCL  Initial version
@@  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     UndefinedTarget


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- This will set up the processor for handling an Undefined Instruction exception
@@    ------------------------------------------------------------------------------
UndefinedTarget:
    srsdb       sp!,#0x13                                   @@ save spsr and lr to the svc stack
    cpsid       ifa,#0x13                                   @@ switch to supervisor mode; disable interrupts

    push        {r0-r12}                                    @@ push r0 to r12 onto the stack

    mov         r0,#0x1b                                    @@ mode is UNDEF, or 0x1b
    push        {r0}

    push        {sp}
    push        {lr}                                        @@ push the sp and lr registers

    sub         sp,#8                                       @@ make room for the user-space sp and lr registers
    stmia       sp,{sp,lr}^                                 @@ taking these one at a time, we will save the registers

    mov         r0,sp                                       @@ set the register list for the called function
    bl          UndefinedHandler                            @@ Handle the interrupt

    ldmia       sp,{sp,lr}^                                 @@ restore the user sp and lr
    add         sp,#8                                       @@ update the tack pointer

    pop         {lr}                                        @@ restore the sp and lr for svc mode
    add         sp,#4                                       @@ throw away the stack pointer

    pop         {r0}                                        @@ pop the temporary r0 value - overwritten later

    pop         {r0-r12}                                    @@ restore all the working registers

    rfeia       sp!                                         @@ return from the exception

