#include "bus_interface.h"

extern bool g_emulationPaused;

//Memory map
#define RAM (((addr >= 0x00000000) && (addr < 0x1f000000)) || \
             ((addr >= 0x80000000) && (addr < 0x9f000000)) || \
             ((addr >= 0xa0000000) && (addr < 0xbf000000)))

#define EXPANSION1 (((addr >= 0x1f000000) && (addr < 0x1f800000)) || \
                    ((addr >= 0x9f000000) && (addr < 0x9f800000)) || \
                    ((addr >= 0xbf000000) && (addr < 0xbf800000)))

#define SCRATCHPAD (((addr >= 0x1f800000) && (addr < 0x1f800400)) || \
                    ((addr >= 0x9f800000) && (addr < 0x9f800400)))

#define MEMORY_CONTROL_1 (((addr >= 0x1f801000) && (addr <= 0x1f801020))/* || \
                          ((addr >= 0x8f801000) && (addr <= 0x8f801020)) || \
                          ((addr >= 0xbf801000) && (addr <= 0xbf801020))*/)

#define JOY_MEM_CARD (((addr >= 0x1f801040) && (addr <= 0x1f80104e)) || \
					  ((addr >= 0x8f801040) && (addr <= 0x8f80104e)) || \
					  ((addr >= 0xbf801040) && (addr <= 0xbf80104e)))

#define SERIAL (((addr >= 0x1f801050) && (addr <= 0x1f80105e)) || \
			    ((addr >= 0x8f801050) && (addr <= 0x8f80105e)) || \
			    ((addr >= 0xbf801050) && (addr <= 0xbf80105e)))

#define MEMORY_CONTROL_2 ((addr == 0x1f801060) || \
						  (addr == 0x8f801060) || \
                          (addr == 0xbf801060))

#define INTERRUPT_STAT ((addr == 0x1f801070) || \
					    (addr == 0x8f801070) || \
                        (addr == 0xbf801070))

#define INTERRUPT_MASK ((addr == 0x1f801074) || \
					    (addr == 0x8f801074) || \
                        (addr == 0xbf801074))

#define DMA (((addr >= 0x1f801080) && (addr <= 0x1f8010fc)) || \
             ((addr >= 0x8f801080) && (addr <= 0x8f8010fc)) || \
             ((addr >= 0xbf801080) && (addr <= 0xbf8010fc)))

#define TIMERS (((addr >= 0x1f801100) && (addr <= 0x1f801128)) || \
			    ((addr >= 0x8f801100) && (addr <= 0x8f801128)) || \
                ((addr >= 0xbf801100) && (addr <= 0xbf801128)))

#define CD_DRIVE (((addr >= 0x1f801800) && (addr <= 0x1f801803))/* || \
				  ((addr >= 0x8f801800) && (addr <= 0x8f801803)) || \
				  ((addr >= 0xbf801800) && (addr <= 0xbf801803))*/)

#define GPU ((addr == 0x1f801810) || (addr == 0x1f801814)/* || \
			 (addr == 0x8f801810) || (addr == 0x8f801814) || \
			 (addr == 0xbf801810) || (addr == 0xbf801814)*/)

#define MDEC ((addr == 0x1f801820) || (addr == 0x1f801824) || \
              (addr == 0x8f801820) || (addr == 0x8f801824) || \
              (addr == 0xbf801820) || (addr == 0xbf801824))

#define SPU (((addr >= 0x1f801c00) && (addr <= 0x1f801fff)) || \
             ((addr >= 0x8f801c00) && (addr <= 0x8f801fff)) || \
             ((addr >= 0xbf801c00) && (addr <= 0xbf801fff)))

#define BIOS (((addr >= 0x1fc00000) && (addr <= (0x1fc00000 + BIOS_SIZE))) || \
              ((addr >= 0x9fc00000) && (addr <= (0x9fc00000 + BIOS_SIZE))) || \
              ((addr >= 0xbfc00000) && (addr <= (0xbfc00000 + BIOS_SIZE))))

#define POST ((addr  == 0x1f802041) || \
              (addr  == 0x8f802041) || \
              (addr  == 0xbf802041))

