//===================================================================================================================
// kernel/src/x86-common/BuildIdt.cc -- Build the IDT Table in-place
//
// This function will build the IDT table in-place at location 0x00000800.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-05-30  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "locations.h"
#include "cpu.h"
#include "idt.h"


//
// -- The ISR entry points as functions declared
//    ------------------------------------------
extern "C" {
    extern void isr0  (void);
    extern void isr1  (void);
    extern void isr2  (void);
    extern void isr3  (void);
    extern void isr4  (void);
    extern void isr5  (void);
    extern void isr6  (void);
    extern void isr7  (void);
    extern void isr8  (void);
    extern void isr9  (void);
    extern void isr10 (void);
    extern void isr11 (void);
    extern void isr12 (void);
    extern void isr13 (void);
    extern void isr14 (void);
    extern void isr15 (void);
    extern void isr16 (void);
    extern void isr17 (void);
    extern void isr18 (void);
    extern void isr19 (void);
    extern void isr20 (void);
    extern void isr21 (void);
    extern void isr22 (void);
    extern void isr23 (void);
    extern void isr24 (void);
    extern void isr25 (void);
    extern void isr26 (void);
    extern void isr27 (void);
    extern void isr28 (void);
    extern void isr29 (void);
    extern void isr30 (void);
    extern void isr31 (void);

    extern void irq0  (void);
    extern void irq1  (void);
    extern void irq2  (void);
    extern void irq3  (void);
    extern void irq4  (void);
    extern void irq5  (void);
    extern void irq6  (void);
    extern void irq7  (void);
    extern void irq8  (void);
    extern void irq9  (void);
    extern void irq10 (void);
    extern void irq11 (void);
    extern void irq12 (void);
    extern void irq13 (void);
    extern void irq14 (void);
    extern void irq15 (void);
}


//
// -- The location of the IDT.  Note that this is a foxed point in linear memory
//    --------------------------------------------------------------------------
IdtEntry *idtEntries = (IdtEntry *)IDT_ADDRESS;


