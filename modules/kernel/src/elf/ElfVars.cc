//===================================================================================================================
//
//  ElfVars.cc -- Variables used to load an ELF image
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-20  Initial  v0.7.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "elf.h"


//
// -- This is the spinlock required to access ELF_TEMP_HEADER
//    -------------------------------------------------------
EXPORT KERNEL_BSS
Spinlock_t elfHdrLock = {0};