#define CACHE_CONTROL (addr == 0xfffe0130)


void BusInterface::cpuRead32(const uint32_t& addr, uint32_t& data, uint8_t& clocks,const bool& debug) {
	data = 0;
	clocks = 4;

	if (RAM) {
		if ((addr & 0x1fffffff) > 0x00800000) {
			std::cout << "[BUS] EMULATION PAUSED! invalid ram addr read32 0x"
				<< std::hex << addr << " data 0x" << std::hex << data << "\n";
			g_emulationPaused = true;
			return;
		}

		//if (cacheControl != 0x0001e988) {
		//	/*data = (uint32_t)scratchpad[addr & 0x3ff];
		//	data |= (uint32_t)scratchpad[(addr & 0x3ff) + 1] << 8;
		//	data |= (uint32_t)scratchpad[(addr & 0x3ff) + 2] << 16;
		//	data |= (uint32_t)scratchpad[(addr & 0x3ff) + 3] << 24;*/
		//	clocks = 1;
		//	return;
		//}

		data = *((uint32_t*)(ram.get() + (addr & 0x1fffff)));

		//skullmonkeys patch
	/*	if ((addr & 0x1fffff) == 0x131fc)
			data = 0;*/

		clocks = 5;
	}
	//Expansion 1
	else if (EXPANSION1) {
		data = 0x0;
		clocks = 25;
	}
	//Scratchpad
	else if (SCRATCHPAD) {
		data = *((uint32_t*)(scratchpad.get() + (addr & 0x1fffff)));
		clocks = 1;
	}
	else if (MEMORY_CONTROL_1) {
		switch (addr) {
		case 0x1f801000:
			data = 0x1f000000;
			break;
		case 0x1f801004:
			data = 0x1f802000;
			break;
		case 0x1f801008:
		case 0x1f80100c:
		case 0x1f801010:
		case 0x1f801014:
		case 0x1f801018:
		case 0x1f80101C:
			data = delayRegisters[(addr - 0x1f801000 - 0x8) >> 2].data;
			break;
		case 0x1f801020:
			data = commonDelayRegister.data;
			break;
		}
		clocks = 3;//?? no doc
	}
	else if (MEMORY_CONTROL_2) {
		std::cout << "[BUS] unhandled ram_size reg read32 0x" << std::hex << addr << '\n';
		data = ramSize;
		clocks = 3;
	}
	else if (INTERRUPT_STAT) {
		data = interruptStat;
		clocks = 3;
	}
	else if (INTERRUPT_MASK) {
		data = interruptMask;
		clocks = 3;//?? no doc
	}
	else if (TIMERS) {
		uint32_t temp;
		switch ((addr & 0xf0) >> 4) {
		case 0:
			timer0.Read32(addr, temp);
			data = temp;
			break;
		case 1:
			timer1.Read32(addr, temp);
			data = temp;
			break;
		case 2:
			timer2.Read32(addr, temp);
			data = temp;
			break;
		default:
			std::cout << "[BUS] EMULATION PAUSED! unavailable timer addr read32 0x" << std::hex << addr << "\n";
			g_emulationPaused = true;
			break;
		}
		clocks = 3;//??
	}
	else if (GPU) {
		pGpu->ReadGpu32(addr, data);
		clocks = 4;
	}
	else if (MDEC) {
		pMdec->Read32(addr, data);
		clocks = 3;
	}
	else if (SPU) {
		spu.read32(addr, data);
		clocks = 39;
	}
	//BIOS ROM
	else if (BIOS) {
		if ((addr & 0x000fffff) + 3 > BIOS_SIZE) {
			std::cout << "[BUS] bus_controller adress overflow\n";
			g_emulationPaused = true;
		}

		data = biosChip.biosData[(addr & 0x000fffff) + 3] << (6 * 4) | biosChip.biosData[(addr & 0x000fffff) + 2] << (4 * 4) |
			biosChip.biosData[(addr & 0x000fffff) + 1] << (2 * 4) | biosChip.biosData[addr & 0x000fffff];

		clocks = 25;
	}
	else if (DMA) {
		dma.ReadDmaController32(addr, data, clocks);
		clocks = 3;
	}
	else if (JOY_MEM_CARD) {
		pJoyMemCard->cpuRead32(addr, data);
		clocks = 3;
	}
	else if (CD_DRIVE) {
		std::cout << "[BUS] EMULATION PAUSED! unavailable CD_drive addr read32 0x" << std::hex << addr << "\n";
		g_emulationPaused = true;
	}
	else if (CACHE_CONTROL) {
		data = cacheControl;
		//std::cout << "[BUS] unhandled cache control reg read32 0x" << std::hex << addr << "\n";
		//g_emulationPaused = true;
		clocks = 2;
	}
	else
		if (!debug) {
			std::cout << "[BUS] EMULATION PASUED! unhandled addr read32 0x" << std::hex << addr << "\n";
			g_emulationPaused = true;
		}
}

