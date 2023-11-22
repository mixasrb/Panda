#include "joy_mem_card.h"

#include "../bus/bus_interface.h"

extern const char* g_memCardPaths[2];

extern bool g_emulationPaused;
extern bool g_buttonPressed;
extern uint32_t g_systemClocks;
extern uint32_t g_systemFrames;
extern uint32_t g_systemSeconds;
extern uint32_t g_systemMinutes;

joyMemCard_t::joyMemCard_t() {
	/*memCard1.assign(NUMBER_OF_BLOCKS * NUMBER_OF_FRAMES_PRE_BLOCK * SIZE_OF_FRAME, 0);
	memCard1[0x00] = 'M';
	memCard1[0x01] = 'C';
	memCard1[0x7f] = 0x0e;*/
	joyRxData.elem.firstEntry = 0xff;
	readMemCardFiles();
	memCardFlags[0] = 0x08;
	memCardFlags[1] = 0x08;
}

joyMemCard_t::~joyMemCard_t() {
	writeMemCardFiles();
}

void joyMemCard_t::cpuRead32(const uint32_t& addr, uint32_t& data) {
	//std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled read32 addr 0x" << std::hex << addr << std::endl;
	switch (addr) {
	case 0x1f801044:
		data = joyStat.data;
		break;
	default:
		std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled read32 addr 0x" << std::hex << addr << std::endl;
		g_emulationPaused = true;
		break;
	}
}

void joyMemCard_t::cpuWrite32(const uint32_t& addr, const uint32_t& data) {
	std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled write32 addr 0x" << std::hex << addr
		<< " data 0x" << std::hex << data << std::endl;
	g_emulationPaused = true;
}

void joyMemCard_t::cpuRead16(const uint32_t& addr, uint16_t& data) {
	//std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled read16 addr 0x" << std::hex << addr << std::endl;
	switch (addr) {
	case 0x1f801044:
		data = joyStat.data;
		break;
	case 0x1f801048:
		data = joyMode.data;
		break;
	case 0x1f80104a:
		data = joyCtrl.data;
		break;
	case 0x1f80104e:
		data = joyBaud.baudrateReloadValue;
		break;
	default:
		std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled read16 addr 0x" << std::hex << addr << std::endl;
		g_emulationPaused = true;
		break;
	}
}

void joyMemCard_t::cpuWrite16(const uint32_t& addr, const uint16_t& data) {
	/*std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled write16 addr 0x" << std::hex << addr
		<< std::hex << " data 0x" << data << std::endl;*/
	switch (addr) {
	case 0x1f801048:
		joyMode.data = data;

		if (data != 0xd) {
			std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled joy mode 0x"
				<< std::hex << data << std::endl;
			g_emulationPaused = true;
		}
		break;
	case 0x1f80104a: {
		joyCtrl_t tempJoyCtrl;
		tempJoyCtrl.data = data;
		if (tempJoyCtrl.elem.joynOutput != joyCtrl.elem.joynOutput)
			sequvenceIndex = 0;


		joyCtrl.data = data;

		auto changeState = [&] {
			txen = joyCtrl.elem.txEnable ? true : false;
			rxen = joyCtrl.elem.rxEnable ? true : false; //??

			if (txen) {
				joyStat.elem.txReadFlag1 = 1;//??
				joyStat.elem.txReadFlag2 = 1;//??
			}

			if (joyCtrl.elem.acknowledge)
				acknowledge();
			if (joyCtrl.elem.reset)
				reset();//??

			joy.output = joyCtrl.elem.joynOutput ? LOW : HIGH;
			joy.slotNumber = joyCtrl.elem.desiredSlotNumber ? JOY2 : JOY1;

			if (joyCtrl.elem.rxInterruptMode) {
				std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled rx interrupt mode 0x"
					<< std::hex << joyCtrl.elem.rxInterruptMode << std::endl;
				g_emulationPaused = true;
			}

			if (joyCtrl.elem.txInterruptEnable) {
				std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled tx interrupt"
					<< std::endl;
				g_emulationPaused = true;
			}

			if (joyCtrl.elem.rxInterruptEnable) {
				std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled rx interrupt"
					<< std::endl;
				g_emulationPaused = true;
			}

			if (!joyCtrl.elem.ackInterruptEnable) {
				if (g_buttonPressed)
					std::cout << "[JOY/MEMCARD] disabled IRQ"/* << " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes*/ << std::endl;
			}

			if (joyCtrl.elem.ackInterruptEnable) {
				if (g_buttonPressed)
					std::cout << "[JOY/MEMCARD] enabled IRQ"/* << " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes*/ << std::endl;
			}
			};

		changeState();

		if ((data != 0x0000) &&
			(data != 0x0002) &&
			(data != 0x0012) &&
			(data != 0x0040) &&
			(data != 0x1003) &&
			(data != 0x1013) &&
			(data != 0x2002) &&
			(data != 0x2012) &&
			(data != 0x3003) &&
			(data != 0x3013)) {
			std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled joy control 0x"
				<< std::hex << data << std::endl;
			g_emulationPaused = true;
		}

		break;
	}

	case 0x1f80104e:
		joyBaud.baudrateReloadValue = data;
		baudrateTimerClocks = 0x3fffff;

		if (data != 0x88) {
			std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled baudrate reload value 0x"
				<< std::hex << data << std::endl;
			g_emulationPaused = true;
		}
		break;
	default:
		std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled write16 addr 0x" << std::hex << addr
			<< std::hex << " data 0x" << data << std::endl;
		g_emulationPaused = true;
		break;
	}
}

