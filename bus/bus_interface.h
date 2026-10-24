#pragma once

#include <cstdint>
#include <memory>

#include "../cpu/cw33300.h"
#include "../bus/dma/dma_controller.h"
#include "../mdec/mdec.h"
#include "../cd_drive/hc05_pux.h"
#include "../gpu/cxdxxxxx.h"
#include "../bios_chip/bios_chip.h"
#include "../timers/timer_0.h"
#include "../timers/timer_1.h"
#include "../timers/timer_2.h"
#include "../peripherals/joy_mem_card.h"
#include "../spu/spu.h"

#include "../defines/defines.h"

#include "../debug_utilities/debug_utilities.h"

#define RAM_SIZE  4 * 2 * 1024 * 1024 / 4
#define SCRATCHPAD_SIZE  1024

class BusInterface {
public:

	void Read32(const uint32_t& addr, uint32_t& data, uint8_t& cycles,const bool& debug = false);
	void Write32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles);

	void Read16(const uint32_t& addr, uint16_t& data, uint8_t& cycles,const bool& debug = false);
	void Write16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles);

	void Read8(const uint32_t& addr, uint8_t& data, uint8_t& cycles,const bool& debug = false);
	void Write8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles);

	//cw33300
	Cpu* pCpu = nullptr;

	DmaController dma;

	Mdec* pMdec = nullptr;

	//cxd85xxx
	Gpu* pGpu = nullptr;

	//hc05_pux
	cdDrive* pCdDrive = nullptr;

	timer0_t timer0;
	timer1_t timer1;
	timer2_t timer2;

	joyMemCard_t* pJoyMemCard = nullptr;

	Spu spu;

	biosChip_t biosChip;

	std::unique_ptr<uint8_t[]> ram = std::make_unique<uint8_t[]>(RAM_SIZE);
	std::unique_ptr<uint8_t[]> scratchpad = std::make_unique<uint8_t[]>(SCRATCHPAD_SIZE);

	uint32_t interruptStat = 0;
	uint32_t interruptMask = 0;

	uint32_t cacheControl;
	uint32_t ramSize = 0;
	uint8_t post = 0;

	union delayRegisterType {
		uint32_t data;
		struct {
			uint32_t writeDelay : 4;
			uint32_t readDelay : 4;
			uint32_t useCOM0 : 1;
			uint32_t useCOM1 : 1;
			uint32_t useCOM2 : 1;
			uint32_t useCOM3 : 1;
			uint32_t dataBusWidth : 1;
			uint32_t autoIncrement : 1;
			uint32_t : 2;
			uint32_t memoryWindowSize : 5;
			uint32_t : 3;
			uint32_t timingOverrideDMA : 4;
			uint32_t addressErrorFlag : 1;
			uint32_t timingSelectDMA : 1;
			uint32_t wideDMA : 1;
			uint32_t wait : 1;
		}elem;
	};

	delayRegisterType delayRegisters[6];

	union commonDelayRegisterType {
		uint32_t data;
		struct {
			uint32_t com0 : 4;
			uint32_t com1 : 4;
			uint32_t com2 : 4;
			uint32_t com3 : 4;
			uint32_t : 16;
		}elem;
	}commonDelayRegister;

	struct delayType {
		uint8_t readFirst;
		uint8_t readSequence;
		uint8_t writeFirst;
		uint8_t writeSequence;
	};

	delayType registerDelays[6];

	delayType calculateDelays(const delayRegisterType& delayRegister);

	//debug
	DebugUtilities* pDebugger = nullptr;
};