#include "bus_interface.h"

extern bool isEmulationPaused;

//Memory map
#define RAM ((addr >= 0x00000000 && addr < 0x1f000000) || \
(addr >= 0x80000000 && addr < 0x9f000000) || \
(addr >= 0xa0000000 && addr < 0xbf000000))

#define EXPANSION1 ((addr >= 0x1f000000 && addr < 0x1f800000) || \
(addr >= 0x9f000000 && addr <= 0x9f800000) || \
(addr >= 0xbf900000 && addr <= 0xbf800000))

#define SCRATCHPAD (addr >= 0x1f800000 && addr < 0x1f800400)

#define MEMORY_CONTROL_1 (addr >= 0x1f801000 && addr <= 0x1f801020)

#define JOY_MEM_CARD (addr >= 0x1f801040 && addr <= 0x1f80104e)

#define MEMORY_CONTROL_2 (addr == 0x1f801060)

#define INTERRUPT_STAT (addr << 4 >> 4 == 0x0f801070)
#define INTERRUPT_MASK (addr << 4 >> 4 == 0x0f801074)

#define DMA (addr >= 0x1f801080 && addr <= 0x1f8010f4)

#define TIMERS (addr >= 0x1f801100 && addr <= 0x1f801128)

#define CD_DRIVE (addr >= 0x1f801800 && addr <= 0x1f801803)

#define GPU ((addr == 0x1f801810) || (addr == 0x1f801814))

#define BIOS ((addr >= 0x1fc00000 && addr <= (0x1fc00000 + BIOS_SIZE)) || \
(addr >= 0x9fc00000 && addr <= (0x9fc00000 + BIOS_SIZE)) || \
(addr >= 0xbfc00000 && addr <= (0xbfc00000 + BIOS_SIZE)))

#define POST ((addr << 4 >> 4) == 0x0f802041)

#define CACHE_CONTROL (addr == 0xfffe0130)


void busInterface::cpuRead32(const uint32_t& addr, uint32_t& data, uint8_t& clocks, bool debug) {
	data = 0;
	clocks = 6;

	if (RAM) {
		const bool accessICache = ((pCp0->get(Isc) & 0x10000) != 0x10000) && (cacheControl != 0x800);
		if (accessICache) {
			//if ((addr & 0x1fffffff) > 0x200000) {
			//	//throw std::runtime_error("unhandled ram addr read32\n");
			//	std::cout << "[BUS] unhandled ram addr read32 0x" << std::hex << addr << std::endl;
			//	//isEmulationPaused = true;
			//}
			//else {
			data = (uint32_t)ram[addr & 0x1fffff];
			data |= (uint32_t)ram[(addr & 0x1fffff) + 1] << 8;
			data |= (uint32_t)ram[(addr & 0x1fffff) + 2] << 16;
			data |= (uint32_t)ram[(addr & 0x1fffff) + 3] << 24;
			//}
			clocks = 5;
		}
		else
			std::cout << "[BUS] unhandled cache read32 0x" << std::hex << addr << std::endl;
	}
	//Expansion 1
	else if (EXPANSION1) {
		data = 0x0;
		clocks = 26;
	}
	//Scratchpad
	else if (SCRATCHPAD) {
		data = (uint32_t)scratchpad[addr & 0x3ff];
		data |= (uint32_t)scratchpad[(addr & 0x3ff) + 1] << 8;
		data |= (uint32_t)scratchpad[(addr & 0x3ff) + 2] << 16;
		data |= (uint32_t)scratchpad[(addr & 0x3ff) + 3] << 24;
		clocks = 1;
	}
	else if (MEMORY_CONTROL_2) {
		std::cout << "[BUS] unhandled ram_size reg read32 0x" << std::hex << addr << std::endl;
		data = ramSize;
		clocks = 3;//?? no doc
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
		uint16_t datum;
		switch (addr << 24 >> 28) {
		case 0:
			timer0.ReadTimer32(addr, &datum);
			data = datum;
			break;
		case 1:
			timer1.ReadTimer32(addr, &datum);
			data = datum;
			break;
		case 2:
			timer2.ReadTimer32(addr, &datum);
			data = datum;
			break;
		default:
			std::cout << "[BUS] EMULATION PAUSED! unavailable timer addr read32 0x" << std::hex << addr << "\n";
			isEmulationPaused = true;
			break;
		}
		clocks = 3;//??
	}
	else if (GPU) {
		pGpu->readGpu32(addr, data);
		clocks = 4;
	}
	//BIOS ROM
	else if (BIOS) {
		if ((addr & 0x000fffff) + 3 > BIOS_SIZE)
			throw std::runtime_error("bus_controller adress overflow\n");

		data = biosChip.biosData[(addr & 0x000fffff) + 3] << (6 * 4) | biosChip.biosData[(addr & 0x000fffff) + 2] << (4 * 4) |
			biosChip.biosData[(addr & 0x000fffff) + 1] << (2 * 4) | biosChip.biosData[addr & 0x000fffff];

		clocks = 25;
	}
	else if (DMA) {
		dma.ReadDMA32(addr, data, clocks);
		clocks = 3;
	}
	else if (JOY_MEM_CARD) {
		//std::cout << "[BUS] EMULATION PASUED! unhandled addr read32 0x" << std::hex << addr << "\n";
		pJoyMemCard->cpuRead32(addr, data);
		clocks = 3;
	}
	else if (CD_DRIVE) {
		//std::cout << "[BUS] EMULATION PAUSED! unavailable CD_drive addr read32 0x" << std::hex << addr << "\n";
		isEmulationPaused = true;
	}
	else
		if (!debug) {
			std::cout << "[BUS] EMULATION PASUED! unhandled addr read32 0x" << std::hex << addr << "\n";
			isEmulationPaused = true;
		}

	//debug
	/*if (!debug)
		if (MEMORY_CONTROL_1 || INPUT_CONTROLLER || MEMORY_CONTROL_2 || INTERRUPT_STAT || INTERRUPT_MASK || DMA || TIMERS || CD_DRIVE || CACHE_CONTROL)
			std::cout << "-------[BUS] I/O addr read32 0x" << std::hex << addr << "\n";*/
}

