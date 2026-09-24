#include "dma_controller.h"
#include "../bus_interface.h"

extern bool g_emulationPaused;

dmaController_t::dmaController_t() {
	ppDMA[0] = &dma0;
	ppDMA[1] = &dma1;
	ppDMA[2] = &dma2;
	ppDMA[3] = &dma3;
	ppDMA[4] = &dma4;
	ppDMA[6] = &dma6;

	controlRegister.data = 0;
	interruptRegister.data = 0;

	clockFunctionsArray[0] = &dmaController_t::clock0;
	clockFunctionsArray[1] = &dmaController_t::clock1;
	clockFunctionsArray[2] = &dmaController_t::clock2;
	clockFunctionsArray[3] = &dmaController_t::clock3;
	clockFunctionsArray[4] = &dmaController_t::clock4;
	clockFunctionsArray[5] = &dmaController_t::clock5;
	clockFunctionsArray[6] = &dmaController_t::clock6;
}

void dmaController_t::readDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	if (addr < 0x1f8010f0) {
		if (((addr - 0x1f801080) >> 4 == 0) || ((addr - 0x1f801080) >> 4 == 1) || ((addr - 0x1f801080) >> 4 == 2) ||
			((addr - 0x1f801080) >> 4 == 3) || ((addr - 0x1f801080) >> 4 == 4) || ((addr - 0x1f801080) >> 4 == 6))
			ppDMA[(addr - 0x1f801080) >> 4]->ReadDMA32(addr, data, cycles);
		else {
			std::cout << "[DMA] EMULATION PAUSED! unhandled read32" << " addr 0x" << std::hex << addr << std::endl;
			//g_emulationPaused = true;
			//std::cout << "[DMA] Unhandled read32" << " addr 0x" << std::hex << addr << std::endl;
		}
	}
	else if (addr == 0x1f8010f0)
		data = controlRegister.data;
	else if (addr == 0x1f8010f4) {
		data = interruptRegister.data;
	}
	else {
		std::cout << "[DMA] EMULATION PAUSED! unhandled read32" << " addr 0x" << std::hex << addr << std::endl;
		g_emulationPaused = true;
	}
}

void dmaController_t::writeDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	if (addr < 0x1f8010f0) {
		if (((addr - 0x1f801080) >> 4 == 0) || ((addr - 0x1f801080) >> 4 == 1) || ((addr - 0x1f801080) >> 4 == 2) ||
			((addr - 0x1f801080) >> 4 == 3) || ((addr - 0x1f801080) >> 4 == 4) || ((addr - 0x1f801080) >> 4 == 6))
			ppDMA[(addr - 0x1f801080) >> 4]->WriteDMA32(addr, data, cycles);
		else {
			/*std::cout << "[DMA] EMULATION PAUSED! unhandled write32" << " addr 0x" << std::hex << addr
				<< " data 0x" << std::hex << data << std::endl;*/
				//g_emulationPaused = true;
			std::cout << "[DMA] Unhandled write32" << " addr 0x" << std::hex << addr
				<< " data 0x" << std::hex << data << std::endl;
		}
	}
	else if (addr == 0x1f8010f0) {
		/*std::cout << "[DMA] EMULATION PAUSED! unhandled write32" << " addr 0x" << std::hex << addr
			<< " data 0x" << std::hex << data << std::endl;*/
		controlRegister.data = data;

		//if (controlRegister.reg.cpu_priority != 0) {
			//std::cout << "[DMA] EMULATION PAUSED! unhandled cpu priority " << std::hex << controlRegister.reg.cpu_priority << std::endl;
			//g_emulationPaused = true;
		//}

		/*for (int i = 0; i < 7; i++) {
			const uint8_t iPriority = (controlRegister.data >> (i * 4)) & 0x7;
			for (int j = i + 1; j < 7; j++) {
				const uint8_t jPriority = (controlRegister.data >> (j * 4)) & 0x7;
				if (i != j) {
					if (jPriority < iPriority) {
						swap(clockFunctionsArray[j], clockFunctionsArray[i]);
					}
					else if (jPriority == iPriority) {
						if (j > i)
							swap(clockFunctionsArray[j], clockFunctionsArray[i]);
					}
				}

			}
		}*/

	}
	else if (addr == 0x1f8010f4) {
		uint32_t dicrTemp = (interruptRegister.data << 1 >> 25) & (~data << 1 >> 25);
		interruptRegister.data = (interruptRegister.data & 0x80000000) | (data & 0x00ff803f);
		interruptRegister.data |= dicrTemp << 24;

		if (data & 0x3f) {
			std::cout << "[DMA] EMULATION PAUSED! unhandled interrupt control" << std::endl;
			g_emulationPaused = true;
		}

		if (data & 0x8000) {
			std::cout << "[DMA] EMULATION PAUSED! interruptRegister.reg.force_IRQ" << std::endl;
			g_emulationPaused = true;
		}

		/*	std::cout << "[DMA] EMULATION PAUSED! unhandled write32" << " addr 0x" << std::hex << addr
				<< " data 0x" << std::hex << data << std::endl;*/
				/*interruptRegister.data = (interruptRegister.data & 0xff000000) | (data & 0x00ff803f);
				interruptRegister.data &= ~((data << 1 >> 1) & 0x7f000000);*/

				/*if ((dicrTemp & 0x7f000000) & (data & 0x7f000000))
					interruptRegister.reg.IRQ_master_flag = 0;*/
	}
	else {
		std::cout << "[DMA] EMULATION PAUSED! unhandled write32" << " addr 0x" << std::hex << addr
			<< " data 0x" << std::hex << data << std::endl;
		g_emulationPaused = true;
	}
}

