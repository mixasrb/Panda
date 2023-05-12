#include "dma_3.h"
#include "../bus_interface.h"
#include "dma_controller.h"

extern bool isCpuStopped;
extern bool isEmulationPaused;

void dma3::ReadDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
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

void dma3::WriteDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		memoryAddress.data = data;
		std::cout << "~[DMA3] madr: 0x" << std::hex << memoryAddress.data << std::endl;
		if (data == 0)
			isEmulationPaused = true;
		break;
	case 4:
		blockControl.data = data;
		break;
	case 8:
		channelControl.data = data;

		if ((channelControl.data != 0x11000000) &&
			(channelControl.data != 0)) {
			std::cout << "[DMA3] EMULATION PAUSED! unhandled sync channel control mode data 0x"
				<< std::hex << channelControl.data << std::endl;
			isEmulationPaused = true;
		}

		if (channelControl.reg.startBusy && channelControl.reg.startTrigger) {
			bStart = true;
			float fcloks = blockControl.reg.bc_bs * clks_per_word;
			clocks = round(fcloks) == fcloks ? fcloks : round(fcloks) + 1;
			channelControl.reg.startTrigger = 0;
			memAddrTemp = memoryAddress.reg.memAddr;
			isCpuStopped = true;
		}

		break;
	}
}

void dma3::ReadDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA3] EMULATION PAUSED! unhandled ReadDMA16 0x" << addr << std::endl;
	isEmulationPaused = true;
}

void dma3::WriteDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA3] EMULATION PAUSED! unhandled WriteDMA16 0x" << addr << "data 0x" << data << std::endl;
	isEmulationPaused = true;
}

void dma3::DMARead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
}

void dma3::DMAWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	p_bus->cpuWrite32(addr, data, cycles);
}

void dma3::DMAWrite8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles) {
	p_bus->cpuWrite8(addr, data, cycles);
}

void dma3::DMARead8(const uint32_t& addr, uint8_t& data, uint8_t& cycles) {
	p_bus->cpuRead8(addr, data, cycles);
}

void dma3::DMARead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
}

void dma3::DMAWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
}

void dma3::triggerIRQ() {
	if (p_dma->interruptRegister.reg.IRQ_enable_dma3)
		p_dma->interruptRegister.reg.IRQ_flag_dma3 = 1;
	else
		p_dma->interruptRegister.reg.IRQ_flag_dma3 = 0;

	if (p_dma->interruptRegister.reg.IRQ_master_enable &&
		p_dma->interruptRegister.reg.IRQ_enable_dma3 &&
		p_dma->interruptRegister.reg.IRQ_flag_dma3)
		p_dma->interruptRegister.reg.IRQ_master_flag = 1;
	else
		p_dma->interruptRegister.reg.IRQ_master_flag = 0;
}

void dma3::Clock() {
	if (clocks == 0) {
		channelControl.reg.startBusy = 0;
		bStart = false;
		isCpuStopped = false;

		triggerIRQ();
	}

	clocks--;
	if (clocks < blockControl.reg.bc_bs) {
		for (int i = 0; i < 4; i++) {
			uint8_t cycles;
			uint8_t data;
			uint32_t cdDriveDataFifoAddr = 0x1f801802;
			DMARead8(cdDriveDataFifoAddr, data, cycles);
			DMAWrite8(memAddrTemp + i, data, cycles);
		}
		memAddrTemp += channelControl.reg.memAddrStep ? -4 : +4; //should always be +4 (forward)
	}
}