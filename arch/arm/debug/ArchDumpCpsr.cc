//===================================================================================================================
//
//  ArchDumpCpsr.cc -- Dump the state of Current Program Status Register
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-May-25  Initial   0.7.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "pic.h"
#include "printf.h"
#include "cpu.h"


EXPORT KERNEL_DATA
const char *modes[] = {
    "Unknown",          // 0b00000
    "Unknown",          // 0b00001
    "Unknown",          // 0b00010
    "Unknown",          // 0b00011
    "Unknown",          // 0b00100
    "Unknown",          // 0b00101
    "Unknown",          // 0b00110
    "Unknown",          // 0b00111
    "Unknown",          // 0b01000
    "Unknown",          // 0b01001
    "Unknown",          // 0b01010
    "Unknown",          // 0b01011
    "Unknown",          // 0b01100
    "Unknown",          // 0b01101
    "Unknown",          // 0b01110
    "Unknown",          // 0b01111
    "usr",              // 0b10000
    "fiq",              // 0b10001
    "irq",              // 0b10010
    "svc",              // 0b10011
    "Unknown",          // 0b10100
    "Unknown",          // 0b10101
    "mon",              // 0b10110
    "abt",              // 0b10111
    "Unknown",          // 0b11000
    "Unknown",          // 0b11001
    "hyp",              // 0b11010
    "und",              // 0b11011
    "Unknown",          // 0b11100
    "Unknown",          // 0b11101
    "Unknown",          // 0b11110
    "sys",              // 0b11111
};


//
// -- Dump the Current Program Status Register (CPSR)
//    -----------------------------------------------
EXTERN_C EXPORT KERNEL
void ArchDumpCpsr(void)
{
    archsize_t cpsr = ReadCpsr();

    kprintf("------------------------------------------------------------\n");
    kprintf("CPSR: %p\n", cpsr);
    kprintf("   Condition flags: %c %c %c %c\n", cpsr&(1<<31)?'N':'n', cpsr&(1<<30)?'Z':'z', cpsr&(1<<29)?'C':'c',
            cpsr&(1<<28)?'V':'v');
    kprintf("   Cumulative Saturation: %d\n", (cpsr>>27)&1);
    kprintf("   Thumb If-Then execution state: %d\n", ((cpsr>>25)&3) | ((cpsr>>8) & 0xfc));
    kprintf("   Jazelle: %d\n", (cpsr>>24)&1);
    kprintf("   GE bits: %c %c %c %c\n", cpsr&(1<<19)?'1':'0', cpsr&(1<<18)?'1':'0', cpsr&(1<<17)?'1':'0',
            cpsr&(1<<16)?'1':'0');
    kprintf("   Endianness state: %s\n", cpsr&(1<<9)?"Big Endian":"Little Endian");
    kprintf("   Asynchronous Aborts: %s\n", cpsr&(1<<8)?"masked":"allowed");
    kprintf("   IRQ Exceptions: %s\n", cpsr&(1<<7)?"masked":"allowed");
    kprintf("   FIQ Exceptions: %s\n", cpsr&(1<<6)?"masked":"allowed");
    kprintf("   Thumb Execution State: %d\n", (cpsr>>5)&1);
    kprintf("   Mode: %s\n", modes[cpsr&0x1f]);
}




