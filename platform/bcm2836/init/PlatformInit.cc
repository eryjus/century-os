//===================================================================================================================
//
//  PlatformInit.cc -- Handle the initialization for the rpi2b platform
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Complete the platform initialization.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-18  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "pic.h"
#include "interrupt.h"
#include "platform.h"


//
// -- Complete the platform initialization
//    ------------------------------------
EXTERN_C EXPORT LOADER
void PlatformInit(void)
{
    PicUnmaskIrq(picControl, BCM2836_CORE_MAILBOX0);
    IsrRegister(0x64, PicMailbox0Handler);
}

