@@===================================================================================================================
@@
@@  syscall.s -- execute a system call
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  This function will execute a system call to the kernel.  To accomplish this, the following ABI will be used:
@@  r0: parameter 1
@@  r1: parameter 2
@@  r2: parameter 3
@@  r3: parameter 4
@@  r4: parameter 5
@@  r5: parameter 6
@@
@@  The systen call instruction contains an immediate value which will be the function to call.
@@
@@  Now, the prototype for this call is the following:
@@      int syscall(int func, int parmCnt, ...);
@@
@@  All register contents (including cpsr) will be preserved across this function call, with the return result
@@  being passed back in r0.
@@
@@  When the system call returns a value < 0, the variable errno will be populated with the -r0 and r0 will be
@@  set to -1.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2020-Apr-18  Initial  v0.7.0a  ADCL  Initial version
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .globl  syscall
    .globl  errno


    .equ    TBL_SIZE,((jumpTableEnd - jumpTable) / 4)


@@
@@ -- this is the error number from the last call
@@    -------------------------------------------
    .section    .data
errno:
    .long   0                           @@ this is the error number global variable



@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
    .section    .text


@@
@@ -- perform the system call
@@    -----------------------
syscall:
    push    {r1-r12,lr}                 @@ save all the important registers
    mrs     r8,cpsr                     @@ Get the Current Program Status Register
    push    {r8}                        @@ save the cpsr

    cmp     r0,#TBL_SIZE                @@ is this a good syscall value?
    bl      .ok

.enosys:
    mov     r0,#38                      @@ we will return ENOSYS
    ldr     r1,=errno                   @@ get the errno location
    str     r0,[r1]                     @@ store the error code
    mov     r0,#0                       @@ set the return value
    sub     r0,#1                       @@ r0 == -1

    b       .out                        @@ exit

.ok:
    mov     r8,r0                       @@ save the syscall function number


@@ -- clear the possibly unused registers
    mov     r4,#0                       @@ clear r4
    mov     r5,#0                       @@ clear r5


@@ -- r0 already contains the system call number; r1 is the number of args
    mov     r7,r1                       @@ r7 now has the numebr of args
    mov     r0,r1                       @@ r1 has arg1
    mov     r1,r2                       @@ r1 has arg2
    mov     r2,r3                       @@ r2 has arg3

    mov     r3,#0                       @@ and clear out r3

    cmp     r7,#1                       @@ is the number of parameters 1?
    movlt   r0,#0                       @@ if 0 parms, clear r0

    cmp     r7,#2                       @@ is the numebr of parameters 2?
    movlt   r1,#0                       @@ if 0-1 parms, clear r1

    cmp     r7,#3                       @@ is the number of parameters 3?
    movlt   r2,#0                       @@ if 0-2 parms, clear r2

    cmp     r7,#4                       @@ is the number of parameters 4?
    movlt   r3,#0                       @@ if 0-3 parms, clear r3

    cmp     r7,#5                       @@ is the numebr of parameters 5?
    ldrge   r4,[sp,#56]                 @@ get arg 5 from the stack

    cmp     r7,#6                       @@ is the numebr of parameters 6?
    ldrge   r5,[sp,#60]                 @@ get arg 6 from the stack


@@ -- perform the supervisor call
    lsl     r8,#2                       @@ multiply by 4
    ldr     r7,=jumpTable               @@ get the base address of the jump table
    add     r7,r8                       @@ r7 now contains the address of the jump to the system call
    bx      r7                          @@ jump to that address

@@ -- ===============================================================================================================
@@    System call Jump targets for each system call

.syscall0:
    svc     0                           @@ perform the system calls
    b       .return

@@    End of the system call targets
@@ -- ===============================================================================================================

.return:
    ldr     r1,=errno                   @@ get the address of error number

    cmp     r0,#0                       @@ did we have an error
    bge     .noError

    neg     r0,r0                       @@ make it an error number
    str     r0,[r1]                     @@ save error number
    mov     r0,#1                       @@ get the return value
    neg     r0,r0                       @@ negate it

    b       .out

.noError:
    mov     r2,#0                       @@ clear the error number
    str     r2,[r1]                     @@ and set the result

.out:
    pop     {r1}                        @@ restore
    msr     cpsr,r1                     @@ restore the program status register
    pop     {r1-r12,pc}


//
// -- This is the jump table to handle each system call
//    -------------------------------------------------
jumpTable:
    .long   .syscall0


jumpTableEnd:
