//===================================================================================================================
//
//  TimerMain.cc -- The main routine for the near-bare-metal timer test
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Dec-14  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


//
// -- some global types
//    -----------------
#include <stdint.h>


//
// -- some asm prototypes
//    -------------------
extern "C" void Dummy(void);
extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" uint32_t GetCBAR(void);
extern "C" uint32_t GetMode(void);
extern "C" uint32_t GetVBAR(void);
extern "C" uint32_t GetSCTLR(void);
extern "C" void Halt(void);
extern "C" void IRQTarget(void);
extern "C" uint64_t SysTimerCount(void);
extern "C" void Undef(void);


//
// -- this is the coding of a near jump
//    ---------------------------------
#define IVEC_JUMP_ASM       (0xe59ff018)


//
// -- Some defined from the hardware config
//    -------------------------------------
#define HW_BASE             (0x3f000000)                   // This is the hardware base address for the system
#define INT_BASE            (HW_BASE+0x00b000)          // The base address for the interrupt registers
#define GPIO_BASE           (HW_BASE+0x200000)          // This is the base address of the GPIO
#define AUX_BASE            (HW_BASE+0x215000)          // Auxiliary peripheral base register
#define TMR_BASE            (HW_BASE+0x01000000)        // The base of the timer registers for each core

#define GPIO_FSEL1          (GPIO_BASE+4)               // GPIO Function Select 1
#define GPIO_GPPUD          (GPIO_BASE+0x94)            // GPIO Pin Pull Up/Down Enable
#define GPIO_GPPUDCLK1      (GPIO_BASE+0x98)            // GPIO Pin Pull Up/Down Enable Clock 0

#define INT_IRQPEND0        (INT_BASE+0x200)            // The basic interrupt pending register
#define INT_FIQCONTROL      (INT_BASE+0x20c)            // This is the FIQ controller and source select
#define INT_IRQENB1         (INT_BASE+0x210)            // IRQ Enable 1
#define INT_IRQENB2         (INT_BASE+0x214)            // IRQ Enable 2
#define INT_IRQENB0         (INT_BASE+0x218)            // Basic IRQ Enable
#define INT_IRQDIS1         (INT_BASE+0x21c)            // IRQ Disable 1
#define INT_IRQDIS2         (INT_BASE+0x220)            // IRQ Disable 2
#define INT_IRQDIS0         (INT_BASE+0x224)            // Basic IRQ Disable
#define INT_TMR_ACK         (INT_BASE+0x40c)            // Writing a non-zero value resets the IRQ

#define AUX_ENABLES         (AUX_BASE+0x004)            // Auxiliary Enables
#define AUX_MU_IO_REG       (AUX_BASE+0x040)            // Mini UART I/O Data
#define AUX_MU_IER_REG      (AUX_BASE+0x044)            // Mini UART Interrupt Enable
#define AUX_MU_IIR_REG      (AUX_BASE+0x048)            // Mini UART Interrupt Identify
#define AUX_MU_LCR_REG      (AUX_BASE+0x04c)            // Mini UART Line Control
#define AUX_MU_MCR_REG      (AUX_BASE+0x050)            // Mini UART Modem Control
#define AUX_MU_LSR_REG      (AUX_BASE+0x054)            // Mini UART Line Status
#define AUX_MU_CNTL_REG     (AUX_BASE+0x060)            // Mini UART Extra Control
#define AUX_MU_BAUD_REG     (AUX_BASE+0x068)            // Mini UART Baudrate

//
// -- Write to a Memory Mapped I/O Register
//    -------------------------------------
inline void MmioWrite(uint32_t reg, uint32_t data) { *(volatile uint32_t *)reg = data; }


//
// -- Read from a Memory Mapped I/O Register
//    --------------------------------------
inline uint32_t MmioRead(uint32_t reg) { return *(volatile uint32_t *)reg; }


//
// -- Write a single character to the UART
//    ------------------------------------
extern "C" void SerialPutChar(char byte)
{
    if (byte == '\n') SerialPutChar('\r');
    while ((MmioRead(AUX_MU_LSR_REG) & (1<<5)) == 0) { }
    MmioWrite(AUX_MU_IO_REG, byte);
}


//
// -- Send a character string to a serial port
//    ----------------------------------------
void SerialPutS(const char *s)
{
    while (*s) SerialPutChar(*s ++);
}


//
// -- Print a hex number to the serial port
//    -------------------------------------
void SerialPutHex(uint32_t val)
{
    char hex[] = "0123456789abcdef";

    SerialPutS("0x");
    for (int i = 28; i >= 0; i -= 4) {
        SerialPutChar(hex[(val >> i) & 0x0f]);
    }
}


//
// -- Wait a some number of MICRO-seconds
//    -----------------------------------
void BusyWait(uint32_t microSecs)
{
    for (uint32_t i = 0; i < microSecs; i ++) Dummy();
}