void joyMemCard_t::cpuRead8(const uint32_t& addr, uint8_t& data) {
	//std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled read8 addr 0x" << std::hex << addr << std::endl;
	switch (addr) {
	case 0x1f801040:
		data = joyRxData.elem.firstEntry;
		joyStat.elem.rxFifoNotEmpty = 0;
		if (g_buttonPressed)
			std::cout << "[JOY/MEMCARD] received data < 0x" << std::hex << (uint16_t)data /*<< " sequence index: " << std::dec << (uint16_t)sequvenceIndex
			<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
			<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes*/ << std::endl;
		break;
	default:
		std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled read8 addr 0x" << std::hex << addr << std::endl;
		g_emulationPaused = true;
		break;
	}
}

void joyMemCard_t::cpuWrite8(const uint32_t& addr, const uint8_t& data) {
	/*std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled write8 addr 0x" << std::hex << addr
		<< std::hex << " data 0x" << (uint16_t)data << std::endl;*/
	switch (addr) {
	case 0x1f801040:
		joyTxData.elem.dataToBeSent = data;

		if (g_buttonPressed)
			std::cout << "[JOY/MEMCARD] sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent /* << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
			<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
			<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes*/ << std::endl;

		bSendingData = true;
		sendingClocks = 1088;
		receivingClocks = 1088;
		ackClocks = 400;
		ackPendingClocks = 0x5;
		bACK = true;

		if (device == MEM_CARD) {
			sendingClocks = 1088 / 2;
			receivingClocks = 1088 / 2;
			ackClocks = 400 / 2;
			ackPendingClocks = 0x5;
		}

		switch (sequvenceIndex) {
		case 0:
			if (joyTxData.elem.dataToBeSent == 0x1) {
				device = PAD;
				joyRxData.elem.firstEntry = 0xff;
				sequvenceIndex++;
			}
			else if (joyTxData.elem.dataToBeSent == 0x81) {
				joyRxData.elem.firstEntry = 0xff;
				if (joy.slotNumber == JOY1) {
					device = MEM_CARD;
					sequvenceIndex++;
					/*bSendingData = false;
					bACK = false;
					sequvenceIndex = 0;*/
				}
				else {
					bSendingData = false;
					bACK = false;
					sequvenceIndex = 0;
				}
			}
			else {

				/*std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;*/

				bSendingData = false;
				bACK = false;
				joyRxData.elem.firstEntry = 0xff;
				sequvenceIndex = 0;
			}
			break;
		case 1:
			if (device == PAD) {
				if (joyTxData.elem.dataToBeSent == 0x42) {
					joyRxData.elem.firstEntry = 0x41;
					sequvenceIndex++;
				}
				else {
					bACK = false;
					joyRxData.elem.firstEntry = 0xff;
					sequvenceIndex = 0;
				}
				break;
			}
			if (device == MEM_CARD) {
				memCardCommand = joyTxData.elem.dataToBeSent;

				if ((memCardCommand != 0x52) && (memCardCommand != 0x57)) {
					std::cout << "[JOY/MEMCARD] Unhandled memory card command 0x" << std::hex << (uint16_t)memCardCommand << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
						<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
						<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
					g_emulationPaused = true;
				}
				if (memCardCommand == 0x58) { //Street Fighter Alpha 3
					bSendingData = false;
					bACK = false;
					sequvenceIndex = 0;
					break;
				}

				joyRxData.elem.firstEntry = memCardFlags[joy.slotNumber];
				sequvenceIndex++;
				break;
			}
			break;
		case 2:
			if (device == PAD) {
				joyRxData.elem.firstEntry = 0x5a;
				sequvenceIndex++;
				break;
			}
			if ((device == MEM_CARD) && (joyTxData.elem.dataToBeSent == 0)) {
				switch (memCardCommand) {
				case 'R':
					joyRxData.elem.firstEntry = 0x5a;
					break;
				case 'W':
					joyRxData.elem.firstEntry = 0x5a;
					break;
				}
				sequvenceIndex++;
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 3:
			if (device == PAD) {
				if (joy.slotNumber == JOY1)
					joyRxData.elem.firstEntry = swloPad1;
				else
					joyRxData.elem.firstEntry = swloPad2;
				sequvenceIndex++;
				break;
			}
			else if ((device == MEM_CARD) && (joyTxData.elem.dataToBeSent == 0)) {
				switch (memCardCommand) {
				case 'R':
					joyRxData.elem.firstEntry = 0x5d;
					break;
				case 'W':
					joyRxData.elem.firstEntry = 0x5d;
					break;
				}
				sequvenceIndex++;
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 4:
			if (device == PAD) {
				bACK = false;
				if (joy.slotNumber == JOY1)
					joyRxData.elem.firstEntry = swhiPad1;
				else
					joyRxData.elem.firstEntry = swhiPad2;
				sequvenceIndex = 0;
				break;
			}
			if (device == MEM_CARD) {
				switch (memCardCommand) {
				case 'R':
					sectorNum = joyTxData.elem.dataToBeSent << 8;
					joyRxData.elem.firstEntry = 0x00;
					break;
				case 'W':
					sectorNum = joyTxData.elem.dataToBeSent << 8;
					joyRxData.elem.firstEntry = 0x00;
					break;
				}
				sequvenceIndex++;
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 5:
			if (device == MEM_CARD) {
				switch (memCardCommand) {
				case 'R':
					sectorNum |= joyTxData.elem.dataToBeSent;
					joyRxData.elem.firstEntry = 0x00;
					break;
				case 'W':
					sectorNum |= joyTxData.elem.dataToBeSent;
					joyRxData.elem.firstEntry = 0x00;
					break;
				}
				if (sectorNum > 0x3fff) {
					std::cout << "[JOY/MEMCARD] invalid memory card sector 0x" << std::hex << sectorNum << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
						<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
						<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
					g_emulationPaused = true;
				}

				sequvenceIndex++;
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 6:
			if (device == MEM_CARD) {
				switch (memCardCommand) {
				case 'R':
					if (joyTxData.elem.dataToBeSent == 0) {
						joyRxData.elem.firstEntry = 0x5c;
						//ackClocks = (31000 - 1500);
					}
					else {
						std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
							<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
							<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
						g_emulationPaused = true;
					}
					break;
				case 'W':
					memCards[joy.slotNumber][sectorNum * 128] = joyTxData.elem.dataToBeSent;
					joyRxData.elem.firstEntry = 0x00;
					break;
				}
				sequvenceIndex++;
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 7:
			if (device == MEM_CARD) {
				switch (memCardCommand) {
				case 'R':
					if (joyTxData.elem.dataToBeSent == 0) {
						joyRxData.elem.firstEntry = 0x5d;
					}
					else {
						std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
							<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
							<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
						g_emulationPaused = true;
					}
					break;
				case 'W':
					memCards[joy.slotNumber][sectorNum * 128 + 1] = joyTxData.elem.dataToBeSent;
					joyRxData.elem.firstEntry = 0x00;
					break;
				}
				sequvenceIndex++;
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 8:
			if (device == MEM_CARD) {
				switch (memCardCommand) {
				case 'R':
					if (joyTxData.elem.dataToBeSent == 0) {
						joyRxData.elem.firstEntry = (uint8_t)(sectorNum >> 8);
					}
					else {
						std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
							<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
							<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
						g_emulationPaused = true;
					}
					break;
				case 'W':
					memCards[joy.slotNumber][sectorNum * 128 + 2] = joyTxData.elem.dataToBeSent;
					joyRxData.elem.firstEntry = 0x00;
					break;
				}
				sequvenceIndex++;
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 9:
			if (device == MEM_CARD) {
				switch (memCardCommand) {
				case 'R':
					if (joyTxData.elem.dataToBeSent == 0) {
						joyRxData.elem.firstEntry = (uint8_t)sectorNum;
					}
					else {
						std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
							<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
							<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
						g_emulationPaused = true;
					}
					break;
				case 'W':
					memCards[joy.slotNumber][sectorNum * 128 + 3] = joyTxData.elem.dataToBeSent;
					joyRxData.elem.firstEntry = 0x00;
					break;
				}
				sequvenceIndex++;
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 128 + 5 + 1:
			if (device == MEM_CARD) {
				switch (memCardCommand) {
				case 'R':
					if (joyTxData.elem.dataToBeSent == 0) {
						joyRxData.elem.firstEntry = memCards[joy.slotNumber][sectorNum * 128 + (sequvenceIndex - 10)];
					}
					else {
						std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
							<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
							<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
						g_emulationPaused = true;
					}
					break;
				case 'W':
					joyRxData.elem.firstEntry = 0x00;
					break;
				}
				sequvenceIndex++;
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 128 + 6 + 1:
			if (device == MEM_CARD) {
				switch (memCardCommand) {
				case 'R':
					if (joyTxData.elem.dataToBeSent == 0) {
						joyRxData.elem.firstEntry = memCards[joy.slotNumber][sectorNum * 128 + (sequvenceIndex - 10)];
					}
					else {
						std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
							<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
							<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
						g_emulationPaused = true;
					}
					break;
				case 'W':
					if (joyTxData.elem.dataToBeSent == 0) {
						joyRxData.elem.firstEntry = 0x5c;
					}
					else {
						std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
							<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
							<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
						g_emulationPaused = true;
					}
					break;
				}
				sequvenceIndex++;
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 128 + 7 + 1:
			if (device == MEM_CARD) {
				switch (memCardCommand) {
				case 'R':
					if (joyTxData.elem.dataToBeSent == 0) {
						joyRxData.elem.firstEntry = memCards[joy.slotNumber][sectorNum * 128 + (sequvenceIndex - 10)];
					}
					else {
						std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
							<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
							<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
						g_emulationPaused = true;
					}
					break;
				case 'W':
					if (joyTxData.elem.dataToBeSent == 0) {
						joyRxData.elem.firstEntry = 0x5d;
					}
					else {
						std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
							<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
							<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
						g_emulationPaused = true;
					}
					break;
				}
				sequvenceIndex++;
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 128 + 8 + 1:
			if (device == MEM_CARD) {
				switch (memCardCommand) {
				case 'R':
					if (joyTxData.elem.dataToBeSent == 0) {
						joyRxData.elem.firstEntry = memCards[joy.slotNumber][sectorNum * 128 + (sequvenceIndex - 10)];
						sequvenceIndex++;
					}
					else {
						std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
							<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
							<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
						g_emulationPaused = true;
					}
					break;
				case 'W':
					if (joyTxData.elem.dataToBeSent == 0) {
						joyRxData.elem.firstEntry = 0x47;
						sequvenceIndex = 0;
						//if (sectorNum = 0x003f)
						memCardFlags[joy.slotNumber] = 0;
					}
					else {
						std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
							<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
							<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
						g_emulationPaused = true;
					}
					break;
				}
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 128 + 9 + 1:
			if (device == MEM_CARD) {
				switch (memCardCommand) {
				case 'R':
					if (joyTxData.elem.dataToBeSent == 0) {
						uint8_t chk = (sectorNum >> 8) ^ (sectorNum & 0xff);
						for (int i = 0; i < 128; i++) {
							chk ^= memCards[joy.slotNumber][sectorNum * 128 + i];
						}
						joyRxData.elem.firstEntry = chk;
					}
					else {
						std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
							<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
							<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
						g_emulationPaused = true;
					}
					break;
				case 'W':
					std::cout << "[JOY/MEMCARD] Unhandled memory card command 0x" << std::hex << (uint16_t)memCardCommand << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
						<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
						<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
					g_emulationPaused = true;
					break;
				}
				sequvenceIndex++;
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 128 + 10 + 1:
			if ((device == MEM_CARD) && (joyTxData.elem.dataToBeSent == 0)) {
				bACK = false;
				switch (memCardCommand) {
				case 'R':
					joyRxData.elem.firstEntry = 0x47;
					sequvenceIndex = 0;
					//memCardFlags[joy.slotNumber] = 0;
					break;
				case 'W':
					std::cout << "[JOY/MEMCARD] Unhandled memory card command 0x" << std::hex << (uint16_t)memCardCommand << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
						<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
						<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
					g_emulationPaused = true;
					break;
				}
				break;
			}
			else {
				std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
					<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
				g_emulationPaused = true;
			}
			break;
		default:
			if (device == PAD) {
				std::cout << "[JOY/MEMCARD] EMULATION PAUSED! Pad communication has gone wrong!" << std::endl;
				g_emulationPaused = true;
			}
			else if (device == MEM_CARD) {
				switch (memCardCommand) {
				case 'R':
					if (joyTxData.elem.dataToBeSent == 0) {
						joyRxData.elem.firstEntry = memCards[joy.slotNumber][sectorNum * 128 + (sequvenceIndex - 10)];
					}
					else {
						std::cout << "[JOY/MEMCARD] Unhandled sent tx data >>>>>> 0x" << std::hex << (uint16_t)joyTxData.elem.dataToBeSent << " sequence index: " << std::dec << (uint16_t)sequvenceIndex
							<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
							<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;
						g_emulationPaused = true;
					}
					break;
				case 'W':
					memCards[joy.slotNumber][sectorNum * 128 + sequvenceIndex - 6] = joyTxData.elem.dataToBeSent;
					joyRxData.elem.firstEntry = 0x00;
					break;
				}
				sequvenceIndex++;
				break;
			}
			break;
		}

		break;
	default:
		std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled write8 addr 0x" << std::hex << addr
			<< std::hex << " data 0x" << (uint16_t)data << std::endl;
		g_emulationPaused = true;
		break;
	}
}

void joyMemCard_t::clock() {

	if (joyStat.elem.ackInputLevel == LOW) {
		ackPendingClocks--;
		if (ackPendingClocks == 0)
			joyStat.elem.ackInputLevel = HIGH;
	}

	voltageLevel_t oldAckInputLevel = joyStat.elem.ackInputLevel;

	if (bSendingData) {
		sendingClocks--;
		if (sendingClocks == 0) {
			bSendingData = false;
			bReceivingData = true;
		}
	}

	if (bReceivingData) {
		receivingClocks--;
		if (receivingClocks == 0) {
			bReceivingData = false;
			bAcknowledging = true;
			joyStat.elem.rxFifoNotEmpty = 1;
			if (g_buttonPressed)
				std::cout << "[JOY/MEMCARD] rx fifo filled" << /*" system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
				<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes <<*/ std::endl;
		}
	}

	if (bAcknowledging) {
		ackClocks--;
		if (ackClocks == 0) {
			bAcknowledging = false;
			if (bACK)
				joyStat.elem.ackInputLevel = LOW;
		}
	}

	if ((oldAckInputLevel == HIGH) && (joyStat.elem.ackInputLevel == LOW)) {
		if (joyCtrl.elem.ackInterruptEnable) {
			joyStat.elem.irq = 1;
			if (g_buttonPressed)
				std::cout << "[JOY/MEMCARD] IRQ7" << /*" system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
				<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes <<*/ std::endl;
			pBus->pCpu->cp0.interruptHandler(_IRQ_7);
		}
		else {
			if (g_buttonPressed)
				std::cout << "[JOY/MEMCARD] IRQ7 not sent" << /*" system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
				<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes <<*/ std::endl;
		}

	}

	baudrateTimerClocks--;
	joyStat.elem.baudrateTimer = baudrateTimerClocks;
	if (baudrateTimerClocks == 0)
		baudrateTimerClocks = 0x3fffff;
}

void joyMemCard_t::readMemCardFiles() {
	constexpr size_t memCardSize = 128 * 1024;

	//for (int i = 0; i < 2; i++) {
	//	std::fstream memCardFile(g_memCardPaths[i], std::ios::binary);
	//	/*if (!memCardFile.is_open()) {
	//		std::ofstream newMemCardFile(g_memCardPaths[i], std::ios::binary);
	//		if (!newMemCardFile.is_open())
	//			throw std::runtime_error("failed to create memory card files");

	//		const char zero[memCardSize] = { 0 };
	//		newMemCardFile.write(zero, memCardSize);
	//		newMemCardFile.close();
	//	}*/

	//	memCards[i].resize(memCardSize);
	//	memCardFile.read((char*)memCards[i].data(), memCardSize);
	//	memCardFile.close();
	//}

	std::ifstream memCardFile("D:/projects/emulation_psx/binaries/memory_cards/memCard1.bin", std::ios::binary);

	/*if (!memCardFile.is_open()) {
		std::ofstream newMemCardFile(g_memCardPaths[i], std::ios::binary);
		if (!newMemCardFile.is_open())
			throw std::runtime_error("failed to create memory card files");

		const char zero[memCardSize] = { 0 };
		newMemCardFile.write(zero, memCardSize);
		newMemCardFile.close();
	}*/

	if (!memCardFile.is_open())
		throw std::runtime_error("failed to open memory card files");
	else {
		memCardFile.read((char*)memCards[0], memCardSize);
		memCardFile.close();
	}
}

void joyMemCard_t::writeMemCardFiles() {
	constexpr size_t memCardSize = 128 * 1024;

	std::ofstream memCardFile("D:/projects/emulation_psx/binaries/memory_cards/memCard1.bin", std::ios::binary);

	if (!memCardFile.is_open())
		throw std::runtime_error("failed to open memory card files");
	else {
		memCardFile.write((char*)memCards[0], memCardSize);
		memCardFile.close();
	}
	/*for (int i = 0; i < 2; i++) {
		std::ofstream newMemCardFile(g_memCardPaths[i], std::ios::binary);
		if (!newMemCardFile.is_open())
			throw std::runtime_error("failed to open memory card files");

		newMemCardFile.write((char*)memCards[i], memCardSize);
		newMemCardFile.close();
	}*/
}

void joyMemCard_t::reset() {
	joyTxData.data = 0;
	joyRxData.data = 0xff;
	joyStat.data = 0;
	//joyMode.data = 0;
	joyBaud.baudrateReloadValue = 0;
	//sequvenceIndex = 0;
	/*if (g_buttonPressed)
		std::cout << "[JOY/MEMCARD] Reseted" << " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
		<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;*/
}

void joyMemCard_t::acknowledge() {
	joyStat.elem.rxParityError = 0;
	joyStat.elem.irq = 0;
	if (g_buttonPressed)
		std::cout << "[JOY/MEMCARD] IRQ7 Acknowledged" /*<< " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
		<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes*/ << std::endl;
}
