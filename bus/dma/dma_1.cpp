#include "dma_1.h"
#include "../bus_interface.h"
#include "dma_controller.h"

extern bool g_emulationPaused;

void dma1_t::ReadDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
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

void dma1_t::WriteDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		memoryAddress.data = data;
		std::cout << "~[DMA1] (MDECout) madr: 0x" << std::hex << memoryAddress.data << std::endl;
		break;
	case 4:
		blockControl.data = data;
		break;
	case 8:
		channelControl.data = data;

		if ((channelControl.data != 0) &&
			(channelControl.data != 0x00000200) &&
			(channelControl.data != 0x01000200)) {
			std::cout << "[DMA1] EMULATION PAUSED! unhandled sync channel control mode data 0x"
				<< std::hex << channelControl.data << std::endl;
			g_emulationPaused = true;
		}

		if (channelControl.reg.startBusy) {
			pDMAController->isCpuStopped = true;
			if (channelControl.reg.syncMode == syncBlocks) {
				memAddrTemp = memoryAddress.reg.memAddr;
				elementCount = blockControl.reg.ba * blockControl.reg.bc_bs;
				std::cout << "~[DMA1] (MDECout) element count: 0x"
					<< std::hex << elementCount << std::endl;
				float clks = elementCount * CLOCKS_PER_WORD_DMA_1;
				clocks = round(clks) == clks ? clks : round(clks) + 1;
			}
		}
		break;
	}
}

void dma1_t::ReadDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA1] EMULATION PAUSED! unhandled ReadDMA16 0x" << addr << std::endl;
	g_emulationPaused = true;
}

void dma1_t::WriteDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA1] EMULATION PAUSED! unhandled WriteDMA16 0x" << addr << " data 0x" << data << std::endl;
	g_emulationPaused = true;
}

void dma1_t::DMARead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	pBus->cpuRead32(addr, data, cycles);
}

void dma1_t::DMAWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
}

void dma1_t::DMARead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
}

void dma1_t::DMAWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
}

void dma1_t::triggerIRQ() {
	if (pDMAController->interruptRegister.reg.IRQ_enable_dma1)
		pDMAController->interruptRegister.reg.IRQ_flag_dma1 = 1;
	else
		pDMAController->interruptRegister.reg.IRQ_flag_dma1 = 0;

	//if (pDMAController->interruptRegister.reg.IRQ_master_enable &&
	//	pDMAController->interruptRegister.reg.IRQ_enable_dma1 &&
	//	pDMAController->interruptRegister.reg.IRQ_flag_dma1)
	//	pDMAController->interruptRegister.reg.IRQ_master_flag = 1;
	//else
	//	pDMAController->interruptRegister.reg.IRQ_master_flag = 0;
}

void dma1_t::endDMATransfer() {
	channelControl.reg.startBusy = 0;
	pDMAController->isCpuStopped = false;

	triggerIRQ();
}

void dma1_t::Clock() {

	if (clocks == 0) {
		if (channelControl.reg.syncMode == syncBlocks) {
			memoryAddress.reg.memAddr = memAddrTemp;
			endDMATransfer();
			return;
		}
	}

	clocks--;
	if (clocks < elementCount) {
		uint8_t cycles;
		uint32_t data;

		if (channelControl.reg.transDir == toMainRAM) {
			DMARead32(0x1f801820, data, cycles);
			pBus->cpuWrite32(memAddrTemp, data, cycles);
		}
		else {
			//Unhandled
		}
		memAddrTemp += channelControl.reg.memAddrStep ? -4 : +4; //should always be +4 (forward)
	}
}