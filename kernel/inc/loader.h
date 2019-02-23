//===================================================================================================================
//
//  loader.h -- These are the prototypes for function renames for the loader to call at physical locations
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-10  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __LOADER_H__
#define __LOADER_H__


//
// -- these 2 variable are provided by the linker
//    -------------------------------------------
extern archsize_t phys_loc;
extern archsize_t kern_loc;
extern uint8_t *_loaderEnd;
extern uint8_t *_loaderStart;
extern frame_t pmmEarlyFrame;
extern frame_t pmmEarlyFrameSave;
extern archsize_t mmuLvl1Table;
extern frame_t intTableAddr;


//
// -- This is a call to SerialPutChar
//    -------------------------------
typedef void (*SerialPutChar_t)(char byte);
extern SerialPutChar_t LoaderSerialPutChar;


//
// -- This is a call to kMemSetB
//    --------------------------
typedef void (*kMemSetB_t)(void *buf, uint8_t wrd, size_t cnt);
extern kMemSetB_t lMemSetB;


//
// -- Put a String to the Serial Port
//    -------------------------------
#define LoaderSerialPutS(x)                     \
    do {                                        \
        char *s = (char *)PHYS_OF(x);           \
        while (*s) LoaderSerialPutChar(*s ++);  \
    } while (0)


//
// -- Put a hex value to the serial port
//    ----------------------------------
#define LoaderSerialPutHex(val)                             \
    do {                                                    \
        LoaderSerialPutChar('0');                           \
        LoaderSerialPutChar('x');                           \
        for (int i = 28; i >= 0; i -= 4) {                  \
            char c = (((val) >> i) & 0x0f);                 \
                                                            \
            if (c > 9) LoaderSerialPutChar(c - 10 + 'a');   \
            else LoaderSerialPutChar(c + '0');              \
        }                                                   \
    } while (0)


//
// -- This is the function that will allocate a frame during early initialization (< 4MB)
//    -----------------------------------------------------------------------------------
__CFUNC frame_t NextEarlyFrame(void);

//
// -- function to initialize the loader functions
//    -------------------------------------------
__CFUNC void LoaderFunctionInit(void);


//
// -- Early Initialization function to handle this initialization by architecture
//    ---------------------------------------------------------------------------
__CFUNC void EarlyInit(void);


//
// -- Perform the MMU Early Initialization so that we can use the whole kernel source no matter where it is located
//    -------------------------------------------------------------------------------------------------------------
__CFUNC void MmuEarlyInit(void);



#endif
