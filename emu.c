#include "emu.h"

#define EMU_RAM_SIZE_BYTES      0x10000
#define EMU_VIDEO_RAM_OFFSET    0x8000

#define EMU_IO_ADDRESS_MASK     0xF000
#define EMU_IO_ADDRESS_VALUE    0xE000

#define EMU_TICKS_PER_SEC       1e9

struct
{
    uint32_t irqPending;
    uint8_t ram[EMU_RAM_SIZE_BYTES];
} gEmu;

uint8_t read6502(uint16_t address)
{
    uint8_t value = gEmu.ram[address];
    if(address & EMU_IO_ADDRESS_MASK == EMU_IO_ADDRESS_VALUE)
    {
    }
    return value;
}

void write6502(uint16_t address, uint8_t value)
{
    gEmu.ram[address] = value;
    if(address & EMU_IO_ADDRESS_MASK == EMU_IO_ADDRESS_VALUE)
    {
        // switch(address)
        // {
        // }
    }
}

void emu_init(void)
{
    emu_clearMemory();
}

void emu_clearMemory(void)
{
    memset(&gEmu, 0, sizeof(gEmu));
}

void emu_loadMemory(uint16_t addr, const char* file)
{
    
}

uint8_t* emu_getVideoRam(void)
{
    return &gEmu.ram[EMU_VIDEO_RAM_OFFSET];
}

void emu_run(uint32_t usec)
{
    uint32_t tickCount = (EMU_TICKS_PER_SEC*1e-6) * usec;
    if(gEmu.irqPending)
    {
        irq6502();
        gEmu.irqPending = 0;
    }
    exec6502(tickCount);
}

void emu_triggerIRQ(uint32_t mask)
{
    gEmu.irqPending |= mask;
}