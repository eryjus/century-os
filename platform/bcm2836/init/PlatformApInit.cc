//===================================================================================================================
//
//  PlatformApInit.cc -- Perform any AP core specific initialization for the platform
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Feb-29  Initial  v0.5.0h  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "platform.h"
#include "interrupt.h"
#include "serial.h"
#include "pic.h"
#include "cpu.h"


//
// -- Complete the platform initialization for the AP
//    -----------------------------------------------
EXTERN_C EXPORT KERNEL
void PlatformApInit(void)
{
    ExceptionInit();
    PicInit(picControl, "PIC");
    PicUnmaskIrq(picControl, BCM2836_CORE_MAILBOX0);
}
