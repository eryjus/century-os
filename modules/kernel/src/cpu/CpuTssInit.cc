

#include "cpu.h"
#include "printf.h"
#include "tss.h"


void CpuTssInit(void)
{
    kprintf("The TSS contains: %p:%p\n", *((uint32_t *)(&cpuStructs->gdt[9])), *(((uint32_t *)&cpuStructs->gdt[9]) + 1));
	kMemSetB((void *)512, 0, 512);
	cpuStructs->tss.ss0 = 0x10;
	cpuStructs->tss.esp0 = ((ptrsize_t)tssStack + TSS_STACK_SIZE);
	cpuStructs->tss.ds = cpuStructs->tss.es = cpuStructs->tss.fs = cpuStructs->tss.gs = cpuStructs->tss.ss = 0x10;
	cpuStructs->tss.cs = 0x08;

	Ltr(0x4b);
}