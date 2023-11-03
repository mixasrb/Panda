#pragma once

#include <cstdint>
#include <stdio.h>

#include "../cpu/cw33300.h"
#include "../bus/dma/dma_controller.h"
#include "../mdec/mdec.h"
#include "../cd_drive/hc05_pux.h"
#include "../gpu/cxd85xxx.h"
#include "../bios_chip/bios_chip.h"
#include "../timers/timer.h"
#include "../timers/timer_0.h"
#include "../timers/timer_1.h"
#include "../timers/timer_2.h"
#include "../peripherals/joy_mem_card.h"
#include "../spu/spu.h"

#include "../defines/defines.h"

#include "../debug_utilities/debug_utilities.h"

#define RAM_SIZE  4 * 2 * 1024 * 1024 / 4
#define SCRATCHPAD_SIZE  1024

class busInterface_t {
public:

	void cpuRead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles, bool debug = false);
	void cpuWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles);

	void cpuRead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles, bool debug = false);
	void cpuWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles);

	void cpuRead8(const uint32_t& addr, uint8_t& data, uint8_t& cycles, bool debug = false);
	void cpuWrite8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles);

	//cw33300
	cpu_t* pCpu = nullptr;

	dmaController_t dma;

	mdec_t* pMdec = nullptr;

	//cxd85xxx
	gpu_t* pGpu = nullptr;

	//hc05_pux
	cdDrive* pCdDrive = nullptr;

	timer0_t timer0;
	timer1_t timer1;
	timer2_t timer2;

	joyMemCard_t* pJoyMemCard = nullptr;

	spu_t spu;

	biosChip_t biosChip;

	uint8_t* ram = new uint8_t[RAM_SIZE];
	uint8_t* scratchpad = new uint8_t[SCRATCHPAD_SIZE];

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

	//spu

	union spuControlRegister_t {
		uint16_t data;
		struct {
			uint16_t cdAudioEnable : 1;
			uint16_t externalAudioEnable : 1;
			uint16_t cdAudioReverb : 1;
			uint16_t externalAudioReverb : 1;
			uint16_t soundRAMTransferMode : 2;
			uint16_t irq9Enable : 1;
			uint16_t reverbMasterEnable : 1;
			uint16_t noiseFrequencyStep : 2;
			uint16_t noiseFrequencyShift : 4;
			uint16_t muteSPU : 1;
			uint16_t spuEnable : 1;
		}elem;
	}spuControlRegister;

	union spuStatusRegister_t {
		uint16_t data;
		struct {
			uint16_t currentSPUMode : 6;
			uint16_t irq9Flag : 1;
			uint16_t dataTransferDMAReadWriteRequest : 1;
			uint16_t dataTransferDMAWriteRequest : 1;
			uint16_t dataTransferDMAReadRequest : 1;
			uint16_t dataTransferBusyFlag : 1;
			uint16_t writingFirstSecondHalfCaptureBuffers : 1;
			uint16_t unused : 4;
		}elem;
	}spuStatusRegister;

	uint16_t soundRAM_IRQAddress;

	//debug
	debugUtilities* pDebugger = nullptr;
};