void BusInterface::cpuWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& clocks) {
	if (RAM) {
		if ((addr & 0x1fffffff) > 0x00800000) {
			std::cout << "[BUS] EMULATION PAUSED! invalid ram addr write32 0x"
				<< std::hex << addr << " data 0x" << std::hex << data << "\n";
			g_emulationPaused = true;
			return;
		}

		if (cacheControl != 0x0001e988) {
			/*scratchpad[addr & 0x3ff] = (uint8_t)data;
			scratchpad[(addr & 0x3ff) + 1] = (uint8_t)(data >> 8);
			scratchpad[(addr & 0x3ff) + 2] = (uint8_t)(data >> 16);
			scratchpad[(addr & 0x3ff) + 3] = (uint8_t)(data >> 24);*/
			return;
		}


		*((uint32_t*)(ram.get() + (addr & 0x1fffff))) = data;
	}
	//Expansion 1
	else if (EXPANSION1) {
		return;
	}
	else if (SCRATCHPAD)
		*((uint32_t*)(scratchpad.get() + (addr & 0x1fffff))) = data;

	else if (MEMORY_CONTROL_1) {
		switch (addr) {
		case 0x1f801000:
			if (data != 0x1f000000) {
				std::cout << "[BUS] EMULATION PAUSED! unhandled memory control reg data 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
				g_emulationPaused = true;
			}
			break;
		case 0x1f801004:
			if (data != 0x1f802000) {
				std::cout << "[BUS] EMULATION PAUSED! unhandled memory control reg data 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
				g_emulationPaused = true;
			}
			break;
		case 0x1f801008:
		case 0x1f80100c:
		case 0x1f801010:
		case 0x1f801014:
		case 0x1f801018:
		case 0x1f80101C:
			if ((addr == 0x1f801008) && (data != 0x0013243f)) {
				std::cout << "[BUS] EMULATION PAUSED! unhandled memory control reg data 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
				g_emulationPaused = true;
			}
			else if ((addr == 0x1f80100c) && (data != 0x00003022)) {
				std::cout << "[BUS] EMULATION PAUSED! unhandled memory control reg data 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
				g_emulationPaused = true;
			}
			else if ((addr == 0x1f801010) && (data != 0x0013243f)) {
				std::cout << "[BUS] EMULATION PAUSED! unhandled memory control reg data 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
				g_emulationPaused = true;
			}
			else if ((addr == 0x1f801014) && (data != 0x200931e1) && (data != 0x220931e1)) {
				std::cout << "[BUS] EMULATION PAUSED! unhandled memory control reg data 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
				g_emulationPaused = true;
			}
			else if ((addr == 0x1f801018) && (data != 0x00020843) && (data != 0x00020943) && (data != 0x21020843)) {
				std::cout << "[BUS] EMULATION PAUSED! unhandled memory control reg data 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
				g_emulationPaused = true;
			}
			else if ((addr == 0x1f80101c) && (data != 0x00070777)) {
				std::cout << "[BUS] EMULATION PAUSED! unhandled memory control reg data 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
				g_emulationPaused = true;
			}

			delayRegisters[(addr - 0x1f801000 - 0x8) >> 2].data = data;
			registerDelays[(addr - 0x1f801000 - 0x8) >> 2] = calculateDelays(delayRegisters[(addr - 0x1f801000 - 0x8) >> 2]);
			break;
		case 0x1f801020:
			if ((data != 0x00031125) && (data != 0x0000132C) && (data != 0x00001325) && (data != 0x00001323)) {
				std::cout << "[BUS] EMULATION PAUSED! unhandled memory control reg data 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
				g_emulationPaused = true;
			}
			commonDelayRegister.data = data;
			for (int i = 0; i < 6; i++) {
				registerDelays[i] = calculateDelays(delayRegisters[i]);
			}
			break;
		}
	}
	else if (MEMORY_CONTROL_2) {
		if ((data != 0xb88) && (data != 0x888)) {
			std::cout << "[BUS] unhandled ram_size reg write32 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
			g_emulationPaused = true;
		}
		ramSize = data;
	}
	else if (INTERRUPT_STAT) {
		interruptStat &= data;
		pCpu->cp0.updateInterruptMaskStat();
	}
	else if (INTERRUPT_MASK) {
		interruptMask = data;
		pCpu->cp0.updateInterruptMaskStat();
	}
	else if (TIMERS) {
		switch ((addr & 0xf0) >> 4) {
		case 0:
			timer0.Write32(addr, data);
			break;
		case 1:
			timer1.Write32(addr, data);
			break;
		case 2:
			timer2.Write32(addr, data);
			break;
		default:
			std::cout << "[BUS] EMULATION PAUSED! unavailable timer addr write32 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
			g_emulationPaused = true;
			break;
		}
	}
	else if (SPU)
		spu.write32(addr, data);
	else if (DMA)
		dma.WriteDmaController32(addr, data, clocks);
	else if (GPU)
		pGpu->WriteGpu32(addr, data);
	else if (MDEC)
		pMdec->Write32(addr, data);
	else if (JOY_MEM_CARD)
		pJoyMemCard->cpuWrite32(addr, data);
	else if (CACHE_CONTROL) {
		cacheControl = data;

		/*	if (cacheControl != 0x0001e988)
				pCpu->iCacheEnabled = false;
			else
				pCpu->iCacheEnabled = true;


			for (uint16_t i = 0; i < 16; i++) {
				pCpu->iCache[i].pc = 0xffffffff;
			}*/

		if ((data != 0x0001e988) && (data != 0x00000804) && (data != 0x00000800)) {
			std::cout << "[BUS] unhandled cache control reg write32 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
			//g_emulationPaused = true;
		}
	}
	else {
		std::cout << "[BUS] EMULATION PASUED! unhandled write32 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		g_emulationPaused = true;
	}
}

