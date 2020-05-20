//===================================================================================================================
//
//  MmuIsMapped.cc -- Determine if the address is mapped in the current address space
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-May-01  Initial   0.4.3   ADCL  Initial version
//  2020-Apr-30  Initial  v0.7.0a  ADCL  Rewrite the MMU code
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "mmu.h"


//
// -- Check for the page and unmap if it is mapped.
//    ---------------------------------------------
EXTERN_C EXPORT KERNEL
bool MmuIsMapped(archsize_t addr)
{
    return (MmuVirtToPhys(addr) != (archsize_t)-1);
}
