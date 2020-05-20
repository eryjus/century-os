//===================================================================================================================
//
//  MmuyInit.cc -- Complete the MMU initialization for the arm architecture
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The goal of this function is to make sure that MMU is fully mapped.  Now that we have access to upper memory
//  functions, we will use them to complete the mappings still pening.  These are:
//  * MMIO space
//  * Interrupt Vector Table (map to existing frame but in upper address space)
//  * Frame buffer
//
//  All other addresses should be mapped properly before handing control to the loader.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-13  Initial   0.3.0   ADCL  Initial version
//  2020-Apr-27  Initial  v0.7.0a  ADCL  Rewrite the MMU code
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "serial.h"
#include "mmu.h"
#include "hw-disc.h"
#include "printf.h"
#include "entry.h"
#include "loader.h"


//
// -- Complete the initialization of the Mmu for the loader to function properly
//    --------------------------------------------------------------------------
EXTERN_C EXPORT LOADER
void MmuInit(void)
{
    //
    // -- Next up is the MMIO locations.  These are currently at physical address `0x3f000000` to `0x4003ffff`, if
    //    you include the BCM2836 extensions for multiple cores.  The goal here is to re-map these to be in kernel
    //    space at `0xf8000000` to `0xf903ffff`.  This should be trivial, almost.
    //    --------------------------------------------------------------------------------------------------------
    for (archsize_t mmioVirt = MMIO_VADDR, mmioPhys = MMIO_LOADER_LOC;
            mmioPhys < MMIO_LOADER_END;
            mmioPhys ++, mmioVirt += PAGE_SIZE) {
        MmuMapToFrame(mmioVirt, mmioPhys, PG_KRN | PG_DEVICE | PG_WRT);
    }

    kPrintfEnabled = true;
}

