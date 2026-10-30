#include "dma_3.h"
#include "../bus_interface.h"
#include "dma_controller.h"

extern bool g_isEmulationPaused;

void Dma3::ReadDma32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
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

void Dma3::WriteDma32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		memoryAddress.data = data;
		std::cout << "~[DMA3] (CD DRIVE) madr: 0x" << std::hex << memoryAddress.data << '\n';
		if (data == 0)
			g_isEmulationPaused = true;
		break;
	case 4:
		blockControl.data = data;
		std::cout << "~[DMA3] (CD DRIVE) number of words: 0x" << std::hex << blockControl.reg.bc_bs << '\n';
		break;
	case 8:
		channelControl.data = data;

		if ((channelControl.data != 0x11000000) &&
			(channelControl.data != 0) &&
			(channelControl.data != 0x11400100)) {
			std::cout << "[DMA3] EMULATION PAUSED! unhandled sync channel control mode data 0x"
				<< std::hex << channelControl.data << '\n';
			g_isEmulationPaused = true;
		}

		if (channelControl.reg.startBusy && channelControl.reg.startTrigger) {
			std::cout << "~[DMA3] (CD DRIVE) started"<< '\n';
			bStart = true;
			//float fcloks = blockControl.reg.bc_bs * CLOCKS_PER_WORD_DMA_3;
			//clocks = round(fcloks) == fcloks ? fcloks : round(fcloks) + 1;
			clocks = blockControl.reg.bc_bs * CLOCKS_PER_WORD_DMA_3;
			channelControl.reg.startTrigger = 0;
			memAddrTemp = memoryAddress.reg.memAddr;
			pDMAController->isCpuStopped = true;
		}

		if (pDMAController->controlRegister.reg.cd_drive_master_enable && bStart)
			pDMAController->currentClockFunction = 3;

		break;
	}
}

void Dma3::ReadDma16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA3] EMULATION PAUSED! unhandled ReadDMA16 0x" << addr << '\n';
	g_isEmulationPaused = true;
}

void Dma3::WriteDma16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA3] EMULATION PAUSED! unhandled WriteDMA16 0x" << addr << "data 0x" << data << '\n';
	g_isEmulationPaused = true;
}

void Dma3::DmaRead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
}

void Dma3::DmaWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	p_bus->Write32(addr, data, cycles);
}

void Dma3::DMAWrite8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles) {
	p_bus->Write8(addr, data, cycles);
}

void Dma3::DMARead8(const uint32_t& addr, uint8_t& data, uint8_t& cycles) {
	p_bus->Read8(addr, data, cycles);
}

void Dma3::DmaRead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
}

void Dma3::DmaWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
}

void Dma3::TriggerInterrupt() {
	if (pDMAController->interruptRegister.reg.IRQ_enable_dma3)
		pDMAController->interruptRegister.reg.IRQ_flag_dma3 = 1;
	else
		pDMAController->interruptRegister.reg.IRQ_flag_dma3 = 0;

	//if (pDMAController->interruptRegister.reg.IRQ_master_enable &&
	//	pDMAController->interruptRegister.reg.IRQ_enable_dma3 &&
	//	pDMAController->interruptRegister.reg.IRQ_flag_dma3)
	//	pDMAController->interruptRegister.reg.IRQ_master_flag = 1;
	//else
	//	pDMAController->interruptRegister.reg.IRQ_master_flag = 0;

	pDMAController->currentClockFunction = 8;
}

void Dma3::Clock() {
	if (clocks == 0) {
		std::cout << "~[DMA3] (CD DRIVE) finished" << '\n';
		channelControl.reg.startBusy = 0;
		choppingClocks = 0;
		bStart = false;
		pDMAController->isCpuStopped = false;

		TriggerInterrupt();
		return;
	}

	if (choppingClocks) {
		//pDMAController->isCpuStopped = false;//?? makes graphic bugs in gta 
		choppingClocks--;
		return;
	}

	channelControl.reg.startBusy = 1;
	clocks--;
	if (clocks % CLOCKS_PER_WORD_DMA_3 == 0) {
		//pDMAController->isCpuStopped = true;//?? makes graphic bugs in gta 
		for (int i = 0; i < 4; i++) {
			uint8_t cycles;
			uint8_t data;
			uint32_t cdDriveDataFifoAddr = 0x1f801802;
			DMARead8(cdDriveDataFifoAddr, data, cycles);
			DMAWrite8(memAddrTemp + i, data, cycles);
		}
		memAddrTemp += channelControl.reg.memAddrStep ? -4 : +4; //should always be +4 (forward) for dma3

		if (channelControl.reg.chopping) {
			channelControl.reg.startBusy = 0;
			//assumed that chopping DMA window is 1 word
			blockControl.reg.bc_bs--;
			memoryAddress.reg.memAddr += channelControl.reg.memAddrStep ? -4 : +4; //should always be +4 (forward) for dma3
			choppingClocks = 1 << channelControl.reg.choppingCPUSize;
		}
	}
}