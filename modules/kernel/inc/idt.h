

#include "types.h"


void IdtSetGate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

void IdtBuild(void);

void IsrUnregister(uint8_t interrupt);

void IsrRegister(uint8_t interrupt, isrFunc_t func);

void IsrDumpState(isrRegs_t *regs);


void IsrInt00(isrRegs_t *regs);
void IsrInt01(isrRegs_t *regs);
void IsrInt02(isrRegs_t *regs);
void IsrInt03(isrRegs_t *regs);
void IsrInt04(isrRegs_t *regs);
void IsrInt05(isrRegs_t *regs);
void IsrInt06(isrRegs_t *regs);
void IsrInt07(isrRegs_t *regs);
void IsrInt08(isrRegs_t *regs);
void IsrInt09(isrRegs_t *regs);
void IsrInt0a(isrRegs_t *regs);
void IsrInt0b(isrRegs_t *regs);
void IsrInt0c(isrRegs_t *regs);
void IsrInt0d(isrRegs_t *regs);
void IsrInt0e(isrRegs_t *regs);
void IsrInt0f(isrRegs_t *regs);
void IsrInt10(isrRegs_t *regs);
void IsrInt11(isrRegs_t *regs);
void IsrInt12(isrRegs_t *regs);
void IsrInt13(isrRegs_t *regs);
void IsrInt14(isrRegs_t *regs);
void IsrInt15(isrRegs_t *regs);
void IsrInt16(isrRegs_t *regs);
void IsrInt17(isrRegs_t *regs);
void IsrInt18(isrRegs_t *regs);
void IsrInt19(isrRegs_t *regs);
void IsrInt1a(isrRegs_t *regs);
void IsrInt1b(isrRegs_t *regs);
void IsrInt1c(isrRegs_t *regs);
void IsrInt1d(isrRegs_t *regs);
void IsrInt1e(isrRegs_t *regs);
void IsrInt1f(isrRegs_t *regs);
