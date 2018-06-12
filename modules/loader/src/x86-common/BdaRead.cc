//===================================================================================================================
//
//  loader/src/x86-common/BdaRead.cc -- Read the BDA and store its contents for later
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-06-09  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial.h"
#include "hw-disc.h"
#include "bda.h"


//
// -- Get the critical information from the BDA before it is overwritten
//    ------------------------------------------------------------------
void BdaRead(void)
{
    SerialPutS("Getting BDA information");
    SetCom1(BdaGetCom1());
    SetCom2(BdaGetCom2());
    SetCom3(BdaGetCom3());
    SetCom4(BdaGetCom4());
    SetLpt1(BdaGetLpt1());
    SetLpt2(BdaGetLpt2());
    SetLpt3(BdaGetLpt3());
    SetVideo(BdaGetVideo());
    SetEbda(BdaGetEbda());
}
