//===================================================================================================================
//
// inc/console.h -- Standard Console manipulation functions
//
// These are the common functions that need to be implemented to manage the console screen across all architectures.
// 
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-05-26  Initial   0.1.0   ADCL  Initial version as I move functions from century32
//
//===================================================================================================================


#ifndef __CONSOLE_H__
#define __CONSOLE_H__


#include "types.h"
#include "arch-console.h"


//
// -- Write a string to the console and follow it up with a newline
//    -------------------------------------------------------------
void ConsolePutS(const char *str);


//
// -- Put a single character on the screen and advance the cursor position
//    --------------------------------------------------------------------
void ConsolePutChar(const int c);


//
// -- Paint a character on the screen
//    -------------------------------
void ConsolePaintChar(const uint16_t row, const uint16_t col, const uint8_t attr, const uint8_t byte);


//
// -- Update a the status bar with the complete new contents
//    ------------------------------------------------------
void ConsoleUpdateStatus(void);


//
// -- Clear the console
//    -----------------
void ConsoleClear (void);


#endif
