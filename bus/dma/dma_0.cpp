#include "dma_0.h"
#include "../bus_interface.h"
#include "dma_controller.h"

extern bool g_emulationPaused;

void Dma0::ReadDma32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
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

void Dma0::WriteDma32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		memoryAddress.data = data;
		break;
	case 4:
		blockControl.data = data;
		break;
	case 8:
		channelControl.data = data;

		if ((channelControl.data != 0) &&
			(channelControl.data != 0x00000201) &&
			(channelControl.data != 0x01000201)) {
			std::cout << "[DMA0] EMULATION PAUSED! unhandled sync channel control mode data 0x"
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
				std::cout << "~[DMA0] Start (MDECin) element count: 0x"

					<< std::hex << elementCount << '\n';
				float clks = elementCount * CLOCKS_PER_WORD_DMA_0;
				clocks = round(clks) == clks ? clks : round(clks) + 1;
			}
		}

		if (pDMAController->controlRegister.reg.mdecIn_master_enable && channelControl.reg.startBusy)
			pDMAController->currentClockFunction = 0;

		break;
	}
}

void Dma0::ReadDma16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA0] EMULATION PAUSED! unhandled ReadDMA16 0x" << addr << '\n';
	g_emulationPaused = true;
}

void Dma0::WriteDma16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA0] EMULATION PAUSED! unhandled WriteDMA16 0x" << addr << " data 0x" << data << '\n';
	g_emulationPaused = true;
}

void Dma0::DmaRead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	pBus->cpuRead32(addr, data, cycles);
}

void Dma0::DmaWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
}

void Dma0::DmaRead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
}

void Dma0::DmaWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
}

void Dma0::TriggerInterrupt() {
	if (pDMAController->interruptRegister.reg.IRQ_enable_dma0)
		pDMAController->interruptRegister.reg.IRQ_flag_dma0 = 1;
	else
		pDMAController->interruptRegister.reg.IRQ_flag_dma0 = 0;

	/*if (pDMAController->interruptRegister.reg.IRQ_master_enable &&
		pDMAController->interruptRegister.reg.IRQ_enable_dma0 &&
		pDMAController->interruptRegister.reg.IRQ_flag_dma0)
		pDMAController->interruptRegister.reg.IRQ_master_flag = 1;
	else
		pDMAController->interruptRegister.reg.IRQ_master_flag = 0;*/
}

void Dma0::endDMATransfer() {
	bTransferActive = false;
	channelControl.reg.startBusy = 0;
	pDMAController->isCpuStopped = false;

	pDMAController->currentClockFunction = 8;

	TriggerInterrupt();
	std::cout << "@@@@@@@DMA0 end" << '\n';
}

void Dma0::Clock() {

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
			//Unhandled
		}
		else {
			DmaRead32(memAddrTemp, data, cycles);
			pBus->cpuWrite32(0x1f801820, data, cycles);
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