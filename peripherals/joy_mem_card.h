#pragma once

#include <cstdint>
#include <sstream>
#include <vector>

#include "../debug_utilities/debug_utilities.h"

class busInterface;

class joyMemCard {
public:
	joyMemCard();

	busInterface* pBus;

	void clock();

	void cpuRead8(const uint32_t& addr, uint8_t& data);
	void cpuWrite8(const uint32_t& addr, const uint8_t& data);

	void cpuRead16(const uint32_t& addr, uint16_t& data);
	void cpuWrite16(const uint32_t& addr, const uint16_t& data);

	void cpuRead32(const uint32_t& addr, uint32_t& data);
	void cpuWrite32(const uint32_t& addr, const uint32_t& data);

	union JoyTxData {
		uint32_t data;
		struct {
			uint32_t dataToBeSent : 8;
			uint32_t : 24;
		}elem;
	}joyTxData;

	union JoyRxData {
		uint32_t data;
		struct {
			uint32_t firstEntry : 8;
			uint32_t second_entry : 8;
			uint32_t third_entry : 8;
			uint32_t fourth_entry : 8;
		}elem;
	}joyRxData;


	enum voltageLevel_t : uint32_t {
		HIGH = 0,
		LOW = 1
	};

	union JoyStat {
		uint32_t data;
		struct {
			uint32_t tx_read_flag_1 : 1;
			uint32_t rx_fifo_not_empty : 1;
			uint32_t tx_read_flag_2 : 1;
			uint32_t rxParityError : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			voltageLevel_t ackInputLevel : 1;
			uint32_t : 1;
			uint32_t irq : 1;
			uint32_t : 1;
			uint32_t baudrate_timer : 21;
		}elem;
	}joyStat;

	union JoyMode {
		uint16_t data;
		struct {
			uint16_t baudrate_reload_factor : 2;
			uint16_t character_length : 2;
			uint16_t parity_enable : 1;
			uint16_t parity_type : 1;
			uint16_t : 2;
			uint16_t clk_output_polarity : 1;
			uint16_t : 7;
		}elem;
	}joyMode;

	union JoyCtrl {
		uint16_t data;
		struct {
			uint16_t tx_enable : 1;
			uint16_t joynOutput : 1;
			uint16_t rx_enable : 1;
			uint16_t : 1;
			uint16_t acknowledge : 1;
			uint16_t : 1;
			uint16_t reset : 1;
			uint16_t : 1;
			uint16_t rx_interrupt_mode : 2;
			uint16_t tx_interrupt_enable : 1;
			uint16_t rx_interrupt_enable : 1;
			uint16_t ack_interrupt_enable : 1;
			uint16_t desiredSlotNumber : 1;
			uint16_t : 2;
		}elem;
	}joyCtrl;

	union JoyBaud {
		uint16_t baudrateReloadValue;
	}joyBaud;

	void reset();
	void acknowledge();

	enum slot_t {
		JOY1 = 0,
		JOY2 = 1
	};

	voltageLevel_t ackInputLevel = HIGH;

	struct joy_t {
		voltageLevel_t output;
		slot_t slotNumber;
	}joy;

	enum device_t {
		PAD = 0,
		MEM_CARD = 1
	}device;

	enum mode_t : uint8_t{
		unused1 = 0x40,
		unused2 = 0x41,
		readButtons = 0x42,
		enterExitConfig = 0x43,
		setLedState = 0x44,
		getLedState = 0x45,
		getVariableResponseA = 0x46,
		getWhateverValues = 0x47,
		unknown = 0x48,
		unused3 = 0x49,
		unused4 = 0x4a,
		unused5 = 0x4b,
		getVariableResponseB = 0x4c,
		getSetRumbleProtocol = 0x4d,
		unused6 = 0x4e,
		unused7 = 0x4f,
	}mode;
	bool bConfigurationMode = false;
	bool bChangeMode = false;
	uint8_t led = 0x00;
	uint8_t key;
	uint8_t aa = 0x00;
	uint8_t bb = 0x00;
	uint8_t cc;
	uint8_t dd;
	uint8_t ee;
	uint8_t ff;

	uint8_t sequvenceIndex = 0;

	bool txen = false;
	bool rxen = false;
	uint64_t clocks = 0;
	uint64_t sendingClocks = 0;
	uint64_t receivingClocks = 0;
	uint64_t ackClocks = 0;
	uint64_t ackPendingClocks = 0;

	bool bSendingData = false;
	bool bReceivingData = false;
	bool bACK = false;

	std::vector<uint8_t> rxFifo;

	bool b_irq_7_change = false;
	uint8_t swloPad1 = 0xff;
	uint8_t swhiPad1 = 0xff;
	uint8_t swloPad2 = 0xff;
	uint8_t swhiPad2 = 0xff;
	uint8_t tx = 0;

	uint8_t msb;
	uint8_t lsb;

	const uint32_t NUMBER_OF_BLOCKS = 16;
	const uint32_t NUMBER_OF_FRAMES_PRE_BLOCK = 64; //sectors
	const uint32_t SIZE_OF_FRAME = 128;
	std::vector<uint8_t> memCard1;

	//debug
	debugUtilities* p_debugger;
};