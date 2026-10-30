#include "dma_6.h"
#include "../bus_interface.h"

extern bool g_isEmulationPaused;

void Dma6::ReadDma32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
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

void Dma6::WriteDma32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		memoryAddress.data = data;
		//std::cout << "~[DMA6] (OTC) madr: 0x" << std::hex << memoryAddress.data << '\n';
		break;
	case 4:
		blockControl.data = data;
		//std::cout << "~[DMA6] (OTC) BC/BS : 0x" << std::hex << blockControl.reg.bc_bs << '\n';
		break;
	case 8:
		channelControl.data = data;

		if ((channelControl.data != 0x11000002) &&
			(channelControl.data != 0)) {
			std::cout << "[DMA6] EMULATION PAUSED! unhandled sync channel control mode data 0x"
				<< std::hex << channelControl.data << '\n';
			g_isEmulationPaused = true;
		}

		if (channelControl.reg.startBusy && channelControl.reg.startTrigger) {
			//std::cout << "~[DMA6] (OTC) started" << '\n';
			bStart = true;
			float clks = (blockControl.reg.bc_bs ? blockControl.reg.bc_bs : 0x10000) * CLOCKS_PER_WORD_DMA_6;
			clocks = round(clks) == clks ? clks : round(clks) + 1;
			channelControl.reg.startTrigger = 0;
			memAddrTemp = memoryAddress.reg.memAddr;
			pDMAController->isCpuStopped = true;
		}

		if (pDMAController->controlRegister.reg.otc_master_enable && bStart)
			pDMAController->currentClockFunction = 6;

		break;
	}
}

void Dma6::ReadDma16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA6] EMULATION PAUSED! unhandled ReadDMA16 0x" << addr << '\n';
	g_isEmulationPaused = true;
}

void Dma6::WriteDma16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA6] EMULATION PAUSED! unhandled WriteDMA16 0x" << addr << "data 0x" << data << '\n';
	g_isEmulationPaused = true;
}

void Dma6::DmaRead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
}

void Dma6::DmaWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	pbus->Write32(addr, data, cycles);
}

void Dma6::DmaRead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
}

void Dma6::DmaWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
}

void Dma6::TriggerInterrupt() {
	if (pDMAController->interruptRegister.reg.IRQ_enable_dma6)
		pDMAController->interruptRegister.reg.IRQ_flag_dma6 = 1;
	else
		pDMAController->interruptRegister.reg.IRQ_flag_dma6 = 0;

	/*if (pDMAController->interruptRegister.reg.IRQ_master_enable &&
		pDMAController->interruptRegister.reg.IRQ_enable_dma6 &&
		pDMAController->interruptRegister.reg.IRQ_flag_dma6)
		pDMAController->interruptRegister.reg.IRQ_master_flag = 1;
	else
		pDMAController->interruptRegister.reg.IRQ_master_flag = 0;*/

	pDMAController->currentClockFunction = 8;
}

void Dma6::Clock() {
	if (clocks == 0) {
		channelControl.reg.startBusy = 0;
		bStart = false;
		pDMAController->isCpuStopped = false;

		TriggerInterrupt();
	}

	clocks--;
	if (clocks < blockControl.reg.bc_bs) {
		uint8_t cycles;
		if (clocks == 0) {
			//std::cout << "~[DMA6] (OTC) finished" << '\n';
			DmaWrite32(memAddrTemp, 0x00ffffff, cycles);
			return;
		}

		DmaWrite32(memAddrTemp, memAddrTemp - 4, cycles);
		memAddrTemp += channelControl.reg.memAddrStep ? -4 : +4; //should always be -4 (backward)
	}
}
