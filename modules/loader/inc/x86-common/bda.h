//===================================================================================================================
//
// loader/inc/x86-common/bda.h -- BIOS Data Area locations
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-04-17  Initial   0.0.0   ADCL  Initial version
//
//===================================================================================================================

#ifndef __BDA_H__
#define __BDA_H__


#include "types.h"


//
// -- The BIOS Data Area identified system capabilities
//    -------------------------------------------------
#define BDA             0x400

#define BDA_COM1        0x00
#define BDA_COM2        0x02
#define BDA_COM3        0x04
#define BDA_COM4        0x06
#define BDA_LPT1        0x08
#define BDA_LPT2        0x0a
#define BDA_LPT3        0x0c
#define BDA_EBDA        0x0e
#define BDA_VIDEO       0x63


//
// -- Read a 16-bit word from the BDA
//    -------------------------------
inline uint16_t BdaGetValue(int offset) { return *((uint16_t *)(BDA + offset)); }


//
// -- Get the location of COM1 from the BDA
//    -------------------------------------
inline uint16_t BdaGetCom1(void) { return BdaGetValue(BDA_COM1); }


//
// -- Get the location of COM2 from the BDA
//    -------------------------------------
inline uint16_t BdaGetCom2(void) { return BdaGetValue(BDA_COM2); }


//
// -- Get the location of COM3 from the BDA
//    -------------------------------------
inline uint16_t BdaGetCom3(void) { return BdaGetValue(BDA_COM3); }


//
// -- Get the location of COM4 from the BDA
//    -------------------------------------
inline uint16_t BdaGetCom4(void) { return BdaGetValue(BDA_COM4); }


//
// -- Get the location of LPT1 from the BDA
//    -------------------------------------
inline uint16_t BdaGetLpt1(void) { return BdaGetValue(BDA_LPT1); }


//
// -- Get the location of LPT2 from the BDA
//    -------------------------------------
inline uint16_t BdaGetLpt2(void) { return BdaGetValue(BDA_LPT2); }


//
// -- Get the location of LPT3 from the BDA
//    -------------------------------------
inline uint16_t BdaGetLpt3(void) { return BdaGetValue(BDA_LPT3); }


//
// -- Get the location of EBDA from the BDA; convert real mode segment to linear address
//    ----------------------------------------------------------------------------------
inline ptrsize_t BdaGetEbda(void) { return BdaGetValue(BDA_EBDA) << 4; }


//
// -- Get the location of VIDEO from the BDA
//    --------------------------------------
inline uint16_t BdaGetVideo(void) { return BdaGetValue(BDA_VIDEO); }


//
// -- Read the BDA and store the results in the hardware data structure
//    -----------------------------------------------------------------
void BdaRead(void);


#endif