void busInterface::cpuWrite32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles) {

	if (RAM) {
		const bool accessICache = ((pCp0->get(Isc) & 0x10000) != 0x10000) && (cacheControl != 0x800);
		if (accessICache) {
			/*if ((addr & 0x1fffffff) > 0x200000)
				std::cout << "[BUS] unhandled ram addr write32 0x" << std::hex << addr << " data 0x" << data << std::endl;
			else {*/
			ram[addr & 0x1fffff] = (uint8_t)data;
			ram[(addr & 0x1fffff) + 1] = (uint8_t)(data >> 8);
			ram[(addr & 0x1fffff) + 2] = (uint8_t)(data >> 16);
			ram[(addr & 0x1fffff) + 3] = (uint8_t)(data >> 24);
			//}
		}
		else if (data)
			std::cout << "[BUS] unhandled cache write32 0x" << std::hex << addr << " data 0x" << data << std::endl;
	}
	//Expansion 1
	else if (EXPANSION1) {
		return;
	}
	else if (SCRATCHPAD) {
		scratchpad[addr & 0x3ff] = (uint8_t)data;
		scratchpad[(addr & 0x3ff) + 1] = (uint8_t)(data >> 8);
		scratchpad[(addr & 0x3ff) + 2] = (uint8_t)(data >> 16);
		scratchpad[(addr & 0x3ff) + 3] = (uint8_t)(data >> 24);
	}
	else if (MEMORY_CONTROL_1)
		std::cout << "[BUS] unhandled memory control reg write32 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
	else if (MEMORY_CONTROL_2) {
		std::cout << "[BUS] unhandled ram_size reg write32 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		ramSize = data;
	}
	else if (INTERRUPT_STAT)
		interruptStat &= data;
	else if (INTERRUPT_MASK)
		interruptMask = data;
	else if (TIMERS) {
		switch (addr << 24 >> 28) {
		case 0:
			timer0.WriteTimer32(addr, data);
			break;
		case 1:
			timer1.WriteTimer32(addr, data);
			break;
		case 2:
			timer2.WriteTimer32(addr, data);
			break;
		default:
			std::cout << "[BUS] EMULATION PAUSED! unavailable timer addr write32 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
			isEmulationPaused = true;
			break;
		}
	}
	else if (addr == 0xfffe0130) {
		cacheControl = data;
		std::cout << "[BUS] unhandled cache control reg write32 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
	}
	else if (DMA)
		dma.WriteDMA32(addr, data, cycles);
	else if (GPU)
		pGpu->writeGpu32(addr, data);
	else if (JOY_MEM_CARD) {
		//std::cout << "[BUS] EMULATION PASUED! unhandled write32 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		pJoyMemCard->cpuWrite32(addr, data);
	}
	else {
		std::cout << "[BUS] EMULATION PASUED! unhandled write32 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		isEmulationPaused = true;
	}

	//debug
	/*if (MEMORY_CONTROL_1 || INPUT_CONTROLLER || MEMORY_CONTROL_2 || INTERRUPT_STAT || INTERRUPT_MASK || DMA || TIMERS || CD_DRIVE || CACHE_CONTROL)
		std::cout << "-------[BUS] I/O write32 adress 0x" << std::hex << addr << " data 0x" << std::hex << (uint32_t)data << "\n";*/
}

