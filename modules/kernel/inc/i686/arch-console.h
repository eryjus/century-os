//===================================================================================================================
//
// kernel/inc/i686/arch-console.h -- Constants and definitions for an i686 console screen.
// 
// These types are architecture dependent.  
//
// ------------------------------------------------------------------------------------------------------------------
//
// IMPORTANT PROGRAMMING NOTE:
// The constants in this file are also duplicated in arch-console.inc for the assembly language counterpart to this
// file.  EVERY change in this file MUST be reflected in the other and vice-versa.  If not, undesirable results
// will occur.
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-05-26  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __CONSOLE_H__
#error "Do not include 'arch-console.h' directly; include 'console.h' instead, which will pick up this file."
#endif


//
// -- Several definitions that indicate how to interact with the screen
//    -----------------------------------------------------------------
#define CONSOLE_COLS            80
#define CONSOLE_ROWS            24
#define CONSOLE_ATTR            0x07
#define CONSOLE_VIDEO           ((unsigned char *)0xb8000)
#define CONSOLE_CLEAR           0x0700

#define CONSOLE_STATUS_ATTR     0xe0
