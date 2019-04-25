//===================================================================================================================
//
//  PlatformInit.cc -- Handle the initialization for the pc platform
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
#include "printf.h"
#include "platform.h"
#include "pic.h"


//
// -- Complete the platform initialization
//    ------------------------------------
void PlatformInit(void)
{
    UpdateKprintfPort();
    PicPick();
}
