#include "dma_controller.h"
#include "../bus_interface.h"

extern bool g_emulationPaused;

dmaController::dmaController() {
	ppDMA[0] = &dma0;
	ppDMA[2] = &dma2;
	ppDMA[3] = &dma3;
	ppDMA[6] = &dma6;
}

void dmaController::ReadDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	if (addr < 0x1f8010f0) {
		if (((addr - 0x1f801080) >> 4 == 0) || ((addr - 0x1f801080) >> 4 == 2) || 
			((addr - 0x1f801080) >> 4 == 3) || ((addr - 0x1f801080) >> 4 == 6))
			ppDMA[(addr - 0x1f801080) >> 4]->ReadDMA32(addr, data, cycles);
		else {
			/*std::cout << "[DMA] EMULATION PAUSED! unhandled read32" << " addr 0x" << std::hex << addr << std::endl;
			isEmulationPaused = true;*/
		}
	}
	else if (addr == 0x1f8010f0)
		data = controlRegister.data;
	else if (addr == 0x1f8010f4)
		data = interruptRegister.data;
	else {
		std::cout << "[DMA] EMULATION PAUSED! unhandled read32" << " addr 0x" << std::hex << addr << std::endl;
		g_emulationPaused = true;
	}
}

void dmaController::WriteDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	if (addr < 0x1f8010f0) {
		if (((addr - 0x1f801080) >> 4 == 0) || ((addr - 0x1f801080) >> 4 == 2) ||
			((addr - 0x1f801080) >> 4 == 3) || ((addr - 0x1f801080) >> 4 == 6))
			ppDMA[(addr - 0x1f801080) >> 4]->WriteDMA32(addr, data, cycles);
		else {
			std::cout << "[DMA] EMULATION PAUSED! unhandled write32" << " addr 0x" << std::hex << addr
			<< " data 0x" << std::hex << data << std::endl;
			g_emulationPaused = true;
		}
	}
	else if (addr == 0x1f8010f0) 
		controlRegister.data = data;
	else if (addr == 0x1f8010f4) {
		uint32_t dicrTemp = interruptRegister.data;
		interruptRegister.data = (interruptRegister.data & 0xff000000) | (data & 0x00ff803f);
		interruptRegister.data &= ~((data << 1 >> 1) & 0x7f000000);

		if ((dicrTemp & 0x7f000000) & (data & 0x7f000000))
			interruptRegister.reg.IRQ_master_flag = 0;
	}
	else {
		std::cout << "[DMA] EMULATION PAUSED! unhandled write32" << " addr 0x" << std::hex << addr
			<< " data 0x" << std::hex << (uint16_t)data << std::endl;
		g_emulationPaused = true;
	}
}

void dmaController::ReadDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA] EMULATION PAUSED! unhandled read16" << " addr 0x" << std::hex << addr << std::endl;
	g_emulationPaused = true;
}

void dmaController::WriteDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA] EMULATION PAUSED! unhandled write32" << " addr 0x" << std::hex << addr
		<< " data 0x" << std::hex << (uint16_t)data << std::endl;
	g_emulationPaused = true;
}

void dmaController::ConnectBus(busInterface* p) {
	pbus = p;

	dma0.pBus = pbus;
	dma0.pDMAController = this;

	dma2.pBus = pbus;
	dma2.pDMAController = this;

	dma3.p_bus = pbus;
	dma3.p_dma = this;

	dma6.pbus = pbus;
	dma6.p_dma = this;
}

void dmaController::clock() {

	bool bOldMasterFlag = interruptRegister.reg.IRQ_master_flag;

	if (interruptRegister.reg.force_IRQ)
		interruptRegister.reg.IRQ_master_flag = 1;

	if (controlRegister.reg.gpu_master_enable && dma0.channelControl.reg.startBusy)
		dma0.Clock();
	if (controlRegister.reg.gpu_master_enable && dma2.channelControl.reg.startBusy)
		dma2.Clock();
	if (controlRegister.reg.cd_drive_master_enable && dma3.bStart)
		dma3.Clock();
	if (controlRegister.reg.otc_master_enable && dma6.bStart)
		dma6.Clock();

	//0->1
	if (interruptRegister.reg.IRQ_master_flag && !bOldMasterFlag)
		pbus->pCp0->interruptHandler(_DMA);
}