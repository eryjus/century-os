
#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "loader.h"
#include "timer.h"
#include "serial.h"


bool bootAp = false;

__CENTURY_FUNC__ void kInitAp(void);

void __ldrtext kInitAp(void)
{
    cpus.cpuCount ++;
    FpuInit();

    SerialPutChar(&kernelSerial, '*');
    kprintf("CPU %x running\n", CpuNum());

    TimerInit(timerControl, 1000);
    EnableInterrupts();

    while (true) {SerialPutChar(&kernelSerial, '*');}
}