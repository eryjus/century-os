//===================================================================================================================
//
//  ArchIdtSetGate.cc -- Set the gate in the IDT.
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function sets up the IDT gate in the table itself
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-May-30  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"


//
// -- Construct a single IDT gate
//    ---------------------------
EXTERN_C EXPORT KERNEL
void ArchIdtSetGate(uint8_t num, archsize_t base, archsize_t sel, uint8_t flags)
{
    IdtEntry_t *idtEntries = (IdtEntry_t *)X86_VIRT_IDT;

    if (num > 255) return;

    idtEntries[num].baseLow = (uint16_t)(base & 0xffff);
    idtEntries[num].baseHigh = (uint16_t)((base >> 16) & 0xffff);

    idtEntries[num].sel = sel;
    idtEntries[num].always0 = 0;
    idtEntries[num].flags = flags;
}

