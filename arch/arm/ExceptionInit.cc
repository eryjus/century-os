//===================================================================================================================
//
//  ExceptionInit.cc -- This function will initialize the system to handle interrupts
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-27  Initial   0.2.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "mmu.h"
#include "interrupt.h"


//
// -- This is a local prototype for a low level setup function
//    --------------------------------------------------------
EXTERN_C EXPORT KERNEL
void IdtSetAddr(void);

EXTERN_C EXPORT KERNEL
uint32_t GetMode(void);


//
// -- This is the actual interrupt exception table
//    --------------------------------------------
InterruptVector_t *exceptVect = (InterruptVector_t *)EXCEPT_VECTOR_TABLE;


//
// -- These are the handlers that get control on an interrupt
//    -------------------------------------------------------
EXTERN_C EXPORT KERNEL
void ResetTarget(void) __attribute__((noreturn));

EXTERN_C EXPORT KERNEL
void UndefinedTarget(void) __attribute__((noreturn));

EXTERN_C EXPORT KERNEL
void SuperTarget(void);

EXTERN_C EXPORT KERNEL
void PrefetchTarget(void) __attribute__((noreturn));

EXTERN_C EXPORT KERNEL
void DataAbortTarget(void) __attribute__((noreturn));

EXTERN_C EXPORT KERNEL
void IRQTarget(void);

EXTERN_C EXPORT KERNEL
void FIQTarget(void);


//
// -- Set up the Exception Vector Table
//    ---------------------------------
EXTERN_C EXPORT LOADER
void ExceptionInit(void)
{
    IdtSetAddr();

    kprintf("Mode is: %x\n", GetMode());

    exceptVect->reset = IVEC_JUMP_ASM;
    exceptVect->undefined = IVEC_JUMP_ASM;
    exceptVect->supervisorCall = IVEC_JUMP_ASM;
    exceptVect->prefetchAbort = IVEC_JUMP_ASM;
    exceptVect->dataAbort = IVEC_JUMP_ASM;
    exceptVect->unused = IVEC_JUMP_ASM;
    exceptVect->irqInterrupt = IVEC_JUMP_ASM;
    exceptVect->fiqInterrupt = IVEC_JUMP_ASM;

    exceptVect->resetTarget = (archsize_t)ResetTarget;
    exceptVect->undefinedTarget = (archsize_t)UndefinedTarget;
    exceptVect->supervisorCallTarget = (archsize_t)SuperTarget;
    exceptVect->perfetchAbortTarget = (archsize_t)PrefetchTarget;
    exceptVect->dataAbortTarget = (archsize_t)DataAbortTarget;
    exceptVect->unusedTarget = (archsize_t)NULL;                     // Never used
    exceptVect->irqInterruptTarget = (archsize_t)IRQTarget;
    exceptVect->fiqInterruptTarget = (archsize_t)FIQTarget;
}

