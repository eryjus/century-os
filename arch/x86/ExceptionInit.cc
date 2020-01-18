//===================================================================================================================
//
//  ExceptionInit.cc -- Build the IDT Table in-place
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function will build the IDT table in-place at location 0x00000800.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-May-30  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "mmu.h"
#include "cpu.h"
#include "printf.h"
#include "interrupt.h"


//
// -- The ISR entry points as functions declared
//    ------------------------------------------
EXTERN_C EXPORT KERNEL
void isr0  (void);

EXTERN_C EXPORT KERNEL
void isr1  (void);

EXTERN_C EXPORT KERNEL
void isr2  (void);

EXTERN_C EXPORT KERNEL
void isr3  (void);

EXTERN_C EXPORT KERNEL
void isr4  (void);

EXTERN_C EXPORT KERNEL
void isr5  (void);

EXTERN_C EXPORT KERNEL
void isr6  (void);

EXTERN_C EXPORT KERNEL
void isr7  (void);

EXTERN_C EXPORT KERNEL
void isr8  (void);

EXTERN_C EXPORT KERNEL
void isr9  (void);

EXTERN_C EXPORT KERNEL
void isr10 (void);

EXTERN_C EXPORT KERNEL
void isr11 (void);

EXTERN_C EXPORT KERNEL
void isr12 (void);

EXTERN_C EXPORT KERNEL
void isr13 (void);

EXTERN_C EXPORT KERNEL
void isr14 (void);

EXTERN_C EXPORT KERNEL
void isr15 (void);

EXTERN_C EXPORT KERNEL
void isr16 (void);

EXTERN_C EXPORT KERNEL
void isr17 (void);

EXTERN_C EXPORT KERNEL
void isr18 (void);

EXTERN_C EXPORT KERNEL
void isr19 (void);

EXTERN_C EXPORT KERNEL
void isr20 (void);

EXTERN_C EXPORT KERNEL
void isr21 (void);

EXTERN_C EXPORT KERNEL
void isr22 (void);

EXTERN_C EXPORT KERNEL
void isr23 (void);

EXTERN_C EXPORT KERNEL
void isr24 (void);

EXTERN_C EXPORT KERNEL
void isr25 (void);

EXTERN_C EXPORT KERNEL
void isr26 (void);

EXTERN_C EXPORT KERNEL
void isr27 (void);

EXTERN_C EXPORT KERNEL
void isr28 (void);

EXTERN_C EXPORT KERNEL
void isr29 (void);

EXTERN_C EXPORT KERNEL
void isr30 (void);

EXTERN_C EXPORT KERNEL
void isr31 (void);

EXTERN_C EXPORT SYSCALL
void isr100(void);

EXTERN_C EXPORT KERNEL
void irq0  (void);

EXTERN_C EXPORT KERNEL
void irq1  (void);

EXTERN_C EXPORT KERNEL
void irq2  (void);

EXTERN_C EXPORT KERNEL
void irq3  (void);

EXTERN_C EXPORT KERNEL
void irq4  (void);

EXTERN_C EXPORT KERNEL
void irq5  (void);

EXTERN_C EXPORT KERNEL
void irq6  (void);

EXTERN_C EXPORT KERNEL
void irq7  (void);

EXTERN_C EXPORT KERNEL
void irq8  (void);

EXTERN_C EXPORT KERNEL
void irq9  (void);

EXTERN_C EXPORT KERNEL

void irq10 (void);
EXTERN_C EXPORT KERNEL
void irq11 (void);

EXTERN_C EXPORT KERNEL
void irq12 (void);

EXTERN_C EXPORT KERNEL
void irq13 (void);

EXTERN_C EXPORT KERNEL
void irq14 (void);

EXTERN_C EXPORT KERNEL
void irq15 (void);

EXTERN_C EXPORT KERNEL
void irq16 (void);

EXTERN_C EXPORT KERNEL
void irq17 (void);

EXTERN_C EXPORT KERNEL
void irq18 (void);

EXTERN_C EXPORT KERNEL
void irq19 (void);

EXTERN_C EXPORT KERNEL
void irq20 (void);

EXTERN_C EXPORT KERNEL
void irq21 (void);

