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


#include "arch-console.h"


//
// -- Write a string to the console and follow it up with a newline
//    -------------------------------------------------------------
void ConsolePutS(const char *str);


#endif
