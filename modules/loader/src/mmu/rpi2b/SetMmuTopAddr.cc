//===================================================================================================================
//
//  SetMmuTopAddr.cc -- Set the opt address for the MMU and enable paging
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-24  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial.h"
#include "mmu-loader.h"


void SetMmuTopAddr(void)
{
    SerialPutS("Preparing to enable paging\n");

    // -- The ttl1 variable is set already in `MmuInit()`
    if (MmuIsEnabled()) {
        SerialPutS("!!! Found that the MMU is already enabled!!!\n");
    }

    MmuEnablePaging(ttl1);

    SerialPutS("Paging is enabled\n");
}