void busInterface::cpuRead16(const uint32_t& addr, uint16_t& data, uint8_t& clocks, bool debug) {
	data = 0;
	clocks = 6;
	if (RAM) {
		const bool accessICache = ((pCp0->get(Isc) & 0x10000) != 0x10000) && (cacheControl != 0x800);
		if (accessICache) {
			/*if ((addr & 0x1fffffff) > 0x200000)
				std::cout << "[BUS] unhandled ram addr read16 0x" << std::hex << addr << std::endl;
			else {*/
			data = (uint32_t)ram[addr & 0x1fffff];
			data |= (uint32_t)ram[(addr & 0x1fffff) + 1] << 8;
			//}
			clocks = 5;
		}
		else
			std::cout << "[BUS] unhandled cache read16 0x" << std::hex << addr << std::endl;
	}
	//Expansion 1
	else if (EXPANSION1) {
		data = 0x0;
		clocks = 26;
	}
	//Scratchpad
	else if (SCRATCHPAD) {
		data = (uint32_t)scratchpad[addr & 0x3ff];
		data |= (uint32_t)scratchpad[(addr & 0x3ff) + 1] << 8;
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
		uint16_t datum;
		switch (addr << 24 >> 28) {
		case 0:
			timer0.ReadTimer32(addr, &datum);
			data = datum;
			break;
		case 1:
			timer1.ReadTimer32(addr, &datum);
			data = datum;
			break;
		case 2:
			timer2.ReadTimer32(addr, &datum);
			data = datum;
			break;
		default:
			std::cout << "[BUS] EMULATION PAUSED! unavailable timer addr read16 0x" << std::hex << addr << "\n";
			isEmulationPaused = true;
			break;
		}
		clocks = 3;
	}
	//BIOS ROM
	else if (BIOS) {
		if ((addr & 0x000fffff) + 3 > BIOS_SIZE)
			throw std::runtime_error("[BUS] bus_controller adress overflow\n");

		data = biosChip.biosData[(addr & 0x000fffff) + 1] << (2 * 4) | biosChip.biosData[addr & 0x000fffff];
		clocks = 13;
	}
	else if ((addr >= 0x1f801c00) && (addr < 0x1f801d80)) {
		//std::cout << "[BUS] unhandled SPU Voice Registers read16 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		clocks = 18;//??
		return;
	}
	else if ((addr >= 0x1f801d80) && (addr <= 0x1f801dfe)) {
		//std::cout << "[BUS] unhandled SPU Control Registers read16 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		clocks = 18;//??
		return;
	}
	else if (JOY_MEM_CARD) {
		//std::cout << "[BUS] EMULATION PAUSED! unhandled addr read16 0x" << std::hex << addr << "\n";
		pJoyMemCard->cpuRead16(addr, data);
		clocks = 3;
	}
	else if (CD_DRIVE) {
		std::cout << "[BUS] EMULATION PAUSED! unavailable CD_drive addr read16 0x" << std::hex << addr << "\n";
		isEmulationPaused = true;
	}
	else
		if (!debug) {
			std::cout << "[BUS] EMULATION PAUSED! unhandled addr read16 0x" << std::hex << addr << "\n";
			isEmulationPaused = true;
		}

	//debug
	/*if (!debug)
		if (MEMORY_CONTROL_1 || INPUT_CONTROLLER || MEMORY_CONTROL_2 || INTERRUPT_STAT || INTERRUPT_MASK || DMA || TIMERS || CD_DRIVE || CACHE_CONTROL)
			std::cout << "-------[BUS] I/O addr read16 0x" << std::hex << addr << "\n";*/
}

