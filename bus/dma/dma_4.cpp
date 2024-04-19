#include "dma_4.h"
#include "../bus_interface.h"
#include "dma_controller.h"

extern bool g_emulationPaused;

void dma4_t::ReadDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
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

void dma4_t::WriteDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		memoryAddress.data = data;
		std::cout << "~[DMA4] (SPU) madr: 0x" << std::hex << memoryAddress.data << std::endl;
		break;
	case 4:
		blockControl.data = data;
		break;
	case 8:
		channelControl.data = data;

		if ((channelControl.data != 0) &&
			(channelControl.data != 0x01000200) &&
			(channelControl.data != 0x01000201)) {
			std::cout << "[DMA4] EMULATION PAUSED! unhandled sync channel control mode data 0x"
				<< std::hex << channelControl.data << std::endl;
			g_emulationPaused = true;
		}

		if (channelControl.reg.startBusy) {
			bTransferActive = true;
			pDMAController->isCpuStopped = true;
			pDMAController->isCpuStopped = false;
			syncClocks = 0x100;
			if (channelControl.reg.syncMode == syncBlocks) {
				memAddrTemp = memoryAddress.reg.memAddr;
				elementCount = blockControl.reg.ba * blockControl.reg.bc_bs;
				blockWordCount = blockControl.reg.bc_bs;
				std::cout << "~[DMA4] (SPU) element count: 0x"

					<< std::hex << elementCount << std::endl;
				float clks = elementCount * CLOCKS_PER_WORD_DMA_4;
				clocks = round(clks) == clks ? clks : round(clks) + 1;
			}
		}

		if (pDMAController->controlRegister.reg.spu_master_enable && channelControl.reg.startBusy)
			pDMAController->currentClockFunction = 4;

		break;
	}
}

void dma4_t::ReadDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA4] EMULATION PAUSED! unhandled ReadDMA16 0x" << addr << std::endl;
	g_emulationPaused = true;
}

void dma4_t::WriteDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA4] EMULATION PAUSED! unhandled WriteDMA16 0x" << addr << " data 0x" << data << std::endl;
	g_emulationPaused = true;
}

void dma4_t::DMARead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	pBus->cpuRead32(addr, data, cycles);
}

void dma4_t::DMAWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
}

void dma4_t::DMARead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	pBus->cpuRead16(addr, data, cycles);
}

void dma4_t::DMAWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
}

void dma4_t::triggerIRQ() {
	if (pDMAController->interruptRegister.reg.IRQ_enable_dma4)
		pDMAController->interruptRegister.reg.IRQ_flag_dma4 = 1;
	else
		pDMAController->interruptRegister.reg.IRQ_flag_dma4 = 0;

	/*if (pDMAController->interruptRegister.reg.IRQ_master_enable &&
		pDMAController->interruptRegister.reg.IRQ_enable_dma4 &&
		pDMAController->interruptRegister.reg.IRQ_flag_dma4)
		pDMAController->interruptRegister.reg.IRQ_master_flag = 1;
	else
		pDMAController->interruptRegister.reg.IRQ_master_flag = 0;*/
}

void dma4_t::endDMATransfer() {
	bTransferActive = false;
	channelControl.reg.startBusy = 0;
	pDMAController->isCpuStopped = false;

	pDMAController->currentClockFunction = 8;

	triggerIRQ();
}

void dma4_t::Clock() {

	if (pDMAController->isCpuStopped == false) {
		if (syncClocks == 0) {
			pDMAController->isCpuStopped = true;
			channelControl.reg.startBusy = 1;
		}
		else
			syncClocks--;
		return;
	}

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
		uint16_t data;

		if (channelControl.reg.transDir == toMainRAM) {
			DMARead16(0x1f801da8, data, cycles);
			pBus->cpuWrite16(memAddrTemp, data, cycles);
			memAddrTemp += channelControl.reg.memAddrStep ? -2 : +2; //should always be +2 (forward)
			DMARead16(0x1f801da8, data, cycles);
			pBus->cpuWrite16(memAddrTemp, data, cycles);
		}
		else {
			DMARead16(memAddrTemp, data, cycles);
			pBus->cpuWrite16(0x1f801da8, data, cycles);
			memAddrTemp += channelControl.reg.memAddrStep ? -2 : +2; //should always be +2 (forward)
			DMARead16(memAddrTemp, data, cycles);
			pBus->cpuWrite16(0x1f801da8, data, cycles);
		}
		memAddrTemp += channelControl.reg.memAddrStep ? -2 : +2; //should always be +2 (forward)

		/*blockWordCount--;
		if (blockWordCount == 0) {
			channelControl.reg.startBusy = 0;
			blockWordCount = blockControl.reg.bc_bs;
			pDMAController->isCpuStopped = false;
			syncClocks = 0x20;
			memoryAddress.reg.memAddr = memAddrTemp;
		}*/
	}
}