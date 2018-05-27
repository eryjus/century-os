//===================================================================================================================
// kernel/src/ConsolePutS.cc -- Put a string to the console screen and append a newline
// 
// This is an implementation of puts(), which is deliberately named differently to prevent confusion over whether
// the standard library routines are included.  They are not.
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-05-27  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "console.h"


//
// -- Loop through the string and put each character to the screen, then follow it up with a newline.
//    -----------------------------------------------------------------------------------------------
void ConsolePutS(const char *str) {
    while (*str) ConsolePutChar(*str ++);
    ConsolePutChar('\n');
}
