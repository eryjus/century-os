@@===================================================================================================================
@@
@@  LoaderExceptionTargets.s -- These are the targets for handling exceptions during the loader operations
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2019-Apr-02  Initial   0.4.0   ADCL  Relocated
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .global     LoaderResetTarget
    .global     LoaderUndefinedTarget
    .global     LoaderSuperTarget
    .global     LoaderPrefetchTarget
    .global     LoaderDataAbortTarget
    .global     LoaderIRQTarget
    .global     LoaderFIQTarget


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .ldrtext,"ax"


@@
@@ -- This will set up the processor for handling a Reset
@@    ---------------------------------------------------
LoaderResetTarget:
    cpsid       ifa,#0x13                                   @@ switch to supervisor mode; disable interrupts
    stm         sp,{r14}^                                   @@ taking these one at a time, we will save the registers
    stm         sp,{r13}^
    push        {r12}
    push        {r11}
    push        {r10}
    push        {r9}
    push        {r8}
    push        {r7}
    push        {r6}
    push        {r5}
    push        {r4}
    push        {r3}
    push        {r2}
    push        {r1}
    push        {r0}
    mrs         r0,spsr
    push        {r0}
    mov         r0,sp
.halt:
    wfe
    b           .halt                                @@ never returns


@@
@@ -- This will set up the processor for handling an Undefined Instruction exception
@@    ------------------------------------------------------------------------------
LoaderUndefinedTarget:
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
    bl          LoaderExceptHandler                         @@ Handle the interrupt

    ldmia       sp,{sp,lr}^                                 @@ restore the user sp and lr
    add         sp,#8                                       @@ update the tack pointer

    pop         {lr}                                        @@ restore the sp and lr for svc mode
    add         sp,#4                                       @@ throw away the stack pointer

    pop         {r0}                                        @@ pop the temporary r0 value - overwritten later

    pop         {r0-r12}                                    @@ restore all the working registers

    rfeia       sp!                                         @@ return from the exception


@@
@@ -- This will set up the processor for handling a Supervisor Request
@@    ----------------------------------------------------------------
LoaderSuperTarget:
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
    bl          LoaderExceptHandler                         @@ Handle the interrupt

    ldmia       sp,{sp,lr}^                                 @@ restore the user sp and lr
    add         sp,#8                                       @@ update the tack pointer

    pop         {lr}                                        @@ restore the sp and lr for svc mode
    add         sp,#4                                       @@ throw away the stack pointer

    pop         {r0}                                        @@ pop the temporary r0 value - overwritten later

    pop         {r0-r12}                                    @@ restore all the working registers

    rfeia       sp!                                         @@ return from the exception


@@
@@ -- This will set up the processor for handling a prefetch abort
@@    ------------------------------------------------------------
LoaderPrefetchTarget:
    sub         lr,lr,#4                                    @@ adjust the return address
    srsdb       sp!,#0x13                                   @@ save spsr and lr to the svc stack
    cpsid       ifa,#0x13                                   @@ switch to supervisor mode; disable interrupts

    push        {r0-r12}                                    @@ push r0 to r12 onto the stack

    mov         r0,#0x18                                    @@ mode is Abort, or 0x17
    push        {r0}

    push        {sp}
    push        {lr}                                        @@ push the sp and lr registers

    sub         sp,#8                                       @@ make room for the user-space sp and lr registers
    stmia       sp,{sp,lr}^                                 @@ taking these one at a time, we will save the registers

    mov         r0,sp                                       @@ set the register list for the called function
    bl          LoaderExceptHandler                         @@ Handle the interrupt

    ldmia       sp,{sp,lr}^                                 @@ restore the user sp and lr
    add         sp,#8                                       @@ update the tack pointer

    pop         {lr}                                        @@ restore the sp and lr for svc mode
    add         sp,#4                                       @@ throw away the stack pointer

    pop         {r0}                                        @@ pop the temporary r0 value - overwritten later

    pop         {r0-r12}                                    @@ restore all the working registers

    rfeia       sp!                                         @@ return from the exception



