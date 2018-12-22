//===================================================================================================================
//
//  BusyWait.cc -- This is a basic wait function busily waiting for some time to pass
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  TODO: When QEMU has addressed the missing System Timer, address the hack in BusyWait.  The progrss of the QEMU
//        team resolving this issue can be tracked at https://bugs.launchpad.net/qemu/+bug/1680991
//
// This is 1 MILLIONTH of a second, not 1 THOUSANDTH of a second.  Be aware of the values you pass in here.
//
// FIXME: This function includes a hack to get around QEMU which does not currently implement the System Timer
//        at 0x3f003000.  When the starting ticks come back as 0, we assume that we need to take a less accurate
//        method.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Apr-04  Initial   0.0.0   ADCL  Initial version
//  2018-Nov-13  Initial   0.2.0   ADCL  Leveraged from century
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"


//
// -- Wait a some number of MICRO-seconds
//    -----------------------------------
void BusyWait(uint32_t microSecs)
{
    volatile uint64_t start = SysTimerCount();      // use volatile to prevent the compiler from optimizing away
    uint64_t target = start + microSecs;

    if (start == 0) while (microSecs) --microSecs;
    else while (SysTimerCount() < target) {}
}
