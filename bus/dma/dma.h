#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>

#include "../../defines/defines.h"
#include "../../debug_utilities/debug_utilities.h"

class busInterface;

class dma {
public:

	virtual void ReadDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) = 0;
	virtual void WriteDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) = 0;
	virtual void ReadDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) = 0;
	virtual void WriteDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) = 0;

	virtual void DMARead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) = 0;
	virtual void DMAWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) = 0;
	virtual void DMARead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) = 0;
	virtual void DMAWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) = 0;

	virtual void triggerIRQ() = 0;

	virtual void Clock() = 0;

	union memoryAddress_t {
		uint32_t data;
		struct {
			uint32_t memAddr : 24;
			const uint32_t : 8;
		}reg;
	};

	union blockControl_t {
		uint32_t data;
		struct {
			uint32_t bc_bs : 16;
			uint32_t ba : 16;
		}reg;
	};

	enum syncMode_t : uint32_t {
		startImm,
		syncBlocks,
		linkedList,
		reserved,
	};

	enum transDir_t : uint32_t {
		toMainRAM,
		fromMainRAM
	};

	union channelControl_t {
		uint32_t data;
		struct {
			transDir_t transDir : 1;
			uint32_t memAddrStep : 1;
			uint32_t : 6;
			uint32_t chopping : 1;
			syncMode_t syncMode : 2;
			uint32_t : 5;
			uint32_t choppingDMASize : 3;
			uint32_t : 1;
			uint32_t choppingCPUSize : 3;
			uint32_t : 1;
			uint32_t startBusy : 1;
			uint32_t : 3;
			uint32_t startTrigger : 1;
			uint32_t pause : 1;
			uint32_t unknown : 1;
			uint32_t : 1;
		}reg;
	};
};