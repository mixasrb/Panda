#include "dma_1.h"
#include "../bus_interface.h"
#include "dma_controller.h"

extern bool g_emulationPaused;

void Dma1::ReadDma32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		data = memoryAddress.reg.memAddr;
		break;
	case 4:
		data = blockControl.data;
		break;
	case 8:
		data = channelControl.data;
		//data |= 0x1000000; // Shadow Madness (USA) hack
		break;
	}
}

void Dma1::WriteDma32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		memoryAddress.data = data;
		std::cout << "~[DMA1] (MDECout) madr: 0x" << std::hex << memoryAddress.data << '\n';
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
				<< std::hex << channelControl.data << '\n';
			g_emulationPaused = true;
		}

		if (channelControl.reg.startBusy) {
			bTransferActive = true;
			pDMAController->isCpuStopped = true;
			/*pDMAController->isCpuStopped = false;
			syncClocks = 0x100;*/
			if (channelControl.reg.syncMode == syncBlocks) {
				memAddrTemp = memoryAddress.reg.memAddr;
				elementCount = blockControl.reg.ba * blockControl.reg.bc_bs;
				blockWordCount = blockControl.reg.bc_bs;
				std::cout << "~[DMA1] Start (MDECout) element count: 0x"

					<< std::hex << elementCount << '\n';
				float clks = elementCount * CLOCKS_PER_WORD_DMA_1;
				clocks = round(clks) == clks ? clks : round(clks) + 1;
			}
		}

		if (pDMAController->controlRegister.reg.mdecOut_master_enable && channelControl.reg.startBusy)
			pDMAController->currentClockFunction = 1;

		break;
	}
}

void Dma1::ReadDma16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA1] EMULATION PAUSED! unhandled ReadDMA16 0x" << addr << '\n';
	g_emulationPaused = true;
}

void Dma1::WriteDma16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA1] EMULATION PAUSED! unhandled WriteDMA16 0x" << addr << " data 0x" << data << '\n';
	g_emulationPaused = true;
}

void Dma1::DmaRead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	pBus->Read32(addr, data, cycles);
}

void Dma1::DmaWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
}

void Dma1::DmaRead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
}

void Dma1::DmaWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
}

void Dma1::TriggerInterrupt() {
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

void Dma1::endDMATransfer() {
	bTransferActive = false;
	channelControl.reg.startBusy = 0;
	pDMAController->isCpuStopped = false;

	pDMAController->currentClockFunction = 8;

	TriggerInterrupt();
	std::cout << "@@@@@@@DMA1 end" << '\n';
}

void Dma1::Clock() {

	if (pDMAController->isCpuStopped == false) {
		memoryAddress.reg.memAddr = memAddrTemp - 0x20 * 4;
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
		uint32_t data;

		if (channelControl.reg.transDir == toMainRAM) {
			DmaRead32(0x1f801820, data, cycles);
			pBus->Write32(memAddrTemp, data, cycles);
		}
		else {
			//Unhandled
		}
		memAddrTemp += channelControl.reg.memAddrStep ? -4 : +4; //should always be +4 (forward)

		blockWordCount--;
		if (blockWordCount == 0) {
			blockControl.reg.ba--;
			channelControl.reg.startBusy = 0;
			blockWordCount = blockControl.reg.bc_bs;
			//pDMAController->isCpuStopped = false;
			syncClocks = 0;
			memoryAddress.reg.memAddr = memAddrTemp;
		}
	}
}