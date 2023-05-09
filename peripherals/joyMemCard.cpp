#include "joyMemCard.h"

#include "../bus/bus_interface.h"

extern bool isEmulationPaused;

void joyMemCard::clock() {
	if (txen)
		joyStat.elem.tx_read_flag_1 = SET;

	if (joyStat.elem.ackInputLevel == LOW) {
		ackClocks--;
		if (ackClocks == 0)
			joyStat.elem.ackInputLevel = HIGH;
	}

	if (bReceivingData) {
		receivingClocks--;
		if (receivingClocks == 0) {
			bReceivingData = false;
			joyStat.elem.rx_fifo_not_empty = SET;
		}
	}

	if (bSendingData) {
		sendingClocks--;
		if (sendingClocks == 0) {
			bSendingData = false;
			bReceivingData = true;
			joyStat.elem.ackInputLevel = LOW;
		}
	}

	if (joyStat.elem.ackInputLevel = LOW) {
		if (joyCtrl.elem.ack_interrupt_enable) {
			joyStat.elem.irq = SET;
			pBus->pCp0->interruptHandler(_IRQ7);
		}
	}
}

void joyMemCard::cpuRead8(const uint32_t& addr, uint8_t& data) {
	//EmulationPaused = true;
	//std::cout << "[PAD] read8 addr 0x" << addr << std::endl;
	switch (addr) {
	case 0x1f801040:
		data = joy_rx_data.elem.first_entry;
		joyStat.elem.rx_fifo_not_empty = RESET;
		break;
	default:
		std::cout << "[PAD] EMULATION PAUSED! unhandled read8 addr 0x" << addr << std::endl;
		isEmulationPaused = true;
		break;
	}
}

void joyMemCard::cpuWrite8(const uint32_t& addr, const uint8_t& data) {
	//EmulationPaused = true;
	//std::cout << "[PAD] write8 addr 0x" << addr
	//	<< " data 0x" << (uint16_t)data << std::endl;
	switch (addr) {
	case 0x1f801040:
		joy_tx_data.elem.data_to_be_sent = data;

		bSendingData = true;
		sendingClocks = 0x88;
		receivingClocks = 0x88 + 10;
		ackClocks = 100;


		if ((joy_tx_data.elem.data_to_be_sent == 0x1) && (sequvenceIndex == 0)) {
			joy_rx_data.elem.first_entry = 0;
			device = PAD;
			sequvenceIndex++;
		}
		else if ((joy_tx_data.elem.data_to_be_sent == 0x81) && (sequvenceIndex == 0)) {
			joy_rx_data.elem.first_entry = 0;
			device = MEM_CARD;
			bSendingData = false;
			sequvenceIndex++;
		}
		else if ((joy_tx_data.elem.data_to_be_sent == 0x42) && (device == PAD) && (sequvenceIndex == 1)) {
			joy_rx_data.elem.first_entry = 0x41;
			sequvenceIndex++;
		}
		else if ((joy_tx_data.elem.data_to_be_sent == 0x52) && (device == MEM_CARD) && (sequvenceIndex == 1)) {
			joy_rx_data.elem.first_entry = 0x00;
			bSendingData = false;
			sequvenceIndex++;
		}
		else if ((joy_tx_data.elem.data_to_be_sent == 0) && (device == PAD)) {
			if (sequvenceIndex == 2) {
				joy_rx_data.elem.first_entry = 0x5a;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 3) {
				joy_rx_data.elem.first_entry = swlo;
				swlo = 0xff;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 4) {
				joy_rx_data.elem.first_entry = swhi;
				swhi = 0xff;
				sequvenceIndex = 0;
			}
		}
		else if ((joy_tx_data.elem.data_to_be_sent == 0) && (device == MEM_CARD)) {
			bSendingData = false;
			if (sequvenceIndex == 2) {
				joy_rx_data.elem.first_entry = 0x00;
				sequvenceIndex++;
			}
			else if (sequvenceIndex == 3) {
				joy_rx_data.elem.first_entry = 0x00;
				sequvenceIndex = 0;
			}
		}
		else {
			//bSendingData = false;
			std::cout << "[PAD] EMULATION PAUSED! unhandled tx data 0x" << (uint16_t)data
				<< " device " << device << std::endl;
			//isEmulationPaused = true;
		}

		lastTxData = data;
		break;
	default:
		std::cout << "[PAD] EMULATION PAUSED! unhandled write8 addr 0x" << addr
			<< " data 0x" << (uint16_t)data << std::endl;
		isEmulationPaused = true;
		break;
	}
}

