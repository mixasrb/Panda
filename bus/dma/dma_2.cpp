#include "dma_2.h"
#include "../bus_interface.h"
#include "dma_controller.h"
#include <cmath>

extern bool g_emulationPaused;
extern uint32_t g_systemClocks;
extern bool g_manualInterruptDMA;

void Dma2::ReadDma32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
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

void Dma2::WriteDma32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	switch (addr << 28 >> 28) {
	case 0:
		memoryAddress.data = data;
		//std::cout << "~[DMA2] (GPU) madr: 0x" << std::hex << memoryAddress.data << '\n';
		if (data == 0) {
			std::cout << "~[DMA2] EMULATION PAUSED! (GPU) madr: 0" << '\n';
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
				<< std::hex << channelControl.data << '\n';
			g_emulationPaused = true;
		}

		if (channelControl.reg.startBusy) {
			//std::cout << "~[DMA2] (GPU) started" << '\n';
			bTransferActive = true;
			pDMAController->isCpuStopped = true;
			
			if (channelControl.reg.syncMode == syncBlocks) {
				memAddrTemp = memoryAddress.reg.memAddr;
				elementCount = blockControl.reg.ba * blockControl.reg.bc_bs;
				float clks = elementCount * CLOCKS_PER_WORD_DMA_2;
				clocks = round(clks) == clks ? clks : round(clks) + 1;
			}
			else if (channelControl.reg.syncMode == linkedList) {
				i = 0;
				memAddrTemp = memoryAddress.reg.memAddr /*& 0x00fffffc*/;
				//if (memoryAddress.reg.memAddr & 0x3) { //??
				//	endDMATransfer();
				//	return;
				uint8_t cycles;
				uint32_t node;
				DmaRead32(memAddrTemp, node, cycles);
				memAddrTemp += 4;
				memoryAddress.data = node & 0x00ffffff;
				elementCount = node >> 24;
				float clks = elementCount * CLOCKS_PER_WORD_DMA_2;
				clocks = round(clks) == clks ? clks : round(clks) + 1;

				/*pDMAController->isCpuStopped = false;
				syncClocks = 0x10;*/

				//std::cout << "~[DMA2] (GPU) started linked list" << '\n';
				//g_emulationPaused = true;
			}
		}

		if (pDMAController->controlRegister.reg.gpu_master_enable && channelControl.reg.startBusy)
			pDMAController->currentClockFunction = 2;

		break;
	}
}

void Dma2::ReadDma16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA2] EMULATION PAUSED! unhandled ReadDMA16 0x" << addr << '\n';
	g_emulationPaused = true;
}

void Dma2::WriteDma16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA2] EMULATION PAUSED! unhandled WriteDMA16 0x" << addr << " data 0x" << data << '\n';
	g_emulationPaused = true;
}

void Dma2::DmaRead32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	pBus->cpuRead32(addr, data, cycles);
	//if (addr & 0x3)
	//	g_emulationPaused = true;
}

void Dma2::DmaWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
}

void Dma2::DmaRead16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
}

void Dma2::DmaWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
}

void Dma2::TriggerInterrupt() {
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

void Dma2::endDMATransfer() {
	clocks = 0;
	bTransferActive = false;
	channelControl.reg.startBusy = 0;
	//pDMAController->pbus->pGpu->gpuStat.elem.readyDma = 0;
	pDMAController->isCpuStopped = false;
	//std::cout << "~[DMA2] (GPU) finished" << '\n';

	pDMAController->currentClockFunction = 8;

	dma2_Pending = false;
	TriggerInterrupt();
	//std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@DMA2 end" << std::dec << g_systemClocks << '\n';
}

void Dma2::Clock() {

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
			((channelControl.reg.syncMode == linkedList) && 
				((memoryAddress.data & 0x00800000) /*|| ((memoryAddress.data & 0xffffffff) == 0)
					|| (memoryAddress.reg.memAddr == memAddrTemp)*/))) {
			//std::cout << "~[DMA2] (GPU) end madr: 0x" << std::hex << memAddrTemp << '\n';
			endDMATransfer();
			return;
		}

		/*	if (memoryAddress.reg.memAddr & 0x3)
				endDMATransfer();*/

		if (channelControl.reg.syncMode == linkedList) {
			memAddrTemp = memoryAddress.reg.memAddr /*& 0x00fffffc*/;
			//if (memoryAddress.reg.memAddr & 0x3) { //??
			//	endDMATransfer();
			//	return;
			//}
			uint8_t cycles;
			uint32_t node;
			DmaRead32(memAddrTemp, node, cycles);
			memoryAddress.data = node & 0x00fffffc;

			dma2_Pending = true;

			//if (memAddrTemp == memoryAddress.reg.memAddr) {
			//	clocks = 0;
			//	bTransferActive = false;
			//	//channelControl.reg.startBusy = 0;
			//	pDMAController->isCpuStopped = false;
			//	//std::cout << "~[DMA2] (GPU) finished" << '\n';

			//	pDMAController->currentClockFunction = 8;

			//	std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@DMA2 self-referencing end" << std::dec << g_systemClocks << '\n';
			//}

			memAddrTemp += 4;
			elementCount = node >> 24;
			float clks = elementCount * CLOCKS_PER_WORD_DMA_2;
			clocks = round(clks) == clks ? clks : round(clks) + 1;


			//i++;
			//if (i > 16384) {
			//	//pDMAController->pbus->pGpu->gpuStat.elem.readyCommand = 0;
			//	//pDMAController->pbus->pGpu->gpuStat.elem.readyDma = 0;
			//	clocks = 0;
			//	bTransferActive = false;
			//	//channelControl.reg.startBusy = 0;
			//	pDMAController->isCpuStopped = false;
			//	//std::cout << "~[DMA2] (GPU) finished" << '\n';

			//	pDMAController->currentClockFunction = 8;

			//	std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@DMA2 self-referencing end" << std::dec << g_systemClocks << '\n';
			//}

			//metal slug x, Syphon Filter (USA) (v1.1), Valkyrie Profile (Disc 1) helps a bit
			//if (abs((float)memAddrTemp - (float)memoryAddress.reg.memAddr) > (float)elementCount) {
			//	//channelControl.reg.startBusy = 0;
			/*pDMAController->isCpuStopped = false;
			syncClocks = 0x0;*/
			//}
			//g_emulationPaused = true;

			if (g_manualInterruptDMA)
				std::cout << "@@@@@@@@@@DMA2 node end 0x" << std::hex << memoryAddress.data << '\n';

			return;
		}
	}

	clocks--;
	if (clocks < elementCount) {
		uint8_t cycles;
		uint32_t data;

		if (channelControl.reg.transDir == toMainRAM) {
			DmaRead32(0x1f801810, data, cycles);
			pBus->cpuWrite32(memAddrTemp, data, cycles);
		}
		else {
			DmaRead32(memAddrTemp, data, cycles);
			pBus->cpuWrite32(0x1f801810, data, cycles);
		}
		memAddrTemp += channelControl.reg.memAddrStep ? -4 : +4; //should always be +4 (forward)
		/*if ((memAddrTemp >= 0x001c0000) && (memAddrTemp <= 0x001c0100))
			endDMATransfer();*/
	}
}