void busInterface::cpuWrite16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles) {
	/*if (data == 0xffdc)
		isEmulationPaused = true;*/

	if (RAM) {
		const bool accessICache = ((pCp0->get(Isc) & 0x10000) != 0x10000) && (cacheControl != 0x800);
		if (accessICache) {
			/*if ((addr & 0x1fffffff) > 0x200000)
				std::cout << "[BUS] unhandled ram addr write16 0x" << std::hex << addr << " data 0x" << data << std::endl;
			else {*/
			ram[addr & 0x1fffff] = (uint8_t)data;
			ram[(addr & 0x1fffff) + 1] = (uint8_t)(data >> 8);
			//}
		}
		else if (data)
			std::cout << "[BUS] unhandled cache write16 0x" << std::hex << addr << " data 0x" << data << std::endl;//???
	}
	//Expansion 1
	else if (EXPANSION1) {
		return;
	}
	else if (SCRATCHPAD) {
		scratchpad[addr & 0x3ff] = (uint8_t)data;
		scratchpad[(addr & 0x3ff) + 1] = (uint8_t)(data >> 8);
	}
	else if (MEMORY_CONTROL_1)
		std::cout << "[BUS] unhandled memory control reg write16 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
	else if (MEMORY_CONTROL_2) {
		std::cout << "[BUS] unhandled ram_size reg write16 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		ramSize = (uint32_t)data;
	}
	else if (INTERRUPT_STAT)
		interruptStat &= 0xffff0000 | (uint32_t)data;
	else if (INTERRUPT_MASK)
		interruptMask = data;
	else if (TIMERS) {
		switch (addr << 24 >> 28) {
		case 0:
			timer0.WriteTimer32(addr, data);
			break;
		case 1:
			timer1.WriteTimer32(addr, data);
			break;
		case 2:
			timer2.WriteTimer32(addr, data);
			break;
		default:
			std::cout << "[BUS] EMULATION PAUSED! unavailable timer addr write16 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
			isEmulationPaused = true;
			break;
		}
	}
	else if (addr == 0xfffe0130) {
		cacheControl = data;
		std::cout << "[BUS] unhandled cache control reg write16 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
	}
	else if ((addr >= 0x1f801c00) && (addr < 0x1f801d80)) {
		//std::cout << "[BUS] unhandled SPU Voice Registers write16 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		return;
	}
	else if ((addr >= 0x1f801d80) && (addr <= 0x1f801dbc)) {
		//std::cout << "[BUS] unhandled SPU Control Registers write16 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		return;
	}
	else if (addr >= 0x1f801dc0 && addr <= 0x1f801dfe) {
		//std::cout << "[BUS] unhandled SPU Reverb Configuration Area write16 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		return;
	}
	else if (JOY_MEM_CARD) {
		//std::cout << "[BUS] EMULATION PAUSED! unhandled write16 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		pJoyMemCard->cpuWrite16(addr, data);
	}
	else {
		std::cout << "[BUS] EMULATION PAUSED! unhandled write16 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		isEmulationPaused = true;
	}

	//debug
	/*if (MEMORY_CONTROL_1 || INPUT_CONTROLLER || MEMORY_CONTROL_2 || INTERRUPT_STAT || INTERRUPT_MASK || DMA || TIMERS || CD_DRIVE || CACHE_CONTROL)
		std::cout << "-------[BUS] I/O write16 adress 0x" << std::hex << addr << " data 0x" << std::hex << (uint32_t)data << "\n";*/
}

