//===================================================================================================================
//
//  arch-interrupt.cc -- These are functions related to interrupts for the rpi2b architecture
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  These are function prototypes for interrupts management
//
//  The interrupt vector deserves some discussion.  This is documented in the ARM Cortex-A Programmers Reference
//  section 11.1.1.  This vector table is located at address 0xffff0000.  Each entry in the table is a 4-byte
//  instruction.  A simple branch to a relative offset requires the offset to be withing 24-bits.  Since the
//  kernel is located it 0xc0000000, this is not the case.
//
//  The alternative is to use a long (32-bit) jump where the target is relative to the pc register.  For this
//  approach to work, the jump target address must be stored close (12 bits offset).  By putting these jump targets
//  immediately above the Interrupt Vector table, I meet that requirement.  Now, I can jump to an address that is
//  stored relative to the pc register in a single instruction, and that target is a 32-bit address location.
//
//  To accomplish this, I have an instruction that I will place in the Interrupt Vector portion, IVEC_JUMP_ASM.
//  This instruction is a pre-assembled instrution `ldr pc,[pc,#0x18]`.  Now, the astute reader will note that the
//  actual offset in the table is 0x20 bytes but I am only offsetting the instruction by 0x18 bytes.  The ARM ARM
//  section A5.2.2 notes that when the base register for the addressing mode calculation, "the value is the
//  address of the instruction plus eight."  Thus, the 8-byte difference.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __INTERRUPT_H__
#   error "Do not include 'arch-interrupt.h' directly.  Include 'interrupt.h' and this file will be included"
#endif


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "hw-kernel.h"


//
// -- This is the code that will be placed in each of the vector locations (not the targets)
//    Note that this is written in little-endian order for: 0xe59ff018
//    --------------------------------------------------------------------------------------
#define IVEC_JUMP_ASM       (0xe59ff018)


//
// -- This structure is the interrupt vector table for the rpi2b
//    ----------------------------------------------------------
typedef struct InterruptVector_t {
    uint32_t reset;
    uint32_t undefined;
    uint32_t supervisorCall;
    uint32_t prefetchAbort;
    uint32_t dataAbort;
    uint32_t unused;
    uint32_t irqInterrupt;
    uint32_t fiqInterrupt;
    uint32_t resetTarget;
    uint32_t undefinedTarget;
    uint32_t supervisorCallTarget;
    uint32_t perfetchAbortTarget;
    uint32_t dataAbortTarget;
    uint32_t unusedTarget;
    uint32_t irqInterruptTarget;
    uint32_t fiqInterruptTarget;
} __attribute__((packed)) InterruptVector_t;


//
// -- Build the IDT and populate its gates; initialize the handlers to NULL
//    ---------------------------------------------------------------------
void IdtBuild(void);


//
// -- A Local prototype to prevent the compiler from name mangling
//    ------------------------------------------------------------
extern "C" void IsrHandler(isrRegs_t *regs);



