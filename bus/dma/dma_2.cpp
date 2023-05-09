#include "dma_2.h"
#include "../bus_interface.h"
#include "dma_controller.h"

extern bool isCpuStopped;
extern bool isEmulationPaused;

void dma2::ReadDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		data = memoryAddress.reg.memAddr;
		break;
	case 4:
		data = blockControl.data;
		break;
	case 8:
		data = channelControl.data;
		break;
	}
}

void dma2::WriteDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		memoryAddress.data = data;
		break;
	case 4:
		blockControl.data = data;
		break;
	case 8:
		channelControl.data = data;

		if ((channelControl.data != 0x01000200) &&
			(channelControl.data != 0x01000201) &&
			(channelControl.data != 0x00000401) && //??? maybe some initializing
			(channelControl.data != 0x01000401)) {
			std::cout << "[DMA2] EMULATION PAUSED! unhandled sync channel control mode data 0x"
				<< std::hex << channelControl.data << std::endl;
			isEmulationPaused = true;
		}

		if (channelControl.reg.startBusy) {
			isCpuStopped = true;
			if (channelControl.reg.syncMode == syncBlocks) {
				memAddrTemp = memoryAddress.reg.memAddr;
				elementCount = blockControl.reg.ba * blockControl.reg.bc_bs;
				float clks = elementCount * clks_per_word;
				clocks = round(clks) == clks ? clks : round(clks) + 1;
			}
		}
		break;
	}
}

void dma2::ReadDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA2] EMULATION PAUSED! unhandled ReadDMA16 0x" << addr << std::endl;
	isEmulationPaused = true;
}

void dma2::WriteDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA2] EMULATION PAUSED! unhandled WriteDMA16 0x" << addr << " data 0x" << data << std::endl;
	isEmulationPaused = true;
}

void dma2::DMARead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	pBus->cpuRead32(addr, data, cycles);
}

void dma2::DMAWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
}

void dma2::DMARead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
}

void dma2::DMAWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
}

void dma2::triggerIRQ() {
	if (pDMAController->dicr.reg.IRQ_enable_dma2)
		pDMAController->dicr.reg.IRQ_flag_dma2 = 1;
	else
		pDMAController->dicr.reg.IRQ_flag_dma2 = 0;

	if (pDMAController->dicr.reg.IRQ_master_enable &&
		pDMAController->dicr.reg.IRQ_enable_dma2 &&
		pDMAController->dicr.reg.IRQ_flag_dma2)
		pDMAController->dicr.reg.IRQ_master_flag = 1;
	else
		pDMAController->dicr.reg.IRQ_master_flag = 0;
}

void dma2::endDMATransfer() {
	channelControl.reg.startBusy = 0;
	isCpuStopped = false;

	triggerIRQ();
}

void dma2::Clock() {

	if (clocks == 0) {
		if ((channelControl.reg.syncMode == syncBlocks) ||
			((channelControl.reg.syncMode == linkedList) && (memoryAddress.data & 0x00800000))) {
			endDMATransfer();
			return;
		}

		if (channelControl.reg.syncMode == linkedList) {
			memAddrTemp = memoryAddress.reg.memAddr;
			uint8_t cycles;
			uint32_t node;
			DMARead32(memAddrTemp, node, cycles);
			memAddrTemp += 4;
			memoryAddress.data = node & 0x00ffffff;
			elementCount = node >> 24;
			float clks = elementCount * clks_per_word;
			clocks = round(clks) == clks ? clks : round(clks) + 1;
			return;
		}
	}

	clocks--;
	if (clocks < elementCount) {
		uint8_t cycles;
		uint32_t data;

		if (channelControl.reg.transDir == toMainRAM) {
			DMARead32(0x1f801810, data, cycles);
			pBus->cpuWrite32(memAddrTemp, data, cycles);
		}
		else {
			DMARead32(memAddrTemp, data, cycles);
			pBus->cpuWrite32(0x1f801810, data, cycles);
		}
		memAddrTemp += channelControl.reg.memAddrStep ? -4 : +4; //should always be +4 (forward)
	}
}