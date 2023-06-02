#pragma once

#include "dma.h"

#define CLOCKS_PER_WORD_DMA_6 272.f/256.f;

class dmaController;

class dma6 :
	public dma {
public:
	void ReadDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) override;
	void WriteDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) override;
	void ReadDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) override;
	void WriteDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) override;

	void DMARead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) override;
	void DMAWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) override;
	void DMARead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) override;
	void DMAWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) override;

	void triggerIRQ() override;

	void Clock() override;

	busInterface* pbus;

	dmaController* pDMAController;

	memoryAddress_t memoryAddress;
	blockControl_t blockControl;
	channelControl_t channelControl;

	bool bStart;
	uint64_t clocks;
	uint32_t memAddrTemp;
};

