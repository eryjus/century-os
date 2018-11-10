

#include "tss.h"


//
// -- Point to the CPU structures in virtual memory
//    ---------------------------------------------
Frame0_t *cpuStructs = (Frame0_t *)0xff401000;


//
// -- This is a small stack for the TSS
//    ---------------------------------
byte_t tssStack[TSS_STACK_SIZE];