//
// -- Build the parts of the IDT we are going to use so far
//    -----------------------------------------------------
void IdtBuild(void)
{
	kMemSetB((uint8_t *)idtEntries, 0, sizeof(IdtEntry) * 256);

	IdtSetGate( 0, (uint32_t)isr0 , 0x08, 0x8e);
	IdtSetGate( 1, (uint32_t)isr1 , 0x08, 0x8e);
	IdtSetGate( 2, (uint32_t)isr2 , 0x08, 0x8e);
	IdtSetGate( 3, (uint32_t)isr3 , 0x08, 0x8e);
	IdtSetGate( 4, (uint32_t)isr4 , 0x08, 0x8e);
	IdtSetGate( 5, (uint32_t)isr5 , 0x08, 0x8e);
	IdtSetGate( 6, (uint32_t)isr6 , 0x08, 0x8e);
	IdtSetGate( 7, (uint32_t)isr7 , 0x08, 0x8e);
	IdtSetGate( 8, (uint32_t)isr8 , 0x08, 0x8e);
	IdtSetGate( 9, (uint32_t)isr9 , 0x08, 0x8e);
	IdtSetGate(10, (uint32_t)isr10, 0x08, 0x8e);
	IdtSetGate(11, (uint32_t)isr11, 0x08, 0x8e);
	IdtSetGate(12, (uint32_t)isr12, 0x08, 0x8e);
	IdtSetGate(13, (uint32_t)isr13, 0x08, 0x8e);
	IdtSetGate(14, (uint32_t)isr14, 0x08, 0x8e);
	IdtSetGate(15, (uint32_t)isr15, 0x08, 0x8e);
	IdtSetGate(16, (uint32_t)isr16, 0x08, 0x8e);
	IdtSetGate(17, (uint32_t)isr17, 0x08, 0x8e);
	IdtSetGate(18, (uint32_t)isr18, 0x08, 0x8e);
	IdtSetGate(19, (uint32_t)isr19, 0x08, 0x8e);
	IdtSetGate(20, (uint32_t)isr20, 0x08, 0x8e);
	IdtSetGate(21, (uint32_t)isr21, 0x08, 0x8e);
	IdtSetGate(22, (uint32_t)isr22, 0x08, 0x8e);
	IdtSetGate(23, (uint32_t)isr23, 0x08, 0x8e);
	IdtSetGate(24, (uint32_t)isr24, 0x08, 0x8e);
	IdtSetGate(25, (uint32_t)isr25, 0x08, 0x8e);
	IdtSetGate(26, (uint32_t)isr26, 0x08, 0x8e);
	IdtSetGate(27, (uint32_t)isr27, 0x08, 0x8e);
	IdtSetGate(28, (uint32_t)isr28, 0x08, 0x8e);
	IdtSetGate(29, (uint32_t)isr29, 0x08, 0x8e);
	IdtSetGate(30, (uint32_t)isr30, 0x08, 0x8e);
	IdtSetGate(31, (uint32_t)isr31, 0x08, 0x8e);

	IdtSetGate(32, (uint32_t)irq0 , 0x08, 0x8e);
	IdtSetGate(33, (uint32_t)irq1 , 0x08, 0x8e);
	IdtSetGate(34, (uint32_t)irq2 , 0x08, 0x8e);
	IdtSetGate(35, (uint32_t)irq3 , 0x08, 0x8e);
	IdtSetGate(36, (uint32_t)irq4 , 0x08, 0x8e);
	IdtSetGate(37, (uint32_t)irq5 , 0x08, 0x8e);
	IdtSetGate(38, (uint32_t)irq6 , 0x08, 0x8e);
	IdtSetGate(39, (uint32_t)irq7 , 0x08, 0x8e);
	IdtSetGate(40, (uint32_t)irq8 , 0x08, 0x8e);
	IdtSetGate(41, (uint32_t)irq9 , 0x08, 0x8e);
	IdtSetGate(42, (uint32_t)irq10, 0x08, 0x8e);
	IdtSetGate(43, (uint32_t)irq11, 0x08, 0x8e);
	IdtSetGate(44, (uint32_t)irq12, 0x08, 0x8e);
	IdtSetGate(45, (uint32_t)irq13, 0x08, 0x8e);
	IdtSetGate(46, (uint32_t)irq14, 0x08, 0x8e);
	IdtSetGate(47, (uint32_t)irq15, 0x08, 0x8e);

	// -- Register the individual ISR routines
	IsrRegister(0x00, IsrInt00);
	IsrRegister(0x01, IsrInt01);
	IsrRegister(0x02, IsrInt02);
	IsrRegister(0x03, IsrInt03);
	IsrRegister(0x04, IsrInt04);
	IsrRegister(0x05, IsrInt05);
	IsrRegister(0x06, IsrInt06);
	IsrRegister(0x07, IsrInt07);
	IsrRegister(0x08, IsrInt08);
	IsrRegister(0x09, IsrInt09);
	IsrRegister(0x0a, IsrInt0a);
	IsrRegister(0x0b, IsrInt0b);
	IsrRegister(0x0c, IsrInt0c);
	IsrRegister(0x0d, IsrInt0d);
	IsrRegister(0x0e, IsrInt0e);
	IsrRegister(0x0f, IsrInt0f);
	IsrRegister(0x10, IsrInt10);
	IsrRegister(0x11, IsrInt11);
	IsrRegister(0x12, IsrInt12);
	IsrRegister(0x13, IsrInt13);
	IsrRegister(0x14, IsrInt14);
	IsrRegister(0x15, IsrInt15);
	IsrRegister(0x16, IsrInt16);
	IsrRegister(0x17, IsrInt17);
	IsrRegister(0x18, IsrInt18);
	IsrRegister(0x19, IsrInt19);
	IsrRegister(0x1a, IsrInt1a);
	IsrRegister(0x1b, IsrInt1b);
	IsrRegister(0x1c, IsrInt1c);
	IsrRegister(0x1d, IsrInt1d);
	IsrRegister(0x1e, IsrInt1e);
	IsrRegister(0x1f, IsrInt1f);
}