@@===================================================================================================================
@@
@@  entry.s -- This is the entry point for the Raspberry Pi 2B
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  A couple of notes here... 1) multiboot2 is not currently supported by rpi-boot ao I am leaving these structures
@@  out of this file; 2) video setup is not supported by rpi-boot so I am leaving the definitions out of these
@@  structures.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@  2018-Dec-14  Initial   0.2.0   ADCL  Initial version -- copied from the loader and modified to fit my need
@@
@@===================================================================================================================


.globl          Dummy
.globl          Halt
.globl          _start
.globl          IVT
.globl          IvtFunc


.section    .entry

@@
@@ -- This is the main entry point for the OS.  Control is handed to all 4 cores to this point.  This means that
@@    all 4 cores will be executing the same code at the same time, battling over the which core gets to do what.
@@    Ultimately, we need to turn off all the cores except core 0 and let that core take care of the initialization.
@@    In our case so far, this is the only core that will run.
@@
@@    This code is lifted from the ARM Cortex A-Series Version 4.0 programmer's Guide (Example 13-5).
@@    ---------------------------------------------------------------------------------------------------------------
_start:
    mrs     r0,cpsr                     @@ get the current program status register
    and     r0,#0x1f                    @@ and mask out the mode bits
    cmp     r0,#0x1a                    @@ are we in hyp mode?
    beq     hyp                         @@ if we are in hyp mode, go to that section
    cpsid   iaf,#0x13                   @@ if not switch to svc mode, ensure we have a stack for the kernel; no ints
    b       cont                        @@ and then jump to set up the stack

@@ -- from here we are in hyp mode so we need to exception return to the svc mode
hyp:
    mrs     r0,cpsr                     @@ get the cpsr again
    and     r0,#~0x1f                   @@ clear the mode bits
    orr     r0,#0x013                   @@ set the mode for svc
    orr     r0,#1<<6|1<<7|1<<8          @@ disable interrupts as well
    msr     spsr_cxsf,r0                @@ and save that in the spsr

    ldr     r0,=cont                    @@ get the address where we continue
    msr     elr_hyp,r0                  @@ store that in the elr register

    eret                                @@ this is an exception return

@@ -- everyone continues from here
cont:
    mov     sp,#0x8000                  @@ set up a stack

    mrc     p15,0,r3,c0,c0,5            @@ Read Multiprocessor Affinity Register
    and     r3,r3,#0x3                  @@ Extract CPU ID bits
    cmp     r3,#0
    beq     initialize                  @@ if we’re on CPU0 goto the start

@@ -- all other cores will drop in to this loop - a low power mode infinite loop
wait_loop:
    wfi                                 @@ wait for interrupt
    b       wait_loop                   @@ go back and do it again

@@
@@ -- This is the code for CPU @@0 to execute
@@    --------------------------------------
initialize:
    ldr     r0,=IVT                     @@ The location of the vector base address register (1MB)
    mcr     p15,0,r0,c12,c0,0           @@ Write this location to the Vector Base Address Register

    bl      TimerMain                   @@ go to the main C program


.section    .text


Halt:
    wfi
    b       Halt                        @@ go back and loop through more

Dummy:
    mov     pc,lr                       @@ quick return

IvtFunc:
    ldr     r0,=IVT                     @@ The location of the vector base address register (1MB)
    mov     pc,r0                       @@ and jump to that location



.align      5                           @@ align to 2^5 or 32 bytes
IVT:
    nop                                 @@ reset vector -- skip to the next instruction
    nop                                 @@ undefined vector -- skip to the next instruction
    nop                                 @@ supervisor call -- skip to the next instruction
    nop                                 @@ prefetch abort -- skip to the next instruction
    nop                                 @@ data abort -- skip to the next instruction
    nop                                 @@ usused vector -- skip to the next instruction
    nop                                 @@ Irq -- skip to the next instruction
    ldr     r0,=IRQTarget               @@ FIQ -- Get the address of the target to jump
    mov     pc,r0                       @@ and jump to that address