void BusInterface::cpuRead16(const uint32_t& addr, uint16_t& data, uint8_t& clocks, const bool& debug) {
	data = 0;
	clocks = 4;

	if (RAM) {
		if ((addr & 0x1fffffff) > 0x00800000) {
			std::cout << "[BUS] EMULATION PAUSED! invalid ram addr read16 0x"
				<< std::hex << addr << " data 0x" << std::hex << data << "\n";
			g_emulationPaused = true;
			return;
		}

		//if (cacheControl != 0x0001e988) {
		//	/*data = (uint32_t)scratchpad[addr & 0x3ff];
		//	data |= (uint32_t)scratchpad[(addr & 0x3ff) + 1] << 8;*/
		//	clocks = 1;
		//	return;
		//}

		data = *((uint16_t*)(ram.get() + (addr & 0x1fffff)));

		clocks = 5;
	}
	//Expansion 1
	else if (EXPANSION1) {
		data = 0x0;
		clocks = 13;
	}
	//Scratchpad
	else if (SCRATCHPAD) {
		data = *((uint16_t*)(scratchpad.get() + (addr & 0x1fffff)));
		clocks = 1;
	}
	else if (INTERRUPT_STAT) {
		data = interruptStat;
		clocks = 4;
	}
	else if (INTERRUPT_MASK) {
		data = interruptMask;
		clocks = 4; //??no docs
	}
	//Ussed only in shell??
	else if (TIMERS) {
		uint32_t temp;
		switch ((addr & 0xf0) >> 4) {
		case 0:
			timer0.Read32(addr, temp);
			data = temp;
			break;
		case 1:
			timer1.Read32(addr, temp);
			data = temp;
			break;
		case 2:
			timer2.Read32(addr, temp);
			data = temp;
			break;
		default:
			std::cout << "[BUS] EMULATION PAUSED! unavailable timer addr read16 0x" << std::hex << addr << "\n";
			g_emulationPaused = true;
			break;
		}
		clocks = 3;//??
	}
	else if (addr == 0x1f801130) { //Gran Turismo 2 uses that reg
		data = 0;
		clocks = 3;//??
	}
	else if (BIOS) {
		if ((addr & 0x000fffff) + 3 > BIOS_SIZE) {
			std::cout << "[BUS] bus_controller adress overflow\n";
			g_emulationPaused = false;
		}

		data = biosChip.biosData[(addr & 0x000fffff) + 1] << (2 * 4) | biosChip.biosData[addr & 0x000fffff];
		clocks = 13;
	}
	else if (SPU) {
		spu.read16(addr, data);
		clocks = 18;
	}
	else if (JOY_MEM_CARD) {
		pJoyMemCard->cpuRead16(addr, data);
		clocks = 3;
	}
	else if (SERIAL) {
		std::cout << "[BUS] Unhandled serial unhandled addr read16 0x" << std::hex << addr << "\n";
		clocks = 3;
	}
	else if (CD_DRIVE) {
		std::cout << "[BUS] EMULATION PAUSED! unavailable CD_drive addr read16 0x" << std::hex << addr << "\n";
		g_emulationPaused = true;
	}
	else
		if (!debug) {
			std::cout << "[BUS] EMULATION PAUSED! unhandled addr read16 0x" << std::hex << addr << "\n";
			g_emulationPaused = true;
		}
}