void busInterface::cpuRead8(const uint32_t& addr, uint8_t& data, uint8_t& clocks, bool debug) {
	data = 0x00;
	clocks = 6;

	if (RAM) {
		const bool accessICache = ((pCp0->get(Isc) & 0x10000) != 0x10000) && (cacheControl != 0x800);
		if (accessICache) {
			/*if ((addr & 0x1fffffff) > 0x200000) {
				std::cout << "unhandled ram addr read8 0x" << std::hex << addr << std::endl;
				isEmulationPaused = true;
			}*/
			//else
			data = ram[addr & 0x1fffff];
			clocks = 5;
		}
		else
			std::cout << "[BUS] unhandled cache read8 0x" << std::hex << addr << std::endl;
	}
	//Expansion1
	else if (EXPANSION1) {
		data = 0;
		clocks = 7;
	}
	//Scratchpad
	else if (SCRATCHPAD) {
		data = (uint32_t)scratchpad[addr & 0x3ff];
		clocks = 1;
	}
	//BIOS ROM
	else if (BIOS) {
		data = biosChip.biosData[addr & 0x000fffff];
		clocks = 8;
	}
	else if (POST) {
		data = post;
		clocks = 11;
	}
	else if (CD_DRIVE) {
		pCdDrive->ReadCdDrive8(addr, data);
		clocks = 8;//??
	}
	else if (JOY_MEM_CARD) {
		//std::cout << "[BUS] EMULATION PASUED! unhandled read8 adress 0x" << addr << "\n";
		pJoyMemCard->cpuRead8(addr, data);
		clocks = 3;
	}
	else
		if (!debug) {
			std::cout << "[BUS] EMULATION PASUED! unhandled read8 0x" << addr << "\n";
			isEmulationPaused = true;
		}

	//debug
	/*if (!debug)
		if (MEMORY_CONTROL_1 || INPUT_CONTROLLER || MEMORY_CONTROL_2 || INTERRUPT_STAT || INTERRUPT_MASK || DMA || TIMERS || CD_DRIVE || CACHE_CONTROL)
			std::cout << "-------[BUS] I/O addr read8 0x" << std::hex << addr << "\n";*/
}

void busInterface::cpuWrite8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles) {
	if (RAM) {
		const bool accessICache = ((pCp0->get(Isc) & 0x10000) != 0x10000) && (cacheControl != 0x800);
		if (accessICache) {
			/*if ((addr & 0x1fffffff) > 0x200000)
				std::cout << "unhandled ram addr write8 0x" << std::hex << addr << " data 0x" << data << std::endl;*/
				//else
			ram[addr & 0x1fffff] = data;
		}
		else if (data)
			std::cout << "[BUS] unhandled cache write8 0x" << std::hex << addr << " data0x" << (uint16_t)data << std::endl;
	}
	//Scratchpad
	else if (SCRATCHPAD) {
		scratchpad[addr & 0x3ff] = data;
	}
	else if (POST)
		post = data;
	else if (CD_DRIVE)
		pCdDrive->WriteCdDrive8(addr, data);
	else if (JOY_MEM_CARD) {
		//std::cout << "[BUS] EMULATION PASUED! unhandled write8 adress 0x" << std::hex << addr << " data 0x" << std::hex << (uint32_t)data << "\n";
		pJoyMemCard->cpuWrite8(addr, data);
	}
	else {
		std::cout << "[BUS] EMULATION PASUED! unhandled write8 0x" << std::hex << addr << " data 0x" << std::hex << (uint16_t)data << "\n";
		isEmulationPaused = true;
	}

	//debug
	/*if (MEMORY_CONTROL_1 || INPUT_CONTROLLER || MEMORY_CONTROL_2 || INTERRUPT_STAT || INTERRUPT_MASK || DMA || TIMERS || CD_DRIVE || CACHE_CONTROL)
		std::cout << "-------[BUS] I/O write8 adress 0x" << std::hex << addr << " data 0x" << std::hex << (uint32_t)data << "\n";*/
}