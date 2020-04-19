//===================================================================================================================
//
//  stdlib.h -- Standard library
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This is the Century implementation of stdlib.h
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-18  Initial  v0.7.0a  ADCL  Initial version
//
//===================================================================================================================


#pragma once


//
// -- these are the system call numbers
//    ---------------------------------
typedef enum {
    SYS_EXIT = 0,
} SyscallNumber_t;




extern "C" int syscall(int func, int parmCnt, ...);
extern "C" void exit(int status);


