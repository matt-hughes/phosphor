#ifndef _emu_h_
#define _emu_h_

#include <stdint.h>

void emu_init(void);
void emu_clearMemory(void);
void emu_loadMemory(uint16_t addr, const char* file);
uint8_t* emu_getVideoRam(void);
void emu_run(uint32_t usec);
void emu_triggerIRQ(uint32_t mask);

#endif
