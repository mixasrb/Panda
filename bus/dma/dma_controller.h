#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>

#include "dma.h"
#include "dma_0.h"
#include "dma_1.h"
#include "dma_2.h"
#include "dma_3.h"
#include "dma_4.h"
#include "dma_6.h"

#include "../../debug_utilities/debug_utilities.h"

class busInterface_t;

class dmaController_t {
public:

	dmaController_t();

	busInterface_t* pbus = nullptr;

	void readDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles);
	void writeDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles);
	void readDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles);
	void writeDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles);
	void readDMA8(const uint32_t& addr, uint8_t& data, uint8_t& cycles);
	void writeDMA8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles);

	void connectBus(busInterface_t* p);

	void clock();

	dma_t* ppDMA[7];

	dma0_t dma0;
	dma1_t dma1;
	dma2_t dma2;
	dma3_t dma3;
	dma4_t dma4;
	dma6_t dma6;

	bool clock0();
	bool clock1();
	bool clock2();
	bool clock3();
	bool clock4();
	bool clock5();
	bool clock6();

	bool (dmaController_t::* clockFunctionsArray[7])();
	bool bmasterEnable = false;

	union controlRegister_t {
		uint32_t data;
		struct {
			uint32_t mdecIn_priority : 3;
			uint32_t mdecIn_master_enable : 1;
			uint32_t mdecOut_priority : 3;
			uint32_t mdecOut_master_enable : 1;
			uint32_t gpu_priority : 3;
			uint32_t gpu_master_enable : 1;
			uint32_t cd_drive_priority : 3;
			uint32_t cd_drive_master_enable : 1;
			uint32_t spu_priority : 3;
			uint32_t spu_master_enable : 1;
			uint32_t : 4;
			uint32_t otc_priority : 3;
			uint32_t otc_master_enable : 1;
			uint32_t : 4;
		}reg;
	}controlRegister;

	union interruptRegister_t {
		uint32_t data;
		struct {
			uint32_t : 6;
			uint32_t : 9;
			uint32_t force_IRQ : 1;
			uint32_t IRQ_enable_dma0 : 1;
			uint32_t IRQ_enable_dma1 : 1;
			uint32_t IRQ_enable_dma2 : 1;
			uint32_t IRQ_enable_dma3 : 1;
			uint32_t IRQ_enable_dma4 : 1;
			uint32_t IRQ_enable_dma5 : 1;
			uint32_t IRQ_enable_dma6 : 1;
			uint32_t IRQ_master_enable : 1;
			uint32_t IRQ_flag_dma0 : 1;
			uint32_t IRQ_flag_dma1 : 1;
			uint32_t IRQ_flag_dma2 : 1;
			uint32_t IRQ_flag_dma3 : 1;
			uint32_t IRQ_flag_dma4 : 1;
			uint32_t IRQ_flag_dma5 : 1;
			uint32_t IRQ_flag_dma6 : 1;
			uint32_t IRQ_master_flag : 1;
		}reg;
	}interruptRegister;

	bool isCpuStopped = false;

	//debug
	debugUtilities* p_debugger = nullptr;
};