void dmaController_t::readDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA] EMULATION PAUSED! unhandled read16" << " addr 0x" << std::hex << addr << std::endl;
	g_emulationPaused = true;
}

void dmaController_t::writeDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA] EMULATION PAUSED! unhandled write16" << " addr 0x" << std::hex << addr
		<< " data 0x" << std::hex << data << std::endl;
	g_emulationPaused = true;
}

void dmaController_t::readDMA8(const uint32_t& addr, uint8_t& data, uint8_t& cycles) {
	if (addr == 0x1f8010f6)
		data = (interruptRegister.data & 0x00ff0000) >> 16;
	else {
		std::cout << "[DMA] EMULATION PAUSED! unhandled read8" << " addr 0x" << std::hex << addr << std::endl;
		g_emulationPaused = true;
	}
}

void dmaController_t::writeDMA8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles) {
	if (addr == 0x1f8010f6)
		interruptRegister.data = (interruptRegister.data & 0xff00ffff) | ((uint32_t)data << 16);
	else {
		std::cout << "[DMA] EMULATION PAUSED! unhandled write8" << " addr 0x" << std::hex << addr
			<< " data 0x" << std::hex << (uint16_t)data << std::endl;
		g_emulationPaused = true;
	}
}

void dmaController_t::connectBus(BusInterface* p) {
	pbus = p;

	dma0.pBus = pbus;
	dma0.pDMAController = this;

	dma1.pBus = pbus;
	dma1.pDMAController = this;

	dma2.pBus = pbus;
	dma2.pDMAController = this;

	dma3.p_bus = pbus;
	dma3.pDMAController = this;

	dma4.pBus = pbus;
	dma4.pDMAController = this;
	dma4.pDMAController = this;

	dma6.pbus = pbus;
	dma6.pDMAController = this;
}

void dmaController_t::clock() {
	//	if (((pbus->interruptStat & 0x8) == 0)/* && ((interruptRegister.data << 1 >> 25) == 0)*/) 
	{

		bool bOldMasterFlag = interruptRegister.reg.IRQ_master_flag;

		/*for (int i = 0; i < 7; i++) {
			if ((this->*clockFunctionsArray[i])())
				break;
		}*/

		if (currentClockFunction != 8)
			(this->*clockFunctionsArray[currentClockFunction])();

		//if (controlRegister.reg.mdecIn_master_enable && dma0.channelControl.reg.startBusy)//??
		//	dma0.Clock();
		//else if (controlRegister.reg.mdecOut_master_enable && dma1.channelControl.reg.startBusy)//??
		//	dma1.Clock();
		//else if (controlRegister.reg.gpu_master_enable && dma2.channelControl.reg.startBusy)
		//	dma2.Clock();
		//else if (controlRegister.reg.cd_drive_master_enable && dma3.bStart)
		//	dma3.Clock();
		//else if (controlRegister.reg.spu_master_enable && dma4.channelControl.reg.startBusy)//??
		//	dma4.Clock();
		//else if (controlRegister.reg.otc_master_enable && dma6.bStart)
		//	dma6.Clock();

		if ((interruptRegister.reg.force_IRQ) ||
			(interruptRegister.reg.IRQ_master_enable && (interruptRegister.data << 1 >> 25)))
			interruptRegister.reg.IRQ_master_flag = 1;
		else
			interruptRegister.reg.IRQ_master_flag = 0;

		//0->1
		if (!bOldMasterFlag && interruptRegister.reg.IRQ_master_flag)
			pbus->pCpu->cp0.interruptHandler(_IRQ_DMA);
	}
}

bool dmaController_t::clock0() {
	if (controlRegister.reg.mdecIn_master_enable && dma0.bTransferActive) { //??
		dma0.Clock();
		return true;
	}
	return false;
}

bool dmaController_t::clock1() {
	if (controlRegister.reg.mdecOut_master_enable && dma1.bTransferActive) {//??
		dma1.Clock();
		return true;
	}
	return false;
}

bool dmaController_t::clock2() {
	if (controlRegister.reg.gpu_master_enable && dma2.bTransferActive) {
		dma2.Clock();
		return true;
	}
	return false;
}

bool dmaController_t::clock3() {
	if (controlRegister.reg.cd_drive_master_enable && dma3.bStart) {
		dma3.Clock();
		return true;
	}
	return false;
}

bool dmaController_t::clock4() {
	if (controlRegister.reg.spu_master_enable && dma4.bTransferActive) {//??
		dma4.Clock();
		return true;
	}
	return false;
}

bool dmaController_t::clock5() {
	//	g_emulationPaused = true;
	return false;
}

bool dmaController_t::clock6() {
	if (controlRegister.reg.otc_master_enable && dma6.bStart) {
		dma6.Clock();
		return true;
	}
	return false;
}
