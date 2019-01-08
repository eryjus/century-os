

#include <stdint.h>

#include "hw.h"


void BusyWait(uint32_t microSecs);
extern "C" uint32_t SysTimerCount(void);


void SerialInit(void);
void SerialPutChar(char byte);
void SerialPutHex(uint32_t val);
void SerialPutS(const char *s);



inline void MmioWrite(uint32_t reg, uint32_t data) { *(volatile uint32_t *)reg = data; }
inline uint32_t MmioRead(uint32_t reg) { return *(volatile uint32_t *)reg; }


uint32_t MailboxReceive(uint32_t mailbox);
bool MailboxSend(uint32_t message, uint32_t mailbox);
inline void MailboxWaitReadyToSend(void) { while (MmioRead(MB_BASE + MB_STATUS) & (1 << 31)) { } }
inline void MailboxWaitReadyToReceive(void) { while (MmioRead(MB_BASE + MB_STATUS) & (1 << 30)) { } }
