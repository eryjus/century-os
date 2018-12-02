//===================================================================================================================
//
//  arch-tss.h -- This is a dummy include file since the rpi2b has nothing like a tss
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-21  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


//
// -- Dummy function to mimic the TSS initialization
//    ----------------------------------------------
inline void CpuTssInit(void) { }


//
// -- This is the size of the short exception stacks
//    ----------------------------------------------
#define EXCEPTION_STACK_SIZE  512
