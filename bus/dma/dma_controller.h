#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>

#include "../../debug_utilities/debug_utilities.h""
#include "dma.h"
#include "dma_2.h"
#include "dma_3.h"
#include "dma_6.h"

class busInterface;

class dmaController {
public:

	dmaController();

	busInterface* pbus;

	dma2 dma_2;
	dma3 dma_3;
	dma6 dma_6;

	dma* ppDMA[7];

	void ReadDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles);
	void WriteDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles);
	void ReadDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles);
	void WriteDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles);

	void ConnectBus(busInterface* p);

	void clock();

	union controlRegister_t
	{
		uint32_t data;
		struct
		{
			uint32_t : 8;
			uint32_t gpu_priority : 3;
			uint32_t gpu_master_enable : 1;
			uint32_t cd_drive_priority : 3;
			uint32_t cd_drive_master_enable : 1;
			uint32_t : 8;
			uint32_t otc_priority : 3;
			uint32_t otc_master_enable : 1;
			uint32_t : 4;
		}reg;
	}dpcr;

	union interruptRegister_t
	{
		uint32_t data;
		struct
		{
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
	}dicr;

	//uint32_t wait;
	//bool irq = false;
	//debug
	debugUtilities* p_debugger;
};
