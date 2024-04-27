#include "dma_2.h"
#include "../bus_interface.h"
#include "dma_controller.h"
#include <cmath>

extern bool g_emulationPaused;
extern uint32_t g_systemClocks;

void dma2_t::ReadDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
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

void dma2_t::WriteDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		memoryAddress.data = data;
		//std::cout << "~[DMA2] (GPU) madr: 0x" << std::hex << memoryAddress.data << std::endl;
		if (data == 0) {
			std::cout << "~[DMA2] EMULATION PAUSED! (GPU) madr: 0" << std::endl;
			//g_emulationPaused = true;
		}
		break;
	case 4:
		blockControl.data = data;
		break;
	case 8:
		channelControl.data = data;

		if ((channelControl.data != 0x00000000) &&
			(channelControl.data != 0x01000200) &&
			(channelControl.data != 0x01000201) &&
			(channelControl.data != 0x00000401) && //??? maybe some initializing
			(channelControl.data != 0x01000401)) {
			std::cout << "[DMA2] EMULATION PAUSED! unhandled sync channel control mode data 0x"
				<< std::hex << channelControl.data << std::endl;
			g_emulationPaused = true;
		}

		if (channelControl.reg.startBusy) {
			//std::cout << "~[DMA2] (GPU) started" << std::endl;
			bTransferActive = true;
			pDMAController->isCpuStopped = true;
			if (channelControl.reg.syncMode == syncBlocks) {
				memAddrTemp = memoryAddress.reg.memAddr;
				elementCount = blockControl.reg.ba * blockControl.reg.bc_bs;
				float clks = elementCount * CLOCKS_PER_WORD_DMA_2;
				clocks = round(clks) == clks ? clks : round(clks) + 1;
			}
			else if (channelControl.reg.syncMode == linkedList) {
				memAddrTemp = memoryAddress.reg.memAddr /*& 0x00fffffc*/;
				//if (memoryAddress.reg.memAddr & 0x3) { //??
				//	endDMATransfer();
				//	return;
				uint8_t cycles;
				uint32_t node;
				DMARead32(memAddrTemp, node, cycles);
				memAddrTemp += 4;
				memoryAddress.data = node & 0x00ffffff;
				elementCount = node >> 24;
				float clks = elementCount * CLOCKS_PER_WORD_DMA_2;
				clocks = round(clks) == clks ? clks : round(clks) + 1;

				/*pDMAController->isCpuStopped = false;
				syncClocks = 0x10;*/

				//std::cout << "~[DMA2] (GPU) started linked list" << std::endl;
				//g_emulationPaused = true;
			}
		}

		if (pDMAController->controlRegister.reg.gpu_master_enable && channelControl.reg.startBusy)
			pDMAController->currentClockFunction = 2;

		break;
	}
}

void dma2_t::ReadDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA2] EMULATION PAUSED! unhandled ReadDMA16 0x" << addr << std::endl;
	g_emulationPaused = true;
}

void dma2_t::WriteDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA2] EMULATION PAUSED! unhandled WriteDMA16 0x" << addr << " data 0x" << data << std::endl;
	g_emulationPaused = true;
}

void dma2_t::DMARead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	pBus->cpuRead32(addr, data, cycles);
	//if (addr & 0x3)
	//	g_emulationPaused = true;
}

void dma2_t::DMAWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
}

void dma2_t::DMARead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
}

void dma2_t::DMAWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
}

void dma2_t::triggerIRQ() {
	if (pDMAController->interruptRegister.reg.IRQ_enable_dma2)
		pDMAController->interruptRegister.reg.IRQ_flag_dma2 = 1;
	else
		pDMAController->interruptRegister.reg.IRQ_flag_dma2 = 0;

	/*if (pDMAController->interruptRegister.reg.IRQ_master_enable &&
		pDMAController->interruptRegister.reg.IRQ_enable_dma2 &&
		pDMAController->interruptRegister.reg.IRQ_flag_dma2)
		pDMAController->interruptRegister.reg.IRQ_master_flag = 1;
	else
		pDMAController->interruptRegister.reg.IRQ_master_flag = 0;*/
}

void dma2_t::endDMATransfer() {
	bTransferActive = false;
	channelControl.reg.startBusy = 0;
	pDMAController->isCpuStopped = false;
	//std::cout << "~[DMA2] (GPU) finished" << std::endl;

	pDMAController->currentClockFunction = 8;

	//clocks = 0;
	triggerIRQ();
	std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@DMA2 end" << std::dec << g_systemClocks << std::endl;
}

void dma2_t::Clock() {

	/*if (pDMAController->isCpuStopped == false) {
		if (syncClocks == 0) {
			channelControl.reg.startBusy = 1;
			pDMAController->isCpuStopped = true;
		}
		else
			syncClocks--;
		return;
	}*/

	if (clocks == 0) {

		if ((channelControl.reg.syncMode == syncBlocks) ||
			((channelControl.reg.syncMode == linkedList) && (memoryAddress.data & 0x00800000))) {
			//std::cout << "~[DMA2] (GPU) end madr: 0x" << std::hex << memAddrTemp << std::endl;
			endDMATransfer();
			return;
		}

		/*	if (memoryAddress.reg.memAddr & 0x3)
				endDMATransfer();*/

		if (channelControl.reg.syncMode == linkedList) {
			memAddrTemp = memoryAddress.reg.memAddr /*& 0x00fffffc*/;
			if (memoryAddress.reg.memAddr & 0x3) { //??
				endDMATransfer();
				return;
			}
			uint8_t cycles;
			uint32_t node;
			DMARead32(memAddrTemp, node, cycles);
			memAddrTemp += 4;
			memoryAddress.data = node & 0x00ffffff;
			elementCount = node >> 24;
			float clks = elementCount * CLOCKS_PER_WORD_DMA_2;
			clocks = round(clks) == clks ? clks : round(clks) + 1;

			//metal slug x, Syphon Filter (USA) (v1.1), Valkyrie Profile (Disc 1) helps a bit
			//if (abs((float)memAddrTemp - (float)memoryAddress.reg.memAddr) > (float)elementCount) {
			//	//channelControl.reg.startBusy = 0;
				/*pDMAController->isCpuStopped = false;
				syncClocks = 0x0;*/
			//}
			//g_emulationPaused = true;
			//std::cout << "@@@@@@@@@@DMA2 node end 0x" << std::hex << memoryAddress.data << std::endl;

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
		/*if ((memAddrTemp >= 0x001c0000) && (memAddrTemp <= 0x001c0100))
			endDMATransfer();*/
	}
}