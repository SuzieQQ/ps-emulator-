 #pragma once
 #include "typedef.h"

// BIOS Region (default 512 Kbytes, max 4 MBytes)
#define BIOS_ADDR 0x1FC00000
#define BIOS_SIZE KB * 512

#define RAM_ADDR 0x00000000

#define RAM_SIZE_2MB MB * 2
#define RAM_SIZE_8MB MB * 8

// Scratchpad
#define SCRATCHPAD_ADDR 0x1F800000
#define SCRATCHPAD_SIZE KB

//Expansion Region 1
#define EXPANSION_REGION1_ADDR  0x1F000000 
#define EXPANSION_REGION1_SIZE  MB


// Expansion Region 2 (default 128 bytes, max 8 KBytes)
#define EXPANSION_REGION2_ADDR  0x1F802000 //  1F802000h      80h Expansion Region (8bit data bus, crashes on 16bit access?)
#define EXPANSION_REGION2_SIZE  0x42

// Expansion Region 3 (default 1 byte, max 2 MBytes)
#define EXPANSION_REGION3_ADDR 0x1FA00000
#define EXPANSION_REGION3_SIZE MB * 2


// Memory Control 1
#define MEMORY_CONTROL1_ADDR 0x1F801000
#define MEMORY_CONTROL1_SIZE 0x24

// Memory Control 2
#define MEMORY_CONTROL2_ADDR 0x1F801060
#define MEMORY_CONTROL2_SIZE 0x4

// Memory Control 3
#define MEMORY_CONTROL3_ADDR 0xFFFE0130
#define MEMORY_CONTROL3_SIZE 0x4


// Peripheral I/O Ports
#define JOYPAD_ADDR 0x1F801040
#define JOYPAD_SIZE 0x10

// CDROM Registers (Address.Read/Write.Index)
#define CD_ROM_ADDR 0x1F801800
#define CD_ROM_SIZE 4


#define SIO_ADDR 0x1F801050
#define SIO_SIZE 0x10

// Interrupt Control
#define IRQ_CONTROL_ADDR 0x1F801070
#define IRQ_CONTROL_SIZE 8

// DMA Registers
 #define DMA_REG_ADDR 0x1F801080
 #define DMA_REG_SIZE 0x80

// Timers (aka Root counters)
#define TIMER_ADDR 0x1F801100
#define TIMER_SIZE 0x2C

// GPU Registers
#define GPU_REG_ADDR 0x1F801810
#define GPU_REG_SIZE 8

// MDEC Registers
#define MDEC_REG_ADDR 0x1F801820
#define MDEC_REG_SIZE 8

// SPU Control Registers
#define SPU_CONTROL_ADDR 0x1F801D80
#define SPU_SIZE 0x280 

static const int KB = (0x400);  // 1 KB (kilobyte) in Byte 1.024
static const int MB = (0x400 * 0x400); // 1 MB (megabyte) in Byte 1.048.576
static const int GB = (0x400 * 0x400 * 0x400);  // 1 GB (gigabyte) in Byte 1.073.741.824


typedef struct 
{
    u8 ram[(0x400 * 0x400) * 2];

}Memory;

static Memory var;

u32 fix_addresses(u32 addr ,u32 index,u32 size,u32 offset); // offeset index

u8  read8(u32 addr);
u16 read16(u32 addr);
u32 read32(u32 addr);

void write8(u32 addr,u8 value);
void write16(u32 addr,u16 value);
void write32(u32 addr,u32 value);

 
 // KSEG0 contains kernel code and data, but is unmapped. Translations are direct.
 
 // KSEG1 like KSEG0, but uncached. Used for I/O space.
 
 // KSEG2 is kernel space, but cached and mapped. Contains page tables for KUSEG.
 
 // Implication is that the page tables are kept in VIRTUAL memory!

 u32 region_memory(u32 addr); // kuseg / kseg0 / kseg1 / kseg2