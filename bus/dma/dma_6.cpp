#include "dma_6.h"
#include "../bus_interface.h"

extern bool isCpuStopped;
extern bool isEmulationPaused;

void dma6::ReadDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		data = memoryAddress.data;
		break;
	case 4:
		data = blockControl.data;
		break;
	case 8:
		data = channelControl.data;
		break;
	}
}

void dma6::WriteDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		memoryAddress.data = data;
		break;
	case 4:
		blockControl.data = data;
		break;
	case 8:
		channelControl.data = data;

		if ((channelControl.data != 0x11000002) &&
			(channelControl.data != 0)) {
			std::cout << "[DMA6] EMULATION PAUSED! unhandled sync channel control mode data 0x"
				<< std::hex << channelControl.data << std::endl;
			isEmulationPaused = true;
		}

		if (channelControl.reg.startBusy && channelControl.reg.startTrigger) {
			bStart = true;
			float clks = blockControl.reg.bc_bs * clks_per_word;
			clocks = round(clks) == clks ? clks : round(clks) + 1;
			channelControl.reg.startTrigger = 0;
			memAddrTemp = memoryAddress.reg.memAddr;
			isCpuStopped = true;
		}
		break;
	}
}

void dma6::ReadDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA6] EMULATION PAUSED! unhandled ReadDMA16 0x" << addr << std::endl;
	isEmulationPaused = true;
}

void dma6::WriteDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA6] EMULATION PAUSED! unhandled WriteDMA16 0x" << addr << "data 0x" << data << std::endl;
	isEmulationPaused = true;
}

void dma6::DMARead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
}

void dma6::DMAWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	pbus->cpuWrite32(addr, data, cycles);
}

void dma6::DMARead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
}

void dma6::DMAWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
}

void dma6::triggerIRQ() {
	if (p_dma->dicr.reg.IRQ_enable_dma6)
		p_dma->dicr.reg.IRQ_flag_dma6 = 1;
	else
		p_dma->dicr.reg.IRQ_flag_dma6 = 0;

	if (p_dma->dicr.reg.IRQ_master_enable &&
		p_dma->dicr.reg.IRQ_enable_dma6 &&
		p_dma->dicr.reg.IRQ_flag_dma6)
		p_dma->dicr.reg.IRQ_master_flag = 1;
	else
		p_dma->dicr.reg.IRQ_master_flag = 0;
}

void dma6::Clock() {
	if (clocks == 0) {
		channelControl.reg.startBusy = 0;
		bStart = false;
		isCpuStopped = false;

		triggerIRQ();
	}

	clocks--;
	if (clocks < blockControl.reg.bc_bs) {
		uint8_t cycles;
		if (clocks == 0) {
			DMAWrite32(memAddrTemp, 0x00ffffff, cycles);
			return;
		}

		DMAWrite32(memAddrTemp, memAddrTemp - 4, cycles);
		memAddrTemp += channelControl.reg.memAddrStep ? -4 : +4; //should always be -4 (backward)
	}
}
