

#include "cpu.h"
#include "serial.h"


void SerialPutChar(const char ch)
{
    while ((inb(serialPort + 5) & 0x20) == 0) {}

    outb(serialPort, ch);
}