EXTERN_C EXPORT KERNEL
void irq22 (void);

EXTERN_C EXPORT KERNEL
void irq23 (void);

EXTERN_C EXPORT KERNEL
void irq240(void);

EXTERN_C EXPORT KERNEL
void irq241(void);

EXTERN_C EXPORT KERNEL
void irq242(void);

EXTERN_C EXPORT KERNEL
void irq243(void);

EXTERN_C EXPORT KERNEL
void irq244(void);

EXTERN_C EXPORT KERNEL
void irq245(void);

EXTERN_C EXPORT KERNEL
void irq246(void);

EXTERN_C EXPORT KERNEL
void irq247(void);

EXTERN_C EXPORT KERNEL
void irq248(void);

EXTERN_C EXPORT KERNEL
void irq249(void);

EXTERN_C EXPORT KERNEL
void irq250(void);

EXTERN_C EXPORT KERNEL
void irq251(void);

EXTERN_C EXPORT KERNEL
void irq252(void);

EXTERN_C EXPORT KERNEL
void irq253(void);

EXTERN_C EXPORT KERNEL
void irq254(void);

EXTERN_C EXPORT KERNEL
void irq255(void);


//
// -- Build the parts of the IDT we are going to use so far
//    -----------------------------------------------------
EXTERN_C EXPORT LOADER
void ExceptionInit(void)
{
    kprintf("Initializing the IDT properly\n");

    MmuMapToFrame(X86_VIRT_IDT, X86_PHYS_IDT >> 12, PG_WRT | PG_KRN);
    kMemSetB((void *)X86_VIRT_IDT, 0, sizeof(IdtEntry_t) * 256);

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
    IdtSetGate(48, (uint32_t)irq16, 0x08, 0x8e);
    IdtSetGate(49, (uint32_t)irq17, 0x08, 0x8e);
    IdtSetGate(50, (uint32_t)irq18, 0x08, 0x8e);
    IdtSetGate(51, (uint32_t)irq19, 0x08, 0x8e);
    IdtSetGate(52, (uint32_t)irq20, 0x08, 0x8e);
    IdtSetGate(53, (uint32_t)irq21, 0x08, 0x8e);
    IdtSetGate(54, (uint32_t)irq22, 0x08, 0x8e);
    IdtSetGate(55, (uint32_t)irq23, 0x08, 0x8e);

    IdtSetGate(100, (uint32_t)isr100, 0x0b, 0x8e|0x60);     // available from user space

    IdtSetGate(240, (uint32_t)irq240, 0x08, 0x8e);
    IdtSetGate(241, (uint32_t)irq241, 0x08, 0x8e);
    IdtSetGate(242, (uint32_t)irq242, 0x08, 0x8e);
    IdtSetGate(243, (uint32_t)irq243, 0x08, 0x8e);
    IdtSetGate(244, (uint32_t)irq244, 0x08, 0x8e);
    IdtSetGate(245, (uint32_t)irq245, 0x08, 0x8e);
    IdtSetGate(246, (uint32_t)irq246, 0x08, 0x8e);
    IdtSetGate(247, (uint32_t)irq247, 0x08, 0x8e);
    IdtSetGate(248, (uint32_t)irq248, 0x08, 0x8e);
    IdtSetGate(249, (uint32_t)irq249, 0x08, 0x8e);
    IdtSetGate(250, (uint32_t)irq250, 0x08, 0x8e);
    IdtSetGate(251, (uint32_t)irq251, 0x08, 0x8e);
    IdtSetGate(252, (uint32_t)irq252, 0x08, 0x8e);
    IdtSetGate(253, (uint32_t)irq253, 0x08, 0x8e);
    IdtSetGate(254, (uint32_t)irq254, 0x08, 0x8e);
    IdtSetGate(255, (uint32_t)irq255, 0x08, 0x8e);


    // -- Finally we need to load the new GDT
    struct {
        uint16_t size;
        uintptr_t loc;
    } __attribute__((packed)) idtRec = {
        (uint16_t)((sizeof(IdtEntry_t) * 256) - 1),
        X86_VIRT_IDT,
    };

    LoadIdt(&idtRec);


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

    IsrRegister(100, SyscallHandler);
}
