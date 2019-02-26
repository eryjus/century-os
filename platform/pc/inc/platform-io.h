//===================================================================================================================
//
//  platform-io.h -- These are additional I/O functions that are use for x86
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-23  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __HARDWARE_H__
#   error "Use #include \"hardware.h\" and it will pick up this file; do not #include this file directly."
#endif


//
// -- Get a byte from an I/O Port
//    ---------------------------
inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}


//
// -- Output a byte to an I/O Port
//    ----------------------------
inline void outb(uint16_t port, uint8_t val) { asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) ); }



