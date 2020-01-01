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
#include "interrupt.h"

const char *causes[] = {
    "Unknown",                                                                      // 0b00000
    "Alignment Fault (fault on first lookup)",                                      // 0b00001
    "Debug event",                                                                  // 0b00010
    "Access Flag fault (First level)",                                              // 0b00011
    "Fault on instruction cache maintenance",                                       // 0b00100
    "Translation fault (First level)",                                              // 0b00101
    "Access Flag fault (Second level)",                                             // 0b00110
    "Translation fault (Second level)",                                             // 0b00111
    "Synchronous external abort",                                                   // 0b01000
    "Domain fault (First level)",                                                   // 0b01001
    "Unknown",                                                                      // 0b01010
    "Domain fault (Second level)",                                                  // 0b01011
    "Synchronous external abort on translation table walk (First level)",           // 0b01100
    "Permission fault (First level)",                                               // 0b01101
    "Synchronous external abort on translation table walk (Second level)",          // 0b01110
    "Permission fault (Second level)",                                              // 0b01111
    "TLB conflict abort",                                                           // 0b10000
    "Unknown",                                                                      // 0b10001
    "Unknown",                                                                      // 0b10010
    "Unknown",                                                                      // 0b10011
    "Implementation Defined Lockdown",                                              // 0b10100
    "Unknown",                                                                      // 0b10101
    "Asynchronous external abort",                                                  // 0b10110
    "Unknown",                                                                      // 0b10111
    "Asynchronous parity error on memory access",                                   // 0b11000
    "Synchronous parity error on memory access",                                    // 0b11001
    "Implementation Defined coprocessor",                                           // 0b11010
    "Unknown",                                                                      // 0b11011
    "Synchronous parity error on translation table walk (First level)",             // 0b11100
    "Unknown",                                                                      // 0b11101
    "Synchronous parity error on translation table walk (Second level)",            // 0b11110
    "Unknown",                                                                      // 0b11111
};


extern "C" void DataAbortHandler(isrRegs_t *regs)
{
    archsize_t dfsr = GetDFSR();
    int cause = ((dfsr & (1 << 10)) >> 6) | (dfsr & 0xf);

    kprintf("Data Exception:\n");
    kprintf(".. Data Fault Address: %p\n", GetDFAR());
    kprintf(".. Data Fault Status Register: %p\n", dfsr);
    kprintf(".. Fault status %x: %s\n", cause, causes[cause]);
    kprintf(".. Fault occurred because of a %s\n", (dfsr&(1<<11)?"write":"read"));

    IsrDumpState(regs);
    Halt();
}
