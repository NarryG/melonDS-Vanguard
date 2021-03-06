/*
    Copyright 2016-2019 Arisotura

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#ifndef NDS_H
#define NDS_H

#include "Savestate.h"
#include "types.h"

// when touching the main loop/timing code, pls test a lot of shit
// with this enabled, to make sure it doesn't desync
//#define DEBUG_CHECK_DESYNC

namespace NDS
{

enum
{
    Event_LCD = 0,
    Event_SPU,
    Event_Wifi,

    Event_DisplayFIFO,
    Event_ROMTransfer,
    Event_ROMSPITransfer,
    Event_SPITransfer,
    Event_Div,
    Event_Sqrt,

    Event_MAX
};

typedef struct
{
    void (*Func)(u32 param);
    u64 Timestamp;
    u32 Param;

} SchedEvent;

enum
{
    IRQ_VBlank = 0,
    IRQ_HBlank,
    IRQ_VCount,
    IRQ_Timer0,
    IRQ_Timer1,
    IRQ_Timer2,
    IRQ_Timer3,
    IRQ_RTC,
    IRQ_DMA0,
    IRQ_DMA1,
    IRQ_DMA2,
    IRQ_DMA3,
    IRQ_Keypad,
    IRQ_GBASlot,
    IRQ_Unused14,
    IRQ_Unused15,
    IRQ_IPCSync,
    IRQ_IPCSendDone,
    IRQ_IPCRecv,
    IRQ_CartSendDone,
    IRQ_CartIREQMC,
    IRQ_GXFIFO,
    IRQ_LidOpen,
    IRQ_SPI,
    IRQ_Wifi
};

typedef struct
{
    u16 Reload;
    u16 Cnt;
    u32 Counter;
    u32 CycleShift;

} Timer;

typedef struct
{
    u8* Mem;
    u32 Mask;

} MemRegion;

extern u8 ARM9MemTimings[0x40000][4];
extern u8 ARM7MemTimings[0x20000][4];

extern u64 ARM9Timestamp, ARM9Target;
extern u64 ARM7Timestamp, ARM7Target;
extern u32 ARM9ClockShift;

// hax
extern u32 IME[2];
extern u32 IE[2];
extern u32 IF[2];
extern Timer Timers[8];

extern u16 PowerControl9;

extern u16 ExMemCnt[2];
extern u8 ROMSeed0[2*8];
extern u8 ROMSeed1[2*8];

extern u8 ARM9BIOS[0x1000];
extern u8 ARM7BIOS[0x4000];

#define MAIN_RAM_SIZE 0x400000

extern u8 MainRAM[MAIN_RAM_SIZE];
extern u8 SharedWRAM[0x8000];
extern u8 ARM7WRAM[0x10000];

bool Init();
void DeInit();
void Reset();
void Stop();

bool DoSavestate(Savestate* file);

void SetARM9RegionTimings(u32 addrstart, u32 addrend, int buswidth, int nonseq, int seq);
void SetARM7RegionTimings(u32 addrstart, u32 addrend, int buswidth, int nonseq, int seq);

bool LoadROM(const char* path, const char* sram, bool direct);
void LoadBIOS();
void SetupDirectBoot();
void RelocateSave(const char* path, bool write);

u32 RunFrame();

void PressKey(u32 key);
void ReleaseKey(u32 key);
void TouchScreen(u16 x, u16 y);
void ReleaseScreen();

void SetKeyMask(u32 mask);

void SetLidClosed(bool closed);

void MicInputFrame(s16* data, int samples);

void ScheduleEvent(u32 id, bool periodic, s32 delay, void (*func)(u32), u32 param);
void CancelEvent(u32 id);

void debug(u32 p);

void Halt();

void MapSharedWRAM(u8 val);

void SetIRQ(u32 cpu, u32 irq);
void ClearIRQ(u32 cpu, u32 irq);
bool HaltInterrupted(u32 cpu);
void StopCPU(u32 cpu, u32 mask);
void ResumeCPU(u32 cpu, u32 mask);
void GXFIFOStall();
void GXFIFOUnstall();

u32 GetPC(u32 cpu);
u64 GetSysClockCycles(int num);
void NocashPrint(u32 cpu, u32 addr);

bool DMAsInMode(u32 cpu, u32 mode);
bool DMAsRunning(u32 cpu);
void CheckDMAs(u32 cpu, u32 mode);
void StopDMAs(u32 cpu, u32 mode);

void RunTimers(u32 cpu);

u8 ARM9Read8(u32 addr);
u16 ARM9Read16(u32 addr);
u32 ARM9Read32(u32 addr);
void ARM9Write8(u32 addr, u8 val);
void ARM9Write16(u32 addr, u16 val);
void ARM9Write32(u32 addr, u32 val);

bool ARM9GetMemRegion(u32 addr, bool write, MemRegion* region);

u8 ARM7Read8(u32 addr);
u16 ARM7Read16(u32 addr);
u32 ARM7Read32(u32 addr);
void ARM7Write8(u32 addr, u8 val);
void ARM7Write16(u32 addr, u16 val);
void ARM7Write32(u32 addr, u32 val);

bool ARM7GetMemRegion(u32 addr, bool write, MemRegion* region);

u8 ARM9IORead8(u32 addr);
u16 ARM9IORead16(u32 addr);
u32 ARM9IORead32(u32 addr);
void ARM9IOWrite8(u32 addr, u8 val);
void ARM9IOWrite16(u32 addr, u16 val);
void ARM9IOWrite32(u32 addr, u32 val);

u8 ARM7IORead8(u32 addr);
u16 ARM7IORead16(u32 addr);
u32 ARM7IORead32(u32 addr);
void ARM7IOWrite8(u32 addr, u8 val);
void ARM7IOWrite16(u32 addr, u16 val);
void ARM7IOWrite32(u32 addr, u32 val);

}

#endif // NDS_H
