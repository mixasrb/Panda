#pragma once

#include "dma.h"

#define CLOCKS_PER_WORD_DMA_6 272.f/256.f

class DmaController;

class Dma6 :
	public Dma {
public:
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

	BusInterface* pbus;

	DmaController* pDMAController;

	MemoryAddress memoryAddress;
	BlockControl blockControl;
	ChannelControl channelControl;

	bool bStart;
	uint64_t clocks;
	uint32_t memAddrTemp;
};

