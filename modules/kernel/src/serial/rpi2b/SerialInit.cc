//===================================================================================================================
//
//  SerialInit.cc -- Initialize a serial port for debugging output
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-13  Initial   0.2.0   ADCL  Initial version -- leveraged out of century's `uart-dev.c`
//
//===================================================================================================================


#include "cpu.h"
#include "hw.h"
#include "serial.h"


//
// -- Initialize the UART Serial Port
//    -------------------------------
void SerialInit(void)
{
    // -- Disable the UART
    MmioWrite(UART_BASE + UART_CR, 0x00000000);

    // -- Enable GPIO pins 14/15 only
    MmioWrite(GPIO_GPPUD, 0x00000000);
    BusyWait(150);
    MmioWrite(GPIO_GPPUDCLK1, GPIOCLK1_14 | GPIOCLK1_15);
    BusyWait(150);
    MmioWrite(GPIO_GPPUDCLK1, 0x00000000);              // LEARN: Why does this make sense?

    // -- Clear any pending interrupts
    MmioWrite(UART_BASE + UART_ICR, UARTMIS_OEIC | UARTMIS_BEIC | UARTMIS_PEIC | UARTMIS_FEIC |
                                           UARTMIS_RTIC | UARTMIS_TXIC | UARTMIS_RXIC | UARTMIS_CTSMIC);

    // -- Mask all interrupts
    MmioWrite(UART_BASE + UART_IMSC, UARTIMSC_OEIM | UARTIMSC_BEIM | UARTIMSC_PEIM | UARTIMSC_FEIM |
                                            UARTIMSC_RTIM | UARTIMSC_TXIM | UARTIMSC_RXIM | UARTIMSC_CTSMIM);

    // -- Enable the FIFO queues
    MmioWrite(UART_BASE + UART_LCRH, UARTLCRH_FEN);

    // -- Set the parameters for the uart: 38400, 8, N, 1
    uint32_t iBaud;
    uint32_t fBaud;
    uint32_t lcrh = 0;

    // -- Set the data width size
    lcrh |= SH_UARTLCRHWLEN_8;

    // -- calculate the baud rate integer and fractional divisor parts.  Note this works since it can be calculated
    //    statically at compile time -- even for fBaud since iBaud is constant.
    iBaud = 3000000 / (16 * 38400);
    fBaud = ((((3000000 / (16 * 38400)) - (iBaud * 100)) * 64) + 50) / 100;

    // -- Finally, configure the UART
    MmioWrite(UART_BASE + UART_IBRD, iBaud);
    MmioWrite(UART_BASE + UART_FBRD, fBaud);
    MmioWrite(UART_BASE + UART_LCRH, lcrh);

    // -- Enable the newly configured UART (not transmitting/receiving yet)
    MmioWrite(UART_BASE + UART_CR, UARTCR_EN);

}
