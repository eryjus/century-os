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
    .equ        PROC_STATUS,8
    .equ        PROC_PRIORITY,12
    .equ        PROC_QUANTUM_LEFT,16


@@
@@ -- Some additional constants for use when managing process status
@@    --------------------------------------------------------------
    .equ        PROC_STS_RUNNING,1
    .equ        PROC_STS_READY,2


@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section     .text


@@
@@ -- Execute a process switch
@@    ------------------------
ProcessSwitch:
@@
@@ -- before we get too crazy, do we need to postpone?
@@    ------------------------------------------------
    ldr     r1,=schedulerLocksHeld          @@ get the locks held address
    ldr     r1,[r1]                         @@ and the count
    cmp     r1,#0                           @@ is this zero?
    beq     .cont                           @@ if zero, contunue

    ldr     r1,=processChangePending        @@ get the address of the process change pending
    mov     r0,#1                           @@ load the value to store
    str     r0,[r1]                         @@ set the flag
    mov     pc,lr                           @@ and return

.cont:
    push    {lr}                            @@ save the link register; will pop it later to pc
    push    {lr}                            @@ save this value twice
    push    {ip}                            @@ save the intra-procedure call register (r12)
    push    {r4-r11}                        @@ save the rest of the register (r0-r3 are caller saved)


@@
@@ -- Save the state by writing the current stack pointer to the current structure
@@    ----------------------------------------------------------------------------
    ldr     r1,=currentProcess              @@ get the address of the current process pointer
    ldr     r2,[r1]                         @@ get the address of the structure

    ldr     r4,[r2,#PROC_STATUS]            @@ get the status
    cmp     r4,#PROC_STS_RUNNING            @@ is the status running
    addeq   r4,#1                           @@ this will change the status to PROC_STS_READY
    streq   r4,[r2,#PROC_STATUS]            @@ store the result

    ldr     r3,[r2,#PROC_PRIORITY]          @@ get the process priority
    ldr     r4,[r2,#PROC_QUANTUM_LEFT]      @@ get the quantum left
    add     r4,r4,r3                        @@ add the new quantum allotment to the amount remaining
                                            @@ -- adjusts for "overdrawn" processes
    str     r4,[r2,#PROC_QUANTUM_LEFT]      @@ and store the result

    str     sp,[r2,#PROC_TOP_OF_STACK]      @@ save the current stack pointer
    mrc     p15,0,r3,c2,c0,0                @@ get the address of the current address space


@@
@@ -- now, restore the state of the next task; r0 contains the address of this task
@@    -----------------------------------------------------------------------------
    str     r0,[r1]                         @@ set the new current process
    ldr     sp,[r0,#PROC_TOP_OF_STACK]      @@ restore the top of the stack
    mov     r4,#PROC_STS_RUNNING            @@ load the status into a register
    str     r4,[r0,#PROC_STATUS]            @@ ... and set the status
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

