//===================================================================================================================
//
//  DataAbortHandler.cc -- Handle a data abort
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Dec-01  Initial   0.2.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "mmu.h"
#include "interrupt.h"


EXPORT KERNEL_DATA
const char *causes[] = {
    "Unknown",                                                                          // 0b000000
    "Unknown",                                                                          // 0b000001
    "Unknown",                                                                          // 0b000010
    "Unknown",                                                                          // 0b000011
    "Unknown",                                                                          // 0b000100
    "Translation Fault -- Level 1",                                                     // 0b000101
    "Translation Fault -- Level 2",                                                     // 0b000110
    "Translation Fault -- Level 3",                                                     // 0b000111
    "Unknown",                                                                          // 0b001000
    "Access Flag Fault -- Level 1",                                                     // 0b001001
    "Access Flag Fault -- Level 2",                                                     // 0b001010
    "Access Flag Fault -- Level 3",                                                     // 0b001011
    "Unknown",                                                                          // 0b001100
    "Permission Flag Fault -- Level 1",                                                 // 0b001101
    "Permission Flag Fault -- Level 2",                                                 // 0b001110
    "Permission Flag Fault -- Level 3",                                                 // 0b001111
    "Synchronous External Abort",                                                       // 0b010000
    "Asynchronous External Abort",                                                      // 0b010001
    "Unknown",                                                                          // 0b010011
    "Unknown",                                                                          // 0b010100
    "Synchronous External Abort on Translation Table Walk -- Level 1",                  // 0b010101
    "Synchronous External Abort on Translation Table Walk -- Level 2",                  // 0b010110
    "Synchronous External Abort on Translation Table Walk -- Level 3",                  // 0b010111
    "Unknown",                                                                          // 0b011000
    "Asynchronous Parity Error on Memory Access",                                       // 0b011001
    "Unknown",                                                                          // 0b011010
    "Unknown",                                                                          // 0b011011
    "Unknown",                                                                          // 0b011100
    "Synchronous Parity Error on Memory Access on Translation Table Walk -- Level 1",   // 0b011101
    "Synchronous Parity Error on Memory Access on Translation Table Walk -- Level 2",   // 0b011110
    "Synchronous Parity Error on Memory Access on Translation Table Walk -- Level 3",   // 0b011111
    "Unknown",                                                                          // 0b100000
    "Alignment Fault",                                                                  // 0b100001
    "Debug Event",                                                                      // 0b100010
    "Unknown",                                                                          // 0b100011
    "Unknown",                                                                          // 0b100100
    "Unknown",                                                                          // 0b100101
    "Unknown",                                                                          // 0b100110
    "Unknown",                                                                          // 0b100111
    "Unknown",                                                                          // 0b101000
    "Unknown",                                                                          // 0b101001
    "Unknown",                                                                          // 0b101010
    "Unknown",                                                                          // 0b101011
    "Unknown",                                                                          // 0b101100
    "Unknown",                                                                          // 0b101101
    "Unknown",                                                                          // 0b101110
    "Unknown",                                                                          // 0b101111
    "TLB Conflict Abort",                                                               // 0b110000
    "Unknown",                                                                          // 0b110001
    "Unknown",                                                                          // 0b110010
    "Unknown",                                                                          // 0b110011
    "Implementation Defined",                                                           // 0b110100
    "Unknown",                                                                          // 0b110101
    "Unknown",                                                                          // 0b110110
    "Unknown",                                                                          // 0b110111
    "Unknown",                                                                          // 0b111000
    "Unknown",                                                                          // 0b111001
    "Implementation Defined",                                                           // 0b111010
    "Unknown",                                                                          // 0b111011
    "Unknown",                                                                          // 0b111100
    "Domain Fault -- Level 1",                                                          // 0b111101
    "Domain Fault -- Level 2",                                                          // 0b111110
    "Domain Fault -- Level 3",                                                          // 0b111111
};



//
// -- Handle a data exception
//    -----------------------
EXTERN_C EXPORT KERNEL
void DataAbortHandler(isrRegs_t *regs)
{
    archsize_t dfsr = ReadDFSR();
    int cause = (dfsr & 0x3f);

    kprintf("Data Exception:\n");
    kprintf(".. Data Fault Address: %p\n", ReadDFAR());
    if (dfsr & (1<<13)) kprintf(".. Cache Maintenance Fault\n");
    if (dfsr & (1<<12)) kprintf(".. External Abort\n");
    kprintf(".. Fault occurred because of a %s\n", (dfsr&(1<<11)?"write":"read"));
    kprintf(".. LPAE is %s\n", dfsr & (1<<9) ? "enabled" : "disabled");
    kprintf(".. Data Fault Status Register: %p\n", dfsr);
    kprintf(".. Fault status %x: %s\n", cause, causes[cause]);

    MmuDumpTables(ReadDFAR());

    IsrDumpState(regs);
}