@@
@@ -- This will set up the processor for handling a Data Abort
@@    --------------------------------------------------------
LoaderDataAbortTarget:
    sub         lr,lr,#8                                    @@ adjust the return address
    srsdb       sp!,#0x13                                   @@ save spsr and lr to the svc stack
    cpsid       ifa,#0x13                                   @@ switch to supervisor mode; disable interrupts

    push        {r0-r12}                                    @@ push r0 to r12 onto the stack

    mov         r0,#0x17                                    @@ mode is Abort, or 0x17
    push        {r0}

    push        {sp}
    push        {lr}                                        @@ push the sp and lr registers

    sub         sp,#8                                       @@ make room for the user-space sp and lr registers
    stmia       sp,{sp,lr}^                                 @@ taking these one at a time, we will save the registers

    mov         r0,sp                                       @@ set the register list for the called function
    bl          LoaderExceptHandler                         @@ Handle the interrupt

    ldmia       sp,{sp,lr}^                                 @@ restore the user sp and lr
    add         sp,#8                                       @@ update the tack pointer

    pop         {lr}                                        @@ restore the sp and lr for svc mode
    add         sp,#4                                       @@ throw away the stack pointer

    pop         {r0}                                        @@ pop the temporary r0 value - overwritten later

    pop         {r0-r12}                                    @@ restore all the working registers

    rfeia       sp!                                         @@ return from the exception

@@
@@ -- This will set up the processor for handling an IRQ
@@    --------------------------------------------------
LoaderIRQTarget:
    sub         lr,lr,#4                                    @@ adjust the return address
    srsdb       sp!,#0x13                                   @@ save spsr and lr to the svc stack
    cpsid       ifa,#0x13                                   @@ switch to supervisor mode; disable interrupts

    push        {r0-r12}                                    @@ push r0 to r12 onto the stack

    mov         r0,#0x12                                    @@ mode is IRQ, or 0x12
    push        {r0}

    push        {sp}
    push        {lr}                                        @@ push the sp and lr registers

    sub         sp,#8                                       @@ make room for the user-space sp and lr registers
    stmia       sp,{sp,lr}^                                 @@ taking these one at a time, we will save the registers

    mov         r0,sp                                       @@ set the register list for the called function
    bl          LoaderExceptHandler                         @@ Handle the interrupt

    ldmia       sp,{sp,lr}^                                 @@ restore the user sp and lr
    add         sp,#8                                       @@ update the tack pointer

    pop         {lr}                                        @@ restore the sp and lr for svc mode
    add         sp,#4                                       @@ throw away the stack pointer

    pop         {r0}                                        @@ pop the temporary r0 value - overwritten later

    pop         {r0-r12}                                    @@ restore all the working registers

    rfeia       sp!                                         @@ return from the exception

@@
@@ -- This will set up the processor for handling an FIQ
@@    --------------------------------------------------
LoaderFIQTarget:
    sub         lr,lr,#4                                    @@ adjust the return address
    srsdb       sp!,#0x13                                   @@ save spsr and lr to the svc stack
    cpsid       ifa,#0x13                                   @@ switch to supervisor mode; disable interrupts

    push        {r0-r12}                                    @@ push r0 to r12 onto the stack

    mov         r0,#0x11                                    @@ mode is FIQ, or 0x11
    push        {r0}

    push        {sp}
    push        {lr}                                        @@ push the sp and lr registers

    sub         sp,#8                                       @@ make room for the user-space sp and lr registers
    stmia       sp,{sp,lr}^                                 @@ taking these one at a time, we will save the registers

    mov         r0,sp                                       @@ set the register list for the called function
    bl          LoaderExceptHandler                         @@ Handle the interrupt

    ldmia       sp,{sp,lr}^                                 @@ restore the user sp and lr
    add         sp,#8                                       @@ update the tack pointer

    pop         {lr}                                        @@ restore the sp and lr for svc mode
    add         sp,#4                                       @@ throw away the stack pointer

    pop         {r0}                                        @@ pop the temporary r0 value - overwritten later

    pop         {r0-r12}                                    @@ restore all the working registers

    rfeia       sp!                                         @@ return from the exception

