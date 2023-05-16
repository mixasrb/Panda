#include "joy_mem_card.h"

#include "../bus/bus_interface.h"

extern bool g_emulationPaused;

joyMemCard::joyMemCard() {
	memCard1.assign(NUMBER_OF_BLOCKS * NUMBER_OF_FRAMES_PRE_BLOCK * SIZE_OF_FRAME, 0);
	memCard1[0x00] = 'M';
	memCard1[0x01] = 'C';
	memCard1[0x7f] = 0x0e;
	joy_rx_data.elem.first_entry = 0xff;
}

void joyMemCard::clock() {
	if (txen)
		joyStat.elem.tx_read_flag_1 = 1;//??

	if (joyStat.elem.ackInputLevel == LOW) {
		ackClocks--;
		if (ackClocks == 0)
			joyStat.elem.ackInputLevel = HIGH;
	}


	voltageLevel_t oldAckInputLevel = joyStat.elem.ackInputLevel;

	if (bReceivingData) {
		receivingClocks--;
		if (receivingClocks == 0) {
			bReceivingData = false;
			joyStat.elem.rx_fifo_not_empty = 1;
			joyStat.elem.ackInputLevel = LOW;
		}
	}

	if (bSendingData) {
		sendingClocks--;
		if (sendingClocks == 0) {
			bSendingData = false;
			bReceivingData = true;
		}
	}

	if ((oldAckInputLevel == HIGH) && (joyStat.elem.ackInputLevel == LOW)) {
		if (joyCtrl.elem.ack_interrupt_enable) {
			joyStat.elem.irq = 1;
			//std::cout << "[JOY/MEMCARD] IRQ7" << std::endl;
			pBus->pCpu->cp0.interruptHandler(_IRQ_7);
		}
	}
}

void joyMemCard::cpuRead8(const uint32_t& addr, uint8_t& data) {
	switch (addr) {
	case 0x1f801040:
		data = joy_rx_data.elem.first_entry;
		joyStat.elem.rx_fifo_not_empty = 0;
		//std::cout << "[JOY/MEMCARD] received data < 0x" << (uint16_t)data << std::endl;
		break;
	default:
		std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled read8 addr 0x" << addr << std::endl;
		g_emulationPaused = true;
		break;
	}
}

void joyMemCard::cpuWrite8(const uint32_t& addr, const uint8_t& data) {
	switch (addr) {
	case 0x1f801040:
		joy_tx_data.elem.data_to_be_sent = data;

		bSendingData = true;
		sendingClocks = 0x880/2;
		receivingClocks = 0x880/2;
		ackClocks = 0x20;

		//std::cout << "[JOY/MEMCARD] sent tx data >>>>>> 0x" << (uint16_t)data << std::endl;

		if (sequvenceIndex == 0) {
			joy_rx_data.elem.first_entry = 0xff;
			sequvenceIndex++;
			if (joy_tx_data.elem.data_to_be_sent == 0x1)
				device = PAD;
			else if (joy_tx_data.elem.data_to_be_sent == 0x81) {
				bSendingData = false;
				device = MEM_CARD;
				sequvenceIndex = 0;
			}
			else {
				std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled tx device 0x" << (uint16_t)data << std::endl;
				g_emulationPaused = true;
			}
			break;
		}
		else if (device == PAD) {
			if (sequvenceIndex == 1) {
				joy_rx_data.elem.first_entry = 0x41;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 2) {
				joy_rx_data.elem.first_entry = 0x5a;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 3) {
				if (joy.slotNumber == JOY1) {
					joy_rx_data.elem.first_entry = swloPad1;
					swloPad1 = 0xff;
				}
				else {
					joy_rx_data.elem.first_entry = swloPad2;
					swloPad2 = 0xff;
				}
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 4) {
				if (joy.slotNumber == JOY1) {
					joy_rx_data.elem.first_entry = swhiPad1;
					swhiPad1 = 0xff;
				}
				else {
					joy_rx_data.elem.first_entry = swhiPad2;
					swhiPad2 = 0xff;
				}
				sequvenceIndex = 0;
			}
		}
		else if (device == MEM_CARD) {
			if (sequvenceIndex == 1) {
				joy_rx_data.elem.first_entry = 0x08;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 2) {
				joy_rx_data.elem.first_entry = 0x5a;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 3) {
				joy_rx_data.elem.first_entry = 0x5d;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 4) {
				joy_rx_data.elem.first_entry = 0x00;
				msb = joy_tx_data.elem.data_to_be_sent;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 5) {
				joy_rx_data.elem.first_entry = 0x00;
				lsb = joy_tx_data.elem.data_to_be_sent;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 6) {
				joy_rx_data.elem.first_entry = 0x5c;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 7) {
				joy_rx_data.elem.first_entry = 0x5d;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 8) {
				joy_rx_data.elem.first_entry = msb;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 9) {
				joy_rx_data.elem.first_entry = lsb;
				sequvenceIndex++;
			}
			else if ((sequvenceIndex > 9) && (sequvenceIndex <= 9 + 128)) {
				joy_rx_data.elem.first_entry = memCard1[sequvenceIndex - 10];
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 9 + 128 + 1) {
				joy_rx_data.elem.first_entry = msb ^ lsb ^ 0x0e;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 9 + 128 + 2) {
				joy_rx_data.elem.first_entry = 0x47;
				sequvenceIndex = 0;
			}
		}

		break;
	default:
		std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled write8 addr 0x" << addr
			<< " data 0x" << (uint16_t)data << std::endl;
		g_emulationPaused = true;
		break;
	}
}