void BusInterface::cpuWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	if (RAM) {
		if ((addr & 0x1fffffff) > 0x00800000) {
			std::cout << "[BUS] EMULATION PAUSED! invalid ram addr write16 0x"
				<< std::hex << addr << " data 0x" << std::hex << data << "\n";
			g_emulationPaused = true;
			return;
		}

		if (cacheControl != 0x0001e988) {
			/*scratchpad[addr & 0x3ff] = (uint8_t)data;
			scratchpad[(addr & 0x3ff) + 1] = (uint8_t)(data >> 8);*/
			return;
		}

		*((uint16_t*)(ram.get() + (addr & 0x1fffff))) = data;
	}
	//Expansion 1
	else if (EXPANSION1) {
		return;
	}
	else if (SCRATCHPAD)
		*((uint16_t*)(scratchpad.get() + (addr & 0x1fffff))) = data;
	else if (INTERRUPT_STAT) {
		interruptStat &= 0xffff0000 | (uint32_t)data;
		pCpu->cp0.updateInterruptMaskStat();
	}
	else if (INTERRUPT_MASK) {
		interruptMask = data;
		pCpu->cp0.updateInterruptMaskStat();
	}
	else if (TIMERS) {
		switch ((addr & 0xf0) >> 4) {
		case 0:
			timer0.Write32(addr, data);
			break;
		case 1:
			timer1.Write32(addr, data);
			break;
		case 2:
			timer2.Write32(addr, data);
			break;
		default:
			std::cout << "[BUS] EMULATION PAUSED! unavailable timer addr write16 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
			g_emulationPaused = true;
			break;
		}
	}
	else if (SPU)
		spu.write16(addr, data);
	else if (JOY_MEM_CARD)
		pJoyMemCard->cpuWrite16(addr, data);
	else if (SERIAL)
		std::cout << "[BUS] Unhandled serial write16 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
	else if (CACHE_CONTROL) {
		cacheControl = data;
		std::cout << "[BUS] unhandled cache control reg write16 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
	}
	else {
		std::cout << "[BUS] EMULATION PAUSED! unhandled write16 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		g_emulationPaused = true;
	}
}