//
// -- Initialize the UART Serial Port (Might need to enable the GPIO Pins for real hardware)
//    --------------------------------------------------------------------------------------
void SerialInit(void)
{
    // -- must start by enabling the mini-UART; no register access will work until...
    MmioWrite(AUX_ENABLES, 1);

    // -- Disable all interrupts
    MmioWrite(AUX_MU_IER_REG, 0);

    // -- Reset the control register
    MmioWrite(AUX_MU_CNTL_REG, 0);

    // -- Program the Line Control Register -- 8 bits, please
    MmioWrite(AUX_MU_LCR_REG, 3);

    // -- Program the Modem Control Register -- reset
    MmioWrite(AUX_MU_MCR_REG, 0);

    // -- Disable all interrupts -- again
    MmioWrite(AUX_MU_IER_REG, 0);

    // -- Clear all interrupts
    MmioWrite(AUX_MU_IIR_REG, 0xc6);

    // -- Set the BAUD to 115200 -- ((250,000,000/115200)/8)-1 = 270
    MmioWrite(AUX_MU_BAUD_REG, 270);

    // -- Select alternate function 5 to work on GPIO pin 14
    uint32_t sel = MmioRead(GPIO_FSEL1);
    sel &= ~(7<<12);
    sel |= (0b010<<12);
    MmioWrite(GPIO_FSEL1, sel);

    // -- Enable GPIO pins 14/15 only
    MmioWrite(GPIO_GPPUD, 0x00000000);
    BusyWait(150);
    MmioWrite(GPIO_GPPUDCLK1, (1<<14));
    BusyWait(150);
    MmioWrite(GPIO_GPPUDCLK1, 0x00000000);              // LEARN: Why does this make sense?

    // -- Enable TX
    MmioWrite(AUX_MU_CNTL_REG, 2);

    SerialPutS("Serial port initialized!\n");
}


//
// -- This is the interrupt handler (watered down with no registers passed in)
//    ------------------------------------------------------------------------
extern "C" void IRQHandler(void)
{
    SerialPutChar('!');
    MmioWrite(TMR_BASE + 0x38, (1<<30) | (1<<31));  // clear and reload the timer
}


//
// -- This is the main entry point for the test
//    -----------------------------------------
extern "C" void TimerMain(void)
{
    DisableInterrupts();

    // -- quickly initialize the serial port for debugging purposes
    SerialInit();

    // -- for good measure, disable the FIQ
    MmioWrite(INT_FIQCONTROL, 0x0);

    // -- Disable all IRQs -- write to clear, anything that is high will be pulled low
    MmioWrite(INT_IRQDIS0, 0xffffffff);
    MmioWrite(INT_IRQDIS1, 0xffffffff);
    MmioWrite(INT_IRQDIS2, 0xffffffff);

    // -- Now I should be able to set up the timer
    MmioWrite(TMR_BASE + 0x00, 0x00);               // this register is not emulated by qemu
    MmioWrite(TMR_BASE + 0x08, 0x80000000);         // this register is not emulated by qemu
    MmioWrite(TMR_BASE + 0x40, 0x00000002);         // select as IRQ for core 0
    MmioWrite(TMR_BASE + 0x60, 0x00000002);         // enable IRQs from the core for this CPU
    MmioWrite(TMR_BASE + 0x70, 0x00000000);         // force disable FIQ for all sources
    MmioWrite(TMR_BASE + 0x34, (1<<21) | (1<<28) | (1<<29));  // set up the counter for the timer and start it
    MmioWrite(TMR_BASE + 0x38, (1<<30) | (1<<31));  // clear and reload the timer
    MmioWrite(TMR_BASE + 0x24, 0x00000000);         // local timer goes to core 0 IRQ

    // -- Now, enable the ARM Timer interrupt only
    MmioWrite(INT_IRQENB0, 1);

    SerialPutS("Ready to enable interrupts!\n");
    SerialPutS("This is the system configuration:\n");
    SerialPutS("The processor mode is: "); SerialPutHex(GetMode()); SerialPutChar('\n');
    SerialPutS("  VBAR: "); SerialPutHex(GetVBAR()); SerialPutChar('\n');
    SerialPutS("  SCTLR.V: "); SerialPutS(GetSCTLR() & (1<<13) ? "set\n" : "clear\n");
    SerialPutS("  The code at VBAR[0] is: "); SerialPutHex(((uint32_t *)GetVBAR())[0]); SerialPutChar('\n');
    SerialPutS("  The code at VBAR[1] is: "); SerialPutHex(((uint32_t *)GetVBAR())[1]); SerialPutChar('\n');
    SerialPutS("  The code at VBAR[2] is: "); SerialPutHex(((uint32_t *)GetVBAR())[2]); SerialPutChar('\n');
    SerialPutS("  The code at VBAR[3] is: "); SerialPutHex(((uint32_t *)GetVBAR())[3]); SerialPutChar('\n');
    SerialPutS("  The code at VBAR[4] is: "); SerialPutHex(((uint32_t *)GetVBAR())[4]); SerialPutChar('\n');
    SerialPutS("  The code at VBAR[5] is: "); SerialPutHex(((uint32_t *)GetVBAR())[5]); SerialPutChar('\n');
    SerialPutS("  The code at VBAR[6] is: "); SerialPutHex(((uint32_t *)GetVBAR())[6]); SerialPutChar('\n');
    SerialPutS("  The code at VBAR[7] is: "); SerialPutHex(((uint32_t *)GetVBAR())[7]); SerialPutChar('\n');
    SerialPutS("  The code at VBAR[8] is: "); SerialPutHex(((uint32_t *)GetVBAR())[8]); SerialPutChar('\n');
    SerialPutS("  The code at VBAR[9] is: "); SerialPutHex(((uint32_t *)GetVBAR())[9]); SerialPutChar('\n');
    SerialPutS("  The Basic Interrupt register is: "); SerialPutHex(MmioRead(INT_IRQDIS0)); SerialPutChar('\n');

    EnableInterrupts();             // This enables interrupts and the timer should start firing -- does not work

    SerialPutS("Timer is initialized -- interrupts should be happening\n");

    for (int i = 0; i < 10; i ++) {
        for (volatile int j = 0; j < 1000000; j ++) { }
        SerialPutS("The timer value is: "); SerialPutHex(MmioRead(TMR_BASE + 0x1c)); SerialPutChar('\n');
    }

//    DisableInterrupts();            // cap the duration of the test

    while(1) {}
}

