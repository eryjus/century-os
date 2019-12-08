
#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "loader.h"
#include "timer.h"
#include "serial.h"


bool bootAp = false;

extern "C" {
    EXPORT LOADER void kInitAp(void);
}


//
// -- This is AP Entry point.  While we have a shared temporary stack and need to get that
//    -----------------------------------------------------------------------------------------------------
extern "C" EXPORT KERNEL
void kInitAp(void)
{
    FpuInit();

    TimerInit(timerControl, 1000);
    EnableInterrupts();

    kprintf("CPU %x running\n", CpuNum());

    while (true) {
        kprintf("*");
        for (volatile int i = 0; i < 10000; i ++) {}
    }
}