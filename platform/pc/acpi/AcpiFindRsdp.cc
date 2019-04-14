//===================================================================================================================
//
//  AcpiFindRdsp.cc -- Find the Root System Description Pointer (RSDP), which is really a table.
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The RDSP can be in one of 2 places:
//  1) The first 1K of the EBDA
//  2) Between 0xe0000 and 0xfffff
//
//  In either case, the value must be on a 16-byte boundary.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-05  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "hw-disc.h"
#include "printf.h"
#include "hardware.h"


//
// -- check a pointer to see if it really qualifies as a RSDP
//    -------------------------------------------------------
static bool IsRsdp(RSDP_t *rsdp)
{
    if (!rsdp) return false;

    uint32_t cs = 0;
    uint8_t *data = (uint8_t *)rsdp;

    for (uint32_t i = 0; i < 20; i ++) {
        cs += data[i];
    }

    return (cs & 0xff) == 0;
}


//
// -- Locate the Root System Description Table
//    ----------------------------------------
RSDP_t *__ldrtext AcpiFindRsdp(void)
{
    archsize_t wrk = GetEbda() & ~0x000f;
    archsize_t end = wrk + 1024;
    RSDP_t *rsdp;

    if (wrk != 0) {
        while (wrk < end) {
            rsdp = (RSDP_t *)wrk;

            if (rsdp->lSignature == RSDP_SIG && IsRsdp(rsdp)) {
                kprintf("RSDP found at address %p\n", wrk);
                SetRsdp(wrk);
                return rsdp;
            }

            wrk += 16;
        }
    }

    wrk = 0xe0000;
    end = 0xfffff;

    while (wrk < end) {
        rsdp = (RSDP_t *)wrk;

        if (rsdp->lSignature == RSDP_SIG && IsRsdp(rsdp)) {
            kprintf("RSDP found at address %p\n", wrk);
            kprintf(".. Version %x\n", rsdp->revision);
            kprintf(".. Rsdt Address %p\n", rsdp->rsdtAddress);
            kprintf(".. Xsdt Address %p\n", rsdp->xsdtAddress);
            SetRsdp(wrk);
            return rsdp;
        }

        wrk += 16;
    }

    return NULL;
}


