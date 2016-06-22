#ifndef _fake6502_h_
#define _fake6502_h_

#include <stdint.h>

//externally supplied functions
extern uint8_t read6502(uint16_t address);
extern void write6502(uint16_t address, uint8_t value);

void reset6502();
void exec6502(uint32_t tickcount);
void step6502();
void irq6502();
void nmi6502();
void hookexternal(void *funcptr);

#endif
