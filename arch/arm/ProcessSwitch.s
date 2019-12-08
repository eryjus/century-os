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
@@ -- make sure that if the required symbols are defined
@@    --------------------------------------------------
.ifndef ENABLE_BRANCH_PREDICTOR
    .equ        ENABLE_BRANCH_PREDICTOR,0
.endif


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
@@ -- Some local equates to access the scheduler elements
@@    ---------------------------------------------------
    .equ        SCH_CURRENT_PROCESS,0
    .equ        SCH_CHG_PENDING,0x10
    .equ        SCH_LOCK_COUNT,0x18
    .equ        SCH_POSTPONE_COUNT,0x1c


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
    ldr     r1,=(scheduler+SCH_POSTPONE_COUNT)  @@ get the locks held address (&scheduler.postponeCount)
    ldr     r1,[r1]                         @@ and the count (scheduler.postponeCount)
    cmp     r1,#0                           @@ is this zero?
    beq     .cont                           @@ if zero, contunue

    ldr     r1,=(scheduler+SCH_CHG_PENDING) @@ get addr of the proc chg pend (&scheduler.processChangePending)
    mov     r0,#1                           @@ load the value to store (value 1)
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
    ldr     r1,=(scheduler+SCH_CURRENT_PROCESS) @@ get &scheduler.currentProcess
    ldr     r2,[r1]                         @@ get scheduler.currentProcess (or the pointer to Process_t)

    ldr     r4,[r2,#PROC_STATUS]            @@ get the status scheduler.currentProcess->status
    cmp     r4,#PROC_STS_RUNNING            @@ is the status running
    bne     .saveStack                      @@ if not, skip the next part

    push    {r0,r1,r2,r3}                   @@ save the clobber registers
    ldr     r0,[r1]                         @@ get the current process (scheduler.currentProcess)
    bl      ProcessDoReady                  @@ go make the current process ready
    pop     {r0,r1,r2,r3}                   @@ restore the vlobber registers

.saveStack:
    push    {r0,r1,r2,r3}                   @@ save the clobber registers
    bl      ProcessUpdateTimeUsed           @@ update the time accounting
    pop     {r0,r1,r2,r3}                   @@ restore the vlobber registers


    str     sp,[r2,#PROC_TOP_OF_STACK]      @@ save the current stack pointer
    mrc     p15,0,r3,c2,c0,0                @@ get the address of the current address space


@@
@@ -- now, restore the state of the next task; r0 contains the address of this task
@@    -----------------------------------------------------------------------------
    str     r0,[r1]                         @@ set the new current process (scheduler.currentProcess = r0)
    ldr     sp,[r0,#PROC_TOP_OF_STACK]      @@ restore the top of the stack (scheduler.currentProcess->topOfStack)
    mov     r4,#PROC_STS_RUNNING            @@ load the status into a register (value 1)
    str     r4,[r0,#PROC_STATUS]            @@ ... and set the status (scheduler.currentProcess->status = 1)
    ldr     r2,[r0,#PROC_VIRT_ADDR_SPACE]   @@ get addr spc of new tsk (r2 = scheduler.currentProcess->virtAddrSpace)

    ldr     r1,[r0,#PROC_PRIORITY]          @@ get the process priority (r1 = scheduler.currentProcess->priority)
    ldr     r4,[r0,#PROC_QUANTUM_LEFT]      @@ get the quantum left (r4 = scheduler.currentProcess->quantumLeft)
    add     r4,r4,r1                        @@ add the new quantum allotment to the amount remaining
                                            @@ -- adjusts for "overdrawn" processes
    str     r4,[r0,#PROC_QUANTUM_LEFT]      @@ and store the result (scheduler.currentProcess->quantumLeft = r4)

    cmp     r2,r3                           @@ are they the same virtual address space?
    mcrne   p15,0,r2,c2,c0,0                @@ replace the top level mmu tables

.if ENABLE_BRANCH_PREDICTOR
    mcrne   p15,0,r0,c7,c5,6                @@ invalidate the branch predictor (required maintenance when enabled)
.endif

    isb                                     @@ flush the instruction fetch buffer and start over
    dsb                                     @@ reset all memory fetches


@@
@@ -- restore the registers in preparation to return
@@    ----------------------------------------------
    pop     {r4-r11}                        @@ restore the rest of the register (r0-r3 are caller saved)
    pop     {ip}                            @@ restore the intra-process register
    pop     {lr}
    pop     {pc}                            @@ return from this function

