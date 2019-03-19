@@===================================================================================================================
@@
@@  ProcessSwitch.s -- Execute a task switch at the lowest switch
@@
@@  This function will perform a voluntary task switch, as opposed to a preemptive switch.  Therefore, there will
@@  never need to have an EOI sent to the IC.
@@
@@  There are a couple of call-outs here for the changes.
@@  1) I was saving everything and it was overkill.  There is no need for this complexity
@@  2) r0-r3 are considered to be argument/return registers for the arm abi.  Any interesting value would have been
@@     saved by the caller so no need to worry about them.
@@  3) The virtual address space register does not need to be saved; it is static once identified and therefore
@@     it is already saved in the structure.
@@
@@  Function Prototype:
@@  extern "C" void ProcessSwitch(Process_t *nextProcess);
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2018-Nov-21  Initial   0.2.0   ADCL  Initial version
@@  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
@@  2019-Mar-16  Initial   0.3.2   ADCL  Clean up the scheduler
@@
@@===================================================================================================================


@@
@@ -- Now, expose our function to everyone
@@    ------------------------------------
    .global     ProcessSwitch


@@
@@ -- Some local equates for use with access structure elements
@@    ---------------------------------------------------------
    .equ        PROC_TOP_OF_STACK,0
    .equ        PROC_VIRT_ADDR_SPACE,4

@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section     .text


@@
@@ -- Execute a process switch
@@    ------------------------
ProcessSwitch:
    push    {lr}                            @@ save the link register; will pop it later to pc
    push    {lr}                            @@ save this value twice
    push    {ip}                            @@ save the intra-procedure call register (r12)
    push    {r4-r11}                        @@ save the rest of the register (r0-r3 are caller saved)


@@
@@ -- Save the state by writing the current stack pointer to the current structure
@@    ----------------------------------------------------------------------------
    ldr     r1,=currentProcess              @@ get the address of the current process pointer
    ldr     r2,[r1]                         @@ get the address of the structure
    str     sp,[r2,#PROC_TOP_OF_STACK]      @@ save the current stack pointer
    mrc     p15,0,r3,c2,c0,0                @@ get the address of the current address space


@@
@@ -- now, restore the state of the next task; r0 contains the address of this task
@@    -----------------------------------------------------------------------------
    str     r0,[r1]                         @@ set the new current process
    ldr     sp,[r0,#PROC_TOP_OF_STACK]      @@ restore the top of the stack
    ldr     r2,[r0,#PROC_VIRT_ADDR_SPACE]   @@ get the address space of the new task

    cmp     r2,r3                           @@ are they the same virtual address space?
    mcrne   p15,0,r2,c2,c0,0                @@ replace the top level mmu tables
    isb                                     @@ flush the instruction fetch buffer and start over
    dsb                                     @@ reset all memory fetches


@@
@@ -- restore the registers in preparation to return
@@    ----------------------------------------------
    pop     {r4-r11}                        @@ restore the rest of the register (r0-r3 are caller saved)
    pop     {ip}                            @@ restore the intra-process register
    pop     {lr}
    pop     {pc}                            @@ return from this function