void BusInterface::cpuRead8(const uint32_t& addr, uint8_t& data, uint8_t& clocks, const bool& debug) {
	data = 0x00;
	clocks = 3;

	if (RAM) {
		if ((addr & 0x1fffffff) > 0x00800000) {
			std::cout << "[BUS] EMULATION PAUSED! invalid ram addr read8 0x"
				<< std::hex << addr << " data 0x" << std::hex << (uint16_t)data << "\n";
			g_emulationPaused = true;
			return;
		}

		data = ram[addr & 0x1fffff];
		clocks = 5;
	}
	//Expansion1
	else if (EXPANSION1) {
		data = 0;
		clocks = 7;
	}
	//Scratchpad
	else if (SCRATCHPAD) {
		data = scratchpad[addr & 0x3ff];
		clocks = 1;
	}
	//BIOS ROM
	else if (BIOS) {
		data = biosChip.biosData[addr & 0x000fffff];
		clocks = 8;
	}
	else if (DMA) {
		dma.ReadDmaController8(addr, data, clocks);
		clocks = 4;
	}
	else if (POST) {
		data = post;
		clocks = 11;
	}
	else if (CD_DRIVE) {
		pCdDrive->readCdDrive8(addr, data);
		clocks = 8;//??
	}
	else if (SPU) {
		spu.read8(addr, data);
		clocks = 18;
	}
	else if (JOY_MEM_CARD) {
		pJoyMemCard->cpuRead8(addr, data);
		clocks = 3;
	}
	else
		if (!debug) {
			std::cout << "[BUS] EMULATION PASUED! unhandled read8 0x" << std::hex << addr << "\n";
			g_emulationPaused = true;
		}
}

void BusInterface::cpuWrite8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles) {
	if (RAM) {
		if ((addr & 0x1fffffff) > 0x00800000) {
			std::cout << "[BUS] EMULATION PAUSED! invalid ram addr write8 0x"
				<< std::hex << addr << " data 0x" << std::hex << (uint16_t)data << "\n";
			g_emulationPaused = true;
			return;
		}


		if (cacheControl != 0x0001e988) {
			//scratchpad[addr & 0x3ff] = data;
			return;
		}

		ram[addr & 0x1fffff] = data;
	}
	//Scratchpad
	else if (SCRATCHPAD)
		scratchpad[addr & 0x3ff] = data;
	else if (DMA)
		dma.WriteDmaController8(addr, data, cycles);
	else if (POST)
		post = data;
	else if (CD_DRIVE)
		pCdDrive->writeCdDrive8(addr, data);
	else if (JOY_MEM_CARD)
		pJoyMemCard->cpuWrite8(addr, data);
	else {
		std::cout << "[BUS] EMULATION PASUED! unhandled write8 0x" << std::hex << addr << " data 0x" << std::hex << (uint16_t)data << "\n";
		g_emulationPaused = true;
	}
}

BusInterface::delayType BusInterface::calculateDelays(const delayRegisterType& delayRegister) {
	delayType delay;

	uint8_t first = 0;
	uint8_t sequence = 0;
	uint8_t readFirst = 0;
	uint8_t readSequence = 0;
	uint8_t writeFirst = 0;
	uint8_t writeSequence = 0;
	uint8_t min = 0;

	if (delayRegister.elem.useCOM0) {
		first = first + commonDelayRegister.elem.com0 - 1;
		sequence = sequence + commonDelayRegister.elem.com0 - 1;
	}
	if (delayRegister.elem.useCOM2) {
		first = first + commonDelayRegister.elem.com2;
		sequence = sequence + commonDelayRegister.elem.com2;
	}
	if (delayRegister.elem.useCOM3)
		min = commonDelayRegister.elem.com3;
	if (first < 6) {
		first++;
		readFirst = first + delayRegister.elem.readDelay + 2;
		readSequence = sequence + delayRegister.elem.readDelay + 2;
		writeFirst = first + delayRegister.elem.readDelay + 2;
		writeSequence = sequence + delayRegister.elem.readDelay + 2;
	}
	if (readFirst < (min + 6)) readFirst = min + 6;
	if (readSequence < (min + 2)) readSequence = min + 2;
	if (writeFirst < (min + 6)) writeFirst = min + 6;
	if (writeSequence < (min + 2)) writeSequence = min + 2;

	delay.readFirst = readFirst;
	delay.readSequence = readSequence;
	delay.writeFirst = writeFirst;
	delay.writeSequence = writeSequence;
	return delay;
}
