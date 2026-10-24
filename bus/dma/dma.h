#pragma once

#include <cstdint>

class BusInterface;

class Dma {
public:

	virtual void ReadDma32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) = 0;
	virtual void WriteDma32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) = 0;
	virtual void ReadDma16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) = 0;
	virtual void WriteDma16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) = 0;

	virtual void DmaRead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) = 0;
	virtual void DmaWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) = 0;
	virtual void DmaRead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) = 0;
	virtual void DmaWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) = 0;

	virtual void Clock() = 0;

	virtual void TriggerInterrupt() = 0;

	union MemoryAddress {
		uint32_t data;
		struct {
			uint32_t memAddr : 24;
			const uint32_t : 8;
		}reg;
	};

	union BlockControl {
		uint32_t data;
		struct {
			uint32_t bc_bs : 16;
			uint32_t ba : 16;
		}reg;
	};

	enum SyncMode : uint32_t {
		startImm,
		syncBlocks,
		linkedList,
		reserved,
	};

	enum TransDirection : uint32_t {
		toMainRAM,
		fromMainRAM
	};

	union ChannelControl {
		uint32_t data;
		struct {
			TransDirection transDir : 1;
			uint32_t memAddrStep : 1;
			uint32_t : 6;
			uint32_t chopping : 1;
			SyncMode syncMode : 2;
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