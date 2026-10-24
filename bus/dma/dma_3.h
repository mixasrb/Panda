#pragma once

#include "dma.h"

#define CLOCKS_PER_WORD_DMA_3 (p_bus->registerDelays[4].readSequence << 2)

class DmaController;

class Dma3 :
	public Dma
{
public:

	void DMAWrite8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles);
	void DMARead8(const uint32_t& addr, uint8_t& data, uint8_t& cycles);

	void ReadDma32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) override;
	void WriteDma32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) override;
	void ReadDma16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) override;
	void WriteDma16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) override;

	void DmaRead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) override;
	void DmaWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) override;
	void DmaRead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) override;
	void DmaWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) override;

	void Clock() override;

	void TriggerInterrupt() override;

	BusInterface* p_bus;

	DmaController* pDMAController;

	MemoryAddress memoryAddress;
	BlockControl blockControl;
	ChannelControl channelControl;

	bool bStart;
	uint64_t clocks;
	uint32_t choppingClocks;
	uint32_t memAddrTemp;
};