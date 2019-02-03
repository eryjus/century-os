@@===================================================================================================================
@@
@@  SuperTarget.s -- This is the target for handling a Supervisor Request
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@    Date      Tracker  Version  Pgmr  Description
@@ -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@ 2018-Dec-01  Initial   0.2.0   ADCL  Initial version
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     SuperTarget


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- This will set up the processor for handling a Supervisor Request
@@    ----------------------------------------------------------------
SuperTarget:
    srsdb       sp!,#0x13                                   @@ save spsr and lr to the svc stack
    cpsid       ifa,#0x13                                   @@ switch to supervisor mode; disable interrupts

    push        {r0-r12}                                    @@ push r0 to r12 onto the stack

    ldr         r0,[lr,#-4]                                 @@ get the instruction that generated the system call
    bic         r0,#0xff000000                              @@ remove the top 8 bits of that instruction
    push        {r0}                                        @@ put this value into the mode field

    push        {sp}
    push        {lr}                                        @@ push the sp and lr registers

    sub         sp,#8                                       @@ make room for the user-space sp and lr registers
    stmia       sp,{sp,lr}^                                 @@ taking these one at a time, we will save the registers

    mov         r0,sp                                       @@ set the register list for the called function
    bl          SyscallHandler                              @@ Handle the interrupt

    ldmia       sp,{sp,lr}^                                 @@ restore the user sp and lr
    add         sp,#8                                       @@ update the tack pointer

    pop         {lr}                                        @@ restore the sp and lr for svc mode
    add         sp,#4                                       @@ throw away the stack pointer

    pop         {r0}                                        @@ pop the temporary r0 value - overwritten later

    pop         {r0-r12}                                    @@ restore all the working registers

    rfeia       sp!                                         @@ return from the exception

