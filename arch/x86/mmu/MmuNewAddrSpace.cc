//===================================================================================================================
//
//  MmuNewAddrSpace.cc -- Create a new address space for a new user process
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function will create and install a new address space intended to be used for a user process.  This
//  function will map all the kernel space into the new address space and then replace the current address space
//  with this one.  The result will be that this is that the user program can be mapped into this address space.
//
//  For this to work, interrupts must be disabled during this process.  If not, then a process change will take
//  place and replace the address space we are woking on with the original one and we will end up updating the
//  kernel address space.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-10  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"
#include "pmm.h"
#include "pic.h"
#include "mmu.h"