void joyMemCard::cpuRead16(const uint32_t& addr, uint16_t& data) {
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
		std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled read16 addr 0x" << addr << std::endl;
		g_emulationPaused = true;
		break;
	}
}

void joyMemCard::cpuWrite16(const uint32_t& addr, const uint16_t& data) {
	switch (addr) {
	case 0x1f801048:
		joyMode.data = data;

		if (data != 0xd) {
			std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled joy mode 0x"
				<< data << std::endl;
			g_emulationPaused = true;
		}
		break;
	case 0x1f80104a: {
		joyCtrl.data = data;

		auto changeState = [=] {
			txen = joyCtrl.elem.tx_enable ? true : false;
			rxen = joyCtrl.elem.rx_enable ? true : false; //??

			if (joyCtrl.elem.acknowledge)
				acknowledge();
			if (joyCtrl.elem.reset)
				reset();//??

			joy.output = joyCtrl.elem.joynOutput ? LOW : HIGH;
			joy.slotNumber = joyCtrl.elem.desiredSlotNumber ? JOY2 : JOY1;

			if (joyCtrl.elem.rx_interrupt_mode) {
				std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled rx interrupt mode 0x"
					<< joyCtrl.elem.rx_interrupt_mode << std::endl;
				g_emulationPaused = true;
			}

			if (joyCtrl.elem.tx_interrupt_enable) {
				std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled tx interrupt"
					<< std::endl;
				g_emulationPaused = true;
			}

			if (joyCtrl.elem.rx_interrupt_enable) {
				std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled rx interrupt"
					<< std::endl;
				g_emulationPaused = true;
			}
		};

		changeState();
		break;
	}

	case 0x1f80104e:
		joyBaud.baudrateReloadValue = data;

		if (data != 0x88) {
			std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled baudrate reload value 0x"
				<< data << std::endl;
			g_emulationPaused = true;
		}
		break;
	default:
		std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled write16 addr 0x" << addr
			<< " data 0x" << data << std::endl;
		g_emulationPaused = true;
		break;
	}
}

void joyMemCard::cpuRead32(const uint32_t& addr, uint32_t& data) {
	switch (addr) {
	case 0x1f801044:
		data = joyStat.data;
		break;
	default:
		std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled read32 addr 0x" << addr << std::endl;
		g_emulationPaused = true;
		break;
	}
}

void joyMemCard::cpuWrite32(const uint32_t& addr, const uint32_t& data) {
	std::cout << "[JOY/MEMCARD] EMULATION PAUSED! unhandled write32 addr 0x" << addr
		<< " data 0x" << data << std::endl;
	g_emulationPaused = true;
}

void joyMemCard::reset() {
	joy_tx_data.data = 0;
	joy_rx_data.data = 0xff;
	joyStat.data = 0;
	joyMode.data = 0;
	joyCtrl.data = 0;
	joyBaud.baudrateReloadValue = 0;
}

void joyMemCard::acknowledge() {
	joyStat.elem.rxParityError = 0;
	joyStat.elem.irq = 0;
}
