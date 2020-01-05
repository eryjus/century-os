//===================================================================================================================
//
//  PicPick.cc -- Make a decision on which PIC will be used
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-18  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "hw-disc.h"
#include "printf.h"
#include "platform.h"
#include "pic.h"


//
// -- This is the pic we are going to use
//    -----------------------------------
EXPORT KERNEL_DATA
PicDevice_t *picControl = &pic8259;


//
// -- Pick the best PIC we have available to us and set up to use that
//    ----------------------------------------------------------------
EXTERN_C EXPORT LOADER
PicDevice_t *PicPick(void)
{
    if (GetIoapicCount() > 0) picControl = &ioapicDriver;
    else picControl = &pic8259;                         // -- fall back in the 8259 PIC


    //
    // -- Complete the initialization
    //    ---------------------------
    PicInit(picControl, "PIC");
    return picControl;
}

