

#include "types.h"


void IdtSetGate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

void IdtBuild(void);

void IsrUnregister(uint8_t interrupt);

void IsrRegister(uint8_t interrupt, isrFunc_t func);

void IsrDumpState(isrRegs_t *regs);


void IsrInt00(isrRegs_t *regs);
void IsrInt08(isrRegs_t *regs);
void IsrInt0d(isrRegs_t *regs);
void IsrInt0e(isrRegs_t *regs);
