#pragma once

#include <cstdint>
#include <stdio.h>

#include "../cpu/cp0.h"
#include "../gpu/cxd85xxx.h"
#include "../bios_chip/bios_chip.h"
#include "../defines/defines.h"
#include "../bus/dma/dma_controller.h"
#include "../cd_drive/hc05_pux.h"
#include "../timers/timer.h"
#include "../timers/timer_0.h"
#include "../timers/timer_1.h"
#include "../timers/timer_2.h"
#include "../peripherals/joyMemCard.h"
#include "../debug_utilities/debug_utilities.h""

#define BIOS_SIZE (uint32_t)(512 * 1024)
#define RAM_SIZE  4 * 2 * 1024 * 1024 / 4
#define SCRATCHPAD_SIZE  1024 * 1024

class busInterface {
public:

	cp0* pCp0;
	cxd85xxx* pGpu;
	dmaController dma;
	hc05_pux* pCdDrive;
	Timer0 timer0;
	Timer1 timer1;
	Timer2 timer2;
	joyMemCard* pJoyMemCard;
	biosChip biosChip;

	uint8_t* ram = new uint8_t[RAM_SIZE];
	uint8_t* scratchpad = new uint8_t[SCRATCHPAD_SIZE];

	void cpuRead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles, bool debug = false);
	void cpuWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles);

	void cpuRead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles, bool debug = false);
	void cpuWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles);

	void cpuRead8(const uint32_t& addr, uint8_t& data, uint8_t& cycles, bool debug = false);
	void cpuWrite8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles);

	uint32_t interruptStat = 0;
	uint32_t interruptMask = 0;

	uint32_t cacheControl;
	uint32_t ramSize = 0;
	uint8_t post;

	//debug
	debugUtilities* pDebugger;
};