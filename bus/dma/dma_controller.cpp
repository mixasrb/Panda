#include "dma_controller.h"
#include "../bus_interface.h"

extern bool isEmulationPaused;

dmaController::dmaController() {
	ppDMA[2] = &dma_2;
	ppDMA[3] = &dma_3;
	ppDMA[6] = &dma_6;
}

void dmaController::ReadDMA32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	if (addr < 0x1f8010f0) {
		if (((addr - 0x1f801080) >> 4 == 2) || ((addr - 0x1f801080) >> 4 == 3) || ((addr - 0x1f801080) >> 4 == 6))
			ppDMA[(addr - 0x1f801080) >> 4]->ReadDMA32(addr, data, cycles);
		else {
			std::cout << "[DMA] EMULATION PAUSED! unhandled read32" << " addr 0x" << std::hex << addr << std::endl;
			isEmulationPaused = true;
		}
	}
	else if (addr == 0x1f8010f0)
		data = dpcr.data;
	else if (addr == 0x1f8010f4)
		data = dicr.data;
	else {
		std::cout << "[DMA] EMULATION PAUSED! unhandled read32" << " addr 0x" << std::hex << addr << std::endl;
		isEmulationPaused = true;
	}
}

void dmaController::WriteDMA32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {
	if (addr < 0x1f8010f0) {
		if (((addr - 0x1f801080) >> 4 == 2) || ((addr - 0x1f801080) >> 4 == 3) || ((addr - 0x1f801080) >> 4 == 6))
			ppDMA[(addr - 0x1f801080) >> 4]->WriteDMA32(addr, data, cycles);
		else {
			std::cout << "[DMA] EMULATION PAUSED! unhandled write32" << " addr 0x" << std::hex << addr
			<< " data 0x" << std::hex << (uint16_t)data << std::endl;
			isEmulationPaused = true;
		}
	}
	else if (addr == 0x1f8010f0)
		dpcr.data = data;
	else if (addr == 0x1f8010f4) {
		uint32_t dicrTemp = dicr.data;
		dicr.data = (dicr.data & 0xff000000) | (data & 0x00ff803f);
		dicr.data &= ~((data << 1 >> 1) & 0x7f000000);

		if ((dicrTemp & 0x7f000000) & (data & 0x7f000000))
			dicr.reg.IRQ_master_flag = 0;
	}
}

void dmaController::ReadDMA16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA] EMULATION PAUSED! unhandled read16" << " addr 0x" << std::hex << addr << std::endl;
	isEmulationPaused = true;
}

void dmaController::WriteDMA16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	std::cout << "[DMA] EMULATION PAUSED! unhandled write32" << " addr 0x" << std::hex << addr
		<< " data 0x" << std::hex << (uint16_t)data << std::endl;
	isEmulationPaused = true;
}

void dmaController::ConnectBus(busInterface* p) {
	pbus = p;

	dma_2.pBus = pbus;
	dma_2.pDMAController = this;

	dma_3.p_bus = pbus;
	dma_3.p_dma = this;

	dma_6.pbus = pbus;
	dma_6.p_dma = this;
}

void dmaController::clock() {

	bool bOldMasterFlag = dicr.reg.IRQ_master_flag;
	if (dicr.reg.force_IRQ)
		dicr.reg.IRQ_master_flag = 1;

	if (dpcr.reg.gpu_master_enable && dma_2.channelControl.reg.startBusy)
		dma_2.Clock();
	if (dpcr.reg.cd_drive_master_enable && dma_3.bStart)
		dma_3.Clock();
	if (dpcr.reg.otc_master_enable && dma_6.bStart)
		dma_6.Clock();

	//0->1
	if (dicr.reg.IRQ_master_flag && !bOldMasterFlag)
		pbus->pCp0->interruptHandler(_DMA);
}