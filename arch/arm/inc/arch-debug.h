//===================================================================================================================
//
//  arch-debug.h -- This file holds the debugging constructs for the arm arch
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-May-25  Initial  v0.7.0a  ADCL  Initial version
//
//===================================================================================================================


#ifndef __CPU_H__
#   error "Do not include 'arch-cpu.h' directly; include 'cpu.h' instead, which will pick up this file."
#endif


#include "types.h"


//
// -- Dump the Current Program Status Register (CPSR)
//    -----------------------------------------------
EXTERN_C EXPORT KERNEL
void ArchDumpCpsr(void);