void joyMemCard::cpuRead16(const uint32_t& addr, uint16_t& data) {
	//isEmulationPaused = true;
	//std::cout << "[PAD] read16 addr 0x" << addr << std::endl;
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
		isEmulationPaused = true;
		std::cout << "[PAD] EMULATION PAUSED! unhandled read16 addr 0x" << addr << std::endl;
		break;
	}
}

void joyMemCard::cpuWrite16(const uint32_t& addr, const uint16_t& data) {
	//isEmulationPaused = true;
	//std::cout << "[PAD] write16 addr 0x" << addr
	//	<< " data 0x" << data << std::endl;
	switch (addr) {
	case 0x1f801048:
		joyMode.data = data;
		if (data != 0xd) {
			std::cout << "[PAD] EMULATION PAUSED! unhandled joy mode 0x"
				<< data << std::endl;
			isEmulationPaused = true;
		}
		break;
	case 0x1f80104a: {
		joyCtrl.data = data;
		/*if (joy_ctrl.elem.reset)
			reset();
		if (joy_ctrl.elem.tx_enable)
			joyStat.elem.tx_read_flag_1 = SET;
		if (joy_ctrl.elem.acknowledge) {
			joyStat.elem.rxParityError = RESET;
			joyStat.elem.irq = RESET;
		}
		if (joy_ctrl.elem.joyn_output == SET)
			joyStat.elem.irq = SET;
		if (joy_ctrl.elem.joyn_output) {
			b_irq_7_change = true;
			wait_clocks = 100;
		}*/

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
				std::cout << "[PAD] EMULATION PAUSED! unhandled rx interrupt mode 0x"
					<< joyCtrl.elem.rx_interrupt_mode << std::endl;
				isEmulationPaused = true;
			}

			if (joyCtrl.elem.tx_interrupt_enable) {
				std::cout << "[PAD] EMULATION PAUSED! unhandled tx interrupt"
					<< std::endl;
				isEmulationPaused = true;
			}

			if (joyCtrl.elem.rx_interrupt_enable) {
				std::cout << "[PAD] EMULATION PAUSED! unhandled rx interrupt"
					<< std::endl;
				isEmulationPaused = true;
			}
		};

		changeState();
		break;
	}

	case 0x1f80104e:
		joyBaud.baudrateReloadValue = data;
		if (data != 0x88) {
			std::cout << "[PAD] EMULATION PAUSED! unhandled baudrate reload value 0x"
				<< data << std::endl;
			isEmulationPaused = true;
		}
		break;
	default:
		std::cout << "[PAD] EMULATION PAUSED! unhandled write16 addr 0x" << addr
			<< " data 0x" << data << std::endl;
		isEmulationPaused = true;
		break;
	}
}

void joyMemCard::cpuRead32(const uint32_t& addr, uint32_t& data) {
	//isEmulationPaused = true;
	//std::cout << "[PAD] read32 addr 0x" << addr << std::endl;
	switch (addr) {
	case 0x1f801044:
		data = joyStat.data;
		break;
	default:
		std::cout << "[PAD] EMULATION PAUSED! unhandled read32 addr 0x" << addr << std::endl;
		isEmulationPaused = true;
		break;
	}
}

void joyMemCard::cpuWrite32(const uint32_t& addr, const uint32_t& data) {
	//isEmulationPaused = true;
	std::cout << "[PAD] write32 addr 0x" << " data 0x" << data << std::endl;
	switch (addr) {
	default:
		std::cout << "[PAD] EMULATION PAUSED! unhandled write32 addr 0x" << addr
			<< " data 0x" << data << std::endl;
		isEmulationPaused = true;
		break;
	}
}

void joyMemCard::reset() {
	joy_tx_data.data = RESET;
	joy_rx_data.data = RESET;
	joyStat.data = RESET;
	joyMode.data = RESET;
	joyCtrl.data = RESET;
	joyBaud.baudrateReloadValue = RESET;
}

void joyMemCard::acknowledge() {
	joyStat.elem.rxParityError = RESET;
	joyStat.elem.irq = RESET;
}
