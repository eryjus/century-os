//===================================================================================================================
//
//  ArchEarlyCpuInit.cc -- Initialize the CPU structures for the x86 arch
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This will have several responsibilities;
//  1) Create the permanent GDT Structure in low memory
//  2) Enable the permanent GDT for CPU 0
//  3) Create the permanent IDT Structure in low memory (copy from existing)
//  4) Enable the permanent IDT for CPU 0
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Feb-01  Initial  v0.5.0f  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"


//
// -- Complete the CPU initialization
//    -------------------------------
EXTERN_C EXPORT LOADER
void ArchEarlyCpuInit(void)
{
    kprintf("Completing CPU initialization\n");
    ArchGdtSetup();
    ArchIdtSetup();
}



