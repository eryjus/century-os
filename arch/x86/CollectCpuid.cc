//===================================================================================================================
//
//  CollectCpuid.cc -- Collect the information from Cpuid -- we will look at it later
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-10  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "hw-disc.h"
#include "cpu.h"


//
// -- Collect and store the CPUID info
//    --------------------------------
void __ldrtext CollectCpuid(void)
{
    CPUID(0x00, &localHwDisc->cpuid00eax, &localHwDisc->cpuid00ebx, &localHwDisc->cpuid00ecx,
            &localHwDisc->cpuid00edx);

    int max = localHwDisc->cpuid00eax;

    if (max >= 1) {
        CPUID(0x01, &localHwDisc->cpuid01eax, &localHwDisc->cpuid01ebx, &localHwDisc->cpuid01ecx,
                &localHwDisc->cpuid01edx);
    }

    if (max >= 2) {
        CPUID(0x02, &localHwDisc->cpuid02eax, &localHwDisc->cpuid02ebx, &localHwDisc->cpuid02ecx,
                &localHwDisc->cpuid02edx);
    }

    if (max >= 3) {
        CPUID(0x03, &localHwDisc->cpuid03eax, &localHwDisc->cpuid03ebx, &localHwDisc->cpuid03ecx,
                &localHwDisc->cpuid03edx);
    }

    if (max >= 4) {
        CPUID(0x04, &localHwDisc->cpuid04eax, &localHwDisc->cpuid04ebx, &localHwDisc->cpuid04ecx,
                &localHwDisc->cpuid04edx);
    }

    if (max >= 5) {
        CPUID(0x01, &localHwDisc->cpuid05eax, &localHwDisc->cpuid05ebx, &localHwDisc->cpuid05ecx,
                &localHwDisc->cpuid05edx);
    }

    if (max >= 6) {
        CPUID(0x06, &localHwDisc->cpuid06eax, &localHwDisc->cpuid06ebx, &localHwDisc->cpuid06ecx,
                &localHwDisc->cpuid06edx);
    }

    if (max >= 7) {
        CPUID(0x07, &localHwDisc->cpuid07eax, &localHwDisc->cpuid07ebx, &localHwDisc->cpuid07ecx,
                &localHwDisc->cpuid07edx);
    }

    if (max >= 9) {
        CPUID(0x09, &localHwDisc->cpuid09eax, &localHwDisc->cpuid09ebx, &localHwDisc->cpuid09ecx,
                &localHwDisc->cpuid09edx);
    }

    if (max >= 0xa) {
        CPUID(0x0a, &localHwDisc->cpuid0aeax, &localHwDisc->cpuid0aebx, &localHwDisc->cpuid0aecx,
                &localHwDisc->cpuid0aedx);
    }

    if (max >= 0xb) {
        CPUID(0x0b, &localHwDisc->cpuid0beax, &localHwDisc->cpuid0bebx, &localHwDisc->cpuid0becx,
                &localHwDisc->cpuid0bedx);
    }
}
