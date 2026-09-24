#pragma once

#include "dma.h"

#define CLOCKS_PER_WORD_DMA_3 (p_bus->registerDelays[4].readSequence << 2)

class dmaController_t;

class dma3_t :
	public dma_t
{
public:

	void DMAWrite8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles);
	void DMARead8(const uint32_t& addr, uint8_t& data, uint8_t& cycles);

	void ReadDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) override;
	void WriteDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) override;
	void ReadDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) override;
	void WriteDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) override;

	void DMARead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) override;
	void DMAWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) override;
	void DMARead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) override;
	void DMAWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) override;

	void Clock() override;

	void triggerIRQ() override;

	BusInterface* p_bus;

	dmaController_t* pDMAController;

	memoryAddress_t memoryAddress;
	blockControl_t blockControl;
	channelControl_t channelControl;

	bool bStart;
	uint64_t clocks;
	uint32_t choppingClocks;
	uint32_t memAddrTemp;
};