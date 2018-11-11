//===================================================================================================================
//
//  IdtSetGate.cc -- Set the gate in the IDT.
//
//        Copyright (c)  2017-2018 -- Adam Clark
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
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"


//
// -- A pointer to the actual IDT
//    ---------------------------
extern IdtEntry *idtEntries;


//
// -- Construct a single IDT gate
//    ---------------------------
void IdtSetGate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    if (num > 255) return;

	idtEntries[num].baseLow = (uint16_t)(base & 0xffff);
	idtEntries[num].baseHigh = (uint16_t)((base >> 16) & 0xffff);

	idtEntries[num].sel = sel;
	idtEntries[num].always0 = 0;
	idtEntries[num].flags = flags | 0x60;	// -- extra bits used for ring 3
}

